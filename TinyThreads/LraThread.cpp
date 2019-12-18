#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include "LraThread.h"

#include <Wire.h>             // For using I2C communication
#include "Adafruit_DRV2605.h" // For interfacing with the DRV2605 chip

using namespace tinythreads;
using namespace fireduino;

friend class Thread;

LraThread lraThread; // Haptic feedback

Adafruit_DRV2605 drv;   // The variable used to interface with the DRV2605 chip
const int powerPin = 4;  // Power to Wireling

// **Wireling boards attached through an Adapter board**
// Selects the correct address of the port being used in the Adapter board
void setWirelingPort(int port) {
  Wire.beginTransmission(0x70);
  Wire.write(0x04 + port);
  Wire.endTransmission();
}

LraThread::LraThread(uint16_t msPeriod, uint8_t port)
    : msPeriod(msPeriod), level(0), effect(0), count(0), port(port)
{}

void LraThread::setup() {
    id = 'L';
    Thread::setup();
    fireduino::pinMode(powerPin, OUTPUT);
    fireduino::digitalWrite(powerPin, HIGH);
    drv.begin();
    drv.selectLibrary(1);
    serial_print("LraThread.setup");
}

void LraThread::buzz(uint8_t level) {
    // 50% duty cycle software PWM buzz
    // PWM period is twice msPeriod
    this->level = level;
    phase = 0;
}

void LraThread::setEffect(uint8_t effect, uint8_t count) {
    this->effect = effect;
    this->count = count;
}

void LraThread::playWaveform() {
    // repeat effect every period until count goes to zero
    if (count) { count--; } else { effect = 0; };
    drv.setMode(DRV2605_MODE_INTTRIG);
    drv.useLRA();
    // Just use waveform buffer for one effect
    drv.setWaveform(0, effect);  // Set effect
    drv.setWaveform(1, 0);       // End waveform
    drv.go(); // play
}

void LraThread::loop() {
    nextLoop.ticks = ticks() + MS_TICKS(msPeriod);

    setWirelingPort(port); // Tiny Adapter port

    if (level) {
        drv.setMode(DRV2605_MODE_REALTIME);
        drv.setRealtimeValue((phase%2) ? level : 0);
        phase++;
    } else {
        playWaveform();
    }
}
