#include <stdio.h>
#include <windows.h>
#include <stdint.h>

#define SENSOR_VALUES 0
#define CALIBRATION_STATUS 1

typedef struct {
    float pressureBMP;
    float altitudeBMP;
    double airspeedMS4525D0;
    double quaternionBNO[4];
    float angVelocityBNO[3];
    float accelerationBNO[3];
    long latitudeZED;
    long longitudeZED;
    int northVelocityZED;
    int eastVelocityZED;
    int downVelocityZED;
    long timeStamp;
} sensors;

// Function to decode the byte array into the sensors struct
void decodeSensors(const uint8_t *data, sensors *decodedSensors) {
    const uint8_t *ptr = data;
    decodedSensors->pressureBMP = *(float*)ptr;
    ptr += sizeof(float);
    decodedSensors->altitudeBMP = *(float*)ptr;
    ptr += sizeof(float);
    decodedSensors->airspeedMS4525D0 = *(double*)ptr;
    ptr += sizeof(double);
    for(int i = 0; i < 4; ++i) {
        decodedSensors->quaternionBNO[i] = *(double*)ptr;
        ptr += sizeof(double);
    }
    for(int i = 0; i < 3; ++i) {
        decodedSensors->angVelocityBNO[i] = *(float*)ptr;
        ptr += sizeof(float);
    }
    for(int i = 0; i < 3; ++i) {
        decodedSensors->accelerationBNO[i] = *(float*)ptr;
        ptr += sizeof(float);
    }
    decodedSensors->latitudeZED = *(long*)ptr;
    ptr += sizeof(long);
    decodedSensors->longitudeZED = *(long*)ptr;
    ptr += sizeof(long);
    decodedSensors->northVelocityZED = *(int*)ptr;
    ptr += sizeof(int);
    decodedSensors->eastVelocityZED = *(int*)ptr;
    ptr += sizeof(int);
    decodedSensors->downVelocityZED = *(int*)ptr;
    ptr += sizeof(int);
    decodedSensors->timeStamp = *(long*)ptr;
}

/* Much shorter implentation that needs to be tested
void decodeSensors(const uint8_t *data, sensors *decodedSensors) {
    *decodedSensors = *(sensors*)data;
}*/

int initUART(LPCWSTR COMPort, HANDLE* hSerial) {
    DCB dcbSerialParams = { 0 };
    COMMTIMEOUTS timeouts = { 0 };

    *hSerial = CreateFile(COMPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

    if(*hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening serial port\n");
        return 1;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if(!GetCommState(*hSerial, &dcbSerialParams)) {
        printf("Error getting device state\n");
        CloseHandle(*hSerial);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_115200;  // Set baud rate to 115200
    dcbSerialParams.ByteSize = 8;           // 8 data bits
    dcbSerialParams.StopBits = ONESTOPBIT;  // 1 stop bit
    dcbSerialParams.Parity = NOPARITY;      // No parity

    if(!SetCommState(*hSerial, &dcbSerialParams)) {
        printf("Error setting device parameters\n");
        CloseHandle(*hSerial);
        return 1;
    }

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if(!SetCommTimeouts(*hSerial, &timeouts)) {
        printf("Error setting timeouts\n");
        CloseHandle(*hSerial);
        return 1;
    }

    return 0;
}

void printSensorValues(sensors* sensors) {
    printf("Pressure BMP: %f\n", sensors->pressureBMP);
    printf("Altitude BMP: %f\n", sensors->altitudeBMP);
    printf("Airspeed: %lf\n", sensors->airspeedMS4525D0);
    printf("Quaternion: (%lf, %lf, %lf, %lf)\n", sensors->quaternionBNO[0], sensors->quaternionBNO[1], sensors->quaternionBNO[2], sensors->quaternionBNO[3]);
    printf("Angular Velocity: (%f, %f, %f)\n", sensors->angVelocityBNO[0], sensors->angVelocityBNO[1], sensors->angVelocityBNO[2]);
    printf("Acceleration: (%f, %f, %f)\n", sensors->accelerationBNO[0], sensors->accelerationBNO[1], sensors->accelerationBNO[2]);
    printf("Latitude ZED: %ld\n", sensors->latitudeZED);
    printf("Longitude ZED: %ld\n", sensors->longitudeZED);
    printf("North Velocity ZED: %d\n", sensors->northVelocityZED);
    printf("East Velocity ZED: %d\n", sensors->eastVelocityZED);
    printf("Down Velocity ZED: %d\n", sensors->downVelocityZED);
    printf("Time Stamp: %ld\n", sensors->timeStamp);
}

void handleSensorValues(HANDLE* hSerial) {
    DWORD bytesRead;

    printf("Message received: SENSOR_VALUES\n");

    // Read the next portion of data (size of sensors struct)
    uint8_t data[sizeof(sensors)];
    if(!ReadFile(*hSerial, data, sizeof(data), &bytesRead, NULL) && bytesRead == sizeof(data)) {
        printf("Error reading sensor data\n");
        return;
        
    }

    sensors decoded;
    decodeSensors(data, &decoded);

    printSensorValues;
}

void readMessage(HANDLE* hSerial) {
    DWORD bytesRead;
    unsigned char messageType;

    // Read 1 byte from UART (message header)
    if(!ReadFile(*hSerial, &messageType, 1, &bytesRead, NULL)) {
        printf("Unable to read from UART\n");
        return;
    }

    if(bytesRead != 1) {
        printf("Unable to read header byte\n");
        return;
    }

    if(messageType == SENSOR_VALUES) {
        handleSensorValues(hSerial);
    }
    else if(messageType == CALIBRATION_STATUS) {
        printf("Message received: CALIBRATION_STATUS\n");
    }
    else {
        printf("Unknown message type received: 0x%02X\n", messageType);
    }
}

void cleanupUART(HANDLE* hSerial) {
    if(*hSerial != INVALID_HANDLE_VALUE) {
        return;
    }

    CloseHandle(*hSerial);
    *hSerial = INVALID_HANDLE_VALUE;
}

/*
int main() {
    HANDLE hSerial;

    if(initUART(L"COM4", &hSerial) != 0) {
        return 1;
    }

    while(1) {
        readMessage(&hSerial);
    }

    cleanupUART(&hSerial);

    return 0;
}
*/