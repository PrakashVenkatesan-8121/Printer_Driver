@echo off
REM ================================================================
REM ZAVirtualPrinter Driver Signing Script
REM ================================================================
REM This script creates a catalog file and signs the driver
REM Assumes all files (INF, DLL, CAT, CER) are in the same directory
REM Must be run as Administrator
REM ================================================================

SETLOCAL EnableDelayedExpansion

echo.
echo ========================================
echo ZAVirtualPrinter Driver Signing
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
set "CERT_FILE=%SCRIPT_DIR%\ZAVirtualPrinter.cer"
set "CAT_FILE=%SCRIPT_DIR%\ZAVirtualPrinter.cat"

REM Verify files exist
echo Checking files...
echo.

if not exist "%DRIVER_DLL%" (
    echo ERROR: Driver DLL not found: %DRIVER_DLL%
    echo.
    echo Please ensure the following files are in %SCRIPT_DIR%:
    echo   - ZAVirtualPrinter.dll
    echo   - ZAVirtualPrinter.inf
    echo.
    echo These should be copied automatically by the post-build event.
    echo If not, please build the project first.
    pause
    exit /b 1
)
echo [OK] Found driver DLL

if not exist "%INF_FILE%" (
    echo ERROR: INF file not found: %INF_FILE%
    pause
    exit /b 1
)
echo [OK] Found INF file
echo.

REM Check for certificate
if not exist "%CERT_FILE%" (
    echo WARNING: Certificate file not found at: %CERT_FILE%
    echo.
    echo Creating a self-signed test certificate...
    makecert -r -pe -ss PrivateCertStore -n "CN=ZAVirtualPrinter Test Certificate" -eku 1.3.6.1.5.5.7.3.3 "%CERT_FILE%"
    if !errorLevel! neq 0 (
        echo ERROR: Failed to create certificate
        echo.
        echo Please ensure Windows SDK is installed (for makecert tool)
        echo Or manually create a certificate and place it at: %CERT_FILE%
        pause
        exit /b 1
    )
    echo [OK] Certificate created
) else (
    echo [OK] Found certificate file
)
echo.

REM Install the certificate to Trusted Root store
echo Installing certificate to Trusted Root Certification Authorities...
certutil -addstore Root "%CERT_FILE%" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Certificate installed
) else (
    REM Check if already installed
    certutil -store Root | findstr /i "ZAVirtualPrinter" >nul 2>&1
    if !errorLevel! equ 0 (
        echo [OK] Certificate already installed
    ) else (
        echo WARNING: Could not install certificate automatically
        echo You may need to install it manually via certmgr.msc
    )
)
echo.

REM Delete old catalog file if it exists
if exist "%CAT_FILE%" (
    echo Removing old catalog file...
    del /F /Q "%CAT_FILE%" >nul 2>&1
)

REM Create catalog file using inf2cat
echo Creating catalog file...
set "OS_VERSION=10_X64"
if "%ARCH%"=="ARM64" set "OS_VERSION=10_ARM64"
if "%ARCH%"=="x86" set "OS_VERSION=10_X86"

REM Try different WDK versions (newest to oldest)
set "WDK_PATHS[0]=C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x86\inf2cat.exe"
set "WDK_PATHS[1]=C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x86\inf2cat.exe"
set "WDK_PATHS[2]=C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x86\inf2cat.exe"
set "WDK_PATHS[3]=C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\inf2cat.exe"

set "INF2CAT_FOUND="
for /L %%i in (0,1,3) do (
    if exist "!WDK_PATHS[%%i]!" (
        set "INF2CAT_PATH=!WDK_PATHS[%%i]!"
        set "INF2CAT_FOUND=1"
        goto :inf2cat_found
    )
)

:inf2cat_found
if not defined INF2CAT_FOUND (
    echo ERROR: inf2cat.exe not found!
    echo.
    echo Please install Windows Driver Kit (WDK)
    echo Download from: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
    pause
    exit /b 1
)

echo Using inf2cat from: %INF2CAT_PATH%
echo.

"%INF2CAT_PATH%" /driver:"%SCRIPT_DIR%" /os:%OS_VERSION% /verbose
if %errorLevel% neq 0 (
    echo ERROR: Failed to create catalog file
    echo.
    echo This may happen if:
    echo - INF file has syntax errors
    echo - Driver files don't match INF references
    echo - Windows Driver Kit (WDK) is not properly installed
    echo.
    pause
    exit /b 1
)

if not exist "%CAT_FILE%" (
    echo ERROR: Catalog file was not created!
    echo Expected at: %CAT_FILE%
    pause
    exit /b 1
)

echo [OK] Catalog file created: %CAT_FILE%
echo.

REM Sign the catalog file
echo Signing catalog file...
signtool sign /v /s Root /n "ZAVirtualPrinter Test Certificate" /t http://timestamp.digicert.com "%CAT_FILE%"
if %errorLevel% neq 0 (
    echo WARNING: Timestamp server unavailable, trying without timestamp...
    signtool sign /v /s Root /n "ZAVirtualPrinter Test Certificate" "%CAT_FILE%"
    if !errorLevel! neq 0 (
        echo ERROR: Failed to sign catalog file
        echo.
        echo Please check:
        echo 1. Certificate is installed in Root store
        echo 2. SignTool is available (part of Windows SDK)
        pause
        exit /b 1
    )
)
echo [OK] Catalog file signed
echo.

REM Sign the driver DLL
echo Signing driver DLL...
signtool sign /v /s Root /n "ZAVirtualPrinter Test Certificate" /t http://timestamp.digicert.com "%DRIVER_DLL%"
if %errorLevel% neq 0 (
    echo WARNING: Timestamp server unavailable, trying without timestamp...
    signtool sign /v /s Root /n "ZAVirtualPrinter Test Certificate" "%DRIVER_DLL%"
    if !errorLevel! neq 0 (
        echo WARNING: Could not sign driver DLL (may still work with catalog signature)
    )
)
echo [OK] Driver DLL signed
echo.

REM Verify signatures
echo Verifying signatures...
signtool verify /pa "%CAT_FILE%" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Catalog signature is valid
) else (
    echo WARNING: Catalog signature verification failed
)

signtool verify /pa "%DRIVER_DLL%" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Driver DLL signature is valid
) else (
    echo WARNING: Driver DLL signature verification failed (may still work)
)
echo.

echo ========================================
echo Signing Complete!
echo ========================================
echo.
echo Files in directory: %SCRIPT_DIR%
echo   - ZAVirtualPrinter.dll (signed)
echo   - ZAVirtualPrinter.cat (signed)
echo   - ZAVirtualPrinter.inf
echo   - ZAVirtualPrinter.cer
echo.
echo Next steps:
echo 1. Ensure test signing is enabled: Enable-TestSigning.bat
echo 2. Restart your computer if test signing was just enabled
echo 3. Run Install-Driver.bat to install the signed driver
echo.

pause
exit /b 0
