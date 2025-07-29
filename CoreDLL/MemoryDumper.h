#pragma once
#include <Windows.h>
#include <vector>
#include <string>

class MemoryDumper {
public:
    static bool DumpLuacFiles(HANDLE hProcess);

private:
    static bool FindLuaSignatures(const BYTE* buf, size_t size, std::vector<DWORD>& offsets);
    static std::string GenerateRandomName(size_t len);
    static void AntiDetectInit();
    static void BypassMemoryScans();
    static BYTE GetLuaSignature(size_t index);
};