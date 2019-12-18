#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include "RangeThread.h"

using namespace tinythreads;
using namespace fireduino;

RangeThread rangeThread;

// **Wireling boards attached through an Adapter board**
// Selects the correct address of the port being used in the Adapter board
void setWirelingPort(int port) {
  Wire.beginTransmission(0x70);
  Wire.write(0x04 + port);
  Wire.endTransmission();
}

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
    switch (accelThread.heading.x) {
        case -2:
            lraThread.setEffect(0);
            break;
        case -1:
            lraThread.setEffect(0);
            break;
        case 1:
            lraThread.setEffect(0);
            break;
        case 2:
            lraThread.setEffect(DRV2605_SHARP_TICK_3);
            break;
    }
}
