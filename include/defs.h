#ifndef DEFS
#define DEFS

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFi.h>

// Function/struct declarations
struct Network
{
  // Fill with vulnerabilities from our future attack
  enum Vulnerabilities
  {
    Placeholder,
    Temp
  };

  // Identifying info
  int id;
  String ssid;
  wifi_auth_mode_t authMode;
  int rssi;
  int channel;

  // List to be uploaded to db
  int vulnerabilitiesCount;
  Vulnerabilities* vulnerabilities;
};

enum OperationMode
{
  ScanMode,
  UploadMode,
  TestMode,
  HoldMode,
  IRScan,
  IRBroadcast
};

OperationMode getMode();

void scanNetworks();
void clearNetworks();

// Automatically connects to defined SSID
void connectWifi();
void disconnectWifi();

void registerDevice();
void uploadNetworks();
void unswitchMode(OperationMode mode);
JSONVar getModeFromWebserver();

String httpGETRequest(String server);
String httpPOSTRequest(String server, const char* string, bool json);
String httpFileUploadRequest(String server, const char* filepath, int chunkOffset, bool finalChunk);

void uploadFile(const char* filepath);

void irScan();
void irBroadcast();
void trimIrScan();
void setIrPattern(int length, bool* pattern);

void runTest();
void uploadTestResult(int networkId, String testname, bool result);
// Add test functions here
void tempTest(int networkId);

// Scanned network info
extern Network* g_networksArray;
extern int g_networksCount;

extern bool g_wifiConnected;

extern String g_deviceId;

// For mode operation
extern unsigned int g_tick;
extern OperationMode g_previousMode;

extern bool g_littleFS;

// Ir pattern variables
extern bool* g_irPattern;
extern int g_irLength;

// Secrets... we should figure out how to do this without defines
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define WEBSERVER_ENDPOINT ""

// Pinouts
#define IR_LED 15
#define IR_REC 12
#define MODE_BTN 27

#define IR_DEFAULT_LENGTH 5000

#define MODE_IR_SCAN "modes/ir" 
#define MODE_BROADCAST "modes/broadcast"
#define MODE_SCAN "modes/scan"
#define MODE_TEST "modes/test"

#define FILE_UPLOAD_BUFFER_BYTES 2048

#endif