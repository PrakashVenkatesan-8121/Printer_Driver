#include "Queue.h"
#include "Logger.h"

// Counter for print jobs
static ULONG g_PrintJobCounter = 0;

NTSTATUS
QueueInitialize(
    _In_ WDFDEVICE Device
)
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG queueConfig;

    Logger::GetInstance().LogInfo(L"QueueInitialize called");

    // Configure a default queue for IO requests
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &queueConfig,
        WdfIoQueueDispatchParallel
    );

    queueConfig.EvtIoDeviceControl = EvtIoDeviceControl;
    queueConfig.EvtIoRead = EvtIoRead;
    queueConfig.EvtIoWrite = EvtIoWrite;

    status = WdfIoQueueCreate(
    Device,
   &queueConfig,
     WDF_NO_OBJECT_ATTRIBUTES,
    &queue
    );

    if (!NT_SUCCESS(status))
    {
        Logger::GetInstance().LogError(L"WdfIoQueueCreate failed with status 0x%08X", status);
        return status;
  }

    Logger::GetInstance().LogInfo(L"Queue created successfully");

    return status;
}

VOID
EvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    Logger::GetInstance().LogInfo(L"IOCTL received: 0x%08X", IoControlCode);

    // Complete the request
    WdfRequestComplete(Request, STATUS_SUCCESS);
}

VOID
EvtIoRead(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t Length
)
{
    UNREFERENCED_PARAMETER(Queue);

 Logger::GetInstance().LogInfo(L"Read request received, Length: %Iu bytes", Length);

    // Complete the read request with no data
 WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 0);
}

VOID
EvtIoWrite(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t Length
)
{
    NTSTATUS status;
    PVOID buffer = nullptr;
    size_t bufferLength = 0;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Length);

    // Get the write buffer
    status = WdfRequestRetrieveInputBuffer(
        Request,
        1,  // Minimum required size
        &buffer,
        &bufferLength
    );

    if (NT_SUCCESS(status))
    {
        // Increment job counter
        g_PrintJobCounter++;

        // Log the print job information
        Logger::GetInstance().LogPrintJob(
            L"Print Job",
        g_PrintJobCounter,
        1,  // Pages - would need to parse print data to get actual count
    1   // Copies - would need to parse print data to get actual count
      );

        Logger::GetInstance().LogInfo(
   L"Print data received - Job #%lu, Size: %Iu bytes",
         g_PrintJobCounter,
    bufferLength
        );

        // Optionally, you could save the print data to a file here
        // For now, we just log it and acknowledge receipt

 // Complete the write request successfully
        WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, bufferLength);
    }
    else
    {
 Logger::GetInstance().LogError(
            L"Failed to retrieve input buffer, status: 0x%08X",
            status
        );

        WdfRequestComplete(Request, status);
    }
}
