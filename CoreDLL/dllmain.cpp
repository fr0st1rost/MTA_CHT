#include "pch.h"
#include "core.h"
#include "SecurityChecks.h"
#include "Stealth.h"
#include "KernelComm.h"
#include "CheatGUI.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {

    if (reason == DLL_PROCESS_ATTACH) {
        // Проверка целевого процесса
        if (!Security::IsValidTargetProcess()) return FALSE;

        DisableThreadLibraryCalls(hModule);
        Stealth::HideModule(hModule);
        Stealth::HideThreadFromDebugger();
        Stealth::DisableETW();

        // Инициализация драйвера
        KernelComm::Initialize();
        KernelComm::ProtectCurrentProcess();

        // Инициализация GUI
        if (reason == DLL_PROCESS_ATTACH) {

            CheatGUI::Initialize(hModule);

            HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartCheat, NULL, 0, NULL);
            if (hThread) CloseHandle(hThread);
        }
        return TRUE;

    }
}