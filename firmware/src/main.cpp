#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SGP41.h>
#include <VOCGasIndexAlgorithm.h>

constexpr uint8_t kI2cSdaPin = 21;
constexpr uint8_t kI2cSclPin = 22;

constexpr uint32_t kI2cFrequencyHz = 100000;

constexpr uint32_t kSerialBaudRate = 115200;

constexpr uint8_t kBme280Address = 0x76;
constexpr uint8_t kSgp41Address = 0x59;

Adafruit_BME280 bme;

Adafruit_SGP41 sgp;

VOCGasIndexAlgorithm vocAlgorithm;

uint32_t delayTime = 1000;

void printValues();

void sgp41Conditioning();

void setup() {

    Serial.begin(kSerialBaudRate);
    Wire.begin(kI2cSdaPin, kI2cSclPin, kI2cFrequencyHz);

    if (!bme.begin(kBme280Address, &Wire)) {
        Serial.println("Could not find a valid BME280 sensor");
        while (1);
    }

    if (!sgp.begin(kSgp41Address, &Wire)) {
        Serial.println("Could not find a valid SGP41 sensor");
        while (1);
    }

    sgp41Conditioning();

    Serial.println("BME280 and SGP41 sensors initialized successfully!");

}

void loop() {
    printValues();
    delay(delayTime);
}

void sgp41Conditioning() {
    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;

    uint16_t sraw_voc = 0;
    uint16_t sraw_nox = 0;
    int32_t vocIndex = 0;

    uint32_t conditioningTime = 60; // seconds

    Serial.printf("Conditioning (%d seconds)...\n", conditioningTime);

    for (int i = 0; i < 60; i++) {
        temperature = bme.readTemperature();
        humidity = bme.readHumidity();
        pressure = bme.readPressure() / 100.0F;

        if (sgp.executeConditioning(&sraw_voc, humidity, temperature)) {
            vocIndex = vocAlgorithm.process(sraw_voc);

            Serial.print("Temperature = ");
            Serial.print(temperature);
            Serial.println(" *C");

            Serial.print("Pressure = ");
            Serial.print(pressure);
            Serial.println(" hPa");

            Serial.print("Humidity = ");
            Serial.print(humidity);
            Serial.println(" %");

            Serial.print("Raw VOC Signal = ");
            Serial.println(sraw_voc);


            Serial.print("VOC Index = ");
            Serial.print(vocIndex);
            Serial.println(" ppb");

            Serial.println();
          

        } else {
            Serial.println("Error conditioning SGP41 sensor");
        }
        delay(1000);
    }

}

void printValues() {

    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;

    uint16_t sraw_voc = 0;
    uint16_t sraw_nox = 0;
    int32_t vocIndex = 0;

    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;

    if (sgp.measureRawSignals(&sraw_voc, &sraw_nox, humidity, temperature)) {
        vocIndex = vocAlgorithm.process(sraw_voc);
    } else {
        Serial.println("Error reading SGP41 sensor");
    }
    


    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Raw VOC Signal = ");
    Serial.println(sraw_voc);


    Serial.print("VOC Index = ");
    Serial.print(vocIndex);
    Serial.println(" ppb");

    Serial.println();
}
