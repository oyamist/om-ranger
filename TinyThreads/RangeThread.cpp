#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>
#include "Accel3Thread.h"
#include "LraThread.h"
#include "RangeThread.h"

using namespace tinythreads;
using namespace fireduino;

namespace tinythreads {

RangeThread rangeThread;

RangeThread::RangeThread(uint16_t msPeriod, uint8_t port)
    : msPeriod(msPeriod), port(port)
{}

void RangeThread::setup() {
    id = 'R';
    Thread::setup();
    serial_print("RangeThread.setup");
}

void RangeThread::loop() {
    nextLoop.ticks = ticks() + MS_TICKS(msPeriod);
    // Sweep ranging pulses within range
    // Static ranging pulses with period proportionate to range
    switch (accelThread.heading.x) {
        case -2: // left
            break;
        case -1: // center left
            break;
        case 0: // damped static ranging
            if ((nextLoop.loops % 32) == 0) { // 
                lraThread.setEffect(DRV2605_SHARP_TICK_3);
                fireduino::serial_print("static range");
            }
            break;
        case 1: // center right 
            break;
        case 2: // right
            lraThread.setEffect(DRV2605_SHARP_TICK_3);
            fireduino::serial_print("dynamic range");
            break;
    }
}

} // tinythreads
