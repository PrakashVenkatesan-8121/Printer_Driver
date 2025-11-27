#include "Driver.h"
#include "Device.h"
#include "Logger.h"

// DriverEntry is already declared in Driver.h, just implement it here
extern "C"
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;

    // Initialize file logger with dynamically constructed path
    WCHAR logPath[MAX_PATH];
    DWORD result = GetEnvironmentVariableW(L"SystemDrive", logPath, MAX_PATH);
    if (result > 0 && result < MAX_PATH) {
        wcscat_s(logPath, L"\\Logs");
    // Create directory if it doesn't exist
        CreateDirectoryW(logPath, NULL);
  wcscat_s(logPath, L"\\VirtualPrinterDriver.log");
    } else {
        // Fallback to C: drive if environment variable fails
        wcscpy_s(logPath, L"C:\\Logs");
        CreateDirectoryW(logPath, NULL);
     wcscat_s(logPath, L"\\VirtualPrinterDriver.log");
    }
    
    Logger::GetInstance().Initialize(logPath);
    Logger::GetInstance().LogInfo(L"DriverEntry called");

    // Initialize driver config
    WDF_DRIVER_CONFIG_INIT(&config, EvtDeviceAdd);
    config.DriverPoolTag = 'TPRV'; // 'VRPT' - Virtual Printer Tag

    // Register cleanup callback
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = EvtDriverContextCleanup;

    status = WdfDriverCreate(
     DriverObject,
        RegistryPath,
   &attributes,
      &config,
 WDF_NO_HANDLE
    );

  if (!NT_SUCCESS(status))
    {
        Logger::GetInstance().LogError(L"WdfDriverCreate failed with status 0x%08X", status);
        Logger::GetInstance().Cleanup();
        return status;
    }

    Logger::GetInstance().LogInfo(L"Driver created successfully");
    Logger::GetInstance().LogInfo(L"Log path: %s", logPath);

    return status;
}

NTSTATUS
EvtDeviceAdd(
    _In_    WDFDRIVER Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(Driver);

    Logger::GetInstance().LogInfo(L"EvtDeviceAdd called");

    status = CreateDevice(DeviceInit);

    return status;
}

VOID
EvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);

    Logger::GetInstance().LogInfo(L"Driver cleanup called");
    Logger::GetInstance().Cleanup();
}
