@echo off
REM Install Speedwire Library for Arduino IDE (Windows)
REM Run this script from the library root directory

echo ========================================
echo  Speedwire Library Installer
echo  for Arduino IDE (Windows)
echo ========================================
echo.

REM Check if we're in the right directory
if not exist "library.properties" (
    echo ERROR: library.properties not found!
    echo Please run this script from the library root directory.
    echo.
    pause
    exit /b 1
)

REM Set Arduino libraries path
set ARDUINO_LIBS=%USERPROFILE%\Documents\Arduino\libraries
set TARGET_DIR=%ARDUINO_LIBS%\Speedwire

echo Target directory: %TARGET_DIR%
echo.

REM Check if Arduino libraries folder exists
if not exist "%ARDUINO_LIBS%" (
    echo ERROR: Arduino libraries folder not found!
    echo Expected: %ARDUINO_LIBS%
    echo.
    echo Please make sure Arduino IDE is installed.
    pause
    exit /b 1
)

REM Check if library already exists
if exist "%TARGET_DIR%" (
    echo WARNING: Speedwire library already exists in Arduino libraries!
    echo.
    set /p OVERWRITE="Do you want to overwrite it? (Y/N): "
    if /i not "%OVERWRITE%"=="Y" (
        echo Installation cancelled.
        pause
        exit /b 0
    )
    echo Removing old version...
    rmdir /s /q "%TARGET_DIR%"
)

REM Create target directory
echo Creating target directory...
mkdir "%TARGET_DIR%"

REM Copy files
echo Copying library files...
xcopy /E /I /Q /Y . "%TARGET_DIR%"

if %ERRORLEVEL% neq 0 (
    echo.
    echo ERROR: Failed to copy files!
    pause
    exit /b 1
)

echo.
echo ========================================
echo  Installation Complete!
echo ========================================
echo.
echo Library installed to: %TARGET_DIR%
echo.
echo Next steps:
echo 1. Restart Arduino IDE
echo 2. Open: File ^> Examples ^> Speedwire ^> EmeterReader
echo 3. Update WiFi credentials in the sketch
echo 4. Select your ESP32 board in Tools ^> Board
echo 5. Upload and enjoy!
echo.
echo For more information, see INSTALL_ARDUINO.md
echo.
pause
