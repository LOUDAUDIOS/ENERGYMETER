
#include "constants.h"


void setup() {
  lcd.begin(20, 4);
  Serial.begin(115200);
  SPIFFS.begin(true);
  
  readEp();

  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);

  digitalWrite(R1, 0);
  digitalWrite(R2, 0);
  digitalWrite(R3, 0);


  load1Timer.setCallback([]() {
    digitalWrite(R1, 0);
    client.publish(serverTopic, "{\"load1\":false}");
  });
  load2Timer.setCallback([]() {
    digitalWrite(R2, 0);
    client.publish(serverTopic, "{\"load2\":false}");
  });
  load3Timer.setCallback([]() {
    digitalWrite(R2, 0);
    client.publish(serverTopic, "{\"load3\":false}");
  });

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("SMART ENERGY METER");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi...");
  WiFiSettings.connect();
  Serial.println("Connected to the WiFi network");
  lcd.setCursor(0, 1);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 2);
  lcd.print("Connecting server..");


  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
      lcd.setCursor(0, 3);
      lcd.print("Server Connected..");
    } else {
      Serial.print("failed with state ");
      lcd.setCursor(0, 3);
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
    if (msg.startsWith("alive")) {
      String m = "{\"paid\":" + String(paidamt) + "}";
      client.publish(serverTopic, m.c_str());
    } else
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




  if (voltage >= 0) {
    if (millis() - tt > 3000) {
      tt = millis();
      if ((amount != lastAmount) && paidamt > 0) {
        lastAmount = amount;
        totalAmount += amount;
        saveEp();
      }
      postData();
    }
    // Serial.printf("Voltage: %.2fV, Current: %.2fA, Power: %.2fW, Energy: %.2fkWh\n", voltage, current, power, energy);
  } else {
    Serial.println("Failed to read from PZEM module");
  }

  // Calculate the bill based on energy consumption
  amount = calculateBill(energy);



  client.loop();
  load1Timer.run();
  load2Timer.run();
  load3Timer.run();
  // postTimer.run();
}

void postData() {
  String data = createJson();
  Serial.println(data);
  client.publish(serverTopic, data.c_str());
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(" SMART ENERGY METER");

  lcd.setCursor(0, 1);
  lcd.print("VOL:" + String(voltage) + "V");

  lcd.setCursor(12, 1);
  lcd.print("CUR:" + String(current, 1) + "A");

  lcd.setCursor(0, 2);
  lcd.print("PWR:" + String(power) + "W");

  lcd.setCursor(12, 2);
  lcd.print("FRQ:" + String(frequency) + "Hz");

  lcd.setCursor(0, 3);
  lcd.print("ENG:" + String(energy) + "KWH");

  lcd.setCursor(12, 3);
  lcd.print("PF:" + String(pf));





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
  json += "\"amount\":" + String(totalAmount) + ",";
  json += "\"paid\":" + String(paidamt);
  json += "}";
  return json;
}
