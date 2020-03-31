#include <climits>
#include "datomsRotateCode.h"
#include "datomsMotionEngine.h"

bool distanceCalculated=false;
vector<unsigned int> tabCellByDistance;

void DatomsRotateCode::startup() {
	lattice = (SkewFCCLattice*)(Datoms::getWorld()->lattice);

	if (module->blockId==1) {
		lattice->initTabDistances();
		lattice->setDistance(module->position,0);
		tabCellByDistance.push_back(1);
		initDistances();
	}
}

void DatomsRotateCode::initDistances() {
	short distance=lattice->getDistance(module->position)+1;
	vector<Cell3DPosition> neighbor = lattice->getNeighborhood(module->position); //
	Cell3DPosition pDest;
	short r;
	vector<std::pair<const DatomsMotionRulesLink*, Deformation>> tab = DatomsMotionEngine::getAllDeformationsForModule(module);
	for (auto p:tab) {
		p.second.getFinalPositionAndOrientation(pDest,r);
		unsigned short d = lattice->getDistance(pDest);
		if (d>distance) {
//			cout << "--> new Distance" << pDest << "," << distance << endl;
			if (d!=USHRT_MAX) {
				tabCellByDistance[d]--;
			}
			if (tabCellByDistance.size()<=distance) {
//				cout << "create " << distance << endl;
				tabCellByDistance.push_back(0);
			}
			tabCellByDistance[distance]++;
			lattice->setDistance(pDest,distance);
			cellsList.push(pDest);
		}
	}

	if (!cellsList.empty()) {
		pDest = cellsList.front();
		cellsList.pop();
		getScheduler()->schedule(new TeleportationStartEvent(scheduler->now()+2000,module,pDest));
	} else {
		distanceCalculated=true;
		pDest.set(0,0,2);
		lattice->pushPtLine(pDest);
		getScheduler()->schedule(new TeleportationStartEvent(scheduler->now()+2000000,module,pDest));
	}
}

bool DatomsRotateCode::tryToMove() {
	vector<std::pair<const DatomsMotionRulesLink*, Deformation>> tab = DatomsMotionEngine::getAllDeformationsForModule(module);
	Cell3DPosition pos;
	unsigned short d,dmin=USHRT_MAX;
	int i=0,imin=-1;
	short n;
	OUTPUT << "Search dest:" << endl;
	for (std::pair<const DatomsMotionRulesLink*, Deformation> v:tab) {
		v.second.getFinalPositionAndOrientation(pos,n);
		OUTPUT << v.second.ptrPivot->blockId << ":" << v.first->getConFromID() << "->" << v.first->getConToID() << "  ";
		OUTPUT << pos << ":";
		d = lattice->getDistance(pos);
		OUTPUT << d << " ";
		if (d<dmin) {
			imin=i;
			dmin=d;
			OUTPUT << "*";
		}
		OUTPUT << endl;
		i++;
	}
	if (imin!=-1) {
		getScheduler()->schedule(new DeformationStartEvent(scheduler->now()+2000,module,tab[imin].second));
	}
	return false;
}

void DatomsRotateCode::onMotionEnd() {
	//OUTPUT << "onMotionEnd" << endl;
	if (distanceCalculated) {
		int i=0;
		for (unsigned int t:tabCellByDistance) {
			cout << i << ": " << t << endl;
			i++;
		}

		/*lattice->showTabDistances(false);
		if (lattice->getDistance(module->position)>0) {
			tryToMove();
		}
		lattice->ptsLine.push_back(module->position);*/
	} else {
		initDistances();
	}
}
