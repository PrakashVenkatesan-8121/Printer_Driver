#pragma once

#include <windows.h>
#include <wdf.h>

EXTERN_C_START

// Queue callbacks
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL EvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_READ EvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE EvtIoWrite;

// Function to initialize queue
NTSTATUS QueueInitialize(_In_ WDFDEVICE Device);

EXTERN_C_END
