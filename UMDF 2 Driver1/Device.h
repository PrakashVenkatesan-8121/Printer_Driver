#pragma once

#include <windows.h>
#include <wdf.h>

EXTERN_C_START

// Device callbacks
EVT_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
EVT_WDF_DEVICE_D0_ENTRY EvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT EvtDeviceD0Exit;

// Function to create device
NTSTATUS CreateDevice(_Inout_ PWDFDEVICE_INIT DeviceInit);

EXTERN_C_END
