#pragma once
#include "math.h"

#define MAX_SENSOR_VALUES 128

typedef enum SensorType {
    IMU,
    BAROMETER,
    GNSS
} SensorType;

typedef struct Sensor {
    SensorType type;
    void *data; // Holds data specific to this type of sensor
    float (*readValue)(void *data); // Reads float data for this sensor
    uint32_t offset;
    float values[MAX_SENSOR_VALUES];
    float minValue;
    float maxValue;
    float hertz;
    uint32_t upTimeMillis;
    Color color;
    const char *typeName;
    const char *productName;
} Sensor;

typedef struct TestSensorData {
    float time;
    float a;
    float b;
} TestSensorData;

float TestSensorReadValue(void *data) {
    TestSensorData *castData = (TestSensorData *)data;
    castData->time += 0.03f;
    return cosf(castData->time + 3.0f * castData->b) + castData->b * sinf(1.0f + castData->a * castData->time) * 0.4f * sinf(castData->a + 2.0f * castData->b * castData->time);
}