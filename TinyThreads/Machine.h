#ifndef MACHINE_H
#define MACHINE_H

#ifdef CMAKE
#include <cmath>
#endif
#include "fireduino.h"
#include "Status.h"
#include "Thread.h"
#include "os_util.h"
#include "Display.h"
#include "pins.h"


namespace firestep {

// #define THROTTLE_SPEED /* Throttles driver speed from high (255) to low (0) */

#define LATCH_BACKOFF 200
#define MOTOR_COUNT 4
#define PIN_ENABLE LOW
#define PIN_DISABLE HIGH
#define MICROSTEPS_DEFAULT 16
#define INDEX_NONE -1
#define EEUSER 2000
#define EEUSER_ENABLED (EEUSER-1)
#define MAX_ATTR_BYTES 30

#define FPD_GEAR_RATIO 9.52374
#define FPD_GEAR_RATIO_S "9.52374"
#define LC_GEAR_RATIO	9.58600
#define LC_GEAR_RATIO_S	"9.58600"
#define TW_GEAR_RATIO	9.460206
#define TW_GEAR_RATIO_S	"9.460206"
#define FPD_GEAR_RATIO_S5 "9.52374"
#define FPD_SPE_ANGLE -54.617
#define FPD_SPE_ANGLE_S "-54.617"
#define FPD_SPE_RATIO -0.383
#define FPD_SPE_RATIO_S "-0.383"
#define FPD_SEARCH_DELAY 800
#define FPD_SEARCH_DELAY_S "800"
#define FPD_SEARCH_DELAY_400 400
#define FPD_SEARCH_DELAY_400_S "400"
#define FPD_SPE_CRITICAL_PULSES -4624
#define FPD_SPE_CRITICAL_PULSES_S "-4624"
#define FPD_FAST_SEARCH_PULSES 3
#define FPD_FAST_SEARCH_PULSES_S "3"
#define FPD_MICROSTEPS 16
#define FPD_STEPS360 200
#define FPD_MICROSTEPS_PER_DEGREE 84.655

typedef int16_t DelayMics; // delay microseconds

enum OutputMode {
    OUTPUT_ARRAY1=0, //  JSON command arrays only return last command response
    OUTPUT_ARRAYN=1, // JSON command arrays return all command responses
    OUTPUT_CMT=2, // Write comments
};

///**
 //* inline replacement for Arduino delayMicroseconds()
 //*/
//inline void delayMics(int32_t usDelay) {
    //if (usDelay > 0) {
//#ifdef TEST
        //delayMicsTotal += usDelay;
//#endif
        //while (usDelay-- > 0) {
            //DELAY500NS;
            //DELAY500NS;
        //}
    //}
//}

#define PULLUP_LIMIT_MIN	0x2
#define PULLUP_LIMIT_MAX 	0x4

typedef class Machine {
public:
    PinConfig	pinConfig;
    bool	 	pinEnableHigh;
    bool		invertLim;
    bool		jsonPrettyPrint;
    bool		autoSync; // auto-save configuration to EEPROM
    uint8_t		pullups;
    uint8_t		debounce;
    Display 	nullDisplay;
    int32_t 	vMax; // maximum stroke velocity (pulses/second)
    int16_t		fastSearchPulses;
    DelayMics 	searchDelay; // limit switch search velocity (pulse delay microseconds)
    PinType		pinStatus;
    OutputMode	outputMode;
    int32_t		syncHash;

public:
    Display*	pDisplay;

protected:

public:
    Machine();
    void setup(PinConfig cfg);
    int32_t hash();
    bool isCorePin(int16_t pin);
    inline bool isAtLimit(PinType pin) {
        uint8_t highCount = fireduino::digitalRead(pin) ? 1 : 0;
        for (uint8_t i=0; i<debounce; i++) {
            highCount += fireduino::digitalRead(pin) ? 1 : 0;
        }
        return (invertLim == !(highCount > debounce/2));
    }
    void setPin(PinType &pinDst, PinType pinSrc, int16_t mode, int16_t value = LOW);
    char * saveSysConfig(char *out, size_t maxLen);
    char * saveDimConfig(char *out, size_t maxLen);
    Status idle(Status status);
    Status sync(Status status);
    void enableEEUser(bool enable);
    bool isEEUserEnabled();
    uint8_t pullupMode(uint8_t mask) {
        return (pullups & mask) ? INPUT_PULLUP : INPUT;
    }
} Machine;

#ifdef TEST
extern int32_t delayMicsTotal;
#endif

char * saveConfigValue(const char *key, const char *value, char *out);
char * saveConfigValue(const char *key, bool value, char *out);
char * saveConfigValue(const char *key, uint8_t value, char *out);
char * saveConfigValue(const char *key, int32_t value, char *out);
char * saveConfigValue(const char *key, int16_t value, char *out);

} // namespace firestep

#endif
