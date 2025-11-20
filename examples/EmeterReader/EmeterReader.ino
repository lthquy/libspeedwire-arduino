/*
 * SMA Energy Meter (Emeter) Reader for ESP32
 *
 * This example demonstrates how to read energy values from
 * SMA Energy Meters using the Speedwire protocol.
 *
 * The SMA Energy Meter broadcasts energy measurements including:
 * - Total power consumption/production
 * - Power per phase (L1, L2, L3)
 * - Voltage and current per phase
 * - Energy counters
 *
 * Hardware: ESP32 development board
 */

#include <WiFi.h>
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#include <ObisData.hpp>

using namespace libspeedwire;

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

SpeedwireSocket* speedwireSocket = nullptr;

void printEmeterData(SpeedwireEmeterProtocol& emeter) {
  Serial.println("\n--- SMA Energy Meter Data ---");

  Serial.print("Serial Number: ");
  Serial.println(emeter.getSerialNumber());

  Serial.print("Timestamp: ");
  Serial.println(emeter.getTime());

  // Get total power values (positive = consumption, negative = production)
  Wire wire;
  for (int i = 0; i < emeter.getNumberOfWireValues(); i++) {
    if (emeter.getWireValue(i, wire)) {
      Serial.print("Wire ");
      Serial.print(i);
      Serial.print(": ");

      // Decode OBIS channel and index
      uint8_t channel = ObisData::getChannel(wire.channel);
      uint8_t index = ObisData::getIndex(wire.channel);
      uint8_t type = ObisData::getType(wire.channel);

      Serial.print("Channel=");
      Serial.print(channel);
      Serial.print(", Index=");
      Serial.print(index);
      Serial.print(", Type=");
      Serial.print(type);
      Serial.print(", Value=");
      Serial.print(wire.value);
      Serial.println();
    }
  }

  // Parse specific OBIS values
  // OBIS codes for common measurements:
  // 1:1.4.0  - Total active power (W)
  // 1:1.8.0  - Total positive active energy (Wh)
  // 1:2.8.0  - Total negative active energy (Wh)
  // 1:21.4.0 - L1 active power (W)
  // 1:41.4.0 - L2 active power (W)
  // 1:61.4.0 - L3 active power (W)

  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== SMA Energy Meter Reader ===\n");

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Initialize Speedwire
  LocalHost& localhost = LocalHost::getInstance();
  speedwireSocket = new SpeedwireSocket(localhost);

  String localIP = WiFi.localIP().toString();
  int result = speedwireSocket->openSocket(localIP.c_str(), true);

  if (result < 0) {
    Serial.println("ERROR: Failed to open socket!");
    return;
  }

  Serial.println("Ready! Listening for SMA Energy Meter packets...\n");
}

void loop() {
  if (speedwireSocket == nullptr) {
    delay(1000);
    return;
  }

  uint8_t buffer[2048];
  struct sockaddr_in src;

  int nbytes = speedwireSocket->recvfrom(buffer, sizeof(buffer), src);

  if (nbytes > 0) {
    SpeedwireHeader header(buffer, nbytes);

    // Check if this is an emeter packet
    if (header.checkHeader() == 0 &&
        SpeedwireHeader::isEmeterProtocolID(header.getProtocolID())) {

      SpeedwireEmeterProtocol emeter(buffer, nbytes);
      printEmeterData(emeter);
    }
  }

  delay(100);
}
