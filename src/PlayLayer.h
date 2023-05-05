#pragma once
#include "pch.h"

class PlayLayer : public gd::PlayLayer {
	public:
        static inline bool(__thiscall* init)(gd::PlayLayer* self, gd::GJGameLevel* level);
        static bool __fastcall hook(gd::PlayLayer* self, int edx, gd::GJGameLevel* level);
};