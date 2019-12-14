#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include "BMA250.h"       
#include "Accel3Thread.h"

using namespace tinythreads;
using namespace fireduino;

// Accelerometer sensor variables for the sensor and its values
BMA250 accel_sensor;

typedef struct XYZ {
    int x, y, z;
    void set(int x, int y, int z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    void print() {
        serial_print("{x:");
        serial_print(this->x);

        serial_print(",y:");
        serial_print(this->y);

        serial_print(",z:");
        serial_print(this->z);
        serial_println("}");
    }
} XYZ;

#define SAMPLES 10
XYZ xyz[SAMPLES];
int iSample = 0;
float kHighLow = 0.032;
float kAvg = 0.05;
float aHigh = 0;
float aLow = 0;
float aAvg = 0;

Accel3Thread::Accel3Thread(){
}

void Accel3Thread::setup() {
    id = 'M';
    Thread::setup();
    accel_sensor.begin(BMA250_range_2g, BMA250_update_time_32ms); 
    serial_print("Accel3Thread.setup");
}

void printSamples() {
    if (iSample == 0) {
        for (int i = 0; i<SAMPLES;i++) {
          xyz[i].print();
        }
    }
}

void Accel3Thread::loop() {
    nextLoop.ticks = ticks() + MS_TICKS(32);
    accel_sensor.read();
    int x = accel_sensor.X;
    int y = accel_sensor.Y;
    int z = accel_sensor.Z;
    xyz[iSample].set(x,y,z);
    iSample = (iSample+1) % SAMPLES;

    if (x > aAvg) {
        aHigh = kHighLow * x + (1-kHighLow)*aHigh;
    } else {
        aLow = kHighLow * x + (1-kHighLow)*aLow;
    }
    aAvg = kAvg * (aHigh+aLow)/2 + (1-kAvg)*aAvg;

    double temp = ((accel_sensor.rawTemp * 0.5) + 24.0);
    if (x == -1 && y == -1 && z == -1) {
        serial_print("ERROR! NO BMA250 DETECTED!");
    } else { 
        if (iSample === 0) {
            serial_print("  avg:");
            serial_print(aAvg);
            serial_print("  high:");
            serial_print(aHigh);
            serial_print("  low:");
            serial_println(aLow);
        }
        //printSamples();
    }
}
