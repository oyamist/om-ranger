#ifndef LRA_THREAD_H
#define LRA_THREAD_H

#include "Thread.h"

namespace tinythreads {

#define DRV2605_LONG_DOUBLE_TICK_SHARP_1 44
#define DRV2605_LONG_DOUBLE_TICK_SHARP_2 45
#define DRV2605_LONG_DOUBLE_TICK_SHARP_3 46
#define DRV2605_BUZZ_1 47
#define DRV2605_BUZZ_2 48
#define DRV2605_BUZZ_3 49
#define DRV2605_BUZZ_4 50
#define DRV2605_BUZZ_5 51
#define DRV2605_PULSING_STRONG_1 52
#define DRV2605_PULSING_STRONG_2 53
#define DRV2605_PULSING_MEDIUM_1 54
#define DRV2605_PULSING_MEDIUM_2 55
#define DRV2605_PULSING_SHARP_1 56
#define DRV2605_PULSING_SHARP_2 57
#define DRV2605_TRANSITION_RAMP_UP_LONG_SMOOTH_1 82
#define DRV2605_TRANSITION_RAMP_UP_LONG_SMOOTH_2 83
#define DRV2605_TRANSITION_RAMP_UP_LONG_SHARP_1 88
#define DRV2605_TRANSITION_RAMP_UP_LONG_SHARP_2 89
#define DRV2605_TRANSITION_RAMP_UP_LONG_SHARP_2 89

typedef class LraThread : Thread {
public:
    LraThread();
    void setup();
    void loop();
    void buzz(uint8_t level);
    void setEffect(uint8_t effect);

protected:
    uint8_t phase;
    uint8_t level;
} LraThread;

} // namespace tinythreads

#endif
