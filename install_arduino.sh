#!/bin/bash
# Install Speedwire Library for Arduino IDE (Linux/macOS)
# Run this script from the library root directory

echo "========================================"
echo " Speedwire Library Installer"
echo " for Arduino IDE (Linux/macOS)"
echo "========================================"
echo

# Check if we're in the right directory
if [ ! -f "library.properties" ]; then
    echo "ERROR: library.properties not found!"
    echo "Please run this script from the library root directory."
    echo
    exit 1
fi

# Determine OS and set Arduino libraries path
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    ARDUINO_LIBS="$HOME/Documents/Arduino/libraries"
else
    # Linux
    ARDUINO_LIBS="$HOME/Arduino/libraries"
fi

TARGET_DIR="$ARDUINO_LIBS/Speedwire"

echo "Target directory: $TARGET_DIR"
echo

# Check if Arduino libraries folder exists
if [ ! -d "$ARDUINO_LIBS" ]; then
    echo "ERROR: Arduino libraries folder not found!"
    echo "Expected: $ARDUINO_LIBS"
    echo
    echo "Please make sure Arduino IDE is installed."
    exit 1
fi

# Check if library already exists
if [ -d "$TARGET_DIR" ]; then
    echo "WARNING: Speedwire library already exists in Arduino libraries!"
    echo
    read -p "Do you want to overwrite it? (y/n): " OVERWRITE
    if [[ ! "$OVERWRITE" =~ ^[Yy]$ ]]; then
        echo "Installation cancelled."
        exit 0
    fi
    echo "Removing old version..."
    rm -rf "$TARGET_DIR"
fi

# Create target directory
echo "Creating target directory..."
mkdir -p "$TARGET_DIR"

# Copy files
echo "Copying library files..."
cp -r . "$TARGET_DIR/"

if [ $? -ne 0 ]; then
    echo
    echo "ERROR: Failed to copy files!"
    exit 1
fi

# Set execute permissions for this script in the target
chmod +x "$TARGET_DIR/install_arduino.sh"

echo
echo "========================================"
echo " Installation Complete!"
echo "========================================"
echo
echo "Library installed to: $TARGET_DIR"
echo
echo "Next steps:"
echo "1. Restart Arduino IDE"
echo "2. Open: File > Examples > Speedwire > EmeterReader"
echo "3. Update WiFi credentials in the sketch"
echo "4. Select your ESP32 board in Tools > Board"
echo "5. Upload and enjoy!"
echo
echo "For more information, see INSTALL_ARDUINO.md"
echo
