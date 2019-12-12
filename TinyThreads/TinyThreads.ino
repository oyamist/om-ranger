#include <Arduino.h>
//#include "MachineThread.h"

///////////////////// CHOOSE DEFAULT PIN CONFIGURATION ///////////

tinythreads::MachineThread machineThread; // TinyThreads command interpreter


void setup() { // run once, when the sketch starts
    // Serial I/O has lowest priority, so you may need to
    // decrease baud rate to fix Serial I/O problems.
    //Serial.begin(38400); // short USB cables
    Serial.begin(19200); // long USB cables

    // Initialize
		// machineThread.setup();
  pinMode(LED_BUILTIN, OUTPUT);
  #ifdef ARDUINO_SAMD_ZERO
  digitalWrite(LED_BUILTIN, 1);
  #endif

//    tinythreads::threadRunner.setup(LED_PIN);
}

void loop() {	// run over and over again
 //   tinythreads::threadRunner.run();
}
