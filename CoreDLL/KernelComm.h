#pragma once
#include <Windows.h>

class KernelComm {
public:
    static bool Initialize();
    static bool ProtectCurrentProcess();
    static bool EncryptString(const char* input, char* output, size_t length);

    static bool EncryptDecryptString(const char* input, char* output, size_t length);

private:
    static HANDLE hDriver;
};