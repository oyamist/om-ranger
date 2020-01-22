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

#define SLOWFLASH 10
#define FASTFLASH 5
#define STEP_CAL_LOOPS 60
#define STEP_CAL_TC 0.5
#define STEP_TC 0.5
#define STEP_LOOPS 5
#define MS_SELFTEST 2000 /* Selftest duration */


char * modeStr[] = {
  "SLEEP ",
  "SLFTST",
  "SWEEP ",
  "CALIB ",
};

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

    
analogReadResolution(12); // BATTERY
analogReference(AR_INTERNAL1V0); // BATTERY

    setMode(MODE_SELFTEST);
}

void RangeThread::notify(
    NotifyType value, CRGB &curLed, uint8_t brightness) {
    uint16_t diffLoops = loops - loopsNotify;
    uint16_t mod16 = diffLoops % 16;
    uint16_t mod48 = diffLoops % 48;
    uint16_t mod64 = diffLoops % 64;
    bool updateDisplay = curLed != ledThread.leds[0] || 
        brightness != ledThread.brightness;
    
    switch (value) {
    case NOTIFY_INRANGE:
        if (loops % STEP_LOOPS == phase) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_100); 
        } else {
            updateDisplay = false;
        }
        break;
    case NOTIFY_CLOSEST:
        lraThread.setEffect(DRV2605_SHARP_TICK_1); 
        break;
    case NOTIFY_BUSY:
        if (mod16 == 0) {
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
        } else {
            updateDisplay = false;
        }
        break;
    case NOTIFY_OK:
        if (mod48 == 0) {
            lraThread.setEffect(DRV2605_SOFT_FUZZ_60); 
        } else {
            updateDisplay = false;
        }
        break;
    default:
    case NOTIFY_ERROR: // SOSOSOSOS...
        if (mod64==0 || mod64==4 || mod64==8) {
            lraThread.setEffect(DRV2605_SHARP_TICK_1); 
        } else if (mod64==22 || mod64==34 || mod64==46) {
            lraThread.setEffect(DRV2605_SHARP_TICK_1); 
        } else {
            updateDisplay = false;
        }
        break;
    }
    if (updateDisplay) {
        ledThread.brightness = brightness;
        ledThread.leds[0] = curLed;
        ledThread.show(SHOWLED_FADE85);
    }
}

#define CAL_FLOOR_DT 400L
#define PIVOT_DIST 300

void RangeThread::calibrateLength(uint16_t d){
    CRGB curLed = ledThread.leds[0];
    int32_t msRemaining = msCalFloor - om::millis();
    uint8_t brightness = 0xff;
    hCal = expAvg(h, hCal, EATC_2);
    if (msRemaining < 0) {
        curLed = CRGB(0x44, 0x44, 0xff);
        notify(NOTIFY_OK, curLed, brightness);
        hStick = hCal;
    } else if (msRemaining < CAL_FLOOR_DT) {
        curLed = CRGB(0x0, 0x0, 0xff);
        notify(NOTIFY_BUSY, curLed, brightness);
    } else if (msRemaining < 2*CAL_FLOOR_DT) {
        curLed = CRGB(0x0, 0x0, 0xee);
        notify(NOTIFY_BUSY, curLed, brightness);
    } else if (msRemaining < 3*CAL_FLOOR_DT) {
        curLed = CRGB(0x0, 0x0, 0xcc);
        notify(NOTIFY_BUSY, curLed, brightness);
    } else if (msRemaining < 4*CAL_FLOOR_DT) {
        curLed = CRGB(0x0, 0x0, 0xaa);
        notify(NOTIFY_BUSY, curLed, brightness);
    } else if (msRemaining < 5*CAL_FLOOR_DT) {
        curLed = CRGB(0x0, 0x0, 0x88);
        notify(NOTIFY_BUSY, curLed, brightness);
    } else if (msRemaining < 6*CAL_FLOOR_DT) {
        curLed = CRGB(0x0, 0x0, 0x44);
        notify(NOTIFY_BUSY, curLed, brightness);
    } else if (msRemaining < 7*CAL_FLOOR_DT) {
        curLed = CRGB(0x0, 0x0, 0x22);
        notify(NOTIFY_BUSY, curLed, brightness);
    } else {
        curLed = CRGB(0x0, 0x0, 0x11);
        notify(NOTIFY_BUSY, curLed, brightness);
    }
}

void RangeThread::sweep(uint16_t d){
    CRGB curLed = ledThread.leds[0];
    uint8_t blue = 0x33;
    bool distInc = eaDistFast > eaDistSlow;
    uint16_t brightness = 0xff;

    if (d > hStick || distInc) { 
        phase = (loops + 1) % STEP_LOOPS;
        brightness = 0x33;
        curLed = CRGB(0,0,blue);
    } else if (d > hStick*0.5) {
        if (loops % STEP_LOOPS == phase) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_100); 
        }
        curLed = CRGB(0xff,0,blue);
    } else {
        brightness = (loops % SLOWFLASH) < SLOWFLASH/2 ? 32 : 255;
        notify(NOTIFY_CLOSEST, curLed, brightness);
        curLed = CRGB(0xff,0,blue);
    }

    ledThread.brightness = brightness;
    if (curLed != ledThread.leds[0]) {
        ledThread.leds[0] = curLed;
        ledThread.show(SHOWLED_FADE85);
    }
}

void RangeThread::selftest(uint16_t d){
    CRGB curLed = ledThread.leds[0];
    bool okRange = d == 65535 ? false : true;
    bool okAccel = -90 < pitch && pitch < 90;
    uint8_t brightness = 0xff;


    if (okRange && okAccel) {
        curLed = CRGB(0, 0xff, 0);
        notify(NOTIFY_OK, curLed, brightness);
    } else {
        if (!okRange) {
            curLed = CRGB(0xff, 0, 0);
        } else if (!okAccel) {
            curLed = CRGB(0xff, 0xff, 0);
        }
        notify(NOTIFY_ERROR, curLed, brightness);
    }
}

void RangeThread::setMode(ModeType mode, bool force) {
    if (this->mode == mode && !force) {
        return;
    }

    uint32_t msNow = om::millis();

    loopsNotify = loops;

    switch (mode) {
    case MODE_SELFTEST:
        distanceSensor.startContinuous(msLoop); // 19mA
        msSelftest = msNow + MS_SELFTEST;
        break;
    case MODE_SLEEP:
        // stopContinuous() can't be restarted?
        // so just slow down ranging
        distanceSensor.startContinuous(msLoop*100L); 
        monitor.quiet(true);
        ledThread.leds[0] = CRGB(0xff, 0xff, 0xff);
        ledThread.show(SHOWLED_FADE50);
        break;
    case MODE_CALIBRATE: 
        msCalFloor = msNow + 8*CAL_FLOOR_DT;
        break;
    case MODE_SWEEP:
        if (this->mode == MODE_SLEEP) {
            msIdle = msNow;
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
#define PITCH_SELFTEST 80
#define PITCH_CAL -70
#define STEADY_DIST 35
#define SLEEP_DIST 50

void RangeThread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    om::setI2CPort(port); 
    uint32_t msNow = om::millis();
    double az = absval((double) pz->valFast); // either flat side up
    double ay = py->valFast;
    pitch = round(90-atan2(az, -ay) * 180 / PI);
    bool steady = px->heading==HEADING_STEADY && 
        py->heading==HEADING_STEADY && 
        pz->heading==HEADING_STEADY;
    if (!steady) { msUnsteady = msNow; }
    uint16_t d = distanceSensor.readRangeContinuousMillimeters();
    bool horizontal = -DEG_HORIZONTAL <= pitch && pitch <= DEG_HORIZONTAL;
    float err = abs(d - eaDistSlow);
    eaDistErr = expAvg(err, eaDistErr, EATC_6);
    uint16_t dist = d;
    if (minRange <= d && d <= maxRange) {
        eaDistFast = expAvg(d, eaDistFast, EATC_0);
        eaDistSlow = expAvg(d, eaDistSlow, EATC_4);
    } 
    eaDistSleep = expAvg(d, eaDistSleep, EATC_6);
    h = (eaDistSlow+PIVOT_DIST) * sin(-pitch * PI / 180.0);
    bool flatStill = horizontal && msNow - msUnsteady > STEADY_IDLE_MS;
    bool testing = om::millis() <= msSelftest;
    // Chose mode of operation
    if (eaDistSleep < SLEEP_DIST || flatStill) {
        setMode(MODE_SLEEP);
    } else if (pitch >= PITCH_SELFTEST || testing ) {
        setMode(MODE_SELFTEST, pitch >= PITCH_SELFTEST);
    } else if (pitch <= PITCH_CAL && mode != MODE_SWEEP) {
        if (testing) {
           setMode(MODE_CALIBRATE, true);
           msSelftest = 0;
        } else {
           setMode(MODE_CALIBRATE, eaDistErr > STEADY_DIST);        
        }
    } else {
        setMode(MODE_SWEEP);
    }

    if (loops % 16 == 0) {
        float canvalue=analogRead(ADC_BATTERY);
        float Vf=(canvalue/4095.0)*(1/1.032)*(1.0/(33.0/(68.0+33.0)));
        om::print(modeStr[(int8_t) mode]);
        om::print(" d");
        om::print(d);
        om::print(" Vf");
        om::print(Vf);
        om::print(" pitch:");
        om::print(pitch);
        for (int ix = 0; ix < HEADING_COUNT; ix++) {
            om::print(" "); 
            if (ix == (int) px->heading+2) {
                om::print(px->dir > 0.5 ? ">" : "<");
            } else {
                om::print("-");
            }
            
        }
        om::println();
    }

    if (mode == MODE_SELFTEST) {
        selftest(dist);
    } else if (mode == MODE_SWEEP) {
        sweep(dist);
        updateOledPosition();
    } else if (mode == MODE_CALIBRATE) {
        calibrateLength(dist);
    } else if (mode == MODE_SLEEP) {
        // Do nothing
    } else {
        om::print("UNKNOWN MODE");
        om::print((uint8_t) mode);
        om::println();
    }
}
