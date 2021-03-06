/*
 *  neighborhood.h
 *
 *  Created on: 28 February 2017
 *  Author: Thadeu
 */

#ifndef NEIGHBOR_H_
#define NEIGHBOR_H_

#define CANFILLLEFT_MESSAGE_ID 12001
#define CANFILLLEFTRESPONSE_MESSAGE_ID 12002
#define CANFILLRIGHT_MESSAGE_ID 12003
#define CANFILLRIGHTRESPONSE_MESSAGE_ID 12004
#define ADDNEXTLINE_EVENT_ID 12005
#define ADDPREVIOUSLINE_EVENT_ID 12006
#define ADDLEFTBLOCK_EVENT_ID 12007
#define ADDRIGHTBLOCK_EVENT_ID 12008

#include "grid/cell3DPosition.h"
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


public:
    static int numberBlockedModules;
    static int numberMessagesToAddBlock;
    Neighborhood(Catoms3D::Catoms3DBlock *catom, Reconf *reconf, SyncNext *sn, SyncPrevious *sp, BlockCodeBuilder bcb);

    void addAllNeighbors();
    bool addFirstNeighbor();
    bool addNeighbor(Cell3DPosition pos);

    void addNeighborToLeft();
    void addNeighborToRight();
    void addNextLineNeighbor();
    void addPreviousLineNeighbor();
    void addNeighborToNextPlane();
    void addNeighborToPreviousPlane();

    bool isFirstCatomOfLine();
    bool isFirstCatomOfPlane();

    void addEventAddNextLineNeighbor();
    void addEventAddPreviousLineNeighbor();
    void sendResponseMessageToAddLeft();
    void sendResponseMessageToAddRight();

    void addNeighbors();
    void addNext();
    void addPrevious();
    void addLeft();
    void addRight();
};

class AddNextLine_event : public BlockEvent {
public:
    AddNextLine_event(Time t, BaseSimulator::BuildingBlock *conBlock) : BlockEvent(t, conBlock) {
        eventType = ADDNEXTLINE_EVENT_ID;
    }
    AddNextLine_event(AddNextLine_event *conBlock) : BlockEvent(conBlock) {
    }

    void consumeBlockEvent() override {
        concernedBlock->scheduleLocalEvent(EventPtr(new AddNextLine_event(this)));
    }

    const string getEventName() override { return "Add next line block event"; }
};

class AddPreviousLine_event : public BlockEvent {
public:
    AddPreviousLine_event(Time t, BaseSimulator::BuildingBlock *conBlock) : BlockEvent(t, conBlock) {
        eventType = ADDPREVIOUSLINE_EVENT_ID;
    }
    AddPreviousLine_event(AddPreviousLine_event *conBlock) : BlockEvent(conBlock) {
    }

    void consumeBlockEvent() override {
        concernedBlock->scheduleLocalEvent(EventPtr(new AddPreviousLine_event(this)));
    }

    const string getEventName() override { return "Add previous line block event"; }
};

class AddLeftBlock_event : public BlockEvent {
public:
    AddLeftBlock_event(Time t, BaseSimulator::BuildingBlock *conBlock) : BlockEvent(t, conBlock) {
        eventType = ADDLEFTBLOCK_EVENT_ID;
    }
    AddLeftBlock_event(AddLeftBlock_event *conBlock) : BlockEvent(conBlock) {
    }

    void consumeBlockEvent() override {
        concernedBlock->scheduleLocalEvent(EventPtr(new AddLeftBlock_event(this)));
    }

    const string getEventName() override { return "Add left block event"; }
};

class AddRightBlock_event : public BlockEvent {
public:
    AddRightBlock_event(Time t, BaseSimulator::BuildingBlock *conBlock) : BlockEvent(t, conBlock) {
        eventType = ADDRIGHTBLOCK_EVENT_ID;
    }
    AddRightBlock_event(AddRightBlock_event *conBlock) : BlockEvent(conBlock) {
    }

    void consumeBlockEvent() override {
        concernedBlock->scheduleLocalEvent(EventPtr(new AddRightBlock_event(this)));
    }

    const string getEventName() override { return "Add right block event"; }
};

class CanFillLeft_message : public Message {
public:
    CanFillLeft_message() {
        this->id = CANFILLLEFT_MESSAGE_ID;
    }
};

class CanFillLeftResponse_message : public Message {
public:
    CanFillLeftResponse_message() {
        this->id = CANFILLLEFTRESPONSE_MESSAGE_ID;
    }
};

class CanFillRight_message : public Message {
public:
    CanFillRight_message() {
        this->id = CANFILLRIGHT_MESSAGE_ID;
    }
};

class CanFillRightResponse_message : public Message {
public:
    CanFillRightResponse_message() {
        this->id = CANFILLRIGHTRESPONSE_MESSAGE_ID;
    }
};

#endif /* NEIGHBORHOOD_H_ */
