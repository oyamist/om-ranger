#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include "Font_042.h"               // The font displayed on the screen
#include "TinyCircuits_HP7240.h"    // Library for OLED screen
#include "exampleSprites.h"         // Holds arrays of example Sprites
#include "BMA250.h"       // For interfacing with the accel. sensor

#include "version.h"
#include "MachineThread.h"

using namespace tinythreads;


// Accelerometer sensor variables for the sensor and its values
BMA250 accel_sensor;
int x, y, z;
double temp;

typedef struct XYZ {
  int x, y, z;
  void set(int x, int y, int z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }
  void print() {
      SerialUSB.print("{x:");
  SerialUSB.print(this->x);
  
  SerialUSB.print(",y:");
  SerialUSB.print(this->y);
  
  SerialUSB.print(",z:");
  SerialUSB.print(this->z);
  SerialUSB.println("}");
  }
} XYZ;

#define SAMPLES 10
XYZ xyz[SAMPLES];
int iSample = 0;




MachineThread::MachineThread(){
}


void MachineThread::setup() {
    id = 'M';
    Thread::setup();

  SerialUSB.print("Initializing BMA...");
  // Set up the BMA250 acccelerometer sensor
  accel_sensor.begin(BMA250_range_2g, BMA250_update_time_32ms); 
}

// Prints the sensor values to the Serial Monitor (found under 'Tools')
void showSerial() {
  /*
  SerialUSB.print("{x:");
  SerialUSB.print(x);
  
  SerialUSB.print(",y:");
  SerialUSB.print(y);
  
  SerialUSB.print(",z:");
  SerialUSB.print(z);
  SerialUSB.println("}");
  */
  if (iSample == 0) {
    for (int i = 0; i<SAMPLES;i++) {
      xyz[i].print();
    
    }
  }
//  SerialUSB.print(" C= ");
//  SerialUSB.println(temp);
}


void MachineThread::loop() {
  nextLoop.ticks = ticks() + MS_TICKS(32);
  accel_sensor.read();//This function gets new data from the acccelerometer

  // Get the acceleration values from the sensor and store them into global variables
  // (Makes reading the rest of the program easier)
  x = accel_sensor.X;
  y = accel_sensor.Y;
  z = accel_sensor.Z;
  xyz[iSample].set(x,y,z);
  iSample = (iSample+1) % SAMPLES;
  temp = ((accel_sensor.rawTemp * 0.5) + 24.0);

  // If the BME280 is not found, nor connected correctly, these values will be produced
  // by the sensor 
  if (x == -1 && y == -1 && z == -1) {
    // Print error message to Serial Monitor
    Serial.print("ERROR! NO BMA250 DETECTED!");
  }
  
  else { // if we have correct sensor readings: 
    showSerial();                 //Print to Serial Monitor 
  }

}
