#include "LuaIntegration.h"

DWORD LuaIntegration::luaStatePtr = 0;

void LuaIntegration::Initialize() {
    // Здесь будет инициализация позже
}

void LuaIntegration::ExecuteLuaScript(const char* script) {
    if (!luaStatePtr || !script) return;

    typedef void(__cdecl* luaL_dostring_t)(void* L, const char* str);
    static luaL_dostring_t luaL_dostring = nullptr;

    if (!luaL_dostring) {
        HMODULE luaDll = GetModuleHandleA("lua51.dll");
        if (luaDll) {
            luaL_dostring = (luaL_dostring_t)GetProcAddress(luaDll, "luaL_dostring");
        }
    }

    if (luaL_dostring) {
        __try {
            __asm {
                push script
                mov ecx, luaStatePtr
                call luaL_dostring
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            // Обработка исключений
        }
    }
}