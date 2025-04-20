//MPU6050
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define SS 5
#define RST 14
#define DIO0 2

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender");

  // Initialize MPU6050
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

  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print to Serial
  Serial.println("Sending packet...");

  // Send data via LoRa
  LoRa.beginPacket();
  LoRa.print("AX:");
  LoRa.print(a.acceleration.x);
  LoRa.print(" AY:");
  LoRa.print(a.acceleration.y);
  LoRa.print(" AZ:");
  LoRa.print(a.acceleration.z);
  LoRa.print(" GX:");
  LoRa.print(g.gyro.x);
  LoRa.print(" GY:");
  LoRa.print(g.gyro.y);
  LoRa.print(" GZ:");
  LoRa.print(g.gyro.z);
  LoRa.print(" T:");
  LoRa.print(temp.temperature);
  LoRa.endPacket();

  delay(2000); // Send every 2 seconds
}
