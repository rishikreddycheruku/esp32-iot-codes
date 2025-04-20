//MPU6050
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 4
#define CSN_PIN 5

RF24 radio(CE_PIN, CSN_PIN);
const byte address[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

Adafruit_MPU6050 mpu;

struct SensorData {
  float ax, ay, az;
  float gx, gy, gz;
  float temp;
};

void setup() {
  Serial.begin(115200);
  
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(100);
  radio.stopListening();
  Serial.println("NRF24L01 Sender Ready.");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  SensorData data = {
    a.acceleration.x, a.acceleration.y, a.acceleration.z,
    g.gyro.x, g.gyro.y, g.gyro.z,
    temp.temperature
  };

  radio.write(&data, sizeof(data));

  Serial.println("Data sent.");
  delay(500);
}
