#ifndef FIREDUINO_TINYZERO_H
#define FIREDUINO_TINYZERO_H

#include "Arduino.h"

#define EEPROM_BYTES 512 /* Actual capacity will be less because eeprom buffer is part of MAX_JSON */
#define EEPROM_END 4096

extern int __heap_start, *__brkval;

// uint16_t hardware timer
#define TIMER_SETUP() TCCR1A = 0 /* Timer mode */; TIMSK1 = (0 << TOIE1) /* disable interrupts */
#define TIMER_VALUE() TCNT1
#define TIMER_CLEAR()	TCNT1 = 0
#define TIMER_ENABLE(enable) \
    if (enable) {\
        TCCR1B = 1 << CS12 | 0 << CS11 | 1 << CS10; /* Timer prescaler div1024 (15625Hz) */\
    } else {\
        TCCR1B = 0;	/* stop clock */\
    }

#ifndef DELAY500NS
#define DELAY500NS \
  asm("nop");asm("nop");asm("nop");asm("nop"); asm("nop");asm("nop");asm("nop");asm("nop");
#endif

// A4983 stepper driver pulse cycle requires 2 microseconds
// DRV8825 stepper driver requires 3.8 microseconds
//   http://www.ti.com/lit/ds/symlink/drv8825.pdf
#define PULSE_WIDTH_DELAY() /* no delay */

// The choice of stepper pulse delay is critical. TinyThreads can drive up to 4
// steppers concurrently each at up to 25,000 pulses per second. 
// Since TinyThreads timeslices its pulse generation, TinyThreads must emit 100,000
// pulses per second. That pulse rate requires that a pulse be generated every
// 10 microseconds. Stepper drivers require minimal pulse widths for the high
// and low portion of the pulse waveform (e.g., DRV8824 requires 2us for each).
// Although the Arduino is capable of generating short pulses, TinyThreads uses
// longer pulses to ensure that the stepper driver microstep indexer is triggered
// properly.

#ifndef PRR
#define PRR PRR0
#endif

namespace fireduino {
	inline Print& get_Print() {
		return Serial;
	}
	inline int16_t serial_read() {
		return Serial.read();
	}
	inline int16_t serial_available() {
		return Serial.available();
	}
	inline void serial_begin(int32_t baud) {
		Serial.begin(baud);
	}
	inline void serial_print(const char *value) {
		Serial.print(value);
	}
	inline void serial_print(const char value) {
		Serial.print(value);
	}
	inline void serial_print(int16_t value, int16_t format = DEC) {
		Serial.print(value, format);
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
	inline uint8_t eeprom_read_byte(uint8_t *addr) {
//		return ::eeprom_read_byte(addr);
	}
	inline void	eeprom_write_byte(uint8_t *addr, uint8_t value) {
//		::eeprom_write_byte(addr, value);
	}
	inline uint32_t millis() {
		return ::millis();
	}
	inline uint32_t get_timer64us() {
		return (* (volatile uint16_t *) &TCNT1);
	}
	inline void enable_timer64us(bool enable) {
		TIMER_ENABLE(enable);
	}
	inline void setup_timer64us() {
		TIMER_SETUP();
	}
	inline void clear_timer64us() {
		TIMER_CLEAR();
	}
	inline int16_t freeRam () {
		int v;
		return (int)(size_t)&v - (__brkval == 0 ? (int)(size_t)&__heap_start : (int)(size_t)__brkval);
	}
} // namespace fireduino

#endif
