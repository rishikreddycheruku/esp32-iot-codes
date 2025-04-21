#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// === WiFi + Adafruit IO Credentials ===
#define WLAN_SSID     "Your_WiFi_Name"
#define WLAN_PASS     "Your_WiFi_Password"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Your_Adafruit_IO_Username"
#define AIO_KEY         "Your_Adafruit_IO_Key"

// === LoRa Pins ===
#define SS 5
#define RST 14
#define DIO0 2

// === Global Instances ===
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish axFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ax");
Adafruit_MQTT_Publish ayFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ay");
Adafruit_MQTT_Publish azFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/az");

Adafruit_MQTT_Publish gxFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gx");
Adafruit_MQTT_Publish gyFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gy");
Adafruit_MQTT_Publish gzFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gz");

Adafruit_MQTT_Publish tempFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish loraReceivedFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lora");

Adafruit_MPU6050 mpu;

void MQTT_Connect() {
  int8_t ret;
  if (mqtt.connected()) return;

  Serial.println("Connecting to MQTT...");
  while ((ret = mqtt.connect()) != 0) {
    Serial.print("MQTT Connect Error: ");
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT Connected!");
}

void setup() {
  Serial.begin(115200);

  // WiFi
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  Serial.println("Connecting to WiFi...");
  int timeout = 40;
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    if (--timeout == 0) {
      Serial.println("WiFi Failed. Restarting...");
      ESP.restart();
    }
  }
  Serial.println("\nWiFi Connected.");

  // MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);

  // LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("Setup complete!");
}

unsigned long lastPublish = 0;
const long publishInterval = 15000;

void loop() {
  MQTT_Connect();

  // === 1. Listen for LoRa packets and publish to Adafruit IO ===
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    Serial.print("Received LoRa packet: ");
    Serial.println(receivedData);

    loraReceivedFeed.publish(receivedData.c_str());
  }

  // === 2. Publish data over LoRa and Adafruit IO every interval ===
  if (millis() - lastPublish > publishInterval) {
    lastPublish = millis();

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // LoRa Send
    Serial.println("Sending sensor data over LoRa...");
    LoRa.beginPacket();
    LoRa.print("AX:"); LoRa.print(a.acceleration.x);
    LoRa.print(" AY:"); LoRa.print(a.acceleration.y);
    LoRa.print(" AZ:"); LoRa.print(a.acceleration.z);
    LoRa.print(" GX:"); LoRa.print(g.gyro.x);
    LoRa.print(" GY:"); LoRa.print(g.gyro.y);
    LoRa.print(" GZ:"); LoRa.print(g.gyro.z);
    LoRa.print(" T:");  LoRa.print(temp.temperature);
    LoRa.endPacket();

    // MQTT Publish
    axFeed.publish(a.acceleration.x);
    ayFeed.publish(a.acceleration.y);
    azFeed.publish(a.acceleration.z);
    gxFeed.publish(g.gyro.x);
    gyFeed.publish(g.gyro.y);
    gzFeed.publish(g.gyro.z);
    tempFeed.publish(temp.temperature);

    Serial.println("Sensor data published to Adafruit IO");
  }
}
