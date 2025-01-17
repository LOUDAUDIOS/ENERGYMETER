#include <WiFi.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <PZEM004Tv30.h>
#include "constants.h"

const char *mqtt_broker = "broker.emqx.io";
const char *topic = "espmeter/data";
const char *serverTopic = "post/data";
const char *mqtt_username = "espmeter";
const char *mqtt_password = "123456";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Define RX and TX pins for PZEM
#define RX_PIN 16
#define TX_PIN 17

// PZEM module initialization
PZEM004Tv30 pzem(Serial2, RX_PIN, TX_PIN);

void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);
    WiFiSettings.connect();
    Serial.println("Connected to the WiFi network");

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);

    postTimer.setCallback([](){
       String data = createJson(voltage, current, power, energy, amount);
    client.publish(serverTopic, data.c_str());
    postTimer.reset();
    });
    postTimer.setTimeout(3000);

    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    client.publish(topic, "Connected");
    client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
    String msg = "";
    for (int i = 0; i < length; i++) {
        msg += ((char)payload[i]);
    }
    if (!String(topic).startsWith("post/data"))
        parseJson(msg);
}

void loop() {
    // Fetch readings from the PZEM module
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    if (voltage != NAN && current != NAN && power != NAN) {
        Serial.printf("Voltage: %.2fV, Current: %.2fA, Power: %.2fW, Energy: %.2fkWh\n", voltage, current, power, energy);
    } else {
        Serial.println("Failed to read from PZEM module");
    }

    // Calculate the bill based on energy consumption
    float amount = calculateBill(energy);

    client.loop();
    postTimer.run();
}

float calculateBill(float energy) {
    if (energy <= 50) return energy * 3.35;
    if (energy <= 100) return 50 * 3.35 + (energy - 50) * 4.25;
    if (energy <= 150) return 50 * 3.35 + 50 * 4.25 + (energy - 100) * 5.35;
    if (energy <= 200) return 50 * 3.35 + 50 * 4.25 + 50 * 5.35 + (energy - 150) * 7.20;
    if (energy <= 250) return 50 * 3.35 + 50 * 4.25 + 50 * 5.35 + 50 * 7.20 + (energy - 200) * 8.50;
    return energy * 6.75; // Non-Telescopic Rate
}

String createJson(float voltage, float current, float power, float energy, float amount) {
    String json = "{";
    json += "\"voltage\":" + String(voltage) + ",";
    json += "\"current\":" + String(current) + ",";
    json += "\"power\":" + String(power) + ",";
    json += "\"energy\":" + String(energy) + ",";
    json += "\"amount\":" + String(amount);
    json += "}";
    return json;
}
