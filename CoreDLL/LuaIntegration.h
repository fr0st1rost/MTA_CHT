#pragma once
#include <Windows.h>

class LuaIntegration {
public:
    static void Initialize();
    static void ExecuteLuaScript(const char* script);

private:
    static DWORD luaStatePtr;
};