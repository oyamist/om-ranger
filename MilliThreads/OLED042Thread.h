
#ifndef OLED042_THREAD_H
#define OLED042_THREAD_H

#include "Thread.h"

namespace MilliThreads {

/* The OLED 0.42" screen is 72 by 40 pixels, 
 * with (0,0) at top left 
 */
typedef class OLED042Thread : Thread {
public:
    OLED042Thread(uint16_t msLoop=1000, uint8_t port=I2CPORT_OLED042);
    void setup();
    void initScreen(void);
    void setPixel(int px, int py);
    void clearOLED();

protected:
    void loop();
    int16_t msLoop;
    uint8_t port;
    uint8_t resetPin;
    int textPos = 0; 
} OLED042Thread;

extern OLED042Thread oledThread;

} // namespace MilliThreads

#endif
