#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>
#include "src/tinycircuts/VL53L0X.h"    // Time-of-Flight Distance sensor
#include <Wireling.h>   // For interfacing with Wirelings
#include "Accel3Thread.h"
#include "OLED042Thread.h"
#include "LEDThread.h"
#include "LraThread.h"
#include "RangeThread.h"

using namespace om;

RangeThread rangeThread;
VL53L0X distanceSensor; 

RangeThread::RangeThread() {}

void RangeThread::setup(uint8_t port, uint16_t msLoop) {
    delay(200);              // Sensor Startup time
    id = 'R';
    Thread::setup();
    this->port = port;
    this->msLoop = msLoop;
    om::print("RangeThread.setup");

    om::setI2CPort(port); 
    distanceSensor.init();
    distanceSensor.setTimeout(500);
    distanceSensor.setMeasurementTimingBudget((msLoop-1)*1000);
    distanceSensor.startContinuous();
}

void RangeThread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    om::setI2CPort(port); 
    uint16_t dist = distanceSensor.readRangeContinuousMillimeters();
    om::print("dist:");
    om::println(dist);

    // Sweep ranging pulses within range
    // Static ranging pulses with period proportionate to range
    int32_t cycleTicks = om::ticks() - accelThread.xCycle.lastCycle;
    if (accelThread.xCycle.center ) {
        accelThread.xCycle.center = false;
        if (200 < cycleTicks && cycleTicks < 3000) {
            lraThread.setEffect(DRV2605_SHARP_CLICK_30);
            ledThread.leds[0] = CRGB(0,128,0);
            ledThread.show(SHOWLED_FADE50);
        }
    }
    
    strcpy(oledThread.lines[1], "");
    accelThread.xCycle.headingToString(oledThread.lines[2]);
    accelThread.yCycle.headingToString(oledThread.lines[3]);
    accelThread.zCycle.headingToString(oledThread.lines[4]);
}

