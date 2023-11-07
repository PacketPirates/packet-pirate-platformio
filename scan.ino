Network::Network(String ssid, wifi_auth_mode_t authMode, int rssi) {
  this->ssid = ssid;
  this->authMode = authMode;
  this->rssi = rssi;
}

void scanNetworks() {
  Serial.println("Scanning networks...");
  int nets = WiFi.scanNetworks();
  if (nets != 0) {
    Serial.print(nets);
    Serial.println(" network(s) found");
    for (int i = 0; i < nets; i++) {
      Serial.print("network ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print("(");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println(WiFi.encryptionType(i));
      delay(50);
    }
  } else {
    Serial.println("No networks found.");
  }
  Serial.println("\n------------------------------------\n");
  delay(5000);
}