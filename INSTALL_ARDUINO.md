# Installing Speedwire Library for Arduino IDE

## Quick Install (Windows)

1. **Download/Clone this repository**
   ```
   git clone https://github.com/lthquy/libspeedwire-arduino.git
   ```

2. **Copy to Arduino libraries folder**

   Open Command Prompt and run:
   ```cmd
   xcopy /E /I libspeedwire-arduino "%USERPROFILE%\Documents\Arduino\libraries\Speedwire"
   ```

3. **Restart Arduino IDE**
   - Close Arduino IDE completely
   - Open Arduino IDE
   - Verify: Sketch → Include Library → You should see "Speedwire"

4. **Open Example**
   - File → Examples → Speedwire → EmeterReader
   - Update WiFi credentials
   - Select your ESP32 board
   - Upload!

## Manual Installation

### Windows
1. Navigate to: `C:\Users\[YourUsername]\Documents\Arduino\libraries\`
2. Create folder: `Speedwire`
3. Copy all contents from this repository into that folder
4. Restart Arduino IDE

### macOS
1. Navigate to: `~/Documents/Arduino/libraries/`
2. Create folder: `Speedwire`
3. Copy all contents from this repository into that folder
4. Restart Arduino IDE

### Linux
1. Navigate to: `~/Arduino/libraries/`
2. Create folder: `Speedwire`
3. Copy all contents from this repository into that folder
4. Restart Arduino IDE

## Verify Installation

After restarting Arduino IDE:

1. Open: **Sketch → Include Library**
2. Look for "Speedwire" in the list
3. If you see it, installation is successful!

## Troubleshooting

### Library not showing in Arduino IDE

**Problem:** Library doesn't appear in Sketch → Include Library

**Solution:**
- Make sure folder name is `Speedwire` (not `libspeedwire-arduino`)
- Make sure `library.properties` file is in the root of the Speedwire folder
- Restart Arduino IDE completely

### Compilation errors "undefined reference"

**Problem:** Linker errors during compilation

**Solution:**
- Library must be installed in Arduino libraries folder, not compiled from download location
- Follow the installation steps above
- Verify the folder structure:
  ```
  Arduino/libraries/Speedwire/
    ├── library.properties
    ├── keywords.txt
    ├── README.md
    ├── src/
    │   ├── *.cpp files
    │   └── Speedwire.h
    ├── include/
    │   └── *.hpp files
    └── examples/
        ├── SpeedwireDiscovery/
        └── EmeterReader/
  ```

### ESP32 board not found

**Problem:** ESP32 board doesn't show in Tools → Board

**Solution:**
1. File → Preferences
2. Additional Boards Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools → Board → Boards Manager
4. Search "esp32" and install

## Using the Library

After successful installation:

```cpp
// In your .ino sketch
#include <WiFi.h>
#include <LocalHost.hpp>
#include <SpeedwireDiscovery.hpp>

using namespace libspeedwire;

void setup() {
  // Your code here
  LocalHost& localhost = LocalHost::getInstance();
  localhost.updateNetworkInfo();
}
```

## Need Help?

If you still have issues:
1. Check Arduino IDE output for specific error messages
2. Verify folder structure matches above
3. Try a simple example first (SpeedwireDiscovery)
4. Make sure ESP32 board package is installed
