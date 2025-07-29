#include "KernelLoader.h"
#include <windows.h>

bool KernelLoader::LoadDriver(const wchar_t* driverPath, const wchar_t* serviceName) {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE); 
    if (!scm) {
        return false;
    }

    SC_HANDLE service = CreateServiceW( 
        scm,
        serviceName,
        serviceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        driverPath,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );

    if (!service) {
        if (GetLastError() == ERROR_SERVICE_EXISTS) {
            service = OpenServiceW(scm, serviceName, SERVICE_ALL_ACCESS); 
        }
        else {
            CloseServiceHandle(scm);
            return false;
        }
    }

    bool success = StartServiceW(service, 0, nullptr); 
    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return success;
}

bool KernelLoader::UnloadDriver(const wchar_t* serviceName) {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT); 
    if (!scm) {
        return false;
    }

    SC_HANDLE service = OpenServiceW(scm, serviceName, SERVICE_STOP | DELETE); 
    if (!service) {
        CloseServiceHandle(scm);
        return false;
    }

    SERVICE_STATUS status;
    ControlService(service, SERVICE_CONTROL_STOP, &status);

    bool success = DeleteService(service) != 0;
    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return success;
}