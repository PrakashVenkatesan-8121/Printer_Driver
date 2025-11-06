#include "Driver.h"
#include "Device.h"
#include "Logger.h"

// No need to redeclare - already declared in Driver.h
// extern "C" DRIVER_INITIALIZE DriverEntry; // <-- REMOVE THIS LINE

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

    // Initialize WPP Tracing - optional, using our custom logger instead
    // For now, initialize our file logger
    Logger::GetInstance().Initialize(L"C:\\Logs\\VirtualPrinterDriver.log");
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