@echo off
REM ================================================================
REM Enable Test Signing for Driver Development
REM ================================================================
REM This script enables test signing mode on Windows
REM Required for loading unsigned or test-signed drivers
REM Must be run as Administrator
REM ================================================================

SETLOCAL

echo.
echo ========================================
echo Enable Test Signing Mode
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

REM Check current test signing status
echo Checking current test signing status...
bcdedit /enum | findstr /i "testsigning" | findstr /i "Yes" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Test signing is already ENABLED
    echo.
    echo No action needed. You can install your driver.
    echo.
    pause
    exit /b 0
)

bcdedit /enum | findstr /i "testsigning" | findstr /i "No" >nul 2>&1
if %errorLevel% equ 0 (
    echo Test signing is currently DISABLED
) else (
    echo Test signing status: Unknown (probably disabled)
)
echo.

REM Explain what test signing is
echo About Test Signing:
echo Test signing allows Windows to load drivers that are:
echo - Unsigned (during development)
echo - Self-signed with a test certificate
echo - Not signed by a trusted Certificate Authority
echo.
echo This is necessary during driver development and testing.
echo.
echo Note: Test signing will show a watermark on your desktop
echo  indicating "Test Mode" in the corner.
echo.

REM Confirm action
set /p "CONFIRM=Do you want to enable test signing now? (Y/N): "
if /i not "%CONFIRM%"=="Y" (
    echo Operation cancelled.
    echo.
    echo To enable test signing manually, run:
    echo   bcdedit /set testsigning on
    echo.
    pause
    exit /b 0
)
echo.

REM Enable test signing
echo Enabling test signing...
bcdedit /set testsigning on
if %errorLevel% neq 0 (
    echo ERROR: Failed to enable test signing!
    echo.
    echo This may happen if:
    echo - Secure Boot is enabled in BIOS/UEFI
    echo - BitLocker is active
    echo - Group Policy restrictions are in place
    echo.
    echo Troubleshooting:
    echo 1. Disable Secure Boot in BIOS/UEFI
    echo 2. Suspend BitLocker: manage-bde -protectors -disable C:
    echo 3. Check with your system administrator for Group Policy
    echo.
    pause
    exit /b 1
)

echo [OK] Test signing enabled successfully!
echo.

REM Verify the change
bcdedit /enum | findstr /i "testsigning"
echo.

echo ========================================
echo Test Signing Enabled!
echo ========================================
echo.
echo IMPORTANT: You must restart your computer for changes to take effect!
echo.
echo After restarting:
echo 1. You will see "Test Mode" watermark on desktop (this is normal)
echo 2. You can install and test your driver
echo 3. Run Install-Driver.bat to install ZAVirtualPrinter
echo.

set /p "RESTART=Do you want to restart now? (Y/N): "
if /i "%RESTART%"=="Y" (
    echo.
    echo Restarting in 10 seconds...
    echo Press Ctrl+C to cancel
    timeout /t 10
    shutdown /r /t 0
) else (
    echo.
    echo Please restart your computer manually before installing the driver.
    echo.
    pause
)

exit /b 0
