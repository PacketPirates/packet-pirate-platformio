#include <Arduino.h>
#include <WiFi.h>
#include <Arduino_JSON.h>

#include "defs.h"

void uploadNetworks() {
  // Connect to WiFi
  Serial.println("Uploading network data...");
  if (!g_wifiConnected)
    connectWifi();

  String json = "";

  for (int i = 0; i < g_networksCount; i++)
  {
    json.concat(i);
    json.concat(": {");
    json.concat("\"authmode\": "); json.concat(g_networksArray[i].authMode); json.concat(",");
    json.concat("\"id\": "); json.concat(g_networksArray[i].id); json.concat(",");
    json.concat("\"rssi\": "); json.concat(g_networksArray[i].rssi); json.concat(",");
    json.concat("\"ssid\": \""); json.concat(g_networksArray[i].ssid); json.concat("\",");
    json.concat("}, ");
  }

  String upPath = "";
  upPath.concat(WEBSERVER_ENDPOINT);
  upPath.concat("/upload");
  String serverReturn = httpPOSTRequest(upPath, json.c_str( ), true);
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

  String modePath = "";
  modePath.concat(WEBSERVER_ENDPOINT);
  modePath.concat("/unswitch");
  String serverReturn = httpPOSTRequest(modePath, id.c_str(), false);
}

JSONVar getModeFromWebserver()
{
  if (!g_wifiConnected)
    connectWifi();

  String modePath = "";
  modePath.concat(WEBSERVER_ENDPOINT);
  modePath.concat("/mode");
  String serverReturn = httpGETRequest(modePath);

  if (serverReturn != "{}")
    return JSON.parse(serverReturn);
  else
    return JSON.parse("\"error\": true");
}