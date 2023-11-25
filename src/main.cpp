#include <Arduino.h>
#include <FirebaseESP32.h>
#include <WiFi.h>

#include "defs.h"

// Define globals specified in defs.h
// Scanned network info
Network* g_networksArray;
int g_networksCount;

// For mode operation
unsigned int g_tick;
unsigned int g_savedTick;
OperationMode g_previousMode;

// Ir pattern variables
bool* g_irPattern;
int g_irLength;

void setup() 
{
  // Scanned network info
  g_networksArray = nullptr;
  g_networksCount = -1;

  // For mode operation
  g_tick = 0;
  g_savedTick = 0;
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

  firebaseSetup();

  // Start by scanning networks
  scanNetworks();
}

void loop() 
{
  OperationMode mode = getMode();
  switch(mode)
  {
    case OperationMode::ScanMode:
      scanNetworks();
      break;
    case OperationMode::UploadMode:
      g_tick = 0;
      g_savedTick = g_tick;
      uploadNetworks();
      break;
    case OperationMode::IRScan:
      g_irLength = 5000;
      irScan();
      break;
    case OperationMode::IRBroadcast:
      irBroadcast();
      break;
    case OperationMode::HoldMode:
    default:
      delay(100);
      break;
  }

  g_previousMode = mode;
  g_tick++;
}

OperationMode getMode()
{
  bool uploadBtn = (bool) digitalRead(MODE_BTN); 

  bool irScan = (bool) digitalRead(32);
  bool irBroadcast = (bool) digitalRead(35);
  // TODO Remove debug print here
  // Serial.print("Scan: ");Serial.print(irScan);Serial.print("Broadcast: ");Serial.println(irBroadcast);

  if (irScan)
    return OperationMode::IRScan;
  else if (irBroadcast)
    return OperationMode::IRBroadcast;

  // Each scan tick takes about 6 seconds to complete,
  // so let it run for about 2 minutes before trying to upload
  // automatically    
  else if ((g_tick - g_savedTick > 20 && g_previousMode == OperationMode::ScanMode) || uploadBtn)
    return OperationMode::UploadMode;
  else if (g_previousMode != OperationMode::ScanMode)
    return OperationMode::HoldMode;

  // Default return for the time being
  return OperationMode::ScanMode;
}