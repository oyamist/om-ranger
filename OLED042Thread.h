
#ifndef OLED042_THREAD_H
#define OLED042_THREAD_H

#include "OMilli.h"

namespace om {

/* The OLED 0.42" screen is 72 by 40 pixels, with (0,0) at top left. 
 * With a 5x7 font, each character takes up 6x8 pixels, so we can
 * display 5 lines of 12 characters.
 */
#define OLED042_LINES 5
#define OLED042_LINECHARS 12

typedef class OLED042Thread : Thread {
public:
    OLED042Thread(uint16_t msLoop=1000, uint8_t port=I2CPORT_DISPLAY);
    void setup();
    void initScreen(void);
    void setPixel(int px, int py);
    void clearOLED();
    char lines[OLED042_LINES][OLED042_LINECHARS+1];

protected:
    void loop();
    int16_t msLoop;
    uint8_t port;
    uint8_t resetPin;
    int textPos = 0; 
} OLED042Thread;

extern OLED042Thread oledThread;

} // namespace om

#endif
