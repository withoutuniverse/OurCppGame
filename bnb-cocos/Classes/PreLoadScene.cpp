﻿// TODO: 增加一个exit时 清场的function
#include "PreLoadScene.h"
#include "SimpleAudioEngine.h"
#include "Settings.h"
#include "OpenScene.h"

using namespace settings::PreLoadScene;
USING_NS_CC;


Scene* PreLoadScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = PreLoadScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool PreLoadScene::init()
{
	if (!Layer::init())
	{
		return false;
	}
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	setProgress();
	scheduleUpdate();

	return true;
}

void PreLoadScene::setProgress() {
	Size visibleSize = Director::getInstance()->getVisibleSize();
	// 创建一个进度条
	auto barSprite = Sprite::create("GameUI/loadingbar.png");
	// 初始化一个ProgressTimer进度条对象
	_progress = ProgressTimer::create(barSprite);
	// setPercentage:0.0f，表示并未加载任何资源，表现在屏幕上就是什么也看不见
	_progress->setPercentage(0.0f);
	// 由于图片大小关系，把scale设置成0.5，即缩小一半
	_progress->setScale(0.5f);
	// 设置进度条动画的起始位置，默认在图片的中点
	// 如果想要显示从左到右的一个动画效果，必须改成(0,y)
	_progress->setMidpoint(Point(0.0f, 0.5f));
	// barChangeRate表示是否改变水平或者垂直方向的比例，设置成1表示改变，0表示不改变
	_progress->setBarChangeRate(Point(1.0f, 0.f));
	// kCCProgressTimerTypeBar表示为条形进度条
	_progress->setType(ProgressTimer::Type::BAR);
	// 设置进度条位置在屏幕中心
	_progress->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	// 将进度条添加为当前层的子节点
	this->addChild(_progress);
}

void PreLoadScene::loadSpriteFrame() {
	using namespace settings::PreLoadScene;
	// 道具动画
	auto sprite_frame_cache = SpriteFrameCache::getInstance();
	for (auto s : gifts_animation) {
		sprite_frame_cache->addSpriteFramesWithFile(s);
	}
	for (auto s : items_animation) {
		sprite_frame_cache->addSpriteFramesWithFile(s);
	}
	for (auto s : person_animation) {
		sprite_frame_cache->addSpriteFramesWithFile(s);
	}
	sprite_frame_cache->addSpriteFramesWithFile(bubbles_animation);
}

void PreLoadScene::loadMusic() {
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic(to_load_music);
}

void PreLoadScene::progressUpdate() {
	// 每次调用该函数说明加载一个资源，自减更新资源总数
	if (--_sourceCount) {
		// 设置进度条百分比
		_progress->setPercentage(100.0f - (_progressInterval * _sourceCount));
	}
	else {
		// ProgressFromTo动作以渐进的方式显示图片
		// 第一个参数表示持续0.5秒，第二个参数表示进度条百分比开始，第三个参数表示一直到100
		auto pft = ProgressFromTo::create(0.5f, _progress->getPercentage(), 100);
		// 定义了一个CallFunc动作
		auto callFunc = CallFunc::create([=] {
			// 当加载资源完成，延迟2秒跳转到游戏主场景
			auto delay = DelayTime::create(delaytime);
			auto callFunc = CallFunc::create([] {
				Director::getInstance()->replaceScene(OpenScene::createScene());
			});
			auto action = Sequence::create(delay, callFunc, NULL);
			this->runAction(action);
		});
		auto action = Sequence::create(pft, callFunc, NULL);
		// 进度条执行动作
		_progress->runAction(action);
	}
}

void PreLoadScene::update(float dt) {
	progressUpdate();
}

void PreLoadScene::onEnterTransitionDidFinish() {
	Layer::onEnterTransitionDidFinish();
	_sourceCount = delayFrame;
	// 设置进度条更新次数=100/需要加载的资源数量
	_progressInterval = 100 / static_cast<float>(_sourceCount);
	// 依次加载资源
	loadMusic();
	loadSpriteFrame();
}

