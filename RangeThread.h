#ifndef RANGE_THREAD_H
#define RANGE_THREAD_H

#include "Thread.h"
#include "Accel3Thread.h"

#define VL53L0X_PERIOD 33

typedef class RangeThread : om::Thread {
public:
    RangeThread(uint16_t msLoop=VL53L0X_PERIOD, uint8_t port=2);
    void setup();
    
protected:
    uint8_t port;
    uint16_t msLoop;
    void loop();
} RangeThread;

extern RangeThread rangeThread;


#endif
