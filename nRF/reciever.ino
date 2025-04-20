//MPU6050
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 4
#define CSN_PIN 5

RF24 radio(CE_PIN, CSN_PIN);
const byte address[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

struct SensorData {
  float ax, ay, az;
  float gx, gy, gz;
  float temp;
};

void setup() {
  Serial.begin(115200);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(100);
  radio.startListening();
  Serial.println("NRF24L01 Receiver Ready (No Firebase).");
}

void loop() {
  if (radio.available()) {
    SensorData receivedData;
    radio.read(&receivedData, sizeof(receivedData));

    Serial.println("MPU6050 Data Received:");
    Serial.print("Acceleration -> X: "); Serial.print(receivedData.ax);
    Serial.print(" | Y: "); Serial.print(receivedData.ay);
    Serial.print(" | Z: "); Serial.println(receivedData.az);

    Serial.print("Rotation -> X: "); Serial.print(receivedData.gx);
    Serial.print(" | Y: "); Serial.print(receivedData.gy);
    Serial.print(" | Z: "); Serial.println(receivedData.gz);

    Serial.print("Temperature: "); Serial.print(receivedData.temp);
    Serial.println(" °C\n");
  }

  delay(200);
}
