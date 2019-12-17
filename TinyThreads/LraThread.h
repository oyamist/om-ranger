#ifndef LRA_THREAD_H
#define LRA_THREAD_H

#include "Thread.h"

namespace tinythreads {

typedef class LraThread : Thread {
public:
    LraThread();
    void setup();
    void loop();

protected:
} LraThread;

} // namespace tinythreads

#endif
