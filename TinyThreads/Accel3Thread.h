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

#define ACCEL_SAMPLES 32

typedef class Accel3Thread : Thread {
public:
    Accel3Thread(uint16_t msPeriod=32, int16_t damping=10);
    void setup();
    void loop();
    XYZ heading;

protected:
    int16_t damping;
    uint16_t msPeriod;
    XYZ xyz[ACCEL_SAMPLES];
    int iSample = 0;
    int8_t headingFromRank(int16_t rank, int16_t range);
} Accel3Thread;


extern Accel3Thread accelThread;
} // namespace tinythreads

#endif
