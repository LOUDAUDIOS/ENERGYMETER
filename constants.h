#include <WiFi.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "myTimer.h"



// WiFi credentials
const char *ssid = "192.161.4.1";      // Replace with your WiFi SSID
const char *password = "myname@7012";  // Replace with your WiFi password


const char *mqtt_broker = "broker.emqx.io";
const char *topic = "espmeter/data";
const char *serverTopic = "post/data";
const char *mqtt_username = "espmeter";
const char *mqtt_password = "123456";
const int mqtt_port = 1883;



// Variables
unsigned long previousMillis = 0;
const long interval = 5000;  // 5 seconds
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
float amount = 15.0, paidamt = 0, totalAmount = 0;
unsigned long lastMillis = 0;

float lastAmount = 0;

#define R1 15
#define R2 2
#define R3 4

// Define LCD pins
#define RS 13
#define E 12
#define D4 14
#define D5 27
#define D6 26
#define D7 25

WiFiClient espClient;
PubSubClient client(espClient);
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

// Define RX and TX pins for PZEM
#define RX_PIN 18
#define TX_PIN 19

// PZEM module initialization
PZEM004Tv30 pzem(Serial2, RX_PIN, TX_PIN);

MyTimer postTimer, load1Timer, load2Timer, load3Timer;
// Create JSON payload to send MQTT data

void saveEp() {
  EEPROM.writeFloat(2, paidamt);
  EEPROM.writeFloat(3, totalAmount);
  EEPROM.commit();
}

void readEp() {
  paidamt = EEPROM.readFloat(2);
  totalAmount = EEPROM.readFloat(3);
}

void parseJson(String json) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  if (doc.containsKey("time1")) {
    Serial.println("Load1 timer start");
    // Serial.println((String)doc["time1"]);
    load1Timer.setTimeout(Time(0, (long)doc["time1"]));
    load1Timer.reset();
  }
  if (doc.containsKey("time2")) {
    Serial.println("Load2 timer start");
    // Serial.println(doc["time2"]);
    load2Timer.setTimeout(Time(0, (long)doc["time2"]));
    load2Timer.reset();
  }
  if (doc.containsKey("time3")) {
    Serial.println("Load3 timer start");
    // Serial.println(doc["time3"]);
    load3Timer.setTimeout(Time(0, (long)doc["time3"]));
    load3Timer.reset();
  }

  // Check and update load states
  if (doc.containsKey("load1State")) {
    load1State = doc["load1State"];
    digitalWrite(R1, load1State);
    Serial.print("Load 1: ");
    Serial.println(load1State ? "ON" : "OFF");
    // lcd.setCursor()
  }

  if (doc.containsKey("load2State")) {
    load2State = doc["load2State"];
    digitalWrite(R2, load2State);
    Serial.print("Load 2: ");
    Serial.println(load2State ? "ON" : "OFF");
  }

  if (doc.containsKey("load3State")) {
    load3State = doc["load3State"];
    digitalWrite(R3, load3State);
    Serial.print("Load 3: ");
    Serial.println(load3State ? "ON" : "OFF");
  }

  if (doc.containsKey("paid")) {
    totalAmount = 0;
    paidamt = doc["paid"];
    saveEp();
    client.publish(serverTopic, "paymentsuccess");
    ESP.restart();
  }
}
