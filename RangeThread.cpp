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

#define STEP_CAL_LOOPS 60
#define STEP_CAL_TC 0.5
#define STEP_TC 0.5
#define STEP_LOOPS 5
#define MS_MODELOCK 1000 /* For modes with timeouts */
#define DEG_HORIZONTAL 10
#define STEADY_IDLE_MS 2000
#define PITCH_SELFTEST 80
#define PITCH_CAL -70
#define STEADY_DIST 35
#define SLEEP_DIST 50
#define MS_INTERMEASUREMENT 1
#define CAL_FLOOR_DT 400L
#define CALIBRATION_DELTA 30


char * modeStr[] = {
  "SLEEP ",
  "SLFTST",
  "SWEEP ",
  "CALIB ",
};

char * notifyStr[] = {
    "SLEEPING",
    "BUSY    ",
    "OK      ",
    "TOUCHING",
    "INCOMING",
    "SWEEPING",
    "ERANGE  ",
    "EACCEL  ",
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
    uint32_t msTimingBudget = msLoop-MS_INTERMEASUREMENT;
    distanceSensor.setMeasurementTimingBudget(msTimingBudget*1000);
    distanceSensor.startContinuous(MS_INTERMEASUREMENT); // 19mA

    setMode(MODE_SELFTEST);
}

void RangeThread::notify(NotifyType value, int8_t level) {
    if (lastNotify != value) {
        loopsNotify = loops;
    }
    uint16_t diffLoops = loops - loopsNotify;
    uint16_t mod16 = diffLoops % 16;
    uint16_t mod24 = diffLoops % 24;
    uint16_t mod32 = diffLoops % 32;
    uint16_t mod48 = diffLoops % 48;
    uint16_t mod64 = diffLoops % 64;
    int16_t showLed = SHOWLED_FADE85;
    CRGB led = ledThread.leds[0];
    uint8_t brightness = ledThread.brightness;
    
    switch (value) {
    case NOTIFY_SWEEP:
        if (mod16 % 0 == 0) {
            lraThread.setEffect(0); 
            led = CRGB(0,0,0);
            brightness = 0;
        }
        break;
    case NOTIFY_SLEEP:
        if (loopsNotify == loops) {
            lraThread.setEffect(DRV2605_TRANSITION_RAMP_DOWN_LONG_SMOOTH_1); 
            led = CRGB(0xff,0xff,0xff);
            brightness = 0xff;
        }
        break;
    case NOTIFY_INCOMING:
        if (mod16 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_100); 
            led = CRGB(0xff,0,0);
            brightness = 0xff;
        }
        break;
    case NOTIFY_TOUCHING:
        if (mod24 == 0) {
            showLed = SHOWLED_FADE85;
            lraThread.setEffect(DRV2605_STRONG_CLICK_100); 
            led = CRGB(0xff,0xff,0);
            brightness = 0xff;
        } else if (level == 2 && mod24 % 12 == 0) {
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
            led = CRGB(0xff,0,0);
            brightness = 0x80;
        } else if (level == 3 && mod24 % 8 == 0) {
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
            led = CRGB(0xff,0,0);
            brightness = 0x80;
        } else if (level == 4 && mod24 % 6 == 0) {
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
            led = CRGB(0xff,0,0);
            brightness = 0x80;
        } else if (level == 5 && mod24 % 4 == 0) {
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
            led = CRGB(0xff,0,0);
            brightness = 0x80;
        }
        break;
    case NOTIFY_BUSY:
        if (mod24 == 0) {
            showLed = SHOWLED_FADE85;
            lraThread.setEffect(DRV2605_STRONG_CLICK_100); 
            led = CRGB(0xff,0,0xff);
            brightness = 0xff;
        } else if (mod24 % 6 == 0) {
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
            led = CRGB(0,0,0xff);
            brightness = 0x80;
        }
        break;
    case NOTIFY_OK:
        if (mod48 == 0) {
            lraThread.setEffect(DRV2605_SOFT_FUZZ_60); 
            led = CRGB(0,0xff,0);
            brightness = 0xff;
        }
        break;
    default:
    case NOTIFY_EACCEL: // SOSOSOSOS...
    case NOTIFY_ERANGE: // SOSOSOSOS...
        showLed = SHOWLED_FADE50;
        if (mod64==0 || mod64==4 || mod64==8) {
            lraThread.setEffect(DRV2605_SHARP_TICK_1); 
            led = CRGB(0xff,0,0);
            brightness = 0xff;
        } else if (mod64==22 || mod64==34 || mod64==46) {
            lraThread.setEffect(DRV2605_SHARP_TICK_1); 
            led = CRGB(0xff,0,0);
            brightness = 0xff;
        }
        break;
    }

    bool updateDisplay = led != ledThread.leds[0] || 
        brightness != ledThread.brightness;
    if (updateDisplay) {
        ledThread.brightness = brightness;
        ledThread.leds[0] = led;
        ledThread.show(showLed);
    }
    lastNotify = value;
}

void RangeThread::calibrateLength(uint16_t d){
    int32_t diffCal = absval(d - eaDistSlow);
    distCal = expAvg(d, distCal, EATC_2);
    if (diffCal <= CALIBRATION_DELTA);
        distStick = distCal + CALIBRATION_DELTA;
        msModeLock = 0;
        setMode(MODE_SWEEP);
    } else {
        notify(NOTIFY_BUSY);
        msModeLock = msNow + MS_MODELOCK;
    }
}

void RangeThread::sweep(uint16_t d){
    if (d > distStick) {
        notify(NOTIFY_SWEEP);
    } else if (d < distStick*1/5) {
        notify(NOTIFY_TOUCHING, 5);
    } else if (d < distStick*2/5) {
        notify(NOTIFY_TOUCHING, 4);
    } else if (d < distStick*3/5) {
        notify(NOTIFY_TOUCHING, 3);
    } else if (d < distStick*4/5) {
        notify(NOTIFY_TOUCHING, 2);
    } else {
        notify(NOTIFY_TOUCHING, 1);
    }
}

void RangeThread::selftest(uint16_t d){
    bool okRange = d == 65535 ? false : true;
    bool okAccel = -90 <= pitch && pitch <= 90;

    if (okRange && okAccel) {
        notify(NOTIFY_OK);
    } else {
        if (!okRange) {
            notify(NOTIFY_ERANGE);
        } else if (!okAccel) {
            notify(NOTIFY_EACCEL);
        }
    }
}

void RangeThread::setMode(ModeType mode, bool force) {
    if (this->mode == mode && !force) {
        return;
    }

    uint32_t msNow = om::millis();

    switch (mode) {
    case MODE_SELFTEST:
        msModeLock = msNow + MS_MODELOCK;
        break;
    case MODE_SLEEP:
        // stopContinuous() can't be restarted?
        // so just slow down ranging
        distanceSensor.startContinuous(msLoop*100L); 
        monitor.quiet(true);
        msModeLock = msNow + MS_MODELOCK;
        break;
    case MODE_CALIBRATE: 
        msModeLock = msNow + MS_MODELOCK;
        break;
    case MODE_SWEEP:
        if (this->mode == MODE_SLEEP) {
            msIdle = msNow;
            monitor.quiet(false);
            distanceSensor.startContinuous(MS_INTERMEASUREMENT); // 19mA
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
    if (d < minRange) { // disregard ghost returns from enclosure
        d = maxRange;
    }
    if (maxRange < d) { // reduce average bias
        d = maxRange;
    }
    bool horizontal = -DEG_HORIZONTAL <= pitch && pitch <= DEG_HORIZONTAL;
    float err = abs(d - eaDistSlow);
    eaDistErr = expAvg(err, eaDistErr, EATC_6);
    eaDistFast = expAvg(d, eaDistFast, EATC_0);
    eaDistSlow = expAvg(d, eaDistSlow, EATC_4);
    eaDistSleep = expAvg(d, eaDistSleep, EATC_6);
    bool flatStill = horizontal && msNow - msUnsteady > STEADY_IDLE_MS;
    bool modeLock = om::millis() <= msModeLock;
    bool testing = mode == MODE_SELFTEST && modeLock;
    bool startTesting = pitch >= PITCH_SELFTEST;
    bool calibrating = mode == MODE_CALIBRATE && modeLock;
    bool startCalibrating = mode == MODE_SELFTEST && pitch <= PITCH_CAL;
    bool sleeping = mode == MODE_SLEEP && modeLock;
    bool startSleep = eaDistSleep < SLEEP_DIST;
    // Chose mode of operation
    if (startSleep || sleeping) {
        setMode(MODE_SLEEP, !sleeping);
        if (flatStill) {
            msModeLock = om::millis() + MS_MODELOCK;
        }
    } else if (startTesting || testing ) {
        setMode(MODE_SELFTEST, !testing);
        if (startTesting) {
            msModeLock = om::millis() + MS_MODELOCK;
        }
    } else if (startCalibrating || calibrating) {
        setMode(MODE_CALIBRATE, !calibrating);
        if (pitch < PITCH_CAL) {
            msModeLock = om::millis() + MS_MODELOCK;
        }
    } else {
        setMode(MODE_SWEEP);
    }

    if (loops % 16 == 0) {
        om::print(modeStr[(int8_t) mode]);
        om::print(" ");
        om::print(notifyStr[(uint8_t)lastNotify]);
        om::print(" d");
        om::print(d);
        om::print(" eaDistSlow:");
        om::print(eaDistSlow);
        om::print(" distStick:");
        om::print(distStick);
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
        selftest(d);
    } else if (mode == MODE_SWEEP) {
        sweep(eaDistSlow);
        updateOledPosition();
    } else if (mode == MODE_CALIBRATE) {
        calibrateLength(d);
    } else if (mode == MODE_SLEEP) {
        notify(NOTIFY_SLEEP);
    } else {
        om::print("UNKNOWN MODE");
        om::print((uint8_t) mode);
        om::println();
    }
}
