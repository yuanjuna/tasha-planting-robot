@echo off
setlocal

cd /d "%~dp0"

set "PIO=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"

if not exist "%PIO%" (
    echo PlatformIO was not found:
    echo %PIO%
    echo.
    pause
    exit /b 1
)

echo Building and uploading pump_test_only firmware...
echo.
"%PIO%" run -e pump_test_only -t upload

echo.
if errorlevel 1 (
    echo Upload failed. Check CMSIS-DAP connection, board power, and USB cable.
) else (
    echo Upload OK. The pump/electric drill test is running.
)
echo.
pause
