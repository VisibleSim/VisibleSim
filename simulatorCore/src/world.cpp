/*
 * world.cpp
 *
 *  Created on: 23 mars 2013
 *      Author: dom
 */

#include <stdlib.h>

#include "world.h"
#include "trace.h"
#include "openglViewer.h"

using namespace std;

namespace BaseSimulator {

World *World::world=NULL;
map<int, BuildingBlock*>World::buildingBlocksMap;
vector <GlBlock*>World::tabGlBlocks;

World::World(int argc, char *argv[]) {
	OUTPUT << "World constructor" << endl;
	selectedBlock = NULL;
	numSelectedFace=0;
	numSelectedBlock=0;
	menuId = 0;

	if (world == NULL) {
		world = this;

		GlutContext::init(argc,argv);

		camera = new Camera(-M_PI/2.0,M_PI/3.0,750.0);
		camera->setLightParameters(Vector3D(0,0,0),45.0,80.0,800.0,45.0,10.0,1500.0);
		camera->setTarget(Vector3D(0,0,1.0));
	} else {
		ERRPUT << "\033[1;31m" << "Only one World instance can be created, aborting !" << "\033[0m" << endl;
		exit(EXIT_FAILURE);
	}
}

World::~World() {
	//MODIF NICO
	std::map<int, BuildingBlock*>::iterator it;
	for( it = buildingBlocksMap.begin() ; it != buildingBlocksMap.end() ; ++it) {
		delete it->second;
	}
	//FIN MODIF NICO

	std::vector<GlBlock*>::const_iterator cit=tabGlBlocks.begin();
	while (cit!=tabGlBlocks.end()) {
		delete *cit;
		cit++;
	}

	delete lattice;
	delete camera;

	OUTPUT << "World destructor" << endl;
}


BuildingBlock* World::getBlockById(int bId) {
	map<int, BuildingBlock*>::iterator it;
	it = buildingBlocksMap.find(bId);
	if (it == buildingBlocksMap.end()) {
		return(NULL);
	} else {
		return(it->second);
	}
}

void World::updateGlData(BuildingBlock *bb) {
	GlBlock *glblc = bb->getGlBlock();
	if (glblc) {
		lock();
		glblc->setPosition(lattice->gridToWorldPosition(bb->position));
		glblc->setColor(bb->color);
		unlock();
	}
}

void World::updateGlData(BuildingBlock*blc, Vector3D &p) {
	GlBlock *glblc = blc->getGlBlock();
	if (glblc) {
		lock();
		glblc->setPosition(p);
		unlock();
	}
}

void World::linkBlocks() {
	Cell3DPosition p;

	for (p.pt[2] = 0; p[2] < lattice->gridSize[2]; p.pt[2]++) { // z
		for (p.pt[1] = 0; p[1] < lattice->gridSize[1]; p.pt[1]++) { // y
			for(p.pt[0] = 0; p[0] < lattice->gridSize[0]; p.pt[0]++) { // x
				if (lattice->cellHasBlock(p)) {
					// cerr << "l.cellHasBlock(" << p << "/" << lattice->getIndex(p) << ")  = true ; id: "
					//	 << lattice->getBlock(p)->blockId << endl;
					linkBlock(p);
				}
			}
		}
	}
}

void World::linkNeighbors(const Cell3DPosition &pos) {
	vector<Cell3DPosition> nCells = lattice->getActiveNeighborCells(pos);

	// Check neighbors for each interface
	for (Cell3DPosition nPos : nCells) {
		linkBlock(nPos);
	}
}

// PTHY: TODO - interface refactoring
// void World::linkBlock(const Cell3DPosition &pos) {
//     BlinkyBlocksBlock *ptrNeighbor;
//     vector<Cell3DPosition> nCells;
//     Cell3DPosition nP;
//     BlinkyBlocksBlock *ptrBlock = (BlinkyBlocksBlock*)lattice->getBlock(pos);

//     // There is a block on cell pos
//     nCells = lattice->getNeighborhood(ptrBlock);

//     // Check neighbors for each interface
//     for (int i = 0; i < NeighborDirection::; i++) {
//	nP = pos + nCells[i];
//	ptrNeighbor = (BlinkyBlocksBlock*)lattice->getBlock(nP);
//	if (ptrNeighbor) {
//		(ptrBlock)->getInterface(NeighborDirection::Direction(i))->
//		connect(ptrNeighbor->getInterface(NeighborDirection::Direction(
//							  NeighborDirection::getOpposite(i))));

//		OUTPUT << "connection #" << (ptrBlock)->blockId <<
//		" to #" << ptrNeighbor->blockId << endl;
//	} else {
//		(ptrBlock)->getInterface(NeighborDirection::Direction(i))->connect(NULL);
//	}
//     }
// }

void World::stopSimulation() {
	map<int, BuildingBlock*>::iterator it;
	for( it = buildingBlocksMap.begin() ; it != buildingBlocksMap.end() ; it++) {
		// it->second->stop();
	}
}

static void swap(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void World::generateIds(int n, int* ids) {
	int a = 0, b = 0;

	//struct timespec t;
	//clock_gettime(CLOCK_REALTIME, &t);
	// not implemented in macos
	//boost::rand48 generator = boost::rand48(t.tv_nsec);

	std::random_device rd;
	std::ranlux48 generator = std::ranlux48(rd());

	for (int i = 0; i < n; i++) {
		ids[i] = i+1;
	}

	if (n==1) {
		return;
	}

	// randomly switch 2n times
	for (int i = 0; i < n*2; i++) {
		do {
			a = generator() % n;
			b = generator() % n;
		} while (a == b);
		swap(&ids[a], &ids[b]);
	}
}

/**
 * @brief Indicates if a block can be added to face numSelectedFace of block with id numSelectedBlock
 *
 *
 * @param numSeletectedBlock : id of the block to consider
 * @param numSelectedFace : id of the face to consider on the block
 * @return true if a block can be added to the cell corresponding to the face of the block
 */
bool World::canAddBlockToFace(int numSelectedBlock, int numSelectedFace) {
	BuildingBlock *bb = getBlockById(tabGlBlocks[numSelectedBlock]->blockId);
	Cell3DPosition pos = bb->position;
	vector<Cell3DPosition> nCells = lattice->getRelativeConnectivity(pos);

	if (numSelectedFace < nCells.size())
		cerr << "numSelectedFace: " << numSelectedFace << " f"
			 << pos << "+" << nCells[numSelectedFace] 
			 << " = " << lattice->isFree(pos + nCells[numSelectedFace]) << endl;
	
	return numSelectedFace < nCells.size() ?
							 lattice->isFree(pos + nCells[numSelectedFace]) : false;
}

void World::menuChoice(int n) {
    BuildingBlock *bb = getMenuBlock();
	
	switch (n) {
	case 1 : {
		OUTPUT << "ADD block link to : " << bb->blockId << "     num Face : " << numSelectedFace << endl;
		vector<Cell3DPosition> nCells = lattice->getRelativeConnectivity(bb->position);
		Cell3DPosition nPos = bb->position + nCells[numSelectedFace];
		
		addBlock(-1, (BlockCode *(*)(BuildingBlock *))bb->buildNewBlockCode, nPos, bb->color);
		linkBlock(nPos);
		linkNeighbors(nPos);
	} break;
	case 2 : {
		OUTPUT << "DEL num block : " << tabGlBlocks[numSelectedBlock]->blockId << endl;
		deleteBlock(bb);
	} break;
	case 3 : {
		tapBlock(getScheduler()->now(), bb->blockId);
	} break;
	case 4:                 // Save current configuration
		exportConfiguration();
		break;
	}
}

void World::createHelpWindow() {
	if (GlutContext::helpWindow)
		delete GlutContext::helpWindow;
	GlutContext::helpWindow = new GlutHelpWindow(NULL,10,40,540,500,"../../simulatorCore/genericHelp.txt");
}

/**
 * @brief Schedules a tap event for block with id bId, at time date.
 *
 * @param date the date at which the tap event must be consumed
 * @param bId the id of the target block
 */
void World::tapBlock(uint64_t date, int bId) {
	BuildingBlock *bb = getBlockById(bId);
	bb->tap(date);
}

void World::createPopupMenu(int ix, int iy) {
	if (!GlutContext::popupMenu) {
		GlutContext::popupMenu = new GlutPopupMenuWindow(NULL,0,0,200,180);
		GlutContext::popupMenu->addButton(1,"../../simulatorCore/menuTextures/menu_add.tga");
		GlutContext::popupMenu->addButton(2,"../../simulatorCore/menuTextures/menu_del.tga");
		GlutContext::popupMenu->addButton(3,"../../simulatorCore/menuTextures/menu_tap.tga");
		GlutContext::popupMenu->addButton(4,"../../simulatorCore/menuTextures/menu_save.tga");
		GlutContext::popupMenu->addButton(5,"../../simulatorCore/menuTextures/menu_cancel.tga");
	}

	if (iy < GlutContext::popupMenu->h) iy = GlutContext::popupMenu->h;

	cerr << "Block " << numSelectedBlock << ":" << numSelectedFace << " selected" << endl;

	GlutContext::popupMenu->activate(1, canAddBlockToFace((int)numSelectedBlock, (int)numSelectedFace));
	GlutContext::popupMenu->setCenterPosition(ix,GlutContext::screenHeight-iy);
	GlutContext::popupMenu->show(true);
}

} // BaseSimulator namespace
