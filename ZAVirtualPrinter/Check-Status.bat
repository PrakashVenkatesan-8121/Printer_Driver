@echo off
REM ================================================================
REM Check ZAVirtualPrinter Driver Status
REM ================================================================
REM This script checks the installation status of the driver
REM Useful for troubleshooting and verification
REM ================================================================

SETLOCAL EnableDelayedExpansion

echo.
echo ========================================
echo ZAVirtualPrinter Status Check
echo ========================================
echo.

REM ================================================================
REM Check 1: Test Signing Mode
REM ================================================================
echo [1] Test Signing Mode:
echo ----------------------------------------
bcdedit /enum {current} | findstr /i "testsigning.*Yes" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Test signing is ENABLED
) else (
    echo [!] Test signing is DISABLED
    echo     Run Enable-TestSigning.bat to enable
)
echo.

REM ================================================================
REM Check 2: Certificate Installation
REM ================================================================
echo [2] Certificate Status:
echo ----------------------------------------
certutil -store Root | findstr /i "ZAVirtualPrinter" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Certificate is installed in Trusted Root store
    certutil -store Root "ZAVirtualPrinter Test Certificate" | findstr /i "Cert Hash"
) else (
    echo [!] Certificate NOT found in Trusted Root store
    echo     Run Sign-Driver.bat to create and install certificate
)
echo.

REM ================================================================
REM Check 3: Driver Files
REM ================================================================
echo [3] Driver Files:
echo ----------------------------------------
set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

if exist "%SCRIPT_DIR%\ZAVirtualPrinter.dll" (
    echo [OK] ZAVirtualPrinter.dll found
) else (
    echo [!] ZAVirtualPrinter.dll NOT found
    echo     Build the project in Visual Studio
)

if exist "%SCRIPT_DIR%\ZAVirtualPrinter.inf" (
    echo [OK] ZAVirtualPrinter.inf found
) else (
    echo [!] ZAVirtualPrinter.inf NOT found
)

if exist "%SCRIPT_DIR%\ZAVirtualPrinter.cat" (
    echo [OK] ZAVirtualPrinter.cat found
) else (
    echo [!] ZAVirtualPrinter.cat NOT found
    echo     Run Sign-Driver.bat to create catalog
)

if exist "%SCRIPT_DIR%\ZAVirtualPrinter.cer" (
    echo [OK] ZAVirtualPrinter.cer found
) else (
    echo [!] ZAVirtualPrinter.cer NOT found
    echo     Run Sign-Driver.bat to create certificate
)
echo.

REM ================================================================
REM Check 4: Driver Installation in Driver Store
REM ================================================================
echo [4] Driver Store Status:
echo ----------------------------------------
set "DRIVER_FOUND=0"
for /f "tokens=*" %%i in ('pnputil /enum-drivers 2^>nul ^| findstr /i "ZAVirtualPrinter"') do (
 set "LINE=%%i"
    if "!LINE:~0,17!"=="Published Name :" (
        set "DRIVER_INF=!LINE:~17!"
        set "DRIVER_INF=!DRIVER_INF: =!"
        echo [OK] Driver installed in driver store: !DRIVER_INF!
   set "DRIVER_FOUND=1"
    )
)

if "%DRIVER_FOUND%"=="0" (
    echo [!] Driver NOT found in driver store
    echo   Run Install-Driver.bat to install
)
echo.

REM ================================================================
REM Check 5: Device Instance
REM ================================================================
echo [5] Device Instance:
echo ----------------------------------------
devcon status "Root\ZAVirtualPrinter" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Device instance found:
    devcon status "Root\ZAVirtualPrinter"
) else (
    echo [!] Device instance NOT found
    echo     Run Install-Driver.bat to create device
)
echo.

REM ================================================================
REM Check 6: Log Files
REM ================================================================
echo [6] Log Files:
echo ----------------------------------------
set "LOG_FOUND=0"

for %%d in (C D E) do (
    if exist "%%d:\Logs\VirtualPrinterDriver.log" (
      echo [OK] Log file found: %%d:\Logs\VirtualPrinterDriver.log
        for %%f in ("%%d:\Logs\VirtualPrinterDriver.log") do echo     Size: %%~zf bytes
        set "LOG_FOUND=1"
    )
)

if exist "%TEMP%\VirtualPrinterDriver.log" (
    echo [OK] Fallback log found: %TEMP%\VirtualPrinterDriver.log
    for %%f in ("%TEMP%\VirtualPrinterDriver.log") do echo     Size: %%~zf bytes
    set "LOG_FOUND=1"
)

if "%LOG_FOUND%"=="0" (
    echo [!] No log files found
    echo     Driver may not have been started yet
)
echo.

REM ================================================================
REM Summary
REM ================================================================
echo ========================================
echo Summary
echo ========================================
echo.

REM Count checks
set "CHECKS_PASSED=0"
set "CHECKS_FAILED=0"

REM This is a simplified summary - in a real script you'd track each check
echo Run the following commands to install the driver:
echo.
echo 1. Build project in Visual Studio
echo 2. Run Sign-Driver.bat as Administrator
echo 3. Run Enable-TestSigning.bat as Administrator
echo 4. Restart computer
echo 5. Run Install-Driver.bat as Administrator
echo.
echo To view logs in real-time:
echo    View-Logs.bat
echo.
echo To uninstall:
echo    Uninstall-Driver.bat
echo.

pause
exit /b 0
