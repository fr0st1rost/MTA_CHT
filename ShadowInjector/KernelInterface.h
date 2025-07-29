#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <TlHelp32.h>
#include "EncryptString.h"

class KernelInterface {
public:
    static bool LoadDriver();
    static void UnloadDriver();
    static bool EnableDebugPrivilege();
    static bool IsAntiCheatActive();
    static DWORD FindTrustedProcess();
    static bool InjectDLL(DWORD pid, const std::string& dllPath);

private:
    static HANDLE hDriver;
};

HANDLE KernelInterface::hDriver = INVALID_HANDLE_VALUE;

bool KernelInterface::LoadDriver() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!scm) return false;

    SC_HANDLE service = CreateServiceA(
        scm, "MTADriver", "MTA Driver",
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        "C:\\drivers\\mta_driver.sys", NULL, NULL, NULL, NULL, NULL
    );

    if (!service) {
        service = OpenServiceA(scm, "MTADriver", SERVICE_ALL_ACCESS);
        if (!service) {
            CloseServiceHandle(scm);
            return false;
        }
    }

    StartService(service, 0, NULL);
    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    hDriver = CreateFileA("\\\\.\\MTADriver", GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL);
    return hDriver != INVALID_HANDLE_VALUE;
}

void KernelInterface::UnloadDriver() {
    if (hDriver != INVALID_HANDLE_VALUE) {
        CloseHandle(hDriver);
        hDriver = INVALID_HANDLE_VALUE;
    }

    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return;

    SC_HANDLE service = OpenServiceA(scm, "MTADriver", SERVICE_STOP | DELETE);
    if (service) {
        SERVICE_STATUS status;
        ControlService(service, SERVICE_CONTROL_STOP, &status);
        DeleteService(service);
        CloseServiceHandle(service);
    }
    CloseServiceHandle(scm);
}

bool KernelInterface::EnableDebugPrivilege() {
    HANDLE token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
        return false;

    TOKEN_PRIVILEGES tp = { 0 };
    LUID luid;

    if (!LookupPrivilegeValueW(NULL, L"SeDebugPrivilege", &luid)) {
        CloseHandle(token);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    bool result = AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
    CloseHandle(token);
    return result && GetLastError() == ERROR_SUCCESS;
}

bool KernelInterface::IsAntiCheatActive() {
    return FindWindowA(NULL, DECRYPT_STR("MTA: San Andreas Anti-Cheat").c_str()) != NULL;
}

DWORD KernelInterface::FindTrustedProcess() {
    const std::vector<std::wstring> targets = {
        L"MTAHelper.exe",
        L"explorer.exe",
        L"chrome.exe"
    };

    PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    DWORD pid = 0;
    if (Process32FirstW(snapshot, &pe)) {
        do {
            for (const auto& target : targets) {
                if (target == pe.szExeFile) {
                    pid = pe.th32ProcessID;
                    break;
                }
            }
        } while (!pid && Process32NextW(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return pid;
}

bool KernelInterface::InjectDLL(DWORD pid, const std::string& dllPath) {
    if (hDriver == INVALID_HANDLE_VALUE)
        return false;

    struct {
        DWORD pid;
        char path[MAX_PATH];
    } data;

    data.pid = pid;
    strcpy_s(data.path, dllPath.c_str());

    DWORD bytesReturned;
    return DeviceIoControl(
        hDriver, 0x80002000,
        &data, sizeof(data),
        NULL, 0,
        &bytesReturned, NULL
    );
}