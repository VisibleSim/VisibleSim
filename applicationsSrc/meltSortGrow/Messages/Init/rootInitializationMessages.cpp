/*
 * rootInitializationMessages.cpp
 *
 *  Created on: 12/02/2018
 *      Author: pthalamy
 */

#include "rootInitializationMessages.hpp"
#include "../../api.hpp"

void RootUpdateMessage::handle(BaseSimulator::BlockCode* bsbc) {
    MeltSortGrowBlockCode *bc = static_cast<MeltSortGrowBlockCode*>(bsbc);
        
    if (bc->challengeRootFitness(candidateRootPosition)) {
        bc->currentRootPosition = candidateRootPosition;
            
        // Broadcast new root candidate to every neighbor except sender
        bc->parent = destinationInterface;
        bc->sendMessageToAllNeighbors("RootUpdate",
                                      new RootUpdateMessage(bc->currentRootPosition),
                                      0, MSG_DELAY, 1, bc->parent);
        bc->expectedConfirms = bc->catom->getNbNeighbors() - 1; // Ignore parent

        if (!bc->expectedConfirms) {
            bc->sendMessage(new RootConfirmationMessage(bc->currentRootPosition),
                            bc->parent, 0, MSG_DELAY);
        }    
    } else {
        // Received best root twice, send a NACK
        bc->sendMessage(new RootRefusalMessage(candidateRootPosition),
                        destinationInterface, 0, MSG_DELAY);
    }
}

void handleRootUpdateAnswer(BaseSimulator::BlockCode* bsbc,
                            Cell3DPosition& rootPos) {
    MeltSortGrowBlockCode *bc = static_cast<MeltSortGrowBlockCode*>(bsbc);
    
    if (rootPos == bc->currentRootPosition)
        --bc->expectedConfirms;
            
    if (!bc->expectedConfirms) {
        if (bc->parent)
            bc->sendMessage(new RootConfirmationMessage(bc->currentRootPosition),
                            bc->parent, 0, MSG_DELAY);
        else {
            if (bc->catom->position == bc->currentRootPosition) { // Election Complete, module is root
                bc->relPos = new Cell3DPosition(0,0,0);//?
                bc->rtg->setOrigin(bc->catom->position);
                cout << "Absolute Target Positions" << endl; 
                bc->rtg->getTargetCellsInConstructionOrder()->pop_front(); // this is the root's target position, in which it already is
                // cout << "xyz removing cell from target: " << bc->catom->position << endl;
                // bc->rtg->removeTargetCell(bc->catom->position);
                cout << bc->rtg << endl;
                // bc->rtg->highlight();
                bc->lattice->highlightCell(bc->catom->position, GREEN);
                bc->rtg->highlightByDistanceToRoot();
                        
                // Proceed to next stage
                bc->meltOneModule();
            }
        }
    }    
}


void RootConfirmationMessage::handle(BaseSimulator::BlockCode* bsbc) {
    handleRootUpdateAnswer(bsbc, confirmationRootPosition);
}

void RootRefusalMessage::handle(BaseSimulator::BlockCode* bsbc) {
    handleRootUpdateAnswer(bsbc, refusedRootPosition);
}
