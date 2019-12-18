#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include "fireduino.h"
#include "BMA250.h"       
#include "Accel3Thread.h"

using namespace tinythreads;
using namespace fireduino;

namespace tinythreads {
  
//////////////////// XYZ ////////////////////

void XYZ::print() {
    serial_print("{x:");
    serial_print(this->x);

    serial_print(",y:");
    serial_print(this->y);

    serial_print(",z:");
    serial_print(this->z);
    serial_println("}");
}

struct XYZ XYZ::mapMin(XYZ that) {
    XYZ result;
    result.x = minval(x, that.x);
    result.y = minval(y, that.y);
    result.z = minval(z, that.z);
    return result;
}
    
struct XYZ XYZ::mapMax(XYZ that) {
    XYZ result;
    result.x = maxval(x, that.x);
    result.y = maxval(y, that.y);
    result.z = maxval(z, that.z);
    return result;
}
    
//////////////////// Accel3Thread ////////////////////

Accel3Thread accelThread; // Acceleromoter tracker

// Accelerometer sensor variables for the sensor and its values
BMA250 accel_sensor;

Accel3Thread::Accel3Thread(uint16_t msPeriod, int16_t damping)
    : msPeriod(msPeriod), damping(damping)
{}

void Accel3Thread::setup() {
    id = 'M';
    Thread::setup();
    accel_sensor.begin(BMA250_range_2g, BMA250_update_time_32ms); 
    serial_print("Accel3Thread.setup");
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

int8_t Accel3Thread::headingFromRank(int16_t rank, int16_t range) {
    if (range <= damping) {
        return 0;
    }
    if (rank <= 25) {
        return -2;
    } else if (rank <= 50) {
        return -1;
    } else if (rank <= 75) {
        return 1;
    } else {
        return 2;
    }
}

void Accel3Thread::loop() {
    nextLoop.ticks = ticks() + MS_TICKS(msPeriod);
    accel_sensor.read();
    int x = accel_sensor.X;
    int y = accel_sensor.Y;
    int z = accel_sensor.Z;
    XYZ rank;
    XYZ curXYZ = xyz[iSample];
    XYZ minXYZ(curXYZ);
    XYZ maxXYZ(curXYZ);
    rank.set(0,0,0);
    for (int i = 0; i < ACCEL_SAMPLES; i++) {
        minXYZ = minXYZ.mapMin(xyz[i]);
        maxXYZ = maxXYZ.mapMax(xyz[i]);
        if (x >= xyz[i].x) { rank.x++; }
        if (y >= xyz[i].y) { rank.y++; }
        if (z >= xyz[i].z) { rank.z++; }
    }
    rank.x = (100*rank.x)/ACCEL_SAMPLES;
    rank.y = (100*rank.y)/ACCEL_SAMPLES;
    rank.z = (100*rank.z)/ACCEL_SAMPLES;
    iSample = (iSample+1) % ACCEL_SAMPLES;
    xyz[iSample].set(x,y,z);
    XYZ range = maxXYZ - minXYZ;
    heading.x = headingFromRank(rank.x, range.x);
    heading.y = headingFromRank(rank.y, range.y);
    heading.z = headingFromRank(rank.z, range.z);

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
    }
}

} // tinythreads
