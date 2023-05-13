#include "pch.h"
#include "LeaderboardsLayer.h"
#include "DLStatsViewer.h"

void LeaderboardsLayer::openButton(CCObject*) {
	DLStatsViewer::create()->show();
}

bool __fastcall LeaderboardsLayer::hook(CCLayer* self, void*, int LeaderboardState) {
	bool result = init(self, LeaderboardState);

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	CCSprite* buttonbg = CCSprite::createWithSpriteFrameName("GJ_rotationControlBtn01_001.png");
	CCSprite* demonIcon = CCSprite::createWithSpriteFrameName("diffIcon_06_btn_001.png");

	auto button = gd::CCMenuItemSpriteExtra::create(buttonbg, self, menu_selector(LeaderboardsLayer::openButton));
	auto menu = CCMenu::create();

	menu->setPosition({ size.width / 2 + 35, size.height / 2  + 35});
	menu->setZOrder(2);
	menu->setScale(2);

	menu->addChild(button);
	buttonbg->addChild(demonIcon);

	self->addChild(menu);

	demonIcon->setScale(0.63f);
	demonIcon->setPosition({ demonIcon->getPositionX() + 14.6f , demonIcon->getPositionY() + 15});

	return result;
}