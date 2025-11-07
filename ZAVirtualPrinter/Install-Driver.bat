@echo off
REM ================================================================
REM ZAVirtualPrinter Driver Installation Script
REM ================================================================
REM This script installs the ZAVirtualPrinter UMDF driver
REM Assumes all files (INF, DLL, CAT, CER) are in the same directory
REM Must be run as Administrator
REM ================================================================

SETLOCAL EnableDelayedExpansion

echo.
echo ========================================
echo ZAVirtualPrinter Driver Installation
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

REM Determine the script directory (all files should be here)
set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

REM Detect architecture
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set "ARCH=x64"
) else if "%PROCESSOR_ARCHITECTURE%"=="ARM64" (
    set "ARCH=ARM64"
) else (
    set "ARCH=x86"
)

echo System Architecture: %ARCH%
echo Working Directory: %SCRIPT_DIR%
echo.

REM Set paths - all files are in the same directory as this script
set "INF_FILE=%SCRIPT_DIR%\ZAVirtualPrinter.inf"
set "DRIVER_DLL=%SCRIPT_DIR%\ZAVirtualPrinter.dll"
set "CAT_FILE=%SCRIPT_DIR%\ZAVirtualPrinter.cat"
set "CERT_FILE=%SCRIPT_DIR%\ZAVirtualPrinter.cer"

REM Verify files exist
echo Checking installation files...
echo.

if not exist "%INF_FILE%" (
    echo ERROR: INF file not found: %INF_FILE%
    echo Please ensure ZAVirtualPrinter.inf is in the same directory as this script.
    pause
    exit /b 1
)
echo [OK] Found INF file

if not exist "%DRIVER_DLL%" (
    echo ERROR: Driver DLL not found: %DRIVER_DLL%
    echo.
    echo Please ensure the following files are in %SCRIPT_DIR%:
    echo   - ZAVirtualPrinter.dll
    echo   - ZAVirtualPrinter.inf
    echo   - ZAVirtualPrinter.cat
    echo   - ZAVirtualPrinter.cer
    echo.
    echo These should be copied automatically by the post-build event.
    pause
    exit /b 1
)
echo [OK] Found driver DLL

REM Check for catalog file (critical for driver signing)
if not exist "%CAT_FILE%" (
    echo.
    echo WARNING: Catalog file not found: %CAT_FILE%
    echo.
    echo The catalog file is REQUIRED for driver installation!
    echo Without it, you will get error: 0xE000022F
    echo "The third-party INF does not contain digital signature information"
    echo.
    echo SOLUTION:
    echo 1. Run: Sign-Driver.bat (as Administrator)
    echo    This will create and sign the catalog file
    echo 2. Then run this script again
    echo.
    echo See FIXING_SIGNATURE_ERROR.md for detailed instructions
    echo.
    pause
    exit /b 1
)
echo [OK] Found catalog file

REM Check if certificate exists
if not exist "%CERT_FILE%" (
    echo WARNING: Certificate file not found: %CERT_FILE%
    echo You may need to run Sign-Driver.bat first
)

echo.

REM Check driver signature
echo Checking driver signature...
signtool verify /pa "%DRIVER_DLL%" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Driver is digitally signed
) else (
    echo WARNING: Driver is not signed or signature verification failed
    echo Test signing mode must be enabled to install this driver
    
    REM Check test signing status
    bcdedit /enum | findstr /i "testsigning" | findstr /i "Yes" >nul 2>&1
    if %errorLevel% neq 0 (
        echo.
        echo ERROR: Test signing is NOT enabled!
        echo.
        echo To enable test signing:
        echo   1. Run: Enable-TestSigning.bat (as Administrator)
        echo   2. Restart your computer
        echo   3. Run: Sign-Driver.bat (as Administrator)
        echo   4. Run this script again
        echo.
        echo See FIXING_SIGNATURE_ERROR.md for detailed instructions
        echo.
        set /p "CONTINUE=Do you want to enable test signing now? (Y/N): "
        if /i "!CONTINUE!"=="Y" (
            bcdedit /set testsigning on
            if !errorLevel! equ 0 (
                echo [OK] Test signing enabled
                echo.
                echo IMPORTANT: You must restart your computer now!
                echo After restart:
                echo   1. Run: Sign-Driver.bat
                echo   2. Run this script again
                pause
                exit /b 0
            ) else (
                echo Failed to enable test signing
                pause
                exit /b 1
            )
        ) else (
            echo Installation cancelled.
            pause
            exit /b 1
        )
    ) else (
        echo [OK] Test signing is enabled
    )
)

REM Check catalog signature
signtool verify /pa "%CAT_FILE%" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Catalog file is digitally signed
) else (
    echo WARNING: Catalog file signature verification failed
    echo You should run Sign-Driver.bat to properly sign the driver
    echo.
    set /p "CONTINUE=Continue installation anyway? (Y/N): "
    if /i not "!CONTINUE!"=="Y" (
        echo Installation cancelled.
        echo Please run Sign-Driver.bat first, then try again.
        pause
        exit /b 1
    )
)
echo.

REM Remove existing driver if present
echo Checking for existing driver installation...
devcon find "Root\ZAVirtualPrinter" >nul 2>&1
if %errorLevel% equ 0 (
    echo Found existing driver, removing...
    devcon remove "Root\ZAVirtualPrinter" >nul 2>&1
    if %errorLevel% equ 0 (
        echo [OK] Existing driver removed
    ) else (
        echo WARNING: Could not remove existing driver (may not be a problem)
    )
) else (
    echo [OK] No existing driver found
)
echo.

REM Install the driver
echo Installing driver...
echo.
echo Installing from: %SCRIPT_DIR%
echo INF file: %INF_FILE%
echo Catalog file: %CAT_FILE%
echo Driver DLL: %DRIVER_DLL%
echo.

REM Use pnputil to install the driver
pnputil /add-driver "%INF_FILE%" /install
if %errorLevel% neq 0 (
    echo.
    echo ERROR: Failed to install driver using pnputil
    echo.
    echo Common causes:
    echo 1. Driver is not properly signed - Run Sign-Driver.bat
    echo 2. Catalog file is missing or invalid
    echo 3. Test signing not enabled - Run Enable-TestSigning.bat
    echo.
    echo See FIXING_SIGNATURE_ERROR.md for troubleshooting steps
    echo.
    echo Trying alternative method with devcon...
    
    REM Try with devcon
    devcon install "%INF_FILE%" "Root\ZAVirtualPrinter"
    if !errorLevel! neq 0 (
        echo ERROR: Driver installation failed!
        echo.
        echo Troubleshooting steps:
        echo 1. Run: Sign-Driver.bat (to create and sign catalog file)
        echo 2. Ensure test signing is enabled: Enable-TestSigning.bat
        echo 3. Check Windows Event Viewer for detailed error messages
        echo 4. See FIXING_SIGNATURE_ERROR.md for complete guide
        echo.
        pause
        exit /b 1
    )
)

echo.
echo [OK] Driver installed successfully!
echo.

REM Install the device
echo Creating device instance...
devcon install "%INF_FILE%" "Root\ZAVirtualPrinter"
if %errorLevel% neq 0 (
    echo WARNING: Device creation may have issues, but driver is installed
) else (
    echo [OK] Device instance created
)
echo.

REM Verify installation
echo Verifying installation...
timeout /t 2 >nul

devcon status "Root\ZAVirtualPrinter" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Device is present in system
    echo.
    echo Device status:
    devcon status "Root\ZAVirtualPrinter"
) else (
    echo WARNING: Device not found, but driver may still be installed
)
echo.

REM Check in Device Manager
echo Installed Drivers:
pnputil /enum-drivers | findstr /i "ZAVirtualPrinter" 2>nul
if %errorLevel% neq 0 (
    echo Driver not found in pnputil list
) else (
    echo [OK] Driver found in system driver store
)
echo.

echo ========================================
echo Installation Complete!
echo ========================================
echo.
echo Next steps:
echo 1. Open Device Manager (devmgmt.msc)
echo 2. Look for "ZA Virtual Logging Printer" under:
echo    - Printers
echo    - Print queues
echo    - or Other devices (if there are issues)
echo 3. Check device status and properties
echo.
echo If the device shows an error:
echo - Right-click the device
echo - Select "Update driver"
echo - Choose "Browse my computer for drivers"
echo - Navigate to: %SCRIPT_DIR%
echo.

pause
exit /b 0
