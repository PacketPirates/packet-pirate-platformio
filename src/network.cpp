#include <Arduino.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "defs.h"

String g_deviceId;

void scanNetworks() {
  Serial.println("Scanning networks...");
  int nets = WiFi.scanNetworks();
  clearNetworks();
  if (nets != 0) {
    g_networksCount = nets;
    g_networksArray = new Network[nets];

    Serial.print(nets);
    Serial.println(" network(s) found");
    for (int i = 0; i < nets; i++) {
      g_networksArray[i].authMode = WiFi.encryptionType(i);
      g_networksArray[i].channel = WiFi.channel(i);
      g_networksArray[i].ssid = WiFi.SSID(i);
      g_networksArray[i].rssi = WiFi.RSSI(i);
      g_networksArray[i].id = i;
    }
  } else {
    Serial.println("No networks found.");
  }
  Serial.println("\n------------------------------------\n");

  if (g_networksArray)
  {
    for (int i = 0; i < g_networksCount; i++)  
    {
      
      Serial.print("network ");
      Serial.print(g_networksArray[i].id);
      Serial.print(": ");
      Serial.print(g_networksArray[i].ssid);
      Serial.print("(");
      Serial.print(g_networksArray[i].rssi);
      Serial.print(") ");
      Serial.println(g_networksArray[i].authMode);
    }
  }
}

void clearNetworks()
{
  Serial.println("Clearing nets");
  if (g_networksCount != -1 || g_networksArray)
  {
    delete[] g_networksArray;
    g_networksArray = nullptr;
    g_networksCount = -1;
  }
}

void registerDevice()
{
  if (!g_wifiConnected)
    connectWifi();

  String macAddr = WiFi.macAddress();

  String registrationURL = "";
  registrationURL.concat(WEBSERVER_ENDPOINT);
  registrationURL.concat("/register-device?mac=");
  registrationURL.concat(macAddr);

  g_deviceId = "";

  while (g_deviceId == "")
  {
    WiFiClient client;
    HTTPClient http;
      
    http.begin(client, registrationURL.c_str());
        
    // Send HTTP POST request
    int httpResponseCode = http.GET();
    
    String payload = "{}"; 
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.getString();
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    if (payload != "{}")
    {
      JSONVar parsed = JSONVar::parse(payload);
      g_deviceId = JSON.stringify(parsed["id"]);
      
      // Why do this after stringify you ask?
      // Because string comparisons are easy! And this JSON library is not!
      if (g_deviceId == "null")
      {
        Serial.println("Unable to get device id. Retrying in 2 seconds...");
        g_deviceId = "";
        delay(2000);
        continue;
      }

      Serial.print("Assigned device id: "); Serial.println(g_deviceId);
    }      
  }
}

void connectWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  g_wifiConnected = true;
}

void disconnectWifi()
{
  Serial.println("Disconnecting...");
  g_wifiConnected = false;
  WiFi.disconnect();
}

String httpGETRequest(String server)
{
  WiFiClient client;
  HTTPClient http;

  server.concat("?device-id=");
  server.concat(g_deviceId);
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, server.c_str());
  Serial.print("Making GET request to "); Serial.println(server);
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

String httpPOSTRequest(String server, const char* string, bool json)
{
  WiFiClient client;
  HTTPClient http;

  server.concat("?device-id=");
  server.concat(g_deviceId);
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, server.c_str());
  Serial.print("Making POST request to "); Serial.println(server);
  
  if (json)
    http.addHeader("Content-Type", "application/json");
  else
    http.addHeader("Content-Type", "text/plain");

  // Send HTTP POST request
  int httpResponseCode = http.POST(string);
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void uploadFile(const char* filepath)
{
  connectWifi();

  Serial.print("Uploading file \""); Serial.print(filepath); Serial.println("\"...");
  File f = LittleFS.open(filepath, "r");
  size_t fileSize = f.size();
  f.close();

  int numberOfChunks = (fileSize / FILE_UPLOAD_BUFFER_BYTES) + ((fileSize % FILE_UPLOAD_BUFFER_BYTES == 0) ? 0 : 1);

  Serial.print("File with size: "); Serial.print(fileSize); Serial.print(" and chunks: "); Serial.println(numberOfChunks);

  String uploadEndpoint = "";
  uploadEndpoint.concat(WEBSERVER_ENDPOINT);
  uploadEndpoint.concat("/upload-pcap");

  for (int i = 0; i < numberOfChunks; i++)
  {
    Serial.print("Starting chunk: "); Serial.println(i);
    httpFileUploadRequest(uploadEndpoint, filepath, i, (i == numberOfChunks - 1));
  }
}

String httpFileUploadRequest(String server, const char* filepath, int chunkOffset, bool finalChunk)
{
  int httpResponseCode;
  String payload = "{}"; 
 
  {
    WiFiClient client;
    HTTPClient http;

    String modifiedPath = filepath;
    modifiedPath.replace('/', '_');

    String serverStr = server;

    serverStr.concat("?device-id=");
    serverStr.concat(g_deviceId);
    serverStr.concat("&path=");
    serverStr.concat(modifiedPath);
    serverStr.concat("&offset=");
    serverStr.concat(chunkOffset);
    serverStr.concat("&final=");
    serverStr.concat(finalChunk);

    char buffer[FILE_UPLOAD_BUFFER_BYTES];
    // 0 our buffer so we can leave ending 0's
    for (int i = 0; i < FILE_UPLOAD_BUFFER_BYTES; i++)
      buffer[i] = 0;

    File f = LittleFS.open(filepath);
    if (!f)
    {
      Serial.println("ERROR: Failed to open LittleFS file for reading in upload!");
      return "{}";
    }

    f.seek(chunkOffset * FILE_UPLOAD_BUFFER_BYTES, SeekSet);
    int bytesRead = f.readBytes(buffer, FILE_UPLOAD_BUFFER_BYTES);
    if (bytesRead < 0) 
    {
      Serial.println("ERROR: Unable to read bytes from file in upload!");
      return "{}";
    }

    f.close();

    // There needs to be a more efficient way of doing this because
    // the memory usage this entails makes chunks so much smaller than they
    // could be
    Serial.println("Converting buffer...");
    uint8_t uintBuffer[FILE_UPLOAD_BUFFER_BYTES];
    for (int i = 0; i < FILE_UPLOAD_BUFFER_BYTES; i++)
      uintBuffer[i] = (uint8_t) buffer[i];

    http.begin(client, serverStr.c_str());
    Serial.print("Making POST request to "); Serial.println(serverStr);
    
    http.addHeader("Content-Type", "application/octet-stream");

    // Send HTTP POST request
    httpResponseCode = http.POST(uintBuffer, FILE_UPLOAD_BUFFER_BYTES);
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.getString();
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    // Free resources
    http.end();
  }

  if (httpResponseCode != 200)
    httpFileUploadRequest(server, filepath, chunkOffset, finalChunk);

  return payload;
}