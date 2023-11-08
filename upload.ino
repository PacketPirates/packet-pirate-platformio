#include <FirebaseESP32.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define API_KEY ""
#define DATABASE_URL "" 

FirebaseData firebaseData

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

  Firebase.begin(DATABASE_URL, API_KEY);

  for(int i = 0; i < g_networksCount; i++){
    String documentID = Firebase.pushInt(firebaseData, "uploads/network", i + 1);

    Firebase.setString(firebaseData, documentID + "/authMode", g_networksArray[i].authMode);
    Firebase.setInt(firebaseData, documentID + "/id", g_networksArray[i].id);
    Firebase.setInt(firebaseData, documentID + "/rssi", g_networksArray[i].rssi);
    Firebase.setString(firebaseData, documentID + "/ssid", g_networksArray[i].ssid);
  }
  Firebase.end();

}

