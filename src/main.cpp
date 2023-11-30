#include <Arduino.h>
#include <Arduino_Json.h>
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

// Ir pattern variables
bool* g_irPattern;
int g_irLength;

void setup() 
{
  // Scanned network info
  g_networksArray = nullptr;
  g_networksCount = -1;
  g_wifiConnected = false;

  // For mode operation
  g_tick = 0;
  g_previousMode;

  // Ir pattern variables
  g_irPattern = nullptr;
  g_irLength = 0;

  pinMode(IR_LED, OUTPUT);
  pinMode(IR_REC, INPUT);
  pinMode(MODE_BTN, INPUT);
  
  // TODO REMOVE WHEN CLOUD IR ADDED
  // SCAN
  pinMode(32, INPUT);
  // BROADCAST
  pinMode(35, INPUT);
  
  Serial.begin(115200);

  delay(1000);
  Serial.println("Starting setup...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

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

  // TODO Remove debug print here
  // Serial.print("Scan: ");Serial.print(irScan);Serial.print("Broadcast: ");Serial.println(irBroadcast);

  if (irScan)
    return OperationMode::IRScan;
  else if (irBroadcast)
    return OperationMode::IRBroadcast;
  else if (rescan)
    return OperationMode::ScanMode;

  return OperationMode::HoldMode;
}