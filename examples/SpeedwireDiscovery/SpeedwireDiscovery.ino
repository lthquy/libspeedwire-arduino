/**
 * SMA Speedwire Device Discovery Example for ESP32
 *
 * This example demonstrates how to discover SMA Speedwire devices
 * (solar inverters and energy meters) on your local network.
 *
 * Hardware: ESP32 (any variant)
 *
 * Setup:
 * 1. Update WiFi credentials below
 * 2. Make sure your ESP32 is on the same network as your SMA devices
 * 3. Upload and open Serial Monitor at 115200 baud
 */

#include <WiFi.h>
#include <LocalHost.hpp>
#include <SpeedwireDiscovery.hpp>
#include <SpeedwireDevice.hpp>

using namespace libspeedwire;

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Speedwire objects
SpeedwireDiscovery* discovery = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nSMA Speedwire Discovery Example");
  Serial.println("================================\n");

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();

  // Initialize LocalHost singleton
  LocalHost& localhost = LocalHost::getInstance();

  // Update network info after WiFi connection
  localhost.updateNetworkInfo();

  // Create discovery instance
  discovery = new SpeedwireDiscovery(localhost);

  Serial.println("Starting device discovery...\n");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  if (discovery == nullptr) {
    Serial.println("Discovery not initialized!");
    delay(1000);
    return;
  }

  // Perform discovery
  Serial.println("Discovering Speedwire devices...");
  int numDevices = discovery->discoverDevices();

  if (numDevices <= 0) {
    Serial.println("No Speedwire devices found.");
  } else {
    const std::vector<SpeedwireDevice>& devices = discovery->getDevices();
    Serial.printf("Found %d device(s):\n\n", devices.size());

    for (const auto& device : devices) {
      Serial.println("--------------------------------");
      Serial.printf("Device: %s\n", device.deviceAddress.toString().c_str());
      Serial.printf("  Serial: %u\n", device.serialNumber);
      Serial.printf("  SusyID: %u\n", device.susyID);
      Serial.printf("  Peer: %s\n", device.peerAddress.toString().c_str());

      // Decode device class if available
      Serial.printf("  Device Class: 0x%08X\n", device.deviceClass);
      Serial.printf("  Device Type: 0x%08X\n", device.deviceType);
      Serial.println();
    }
  }

  // Wait 30 seconds before next discovery
  Serial.println("Waiting 30 seconds before next scan...\n");
  delay(30000);
}
