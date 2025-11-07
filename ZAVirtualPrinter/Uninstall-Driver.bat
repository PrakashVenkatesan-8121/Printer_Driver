@echo off
REM ================================================================
REM ZAVirtualPrinter Driver Uninstallation Script
REM ================================================================
REM This script uninstalls the ZAVirtualPrinter UMDF driver
REM Must be run as Administrator
REM ================================================================

SETLOCAL EnableDelayedExpansion

echo.
echo ========================================
echo ZAVirtualPrinter Driver Uninstallation
echo ========================================
echo.

REM Check for Administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator!
  echo.
echo Right-click this file and select "Run as administrator"
    echo.
    pause
    exit /b 1
)

echo [OK] Running with Administrator privileges
echo.

REM Confirm uninstallation
set /p "CONFIRM=Are you sure you want to uninstall ZAVirtualPrinter driver? (Y/N): "
if /i not "!CONFIRM!"=="Y" (
    echo Uninstallation cancelled.
    pause
    exit /b 0
)
echo.

REM Find and remove device instances
echo Searching for driver devices...
devcon find "Root\ZAVirtualPrinter" >nul 2>&1
if %errorLevel% equ 0 (
    echo Found device instances, removing...
    devcon remove "Root\ZAVirtualPrinter"
  if !errorLevel! equ 0 (
        echo [OK] Device instances removed
    ) else (
   echo ERROR: Failed to remove device instances
        echo You may need to remove them manually from Device Manager
    )
) else (
    echo [OK] No device instances found
)
echo.

REM Find the driver package
echo Searching for driver package...
for /f "tokens=*" %%i in ('pnputil /enum-drivers ^| findstr /i "ZAVirtualPrinter"') do (
    set "LINE=%%i"
 if "!LINE:~0,17!"=="Published Name :" (
        set "DRIVER_INF=!LINE:~17!"
set "DRIVER_INF=!DRIVER_INF: =!"
        echo Found driver package: !DRIVER_INF!
    )
)

if defined DRIVER_INF (
echo Removing driver package: %DRIVER_INF%
    pnputil /delete-driver "%DRIVER_INF%" /uninstall /force
    if !errorLevel! equ 0 (
        echo [OK] Driver package removed
    ) else (
        echo ERROR: Failed to remove driver package
        echo Try manually with: pnputil /delete-driver %DRIVER_INF% /uninstall /force
    )
) else (
    echo WARNING: Driver package not found in system
    echo The driver may have already been uninstalled
)
echo.

REM Try alternative removal method
echo Checking for any remaining driver files...
pnputil /enum-drivers | findstr /i "ZA" | findstr /i "Virtual" | findstr /i "Printer" >nul 2>&1
if %errorLevel% equ 0 (
    echo WARNING: Found potential remaining driver entries
    echo Please check Device Manager and remove any remaining devices manually
) else (
    echo [OK] No driver entries found
)
echo.

REM Remove from Device Manager if still present
echo Checking Device Manager...
devcon findall =Printer | findstr /i "ZAVirtualPrinter" >nul 2>&1
if %errorLevel% equ 0 (
    echo WARNING: Device still visible in Device Manager
  echo Please remove it manually:
    echo 1. Open Device Manager (devmgmt.msc)
    echo 2. Find "ZA Virtual Logging Printer"
    echo 3. Right-click and select "Uninstall device"
    echo 4. Check "Delete the driver software for this device"
  echo 5. Click "Uninstall"
) else (
    echo [OK] Device not found in Device Manager
)
echo.

REM Clean up driver store remnants
echo Cleaning up driver store...
for /f "tokens=2 delims=:" %%a in ('pnputil /enum-drivers ^| findstr /C:"Original Name" ^| findstr /i "ZAVirtualPrinter"') do (
    set "INF_FILE=%%a"
    set "INF_FILE=!INF_FILE: =!"
    if defined INF_FILE (
  echo Removing: !INF_FILE!
        pnputil /delete-driver "!INF_FILE!" /uninstall /force
    )
)
echo [OK] Cleanup complete
echo.

echo ========================================
echo Uninstallation Complete!
echo ========================================
echo.
echo The ZAVirtualPrinter driver has been removed from your system.
echo.
echo To verify:
echo 1. Open Device Manager (devmgmt.msc)
echo 2. Check that "ZA Virtual Logging Printer" is not listed
echo 3. Run: pnputil /enum-drivers and verify no ZAVirtualPrinter entries
echo.
echo If you see any remaining entries:
echo - Restart your computer to complete the removal
echo - Check Event Viewer for any error messages
echo.

pause
exit /b 0
