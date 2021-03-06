#include "BubbleController.h"
#include "Character.h"
#include "GameScene.h"
#include "CharacterFSM.h"

USING_NS_CC;

inline character* getMyplayer() {
    auto scene = GameScene::getCurrentMap();
    return dynamic_cast<character*>(scene->getChildByName("myplayer"));
}

void BubbleController::myKeyboardOff(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
    if (keyCode != EventKeyboard::KeyCode::KEY_SPACE)
        return;
    auto gameLayer = GameScene::getCurrentMap();
    auto mychara = dynamic_cast<character*>(gameLayer->getChildByName("myplayer"));
    if (checkStateWalkAble(mychara))
        gameLayer->setBubble(mychara);
}
