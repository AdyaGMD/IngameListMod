#pragma once
#include "pch.h"
#include "gd.h"
#include "BrownAlertDelegate.h"

class LeaderboardsLayer : public BrownAlertDelegate {
    public:
        void openButton(CCObject*);
        static inline bool(__thiscall* init)(CCLayer* self, int LeaderboardState);
        static bool __fastcall hook(CCLayer* self, void*, int LeaderboardState);
};