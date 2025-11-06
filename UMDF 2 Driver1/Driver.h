#pragma once

#include <windows.h>
#include <wdf.h>
#include <wudfwdm.h>

// Forward declarations for driver callbacks
EVT_WDF_DRIVER_DEVICE_ADD EvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP EvtDriverContextCleanup;

// Driver entry point forward declaration
EXTERN_C_START
DRIVER_INITIALIZE DriverEntry;
EXTERN_C_END
