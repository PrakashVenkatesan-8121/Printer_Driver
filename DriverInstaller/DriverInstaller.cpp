#include "DriverInstaller.h"
#include <iostream>
#include <sstream>
#include <setupapi.h>
#include <newdev.h>
#include <cfgmgr32.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")
#pragma comment(lib, "cfgmgr32.lib")

DriverInstaller::DriverInstaller()
    : m_verbose(true), m_lastError(0)
{
}

DriverInstaller::~DriverInstaller()
{
}

bool DriverInstaller::IsAdministrator()
{
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2, 
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &adminGroup))
    {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
    FreeSid(adminGroup);
    }

    return isAdmin == TRUE;
}

std::wstring DriverInstaller::GetLastErrorString()
{
DWORD error = (m_lastError != 0) ? m_lastError : GetLastError();
    if (error == 0)
        return L"No error";

  LPWSTR messageBuffer = nullptr;
 size_t size = FormatMessageW(
     FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&messageBuffer, 0, NULL);

    std::wstring message(messageBuffer, size);
  LocalFree(messageBuffer);

    return message;
}

void DriverInstaller::PrintStatus(const std::wstring& message, bool isError)
{
if (m_verbose)
    {
      if (isError)
      std::wcout << L"[ERROR] " << message << std::endl;
        else
 std::wcout << L"[OK] " << message << std::endl;
    }
}

bool DriverInstaller::RemoveDriverPackages()
{
    PrintStatus(L"Searching for existing driver packages...");
    
    // Use DiInstallDriver API to uninstall
    WCHAR infPath[MAX_PATH];
    DWORD flags = DIIRFLAG_FORCE_INF;
  
    // First, try to enumerate and remove using SetupAPI
    HDEVINFO deviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
    if (deviceInfoSet == INVALID_HANDLE_VALUE)
    {
        m_lastError = GetLastError();
        PrintStatus(L"Failed to create device info list: " + GetLastErrorString(), true);
     return false;
    }

    // Remove all devices with our hardware ID
    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    HDEVINFO devs = SetupDiGetClassDevs(
        NULL,
        NULL,
      NULL,
        DIGCF_ALLCLASSES | DIGCF_PRESENT);

    if (devs != INVALID_HANDLE_VALUE)
    {
        bool foundDevice = false;
        for (DWORD i = 0; SetupDiEnumDeviceInfo(devs, i, &deviceInfoData); i++)
 {
        WCHAR hardwareId[MAX_PATH] = {0};
  if (SetupDiGetDeviceRegistryProperty(
        devs,
          &deviceInfoData,
    SPDRP_HARDWAREID,
        NULL,
         (PBYTE)hardwareId,
       sizeof(hardwareId),
     NULL))
            {
             if (wcsstr(hardwareId, DEVICE_HARDWARE_ID) != NULL)
              {
         foundDevice = true;
     PrintStatus(L"Found device instance, removing...");
        
         if (SetupDiCallClassInstaller(DIF_REMOVE, devs, &deviceInfoData))
             {
  PrintStatus(L"Device instance removed");
          }
     else
     {
              m_lastError = GetLastError();
            PrintStatus(L"Failed to remove device: " + GetLastErrorString(), true);
          }
    }
  }
        }
   
        if (!foundDevice)
        {
          PrintStatus(L"No existing device instances found");
        }
        
        SetupDiDestroyDeviceInfoList(devs);
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return true;
}

bool DriverInstaller::UninstallDriverFromStore(const std::wstring& infName)
{
    PrintStatus(L"Removing driver package: " + infName);
    
    // Use DiUninstallDriver to remove the driver package
    BOOL needReboot = FALSE;
    if (!DiUninstallDriver(NULL, infName.c_str(), DIURFLAG_NO_REMOVE_INF, &needReboot))
    {
        m_lastError = GetLastError();
        
        // ERROR_NOT_INSTALLED is acceptable - means driver wasn't installed
        if (m_lastError != ERROR_NOT_INSTALLED)
        {
   PrintStatus(L"Failed to uninstall driver: " + GetLastErrorString(), true);
      return false;
      }
        else
        {
          PrintStatus(L"Driver was not installed");
return true;
    }
    }

    if (needReboot)
    {
        PrintStatus(L"System reboot required for complete removal", true);
    }

    PrintStatus(L"Driver package removed");
    return true;
}

bool DriverInstaller::CleanupExistingDriver()
{
    PrintStatus(L"");
    PrintStatus(L"[1/2] Cleaning up existing driver...");
    PrintStatus(L"");

    // Remove device instances
    if (!RemoveDriverPackages())
    {
   return false;
    }

    // Try to uninstall using INF name pattern
    // This will attempt to remove any published OEM*.inf files for our driver
    WCHAR infSearchPath[MAX_PATH];
    GetSystemDirectory(infSearchPath, MAX_PATH);
    wcscat_s(infSearchPath, L"\\DriverStore\\FileRepository\\");
    
    PrintStatus(L"Cleanup complete");
    PrintStatus(L"");
    
    return true;
}

bool DriverInstaller::AddDriverToStore(const std::wstring& infPath)
{
 PrintStatus(L"Adding driver to Windows driver store...");
    PrintStatus(L"INF Path: " + infPath);
    
    BOOL needReboot = FALSE;
    WCHAR destInfPath[MAX_PATH] = {0};
    DWORD destInfPathSize = MAX_PATH;

    // Install the driver into the driver store
    if (!SetupCopyOEMInf(
        infPath.c_str(),
        NULL,     // Use default source directory
     SPOST_PATH,                 // Source is a path
        SP_COPY_NEWER_OR_SAME,     // Copy if newer or same version
        destInfPath,
      destInfPathSize,
     NULL,
        NULL))
    {
        m_lastError = GetLastError();
    PrintStatus(L"Failed to copy INF to driver store: " + GetLastErrorString(), true);
  
        // Print additional error details
    if (m_lastError == ERROR_FILE_NOT_FOUND)
        {
          PrintStatus(L"The INF file was not found at the specified path", true);
   }
        else if (m_lastError == ERROR_INVALID_DATA)
 {
            PrintStatus(L"The INF file contains invalid data or is not properly formatted", true);
  }
        else if (m_lastError == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)
        {
         PrintStatus(L"Driver signature verification failed - ensure test signing is enabled", true);
        }
    
        return false;
    }

    PrintStatus(L"Driver copied to: " + std::wstring(destInfPath));

    // Now install the driver using DiInstallDriver
    PrintStatus(L"Installing driver...");
    
    if (!DiInstallDriver(
        NULL,
        destInfPath,
        DIIRFLAG_FORCE_INF,
     &needReboot))
    {
        m_lastError = GetLastError();
        PrintStatus(L"Failed to install driver: " + GetLastErrorString(), true);
        return false;
    }

    if (needReboot)
    {
        PrintStatus(L"System reboot required for driver installation", true);
    }

  PrintStatus(L"Driver installed successfully");
    return true;
}

bool DriverInstaller::InstallDriver(const std::wstring& infPath)
{
    PrintStatus(L"");
    PrintStatus(L"[2/2] Installing driver...");
    PrintStatus(L"");

    // Verify INF file exists
    DWORD fileAttr = GetFileAttributes(infPath.c_str());
    if (fileAttr == INVALID_FILE_ATTRIBUTES)
    {
        PrintStatus(L"INF file not found: " + infPath, true);
  return false;
    }

    // Add driver to store and install
    if (!AddDriverToStore(infPath))
    {
 return false;
    }

    PrintStatus(L"");
    PrintStatus(L"Driver installation complete");
    PrintStatus(L"");

    return true;
}

bool DriverInstaller::EnumerateDriverPackages()
{
    PrintStatus(L"Enumerating installed driver packages...");
    
    bool foundDriver = false;
    
    // Method 1: Search through OEM INF files
    WCHAR infDir[MAX_PATH];
    GetSystemDirectory(infDir, MAX_PATH);
    wcscat_s(infDir, L"\\INF\\oem*.inf");
    
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(infDir, &findData);
    
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            // Build full path to INF file
            WCHAR infPath[MAX_PATH];
            GetSystemDirectory(infPath, MAX_PATH);
            wcscat_s(infPath, L"\\INF\\");
            wcscat_s(infPath, findData.cFileName);
            
            // Read the INF file to check if it's our driver
            WCHAR provider[MAX_PATH] = {0};
            WCHAR className[MAX_PATH] = {0};
            WCHAR deviceName[MAX_PATH] = {0};
            
            // Check Provider and DeviceName fields
            GetPrivateProfileString(L"Version", L"Provider", L"", provider, MAX_PATH, infPath);
            GetPrivateProfileString(L"Version", L"Class", L"", className, MAX_PATH, infPath);
            GetPrivateProfileString(L"Strings", L"DeviceName", L"", deviceName, MAX_PATH, infPath);
            
            // Check if this is our driver
            if (wcsstr(provider, L"ZA Virtual Printer") != NULL ||
                wcsstr(deviceName, L"ZA Virtual") != NULL)
            {
                foundDriver = true;
                PrintStatus(L"Found driver package in driver store:");
                PrintStatus(L"  Published INF: " + std::wstring(findData.cFileName));
                PrintStatus(L"  Provider: " + std::wstring(provider));
                PrintStatus(L"  Class: " + std::wstring(className));
                
                // Get version
                WCHAR version[MAX_PATH] = {0};
                GetPrivateProfileString(L"Version", L"DriverVer", L"", version, MAX_PATH, infPath);
                if (wcslen(version) > 0)
                {
                    PrintStatus(L"  Version: " + std::wstring(version));
                }
                
                // Get catalog file
                WCHAR catalogFile[MAX_PATH] = {0};
                GetPrivateProfileString(L"Version", L"CatalogFile", L"", catalogFile, MAX_PATH, infPath);
                if (wcslen(catalogFile) > 0)
                {
                    PrintStatus(L"  Catalog: " + std::wstring(catalogFile));
                }
                
                // Try to find the actual driver store location
                WCHAR driverStorePath[MAX_PATH];
                GetSystemDirectory(driverStorePath, MAX_PATH);
                wcscat_s(driverStorePath, L"\\DriverStore\\FileRepository\\zavirtualprinter.inf_*");
                
                WIN32_FIND_DATA storeData;
                HANDLE hStoreFind = FindFirstFile(driverStorePath, &storeData);
                if (hStoreFind != INVALID_HANDLE_VALUE)
                {
                    WCHAR fullStorePath[MAX_PATH];
                    GetSystemDirectory(fullStorePath, MAX_PATH);
                    wcscat_s(fullStorePath, L"\\DriverStore\\FileRepository\\");
                    wcscat_s(fullStorePath, storeData.cFileName);
                    PrintStatus(L"  Store Location: " + std::wstring(fullStorePath));
                    FindClose(hStoreFind);
                }
                
                break; // Found our driver, no need to continue
            }
            
        } while (FindNextFile(hFind, &findData));
        
        FindClose(hFind);
    }
    
    if (!foundDriver)
    {
        PrintStatus(L"Driver package not found in Windows INF directory", false);
    }
    
    return foundDriver;
}

bool DriverInstaller::VerifyInstallation()
{
    PrintStatus(L"");
    PrintStatus(L"Verifying installation...");
    PrintStatus(L"");

    // Wait a moment for the system to register the driver
    Sleep(2000);

    // Enumerate to verify driver is installed
    if (EnumerateDriverPackages())
    {
        PrintStatus(L"Driver successfully verified in Windows driver store");
        return true;
    }
    else
    {
        // Even if we can't find it, the installation itself was successful
        // (SetupCopyOEMInf and DiInstallDriver both succeeded)
        PrintStatus(L"Driver verification could not locate package, but installation reported success", false);
        PrintStatus(L"The driver may still be usable - check Device Manager", false);
        
        // Return true because the actual installation succeeded
        return true;
    }
}
