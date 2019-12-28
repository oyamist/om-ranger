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
}
#define SLOWFLASH 10

void RangeThread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    om::setI2CPort(port); 
    SweepCycle * px = &accelThread.xCycle;
    SweepCycle * py = &accelThread.yCycle;
    SweepCycle * pz = &accelThread.zCycle;
    if (px->heading==HEADING_IDLE && 
        py->heading==HEADING_IDLE && 
        pz->heading==HEADING_IDLE) {
       ledThread.leds[0] = CRGB(0,0,0); // no contact
       return; // Motionless
    }
    uint16_t dist = distanceSensor.readRangeContinuousMillimeters();
    if (dist < minRange || maxRange < dist) {
        return;
    }
    if (loops % 30 == 1) {
        om::print("dist:");
        om::println(dist);
    }

    // Sweep ranging pulses within range
    // Static ranging pulses with period proportionate to range
    if (dist == 8190L) { // out of range
      //int32_t cycleTicks = om::ticks() - px->lastCycle;
      //if (px->center ) {
          //px->center = false;
          //if (200 < cycleTicks && cycleTicks < 3000) {
              //ledThread.leds[0] = CRGB(32,32,32); // no contact
              //ledThread.show(SHOWLED_FADE50);
          //}
      //}
    } else {
        uint32_t now = om::ticks();
        uint32_t cycleTicks = now - px->lastCycle;
        CRGB curLed = ledThread.leds[0];
        uint16_t brightness = 255;
        if (loops %8 == 0) {
        om::print("dist");
        om::println(dist);
        }
        if (dist < 250) { // Very close
            brightness = (loops % SLOWFLASH) < SLOWFLASH/2 ? 32 : 255;
            curLed = CRGB(0xff,0,0);
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); }
        } else if (dist < 400) {
            curLed = CRGB(0xff,0,0);                  
            if (loops % 2 == 0) {
                lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
            }
        } else if (dist < 650) { // Somewhat close
            curLed = CRGB(0xcc,0x33,0); 
            if (loops % 4 == 0) {
                lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
            }
        } else if (dist < 1000) {
            curLed = CRGB(0,0xaa,0);
            if (loops % 8 == 0) {
                lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
            }
        } else if (dist < 2000) {
            curLed = CRGB(0x11,0x66,0x11);
            if (loops % 16 == 0) {
                lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
            }
        }
        ledThread.brightness = brightness;
        if (curLed.r != ledThread.leds[0].r ||
          curLed.g != ledThread.leds[0].g ||
          curLed.b != ledThread.leds[0].b) {
          ledThread.leds[0] = curLed;
          ledThread.show(SHOWLED_FADE85);
        }
    }

    lastDist = dist;

    // Update OLED position display
    strcpy(oledThread.lines[1], "");
    px->headingToString(oledThread.lines[2]);
    py->headingToString(oledThread.lines[3]);
    pz->headingToString(oledThread.lines[4]);
}
