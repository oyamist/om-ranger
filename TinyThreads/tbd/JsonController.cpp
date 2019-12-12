#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#include <cstdio>
#endif
#include "version.h"
#include "JsonController.h"
#include "ProcessField.h"
#include "ProgMem.h"
//#include "git_tag.h"

using namespace firestep;

JsonController::JsonController(Machine& machine)
    : machine(machine) {
}

const char * JsonController::name() {
    return "JsonCtrl";
}

JsonController& JsonController::operator=(JsonController& that) {
    this->machine = that.machine;
    return *this;
}

inline int8_t hexValue(char c) {
    switch (c) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'A':
    case 'a':
        return 0xa;
    case 'B':
    case 'b':
        return 0xb;
    case 'C':
    case 'c':
        return 0xc;
    case 'D':
    case 'd':
        return 0xd;
    case 'E':
    case 'e':
        return 0xe;
    case 'F':
    case 'f':
        return 0xf;
    default:
        return -1;
    }
}

Status JsonController::process_id(JsonCommand &jcmd, JsonObject& jobj, const char* key) {
    Status status = STATUS_OK;
    const char *s;
    if (strcmp_PS(OP_id, key) == 0) {
        if ((s = jobj[key]) && *s == 0) {
            JsonObject& node = jobj.createNestedObject(key);
            jcmd.addQueryAttr(node, OP_app);
            jcmd.addQueryAttr(node, OP_ch);
            jcmd.addQueryAttr(node, OP_git);
            jcmd.addQueryAttr(node, OP_ver);
        }
        JsonObject& kidObj = jobj[key];
        if (kidObj.success()) {
            for (JsonObject::iterator it = kidObj.begin(); it != kidObj.end(); ++it) {
                status = process_id(jcmd, kidObj, it->key);
                if (status != STATUS_OK) {
                    return status;
                }
            }
        }
    } else if (strcmp_PS(OP_app, key) == 0 || strcmp_PS(OP_app, key + 2) == 0) {
		jobj[key] =  "TinyThreads";
    } else if (strcmp_PS(OP_ch, key) == 0 || strcmp_PS(OP_ch, key + 2) == 0) {
        int32_t curHash = machine.hash();
        int32_t jsonHash = curHash;
        status = processField<int32_t, int32_t>(jobj, key, jsonHash);
        if (jsonHash != curHash) {
            machine.syncHash = jsonHash;
        }
    } else if (strcmp_PS(OP_git, key) == 0 || strcmp_PS(OP_git, key + 2) == 0) {
		char *buf = jcmd.allocate(41);
		strcpy_P(buf, GIT_TAG);
		jobj[key] = buf;
    } else if (strcmp_PS(OP_ver, key) == 0 || strcmp_PS(OP_ver, key + 2) == 0) {
		jobj[key] = VERSION_MAJOR + VERSION_MINOR/100.0 + VERSION_PATCH/1000.0;
	} else {
        return jcmd.setError(STATUS_UNRECOGNIZED_NAME, key);
	}
	
    return status;
}

Status JsonController::processPin(JsonObject& jobj, const char *key, PinType &pin, int16_t mode, int16_t value) {
    PinType newPin = pin;
    Status status = processField<PinType, int32_t>(jobj, key, newPin);
    machine.setPin(pin, newPin, mode, value);
    return status;
}

Status JsonController::processSys(JsonCommand& jcmd, JsonObject& jobj, const char* key) {
    Status status = STATUS_OK;
    if (strcmp_PS(OP_sys, key) == 0) {
        const char *s;
        if ((s = jobj[key]) && *s == 0) {
            JsonObject& node = jobj.createNestedObject(key);
            jcmd.addQueryAttr(node, OP_eu);
            jcmd.addQueryAttr(node, OP_hp);
            jcmd.addQueryAttr(node, OP_jp);
            jcmd.addQueryAttr(node, OP_om);
            jcmd.addQueryAttr(node, OP_v);
        }
        JsonObject& kidObj = jobj[key];
        if (kidObj.success()) {
            for (JsonObject::iterator it = kidObj.begin(); it != kidObj.end(); ++it) {
                status = processSys(jcmd, kidObj, it->key);
                if (status != STATUS_OK) {
                    return status;
                }
            }
        }
    } else if (strcmp_PS(OP_eu, key) == 0 || strcmp_PS(OP_syseu, key) == 0) {
        bool euExisting = machine.isEEUserEnabled();
        bool euNew = euExisting;
        status = processField<bool, bool>(jobj, key, euNew);
        if (euNew != euExisting) {
            machine.enableEEUser(euNew);
        }
    } else if (strcmp_PS(OP_fr, key) == 0 || strcmp_PS(OP_sysfr, key) == 0) {
        leastFreeRam = minval(leastFreeRam, fireduino::freeRam());
        jobj[key] = leastFreeRam;
    } else if (strcmp_PS(OP_jp, key) == 0 || strcmp_PS(OP_sysjp, key) == 0) {
        status = processField<bool, bool>(jobj, key, machine.jsonPrettyPrint);
    } else if (strcmp_PS(OP_om, key) == 0 || strcmp_PS(OP_sysom, key) == 0) {
        status = processField<OutputMode, int32_t>(jobj, key, machine.outputMode);
    } else if (strcmp_PS(OP_tc, key) == 0 || strcmp_PS(OP_systc, key) == 0) {
        jobj[key] = threadClock.ticks;
    } else if (strcmp_PS(OP_v, key) == 0 || strcmp_PS(OP_sysv, key) == 0) {
        jobj.at(key).set(VERSION_MAJOR + VERSION_MINOR/100.0 + VERSION_PATCH/1000.0,3);
    } else {
        return jcmd.setError(STATUS_UNRECOGNIZED_NAME, key);
    }
    return status;
}

Status JsonController::processDebug(JsonCommand& jcmd, JsonObject& jobj, const char* key) {
    Status status = STATUS_OK;
    if (strcmp_PS(OP_dbg, key) == 0) {
        const char *s;
        if ((s = jobj[key]) && *s == 0) {
            JsonObject& node = jobj.createNestedObject(key);
            jcmd.addQueryAttr(node, OP_fr);
            jcmd.addQueryAttr(node, OP_lp);
            jcmd.addQueryAttr(node, OP_tc);
        }
        JsonObject& kidObj = jobj[key];
        if (kidObj.success()) {
            for (JsonObject::iterator it = kidObj.begin(); it != kidObj.end(); ++it) {
                status = processDebug(jcmd, kidObj, it->key);
                if (status != STATUS_OK) {
                    return status;
                }
            }
        }
    } else if (strcmp_PS(OP_fr, key) == 0 || strcmp_PS(OP_dbgfr, key) == 0) {
        leastFreeRam = minval(leastFreeRam, fireduino::freeRam());
        jobj[key] = leastFreeRam;
    } else if (strcmp_PS(OP_lp, key) == 0 || strcmp_PS(OP_dbglp, key) == 0) {
        status = processField<int32_t, int32_t>(jobj, key, nLoops);
    } else if (strcmp_PS(OP_tc, key) == 0 || strcmp_PS(OP_dbgtc, key) == 0) {
        jobj[key] = threadClock.ticks;
    } else {
        return jcmd.setError(STATUS_UNRECOGNIZED_NAME, key);
    }
    return status;
}


Status JsonController::processEEPROMValue(JsonCommand& jcmd, JsonObject& jobj, const char* key, const char*addr) {
	if (EEPROM_BYTES == 0) {
		return STATUS_NO_EEPROM;
	}
    Status status = STATUS_OK;
    JsonVariant &jvalue = jobj[key];
    if (addr && *addr == '!') {
        status = processObj(jcmd, jvalue);
        if (status != STATUS_OK) {
            return jcmd.setError(status, key);
        }
        TESTCOUT1("processEEPROMValue!:", addr);
        addr++;
    }
    if (!addr || *addr<'0' || '9'<*addr) {
        return STATUS_JSON_DIGIT;
    }
    long addrLong = strtol(addr, NULL, 10);
    if (addrLong<0 || EEPROM_END <= addrLong) {
        return STATUS_EEPROM_ADDR;
    }
    char buf[EEPROM_BYTES];
    buf[0] = 0;
    if (jvalue.is<JsonArray&>()) {
        JsonArray &jeep = jvalue;
        jeep.printTo(buf, EEPROM_BYTES);
    } else if (jvalue.is<JsonObject&>()) {
        JsonObject &jeep = jvalue;
        jeep.printTo(buf, EEPROM_BYTES);
    } else if (jvalue.is<const char *>()) {
        const char *s = jvalue;
        snprintf(buf, sizeof(buf), "%s", s);
    }
    if (buf[0] == 0) { // query
        uint8_t c = fireduino::eeprom_read_byte((uint8_t*) addrLong);
        if (c && c != 255) {
            char *buf = jcmd.allocate(EEPROM_BYTES);
            if (!buf) {
                return jcmd.setError(STATUS_JSON_MEM3, key);
            }
            for (int16_t i=0; i<EEPROM_BYTES; i++) {
                c = fireduino::eeprom_read_byte((uint8_t*) addrLong+i);
                if (c == 255 || c == 0) {
                    buf[i] = 0;
                    break;
                }
                buf[i] = c;
            }
            jobj[key] = buf;
        }
    } else {
        int16_t len = strlen(buf) + 1;
        if (len >= EEPROM_BYTES) {
            return jcmd.setError(STATUS_JSON_EEPROM, key);
        }
        for (int16_t i=0; i<len; i++) {
            fireduino::eeprom_write_byte((uint8_t*)addrLong+i, buf[i]);
            TESTCOUT3("EEPROM[", ((int)addrLong+i), "]:",
                      (char) fireduino::eeprom_read_byte((uint8_t *) addrLong+i),
                      " ",
                      (int) fireduino::eeprom_read_byte((uint8_t *) addrLong+i)
                     );
        }
    }
    return status;
}

Status JsonController::processEEPROM(JsonCommand& jcmd, JsonObject& jobj, const char* key) {
    Status status = STATUS_OK;
    if (strcmp_PS(OP_eep, key) == 0) {
        JsonObject& kidObj = jobj[key];
        if (!kidObj.success()) {
            return jcmd.setError(STATUS_JSON_OBJECT, key);
        }
        for (JsonObject::iterator it = kidObj.begin(); status>=0 && it != kidObj.end(); ++it) {
            status = processEEPROMValue(jcmd, kidObj, it->key, it->key);
        }
    } else if (strncmp("eep",key,3) == 0) {
        status = processEEPROMValue(jcmd, jobj, key, key+3);
    } else {
        return jcmd.setError(status, key);
    }
    if (status < 0) {
        return jcmd.setError(status, key);
    }
    return status;
}

Status JsonController::processIOPin(JsonCommand& jcmd, JsonObject& jobj, const char* key, bool pullUp) {
    const char *pinStr = *key == 'd' || *key == 'a' ? key+1 : key+3;
    long pinLong = strtol(pinStr, NULL, 10);
    if (machine.isCorePin(pinLong)) {
        return jcmd.setError(STATUS_CORE_PIN, key);
    }
    if (pinLong < 0 || MAX_PIN < pinLong) {
        return jcmd.setError(STATUS_NO_SUCH_PIN, key);
    }
    int16_t pin = (int16_t) pinLong;
    const char *s = jobj[key];
    bool isAnalog = *key == 'a' || strncmp("ioa",key,3)==0;
    if (s && *s == 0) { // read
        if (isAnalog) {
            jobj[key] = fireduino::analogRead(pin+A0);
        } else {
            fireduino::pinMode(pin, pullUp ? INPUT_PULLUP : INPUT);
            jobj[key] = (bool) fireduino::digitalRead(pin);
        }
    } else if (isAnalog) {
        if (jobj[key].is<long>()) { // write
            long value = jobj[key];
            if (value < 0 || 255 < value) {
                return jcmd.setError(STATUS_JSON_255, key);
            }
            fireduino::analogWrite(pin+A0, (int16_t) value);
        } else {
            return jcmd.setError(STATUS_JSON_255, key);
        }
    } else {
        if (jobj[key].is<bool>()) { // write
            bool value = jobj[key];
            fireduino::pinMode(pin, OUTPUT);
            fireduino::digitalWrite(pin, value);
        } else if (jobj[key].is<long>()) { // write
            bool value = (bool) (long)jobj[key];
            fireduino::pinMode(pin, OUTPUT);
            fireduino::digitalWrite(pin, value);
        } else {
            return jcmd.setError(STATUS_JSON_BOOL, key);
        }
    }
    return STATUS_OK;
}

Status JsonController::processIO(JsonCommand& jcmd, JsonObject& jobj, const char* key, bool pullUp) {
    Status status = STATUS_OK;
    if (strcmp_PS(OP_io, key) == 0) {
        JsonObject& kidObj = jobj[key];
        if (!kidObj.success()) {
            return jcmd.setError(STATUS_JSON_OBJECT, key);
        }
		char pu[10];
		strcpy_P(pu, OP_pu);
		if (kidObj.at(pu).success()) {
			pullUp = kidObj[pu];
			TESTCOUT1("iopu:", pullUp);
		}
        for (JsonObject::iterator it = kidObj.begin(); it != kidObj.end(); ++it) {
            status = processIO(jcmd, kidObj, it->key, pullUp);
			if (status < 0) {
				return status;
			}
        }
    } else if (strcmp_PS(OP_pu,key)==0) {
		// handled above
    } else if (strncmp("d",key,1)==0 || strncmp("iod",key,3)==0) {
        status = processIOPin(jcmd, jobj, key, pullUp);
    } else if (strncmp("a",key,1)==0 || strncmp("ioa",key,3)==0) {
        status = processIOPin(jcmd, jobj, key, pullUp);
    } else {
        return jcmd.setError(STATUS_UNRECOGNIZED_NAME, key);
    }
    return status;
}

Status JsonController::cancel(JsonCommand& jcmd, Status cause) {
    sendResponse(jcmd, cause);
    return STATUS_WAIT_CANCELLED;
}

void JsonController::sendResponse(JsonCommand &jcmd, Status status, bool final) {
    jcmd.setStatus(status);
    if (status >= 0) {
        if (jcmd.responseAvailable() < 1) {
            TESTCOUT2("response available:", jcmd.responseAvailable(), " capacity:", jcmd.responseCapacity());
            jcmd.setStatus(STATUS_JSON_MEM1);
        } else if (jcmd.requestAvailable() < 1) {
            TESTCOUT2("request available:", jcmd.requestAvailable(), " capacity:", jcmd.requestCapacity());
            jcmd.setStatus(STATUS_JSON_MEM2);
        }
    }
    if (machine.jsonPrettyPrint) {
        jcmd.response().prettyPrintTo(fireduino::get_Print());
    } else {
        jcmd.response().printTo(fireduino::get_Print());
    }
    jcmd.responseClear();
    if (final) {
        fireduino::serial_println(" "); // }-SPACE-LF marks final output before return to STATUS_WAIT_IDLE
    } else {
        fireduino::serial_println();
    }
}

Status JsonController::processObj(JsonCommand& jcmd, JsonObject&jobj) {
    JsonVariant node;
    node = jobj;
    Status status = STATUS_OK;
    //TESTCOUT1("processObj:", "in");

    for (JsonObject::iterator it = jobj.begin(); status >= 0 && it != jobj.end(); ++it) {
        //TESTCOUT1("processObj key:", it->key);
        if (strncmp("sys", it->key, 3) == 0) {
            status = processSys(jcmd, jobj, it->key);
        } else if (strncmp("dbg", it->key, 3) == 0) {
            status = processDebug(jcmd, jobj, it->key);
        } else if (strncmp("io", it->key, 2) == 0) {
            status = processIO(jcmd, jobj, it->key);
        } else if (strncmp("eep", it->key, 3) == 0) {
            status = processEEPROM(jcmd, jobj, it->key);
        } else if (strcmp_PS(OP_idl, it->key) == 0) {
            int16_t ms = it->value;
            fireduino::delay(ms);
        } else if (strcmp_PS(OP_cmt, it->key) == 0) {
            if (OUTPUT_CMT==(machine.outputMode&OUTPUT_CMT)) {
                const char *s = it->value;
                fireduino::serial_println(s);
            }
            status = STATUS_OK;
        } else if (strncmp("id", it->key, 2) == 0) {
            status = process_id(jcmd, jobj, it->key);
		} else if (strcmp_PS(OP_msg, it->key) == 0) {
            const char *s = it->value;
            fireduino::serial_println(s);
            status = STATUS_OK;
            TESTCOUT1("msg:", s);
        } else {
            status = jcmd.setError(STATUS_UNRECOGNIZED_NAME, it->key);
        }
    }

    //TESTCOUT1("processObj:", "out");
    return status;
}
