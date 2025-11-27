#include "DriverInstaller.h"
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

void PrintUsage()
{
    std::wcout << L"\n";
    std::wcout << L"========================================\n";
    std::wcout << L"ZA Virtual Printer - Driver Installer\n";
  std::wcout << L"========================================\n";
    std::wcout << L"\n";
    std::wcout << L"Usage:\n";
 std::wcout << L"  DriverInstaller.exe [options]\n";
    std::wcout << L"\n";
    std::wcout << L"Options:\n";
    std::wcout << L"  -install    Install the driver (default)\n";
    std::wcout << L"  -uninstall  Uninstall the driver\n";
 std::wcout << L"  -verify  Verify driver installation\n";
    std::wcout << L"  -help       Show this help message\n";
    std::wcout << L"\n";
    std::wcout << L"Examples:\n";
    std::wcout << L"  DriverInstaller.exe\n";
    std::wcout << L"  DriverInstaller.exe -install\n";
    std::wcout << L"  DriverInstaller.exe -uninstall\n";
    std::wcout << L"\n";
}

std::wstring GetExecutableDirectory()
{
    WCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    
    std::wstring exePath(path);
    size_t lastSlash = exePath.find_last_of(L"\\");
    if (lastSlash != std::wstring::npos)
    {
        return exePath.substr(0, lastSlash);
    }
    return L".";
}

int wmain(int argc, wchar_t* argv[])
{
    std::wcout << L"\n";
    std::wcout << L"========================================\n";
 std::wcout << L"ZA Virtual Printer - Driver Installer\n";
    std::wcout << L"========================================\n";
    std::wcout << L"\n";

    // Parse command line arguments
    bool doInstall = true;
    bool doUninstall = false;
    bool doVerify = false;

    for (int i = 1; i < argc; i++)
    {
        std::wstring arg(argv[i]);
        if (arg == L"-help" || arg == L"--help" || arg == L"-h" || arg == L"/?")
        {
            PrintUsage();
 return 0;
    }
   else if (arg == L"-install")
   {
doInstall = true;
            doUninstall = false;
        }
        else if (arg == L"-uninstall")
   {
            doInstall = false;
 doUninstall = true;
        }
        else if (arg == L"-verify")
        {
     doInstall = false;
    doVerify = true;
        }
        else
        {
  std::wcout << L"[ERROR] Unknown option: " << arg << L"\n";
  PrintUsage();
            return 1;
        }
    }

    // Create installer instance
    DriverInstaller installer;

 // Check for administrator privileges
    if (!installer.IsAdministrator())
    {
  std::wcout << L"[ERROR] This program must be run as Administrator!\n";
    std::wcout << L"\n";
        std::wcout << L"Please right-click the executable and select 'Run as administrator'\n";
        std::wcout << L"\n";
        std::wcout << L"Press any key to exit...";
        std::wcin.get();
        return 1;
    }

    std::wcout << L"[OK] Running with Administrator privileges\n";
    std::wcout << L"\n";

    // Get INF file path
    std::wstring exeDir = GetExecutableDirectory();
    std::wstring infPath = exeDir + L"\\ZAVirtualPrinter.inf";

    std::wcout << L"Working Directory: " << exeDir << L"\n";
    std::wcout << L"INF File: " << infPath << L"\n";
  std::wcout << L"\n";

    // Verify INF file exists (for install operation)
    if (doInstall)
    {
        DWORD fileAttr = GetFileAttributes(infPath.c_str());
        if (fileAttr == INVALID_FILE_ATTRIBUTES)
        {
std::wcout << L"[ERROR] INF file not found: " << infPath << L"\n";
       std::wcout << L"\n";
            std::wcout << L"Please ensure ZAVirtualPrinter.inf is in the same directory as this executable.\n";
        std::wcout << L"\n";
  std::wcout << L"Press any key to exit...";
   std::wcin.get();
return 1;
        }
        std::wcout << L"[OK] Found INF file\n";
        std::wcout << L"\n";
    }

    bool success = true;

    // Perform requested operation
    if (doUninstall)
    {
 std::wcout << L"=== UNINSTALLING DRIVER ===\n";
        std::wcout << L"\n";
        success = installer.CleanupExistingDriver();
    }
    else if (doVerify)
    {
    std::wcout << L"=== VERIFYING INSTALLATION ===\n";
        std::wcout << L"\n";
        success = installer.VerifyInstallation();
    }
    else // doInstall
    {
        std::wcout << L"=== INSTALLING DRIVER ===\n";
        std::wcout << L"\n";

        // Step 1: Cleanup existing installation
    if (!installer.CleanupExistingDriver())
     {
   std::wcout << L"\n";
            std::wcout << L"[WARNING] Cleanup encountered issues, but continuing...\n";
        }

        // Step 2: Install driver
    if (!installer.InstallDriver(infPath))
        {
            std::wcout << L"\n";
      std::wcout << L"[ERROR] Driver installation failed!\n";
      std::wcout << L"\n";
  std::wcout << L"Common issues:\n";
        std::wcout << L"1. Driver not properly signed - Run Sign-Driver.bat\n";
  std::wcout << L"2. Test signing not enabled - Run Enable-TestSigning.bat and restart\n";
    std::wcout << L"3. INF file contains errors - Check Windows Event Viewer\n";
            std::wcout << L"4. Missing catalog file (.cat)\n";
   std::wcout << L"\n";
 success = false;
        }
    else
   {
         // Step 3: Verify installation
     installer.VerifyInstallation();
        }
    }

    // Print final status
    std::wcout << L"\n";
    std::wcout << L"========================================\n";
    if (success)
    {
      std::wcout << L"Operation Completed Successfully!\n";
        std::wcout << L"========================================\n";
  std::wcout << L"\n";

 if (doInstall)
        {
         std::wcout << L"Next steps:\n";
            std::wcout << L"1. Open Device Manager (devmgmt.msc)\n";
         std::wcout << L"2. Click Action > Add legacy hardware\n";
            std::wcout << L"3. Select 'Install the hardware that I manually select from a list'\n";
  std::wcout << L"4. Choose 'Printer' from the list\n";
     std::wcout << L"5. Click 'Have Disk' and browse to: " << exeDir << L"\n";
      std::wcout << L"6. Select 'ZA Virtual Logging Printer'\n";
        std::wcout << L"\n";
     std::wcout << L"The driver is now in the Windows driver store and ready to use.\n";
        }
    }
    else
    {
        std::wcout << L"Operation Failed!\n";
        std::wcout << L"========================================\n";
    }
    std::wcout << L"\n";

    std::wcout << L"Press any key to exit...";
    std::wcin.get();

    return success ? 0 : 1;
}
