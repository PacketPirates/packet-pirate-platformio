#include <Arduino.h>
#include <Arduino_Json.h>
#include <WiFi.h>

#include "defs.h"

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
  if (test.equals("\"temp\""))
    tempTest(targetId);
}

void uploadTestResult(int networkId, String testname, bool result)
{
  String uploadTestEndpoint = "";
  uploadTestEndpoint.concat(WEBSERVER_ENDPOINT);
  uploadTestEndpoint.concat("/upload-test-result");

  String json = "";

  json.concat("\"id\": {"); json.concat(String(networkId)); json.concat("}, ");
  json.concat("\"test\": { \""); json.concat(testname); json.concat("\"}, ");
  json.concat("\"result\": {"); json.concat(String(result)); json.concat("} ");

  httpPOSTRequest(uploadTestEndpoint, json.c_str(), true);
}

void tempTest(int networkId)
{
  Serial.print("Running test on network: "); Serial.print(g_networksArray[networkId].ssid);
  Serial.print(", AP id #: "); Serial.print(g_networksArray[networkId].id); Serial.println("...");
  delay(3000);

  uploadTestResult(networkId, "temp", true);
}