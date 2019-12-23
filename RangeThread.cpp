#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>
#include "Accel3Thread.h"
#include "OLED042Thread.h"
#include "LEDThread.h"
#include "LraThread.h"
#include "RangeThread.h"

using namespace om;
using namespace om;

RangeThread rangeThread;

RangeThread::RangeThread(uint16_t msLoop, uint8_t port)
    : msLoop(msLoop), port(port)
{}

void RangeThread::setup() {
    id = 'R';
    Thread::setup();
    om::print("RangeThread.setup");
}

void RangeThread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    // Sweep ranging pulses within range
    // Static ranging pulses with period proportionate to range
    int32_t cycleTicks = om::ticks() - accelThread.xCycle.lastCycle;
    if (accelThread.xCycle.center ) {
        accelThread.xCycle.center = false;
        if (0 < cycleTicks && cycleTicks < 3000) {
            lraThread.setEffect(DRV2605_SHARP_TICK_3);
            ledThread.leds[0] = CRGB(0,255,0);
            ledThread.show(SHOWLED_FADE50);
        }
    }
    strcpy(oledThread.lines[1], "");
    accelThread.xCycle.headingToString(oledThread.lines[2]);
    accelThread.yCycle.headingToString(oledThread.lines[3]);
    accelThread.zCycle.headingToString(oledThread.lines[4]);
}
