#ifndef FIREDUINO_TINYZERO_H
#define FIREDUINO_TINYZERO_H

#include "Arduino.h"

extern int __heap_start, *__brkval;

namespace fireduino {
	inline Print& get_Print() {
		return SerialUSB;
	}
	inline int16_t serial_read() {
		return SerialUSB.read();
	}
	inline int16_t serial_available() {
		return SerialUSB.available();
	}
	inline void serial_begin(int32_t baud) {
		SerialUSB.begin(baud);
	}
	inline void serial_print(const char *value) {
		SerialUSB.print(value);
	}
	inline void serial_print(const char value) {
		SerialUSB.print(value);
	}
  inline void serial_print(int16_t value, int16_t format = DEC) {
    SerialUSB.print(value, format);
  }
  inline void serial_print(uint8_t value, int16_t format = DEC) {
    SerialUSB.print(value, format);
  }
  inline void serial_print(double value, int16_t format = 3) {
    SerialUSB.print(value, format);
  }
  inline void pinMode(int16_t pin, int16_t inout) {
		::pinMode(pin, inout);
	}
	inline int16_t digitalRead(int16_t pin) {
		return ::digitalRead(pin);
	}
	inline void digitalWrite(int16_t dirPin, int16_t value) {
		::digitalWrite(dirPin, value);
	}
	inline void analogWrite(int16_t dirPin, int16_t value) {
		::analogWrite(dirPin, value);
	}
	inline int16_t analogRead(int16_t dirPin) {
		return ::analogRead(dirPin);
	}
	inline void delay(int ms) {
		::delay(ms);
	}
	inline void delayMicroseconds(uint16_t usDelay) {
		::delayMicroseconds(usDelay);
	}
	inline uint32_t millis() {
		return ::millis();
	}

	inline int16_t freeRam () {
		int v;
		return (int)(size_t)&v - (__brkval == 0 
            ? (int)(size_t)&__heap_start 
            : (int)(size_t)__brkval
        );
	}
} // namespace fireduino

#endif
