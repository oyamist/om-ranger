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
    int16_t x=0, y=0, z=0;
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

#define SAMPLES 32
XYZ xyz[SAMPLES];
int iSample = 0;

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

void rankPrint(int v, char *s1, char *s2, char *s3, char *s4) {
    if (v <= 25) { 
        serial_print(s1);
    } else if (v <= 50) { 
        serial_print(s2);
    } else if (v <= 75) { 
        serial_print(s3);
    } else {
        serial_print(s4);
    }
}

void Accel3Thread::loop() {
    nextLoop.ticks = ticks() + MS_TICKS(32);
    accel_sensor.read();
    int x = accel_sensor.X;
    int y = accel_sensor.Y;
    int z = accel_sensor.Z;
    XYZ low;
    XYZ high;
    XYZ rank;
    XYZ curXYZ = xyz[iSample];
    rank.set(0,0,0);
    for (int i = 0; i < SAMPLES; i++) {
        if (x >= xyz[i].x) { rank.x++; }
        if (y >= xyz[i].y) { rank.y++; }
        if (z >= xyz[i].z) { rank.z++; }
    }
    rank.x = (100*rank.x)/SAMPLES;
    rank.y = (100*rank.y)/SAMPLES;
    rank.z = (100*rank.z)/SAMPLES;
    iSample = (iSample+1) % SAMPLES;
    xyz[iSample].set(x,y,z);

    double temp = ((accel_sensor.rawTemp * 0.5) + 24.0);
    if (x == -1 && y == -1 && z == -1) {
        serial_print("ERROR! NO BMA250 DETECTED!");
    } else { 
        if ((iSample % 10) == 0) {
            rankPrint(rank.x, " X---", " -X--", " --X-", " ---X");
            rankPrint(rank.y, " Y---", " -Y--", " --Y-", " ---Y");
            rankPrint(rank.z, " Z---", " -Z--", " --Z-", " ---Z");
            serial_print(" ");
            serial_print((int16_t)(xyz[iSample].x-curXYZ.x));
            serial_print("\n");
        }
        //printSamples();
    }
}
