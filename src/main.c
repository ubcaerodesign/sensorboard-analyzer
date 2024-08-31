#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdint.h>

#include "utils.h"
#include "sensor.h"

#define WIDTH 800
#define HEIGHT 600

typedef struct App {
    SensorType selectedSensor;
    uint32_t sensorCount;
    Sensor *sensors;
} App;

void drawSensorSelection(App *app, SensorType type, uint8_t offset) {
    uint32_t startX = 5;
    uint32_t width = 190;
    uint32_t startY = offset * 60 + 20;
    uint32_t height = 60;

    Rectangle bounds = {startX, startY, width, height};
    Vector2 mousePoint = GetMousePosition();

    Sensor *sensor;

    for(uint32_t i = 0; i < app->sensorCount; i++) {
        if(app->sensors[i].type == type) {
            sensor = &app->sensors[i];
        }
    }

    if(type == app->selectedSensor) {
        DrawRectangleRec(bounds, DARKGREY);
    }
    else if(CheckCollisionPointRec(mousePoint, bounds))
    {
        GuiDrawRectangle(bounds, 1, DARKBLUE, LIGHTBLUE);

        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            app->selectedSensor = type;
        }
    }

    DrawText(sensor->typeName, startX + 70, startY + 10, 20, BLACK);
    DrawText("Specific Data", startX + 70, startY + 30, 10, BLACK);

    // Draw the visualizer mini boxes

    Color lightVersion = sensor->color;
    lightVersion.a = 10;

    Rectangle miniRect = (Rectangle){startX + 10, startY + 10, 50, height - 20};
    GuiDrawRectangle(miniRect, 1, sensor->color, WHITE);

    for(int i = miniRect.x + 1; i < miniRect.x + miniRect.width - 1; i++) {
        int x = ((float)(i - miniRect.x - 1) / (float)miniRect.width) * (float)(MAX_SENSOR_VALUES - 1);
        int y = ((sensor->values[(x + sensor->offset) % MAX_SENSOR_VALUES] - sensor->minValue) / (sensor->maxValue - sensor->minValue)) * (float)miniRect.height;

        DrawPixel(i, miniRect.y + y, sensor->color);

        for(int j = y + 1; j < miniRect.height; j++) {
             DrawPixel(i, miniRect.y + j, lightVersion);
        }
    }
}

void drawSidePanel(App *app) {
    DrawLine(200, 30, 200, 570, DARKGREY);

    drawSensorSelection(app, IMU, 0);
    drawSensorSelection(app, BAROMETER, 1);
    drawSensorSelection(app, GNSS, 2);
}

void drawGraphSection(App *app) {
    Sensor *sensor;

    for(uint32_t i = 0; i < app->sensorCount; i++) {
        if(app->sensors[i].type == app->selectedSensor) {
            sensor = &app->sensors[i];
        }
    }

    //Rectangle textRectangleA = (Rectangle){220, 30, 560, 25};
    //DrawRectangleRec(textRectangleA, RED);

    DrawText(sensor->typeName, 220, 30, 30, BLACK);

    int size = MeasureText(sensor->productName, 20);
    DrawText(sensor->productName, 780 - size, 39, 20, BLACK);

    Rectangle bounds = {220, 80, 560, 300};

    GuiDrawRectangle(bounds, 1, DARKBLUE, WHITE);

    for(uint32_t i = 240; i < 220 + 560; i += 40) {
        DrawLine(i, 80 + 1, i, 380 - 1, LIGHTERBLUE);
    }

    for(uint32_t i = 100; i < 380; i += 20) {
        DrawLine(220 + 1, i, 220 + 560 - 1, i, LIGHTERBLUE);
    }

    int oldX, oldY;

    for(uint32_t i = 0; i < MAX_SENSOR_VALUES; i++) {
        int x = (int)(((float)i / (float)(MAX_SENSOR_VALUES - 1)) * 560.0f + 220.0f);
        float value = sensor->values[(i + sensor->offset) % MAX_SENSOR_VALUES];
        value = (value - sensor->minValue) / (sensor->maxValue - sensor->minValue);
        int y = (int)(value * 300.0f + 80.0f);

        if(i != 0) {
            Color lightVersion = sensor->color;
            lightVersion.a = 10;

            DrawLine(oldX, oldY, x, y, sensor->color);
            DrawTriangle((Vector2){x, y}, (Vector2){oldX, oldY}, (Vector2){oldX, 380.0f}, lightVersion);
            DrawTriangle((Vector2){x, y}, (Vector2){oldX, 380.0f}, (Vector2){x, 380.0f}, lightVersion);
        }

        oldX = x;
        oldY = y;
    }

    DrawText("60 Seconds", 220, 385, 10, DARKERGREY);
    DrawText("0", 780 - MeasureText("0", 10), 385, 10, DARKERGREY);

    DrawText("Value", 220, 410, 20, DARKERGREY);

    char valueText[20];
    sprintf(valueText, "%.2f", sensor->values[(sensor->offset - 1) % MAX_SENSOR_VALUES]);
    DrawText(valueText, 220, 435, 20, BLACK);

    DrawText("Speed", 330, 410, 20, DARKERGREY);
    DrawText("60.0 Hz", 330, 435, 20, BLACK);

    DrawText("Up Time", 220, 470, 20, DARKERGREY);
    uint32_t millis = sensor->upTimeMillis;
    char formattedTime[20];
    snprintf(formattedTime, sizeof(formattedTime), "%lu:%02lu:%02lu", millis / 3600000, (millis / 60000) % 60, (millis / 1000) % 60);
    DrawText(formattedTime, 220, 495, 20, BLACK);
}

void draw(App *app) {
    BeginDrawing();

    ClearBackground(OFFWHITE);

    drawSidePanel(app);
    drawGraphSection(app);

    EndDrawing();
}

void readSensorValues(App *app) {
    for(uint32_t i = 0; i < app->sensorCount; i++) {
        Sensor *sensor = &app->sensors[i];
        float value = sensor->readValue(sensor->data);
        sensor->values[sensor->offset] = value;
        sensor->offset = (sensor->offset + 1) % MAX_SENSOR_VALUES;
        sensor->upTimeMillis += 16;
    }
}

void mainLoop(App *app) {
    while (!WindowShouldClose())
    {
        readSensorValues(app);
        draw(app);
    }
}

void setup(App *app) {
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WIDTH, HEIGHT, "AeroDesign Sensors GUI");
    SetTargetFPS(60);
    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);
    
    app->sensorCount = 3;

    TestSensorData *sensorData = calloc(app->sensorCount, sizeof(TestSensorData));
    sensorData[0].a = 3.5f;
    sensorData[1].a = 1.44f;
    sensorData[2].a = 0.7f;
    sensorData[0].b = 0.2f;
    sensorData[1].b = 0.6f;
    sensorData[2].b = 0.33f;


    app->sensors = calloc(app->sensorCount, sizeof(Sensor));

    app->sensors[0] = (Sensor){
        .type = IMU, 
        .data = (void *)&sensorData[0], 
        .readValue = &TestSensorReadValue,
        .offset = 0,
        .minValue = -2.0f,
        .maxValue = 2.0f,
        .color = {17,125,187,255},
        .typeName = "IMU",
        .productName = "ICM-20948"
    };
    app->sensors[1] = (Sensor){
        .type = BAROMETER, 
        .data = (void *)&sensorData[1], 
        .readValue = &TestSensorReadValue,
        .offset = 0,
        .minValue = -2.0f,
        .maxValue = 2.0f,
        .color = {149,40,180,255},
        .typeName = "Barometer",
        .productName = "BMP390"
    };
    app->sensors[2] = (Sensor){
        .type = GNSS, 
        .data = (void *)&sensorData[2], 
        .readValue = &TestSensorReadValue,
        .offset = 0,
        .minValue = -2.0f,
        .maxValue = 2.0f,
        .color = {77,166,12,255},
        .typeName = "GNSS",
        .productName = "ZED-F9P"
    };
}

void cleanup(App *app) {
    free(app->sensors[0].data);
    free(app->sensors);
    free(app);
    CloseWindow();
}

int main()
{
    App *app = calloc(1, sizeof(App));

    setup(app);
    mainLoop(app);
    cleanup(app);
    
    return 0;
}