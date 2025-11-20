/*
 * Basic Speedwire Receiver Example for ESP32
 *
 * This example demonstrates how to:
 * - Connect ESP32 to WiFi
 * - Initialize the Speedwire library
 * - Listen for Speedwire packets from SMA devices
 * - Parse and display received data
 *
 * Hardware: ESP32 development board
 *
 * Instructions:
 * 1. Update WiFi credentials below
 * 2. Upload to your ESP32
 * 3. Open Serial Monitor at 115200 baud
 * 4. The ESP32 will display packets from SMA devices on your network
 */

#include <WiFi.h>
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <Logger.hpp>

using namespace libspeedwire;

// WiFi credentials - UPDATE THESE!
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Speedwire socket
SpeedwireSocket* speedwireSocket = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=== SMA Speedwire Receiver for ESP32 ===\n");

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
  Serial.print("Subnet mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.println();

  // Initialize LocalHost (Speedwire library)
  LocalHost& localhost = LocalHost::getInstance();
  Serial.print("Hostname: ");
  Serial.println(localhost.getHostname().c_str());

  // Display local interfaces
  const auto& ipAddresses = localhost.getLocalIPAddresses();
  Serial.println("Local IP addresses:");
  for (const auto& ip : ipAddresses) {
    Serial.print("  - ");
    Serial.println(ip.c_str());
  }
  Serial.println();

  // Create and open Speedwire socket
  speedwireSocket = new SpeedwireSocket(localhost);

  // Get local interface IP address
  String localIP = WiFi.localIP().toString();

  // Open socket for multicast reception on port 9522
  int result = speedwireSocket->openSocket(localIP.c_str(), true);

  if (result < 0) {
    Serial.println("ERROR: Failed to open Speedwire socket!");
    Serial.println("Make sure your SMA device is on the same network.");
    return;
  }

  Serial.println("Speedwire socket opened successfully!");
  Serial.print("Listening on port ");
  Serial.println(SpeedwireSocket::speedwire_port_9522);
  Serial.println("\nWaiting for Speedwire packets from SMA devices...\n");
}

void loop() {
  if (speedwireSocket == nullptr) {
    delay(1000);
    return;
  }

  // Buffer for receiving packets
  uint8_t buffer[2048];
  struct sockaddr_in src;

  // Try to receive a packet (non-blocking or with timeout)
  int nbytes = speedwireSocket->recvfrom(buffer, sizeof(buffer), src);

  if (nbytes > 0) {
    Serial.println("========================================");
    Serial.print("Received ");
    Serial.print(nbytes);
    Serial.print(" bytes from ");
    Serial.println(inet_ntoa(src.sin_addr));

    // Parse the Speedwire header
    SpeedwireHeader header(buffer, nbytes);

    if (header.checkHeader() == 0) {
      Serial.println("Valid Speedwire packet!");
      Serial.print("  Group: 0x");
      Serial.println(header.getGroup(), HEX);
      Serial.print("  Length: ");
      Serial.println(header.getLength());
      Serial.print("  Protocol: 0x");
      Serial.println(header.getProtocolID(), HEX);

      // Check if it's an emeter packet
      if (SpeedwireHeader::isEmeterProtocolID(header.getProtocolID())) {
        Serial.println("  Type: Energy Meter (Emeter)");

        SpeedwireEmeterProtocol emeter(buffer, nbytes);
        uint32_t serial = emeter.getSerialNumber();
        uint32_t time = emeter.getTime();

        Serial.print("  Serial Number: ");
        Serial.println(serial);
        Serial.print("  Timestamp: ");
        Serial.println(time);

        // You can parse OBIS values here
        // See SpeedwireEmeterProtocol class for methods

      } else if (SpeedwireHeader::isInverterProtocolID(header.getProtocolID())) {
        Serial.println("  Type: Inverter");

        SpeedwireInverterProtocol inverter(buffer, nbytes);
        // Parse inverter data here
        // See SpeedwireInverterProtocol class for methods
      }

      Serial.println();
    } else {
      Serial.println("Invalid Speedwire header - skipping packet");
    }

  }

  // Small delay to prevent overwhelming the CPU
  delay(10);
}
