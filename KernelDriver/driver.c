#include "driver.h"

// Runtime-шифрование строк
VOID EncryptDecryptString(PUCHAR Data, SIZE_T Length) {
    static const UCHAR key[] = { 0xAB, 0xCD, 0xEF, 0x12, 0x34 };
    for (SIZE_T i = 0; i < Length; i++) {
        Data[i] ^= key[i % sizeof(key)];
    }
}

// Прототипы функций
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
VOID DriverUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS CreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS ProtectProcess(ULONG ProcessId);

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status;
    UNICODE_STRING devName, symLink;
    PDEVICE_OBJECT DeviceObject = NULL;

    // Инициализация имен
    RtlInitUnicodeString(&devName, DEVICE_NAME);
    RtlInitUnicodeString(&symLink, SYMLINK_NAME);

    // Создание устройства
    status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
    if (!NT_SUCCESS(status)) {
        KdPrint((DRIVER_PREFIX "Failed to create device (0x%08X)\n", status));
        return status;
    }

    // Создание символической ссылки
    status = IoCreateSymbolicLink(&symLink, &devName);
    if (!NT_SUCCESS(status)) {
        KdPrint((DRIVER_PREFIX "Failed to create symbolic link (0x%08X)\n", status));
        IoDeleteDevice(DeviceObject);
        return status;
    }

    // Настройка обработчиков
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;

    KdPrint((DRIVER_PREFIX "Driver loaded successfully\n"));
    return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symLink;
    RtlInitUnicodeString(&symLink, SYMLINK_NAME);
    IoDeleteSymbolicLink(&symLink);
    IoDeleteDevice(DriverObject->DeviceObject);
    KdPrint((DRIVER_PREFIX "Driver unloaded\n"));
}

NTSTATUS CreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG_PTR info = 0;

    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_PROTECT_PROCESS: {
        PKERNEL_REQUEST request = (PKERNEL_REQUEST)Irp->AssociatedIrp.SystemBuffer;
        if (request) {
            status = ProtectProcess(request->ProcessId);
        }
        break;
    }
    case IOCTL_ENCRYPT_STRING: {
        PSTRING_ENCRYPT_REQUEST request = (PSTRING_ENCRYPT_REQUEST)Irp->AssociatedIrp.SystemBuffer;
        if (request && request->InputBuffer && request->OutputBuffer) {
            RtlCopyMemory(request->OutputBuffer, request->InputBuffer, request->InputLength);
            EncryptDecryptString((PUCHAR)request->OutputBuffer, request->InputLength);
            info = request->InputLength;
            status = STATUS_SUCCESS;
        }
        break;
    }
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = info;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS ProtectProcess(ULONG ProcessId) {
    PEPROCESS Process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)ProcessId, &Process);
    if (!NT_SUCCESS(status)) {
        KdPrint((DRIVER_PREFIX "Failed to find process %lu (0x%08X)\n", ProcessId, status));
        return status;
    }

    // 1. Сбрасываем флаг отладки
    PPEB peb = PsGetProcessPeb(Process);
    if (peb) {
        peb->BeingDebugged = 0;
        KdPrint((DRIVER_PREFIX "Reset debug flag for process %lu\n", ProcessId));
    }

    // 2. Скрываем CoreDLL.dll
    if (peb && peb->Ldr) {
        PLIST_ENTRY head = &peb->Ldr->InLoadOrderModuleList;
        PLIST_ENTRY entry = head->Flink;
        int safeCounter = 0;
        const int maxIterations = 1000;

        while (entry != head && safeCounter++ < maxIterations) {
            PLDR_DATA_TABLE_ENTRY module = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

            if (module->FullDllName.Buffer) {
                // Проверяем, содержит ли путь CoreDLL.dll
                UNICODE_STRING coreDllName;
                RtlInitUnicodeString(&coreDllName, L"CoreDLL.dll");

                if (wcsstr(module->FullDllName.Buffer, coreDllName.Buffer)) {
                    // Удаляем модуль из всех списков
                    RemoveEntryList(&module->InLoadOrderLinks);
                    RemoveEntryList(&module->InMemoryOrderLinks);
                    RemoveEntryList(&module->InInitializationOrderLinks);
                    KdPrint((DRIVER_PREFIX "Hid CoreDLL.dll in process %lu\n", ProcessId));
                    break;
                }
            }

            entry = entry->Flink;
        }
    }

    if (Process) ObDereferenceObject(Process);
    return STATUS_SUCCESS;
}