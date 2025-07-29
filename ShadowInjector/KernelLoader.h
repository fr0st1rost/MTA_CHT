#pragma once
#include <Windows.h>

class KernelLoader {
public:
    static bool LoadDriver(const wchar_t* driverPath, const wchar_t* serviceName);
    static bool UnloadDriver(const wchar_t* serviceName);
};