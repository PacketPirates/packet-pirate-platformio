#include <Arduino.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "defs.h"

String g_deviceId;

//SocketIOclient socketIO;

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

/*void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length)
{
  Serial.print("Socket received message of type "); Serial.print(socketIOmessageType_t(type)); Serial.println(" !");
  for (int i = 0; i < length; i++)
    Serial.print(payload[i]);

  Serial.println("\nMessage complete.");
}*/

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

  WiFiClient tcpClient;
  if (!tcpClient.connected())
  {
    if (!tcpClient.connect(TCP_ENDOINT, TCP_PORT))
    {
      Serial.println("ERROR: Cannot connect to TCP endpoint!");
      return;
    }
  }

  for (int i = 0; i < numberOfChunks; i++)
  {
    Serial.print("Starting chunk: "); Serial.println(i);
    httpFileUploadRequest(&tcpClient, uploadEndpoint, filepath, i, (i == numberOfChunks - 1));
    delay(100);
  }
}

String httpFileUploadRequest(WiFiClient* client, String server, const char* filepath, int chunkOffset, bool finalChunk)
{
  //socketIO.loop();
  String modifiedPath = filepath;
  modifiedPath.replace('/', '_');

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
  Serial.println("Creating buffer...");
  int deviceLen = g_deviceId.length();
  int pathLen = modifiedPath.length();
  
  int totalBufferSize = FILE_UPLOAD_BUFFER_BYTES + deviceLen + pathLen + 13;
  char intBuffer[sizeof(int)];
  
  for (int i = 0; i < sizeof(int); i++)
  {
    intBuffer[i] = totalBufferSize / sizeof(int);
    if (i == sizeof(int) - 1)
      intBuffer[i] += totalBufferSize % sizeof(int);
  }

  uint8_t* uintBuffer = new uint8_t[totalBufferSize];

  uintBuffer[0] = (uint8_t) deviceLen;
  for (int i = 0; i < deviceLen; i++)
    uintBuffer[i + 1] = g_deviceId.charAt(i);
  
  if (pathLen > 255)
  {
    uintBuffer[deviceLen + 1] = (uint8_t) pathLen - 255;
    uintBuffer[deviceLen + 2] = 255;
  }
  else
  {
    uintBuffer[deviceLen + 1] = 0;
    uintBuffer[deviceLen + 2] = (uint8_t) pathLen;
  }

  for (int i = 0; i < pathLen; i++)
    uintBuffer[deviceLen + 3 + i] = modifiedPath.charAt(i);

  // 8 bytes for current chunk
  uintBuffer[deviceLen + 3 + pathLen + 1 ] = chunkOffset / 8;
  uintBuffer[deviceLen + 3 + pathLen + 2] = chunkOffset / 8;
  uintBuffer[deviceLen + 3 + pathLen + 3] = chunkOffset / 8;
  uintBuffer[deviceLen + 3 + pathLen + 4] = chunkOffset / 8;
  uintBuffer[deviceLen + 3 + pathLen + 5] = chunkOffset / 8;
  uintBuffer[deviceLen + 3 + pathLen + 6] = chunkOffset / 8;
  uintBuffer[deviceLen + 3 + pathLen + 7] = chunkOffset / 8;
  uintBuffer[deviceLen + 3 + pathLen + 8] = (chunkOffset / 8) + chunkOffset % 8;

  uintBuffer[deviceLen + 3 + pathLen + 9] = finalChunk;
  

  for (int i = 0; i < FILE_UPLOAD_BUFFER_BYTES; i++)
    uintBuffer[i + deviceLen + pathLen + 12] = (uint8_t) buffer[i];

  uintBuffer[totalBufferSize - 2] = 69;
  uintBuffer[totalBufferSize - 1] = '\n';
  
  Serial.print("Sending buffer of size "); Serial.print(totalBufferSize); Serial.println("...");
  client->write(uintBuffer, totalBufferSize);
  delete[] uintBuffer;

  return "{success}";
}