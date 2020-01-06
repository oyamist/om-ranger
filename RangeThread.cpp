#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>
#include "src/tinycircuits/VL53L0X.h"    // Time-of-Flight Distance sensor
#include "src/omilli/Thread.h"
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
    distanceSensor.startContinuous(msLoop); // 19mA

    for (int i = 0; i < HEADING_COUNT; i++) {
        dhx[i] = 0;
    }

    mode = MODE_SLEEP;
}
#define SLOWFLASH 10
#define FASTFLASH 5

void RangeThread::sweepForward(uint16_t dist) {
    if (dist < minRange || maxRange < dist) {
        return;
    }

    uint32_t now = om::ticks();
    uint32_t cycleTicks = now - px->lastState;
    CRGB curLed = ledThread.leds[0];
    uint8_t blue = 0;
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
}

#define CAL_FLOOR_TC 0.5
void RangeThread::lraCalibrating(bool done) {
    if (done) {
        if (loops % 16 == 0) {
            lraThread.setEffect(DRV2605_TRANSITION_RAMP_DOWN_LONG_SMOOTH_1); 
        }
    } else {
        if (loops % 16 == 0) {
            lraThread.setEffect(DRV2605_SOFT_BUMP_100); 
        }
    }
}

#define CAL_FLOOR_DT 400L
#define WAND_DIST 170
#define ELBOW_DIST 300

void RangeThread::calFloor(uint16_t d){
    CRGB curLed = ledThread.leds[0];
    int32_t msRemaining = msCalFloor - om::millis();
    uint8_t brightness = 0xff;
    hCal = h*CAL_FLOOR_TC + (1-CAL_FLOOR_TC)*hCal;
    if (msRemaining < 0) {
        lraCalibrating(true);
        hFloor = hCal;
        curLed = CRGB(0x44, 0x44, 0xff);
    } else if (msRemaining < CAL_FLOOR_DT) {
        lraCalibrating(); 
        curLed = CRGB(0x0, 0x0, 0xff);
    } else if (msRemaining < 2*CAL_FLOOR_DT) {
        lraCalibrating(); 
        curLed = CRGB(0x0, 0x0, 0xee);
    } else if (msRemaining < 3*CAL_FLOOR_DT) {
        lraCalibrating(); 
        curLed = CRGB(0x0, 0x0, 0xcc);
    } else if (msRemaining < 4*CAL_FLOOR_DT) {
        lraCalibrating(); 
        curLed = CRGB(0x0, 0x0, 0xaa);
    } else if (msRemaining < 5*CAL_FLOOR_DT) {
        lraCalibrating(); 
        curLed = CRGB(0x0, 0x0, 0x88);
    } else if (msRemaining < 6*CAL_FLOOR_DT) {
        lraCalibrating(); 
        curLed = CRGB(0x0, 0x0, 0x44);
    } else if (msRemaining < 7*CAL_FLOOR_DT) {
        lraCalibrating(); 
        curLed = CRGB(0x0, 0x0, 0x22);
    } else {
        lraCalibrating(); 
        curLed = CRGB(0x0, 0x0, 0x11);
    }
    ledThread.brightness = brightness;
    if (curLed.r != ledThread.leds[0].r ||
        curLed.g != ledThread.leds[0].g ||
        curLed.b != ledThread.leds[0].b) 
    {
        ledThread.leds[0] = curLed;
        ledThread.show(SHOWLED_FADE85);
    }
}

#define STEP_DOWN -75
#define STEP_UP 75
#define STEP_CAL_LOOPS 60
#define STEP_CAL_TC 0.5
#define STEP_TC 0.5
 
void RangeThread::sweepStep(uint16_t d){
    uint32_t now = om::ticks();
    uint32_t cycleTicks = now - px->lastState;
    CRGB curLed = ledThread.leds[0];
    uint8_t blue = 0x33;
    int32_t dist = hFloor-h;
    uint16_t brightness = 0xff;
    int32_t dh = h - hFloor;
    if (minRange <= d && d <= maxRange) {
        dhxAvg = (dhx[0]+dhx[1]+dhx[3]+dhx[4])/4;
        switch (px->heading) {
        case HEADING_LFT:
            dhx[0] = STEP_TC*dh + (1-STEP_TC)*dhx[0];
            break;
        case HEADING_CTR_LFT:
            dhx[1] = STEP_TC*dh + (1-STEP_TC)*dhx[1];
            break;
        case HEADING_STEADY:
            dhx[2] = STEP_TC*dh + (1-STEP_TC)*dhx[2];
            break;
        case HEADING_CTR_RHT:
            dhx[3] = STEP_TC*dh + (1-STEP_TC)*dhx[3];
            break;
        case HEADING_RHT:
            dhx[4] = STEP_TC*dh + (1-STEP_TC)*dhx[4];
            break;
        }
    }
    if (dist < STEP_DOWN) {
        lraThread.setEffect(DRV2605_TRANSITION_RAMP_DOWN_SHORT_SHARP_1); 
        curLed = CRGB(0xff,0,blue);
   } else if (dist < STEP_UP) {
        curLed = CRGB(0,0,blue);
        uint16_t brightness = 0x88;
        // do nothing
    } else if (dist > 450) { 
        brightness = (loops % SLOWFLASH) < SLOWFLASH/2 ? 32 : 255;
        curLed = CRGB(0xff,0,blue);
        lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
    } else if (dist > 350) {
        curLed = CRGB(0xff,0,blue);                  
        if (loops % 2 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    } else if (dist > 250) {
        brightness = (loops % SLOWFLASH) < SLOWFLASH/2 ? 32 : 255;
        curLed = CRGB(0xcc,0x33,blue);                  
        if (loops % 3 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    } else {
        curLed = CRGB(0xcc,0x33,blue); 
        if (loops % 5 == 0) {
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
}

void RangeThread::setMode(ModeType mode) {
    if (this->mode == mode) {
        return;
    }

    switch (mode) {
    case MODE_SLEEP:
        om::println("Idling...");
        // stopContinuous() can't be restarted?
        // so just slow down ranging
        distanceSensor.startContinuous(msLoop*100L); 
        monitor.quiet(false);
        ledThread.leds[0] = CRGB(0xff, 0xff, 0xff);
        ledThread.show(SHOWLED_FADE85);
        break;
    case MODE_CAL_FLOOR: 
        msCalFloor = om::millis() + 8*CAL_FLOOR_DT;
        break;
    case MODE_SWEEP_FORWARD:
    case MODE_SWEEP_STEP:
        if (this->mode == MODE_SLEEP) {
            msIdle = om::millis();
            om::println("Activating...");
            monitor.quiet(false);
            distanceSensor.startContinuous(msLoop); // 19mA
        }
        break;
    }
    this->mode = mode;
}

void RangeThread::updateOledPosition() {
    // Update OLED position display
    strcpy(oledThread.lines[1], "");
    px->headingToString(oledThread.lines[2]);
    py->headingToString(oledThread.lines[3]);
    pz->headingToString(oledThread.lines[4]);
}

#define DEG_HORIZONTAL 10
#define STEADY_IDLE_MS 2000
#define PITCH_STEP -25
#define STEADY_DIST 15
#define STEADY_X 15
#define SLEEP_DIST 100
#define DIST_FAST 0.5
#define DIST_SLOW 0.15
#define DIST_SLEEP 0.075 


void RangeThread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    om::setI2CPort(port); 
    double az = absval((double) pz->valFast); // either flat side up
    double ay = py->valFast;
    pitch = round(90-atan2(az, -ay) * 180 / PI);
    bool steady = px->heading==HEADING_STEADY && 
        py->heading==HEADING_STEADY && 
        pz->heading==HEADING_STEADY;
    uint16_t d = distanceSensor.readRangeContinuousMillimeters();
    bool horizontal = -DEG_HORIZONTAL <= pitch && pitch <= DEG_HORIZONTAL;
    uint32_t msNow = om::millis();
    uint16_t dist = d;
    if (minRange <= d && d <= maxRange) {
        distFast = d * DIST_FAST + (1-DIST_FAST) * distFast;
        distSlow = d * DIST_SLOW + (1-DIST_SLOW) * distSlow;
    } 
    distSleep = d * DIST_SLEEP + (1-DIST_SLEEP) * distSleep;
    int32_t dFastSlow = distFast - distSlow;
    bool steadyDist = absval(dFastSlow) < STEADY_DIST;
    h = (distSlow+WAND_DIST) * sin(-pitch * PI / 180.0);
    int32_t xRange = px->maxVal - px->minVal;
    bool steadyX = xRange < STEADY_X; 
    bool still = horizontal && msNow - msUnsteady > STEADY_IDLE_MS;
    if (distSleep < SLEEP_DIST || steady && still) {
        setMode(MODE_SLEEP);
    } else if (steady) {
        if (pitch <= PITCH_STEP && steadyDist && steadyX) {
            setMode(MODE_CAL_FLOOR);        
        }
    } else {
        msUnsteady = msNow;
        if (pitch > PITCH_STEP) {
            setMode(MODE_SWEEP_FORWARD);
        } else if (steadyDist && steadyX) {
            setMode(MODE_CAL_FLOOR);
        } else {
            setMode(MODE_SWEEP_STEP);
        }
    }
    if (loops % 3 == 0) {
     //   om::print("mode:");
     //   om::print((uint8_t)mode);
 //       om::print(" pitch:");
   //     om::print(pitch);
     //   om::print(" d:");
       // om::print(d);
        //om::print(" hFloor:");
        //om::print(hFloor);
        //om::print(" dhxAvg:");
        char *dir = "<";
        if (px->dir > 0.5) {
            dir = ">";
        }
        for (int ix = 0; ix < HEADING_COUNT; ix++) {
            om::print(" "); 
            if (ix == (int) px->heading+2) {
                om::print(dir);
            } else {
                om::print("-");
            }
            
        }
        om::println();
    }

    if (mode == MODE_SWEEP_FORWARD) {
        sweepForward(dist);
        updateOledPosition();
    } else if (mode == MODE_SWEEP_STEP) {
        sweepStep(dist);
        updateOledPosition();
    } else if (mode == MODE_CAL_FLOOR) {
        calFloor(dist);
    } else if (mode == MODE_SLEEP) {
        // Do nothing
    } else {
        om::print("UNKNOWN MODE");
        om::print((uint8_t) mode);
        om::println();
    }
}
