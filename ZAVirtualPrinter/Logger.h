#pragma once

#include <windows.h>
#include <string>

class Logger
{
public:
    static Logger& GetInstance();
    
    void Initialize(const wchar_t* logFilePath);
    void LogInfo(const wchar_t* format, ...);
    void LogError(const wchar_t* format, ...);
    void LogPrintJob(const wchar_t* jobName, ULONG jobId, ULONG pages, ULONG copies);
    void Cleanup();

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void WriteLog(const wchar_t* level, const wchar_t* message);
    std::wstring GetTimestamp();

    HANDLE m_hLogFile;
    CRITICAL_SECTION m_cs;
    bool m_initialized;
};
