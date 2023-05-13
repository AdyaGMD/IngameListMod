// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "LevelInfoLayer.h"
#include "LeaderboardsLayer.h"

// For debugging
void ShowConsole() {
    AllocConsole();
    SetConsoleTitleA("Mod");
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
}

DWORD WINAPI my_thread(void* hModule) {
    //ShowConsole();
    if (MH_Initialize() != MH_OK) {
        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
    }
    //0x175df0
    MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x175df0), LevelInfoLayer::hook, reinterpret_cast<void**>(&LevelInfoLayer::init));
    MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1587b0), LeaderboardsLayer::hook, reinterpret_cast<void**>(&LeaderboardsLayer::init));
    MH_EnableHook(MH_ALL_HOOKS);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0x1000, my_thread, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

