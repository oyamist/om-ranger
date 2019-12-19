#include <Arduino.h>
#include <Wire.h>         // For I2C communication with sensor
#include <Wireling.h>               // For interfacing with Wirelings
#include "Accel3Thread.h"
#include "LraThread.h"
#include "RangeThread.h"
#include "OLED042Thread.h"

///////////////////// CHOOSE DEFAULT PIN CONFIGURATION ///////////

using namespace om;

void setup() { // run once, when the sketch starts
    // Serial I/O has lowest priority, so you may need to
    // decrease baud rate to fix Serial I/O problems.
    //SerialUSB.begin(38400); // short USB cables
    SerialUSB.begin(19200); // long USB cables
    SerialUSB.println("om.setup()");
    
    Wire.begin();
    Wire.setClock(400000);

    // Initialize
	accelThread.setup();
    lraThread.setup();
    rangeThread.setup();
    oledThread.setup();
    om::pinMode(LED_BUILTIN, OUTPUT);
    om::println("threadRunner.setup");
    threadRunner.setup(LED_BUILTIN);
}

void loop() {	// run over and over again
    threadRunner.run();
}
