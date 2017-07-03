/*
 *  neighborhood.h
 *
 *  Created on: 28 February 2017
 *  Author: Thadeu
 */

#ifndef NEIGHBOR_H_
#define NEIGHBOR_H_

#include "cell3DPosition.h"
#include "directions.h"
#include "../reconf.h"
#include "../sync/syncNext.h"
#include "../sync/syncPrevious.h"

class Neighborhood {
private:
    Catoms3D::Catoms3DBlock *catom;
    Reconf *reconf;
    SyncNext *syncNext;
    SyncPrevious *syncPrevious;
    BlockCodeBuilder blockCodeBuilder;

    void addNeighbor(Cell3DPosition pos);
    void addNeighborToLeft();
    void addNeighborToRight();

public:
    Neighborhood(Catoms3D::Catoms3DBlock *catom, Reconf *reconf, SyncNext *sn, SyncPrevious *sp, BlockCodeBuilder bcb);

    void addNeighborsWithoutSync();
    void addAllNeighbors();

    void tryAddNextLineNeighbor();
    void tryAddPreviousLineNeighbor();
    void tryAddNeighbors();

    bool isOnLeftBorder();
    bool isOnRightBorder();
    bool isFirstCatomOfLine();
    bool isFirstCatomOfPlane();

    void addNeighborToNextPlane();
    void addNeighborToPreviousPlane();
};

#endif /* NEIGHBORHOOD_H_ */
