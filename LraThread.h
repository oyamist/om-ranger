#ifndef LRA_THREAD_H
#define LRA_THREAD_H

#include "src/omilli/Thread.h"

#define DRV2605_STRONG_CLICK_30 3
#define DRV2605_SHARP_CLICK 4
#define DRV2605_SHARP_TICK_1 24
#define DRV2605_SHARP_TICK_2 25
#define DRV2605_SHARP_TICK_3 26
#define DRV2605_SOFT_FUZZ_60 13
#define DRV2605_SHARP_CLICK_30 6
#define DRV2605_SOFT_BUMP_100 7
#define DRV2605_SOFT_BUMP_60 8
#define DRV2605_SOFT_BUMP_30 9
#define DRV2605_DOUBLE_CLICK_1 10
#define DRV2605_TRIPLE_CLICK 12
#define DRV2605_SHORT_DOUBLE_CLICK_STRONG_1 27
#define DRV2605_SHORT_DOUBLE_CLICK_STRONG_2 28
#define DRV2605_SHORT_DOUBLE_CLICK_STRONG_3 29
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
#define DRV2605_TRANSITION_CLICK_6 63
#define DRV2605_TRANSITION_HUM_1 64
#define DRV2605_TRANSITION_HUM_2 65
#define DRV2605_TRANSITION_HUM_3 66
#define DRV2605_TRANSITION_HUM_4 67
#define DRV2605_TRANSITION_HUM_5 68
#define DRV2605_TRANSITION_HUM_6 69
#define DRV2605_TRANSITION_RAMP_UP_LONG_SMOOTH_1 82
#define DRV2605_TRANSITION_RAMP_UP_LONG_SMOOTH_2 83
#define DRV2605_TRANSITION_RAMP_UP_LONG_SHARP_1 88
#define DRV2605_TRANSITION_RAMP_UP_LONG_SHARP_2 89
#define DRV2605_TRANSITION_RAMP_UP_LONG_SHARP_2 89
#define DRV2605_TRANSITION_RAMP_UP_SHORT_SHARP_1_100 92


typedef class LraThread : om::Thread {
public:
    LraThread(uint16_t msLoop=50, uint8_t port=I2CPORT_LRA);
    void buzz(uint8_t level);
    void setEffect(uint8_t effect, uint8_t count=1);
    void setup();

protected:
    uint8_t port;
    uint8_t count;
    uint8_t level;
    uint16_t msLoop;
    uint8_t effect;
    void playWaveform();
    void loop();
} LraThread;

extern LraThread lraThread;


#endif
