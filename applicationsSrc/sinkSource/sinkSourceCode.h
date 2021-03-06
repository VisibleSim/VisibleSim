#ifndef sinkSourceCode_H_
#define sinkSourceCode_H_
#include "robots/smartBlocks/smartBlocksBlockCode.h"

static const int BROADCAST_MSG=1001;
static const int DISTANCE_MSG=1002;
static const int PROPOSE_MSG=1003;
static const int ANSWER_MSG=1004;
static const int MOVE_MSG=1005;

using namespace SmartBlocks;

class SinkSourceCode : public SmartBlocksBlockCode {
private:
    int distance;
    int value;
    bool wait;
    P2PNetworkInterface *proposePath;
    P2PNetworkInterface *answerPath;
    P2PNetworkInterface *movePath;
public :
    SinkSourceCode(SmartBlocksBlock *host):SmartBlocksBlockCode(host) {};
    ~SinkSourceCode() {};

    void startup() override;
    void myBroadcastFunc(const std::shared_ptr<Message> msg,P2PNetworkInterface *sender);
    void myDistanceFunc(const std::shared_ptr<Message> msg,P2PNetworkInterface *sender);
    void myProposeFunc(const std::shared_ptr<Message> msg,P2PNetworkInterface *sender);
    void myAnswerFunc(const std::shared_ptr<Message> msg,P2PNetworkInterface *sender);
    void myMoveFunc(const std::shared_ptr<Message> msg,P2PNetworkInterface *sender);

/*****************************************************************************/
/** needed to associate code to module                                      **/
    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new SinkSourceCode((SmartBlocksBlock*)host));
    };
/*****************************************************************************/
};
#endif /* sinkSourceCode_H_ */
