#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>
#include "Accel3Thread.h"
#include "OLED042Thread.h"
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
    char ** lines = oledThread.lines;
    accelThread.xCycle.headingToString(lines[0]);
    accelThread.yCycle.headingToString(lines[1]);
    accelThread.zCycle.headingToString(lines[2]);
}
