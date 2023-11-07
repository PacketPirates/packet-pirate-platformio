#include <WiFi.h>

const int IR_LED = 15;
const int IR_REC = 12;

// Function/struct declarations
struct Network
{
  // Fill with vulnerabilities from our future attack
  enum Vulnerabilities
  {
    Placeholder,
    Temp
  };

  // Identifying info
  int id;
  String ssid;
  wifi_auth_mode_t authMode;
  int rssi;

  // List to be uploaded to db
  int vulnerabilitiesCount;
  Vulnerabilities* vulnerabilities;
};

enum OperationMode
{
  ScanMode,
  UploadMode,
  TestMode,
  IRScan,
  IRBroadcast
};

OperationMode getMode();
void scanNetworks();
void clearNetworks();
void uploadNetworks();

Network* g_networksArray = nullptr;
int g_networksCount = -1;
int g_tick = 0;
int g_savedTick = 0;

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
  g_tick++;
}