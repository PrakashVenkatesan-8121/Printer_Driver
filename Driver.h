#pragma once

#include <windows.h>
#include <wdf.h>
#include <wudfwdm.h>

// Driver callbacks
EVT_WDF_DRIVER_DEVICE_ADD EvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP EvtDriverContextCleanup;

// Driver entry point - declared only once with C linkage
extern "C" DRIVER_INITIALIZE DriverEntry;