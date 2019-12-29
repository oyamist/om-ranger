#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>
#include "src/tinycircuits/VL53L0X.h"    // Time-of-Flight Distance sensor
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
    mode = MODE_IDLE;
}
#define SLOWFLASH 10

void RangeThread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    om::setI2CPort(port); 
    AxisState * px = &accelThread.xState;
    AxisState * py = &accelThread.yState;
    AxisState * pz = &accelThread.zState;
    if (px->heading==HEADING_IDLE && 
        py->heading==HEADING_IDLE && 
        pz->heading==HEADING_IDLE) {
       ledThread.leds[0] = CRGB(0,0,0); // no contact
       mode = MODE_IDLE;
    }
    mode = absval(py->valSlow) < absval(pz->valSlow) 
        ? MODE_SWEEP    // ranging forward
        : MODE_STEP;    // ranging down
    if (mode == MODE_IDLE) {
        return;
    }

    uint16_t d = distanceSensor.readRangeContinuousMillimeters();
    distFast = d * DIST_FAST + (1-DIST_FAST) * distFast;
    uint16_t dist = distFast;
    if (dist < minRange || maxRange < dist) {
        return;
    }
    if (loops % 16 == 0) {
        om::print("mode");
        om::print((int8_t)mode);
        om::print(" distFast");
        om::print(distFast);
        om::print(" d");
        om::println(d);
    }

    uint32_t now = om::ticks();
    uint32_t cycleTicks = now - px->lastState;
    CRGB curLed = ledThread.leds[0];
    uint8_t blue = mode == MODE_STEP ? 0x66 : 0;
    uint16_t brightness = 255;
    if (dist < 250) { // Very close
        brightness = (loops % SLOWFLASH) < SLOWFLASH/2 ? 32 : 255;
        curLed = CRGB(0xff,0,blue);
        lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
    } else if (dist < 400) {
        curLed = CRGB(0xff,0,blue);                  
        if (loops % 2 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    } else if (dist < 650) { // Somewhat close
        curLed = CRGB(0xcc,0x33,blue); 
        if (loops % 3 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    } else if (dist < 1000) {
        brightness = (loops % SLOWFLASH) < SLOWFLASH/2 ? 32 : 255;
        curLed = CRGB(0,0xaa,blue);
        if (loops % 5 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    } else if (dist < 2000) {
        curLed = CRGB(0,0xaa,blue);
        if (loops % 8 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    }
    ledThread.brightness = brightness;
    if (curLed.r != ledThread.leds[0].r ||
        curLed.g != ledThread.leds[0].g ||
        curLed.b != ledThread.leds[0].b) 
    {
        ledThread.leds[0] = curLed;
        ledThread.show(SHOWLED_FADE85);
    }

    lastDist = dist;

    // Update OLED position display
    strcpy(oledThread.lines[1], "");
    px->headingToString(oledThread.lines[2]);
    py->headingToString(oledThread.lines[3]);
    pz->headingToString(oledThread.lines[4]);
}
