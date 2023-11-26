#include <Arduino.h>
#include <FirebaseESP32.h>
#include <WiFi.h>

#include "defs.h"

FirebaseData g_firebaseData;
FirebaseAuth g_firebaseAuth;
FirebaseConfig g_firebaseConfig;

void firebaseSetup()
{
  g_firebaseConfig.host = DATABASE_URL;
  g_firebaseConfig.api_key = API_KEY;
}

void uploadNetworks() {
  // Connect to WiFi
  Serial.println("Uploading network data...");
  if (!g_wifiConnected)
    connectWifi();  

  // TODO remove
  //Firebase.begin(&g_firebaseConfig, &g_firebaseAuth);
  Serial.print("Auth status: "); Serial.println(Firebase.authenticated());  

  for (int i = 0; i < g_networksCount; i++) {
    Firebase.pushInt(g_firebaseData, "uploads/network", i + 1);
    String documentID = g_firebaseData.pushName();
    Serial.print("Uploading doc: "); Serial.println(documentID);
  
    Firebase.setInt(g_firebaseData, "networks/" + documentID + "/authMode", (int) g_networksArray[i].authMode);
    Firebase.setInt(g_firebaseData, "networks/" + documentID + "/id", g_networksArray[i].id);
    Firebase.setInt(g_firebaseData, "networks/" + documentID + "/rssi", g_networksArray[i].rssi);
    Firebase.setString(g_firebaseData, "networks/" + documentID + "/ssid", g_networksArray[i].ssid);
  }
}

void unswitchMode(OperationMode mode)
{
  String id;
  switch(mode) 
  {
    case OperationMode::IRScan:
      id = MODE_IR_SCAN;
      break;
    case OperationMode::IRBroadcast:
      id = MODE_BROADCAST;
      break;
    case OperationMode::ScanMode:
      id = MODE_SCAN;
      break;
    case OperationMode::TestMode:
      id = MODE_TEST;
      break;
    default:
      return;
  }

  Firebase.setBool(g_firebaseData, id, false);
}

bool fbGetIr()
{
  Serial.println("getting ir...");
  bool val = false;
  bool success = Firebase.getBool(g_firebaseData, MODE_IR_SCAN, &val);
  Serial.print("IR s: ");Serial.print(success);Serial.print("IR v: ");Serial.println(val);

  return (success) ? val : success;
}

bool fbGetBroadcast()
{
  bool val = false;
  bool success = Firebase.getBool(g_firebaseData, MODE_BROADCAST, &val);

  return (success) ? val : success;
}

bool fbGetRescan()
{
  bool val = false;
  bool success = Firebase.getBool(g_firebaseData, MODE_SCAN, &val);

  return (success) ? val : success;
}