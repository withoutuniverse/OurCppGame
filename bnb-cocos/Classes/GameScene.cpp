// 理想状况这里应该用继承的... 但是这里先写一个试试看咯～
// TODO: 改变遮挡关系
// 泡泡炸裂
// 制作FSM
#include "GameScene.h"
#include "Bubbles.h"
#include "Settings.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace settings::GameScene;

Scene* GameScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = GameScene::create();
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
    
    visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    addCloseMenu();
    
    // a temporary background
    auto backG = Sprite::create(backGroundPicture);
    addChild(backG, -10);
    backG->setPosition(visibleSize / 2);
    
    /***** tilemap ******/
    _tileMap = TMXTiledMap::create("gameStart/map01.tmx");
    _tileMap->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _tileMap->setPosition(Point(visibleSize.width / 2 , visibleSize.height / 2));
    _tileMap->setScale(settings::GameScene::_tile_delta_rate);
    
    _meta = _tileMap->getLayer("Unbroken");
    _background = _tileMap->getLayer("Background");
    addChild(_tileMap, -1);
    
    // 注意坐标位置差
    offx = (visibleSize.width - _tileMap->getContentSize().width * _tile_delta_rate)/ 2;
    offy = (visibleSize.height - _tileMap->getContentSize().height * _tile_delta_rate) / 2;
    TMXObjectGroup *objects = _tileMap->getObjectGroup("player");
    CCASSERT(nullptr != objects, "'Objects' object group not found");
    auto spawnPoint = objects->getObject("SpawnPoint1");
    CCASSERT(!spawnPoint.empty(), "SpawnPoint object not found");
    float x = spawnPoint["x"].asFloat() * _tile_delta_rate;
    float y = spawnPoint["y"].asFloat() * _tile_delta_rate;
    
    
    /*** add sprite***/
    _myplayer = character::create(character::MAPLE_WISH);
    _myplayer->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _myplayer->setPosition(offx + x, offy + y);
    addChild(_myplayer, 1);
    
    
    
    // basic bubbles
    _my_bubbles = 0;
    
    // 键盘监听
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = CC_CALLBACK_2(GameScene::myKeyboardOnL, this);
    listener->onKeyReleased = CC_CALLBACK_2(GameScene::myKeyboardOffL, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    this->scheduleUpdate();
    
    return true;
}


void GameScene::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
    
    
}

void GameScene::setViewPointCenter(Point position) {
    auto winSize = Director::getInstance()->getWinSize();
    
    int x = MAX(position.x, winSize.width / 2);
    int y = MAX(position.y, winSize.height / 2);
    x = MIN(x, (_tileMap->getMapSize().width * this->_tileMap->getTileSize().width) - winSize.width / 2);
    y = MIN(y, (_tileMap->getMapSize().height * _tileMap->getTileSize().height) - winSize.height / 2);
    auto actualPosition = Point(x, y);
    
    auto centerOfView = Point(winSize.width / 2, winSize.height / 2);
    auto viewPoint = centerOfView - actualPosition;
    this->setPosition(viewPoint);
}

void GameScene::addCloseMenu() {
    // close menu
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(GameScene::menuCloseCallback, this));
    closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));
    auto closeMenu = Menu::create(closeItem, NULL);
    closeMenu->setPosition(Vec2::ZERO);
    this->addChild(closeMenu, 1);
}

/*****键盘监听*****/
void GameScene::myKeyboardOnL(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
    GameScene::_optionCode code = DEFAULT;
    switch (keyCode) {
        case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
            code = _optionCode::GO_UP;
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
            code = _optionCode::GO_DOWN;
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
            code = _optionCode::GO_LEFT;
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
            code = _optionCode::GO_RIGHT;
            break;
        default:
            break;
    }
    // DEBUG
    // TODO:走路可以用状态机实现
    if (code != DEFAULT) {
        for (auto &b : _my_sprite_move)     b = false;
        _my_sprite_move[code] = true;       //有移动的趋势, 防止爆栈
        _direction = code;
        
        // animation and direction
        std::string next_direction(_myplayer->_spriteName + "_"+ std::string(direc_string[code]) +"_");
        auto anime = getAnimationByName(next_direction, 0.1f, _myplayer->_animation_frames);
        auto animate = Animate::create(anime);
        auto player_action = RepeatForever::create(animate);
        _myplayer->runAction(player_action);
    }
}



void GameScene::myKeyboardOffL(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) {
    GameScene::_optionCode key;
    enum T {
        GO_CODE, BUBBLE_CODE
    } code;
    code = GO_CODE;     //默认为go_code
    
    switch (keyCode) {
        case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
            key = _optionCode::GO_UP;
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
            key = _optionCode::GO_DOWN;
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
            key = _optionCode::GO_LEFT;
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
            key = _optionCode::GO_RIGHT;
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
            code = BUBBLE_CODE;
            break;
        default:
            break;
    }
    
    if (code == GO_CODE) {
        _my_sprite_move[key] = false;       //有移动的趋势
        std::string next_direction(_myplayer->_spriteName + "_"+ std::string(direc_string[_direction]) +"_");
        auto tmp_f = SpriteFrameCache::getInstance()->getSpriteFrameByName(next_direction + "01.png");
        _myplayer->setSpriteFrame(tmp_f);
        _myplayer->stopAllActions();
    }
    else if (code == BUBBLE_CODE) {
        setBubble();
    }
}

/****人物移动****/
void GameScene::mySpriteMove() {
    // last move
    static Vec2 last_move(0,0);
    static _optionCode last_ops = DEFAULT;
    
    Vector<FiniteTimeAction *> moves;
    const int basic_step = 2;
    float dur = 0.1f;
    //TODO: 增加边界检测
    // 获得x y 的上界 下界
    float lowerx = offx, upperx = visibleSize.width - offx;
    float lowery = offy, uppery = visibleSize.height - offy;
    
    lowerx += 3, lowery+=3;
    auto in_tile_map = [&](Vec2 pos)->bool{
        if (pos.x >= lowerx && pos.x <= upperx)
            if (pos.y >= lowery && pos.y <= uppery)
                return true;
        return false;
    };
    
    int curstep = _myplayer->_currentVelocity + basic_step;
    
    // UP, DOWN, LEFT, RIGHT
    Vec2 delta_pos[4]{Vec2(0, curstep), Vec2(0, -curstep), Vec2(-curstep, 0), Vec2(curstep, 0)};
    // 判断动作，调整step
    for (int index = 0; index < 4; ++index) {
        // get exact direction and last move
        Vec2 cur_delta = delta_pos[index];
        if (!_my_sprite_move[index])
            continue;
        auto test_point = cur_delta;
        if ( last_ops == index) {
            test_point += last_move;
            last_move += cur_delta;
        } else {
            last_move = cur_delta;
        }
        last_ops = static_cast<GameScene::_optionCode>(index);
        
        
        bool walk(false);           // really move?
        auto deltas = _myplayer->get_collection_point(index);
        auto pos1 = deltas.first, pos2 = deltas.second;
        if (in_tile_map(pos1 + test_point) && in_tile_map(pos2 + test_point)) {
            if (accessAble(pos1 + test_point) && accessAble(pos2 + test_point))
                walk = true;
        }
        if (walk) {
            moves.pushBack(Sequence::create(MoveBy::create(dur, cur_delta), CallFuncN::create(
            [&](Ref* sender) {
                last_move -= cur_delta;
            }), NULL));
        } else {
            last_move = {0,0};
        }
        break;
    }
    // 有可能啥都没有2333
    if (moves.size())
        _myplayer->runAction(cocos2d::Spawn::create(moves));
}

cocos2d::Vec2 GameScene::PositionForTileCoord(cocos2d::Vec2 pos) {
    auto pos0 = tileCoordForPosition(_myplayer->getPosition());
    if (_myplayer->getAnchorPoint() != Vec2::ZERO) {
        if (pos0.x > 14) pos0.x = 14;
    }
    // TODO: find out what was wrong
    if (pos0.y > 14) pos0.y = 14;
    auto mySpritePos = _background->getPositionAt(pos0) * _tile_delta_rate;
    auto visibleSize = Director::getInstance()->getWinSize();
    mySpritePos += (visibleSize - _tileMap->getContentSize() * _tile_delta_rate) / 2;
    return mySpritePos;
}

// bubble应该设置在tilemap的grid上
// bubble渲染问题
void GameScene::setBubble() {
    if (_my_bubbles >= _myplayer->_currentBubbles) {
        return;
    }
    // TODO: duplicate here!!
    auto pos0 = tileCoordForPosition(_myplayer->getPosition());
    if (_myplayer->getAnchorPoint() != Vec2::ZERO) {
        if (pos0.x > 14) pos0.x = 14;
    }
    // TODO: find out what was wrong
    if (pos0.y > 14) pos0.y = 14;
    
    auto mySpritePos = PositionForTileCoord(_myplayer->getPosition());

    // DEBUG : not mySpritePos
    if (accessAble(_myplayer->getPosition())) {
        // 调整精灵位置
        auto newBubble = Bubbles::create(_myplayer->_currentPower);
        newBubble->setAnchorPoint(Vec2::ZERO);
        newBubble->setScale(_tile_delta_rate);
        
        // DEBUG: 判断泡泡放置是否是accessable 的
        newBubble->setPosition(mySpritePos);
        auto timeBoom = CallFuncN::create(CC_CALLBACK_1(GameScene::BubbleBoom, this));
        
        // 动画(是否可以抽象)
        auto anime = getAnimationByName("Popo_", 0.25, bubble_frame_nums);
        
        auto animate = Animate::create(anime);
        auto bubbleAction = RepeatForever::create(animate);
        newBubble->runAction(bubbleAction);
        
        addChild(newBubble);

        _map_screen_bubbles[pos0] = newBubble;
        ++_my_bubbles;
        newBubble->runAction(Sequence::create(DelayTime::create(3), timeBoom, NULL));
    }
}

// 泡泡爆炸, 可以添加逻辑
void GameScene::BubbleBoom(Ref* sender) {
    auto *sprite = dynamic_cast<Bubbles*>(sender);
    auto beg_pos = sprite->getPosition();
    int power = sprite->get_power();
    --_my_bubbles;
    this->removeChild(sprite);
    for (auto iter = _map_screen_bubbles.begin(); iter != _map_screen_bubbles.end(); ++iter) {
        if (iter->second == sprite) {
            _map_screen_bubbles.erase(iter);
            break;
        }
    }
    /***爆炸逻辑***/
    add_and_clear_with_time(Sprite::create(center_boom), boom_time, beg_pos);
    horizontal_boom(beg_pos, power);
    vertival_boom(beg_pos, power);
}

void GameScene::update(float dt) {
    mySpriteMove();
}

cocos2d::Vec2 GameScene::tileCoordForPosition(cocos2d::Vec2 pos) {
    // 玩家位置的y除以地图的高，得到的是地图纵向第几个格子（tile），
    // 但是因为cocos2d-x的y轴（左下角）和TileMap的y轴（左上角）轴相反，所以使用地图的高度减去玩家位置的y
    float pointHeight = _tileMap->getTileSize().height * _tile_delta_rate / CC_CONTENT_SCALE_FACTOR();
    int x = (int)((pos.x - offx) / (_tileMap->getTileSize().width * _tile_delta_rate / CC_CONTENT_SCALE_FACTOR()));
    int y = static_cast<int>((visibleSize.height - offy - pos.y) / pointHeight);
//    if (y > 14) y = 14;
    if (x > 14) x = 14;
    return Vec2(x,y);
}

bool GameScene::accessAble(cocos2d::Vec2 pos) {
    Vec2 tileCoord = tileCoordForPosition(pos);
    // TODO: find out what was wrong
    if (tileCoord.x < 0 || tileCoord.y < 0)
        return false;
    return hasCollisionInGridPos(tileCoord);
}

bool GameScene::hasCollisionInGridPos(Vec2 tileCoord) {
    int tileGid = _meta->getTileGIDAt(tileCoord);
    if (tileGid) {
        auto propertiy = _tileMap->getPropertiesForGID(tileGid);
        if (propertiy.isNull())
            return false;
        auto properties = propertiy.asValueMap();
        if (!properties.empty()) {
            auto collision = properties["Collidable"].asString();
            if ("true" == collision) {
                return false;
            }
        }
    }
    return true;
}

// 通过名字获取动画, 第二个参数为播放属性, 第三个参数为张数目, 从1开始
Animation* GameScene::getAnimationByName(std::string animName,float delay,int animNum){
    auto animation = Animation::create();
    // 循环从精灵帧缓存中获取与图片名称相对应的精灵帧组成动画
    for(unsigned int i = 1;i<animNum;i++){
        // 获取动画图片名称，例如plane0.png
        std::string frameName = animName;
        frameName.append(StringUtils::format("%02d",i)).append(".png");
        // 将单张图片添加为精灵帧（即动画帧）
        animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName.c_str()));
    }
    // 设置动画播放的属性
    animation->setDelayPerUnit(delay);
    // 让精灵对象在动画执行完后恢复到最初状态
    animation->setRestoreOriginalFrame(true);
    // 返回动画对象
    return animation;
}

void GameScene::add_and_clear_with_time(cocos2d::Sprite* sp, float dt, Vec2 pos) {
    sp->setAnchorPoint(Vec2::ZERO);
    sp->setPosition(pos);
//    sp->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    sp->setScale(_tile_delta_rate);
    this->addChild(sp);
    sp->runAction(Sequence::create(DelayTime::create(dt), CallFuncN::create(CC_CALLBACK_1(GameScene::spriteMoveFinished, this)),NULL));
}

void GameScene::spriteMoveFinished(cocos2d::Object* psender) {
    Sprite *sprite = dynamic_cast<Sprite*>(psender);
    this->removeChild(sprite);
}

static inline bool in_map(int x, int y) {
    if (x >= 0 && x < 15)
        if (y >= 0 && y < 15)
            return true;
    return false;
}

void GameScene::boom_animate(cocos2d::Vec2 pos, int power, int r_vec) {
    /*
     args: pos->position of sprite, power:power of bubble, vector:direction
     */
    auto tiled_position = tileCoordForPosition(pos);
    --tiled_position.y;
    static auto std_delta = Vec2(offx, offy);
    
    // dir: 0->horizontal, 1->vertical;
    Vec2 dirs[] = {Vec2(1, 0), Vec2(0, 1)};
    // 正负方向和该方向是否停止
    int syn[] = {-1, 1};
    bool synb[] = {true, true};
    for (int i = 1; i <= power; ++i) {
        for (int j = 0; j < 2; ++j) {
            if (!synb[j])
                continue;
            // 获取下一个爆炸的位置
            auto next_p = dirs[r_vec] * syn[j] + tiled_position;
            // 判断爆炸位置是否在地图中
            if (!in_map(next_p.x, next_p.y)) {
                synb[j] = false;
                continue;
            }
            auto new_blaze = Sprite::create(boom_anime[r_vec]);
            auto mySpritePos = _background->getPositionAt(next_p) * _tile_delta_rate + std_delta;
            // DEBUG
            if (!check_chain_boom(next_p)) {
                // chain booming!!!
                add_and_clear_with_time(new_blaze, boom_time, mySpritePos);
            } else {
                // 有chain爆破
                synb[j] = false;
            }
        }
    }
}

void GameScene::horizontal_boom(cocos2d::Vec2 pos, int power) {
    boom_animate(pos, power, 1);
}

void GameScene::vertival_boom(cocos2d::Vec2 pos, int power) {
    boom_animate(pos, power, 0);
}


bool GameScene::check_chain_boom(cocos2d::Vec2 blaze_pos) {
    auto bubbleIter = _map_screen_bubbles.find(blaze_pos);
    if (bubbleIter != _map_screen_bubbles.end()) {
        auto bubble = bubbleIter->second;
        _map_screen_bubbles.erase(bubbleIter);
        bubble->stopAllActions();
        BubbleBoom(bubble);
        return true;
    }
    return false;
}
