@echo off
setlocal EnableDelayedExpansion

set INPUT=x64\Installer
set OUTPUT=Installer\GAMEDIRECTORY
set ISCC="C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
set ISS_SCRIPT=Installer\InstallScript.iss

echo Cleaning old output...
if exist "%OUTPUT%" rmdir /s /q "%OUTPUT%"
mkdir "%OUTPUT%"

echo Copying required files...
xcopy "%INPUT%" "%OUTPUT%" /E /I /Y
copy "icon.ico" %OUTPUT%
copy "gameConfig.json" %OUTPUT%

xcopy "Resources" "%OUTPUT%\Resources" /E /I /Y
xcopy "System" "%OUTPUT%\System" /E /I /Y

echo Removing redundant files...

del /s /q "%OUTPUT%\*.pdb" 2>nul
del /s /q "%OUTPUT%\*.exp" 2>nul

REM Remove all json except gameConfig.json
for /r "%OUTPUT%" %%f in (*.json) do (
    if /I not "%%~nxf"=="gameConfig.json" del "%%f"
)

REM Remove nova-editor executable if present
if exist "%OUTPUT%\nova-editor.exe" (
    del "%OUTPUT%\nova-editor.exe"
)

echo Checking for Inno Setup...

set "ISCC_CMD="

REM Try PATH first
where ISCC >nul 2>nul
if %errorlevel% equ 0 (
    set "ISCC_CMD=ISCC"
)

REM If not found in PATH, try default install location
if "%ISCC_CMD%"=="" (
    if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
        set "ISCC_CMD=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
    )
)

REM Final check
if "%ISCC_CMD%"=="" (
    echo ERROR: Inno Setup compiler not found.
    exit /b 1
)

echo Running installer compiler...
"%ISCC_CMD%" "%ISS_SCRIPT%"

if %errorlevel% neq 0 (
    echo ERROR: Installer build failed.
    exit /b 1
)

echo Installer successfully created.