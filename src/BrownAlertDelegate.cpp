#include "BrownAlertDelegate.h"

bool BrownAlertDelegate::init(float w, float h, const char* sprite) {
    width = w; height = h;
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    this->m_pLrSize = CCSize{ w, h };
    if (!this->initWithColor({ 0, 0, 0, 105 })) return false;
    this->m_pLayer = CCLayer::create();
    this->addChild(this->m_pLayer);
    auto bg = cocos2d::extension::CCScale9Sprite::create(sprite, { .0f, .0f, 80.0f, 80.0f, });
    bg->setContentSize(this->m_pLrSize);
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    this->m_pButtonMenu = CCMenu::create();
    this->m_pLayer->addChild(bg);
    this->m_pLayer->addChild(this->m_pButtonMenu, 2);
    setup();
    CCSprite* button = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    button->setScale(1.0F);
    CCMenuItemSprite* closeButton = gd::CCMenuItemSpriteExtra::create(
        button,
        this,
        (SEL_MenuHandler)&BrownAlertDelegate::onClose
    );
    closeButton->setUserData(reinterpret_cast<void*>(this));
    this->m_pButtonMenu->addChild(closeButton);
    closeButton->setPosition((-w / 2) + 10, (h / 2) - 10);
    setKeypadEnabled(true);
    setTouchEnabled(true);
}

void BrownAlertDelegate::onClose(cocos2d::CCObject* pSender) {
    setKeyboardEnabled(false);
    removeFromParentAndCleanup(true);
};