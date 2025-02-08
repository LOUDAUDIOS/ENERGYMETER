#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "myTimer.h"

// WiFi credentials
const char* ssid = "192.161.4.1";         // Replace with your WiFi SSID
const char* password = "myname@7012";                // Replace with your WiFi password

// URLs
const char* fetchUrl = "https://myespback-tixp400e.b4a.run/data"; // Fetch URL
const char* postUrl = "https://myespback-tixp400e.b4a.run/data";   // Post URL

// Variables
unsigned long previousMillis = 0;
const long interval = 5000; // 5 seconds
float voltage = 220;
int frequency = 48;
bool load1State = true;
bool load2State = false;
bool load3State = true;
float current = 1.5;
float power = 330;
float energy = 0.1;
float powerT = 0;
float pf = 0.87;
float amount = 15.0;
unsigned long lastMillis = 0;

#define load1 4
#define load2 16
#define load3 17


MyTimer postTimer,load1Timer, load2Timer, load3Timer;
// Create JSON payload to send MQTT data


void parseJson(String json) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  if(doc.containsKey("time1")){
    Serial.println("Load1 timer start");
    // Serial.println((String)doc["time1"]);
    load1Timer.setTimeout(Time(0,(long)doc["time1"]));
    load1Timer.reset();
  }
  if(doc.containsKey("time2")){
    Serial.println("Load2 timer start");
    // Serial.println(doc["time2"]);
    load2Timer.setTimeout(Time(0,(long)doc["time2"]));
    load2Timer.reset();
  }
  if(doc.containsKey("time3")){
    Serial.println("Load3 timer start");
    // Serial.println(doc["time3"]);
    load3Timer.setTimeout(Time(0,(long)doc["time3"]));
    load3Timer.reset();
  }

  // Check and update load states
  if (doc.containsKey("load1State")) {
    load1State = doc["load1State"];
    digitalWrite(load1, load1State);
    Serial.print("Load 1: ");
    Serial.println(load1State ? "ON" : "OFF");
    // lcd.setCursor()
  }

  if (doc.containsKey("load2State")) {
    load2State = doc["load2State"];
    digitalWrite(load2, load2State);
    Serial.print("Load 2: ");
    Serial.println(load2State ? "ON" : "OFF");
  }

  if (doc.containsKey("load3State")) {
    load3State = doc["load3State"];
    digitalWrite(load3, load3State);
    Serial.print("Load 3: ");
    Serial.println(load3State ? "ON" : "OFF");
  }
}




