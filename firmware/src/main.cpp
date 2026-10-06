#include <Wire.h>
#include <SPI.h>
#include <cinttypes>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SGP41.h>
#include <VOCGasIndexAlgorithm.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

constexpr uint8_t kI2cSdaPin = 21;
constexpr uint8_t kI2cSclPin = 22;

constexpr uint32_t kI2cFrequencyHz = 100000;

constexpr uint32_t kSerialBaudRate = 115200;
constexpr uint32_t kSensorReadIntervalMs = 1000;
constexpr uint32_t kSensorTaskStackSize = 4096;
constexpr UBaseType_t kSensorTaskPriority = 1;
constexpr uint32_t kSgp41ConditioningDurationSeconds = 10;

constexpr uint8_t kBme280Address = 0x76;
constexpr uint8_t kSgp41Address = 0x59;

constexpr char kMainLogTag[] = "MAIN";
constexpr char kSgp41LogTag[] = "SGP41";

Adafruit_BME280 bme;

Adafruit_SGP41 sgp;

VOCGasIndexAlgorithm vocAlgorithm;

void printValues();

void sgp41Conditioning();

void sensorReadTask(void*);

void setup() {

    Serial.begin(kSerialBaudRate);
    Wire.begin(kI2cSdaPin, kI2cSclPin, kI2cFrequencyHz);

    if (!bme.begin(kBme280Address, &Wire)) {
        ESP_LOGE(kMainLogTag, "Could not find a valid BME280 sensor");
        while (1);
    }

    if (!sgp.begin(kSgp41Address, &Wire)) {
        ESP_LOGE(kMainLogTag, "Could not find a valid SGP41 sensor");
        while (1);
    }

    sgp41Conditioning();

    ESP_LOGI(kMainLogTag, "BME280 and SGP41 sensors initialized successfully");

    const BaseType_t taskCreationResult = xTaskCreate(
        sensorReadTask,
        "sensor-read",
        kSensorTaskStackSize,
        nullptr,
        kSensorTaskPriority,
        nullptr
    );

    if (taskCreationResult != pdPASS) {
        ESP_LOGE(kMainLogTag, "Could not create sensor read task");
        while (1) {
            delay(1000);
        }
    }

    ESP_LOGI(kMainLogTag, "Sensor read task started");

}

void loop() {
    delay(1000);
}

void sensorReadTask(void*) {
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (true) {
        printValues();
        vTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(kSensorReadIntervalMs)
        );
    }
}

void sgp41Conditioning() {
    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;

    uint16_t sraw_voc = 0;
    uint16_t sraw_nox = 0;
    int32_t vocIndex = 0;

    ESP_LOGI(kSgp41LogTag, "Conditioning (%" PRIu32 " seconds)...",
             kSgp41ConditioningDurationSeconds);

    for (uint32_t i = 0; i < kSgp41ConditioningDurationSeconds; ++i) {
        temperature = bme.readTemperature();
        humidity = bme.readHumidity();
        pressure = bme.readPressure() / 100.0F;

        if (sgp.executeConditioning(&sraw_voc, humidity, temperature)) {
            vocIndex = vocAlgorithm.process(sraw_voc);

            ESP_LOGI(kSgp41LogTag,
                     "Temperature: %.2f C, pressure: %.2f hPa, humidity: %.2f %%, "
                     "raw VOC: %u, VOC index: %" PRId32,
                     temperature, pressure, humidity, sraw_voc,
                     vocIndex);

        } else {
            ESP_LOGE(kSgp41LogTag, "Error conditioning SGP41 sensor");
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
        ESP_LOGE(kSgp41LogTag, "Error reading SGP41 sensor");
    }

    ESP_LOGI(kSgp41LogTag,
             "Temperature: %.2f C, pressure: %.2f hPa, humidity: %.2f %%, "
             "raw VOC: %u, VOC index: %" PRId32,
             temperature, pressure, humidity, sraw_voc,
             vocIndex);
}
