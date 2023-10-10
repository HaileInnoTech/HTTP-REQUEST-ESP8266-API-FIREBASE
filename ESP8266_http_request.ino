#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "NWVHCM";
const char* password = "N@viiw9rld789#";
const String endpoint = "https://us-central1-fingerprintdb3.cloudfunctions.net/app/api/createfp";

HTTPClient http;
WiFiClientSecure client;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // // Get user input for id, name, and data
  // String id = getUserInput("Enter id:");
  // String name = getUserInput("Enter name:");
  // String data = getUserInput("Enter data:");

  // // Construct JSON string
  // String jsonData = "{\"id\": \"" + id + "\", \"name\": \"" + name + "\", \"data\": \"" + data + "\"}";

  // Make a POST request
  // int httpResponseCode = postToServer(jsonData);

  // if (httpResponseCode > 0) {
  //   String payload = http.getString();
  //   Serial.println(httpResponseCode);
  //   Serial.println(payload);
  // } else {
  //   Serial.println("Error on HTTP request");
  // }

String response = getResponseData();

  if (response != "") {
    parseResponse(response);
  } else {
    Serial.println("Error on HTTP request");
  }


}

int postToServer(String jsonData) {
 
  client.setInsecure();

  http.begin(client, endpoint);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonData);

  http.end();
  return httpResponseCode;
}

String getUserInput(String prompt) {
  Serial.println(prompt);
  while (!Serial.available()) {
    // Wait for user input
  }
  String input = Serial.readStringUntil('\n');
  input.trim();
  return input;
}


String getResponseData() {
  client.setInsecure();

  http.begin(client, "https://us-central1-fingerprintdb3.cloudfunctions.net/app/api/getAllfp");
  int httpResponseCode = http.GET();

  String response;

  if (httpResponseCode > 0) {
    response = http.getString();
  }

  http.end();
  return response;
}
void parseResponse(String responseData) {
  DynamicJsonDocument doc(2048);  // Adjust the size as needed

  deserializeJson(doc, responseData);

  const char* status = doc["status"];
  Serial.println("Status: " + String(status));

  JsonArray msg = doc["msg"];
  for (JsonObject obj : msg) {
    const char* id = obj["id"];
    const char* name = obj["name"];
    const char* data = obj["data"];

    Serial.println("ID: " + String(id));
    Serial.println("Name: " + String(name));
    Serial.println("Data: " + String(data));
    Serial.println();
  }
}


void loop() {
  // Not doing anything in the loop
}
