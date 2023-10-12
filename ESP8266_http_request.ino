#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>

SoftwareSerial mySerial(14, 12);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const char* ssid = "NWVHCM";
const char* password = "N@viiw9rld789#";

HTTPClient http;
WiFiClientSecure client;

const int MAX_IDS = 128;  // Adjust as needed
int idArray[MAX_IDS];

bool check;


void setup() {
  Serial.begin(9600);
  connectToWiFi();
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    Serial.println("Ready to use!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
}

void httpResponseCode(int response) {
  if (response > 0) {
    String payload = http.getString();
    Serial.println(response);
    Serial.println(payload);
  } else {
    Serial.println("Error on HTTP request");
  }
}
int postToServer(String jsonData) {
  client.setInsecure();
  http.begin(client, "https://us-central1-fingerprintdb3.cloudfunctions.net/app/api/createfp");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonData);

  http.end();
  return httpResponseCode;
}
int postAttendence(String jsonData) {
  client.setInsecure();
  http.begin(client, "https://us-central1-fingerprintdb3.cloudfunctions.net/app/api/createAttendence");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonData);

  http.end();
  return httpResponseCode;
}
String getUserInput(String prompt) {
  Serial.println(prompt);
  while (Serial.available() == 0) {}
  String input = Serial.readStringUntil('\n');
  input.trim();
  return input;
}
String getResponseData() {
  client.setInsecure();
  http.begin(client, "https://us-central1-fingerprintdb3.cloudfunctions.net/app/api/getAllfpid");
  int httpResponseCode = http.GET();
  String response;
  if (httpResponseCode > 0) {
    response = http.getString();
  }
  http.end();
  return response;
}

String getResponseByID(int ID) {
  client.setInsecure();

  String endpointurl = "https://us-central1-fingerprintdb3.cloudfunctions.net/app/api/getfingerbyid/" + String(ID);
  http.begin(client, endpointurl);
  int httpResponseCode = http.GET();
  String response;
  if (httpResponseCode > 0) {
    response = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    String name = doc["data"]["name"];
    String id = doc["data"]["id"];

    response = generateJsonData(id,name);

  } else {
    response = "Error on HTTP request";
  }
  http.end();
  return response;
}

int parseResponse(String responseData, int* idArray) {
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, responseData);

  JsonArray msg = doc["msg"];
  int numIds = 0;

  for (JsonObject obj : msg) {
    const char* id = obj["id"];
    if (id != nullptr) {
      idArray[numIds] = atoi(id);
      numIds++;
    }
  }

  return numIds;
}

String generateJsonData(String id, String name) {
  StaticJsonDocument<200> doc;
  doc["id"] = id;
  doc["name"] = name;
  String jsonData;
  serializeJson(doc, jsonData);

  return jsonData;
}
void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
}
uint8_t getFingerprintEnroll(int id) {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");


        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  return true;
}
uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}
bool isIDExist(int* idArray, int arraySize, int ID) {
  for (int i = 0; i < arraySize; i++) {
    if (idArray[i] == ID) {
      return true;  // ID found in the array
    }
  }
  return false;  // ID not found in the array
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  check = true;
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);

  return finger.fingerID;
}


// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  return finger.fingerID;
}




void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Select an option:");
    Serial.println("1. Post data");
    Serial.println("2. Get all data");
    while (!Serial.available()) {}
    int choice = Serial.parseInt();
    String fix;
    int ID = 0;
    String name = "";
    String jason = "";
    int httpresponsecode = 0;
    check = false;
    String result = "";
    int array;
    int i = 0;
    switch (choice) {
      case 1:
        Serial.println("Ready to enroll a fingerprint!");
        Serial.println("Please type in the ID # to save");
        ID = readnumber();
        result = getResponseData();
        array = parseResponse(result, idArray);
        if (isIDExist(idArray, array, ID)) {
          Serial.println("ID already exists in the array.");
          delay(3000);
          break;
        } else {
          fix = getUserInput("");
          name = getUserInput("Please type in the name for the ID #" + String(ID));
          getFingerprintEnroll(ID);
          jason = generateJsonData(String(ID), name);
          Serial.println(jason);
          httpresponsecode = postToServer(jason);
          httpResponseCode(httpresponsecode);
        }
        delay(3000);
        fix = getUserInput("");

        break;


      case 2:
        finger.getTemplateCount();
        if (finger.templateCount == 0) {
          Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
          break;
        } else {
          Serial.println("Waiting for valid finger...");
          Serial.print("Sensor contains ");
          Serial.print(finger.templateCount);
          Serial.println(" templates");
          delay(3000);
          while (check != true) {
            ID = getFingerprintID();
            delay(1000);
          }
        }
        jason = getResponseByID(ID);
        Serial.println(jason);
        httpresponsecode = postAttendence(jason);
        httpResponseCode(httpresponsecode);
        delay(3000);
        fix = getUserInput("");
        break;
      default:
        Serial.println("Please try again");
        break;
    }
  } else {
    connectToWiFi();
  }
}