#include "coatingBlockCode.hpp"

#include "coatingUtils.hpp"

using namespace Catoms3D;

CoatingBlockCode::CoatingBlockCode(Catoms3DBlock *host) : Catoms3DBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Registers a callback (handleSampleMessage) to the message of type SAMPLE_MSG_ID
    // addMessageEventFunc2(SAMPLE_MSG_ID,
    //                      std::bind(&CoatingBlockCode::handleSampleMessage, this,
    //                                std::placeholders::_1, std::placeholders::_2));

    world = BaseSimulator::getWorld();
    // set the module pointer
    catom = static_cast<Catoms3DBlock*>(hostBlock);

    isInG = CoatingBlockCode::isInCSG;
    if (not neighborhood) neighborhood = new Neighborhood(isInG);
  }

void CoatingBlockCode::startup() {
    if (catom->blockId == 1) COATING_SEED_POS = catom->position; // FIXME:

    if (HIGHLIGHT_COATING or HIGHLIGHT_CSG or HIGHLIGHT_SEEDS) {
        highlight();
        HIGHLIGHT_COATING = false;
        HIGHLIGHT_CSG = false;
        HIGHLIGHT_SEEDS = false;
    }

    // Simulate authorizations
    if (watchlist.find(catom->position) != watchlist.end()) {
        auto const& it = watchlist.find(catom->position);

        stringstream info;
        info << " provides attract authorization to " << it->first;
        scheduler->trace(info.str(), catom->blockId, AUTH_DEBUG_COLOR);

        const auto& callback = it->second;
        callback(catom->position);
        watchlist.erase(it);
    }

    if (catom->position == COATING_SEED_POS or isInG(catom->position)) {
        attract();
    }
}

void CoatingBlockCode::handleSampleMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    MessageOf<int>* msg = static_cast<MessageOf<int>*>(msgPtr.get());
}

void CoatingBlockCode::onMotionEnd() {
    console << " has reached " << catom->position << "\n";

    // do stuff
    // ...
}

void CoatingBlockCode::processLocalEvent(EventPtr pev) {
    MessagePtr message;
    stringstream info;

    // Do not remove line below
    BlockCode::processLocalEvent(pev);

    switch (pev->eventType) {
        case EVENT_ADD_NEIGHBOR: {
            // Do something when a neighbor is added to an interface of the catom
            break;
        }

        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the catom
            break;
        }
    }
}

/// ADVANCED BLOCKCODE FUNCTIONS BELOW

void CoatingBlockCode::onBlockSelected() {
    // Debug stuff:
    cout << endl << "--- PRINT CATOM " << *catom << "---" << endl;

    cout << "isNorthSeed(" << catom->position << "): "
         << neighborhood->isNorthSeed(catom->position) << endl;
}

void CoatingBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

bool CoatingBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
    /* Reading the command line */
    if ((argc > 0) && ((*argv)[0][0] == '-')) {
        switch((*argv)[0][1]) {

            // Single character example: -b
            case 'b':   {
                cout << "-b option provided" << endl;
                return true;
            } break;

            // Composite argument example: --foo 13
            case '-': {
                string varg = string((*argv)[0] + 2); // argv[0] without "--"
                if (varg == string("coating")) { //
                    HIGHLIGHT_COATING = true;

                    try {
                        HIGHLIGHT_COATING_LAYER = stoi((*argv)[1]);
                        argc--;
                        (*argv)++;
                    } catch(std::logic_error&) {}

                    cout << "--coating option provided with value: "
                         << HIGHLIGHT_COATING_LAYER << endl;
                } else if (varg == string("csg")) {
                    HIGHLIGHT_CSG = true;

                    cout << "--csg option provided" << endl;
                } else if (varg == string("seeds")) {
                    HIGHLIGHT_SEEDS = true;

                    cout << "--seeds option provided" << endl;
                } else {
                    return false;
                }

                return true;
            }
        }
    }

    return false;
}

void CoatingBlockCode::highlight() const {
    if (HIGHLIGHT_CSG) target->highlight();

    if (HIGHLIGHT_COATING) {
        Cell3DPosition pos;
        for (short iz = 0; iz <= lattice->getGridUpperBounds()[2]; iz++) {
            const Cell3DPosition& glb = lattice->getGridLowerBounds(iz);
            const Cell3DPosition& ulb = lattice->getGridUpperBounds(iz);
            for (short iy = glb[1]; iy <= ulb[1]; iy++) {
                for (short ix = glb[0]; ix <= ulb[0]; ix++) {
                    pos.set(ix,iy,iz);

                    if (isInCoatingLayer(pos, HIGHLIGHT_COATING_LAYER))
                        lattice->highlightCell(pos);
                }
            }
        }
    }

    if (HIGHLIGHT_SEEDS) {
        lattice->highlightAllCellsThatVerify(
            [this](const Cell3DPosition& p) { return neighborhood->isNorthSeed(p); }, GREEN);
        lattice->highlightAllCellsThatVerify(
            [this](const Cell3DPosition& p) { return neighborhood->isSouthSeed(p); }, ORANGE);
        lattice->highlightAllCellsThatVerify( [this](const Cell3DPosition& p) {
            return neighborhood->isNorthLineOnMerge(p); }, RED);
        lattice->highlightAllCellsThatVerify([this](const Cell3DPosition& p) {
            return neighborhood->isSouthLineOnMerge(p); }, BLUE);
    }
}

int CoatingBlockCode::getCoatingLayer(const Cell3DPosition& pos) {
    return pos[2] - COATING_SEED_POS[2];
}

bool CoatingBlockCode::isInCoating(const Cell3DPosition& pos) {
    return BaseSimulator::getWorld()->lattice->isInGrid(pos)
        and pos[2] >= COATING_SEED_POS[2] and isInCoatingLayer(pos, getCoatingLayer(pos));
}

bool CoatingBlockCode::isInCoatingLayer(const Cell3DPosition& pos, int layer) {
    int pLayer = getCoatingLayer(pos);

    if (isInCSG(pos)) return false;

    return (layer == -1 or pLayer == layer)
        // and hasNeighborInCSG(pos);  // Coating at distance 1
        and not hasHorizontalNeighborInCSG(pos) and has2ndOrderNeighborInCSG(pos); // Coating distance 2
}

bool CoatingBlockCode::hasHorizontalNeighborInCSG(const Cell3DPosition& pos) {
    for (const Cell3DPosition& p : BaseSimulator::getWorld()->lattice->getNeighborhood(pos)) {
        if (p[2] < pos[2]) continue;

        if (isInCSG(p)) return true;
    }

    // for (const Cell3DPosition& pRel : diagNeighbors) {
    //     if (isInCSG(pRel + pos)) return true;
    // }

    return false;
}

bool CoatingBlockCode::has2ndOrderNeighborInCSG(const Cell3DPosition& pos) {
    for (const Cell3DPosition& pRel : _2ndOrderNeighbors) {
        if (isInCSG(pRel + pos)) return true;
    }

   return false;
}

void CoatingBlockCode::attract() {
    stringstream info;

    // North attraction
    if (neighborhood->isNorthSeed(catom->position)) {
        sendAttractSignalTo(catom->position.addY(1));
    }

    // South attraction
    if (neighborhood->isSouthSeed(catom->position)) {
        sendAttractSignalTo(catom->position.addY(-1));
    }

    // West attraction
    if (neighborhood->directionIsInCSG(catom->position, West)
        and not hasNeighborInDirection(SkewFCCLattice::Direction::C6West)) {
        const Cell3DPosition& wPos = neighborhood->cellInDirection(catom->position, West);
        if (neighborhood->directionIsInCSG(catom->position, SouthWest)
            and hasNeighborInDirection(SkewFCCLattice::Direction::C7South)) {
            if (getAuthorizationToAttract(wPos, West)) {
                sendAttractSignalTo(wPos);
            }
        } else if (neighborhood->isOnInternalHole(catom->position)) {
            info << " sends a WEST border following request for " << wPos;
            scheduler->trace(info.str(),catom->blockId, ATTRACT_DEBUG_COLOR);

            borderFollowingAttractRequestTo(wPos);
        } else {
            sendAttractSignalTo(wPos);
        }
    }

    // East attraction
    if (neighborhood->directionIsInCSG(catom->position, East)
        and not hasNeighborInDirection(SkewFCCLattice::Direction::C0East)) {
        const Cell3DPosition& ePos = neighborhood->cellInDirection(catom->position, East);
        if (neighborhood->directionIsInCSG(catom->position, NorthEast)
            and hasNeighborInDirection(SkewFCCLattice::Direction::C1North)) {
            if (getAuthorizationToAttract(ePos, East)) {
                sendAttractSignalTo(ePos);
            }
        } else if (neighborhood->isOnInternalHole(catom->position)) {
            borderFollowingAttractRequestTo(ePos);
        } else {
            sendAttractSignalTo(ePos);
        }
    }
}

bool CoatingBlockCode::hasNeighborInDirection(SkewFCCLattice::Direction dir) const {
    return lattice->cellHasBlock(lattice->getCellInDirection(catom->position, dir));
}

void CoatingBlockCode::sendAttractSignalTo(const Cell3DPosition& pos) {
    stringstream info;
    info << " attracts to " << CCWDirectionStringForPosition(catom->position - pos)
         << " position " << pos;
    scheduler->trace(info.str(), catom->blockId, ATTRACT_DEBUG_COLOR);

    world->addBlock(0, buildNewBlockCode, pos, YELLOW);
}

bool CoatingBlockCode::getAuthorizationToAttract(const Cell3DPosition& requester,
                                                 CCWDir d) {
    stringstream info;
    info << " requests authorization to attract " << CCWDirectionStringForDirectionIndex(d)
         << " from " << requester;
    scheduler->trace(info.str(), catom->blockId, AUTH_DEBUG_COLOR);

    const Cell3DPosition& target = requester + CCWDPos[d];
    if (lattice->getBlock(target)) {
        stringstream info;
        info << " modules in place, self-granting authorization from " << requester;
        scheduler->trace(info.str(), catom->blockId, AUTH_DEBUG_COLOR);

        return true;
    }

    // Else. Add to watchlist and get notified when module is added
    VS_ASSERT(watchlist.find(target) == watchlist.end());
    watchlist.emplace(requester, std::bind(&CoatingBlockCode::sendAttractSignalTo, this,
                                           std::placeholders::_1));
    return false;
}

bool CoatingBlockCode::borderFollowingAttractRequestTo(const Cell3DPosition& pos) {
    stringstream info;
    info << " requests BORDER authorization for " << pos;
    scheduler->trace(info.str(), catom->blockId, AUTH_DEBUG_COLOR);

    return true;
}
