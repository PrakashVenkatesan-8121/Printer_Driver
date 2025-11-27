#include "Logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

Logger::Logger() : m_hLogFile(INVALID_HANDLE_VALUE), m_initialized(false)
{
    InitializeCriticalSection(&m_cs);
}

Logger::~Logger()
{
    Cleanup();
    DeleteCriticalSection(&m_cs);
}

Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}

void Logger::Initialize(const wchar_t* logFilePath)
{
    EnterCriticalSection(&m_cs);

    if (!m_initialized)
    {
        // Extract directory path and create if needed
        std::wstring path(logFilePath);
        size_t lastSlash = path.find_last_of(L"\\/ ");
        if (lastSlash != std::wstring::npos)
        {
            std::wstring directory = path.substr(0, lastSlash);
            CreateDirectoryW(directory.c_str(), nullptr);
        }

        m_hLogFile = CreateFileW(
      logFilePath,
     GENERIC_WRITE,
      FILE_SHARE_READ,
            nullptr,
     OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
 nullptr
        );

        if (m_hLogFile != INVALID_HANDLE_VALUE)
        {
            // Move to end of file for append
        SetFilePointer(m_hLogFile, 0, nullptr, FILE_END);
 m_initialized = true;
            
  LogInfo(L"Virtual Printer Driver initialized");
            LogInfo(L"Log file: %s", logFilePath);
        }
        else
        {
 // If file creation fails, try a fallback location
            WCHAR fallbackPath[MAX_PATH];
            GetTempPathW(MAX_PATH, fallbackPath);
  wcscat_s(fallbackPath, L"VirtualPrinterDriver.log");
        
            m_hLogFile = CreateFileW(
      fallbackPath,
   GENERIC_WRITE,
       FILE_SHARE_READ,
         nullptr,
                OPEN_ALWAYS,
  FILE_ATTRIBUTE_NORMAL,
       nullptr
          );
            
            if (m_hLogFile != INVALID_HANDLE_VALUE)
            {
        SetFilePointer(m_hLogFile, 0, nullptr, FILE_END);
    m_initialized = true;
        LogInfo(L"Virtual Printer Driver initialized (fallback location)");
                LogInfo(L"Log file: %s", fallbackPath);
  }
        }
    }

    LeaveCriticalSection(&m_cs);
}

void Logger::LogInfo(const wchar_t* format, ...)
{
    if (!m_initialized) return;

    va_list args;
    va_start(args, format);

  wchar_t buffer[1024];
    _vsnwprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);

    va_end(args);

  WriteLog(L"INFO", buffer);
}

void Logger::LogError(const wchar_t* format, ...)
{
    if (!m_initialized) return;

    va_list args;
    va_start(args, format);

    wchar_t buffer[1024];
    _vsnwprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);

    va_end(args);

    WriteLog(L"ERROR", buffer);
}

void Logger::LogPrintJob(const wchar_t* jobName, ULONG jobId, ULONG pages, ULONG copies)
{
    if (!m_initialized) return;

    wchar_t buffer[1024];
    _snwprintf_s(buffer, _countof(buffer), _TRUNCATE,
        L"Print Job Received - ID: %lu, Name: %s, Pages: %lu, Copies: %lu",
        jobId, jobName ? jobName : L"(unnamed)", pages, copies);

    WriteLog(L"PRINT_JOB", buffer);
}

void Logger::Cleanup()
{
    EnterCriticalSection(&m_cs);

    if (m_initialized && m_hLogFile != INVALID_HANDLE_VALUE)
    {
        LogInfo(L"Virtual Printer Driver shutting down");
     CloseHandle(m_hLogFile);
  m_hLogFile = INVALID_HANDLE_VALUE;
        m_initialized = false;
    }

    LeaveCriticalSection(&m_cs);
}

void Logger::WriteLog(const wchar_t* level, const wchar_t* message)
{
    EnterCriticalSection(&m_cs);

    if (m_hLogFile != INVALID_HANDLE_VALUE)
    {
        std::wstring timestamp = GetTimestamp();
        std::wstring logLine = timestamp + L" [" + level + L"] " + message + L"\r\n";

        // Convert to UTF-8 for file writing
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, logLine.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (sizeNeeded > 0)
        {
         char* utf8Buffer = new char[sizeNeeded];
    WideCharToMultiByte(CP_UTF8, 0, logLine.c_str(), -1, utf8Buffer, sizeNeeded, nullptr, nullptr);

            DWORD bytesWritten;
      WriteFile(m_hLogFile, utf8Buffer, sizeNeeded - 1, &bytesWritten, nullptr);
    FlushFileBuffers(m_hLogFile);

      delete[] utf8Buffer;
        }
    }

    LeaveCriticalSection(&m_cs);
}

std::wstring Logger::GetTimestamp()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    wchar_t buffer[64];
    _snwprintf_s(buffer, _countof(buffer), _TRUNCATE,
        L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    return std::wstring(buffer);
}
