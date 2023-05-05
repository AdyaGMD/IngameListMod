#include "PlayLayer.h"

// Nothing special here
bool __fastcall PlayLayer::hook(gd::PlayLayer* self, int, gd::GJGameLevel* level) {
    bool result = init(self, level);
    auto director = cocos2d::CCDirector::sharedDirector();
    auto size = director->getWinSize();

    return result;
}