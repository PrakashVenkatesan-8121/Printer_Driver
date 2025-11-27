#pragma once

#include <windows.h>
#include <string>

class DriverInstaller
{
public:
    DriverInstaller();
    ~DriverInstaller();

    // Main operations
    bool CleanupExistingDriver();
    bool InstallDriver(const std::wstring& infPath);
    bool VerifyInstallation();
    
    // Utility functions
    bool IsAdministrator();
    std::wstring GetLastErrorString();
  void PrintStatus(const std::wstring& message, bool isError = false);

private:
    // Internal helper functions
    bool RemoveDriverPackages();
    bool UninstallDriverFromStore(const std::wstring& infName);
    bool AddDriverToStore(const std::wstring& infPath);
    bool EnumerateDriverPackages();
  
    // Configuration
    static constexpr const wchar_t* DEVICE_HARDWARE_ID = L"Root\\ZAVirtualPrinter";
    static constexpr const wchar_t* DRIVER_NAME = L"ZAVirtualPrinter";
    
    // State tracking
    bool m_verbose;
    DWORD m_lastError;
};
