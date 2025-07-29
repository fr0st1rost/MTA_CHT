#include "pch.h"
#include "MemoryDumper.h"
#include "Stealth.h"
#include "StringSecurity.h"
#include <Psapi.h>
#include <fstream>
#include <limits>

BYTE MemoryDumper::GetLuaSignature(size_t index) {
    static const BYTE encrypted[4] = { 0x7A, 0x1F, 0x3D, 0x44 };
    static const BYTE key = 0x61;
    return encrypted[index] ^ key;
}

void MemoryDumper::AntiDetectInit() {
    for (int i = 0; i < 5; i++) {
        GetTickCount64();
        GetProcessHeap();
        IsDebuggerPresent();
    }
    Stealth::RandomSleep(50, 150);
}

void MemoryDumper::BypassMemoryScans() {
    BYTE* base = (BYTE*)GetModuleHandle(NULL);
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return;

    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return;

    DWORD oldProtect;
    if (VirtualProtect(base, nt->OptionalHeader.SizeOfHeaders, PAGE_READWRITE, &oldProtect)) {
        for (DWORD i = 0; i < nt->OptionalHeader.SizeOfHeaders; i++) {
            base[i] = (base[i] ^ 0xAA) + 0x55;
        }
        VirtualProtect(base, nt->OptionalHeader.SizeOfHeaders, oldProtect, &oldProtect);
    }
}

std::string MemoryDumper::GenerateRandomName(size_t len) {
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    for (size_t i = 0; i < len; ++i) {
        result += charset[rand() % (sizeof(charset) - 1)];
    }
    return result;
}

bool MemoryDumper::FindLuaSignatures(const BYTE* buf, size_t size, std::vector<DWORD>& offsets) {
    if (size < 4) return false;

    BYTE signature[4];
    for (int i = 0; i < 4; i++) {
        signature[i] = GetLuaSignature(i);
    }

    for (size_t i = 0; i < size - 4; ++i) {
        if (memcmp(&buf[i], signature, 4) == 0) {
            offsets.push_back(static_cast<DWORD>(i));
            i += 3; // Skip signature bytes
        }
    }
    return !offsets.empty();
}

bool MemoryDumper::DumpLuacFiles(HANDLE hProcess) {
    SYSTEM_INFO si = {};
    GetSystemInfo(&si);

    AntiDetectInit();
    BypassMemoryScans();

    char processName[MAX_PATH];
    GetModuleBaseNameA(hProcess, nullptr, processName, MAX_PATH);

    if (strcmp(processName, "gta_sa.exe") != 0 &&
        strcmp(processName, "CCDPlanet.exe") != 0) {
        return false;
    }

    // Расшифровка директории дампов
    std::string dumpDir = DECRYPT_STR("C:\\MTADumps\\");
    CreateDirectoryA(dumpDir.c_str(), NULL);

    BYTE* p = (BYTE*)si.lpMinimumApplicationAddress;
    MEMORY_BASIC_INFORMATION mbi = {};
    int count = 0;

    while (p < si.lpMaximumApplicationAddress) {
        if (VirtualQueryEx(hProcess, p, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS) {
                const SIZE_T maxSafeSize = 100 * 1024 * 1024; // 100MB
                const SIZE_T regionSize = static_cast<SIZE_T>(mbi.RegionSize);

                if (regionSize > 0 && regionSize <= maxSafeSize) {
                    std::vector<BYTE> buffer(regionSize);
                    SIZE_T bytesRead = 0;

                    if (ReadProcessMemory(hProcess, p, buffer.data(), regionSize, &bytesRead) && bytesRead >= 4) {
                        std::vector<DWORD> offsets;
                        if (FindLuaSignatures(buffer.data(), bytesRead, offsets)) {
                            for (DWORD off : offsets) {
                                if (off < bytesRead) {
                                    std::string filename = dumpDir + GenerateRandomName(12) + ".luac";
                                    std::ofstream file(filename, std::ios::binary);
                                    if (file.is_open()) {
                                        file.write(reinterpret_cast<const char*>(buffer.data() + off), bytesRead - off);
                                        file.close();
                                        count++;
                                        Stealth::RandomSleep(100, 500);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Переход к следующему региону
            p += mbi.RegionSize;
        }
        else {
            // Если VirtualQueryEx не сработал — перейти на следующую страницу
            p += si.dwPageSize;
        }
    }

    return count > 0;
}
