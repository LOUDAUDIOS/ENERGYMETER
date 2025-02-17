#include <WiFi.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "myTimer.h"
#include "MyButton.h"



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
bool nobalance = true;
float current = 1.5;
float power = 330;
float energy = 0.1;
float powerT = 0;
float pf = 0.87;
float amount = 15.0, paidamt = 0, totalAmount = 0, totalEnergy=0;
unsigned long lastMillis = 0;

float lastEnergy = 0.0;

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
Preferences preferences;

// Define RX and TX pins for PZEM
#define RX_PIN 18
#define TX_PIN 19

// PZEM module initialization
PZEM004Tv30 pzem(Serial2, RX_PIN, TX_PIN);

MyTimer postTimer, load1Timer, load2Timer, load3Timer;
Button rstBtn;
// Create JSON payload to send MQTT data

void saveEp() {
  // preferences.clear();
  preferences.begin("my-app", false);
  preferences.putFloat("paidamt", paidamt);
  // preferences.putFloat("totalenergy", totalEnergy);
  preferences.end();
}

void readEp() {
  preferences.begin("my-app", false);
  paidamt = preferences.getFloat("paidamt");
  // totalEnergy = preferences.getFloat("totalenergy");
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
  if (doc.containsKey("load1State")&&!nobalance) {
    load1State = doc["load1State"];
    digitalWrite(R1, load1State);
    Serial.print("Load 1: ");
    Serial.println(load1State ? "ON" : "OFF");
    // lcd.setCursor()
  }

  if (doc.containsKey("load2State")&&!nobalance) {
    load2State = doc["load2State"];
    digitalWrite(R2, load2State);
    Serial.print("Load 2: ");
    Serial.println(load2State ? "ON" : "OFF");
  }

  if (doc.containsKey("load3State") && !nobalance) {
    load3State = doc["load3State"];
    digitalWrite(R3, load3State);
    Serial.print("Load 3: ");
    Serial.println(load3State ? "ON" : "OFF");
  }

  if (doc.containsKey("paid")) {
    pzem.resetEnergy();
    float paid = doc["paid"];
    paidamt += paid;
    saveEp();
    Serial.println("Paid amount:"+String(paidamt));
    client.publish(serverTopic, "paymentsuccess");
    delay(2000);
    ESP.restart();
  }
}
