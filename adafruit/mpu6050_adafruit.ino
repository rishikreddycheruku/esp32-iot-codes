#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Replace with your WiFi credentials and Adafruit IO details below.
// Get your AIO username and key from https://io.adafruit.com under My Key.
// Ensure the feeds 'ax', 'ay', 'az', 'gx', 'gy', 'gz', and 'temp' are created.

#define WLAN_SSID     "Your_WiFi_Name"
#define WLAN_PASS     "Your_WiFi_Password"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "Your_Adafruit_IO_Username"
#define AIO_KEY       "Your_Adafruit_IO_Key"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish axFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ax");
Adafruit_MQTT_Publish ayFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ay");
Adafruit_MQTT_Publish azFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/az");

Adafruit_MQTT_Publish gxFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gx");
Adafruit_MQTT_Publish gyFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gy");
Adafruit_MQTT_Publish gzFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gz");

Adafruit_MQTT_Publish tempFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");

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

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);
}

void loop() {
  MQTT_Connect();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  axFeed.publish(a.acceleration.x);
  ayFeed.publish(a.acceleration.y);
  azFeed.publish(a.acceleration.z);

  gxFeed.publish(g.gyro.x);
  gyFeed.publish(g.gyro.y);
  gzFeed.publish(g.gyro.z);

  tempFeed.publish(temp.temperature);

  Serial.println("Data published to all feeds.");
  delay(15000);
}
