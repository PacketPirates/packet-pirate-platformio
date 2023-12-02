#include <Arduino.h>
#include <Arduino_Json.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "defs.h"

// Define globals specified in defs.h
// Scanned network info
Network* g_networksArray;
int g_networksCount;
bool g_wifiConnected;

// For mode operation
unsigned int g_tick;
OperationMode g_previousMode;

bool g_littleFS;

// Ir pattern variables
bool* g_irPattern;
int g_irLength;

void clearStorage()
{
  // Recursively delete all files and directories
  File root = LittleFS.open("/");
  if (!root.isDirectory()) {
    Serial.println("Root directory is not a directory");
    return;
  }

  File child;
  while (child = root.openNextFile()) {
    Serial.print("Clearing child: "); Serial.println(child.name());
    String childName = String("/") + String(child.name());
    if (child.isDirectory()) {
      clearDirectory(childName.c_str()); // Recursively clear subdirectories
    } else {
      child.close();
      LittleFS.remove(childName.c_str()); // Delete file
    }
  }
  root.close();
}

void clearDirectory(const String& path) {
  Serial.print("Clearing dir with path: "); Serial.println(path);
  File dir = LittleFS.open(path);
  if (!dir.isDirectory()) {
    Serial.print("Failed to open directory: ");
    Serial.println(path);
    return;
  }

  File child;
  while (child = dir.openNextFile()) {
    String childName = String("/") + String(child.name());
    if (child.isDirectory()) {
      clearDirectory(childName); // Recursively clear subdirectories
    } else {
      child.close();
      LittleFS.remove(childName); // Delete file
    }
    child.close();
  }
  dir.close();

  // Finally, delete the directory itself
  LittleFS.rmdir(path);
}

void setup() 
{
  // Scanned network info
  g_networksArray = nullptr;
  g_networksCount = -1;
  g_wifiConnected = false;

  // For mode operation
  g_tick = 0;
  g_previousMode;

  g_littleFS = true;

  // Ir pattern variables
  g_irPattern = nullptr;
  g_irLength = 0;

  pinMode(IR_LED, OUTPUT);
  pinMode(IR_REC, INPUT);
  pinMode(MODE_BTN, INPUT);
  
  Serial.begin(115200);

  delay(1000);
  Serial.println("Starting setup...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  Serial.println("Mounting new LittleFS partition...");
  if (!LittleFS.begin(true, "/storage"))
  {
    Serial.println("LittleFS mount failed! Starting in crippled mode");
    g_littleFS = false;
  }

  Serial.println("Clearing filesystem...");
  clearStorage();

  // Start by scanning networks
  scanNetworks();
  registerDevice();
  uploadNetworks();
}

void loop() 
{
  OperationMode mode = getMode();
  switch(mode)
  {
    case OperationMode::ScanMode:
      if (g_wifiConnected)
        disconnectWifi();
      scanNetworks();
      // Not sure if this delay is necessary but better safe than sorry re: fb rate limiting
      delay(5000);
      uploadNetworks();
      break;
    case OperationMode::IRScan:
      g_irLength = IR_DEFAULT_LENGTH;
      irScan();
      break;
    case OperationMode::IRBroadcast:
      irBroadcast();
      break;
    case OperationMode::TestMode:
      runTest();
      break;
    case OperationMode::HoldMode:
    default:
      delay(100);
  }

  Serial.println("Delaying for 6 seconds...");
  delay(6000);
  Serial.println("Delay complete");

  if (!g_wifiConnected)
    connectWifi();

  Serial.print("Unswitching mode: ");Serial.println(mode);
  unswitchMode(mode);

  g_previousMode = mode;
  g_tick++;
}

OperationMode getMode()
{
  bool uploadBtn = (bool) digitalRead(MODE_BTN); 

  if (!g_wifiConnected)
    connectWifi();

  Serial.println("getting from firebase db");
  JSONVar serverResults = getModeFromWebserver();
  bool reqWorking = true;
  for (int i = 0; i < serverResults.keys().length(); i++)
  {
    if (JSONVar::stringify(serverResults[serverResults.keys()[i]]) == "error")
      reqWorking = false;
  }
  Serial.print("REST results: "); Serial.println(serverResults);
  
  bool irScan = reqWorking && serverResults["ir"];
  bool irBroadcast = reqWorking && serverResults["broadcast"];
  bool rescan = reqWorking && serverResults["rescan"];
  bool test = reqWorking && serverResults["test"];

  // TODO Remove debug print here
  // Serial.print("Scan: ");Serial.print(irScan);Serial.print("Broadcast: ");Serial.println(irBroadcast);

  if (rescan)
    return OperationMode::ScanMode;
  else if (irScan)
    return OperationMode::IRScan;
  else if (irBroadcast)
    return OperationMode::IRBroadcast;  
  else if (test)
    return OperationMode::TestMode;  

  return OperationMode::HoldMode;
}