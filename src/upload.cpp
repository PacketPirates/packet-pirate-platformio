#include <Arduino.h>
#include <FirebaseESP32.h>
#include <WiFi.h>

#include "defs.h"

FirebaseData firebaseData;
FirebaseAuth firebaseAuth;
FirebaseConfig firebaseConfig;

void firebaseSetup()
{
  firebaseConfig.host = DATABASE_URL;
  firebaseConfig.api_key = API_KEY;

}

void uploadNetworks() {
  // Connect to WiFi
  Serial.println("Uploading network data...");
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

  if (!Firebase.signUp(&firebaseConfig, &firebaseAuth, "", ""))
    return;

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Serial.print("Auth status: "); Serial.println(Firebase.authenticated());  

  for (int i = 0; i < g_networksCount; i++) {
    Firebase.pushInt(firebaseData, "uploads/network", i + 1);
    String documentID = firebaseData.pushName();
    Serial.print("Uploading doc: "); Serial.println(documentID);
  
    Firebase.setInt(firebaseData, documentID + "/authMode", (int) g_networksArray[i].authMode);
    Firebase.setInt(firebaseData, documentID + "/id", g_networksArray[i].id);
    Firebase.setInt(firebaseData, documentID + "/rssi", g_networksArray[i].rssi);
    Firebase.setString(firebaseData, documentID + "/ssid", g_networksArray[i].ssid);
  }

  Serial.println("Disconnecting...");
  WiFi.disconnect();
}

