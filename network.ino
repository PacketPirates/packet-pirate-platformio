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
      g_networksArray[i].ssid = WiFi.SSID(i);
      g_networksArray[i].rssi = WiFi.RSSI(i);
      g_networksArray[i].id = i;
      delay(50);
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

OperationMode getMode()
{
  return OperationMode::ScanMode;
}