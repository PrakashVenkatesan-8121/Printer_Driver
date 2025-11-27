@echo off
REM ================================================================
REM ZAVirtualPrinter Driver Installation Script (Simplified)
REM ================================================================
REM Must be run as Administrator
REM Assumes test signing is already enabled
REM ================================================================

SETLOCAL EnableDelayedExpansion

echo.
echo ========================================
echo ZAVirtualPrinter Driver Installation
echo ========================================
echo.

REM Check for Administrator privileges
net session 
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator!
    echo Right-click this file and select "Run as administrator"
    pause
    exit /b 1
)

REM Get script directory
set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
set "INF_FILE=%SCRIPT_DIR%\ZAVirtualPrinter.inf"

echo Working Directory: %SCRIPT_DIR%
echo INF File: %INF_FILE%
echo.

REM Verify INF file exists
if not exist "%INF_FILE%" (
    echo ERROR: INF file not found: %INF_FILE%
    pause
    exit /b 1
)

REM ================================================================
REM CLEANUP - Remove existing driver installation
REM ================================================================
echo [1/3] Cleaning up existing driver...
echo.

REM Remove device instances
devcon find "Root\ZAVirtualPrinter" 
if %errorLevel% equ 0 (
    echo Removing existing device instances...
    devcon remove "Root\ZAVirtualPrinter" 
    echo [OK] Device instances removed
)

REM Remove driver package from driver store
echo Removing driver package from driver store...
for /f "tokens=*" %%i in ('pnputil /enum-drivers 2^>nul ^| findstr /i "ZAVirtualPrinter"') do (
    set "LINE=%%i"
    if "!LINE:~0,17!"=="Published Name :" (
        set "DRIVER_INF=!LINE:~17!"
        set "DRIVER_INF=!DRIVER_INF: =!"
        echo Found existing driver: !DRIVER_INF!
        pnputil /delete-driver "!DRIVER_INF!" /uninstall /force 
        echo [OK] Removed !DRIVER_INF!
    )
)

echo [OK] Cleanup complete
echo.

REM ================================================================
REM INSTALLATION - Install the driver
REM ================================================================
echo [2/3] Installing driver...
echo.

REM Add driver to driver store and install
pnputil /add-driver "%INF_FILE%" /install
if %errorLevel% neq 0 (
    echo ERROR: Failed to install driver with pnputil
    echo Trying alternative method...
    
    REM Fallback to devcon
    devcon install "%INF_FILE%" "Root\ZAVirtualPrinter"
    if !errorLevel! neq 0 (
        echo ERROR: Driver installation failed!
        echo.
        echo Possible issues:
        echo - Driver not properly signed
        echo - Test signing not enabled
        echo - INF file contains errors
        pause
        exit /b 1
    )
)

`echo [OK] Driver installed successfully
echo.

REM ================================================================
REM VERIFICATION - Create device and verify installation
REM ================================================================
echo [3/3] Creating device instance...
echo.

REM Create device instance
devcon install "%INF_FILE%" "Root\ZAVirtualPrinter" 
timeout /t 2 >nul

REM Verify device is present
devcon status "Root\ZAVirtualPrinter"
if %errorLevel% equ 0 (
    echo [OK] Device created successfully
    echo.
    devcon status "Root\ZAVirtualPrinter"
) else (
    echo WARNING: Device not found
    echo The driver is installed but device instance may need manual creation
)
echo.

REM Verify driver in driver store
pnputil /enum-drivers | findstr /i "ZAVirtualPrinter" 
if %errorLevel% equ 0 (
    echo [OK] Driver found in system driver store
) else (
    echo WARNING: Driver not found in driver store
)
echo.

REM ================================================================
REM COMPLETION
REM ================================================================
echo ========================================
echo Installation Complete!
echo ========================================
echo.
echo Check Device Manager for "ZA Virtual Logging Printer"
echo Location: Device Manager ^> Printers or Print queues
echo.

pause
exit /b 0
