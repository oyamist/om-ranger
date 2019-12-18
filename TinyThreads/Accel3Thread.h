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

typedef class Accel3Thread : Thread {
public:
    Accel3Thread(uint16_t msPeriod=32);
    void setup();
    void loop();
    XYZ heading;

protected:
    uint16_t msPeriod;
    XYZ xyz[SAMPLES];
    int iSample = 0;
} Accel3Thread;

} // namespace tinythreads

extern Accel3Thread accelThread;

#endif
