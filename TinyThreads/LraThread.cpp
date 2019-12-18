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
    level = 0;
}

void LraThread::setup() {
    id = 'L';
    Thread::setup();
    fireduino::pinMode(powerPin, OUTPUT);
    fireduino::digitalWrite(powerPin, HIGH);
    drv.begin();
    drv.selectLibrary(1);
    // I2C trigger by sending 'go' command
    // default, internal trigger when sending GO command
//    drv.setMode(DRV2605_MODE_INTTRIG);
//    drv.useLRA();
    serial_print("LraThread.setup");
}

void LraThread::buzz(uint8_t level) {
    this->level = level;
    phase = 0;
}

void LraThread::setEffect(uint8_t effect) {
    drv.setMode(DRV2605_MODE_INTTRIG);
    drv.useLRA();
    drv.setWaveform(0, effect);  // Set effect
    drv.setWaveform(1, 0);       // End waveform
    drv.go(); // play
    
    fireduino::serial_print("lra:");
    fireduino::serial_print(effect);
    fireduino::serial_print("\n");
}

void LraThread::loop() {
    nextLoop.ticks = ticks() + MS_TICKS(50);

    setWirelingPort(1); // Tiny Adapter port

    if (level) {
        drv.setMode(DRV2605_MODE_REALTIME);
        drv.setRealtimeValue((phase%2) ? level : 0);
        if (++phase > 40) {
            phase = 0;

            switch(level) {
              default: level = 1; break;
              case 1: level = 2; break;
              case 2: level = 3; break;
              case 3: level = 32; break;
              case 32: level = 64; break;
              case 64: level = 96; break;
              case 96: level = 128; break;
              case 128: level = 190; break;
              case 190: level = 255; break;
            }
            fireduino::serial_print(level);
            fireduino::serial_println(" level");
        }
    } else {
        switch (phase) {
        case 0: 
        case 1: 
        case 2: 
        case 3: 
        case 4: 
          setEffect(0); 
          break;
        case 5: 
          setEffect(0);
          break;
        case 6: 
          setEffect(0);
          break;
        case 7: 
          setEffect(0);
          break;
        case 8: 
          setEffect(0);
          break;
        case 9: 
         // setEffect(0);
          break;
        case 10: 
         // setEffect(0);
          break;
        case 11: 
          setEffect(DRV2605_SHARP_TICK_3);
          break;
        case 12: 
          setEffect(DRV2605_SHARP_TICK_1);
          break;
        case 13: 
          setEffect(DRV2605_SHARP_TICK_1);
          break;
        }
        phase = ++phase % 12;
    }

    //setEffect(effect);
   // effect = (effect + 1) % 117;
}
