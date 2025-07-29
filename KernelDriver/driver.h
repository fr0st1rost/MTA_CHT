#pragma once
#include <ntddk.h>
#include <wdm.h>

#define DRIVER_PREFIX "MTAProtect: "
#define DEVICE_NAME L"\\Device\\MTAProtect"
#define SYMLINK_NAME L"\\DosDevices\\MTAProtect"

// Структуры для обмена данными
typedef struct _KERNEL_REQUEST {
    ULONG ProcessId;
} KERNEL_REQUEST, * PKERNEL_REQUEST;

typedef struct _STRING_ENCRYPT_REQUEST {
    PVOID InputBuffer;
    SIZE_T InputLength;
    PVOID OutputBuffer;
    SIZE_T OutputLength;
} STRING_ENCRYPT_REQUEST, * PSTRING_ENCRYPT_REQUEST;

// IOCTL коды
#define IOCTL_PROTECT_PROCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ENCRYPT_STRING CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Объявление недокументированных структур
typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB {
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    BOOLEAN SpareBool;
    PPEB_LDR_DATA Ldr;
} PEB, * PPEB;

// Объявления функций ядра
NTSYSAPI
NTSTATUS
NTAPI
PsLookupProcessByProcessId(
    _In_ HANDLE ProcessId,
    _Outptr_ PEPROCESS* Process
);

NTSYSAPI
PPEB
NTAPI
PsGetProcessPeb(
    _In_ PEPROCESS Process
);