#ifndef RANGE_THREAD_H
#define RANGE_THREAD_H

#include "Thread.h"
#include "Accel3Thread.h"

namespace tinythreads {

friend class Thread;

#define VL53L0X_PERIOD 33

typedef class RangeThread : Thread {
public:
    RangeThread(uint16_t msPeriod=VL53L0X_PERIOD);

protected:
    void setup();
    void loop();
} RangeThread;

extern RangeThread rangeThread;

} // namespace tinythreads

#endif
