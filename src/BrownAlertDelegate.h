#pragma once
#include "pch.h"
class BrownAlertDelegate : public gd::FLAlertLayer {
protected:
    cocos2d::CCSize m_pLrSize;
    virtual bool init(
        float width,
        float height,
        const char* bg = "GJ_square01.png"
        );
    float width;
    float height;
    virtual void setup() = 0;
    virtual void onClose(cocos2d::CCObject*);
};