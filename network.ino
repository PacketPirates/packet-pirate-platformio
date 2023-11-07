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
  bool uploadBtn = (bool) digitalRead(MODE_BTN);
  Serial.print("Btn: "); Serial.println(uploadBtn);

  // Each scan tick takes about 6 seconds to complete,
  // so let it run for about 2 minutes before trying to upload
  // automatically
  if ((g_tick - g_savedTick > 20 && g_previousMode == OperationMode::ScanMode) || uploadBtn)
    return OperationMode::UploadMode;
  else if (g_previousMode != OperationMode::ScanMode)
    return OperationMode::HoldMode;

  return OperationMode::ScanMode;
}