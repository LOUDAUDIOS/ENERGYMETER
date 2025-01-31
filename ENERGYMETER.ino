#include <WiFi.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <PZEM004Tv30.h>
#include "constants.h"
#include <LiquidCrystal.h>

const char *mqtt_broker = "broker.emqx.io";
const char *topic = "espmeter/data";
const char *serverTopic = "post/data";
const char *mqtt_username = "espmeter";
const char *mqtt_password = "123456";
const int mqtt_port = 1883;


// Define LCD pins
#define RS 13
#define E  12
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

void setup() {
  lcd.begin(20,4);
  Serial.begin(115200);
  SPIFFS.begin(true);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi...");
  WiFiSettings.connect();
  Serial.println("Connected to the WiFi network");
  lcd.setCursor(0,1);
  lcd.print("WiFi Connected");
  lcd.setCursor(0,2);
  lcd.print("Connecting server..");


  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
      lcd.setCursor(0,3);
      lcd.print("Server Connected..");
    } else {
      Serial.print("failed with state ");
      lcd.setCursor(0,3);
      lcd.print("Retrying..");
      Serial.print(client.state());
      delay(2000);

    }
  }
  client.publish(topic, "Connected");
  client.subscribe(topic);
  client.publish(serverTopic, "Connected");
  client.subscribe(serverTopic);
}

void callback(char *topic, byte *payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += ((char)payload[i]);
  }
  if (!String(topic).startsWith("post/data"))
    parseJson(msg);
}
unsigned long tt = 0;
void loop() {
  // Fetch readings from the PZEM module
  voltage = pzem.voltage();
  current = pzem.current();
  power = pzem.power();
  energy = pzem.energy();
  frequency = pzem.frequency();
  pf = pzem.pf();

  if (voltage != NAN && current != NAN && power != NAN) {
    if (millis() - tt > 3000) {
      tt = millis();
      postData();
    }
    // Serial.printf("Voltage: %.2fV, Current: %.2fA, Power: %.2fW, Energy: %.2fkWh\n", voltage, current, power, energy);
  } else {
    Serial.println("Failed to read from PZEM module");
  }

  // Calculate the bill based on energy consumption
  amount = calculateBill(energy);




  client.loop();
  // postTimer.run();
}

void postData() {
  String data = createJson();
  Serial.println(data);
  client.publish(serverTopic, data.c_str());
  lcd.clear();

  lcd.setCursor(1,0);
  lcd.print("SMART ENERGY METER");
  lcd.setCursor(0,1);
  lcd.print("Volt:"+String(voltage)+"V");
  
  lcd.setCursor(0,1);
  lcd.print("Curr:"+String(current)+"A");
  
  lcd.setCursor(10,1);
  lcd.print("Power:"+String(power)+"W");
  
  lcd.setCursor(0,2);
  lcd.print("Freq:"+String(frequency)+"Hz");
  
  lcd.setCursor(10,2);
  lcd.print("PF:"+String(pf));
  
  lcd.setCursor(0,3);
  lcd.print("Energy:"+String(energy)+"Units");
  
  lcd.setCursor(10,3);
  lcd.print("Amount:Rs"+String(amount));
  
  // lcd.setCursor()
  // postTimer.reset();
}

float calculateBill(float energy) {
  if (energy <= 50) return energy * 3.35;
  if (energy <= 100) return 50 * 3.35 + (energy - 50) * 4.25;
  if (energy <= 150) return 50 * 3.35 + 50 * 4.25 + (energy - 100) * 5.35;
  if (energy <= 200) return 50 * 3.35 + 50 * 4.25 + 50 * 5.35 + (energy - 150) * 7.20;
  if (energy <= 250) return 50 * 3.35 + 50 * 4.25 + 50 * 5.35 + 50 * 7.20 + (energy - 200) * 8.50;
  return energy * 6.75;  // Non-Telescopic Rate
}

String createJson() {
  String json = "{";
  json += "\"voltage\":" + String(voltage) + ",";
  json += "\"current\":" + String(current) + ",";
  json += "\"freq\":" + String(frequency) + ",";
  json += "\"pf\":" + String(pf) + ",";
  json += "\"power\":" + String(power) + ",";
  json += "\"energy\":" + String(energy) + ",";
  json += "\"amount\":" + String(amount);
  json += "}";
  return json;
}
