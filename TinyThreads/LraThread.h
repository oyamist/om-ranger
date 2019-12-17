#ifndef LRA_THREAD_H
#define LRA_THREAD_H

#include "Thread.h"

namespace tinythreads {

typedef class LraThread : Thread {
public:
    LraThread();
    void setup();
    void loop();
    void hum(uint8_t level);
    void setEffect(uint8_t effect);

protected:
    uint8_t phase;
    uint8_t level;
} LraThread;

} // namespace tinythreads

#endif
