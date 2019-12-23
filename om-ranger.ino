 #include <Arduino.h>
#include <Wire.h>         // For I2C communication with sensor
#include <Wireling.h>               // For interfacing with Wirelings
#include "Accel3Thread.h"
#include "LraThread.h"
#include "RangeThread.h"
#include "OLED042Thread.h"
#include "LEDThread.h"

#define DISPLAY_OLED false

///////////////////// CHOOSE DEFAULT PIN CONFIGURATION ///////////

using namespace om;

void setup() { // run once, when the sketch starts
    // Serial I/O has lowest priority, so you may need to
    // decrease baud rate to fix Serial I/O problems.
    //SerialUSB.begin(38400); // short USB cables
    om::serial_begin(19200); // long USB cables
    om::println("om.setup()");
    
    Wire.begin();

    // Fast I2C for high bandwidth but short transmission lengths
    Wire.setClock(400000); 

    // Initialize
	accelThread.setup();
    lraThread.setup();
    rangeThread.setup();
    if (DISPLAY_OLED) { // Mutually exclusive I2CPORT_DISPLAY
        oledThread.setup(I2CPORT_DISPLAY, OLED_FRAMERATE);
    } else {
        ledThread.setup(I2CPORT_DISPLAY, LED_FRAMERATE_PLA);
    }
    om::println("threadRunner.setup");

    om::pinMode(LED_BUILTIN, OUTPUT);
    threadRunner.setup(LED_BUILTIN);
}

void loop() {	// run over and over again
    threadRunner.run();
}
