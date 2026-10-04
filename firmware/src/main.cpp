#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

#define BME280_I2C_ADDRESS 0x76

unsigned long delayTime;

void printValues();

void setup() {
    Serial.begin(9600);

    if (!bme.begin(BME280_I2C_ADDRESS)) {
        Serial.println("Could not find a valid BME280 sensor");
        while (1);
    }

    delayTime = 5000;
}

void loop() {
    printValues();
    delay(delayTime);
}

void printValues() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
}
