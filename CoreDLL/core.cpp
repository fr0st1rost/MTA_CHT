#include "pch.h"
#include "core.h"
#include "CheatGUI.h"
#include "MemoryDumper.h"
#include "Stealth.h"
#include "KernelComm.h"

void WINAPI Core_HelperFunction() {
    // Пустая или минимальная реализация
}

void WINAPI StartCheat() {
    if (!KernelComm::Initialize()) return;
    KernelComm::ProtectCurrentProcess();
    Stealth::RandomSleep(3000, 8000);

    while (!GetModuleHandleA("multiplayer_sa.dll") &&
        !GetModuleHandleA("CCDPlanet.exe")) {
        Sleep(1000);
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    if (!hProcess) return;

    while (true) {
        if (GetAsyncKeyState(VK_F1) & 0x8000) {
            CheatGUI::Toggle();
            Stealth::RandomSleep(200, 500);
        }

        CheatGUI::Render();

        if (!Stealth::IsDebuggerPresentAdvanced() &&
            !Stealth::IsSandbox() &&
            !Stealth::IsAntiCheatActive()) {
            MemoryDumper::DumpLuacFiles(hProcess);
        }

        Stealth::RandomSleep(1000, 5000);
    }
    CloseHandle(hProcess);
}