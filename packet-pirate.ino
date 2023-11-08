#include <WiFi.h>

const int IR_LED = 15;
const int IR_REC = 12;
const int MODE_BTN = 27;

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
  HoldMode,
  IRScan,
  IRBroadcast
};

OperationMode getMode();

void scanNetworks();
void clearNetworks();

void uploadNetworks();

void irScan();
void irBroadcast();
void setIrPattern(int length, bool* pattern);

// Scanned network info
Network* g_networksArray = nullptr;
int g_networksCount = -1;

// For mode operation
unsigned int g_tick = 0;
unsigned int g_savedTick = 0;
OperationMode g_previousMode;

// Ir pattern variables
bool* g_irPattern = nullptr;
int g_irLength = 0;
int g_irStart = -1;

void setup() 
{
  pinMode(IR_LED, OUTPUT);
  pinMode(IR_REC, INPUT);
  pinMode(MODE_BTN, INPUT);
  
  // TODO REMOVE WHEN CLOUD IR ADDED
  // SCAN
  pinMode(32, INPUT);
  // BROADCAST
  pinMode(35, INPUT);
  
  Serial.begin(115200);

  delay(1000);
  Serial.println("Starting setup...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  // Start by scanning networks
  scanNetworks();
}

void loop() 
{
  OperationMode mode = getMode();
  switch(mode)
  {
    case OperationMode::ScanMode:
      scanNetworks();
      break;
    case OperationMode::UploadMode:
      g_tick = 0;
      g_savedTick = g_tick;
      uploadNetworks();
      break;
    case OperationMode::IRScan:
      g_irLength = 5000;
      irScan();
      break;
    case OperationMode::IRBroadcast:
      irBroadcast();
      break;
    case OperationMode::HoldMode:
    default:
      delay(100);
      break;
  }

  g_previousMode = mode;
  g_tick++;
}