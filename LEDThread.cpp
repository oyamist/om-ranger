#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>                   // For using I2C communication
#include <Wireling.h>               // For interfacing with Wirelings
#include "src/omilli/Thread.h"
#include "src/omilli/OMilli.h"
#include "LEDThread.h"

LEDThread ledThread; 

LEDThread::LEDThread() {}

void LEDThread::setBrightness(uint8_t value) {
    brightness = value;
}

void LEDThread::show(int16_t effect, int16_t brightness, int16_t msPeriod) {
    this->brightness = brightness;
    this->msPeriod = msPeriod;
    this->effect = effect;
}

void LEDThread::setup(uint8_t port, uint16_t msLoop) {
    id = 'D';
    this->msLoop = msLoop;
    this.port = port;
    Thread::setup();
    Wireling.begin();
    Wireling.selectPort(port);  // 
    switch (port) {
    case 0: ledPin = A0; break;
    case 1: ledPin = A1; break;
    case 2: ledPin = A2; break;
    case 3: ledPin = A3; break;
    default: // INVALID PORT
        ledPin = A0;
        break;
    }
    FastLED.addLeds<WS2812, ledPin, COLOR_ORDER>(leds, NUM_LEDS);

    // Flash slow white on startup
    FastLED.setBrightness(brightness);
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(255, 255, 255);
    }
    msPeriod = 400;

    pinMode(ledPin, OUTPUT);
    FastLED.show();
}

void LEDThread::loop() {
    Wireling.selectPort(port);  
    if (brightness) {
        nextLoop.ticks = om::ticks() + MS_TICKS(msPeriod);
        FastLED.setBrightness(brightness);
        FastLED.show();
        brightness = (brightness * effect)/100;
    } else {
        nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    }
}
