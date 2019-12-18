#ifndef FIREDUINO_H
#define FIREDUINO_H

#define NOPIN -1
#define minval(a,b) ((a)<(b)?(a):(b))
#define maxval(a,b) ((a)>(b)?(a):(b))
#define absval(x) ((x)>0?(x):-(x))
#define roundval(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#ifndef radians
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#endif
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

// Argument to setI2CPort() for accessing peripherals
#define I2CPORT_DISPLAY 0
#define I2CPORT_LRA 1
#define I2CPORT_TOF 2

#if defined(MOCK_MEGA2560)
#define NO_MCU
#include "MockDuino.h"
#elif defined( ARDUINO_SAMD_ZERO )
#include "TinyZero.h"
#else
#define NO_MCU
namespace MilliThreads { // abstract API implementable any way you like
	//////////////////// ARDUINO SPECIFIC ///////////////////
	Print& get_Print();
	int16_t serial_read();
	int16_t serial_available();
	void serial_begin(int32_t baud);
	void serial_print(const char *value);
	void serial_print(const char value);
	void serial_print(int16_t value, int16_t format = DEC);
    void serial_print(uint8_t value, int16_t format = DEC);
    void serial_print(double value, int16_t places = 3);
	void pinMode(int16_t pin, int16_t inout);
	int16_t digitalRead(int16_t pin);
	void digitalWrite(int16_t dirPin, int16_t value);
	void analogWrite(int16_t dirPin, int16_t value);
	int16_t analogRead(int16_t dirPin);
	uint32_t millis();
	void delay(int ms);
	void delayMicroseconds(uint16_t usDelay);

	////////////////// OTHER ///////////////////
    void setI2CPort(uint8_t port);

	int16_t freeRam ();
} // namespace MilliThreads
#endif

namespace MilliThreads {
	inline void serial_println() {
		serial_print('\n');
	}
	inline void serial_println(const char value) {
		serial_print(value);
		serial_print('\n');
	}
	inline void serial_println(const char* value) {
		serial_print(value);
		serial_print('\n');
	}
}


#ifdef TEST
#define TESTCOUT1(k,v) cout << k << v << endl
#define TESTCOUT2(k1,v1,k2,v2) cout << k1<<v1 <<k2<<v2 << endl
#define TESTCOUT3(k1,v1,k2,v2,k3,v3) cout << k1<<v1 <<k2<<v2 <<k3<< v3 << endl
#define TESTCOUT4(k1,v1,k2,v2,k3,v3,k4,v4) cout << k1<<v1 <<k2<<v2 <<k3<<v3 <<k4<<v4 << endl
#define TESTDECL(t,v) t v
#define TESTEXP(e) e
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#define DIE() kill(getpid(), SIGABRT)
#else
#define TESTCOUT1(k,v)
#define TESTCOUT2(k1,v1,k2,v2)
#define TESTCOUT3(k1,v1,k2,v2,k3,v3)
#define TESTCOUT4(k1,v1,k2,v2,k3,v3,k4,v4)
#define TESTDECL(t,v)
#define TESTEXP(e)
#define DIE()
#endif

#define DEBUG_EOL() MilliThreads::serial_println("");
#define DEBUG_HEX(S,V) MilliThreads::serial_print(" " S ":");MilliThreads::serial_print(V,HEX);
#define DEBUG_DEC(S,V) MilliThreads::serial_print(" " S ":");MilliThreads::serial_print(V,DEC);

#define TICKS_PER_SECOND ((int32_t)1000)
#define MS_TICKS(ms) (1*ms) /* Convert milliseconds to Ticks */

#endif