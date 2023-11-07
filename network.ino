void scanNetworks() {
  Serial.println("Scanning networks...");
  int nets = WiFi.scanNetworks();
  clearNetworks();
  if (nets != 0) {
    g_networksCount = nets;

    Serial.print(nets);
    Serial.println(" network(s) found");
    for (int i = 0; i < nets; i++) {
      g_networksArray = new Network[nets];
      g_networksArray[i].authMode = WiFi.encryptionType(i);
      g_networksArray[i].ssid = WiFi.SSID(i);
      g_networksArray[i].rssi = WiFi.RSSI(i);
      g_networksArray[i].id = i;

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

void clearNetworks()
{
  if (g_networksCount != -1 || g_networksArray)
  {
    delete[] g_networksArray;
    g_networksArray = nullptr;
    g_networksCount = -1;
  }
}