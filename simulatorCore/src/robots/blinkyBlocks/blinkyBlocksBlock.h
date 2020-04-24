/*
 * blinkyBlocksBlock.h
 *
 *  Created on: 23 mars 2013
 *      Author: dom
 */

#ifndef BLINKYBLOCKSBLOCK_H_
#define BLINKYBLOCKSBLOCK_H_

#include <stdexcept>

#include "base/buildingBlock.h"
#include "utils/color.h"
#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"
#include "robots/blinkyBlocks/blinkyBlocksGlBlock.h"
#include "grid/lattice.h"

namespace BlinkyBlocks {

class BlinkyBlocksBlockCode;

class BlinkyBlocksBlock : public BaseSimulator::BuildingBlock {
public:
    BlinkyBlocksBlock(int bId, BlockCodeBuilder bcb);
    ~BlinkyBlocksBlock();

    inline virtual BlinkyBlocksGlBlock* getGlBlock() const override {
        return (BlinkyBlocksGlBlock *)ptrGlBlock;
    };

    P2PNetworkInterface *getInterfaceDestId(int id) const;
    int getDirection(P2PNetworkInterface*) const override;

    /* schedule the appropriate event for this action */
    /* void tap(Time date); Now a generic event in buildingBlock.cpp */
    void accel(Time date, int x, int y, int z);
    void shake(Time date, int f);

    void addNeighbor(P2PNetworkInterface *ni, BuildingBlock* target) override;
    void removeNeighbor(P2PNetworkInterface *ni) override;
    void stopBlock(Time date, State s);
    void pauseClock(Time delay, Time start);


    /**
     * @copydoc BuildingBlock::canMoveTo
     */
    virtual bool canMoveTo(const Cell3DPosition& dest) const override { return false; }

    /**
     * @copydoc BuildingBlock::moveTo
     */
    virtual bool moveTo(const Cell3DPosition& dest) override;
};

std::ostream& operator<<(std::ostream &stream, BlinkyBlocksBlock const& bb);

}

#endif /* BLINKYBLOCKSBLOCK_H_ */
