#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <FirebaseESP32.h>  // Use FirebaseESP8266.h if using ESP8266

// Firebase and WiFi credentials
#define WIFI_SSID "Your wifi"
#define WIFI_PASSWORD "Your wifi name"
#define FIREBASE_HOST "firebase http link"
#define FIREBASE_AUTH "firebase project api webkey"

// Initialize Firebase
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Initialize MPU6050
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to WiFi");

  // Configure Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

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

  // Upload data to Firebase
  String path = "/MPU6050_Data";  // Path in Firebase

  Firebase.setFloat(firebaseData, path + "/Acceleration_X", a.acceleration.x);
  Firebase.setFloat(firebaseData, path + "/Acceleration_Y", a.acceleration.y);
  Firebase.setFloat(firebaseData, path + "/Acceleration_Z", a.acceleration.z);
  Firebase.setFloat(firebaseData, path + "/Rotation_X", g.gyro.x);
  Firebase.setFloat(firebaseData, path + "/Rotation_Y", g.gyro.y);
  Firebase.setFloat(firebaseData, path + "/Rotation_Z", g.gyro.z);
  Firebase.setFloat(firebaseData, path + "/Temperature", temp.temperature);

  Serial.println("Data uploaded to Firebase!\n");
  
  delay(500);  // Adjust delay as needed
}