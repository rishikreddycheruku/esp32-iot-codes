#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi Credentials
#define WIFI_SSID "Your wifi name"
#define WIFI_PASSWORD "your password"

// ThingSpeak Settings
#define THINGSPEAK_API_KEY "Your Things Speak Write API"
#define THINGSPEAK_URL "http://api.thingspeak.com/update"


Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to WiFi");

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print values to Serial Monitor
  Serial.print("Acceleration X: "); Serial.print(a.acceleration.x);
  Serial.print(", Y: "); Serial.print(a.acceleration.y);
  Serial.print(", Z: "); Serial.println(a.acceleration.z);

  Serial.print("Rotation X: "); Serial.print(g.gyro.x);
  Serial.print(", Y: "); Serial.print(g.gyro.y);
  Serial.print(", Z: "); Serial.println(g.gyro.z);

  Serial.print("Temperature: "); Serial.print(temp.temperature);
  Serial.println(" °C");

  // ThingSpeak Upload
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(THINGSPEAK_URL) +
                 "?api_key=" + THINGSPEAK_API_KEY +
                 "&field1=" + String(a.acceleration.x) +
                 "&field2=" + String(a.acceleration.y) +
                 "&field3=" + String(a.acceleration.z) +
                 "&field4=" + String(g.gyro.x) +
                 "&field5=" + String(g.gyro.y) +
                 "&field6=" + String(g.gyro.z) +
                 "&field7=" + String(temp.temperature);

    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println("Data sent to ThingSpeak!");
    } else {
      Serial.print("Error sending to ThingSpeak: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  delay(15000); // 15 seconds delay for ThingSpeak rate limit
}
