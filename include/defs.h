#ifndef DEFS
#define DEFS

#include <Arduino.h>
#include <FirebaseESP32.h>
#include <WiFi.h>

#define IR_LED 15
#define IR_REC 12
#define MODE_BTN 27

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

void uploadNetworks();

void irScan();
void irBroadcast();
void trimIrScan();
void setIrPattern(int length, bool* pattern);

// Scanned network info
extern Network* g_networksArray;
extern int g_networksCount;

// For mode operation
extern unsigned int g_tick;
extern unsigned int g_savedTick;
extern OperationMode g_previousMode;

// Ir pattern variables
extern bool* g_irPattern;
extern int g_irLength;

// Firebase stuff
void firebaseSetup();

// Secrets... we should figure out how to do this without defines
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define API_KEY ""
#define DATABASE_URL ""

#endif