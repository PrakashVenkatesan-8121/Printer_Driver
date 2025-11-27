@echo off
REM Build DriverInstaller Project
REM This script builds the DriverInstaller project and copies required files

echo.
echo ========================================
echo Building DriverInstaller
echo ========================================
echo.

REM Check for Visual Studio installation
where MSBuild >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: MSBuild not found!
    echo Please run this from a Visual Studio Developer Command Prompt
    echo OR add MSBuild to your PATH
    pause
    exit /b 1
)

REM Get parameters
set PLATFORM=%1
set CONFIGURATION=%2

REM Set defaults if not provided
if "%PLATFORM%"=="" set PLATFORM=x64
if "%CONFIGURATION%"=="" set CONFIGURATION=Release

echo Platform: %PLATFORM%
echo Configuration: %CONFIGURATION%
echo.

REM Build the project
echo Building DriverInstaller...
MSBuild DriverInstaller\DriverInstaller.vcxproj /p:Configuration=%CONFIGURATION% /p:Platform=%PLATFORM% /v:minimal
if %errorLevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo [OK] Build successful!
echo.

REM Output directory
set "OUTDIR=DriverInstaller\%PLATFORM%\%CONFIGURATION%"

echo Output directory: %OUTDIR%
echo.

REM Copy required files
echo Copying required files...

if exist "ZAVirtualPrinter\ZAVirtualPrinter.inf" (
    copy /Y "ZAVirtualPrinter\ZAVirtualPrinter.inf" "%OUTDIR%\" >nul
    echo [OK] Copied ZAVirtualPrinter.inf
) else (
    echo [WARNING] ZAVirtualPrinter.inf not found
)

if exist "ZAVirtualPrinter\%PLATFORM%\%CONFIGURATION%\ZAVirtualPrinter.dll" (
    copy /Y "ZAVirtualPrinter\%PLATFORM%\%CONFIGURATION%\ZAVirtualPrinter.dll" "%OUTDIR%\" >nul
    echo [OK] Copied ZAVirtualPrinter.dll
) else (
    echo [WARNING] ZAVirtualPrinter.dll not found (build ZAVirtualPrinter project first)
)

if exist "ZAVirtualPrinter\ZAVirtualPrinter.cat" (
    copy /Y "ZAVirtualPrinter\ZAVirtualPrinter.cat" "%OUTDIR%\" >nul
    echo [OK] Copied ZAVirtualPrinter.cat
) else (
    echo [WARNING] ZAVirtualPrinter.cat not found (driver may not be signed yet)
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Executable: %OUTDIR%\DriverInstaller.exe
echo.
echo To run:
echo   cd %OUTDIR%
echo   DriverInstaller.exe
echo.

pause
