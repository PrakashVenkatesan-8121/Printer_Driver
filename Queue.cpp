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
    UNREFERENCED_PARAMETER(Length);  // Add this line

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