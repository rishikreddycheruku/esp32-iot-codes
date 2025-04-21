#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <nRF24L01.h>
#include <RF24.h>

// ---------------- WiFi and Adafruit IO ----------------
#define WLAN_SSID     "Your_WiFi_Name"
#define WLAN_PASS     "Your_WiFi_Password"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Your_Adafruit_IO_Username"
#define AIO_KEY         "Your_Adafruit_IO_Key"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// MQTT Feeds
Adafruit_MQTT_Publish axFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ax");
Adafruit_MQTT_Publish ayFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ay");
Adafruit_MQTT_Publish azFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/az");

Adafruit_MQTT_Publish gxFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gx");
Adafruit_MQTT_Publish gyFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gy");
Adafruit_MQTT_Publish gzFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gz");

Adafruit_MQTT_Publish tempFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");

// ---------------- NRF24L01 ----------------
#define CE_PIN 4
#define CSN_PIN 5
RF24 radio(CE_PIN, CSN_PIN);
const byte address[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

// ---------------- MPU6050 ----------------
Adafruit_MPU6050 mpu;

unsigned long lastUploadTime = 0;
unsigned long lastSendTime = 0;
const unsigned long uploadInterval = 15000;  // 15 sec
const unsigned long sendInterval = 5000;     // 5 sec

struct SensorData {
  float ax, ay, az;
  float gx, gy, gz;
  float temp;
};

void MQTT_Connect() {
  int8_t ret;
  if (mqtt.connected()) return;

  Serial.print("Connecting to MQTT...");
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

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  Serial.print("Connecting to WiFi...");
  int timeout = 40;
  while (WiFi.status() != WL_CONNECTED && timeout--) {
    delay(250);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Failed. Rebooting...");
    ESP.restart();
  }
  Serial.println("\nWiFi Connected.");

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU6050 ready.");

  radio.begin();
  radio.setChannel(100);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);
  radio.startListening();
  Serial.println("NRF24L01 ready.");
}

void loop() {
  MQTT_Connect();
  unsigned long now = millis();

  // === Get sensor readings ===
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // === 1. Send via NRF every 5 seconds ===
  if (now - lastSendTime >= sendInterval) {
    lastSendTime = now;

    SensorData data = {
      a.acceleration.x, a.acceleration.y, a.acceleration.z,
      g.gyro.x, g.gyro.y, g.gyro.z,
      temp.temperature
    };

    radio.stopListening();
    bool success = radio.write(&data, sizeof(data));
    radio.startListening();
    Serial.println(success ? "📡 Data sent via NRF24L01" : "⚠️ NRF send failed");
  }

  // === 2. Upload to Adafruit IO every 15 seconds ===
  if (now - lastUploadTime >= uploadInterval) {
    lastUploadTime = now;

    axFeed.publish(a.acceleration.x);
    ayFeed.publish(a.acceleration.y);
    azFeed.publish(a.acceleration.z);

    gxFeed.publish(g.gyro.x);
    gyFeed.publish(g.gyro.y);
    gzFeed.publish(g.gyro.z);

    tempFeed.publish(temp.temperature);

    Serial.println("✅ Local MPU6050 data uploaded to Adafruit IO.");
  }

  // === 3. If data received via NRF, upload it immediately ===
  if (radio.available()) {
    SensorData receivedData;
    radio.read(&receivedData, sizeof(receivedData));

    Serial.println("📥 NRF24L01 Data Received:");
    Serial.print("AX: "); Serial.print(receivedData.ax);
    Serial.print(" AY: "); Serial.print(receivedData.ay);
    Serial.print(" AZ: "); Serial.println(receivedData.az);
    Serial.print("GX: "); Serial.print(receivedData.gx);
    Serial.print(" GY: "); Serial.print(receivedData.gy);
    Serial.print(" GZ: "); Serial.println(receivedData.gz);
    Serial.print("Temp: "); Serial.println(receivedData.temp);

    axFeed.publish(receivedData.ax);
    ayFeed.publish(receivedData.ay);
    azFeed.publish(receivedData.az);

    gxFeed.publish(receivedData.gx);
    gyFeed.publish(receivedData.gy);
    gzFeed.publish(receivedData.gz);

    tempFeed.publish(receivedData.temp);

    Serial.println("📤 Received NRF data published to Adafruit IO.");
  }
}
