#ifndef FIREDUINO_TINYZERO_H
#define FIREDUINO_TINYZERO_H

#include "Arduino.h"
#include <Wire.h>
#include "TinyZero.h"



namespace tinyzero {

// **Wireling boards attached through an Adapter board**
// Selects the correct address of the port being used in the Adapter board
void setPort(int port) {
  Wire.beginTransmission(0x70);
  Wire.write(0x04 + port);
  Wire.endTransmission();
}

} // tinyzero


#endif
