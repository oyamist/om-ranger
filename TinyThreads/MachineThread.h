#ifndef MACHINETHREAD_H
#define MACHINETHREAD_H

#include "Thread.h"

namespace tinythreads {

typedef class MachineThread : Thread {
protected:

public:

public:
    MachineThread();
    void setup();
    void loop();
} MachineThread;

} // namespace tinythreads

#endif
