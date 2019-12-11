#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#include <cmath>
#endif
#include "FireUtils.h"
#include "Machine.h"
#include "version.h"
#include "ProgMem.h"

using namespace firestep;

TESTDECL(int32_t, firestep::delayMicsTotal = 0);

char * firestep::saveConfigValue(const char *key, const char *value, char *out) {
    sprintf(out, "\"%s\":%s,", key, value);
	//TESTCOUT2("saveConfigValue char* key:", key, " value:", value);
    return out+strlen(out);
}

char * firestep::saveConfigValue(const char *key, bool value, char *out) {
    return saveConfigValue(key, value ? "1":"0", out);
}

char * firestep::saveConfigValue(const char *key, uint8_t value, char *out) {
    sprintf(out, "\"%s\":%d,", key, value);
	//TESTCOUT2("saveConfigValue int16 key:", key, " value:", value);
    return out+strlen(out);
}

char * firestep::saveConfigValue(const char *key, int16_t value, char *out) {
    sprintf(out, "\"%s\":%d,", key, value);
	//TESTCOUT2("saveConfigValue int16 key:", key, " value:", value);
    return out+strlen(out);
}

char * firestep::saveConfigValue(const char *key, int32_t value, char *out) {
    sprintf(out, "\"%s\":%ld,", key, (long) value);
	//TESTCOUT2("saveConfigValue int32 key:", key, " value:", value);
    return out+strlen(out);
}

////////////////////// Machine /////////////////////////

Machine::Machine() :
	invertLim(false),
	jsonPrettyPrint(false),
	autoSync(false),
	pullups(0),
	debounce(0),
	vMax(12800),
	fastSearchPulses(FPD_FAST_SEARCH_PULSES),
	searchDelay(FPD_SEARCH_DELAY),
	pinStatus(NOPIN),
	outputMode(OUTPUT_ARRAY1),
	syncHash(0),
	pDisplay(&nullDisplay)
{
    pinEnableHigh = false;
}

void Machine::setup(PinConfig cfg) {
}

int32_t Machine::hash() {
    int32_t result = 0;
    result = result ^ ((int32_t) outputMode << 8);
    result = result ^ ((int32_t) pinConfig << 10);

    return result;
}

bool Machine::isCorePin(int16_t pin) {
    return false;
}

void Machine::setPin(PinType &pinDst, PinType pinSrc, int16_t mode, int16_t value) {
    pinDst = pinSrc;
    if (pinDst != NOPIN) {
        fireduino::pinMode(pinDst, mode);
        if (mode == OUTPUT) {
            fireduino::digitalWrite(pinDst, value);
        }
    }
}

Status Machine::idle(Status status) {
    return status;
}

char * Machine::saveSysConfig(char *out, size_t maxLen) {
    *out++ = '{';
    // priority 1
    out = saveConfigValue("ch", hash(), out);
    out = saveConfigValue("pc", (uint8_t) pinConfig, out);

    // priority 2
    //out = saveConfigValue("as", autoSync, out); // too dangerous
    out = saveConfigValue("db", debounce, out);
    //out = saveConfigValue("eu", eeUser, out); // saved separately
    out = saveConfigValue("hp", fastSearchPulses, out);
    out = saveConfigValue("jp", jsonPrettyPrint, out);
    out = saveConfigValue("lh", invertLim, out);
    out = saveConfigValue("mv", vMax, out);
    out = saveConfigValue("om", (uint8_t) outputMode, out);
    out = saveConfigValue("pi", pinStatus, out);
    out = saveConfigValue("pu", pullups, out);

    out--;
    *out++ = '}';
    *out = 0;
    return out;
}

char * Machine::saveDimConfig(char *out, size_t maxLen) {
    *out++ = '{';
    // save these so that they will load first before angles
    *out++ = '}';
    *out = 0;
    return out;
}

void Machine::enableEEUser(bool enable) {
    if (isEEUserEnabled() != enable) {
        fireduino::eeprom_write_byte((uint8_t *)EEUSER_ENABLED, (uint8_t)(enable ? 'y' : 'n'));
    }
}

bool Machine::isEEUserEnabled() {
    return 'y' == fireduino::eeprom_read_byte((uint8_t *)EEUSER_ENABLED);
}

