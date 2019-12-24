#ifndef LED_THREAD_H
#define LED_THREAD_H

#include "src/omilli/OMilli.h"
#include "src/fastled/FastLED.h"

#define LED_FRAMERATE_PLA 40 /* 25 fps */
#define SHOWLED_FADE0 0  /* 0% brightness after first period */
#define SHOWLED_FADE50 50  /* 50% brightness each period */

#define NUM_LEDS 1

typedef class LEDThread : om::Thread {
public:
    LEDThread();
    void setup(uint8_t port=I2CPORT_DISPLAY, uint16_t msLoop=LED_FRAMERATE_PLA);
    CRGB leds[NUM_LEDS];
    void show(int16_t effect, int16_t brightness=255, int16_t msPeriod=LED_FRAMERATE_PLA);

protected:
    void loop();
    uint8_t ledPin;
    int16_t msLoop;
    uint8_t port;
    int16_t msPeriod;
    int16_t effect = SHOWLED_FADE50;
    int16_t brightness = 255;
} LEDThread;

extern LEDThread ledThread;

#endif
