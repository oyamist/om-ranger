#include <Arduino.h>
#include <Wire.h>         // For I2C communication with sensor
#include <Wireling.h>               // For interfacing with Wirelings
#include "Accel3Thread.h"
#include "LraThread.h"

///////////////////// CHOOSE DEFAULT PIN CONFIGURATION ///////////

using namespace tinythreads;

Accel3Thread accelThread; // Acceleromoter tracker
LraThread lraThread; // Haptic feedback

void setup() { // run once, when the sketch starts
    // Serial I/O has lowest priority, so you may need to
    // decrease baud rate to fix Serial I/O problems.
    //SerialUSB.begin(38400); // short USB cables
    SerialUSB.begin(19200); // long USB cables
    SerialUSB.println("TinyThreads.setup()");
    
  Wire.begin();
//  Wireling.begin();
//  Wireling.selectPort(3);  // This port# matches the one labeled on the adapter board

    // Initialize
	  accelThread.setup();
    lraThread.setup();
    pinMode(LED_BUILTIN, OUTPUT);
    SerialUSB.println("threadRunner.setup");
    threadRunner.setup(LED_BUILTIN);
}

void loop() {	// run over and over again
    threadRunner.run();
}
