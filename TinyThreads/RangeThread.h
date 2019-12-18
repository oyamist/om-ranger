#ifndef RANGE_THREAD_H
#define RANGE_THREAD_H

#include "Thread.h"
#include "Accel3Thread.h"

namespace tinythreads {

#define VL53L0X_PERIOD 33

typedef class RangeThread : Thread {
public:
    RangeThread(uint16_t msPeriod=VL53L0X_PERIOD, uint8_t port=2);
    void setup();
    
protected:
    uint8_t port;
    uint16_t msPeriod;
    void loop();
} RangeThread;

extern RangeThread rangeThread;

} // namespace tinythreads

#endif
