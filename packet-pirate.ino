#include <WiFi.h>

const int IR_LED = 15;
const int IR_REC = 12;

// Function/struct declarations
struct Network
{
  int id;
  String ssid;
  wifi_auth_mode_t authMode;
  int rssi;
};

enum OperationMode
{
  ScanMode,
  UploadMode
};

OperationMode getMode();
void scanNetworks();
void clearNetworks();
void uploadNetworks();

Network* g_networksArray = nullptr;
int g_networksCount = -1;

void setup() 
{
  pinMode(IR_LED, OUTPUT);
  pinMode(IR_REC, INPUT);
  
  Serial.begin(115200);

  delay(1000);
  Serial.println("Starting setup...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);
}

void loop() 
{
  switch(getMode())
  {
    case OperationMode::ScanMode:
      scanNetworks();
      break;
    case OperationMode::UploadMode:
      uploadNetworks();
  }
}