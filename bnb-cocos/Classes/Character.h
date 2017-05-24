#ifndef Character_hpp
#define Character_hpp

#include "cocos2d.h"
#include <utility>
#include "Settings.h"
#include <array>

class character: public cocos2d::Sprite {
public:
    // 显示怪物类型的枚举变量
    // 建立不同的 enemy type
    enum characterType {
        MAPLE_WISH,
        CHRIS,
        SHADOWFOX
    };
    
    enum characterState {
        GO,
        STUCKED,
        DIE
    } chara_state;
    
    bool initWithPlayerType(characterType type);
    
    static character* create(characterType type);

    /*************人物属性，基本属性可以尽量写在配置类里面***************/
    int _maxVelocity;
    int _currentVelocity;
    
    int _maxPower;
    int _currentPower;
    
    int _maxBubbles;
    int _currentBubbles;
    
    int _animation_frames;
    // 每个精灵对应的_spriteName
    std::string _spriteName;
    
    // 之前的运动状态
    cocos2d::Vec2 last_move;
    settings::directions last_ops;
    std::array<bool, 4> _chara_move;
    /*****碰撞点******/
    // 根据方向给出一对碰撞点
    std::pair<cocos2d::Vec2, cocos2d::Vec2> get_collection_point(int direction);
    // 获取人物移动向量
    cocos2d::Vec2 getMoveVector();
    cocos2d::Vec2 getMoveVector(int index);
    
    /*** animations ***/
    void playStuckedAnimation();
    void playDieAnimation();
    void playAliveAnimation();
};




#endif /* Character_hpp */
