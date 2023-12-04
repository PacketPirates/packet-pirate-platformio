#include <Arduino.h>
#include <Arduino_Json.h>
#include <PCAP.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "defs.h"
#include <esp_wifi.h>
#include <nvs_flash.h>

void runTest()
{
  String getTestEndpoint = "";
  getTestEndpoint.concat(WEBSERVER_ENDPOINT);
  getTestEndpoint.concat("/get-test");

  String responseStr = httpGETRequest(getTestEndpoint);
  if (responseStr == "{}")
  {
    Serial.println("ERROR GETTING TEST COMMAND");
    return;
  }

  JSONVar response = JSON.parse(responseStr);
  
  int targetId = JSON.stringify(response["id"]).toInt();
  if (targetId >= g_networksCount)
  {
    Serial.println("ERROR: Network ID out of range. Cannot run test on target AP");
    return;
  }

  String test = JSON.stringify(response["type"]);

  Serial.print("About to perform test: "); Serial.println(test);
  if (test.equals("\"capture\""))
    tempTest(targetId);
}

void uploadTestResult(int networkId, String testname, bool result)
{
  String uploadTestEndpoint = "";
  uploadTestEndpoint.concat(WEBSERVER_ENDPOINT);
  uploadTestEndpoint.concat("/upload-test-result");

  String json = "{";

  json.concat("\"id\": "); json.concat(String(networkId)); json.concat(", ");
  json.concat("\"test\": \""); json.concat(testname); json.concat("\", ");
  json.concat("\"result\": "); json.concat(String(result)); json.concat("}");

  httpPOSTRequest(uploadTestEndpoint, json.c_str(), true);
}

//===== SETTINGS =====//
#define FILENAME "esp32"
#define SAVE_INTERVAL 10 //save new file every 30s

//===== Run-Time variables =====//
unsigned long lastTime = 0;
unsigned long lastChannelChange = 0;
int counter = 0;
bool fileOpen = false;

PCAP pcap = PCAP();

/* will be executed on every packet the ESP32 gets while beeing in promiscuous mode */
void sniffer(void *buf, wifi_promiscuous_pkt_type_t type){
  
  if(fileOpen){
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;
  
    uint32_t timestamp = millis(); //current timestamp 
    uint32_t microseconds = (unsigned int)(micros() - millis() * 1000); //micro seconds offset (0 - 999)
    pcap.newPacketSD(timestamp, microseconds, ctrl.sig_len, pkt->payload); //write packet to file
    
  }
  
}

/* opens a new file */
void openFile(){

  //searches for the next non-existent file name
  int c = 0;
  String filename = "/" + (String)FILENAME + ".pcap";
  while(LittleFS.open(filename)){
    filename = "/" + (String)FILENAME + "_" + (String)c + ".pcap";
    c++;
  }
  
  //set filename and open the file
  pcap.filename = filename;
  fileOpen = pcap.openFile(LittleFS);

  Serial.println("opened: "+filename);

  //reset counter (counter for saving every X seconds)
  counter = 0;
}

void PCAPLoop()
{
   unsigned long currentTime = millis();
  
  	/* for every second */
  if(fileOpen && currentTime - lastTime > 1000){
    pcap.flushFile(); //save file
    lastTime = currentTime; //update time
    counter++; //add 1 to counter
  }
  /* when counter > 30s interval */
  if(fileOpen && counter > SAVE_INTERVAL){
    String filename = pcap.filename;
    pcap.closeFile(); //save & close the file
    fileOpen = false; //update flag
    Serial.println("==================");
    Serial.println(pcap.filename + " saved!");
    Serial.println("==================");


    esp_wifi_set_promiscuous(false);
    uploadFile(filename.c_str());
    esp_wifi_set_promiscuous(true);

    openFile(); //open new file
  }
}

void tempTest(int networkId)
{
  Serial.print("Running test on network: "); Serial.print(g_networksArray[networkId].ssid);
  Serial.print(", AP id #: "); Serial.print(g_networksArray[networkId].id); Serial.println("...");
  delay(3000);
    
  /* setup wifi */
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(sniffer);
  wifi_second_chan_t secondCh = (wifi_second_chan_t)NULL;
  esp_wifi_set_channel(g_networksArray[networkId].channel,secondCh);

  openFile();

  unsigned long startingTime = millis();
  while (millis() < startingTime + (10 * 1000))
    PCAPLoop();

  String filename = pcap.filename;

  pcap.closeFile();

  uploadFile(filename.c_str());

  uploadTestResult(networkId, "capture", true);
}