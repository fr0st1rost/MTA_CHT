#pragma once
#include <Windows.h>

#ifdef CORE_EXPORTS
#define CORE_API extern "C" __declspec(dllexport)
#else
#define CORE_API extern "C" __declspec(dllimport)
#endif

CORE_API void WINAPI Core_HelperFunction();
CORE_API void WINAPI StartCheat();