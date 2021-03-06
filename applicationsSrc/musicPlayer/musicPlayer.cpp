/*
 * blinky01.cpp
 *
 *  Created on: 23 mars 2013
 *      Author: dom
 */

#include <iostream>
#include "robots/blinkyBlocks/blinkyBlocksSimulator.h"
#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"
#include "musicPlayerBlockCode.h"
#include "utils/trace.h"

using namespace std;
using namespace BlinkyBlocks;

int main(int argc, char **argv) {

    OUTPUT << "\033[1;33m" << "Starting Blinky Blocks simulation (main) ..." << "\033[0m" << endl;

    createSimulator(argc, argv, MusicPlayerBlockCode::buildNewBlockCode);

    {
        using namespace BaseSimulator;

        Simulator *s = Simulator::getSimulator();
        s->printInfo();
    }

    getSimulator()->printInfo();
    getScheduler()->printInfo();
    BaseSimulator::getWorld()->printInfo();

    getScheduler()->waitForSchedulerEnd();

    deleteSimulator();

    OUTPUT << "\033[1;33m" << "end (main)" << "\033[0m" << endl;
    return(0);
}
