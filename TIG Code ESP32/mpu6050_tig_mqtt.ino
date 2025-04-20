#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Replace with your WiFi credentials and MQTT server details
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_BROKER_ADDRESS";
const int mqtt_port = 1883;
const char* mqtt_topic = "iot/sensor";  // MQTT topic to publish data

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_MPU6050 mpu;

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  Serial.print("Connecting to MQTT");
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("\nMQTT connected");
    } else {
      Serial.print(".");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  delay(100);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  StaticJsonDocument<256> doc;
  doc["accel_x"] = a.acceleration.x;
  doc["accel_y"] = a.acceleration.y;
  doc["accel_z"] = a.acceleration.z;
  doc["gyro_x"] = g.gyro.x;
  doc["gyro_y"] = g.gyro.y;
  doc["gyro_z"] = g.gyro.z;
  doc["temperature"] = temp.temperature;

  char buffer[256];
  serializeJson(doc, buffer);

  Serial.print("Publishing: ");
  Serial.println(buffer);
  client.publish(mqtt_topic, buffer);

  delay(5000);
}
