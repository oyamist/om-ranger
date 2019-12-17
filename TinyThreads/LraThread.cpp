#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include "LraThread.h"

#include <Wire.h>             // For using I2C communication
#include "Adafruit_DRV2605.h" // For interfacing with the DRV2605 chip

using namespace tinythreads;
using namespace fireduino;

Adafruit_DRV2605 drv;   // The variable used to interface with the DRV2605 chip
uint8_t effect = 1;  // Waveform effects
const int powerPin = 4;  // Power to Wireling

// **Wireling boards attached through an Adapter board**
// Selects the correct address of the port being used in the Adapter board
void setWirelingPort(int port) {
  Wire.beginTransmission(0x70);
  Wire.write(0x04 + port);
  Wire.endTransmission();
}

LraThread::LraThread(){
}

void LraThread::setup() {
    id = 'L';
    Thread::setup();
    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, HIGH);
    drv.begin();
    drv.selectLibrary(1);
    // I2C trigger by sending 'go' command
    // default, internal trigger when sending GO command
    drv.setMode(DRV2605_MODE_INTTRIG);
    drv.useLRA();
    serial_print("LraThread.setup");
}

void LraThread::loop() {
    nextLoop.ticks = ticks() + MS_TICKS(1000);

    setWirelingPort(1); // Tiny Adapter port
    drv.setWaveform(0, effect);  // Set effect
    drv.setWaveform(1, 0);       // End waveform

    // Play the effect
    drv.go();

    effect = (effect + 1) % 117;
}

