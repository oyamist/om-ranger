#ifndef MACHINETHREAD_H
#define MACHINETHREAD_H

#include "Thread.h"

namespace tinythreads {

typedef class Accel3Thread : Thread {
public:
    Accel3Thread();
    void setup();
    void loop();

protected:
} Accel3Thread;

} // namespace tinythreads

#endif
