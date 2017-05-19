

#ifndef FSM_hpp
#define FSM_hpp

#include "cocos2d.h"
#include <memory>

class BaseFsm;

class state {
public:
    virtual void excute(cocos2d::Sprite* fsm) = 0;
};

/*** 注意nullptr 的情况  ***/
class BaseFsm: public cocos2d::Node {
public:
    CREATE_FUNC(BaseFsm);
    virtual bool init();
    
    void changeState(std::shared_ptr<state> newstate) {_mCurState = newstate;}
    void update(float dt) override {if(_mCurState) _mCurState->excute(_mySprite);}
private:
    std::shared_ptr<state> _mCurState;
    cocos2d::Sprite* _mySprite;
};


#endif /* FSM_hpp */
