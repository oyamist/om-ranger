#ifndef ACCEL3_THREAD_H
#define ACCEL3_THREAD_H

#include "Thread.h"

namespace tinythreads {

typedef struct XYZ {
    int16_t x=0, y=0, z=0;
    void set(int x, int y, int z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    struct XYZ mapMin(XYZ that);
    struct XYZ mapMax(XYZ that);
    inline XYZ operator+(XYZ that) {
        XYZ xyz;
        xyz.x = x + that.x;
        xyz.y = y + that.y;
        xyz.z = z + that.z;
        return xyz;
    }
    inline XYZ operator-(XYZ that) {
        XYZ xyz;
        xyz.x = x - that.x;
        xyz.y = y - that.y;
        xyz.z = z - that.z;
        return xyz;
    }
    inline XYZ operator-() {
        XYZ xyz;
        xyz.x = -x;
        xyz.y = -y;
        xyz.z = -z;
        return xyz;
    }

    void print();
} XYZ;

typedef int8_t Heading;
#define HEADING_LFT     -2
#define HEADING_CTR_LFT -1
#define HEADING_IDLE     0
#define HEADING_CTR_RHT  1
#define HEADING_RHT      2

typedef struct SweepCycle {
    uint16_t cycles = 0;
    Ticks lastCycle = 0;
    Heading nextHeading = HEADING_RHT;
    Heading heading = HEADING_IDLE;

    void setHeading(int16_t rank, bool damped);
} SweepCycle;

#define ACCEL_SAMPLES 32

typedef class Accel3Thread : Thread {
public:
    Accel3Thread(uint16_t msLoop=32, int16_t damping=10);
    void setup();
    void loop();
    SweepCycle xCycle;
    SweepCycle yCycle;
    SweepCycle zCycle;

protected:
    int16_t damping;
    uint16_t msLoop;
    XYZ xyz[ACCEL_SAMPLES];
    int iSample = 0;
} Accel3Thread;


extern Accel3Thread accelThread;
} // namespace tinythreads

#endif
