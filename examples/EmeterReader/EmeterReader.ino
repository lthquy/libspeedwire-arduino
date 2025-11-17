/**
 * SMA Energy Meter Reader Example for ESP32
 *
 * This example demonstrates how to receive and parse energy meter data
 * from SMA Speedwire energy meters. It displays real-time power consumption,
 * grid feed-in, and energy counters.
 *
 * Hardware: ESP32 (any variant)
 *
 * Setup:
 * 1. Update WiFi credentials below
 * 2. Optionally configure NTP for timestamp synchronization
 * 3. Make sure your ESP32 is on the same network as your SMA energy meter
 * 4. Upload and open Serial Monitor at 115200 baud
 */

#include <WiFi.h>
#include <time.h>
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireSocketFactory.hpp>
#include <SpeedwireReceiveDispatcher.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireData2Packet.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#include <ObisData.hpp>

using namespace libspeedwire;

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// NTP server for time synchronization
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;      // Adjust for your timezone
const int daylightOffset_sec = 0;  // Adjust for daylight saving

// Speedwire objects
LocalHost* localhost = nullptr;
SpeedwireSocketFactory* factory = nullptr;
std::vector<SpeedwireSocket> sockets;
SpeedwireReceiveDispatcher* dispatcher = nullptr;

// Custom receiver for emeter packets
class EmeterReceiver : public EmeterPacketReceiverBase {
public:
  EmeterReceiver(LocalHost& host) : EmeterPacketReceiverBase(host) {}

  virtual void receive(SpeedwireHeader& speedwire_packet, struct sockaddr& src) override {
    // Parse emeter packet
    SpeedwireData2Packet data2_packet(speedwire_packet);
    SpeedwireEmeterProtocol emeter(data2_packet);

    Serial.println("\n=== Energy Meter Data ===");
    Serial.printf("Time: %s\n", LocalHost::unixEpochTimeInMsToString(
      LocalHost::getUnixEpochTimeInMs()).c_str());
    Serial.printf("Serial: %u\n", emeter.getSerialNumber());
    Serial.printf("Timestamp: %u\n", emeter.getTime());

    // Iterate through OBIS elements
    const void* obis = emeter.getFirstObisElement();

    Serial.println("\nOBIS Elements:");
    while (obis != NULL) {
      uint8_t channel = SpeedwireEmeterProtocol::getObisChannel(obis);
      uint8_t index = SpeedwireEmeterProtocol::getObisIndex(obis);
      uint8_t type = SpeedwireEmeterProtocol::getObisType(obis);
      uint8_t tariff = SpeedwireEmeterProtocol::getObisTariff(obis);

      // Get value (4 or 8 bytes depending on type)
      uint64_t value64 = 0;
      if (type == 4) {
        value64 = SpeedwireEmeterProtocol::getObisValue4(obis);
      } else if (type == 8) {
        value64 = SpeedwireEmeterProtocol::getObisValue8(obis);
      }

      // Print OBIS element
      Serial.printf("  Ch:%d Idx:%d Type:%d Tariff:%d => %llu (0x%llX)\n",
                    channel, index, type, tariff, value64, value64);

      // Identify common measurements
      // Channel 1 = Positive active power/energy
      // Channel 2 = Negative active power/energy
      // Index 4 = Power (W), Index 8 = Energy (Wh)
      if (channel == 1 && index == 4 && type == 4) {
        Serial.printf("    >> Positive Active Power Total: %.1f W\n", value64 / 10.0);
      }
      else if (channel == 2 && index == 4 && type == 4) {
        Serial.printf("    >> Negative Active Power Total: %.1f W\n", value64 / 10.0);
      }
      else if (channel == 1 && index == 8 && type == 8) {
        Serial.printf("    >> Positive Active Energy Total: %.3f kWh\n", value64 / 3600000.0);
      }
      else if (channel == 2 && index == 8 && type == 8) {
        Serial.printf("    >> Negative Active Energy Total: %.3f kWh\n", value64 / 3600000.0);
      }

      // Get next element
      obis = emeter.getNextObisElement(obis);
    }

    Serial.println("========================\n");
  }
};

EmeterReceiver* emeterReceiver = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nSMA Energy Meter Reader Example");
  Serial.println("=================================\n");

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
  Serial.println();

  // Configure NTP time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for NTP time sync...");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("Time synchronized");
  }

  // Initialize LocalHost singleton
  localhost = &LocalHost::getInstance();

  // Update network info after WiFi connection
  localhost->updateNetworkInfo();

  // Create socket factory and open multicast socket
  factory = SpeedwireSocketFactory::getInstance(*localhost,
    SpeedwireSocketFactory::SocketStrategy::ONE_SOCKET_FOR_EACH_INTERFACE);

  const std::vector<std::string>& localIPs = localhost->getLocalIPv4Addresses();

  if (localIPs.empty()) {
    Serial.println("ERROR: No local IP address found!");
    return;
  }

  Serial.printf("Opening socket on interface: %s\n", localIPs[0].c_str());
  sockets = factory->getRecvSockets(SpeedwireSocketFactory::SocketType::MULTICAST, localIPs);

  if (sockets.empty()) {
    Serial.println("ERROR: Failed to open socket!");
    return;
  }

  Serial.printf("Socket opened successfully (fd: %d)\n", sockets[0].getSocketFd());

  // Create dispatcher and register emeter receiver
  dispatcher = new SpeedwireReceiveDispatcher(*localhost);
  emeterReceiver = new EmeterReceiver(*localhost);
  dispatcher->registerReceiver(*emeterReceiver);

  Serial.println("\nListening for energy meter packets...\n");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  if (sockets.empty()) {
    Serial.println("No sockets available!");
    delay(1000);
    return;
  }

  // Dispatch incoming packets (timeout: 1000ms)
  int result = dispatcher->dispatch(sockets, 1000);

  if (result < 0) {
    Serial.println("Error receiving data");
    delay(1000);
  }
  // If result == 0, it's just a timeout, continue listening
}
