@echo off
setlocal

:: Get the directory where this batch file is located
set "BATCH_DIR=%~dp0"

:: Set the executable path relative to batch file location (modify this)
set "EXE_REL_PATH=build\raylib_game\bin\x64\Release\raylib_game.exe"
if not defined EXE_REL_PATH (
    echo Usage: %0 "relative_path_to_executable.exe"
    echo Example: %0 "Game\MyGame.exe"
    exit /b 1
)

:: Build absolute path from relative path
set "EXE_PATH=%BATCH_DIR%%EXE_REL_PATH%"

:: Validate file exists
if not exist "%EXE_PATH%" (
    echo Error: File not found: %EXE_PATH%
    exit /b 1
)

:: Get absolute path and convert to lowercase
for /f "delims=" %%i in ('powershell -noprofile -command "Resolve-Path '%EXE_PATH%' | ForEach-Object {$_.Path.ToLower()}"') do set "EXE_PATH=%%i"

:: Create registry key with high performance preference
reg add "HKCU\SOFTWARE\Microsoft\DirectX\UserGpuPreferences" /v "%EXE_PATH%" /t REG_SZ /d "GpuPreference=2;" /f >nul 2>&1

if %errorlevel%==0 (
    echo Success: High performance GPU preference set for:
    echo %EXE_PATH%
) else (
    echo Error: Failed to set GPU preference
    pause
    exit /b 1
)

pause