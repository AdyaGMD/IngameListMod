#pragma once
#include "pch.h"
#include "gd.h"

class LevelInfoLayer : public cocos2d::CCLayer {
public:
    void openLink(cocos2d::CCObject* ret);
    static inline bool(__thiscall* init)(CCLayer* self, gd::GJGameLevel* level);
    static bool __fastcall hook(CCLayer* self, void*, gd::GJGameLevel* level);
    static void updateLabelCB(float dt, CCLabelBMFont* thelabel);
};