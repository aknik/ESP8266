#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1100.00)

Adafruit_BME280 bme; // I2C

void setup() {

Wire.begin(4,5);
Serial.begin(115200);
Serial.println(F("BME280 test"));

if (!bme.begin(0x76)) {
Serial.println("Could not find a valid BME280 sensor, check wiring!");
while (1);
}
}

void loop() {
Serial.print("Temperatura: ");
Serial.print(bme.readTemperature());
Serial.println(" *C");

Serial.print("Presión: ");

Serial.print(bme.readPressure() / 100.0F);
Serial.println(" hPa");

//Serial.print("Approx. Altitude = ");
//Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
//Serial.println(" m");

Serial.print("Humedad: ");
Serial.print(bme.readHumidity());
Serial.println(" %");

Serial.println();
delay(10000);
}

