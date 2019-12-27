#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include "src/omilli/OMilli.h"
#include "src/tinycircuits/BMA250.h"       
#include "Accel3Thread.h"

//////////////////// XYZ ////////////////////

void XYZ::print() {
    om::print("{x:");
    om::print(this->x);

    om::print(",y:");
    om::print(this->y);

    om::print(",z:");
    om::print(this->z);
    om::println("}");
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
    
/////////////// SweepCycle /////////////////

#define MAX_CYCLE_TICKS 1500

SweepCycle::SweepCycle(char id, bool invert) 
    : id(id), invert(invert) 
{}

#define SWEEP_END 20
void SweepCycle::setHeading(int16_t rank, bool damped) {
    Heading h;
    if (damped) {
        h = HEADING_IDLE;
    } else if (rank <= SWEEP_END) {
        h = HEADING_LFT;
    } else if (rank <= 50) {
        h = HEADING_CTR_LFT;
    } else if (rank <= 100 - SWEEP_END) {
        h = HEADING_CTR_RHT;
    } else {
        h = HEADING_RHT;
    }
    if (invert) {
        h = -h;
    }
    if (h*heading < 0) {
      center = true;
    }
    heading = h;
    if (heading == nextHeading) {
        om::Ticks now = om::ticks();
        if (now - lastCycle > MAX_CYCLE_TICKS) {
            heading = HEADING_IDLE;
        }
        nextHeading = nextHeading == HEADING_RHT 
            ? HEADING_LFT : HEADING_RHT;
        lastCycle = now;
    }
}

void SweepCycle::print() {
    char buf[6];
    this->headingToString(buf);
    om::print(buf);
    om::print(" ");
    om::print(nextHeading);
}

void SweepCycle::headingToString(char *buf) {
    switch (heading) {
    case HEADING_LFT: sprintf(buf, "%c-:--", id); break;
    case HEADING_CTR_LFT: sprintf(buf, "-%c:--", id); break;
    case HEADING_IDLE: sprintf(buf, "--%c--", id); break;
    case HEADING_CTR_RHT: sprintf(buf, "--:%c-", id); break;
    case HEADING_RHT: sprintf(buf, "--:-%c", id); break;
    default: sprintf(buf, "??%c??", id); break;
    }
}

//////////////////// Accel3Thread ////////////////////

Accel3Thread accelThread; // Acceleromoter tracker

// Accelerometer sensor variables for the sensor and its values
BMA250 accel_sensor;

Accel3Thread::Accel3Thread(uint16_t msLoop, int16_t damping)
    : msLoop(msLoop), damping(damping)
{}

void Accel3Thread::setup() {
    id = 'M';
    Thread::setup();
    accel_sensor.begin(BMA250_range_2g, BMA250_update_time_32ms); 
    om::print("Accel3Thread.setup");
}

void Accel3Thread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
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
    xCycle.setHeading(rank.x, range.x<=damping);
    yCycle.setHeading(rank.y, range.y<=damping);
    zCycle.setHeading(rank.z, range.z<=damping);

    double temp = ((accel_sensor.rawTemp * 0.5) + 24.0);
    if (x == -1 && y == -1 && z == -1) {
        om::print("ERROR! NO BMA250 DETECTED!");
    }
}
