#include "KernelComm.h"
#include <winioctl.h>

HANDLE KernelComm::hDriver = INVALID_HANDLE_VALUE;

bool KernelComm::Initialize() {
    hDriver = CreateFileW(
        L"\\\\.\\MTAProtect",
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    return hDriver != INVALID_HANDLE_VALUE;
}

bool KernelComm::ProtectCurrentProcess() {
    if (hDriver == INVALID_HANDLE_VALUE) return false;

    DWORD processId = GetCurrentProcessId();
    DWORD bytesReturned;
    return DeviceIoControl(
        hDriver,
        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS),
        &processId,
        sizeof(processId),
        nullptr,
        0,
        &bytesReturned,
        nullptr
    );
}

bool KernelComm::EncryptString(const char* input, char* output, size_t length) {
    if (hDriver == INVALID_HANDLE_VALUE || !input || !output) return false;

    struct {
        PVOID InputBuffer;
        SIZE_T InputLength;
        PVOID OutputBuffer;
        SIZE_T OutputLength;
    } request;

    request.InputBuffer = (PVOID)input;
    request.InputLength = length;
    request.OutputBuffer = output;
    request.OutputLength = length;

    DWORD bytesReturned;
    return DeviceIoControl(
        hDriver,
        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS),
        &request,
        sizeof(request),
        nullptr,
        0,
        &bytesReturned,
        nullptr
    );
}
bool KernelComm::EncryptDecryptString(const char* input, char* output, size_t length) {
    if (hDriver == INVALID_HANDLE_VALUE || !input || !output) return false;

    struct {
        PVOID InputBuffer;
        SIZE_T InputLength;
        PVOID OutputBuffer;
        SIZE_T OutputLength;
    } request;

    request.InputBuffer = (PVOID)input;
    request.InputLength = length;
    request.OutputBuffer = output;
    request.OutputLength = length;

    DWORD bytesReturned;
    return DeviceIoControl(
        hDriver,
        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS),
        &request,
        sizeof(request),
        nullptr,
        0,
        &bytesReturned,
        nullptr
    );
}