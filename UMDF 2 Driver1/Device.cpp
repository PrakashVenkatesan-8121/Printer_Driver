#include "Device.h"
#include "Queue.h"
#include "Logger.h"

NTSTATUS
CreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDFDEVICE device;
    NTSTATUS status;

    Logger::GetInstance().LogInfo(L"CreateDevice called");

    // Initialize PnP power callbacks
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
 pnpPowerCallbacks.EvtDevicePrepareHardware = EvtDevicePrepareHardware;
    pnpPowerCallbacks.EvtDeviceD0Entry = EvtDeviceD0Entry;
    pnpPowerCallbacks.EvtDeviceD0Exit = EvtDeviceD0Exit;
  WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    // Initialize device attributes
    WDF_OBJECT_ATTRIBUTES_INIT(&deviceAttributes);

    // Create the device
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (!NT_SUCCESS(status))
    {
   Logger::GetInstance().LogError(L"WdfDeviceCreate failed with status 0x%08X", status);
        return status;
    }

    Logger::GetInstance().LogInfo(L"Device created successfully");

    // Create the device queue
    status = QueueInitialize(device);

    if (!NT_SUCCESS(status))
    {
 Logger::GetInstance().LogError(L"QueueInitialize failed with status 0x%08X", status);
        return status;
    }

    return status;
}

NTSTATUS
EvtDevicePrepareHardware(
    _In_ WDFDEVICE Device,
    _In_ WDFCMRESLIST ResourcesRaw,
    _In_ WDFCMRESLIST ResourcesTranslated
)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(ResourcesRaw);
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    Logger::GetInstance().LogInfo(L"EvtDevicePrepareHardware called");

    return STATUS_SUCCESS;
}

NTSTATUS
EvtDeviceD0Entry(
    _In_ WDFDEVICE Device,
    _In_ WDF_POWER_DEVICE_STATE PreviousState
)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(PreviousState);

    Logger::GetInstance().LogInfo(L"Device entering D0 (working) state");

    return STATUS_SUCCESS;
}

NTSTATUS
EvtDeviceD0Exit(
    _In_ WDFDEVICE Device,
    _In_ WDF_POWER_DEVICE_STATE TargetState
)
{
UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(TargetState);

    Logger::GetInstance().LogInfo(L"Device exiting D0 state");

    return STATUS_SUCCESS;
}
