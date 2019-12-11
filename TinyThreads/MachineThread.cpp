#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif
#include "version.h"
#include "MachineThread.h"

using namespace firestep;

void MachineThread::setup(PinConfig pc) {
    id = 'M';
#ifdef THROTTLE_SPEED
    ADC_LISTEN8(ANALOG_SPEED_PIN);
#endif
    Thread::setup();
    machine.setup(pc);
    status = STATUS_BUSY_SETUP;
    displayStatus();
}

MachineThread::MachineThread()
    : status(STATUS_BUSY_SETUP), jsonController(machine), printBannerOnIdle(true) {
    setController(jsonController);
}

void MachineThread::setController(JsonController &controller) {
    this->pController = &controller;
}

void MachineThread::displayStatus() {
    switch (status) {
    case STATUS_OK:
    case STATUS_WAIT_IDLE:
        machine.pDisplay->setStatus(DISPLAY_WAIT_IDLE);
        break;
    case STATUS_WAIT_EOL:
        machine.pDisplay->setStatus(DISPLAY_WAIT_EOL);
        break;
    case STATUS_WAIT_CAMERA:
        machine.pDisplay->setStatus(DISPLAY_WAIT_CAMERA);
        break;
    case STATUS_WAIT_OPERATOR:
        machine.pDisplay->setStatus(DISPLAY_WAIT_OPERATOR);
        break;
    case STATUS_BUSY_OK:
    case STATUS_BUSY_SETUP:
    case STATUS_BUSY_EEPROM:
    case STATUS_WAIT_BUSY:
    case STATUS_BUSY:
        machine.pDisplay->setStatus(DISPLAY_BUSY);
        break;
    case STATUS_WAIT_CANCELLED:
        machine.pDisplay->setStatus(DISPLAY_WAIT_CANCELLED);
        break;
    case STATUS_BUSY_CALIBRATING:
        machine.pDisplay->setStatus(DISPLAY_BUSY_CALIBRATING);
        break;
    case STATUS_BUSY_MOVING:
    case STATUS_WAIT_MOVING:
        machine.pDisplay->setStatus(DISPLAY_BUSY_MOVING);
        break;
    default:	// errors
        if (status < 0) {
            machine.pDisplay->setStatus(DISPLAY_WAIT_ERROR);
        } else {
            machine.pDisplay->setStatus(DISPLAY_BUSY);
        }
        break;
    }
}

size_t MachineThread::readEEPROM(uint8_t *eeprom_addr, char *dst, size_t maxLen) {
    DisplayStatus ds = machine.pDisplay->getStatus();
    machine.pDisplay->setStatus(DISPLAY_EEPROM);
    uint8_t c = fireduino::eeprom_read_byte(eeprom_addr);
    if (!dst || (c != '{' && c != '[')) {
        return 0;
    }

    size_t len = 0;
    for (len=0; len<maxLen-2; len++) {
        c = fireduino::eeprom_read_byte(eeprom_addr+len);
        if (c == 255 || c == 0) {
            break;
        }
        dst[len] = c;
    }
    dst[len] = 0;
    machine.pDisplay->setStatus(ds);

    return len;
}

char * MachineThread::buildStartupJson() {
    command.clear();
    char *buf = command.allocate(MAX_JSON);
    ASSERT(buf);

    size_t len = 0;
    buf[len++] = '[';
    len += readEEPROM((uint8_t*)(size_t) 0, buf+len, MAX_JSON-len);
    if (len > 1) {
        if (buf[1] == '[') {
            buf[1] = ' ';
        }
        if (buf[len-1] == ']') {
            buf[len-1] = ' ';
        }
        buf[len++] = ',';
    }

    size_t eeUserLen = 0;
    if (machine.isEEUserEnabled()) {
        eeUserLen = readEEPROM((uint8_t*)(size_t) EEUSER, buf+len, MAX_JSON-len);
    }

    if (eeUserLen == 0) {
        if (len > 1) {
            len--; // remove comma
        }
    } else {
        if (buf[len] == '[') {
            buf[len] = ' ';
        }
        len += eeUserLen;
    }
    if (buf[len-1] != ']') {
        buf[len++] = ']';
    }
    buf[len] = 0;

    return buf;
}

Status MachineThread::executeEEPROM() {
    char *buf = buildStartupJson();
    TESTCOUT3("executeEEPROM:", buf, " len:", strlen(buf), " status:", (int) status);
    status = command.parse(buf, status);
    TESTCOUT2("executeEEPROM status:", (int) status, " buf:", buf);
    if (status < 0) {
        fireduino::serial_print("{\"s\":");
        fireduino::serial_print((int16_t)status);
        fireduino::serial_println(",\"r\":");
        buf = buildStartupJson();
        for (char *s = buf; *s; s++) {
            fireduino::serial_print(*s);
        }
        fireduino::serial_println();
        fireduino::serial_println("}");
    }

    return status;
}

Status MachineThread::syncConfig() {
    Status status = STATUS_WAIT_IDLE;
    command.clear();
    char *buf = command.allocate(MAX_JSON);
    ASSERT(buf);
    char *out = buf;

    TESTCOUT1("SAVING CONFIGURATION TO EEPROM","");

    *out++ = '[';

    // save system config
    *out++ = '{';
    *out++ = '"';
    *out++ = 's';
    *out++ = 'y';
    *out++ = 's';
    *out++ = '"';
    *out++ = ':';
    out = machine.saveSysConfig(out, MAX_JSON-(out-buf));

    if (out-buf+2 >= MAX_JSON) {
        return STATUS_JSON_MEM3;
    }
    *out++ = ']';
    *out++ = '\n';
    out += strlen(out);

    // Save to EEPROM before executing config JSON (parsing is destructive)
    DisplayStatus ds = machine.pDisplay->getStatus();
    machine.pDisplay->setStatus(DISPLAY_EEPROM);
    size_t len = strlen(buf);
    uint8_t *eepAddr = 0;
    fireduino::eeprom_write_byte(eepAddr, ' '); // disable eeprom
    for (uint16_t i=1; i<=len; i++) { // include terminator
        fireduino::eeprom_write_byte(eepAddr+i, buf[i]);
        //if (fireduino::serial_available()) { return; }
    }
    machine.pDisplay->setStatus(ds);
    TESTCOUT3("syncConfig len:", strlen(buf), " buf:", buf, " status:", (int) status);
    // Commit config JSON to EEPROM iff JSON is valid
    status = command.parse(buf, status);
    if (status == STATUS_BUSY_PARSED) {
        machine.syncHash = machine.hash(); // commit saved
        fireduino::eeprom_write_byte(eepAddr, buf[0]); // enable eeprom
        fireduino::eeprom_write_byte(eepAddr+len-1, 0); // remove EOL
        status = STATUS_WAIT_IDLE;
    } else {
        machine.autoSync = false; // no point trying again
    }
    return status;
}

void MachineThread::loop() {
#ifdef THROTTLE_SPEED
    if (fireduino::serial_available()) {
        return;
    }
    pController->speed = ADCH;
    if (pController->speed <= 251) {
        ThreadEnable(false);
        for (uint8_t iPause = pController->speed; iPause <= 247; iPause++) {
            for (uint8_t iIdle = 0; iIdle < 10; iIdle++) {
                DELAY500NS;
                DELAY500NS;
            }
        }
        ThreadEnable(true);
    }
#endif

    switch (status) {
    default:
    case STATUS_WAIT_IDLE:
    case STATUS_WAIT_CAMERA:
    case STATUS_WAIT_OPERATOR:
    case STATUS_WAIT_MOVING:
    case STATUS_WAIT_BUSY:
    case STATUS_WAIT_CANCELLED:
        if (fireduino::serial_available()) {
            command.clear();
            status = command.parse(NULL, status);
        } else {
            status = machine.idle(status);
        }
        break;
    case STATUS_WAIT_EOL:
        if (fireduino::serial_available()) {
            status = command.parse(NULL, status);
        }
        break;
    case STATUS_BUSY_PARSED:
    case STATUS_BUSY_OK:
    case STATUS_BUSY:
    case STATUS_BUSY_CALIBRATING:
    case STATUS_BUSY_MOVING:
        if (fireduino::serial_available()) {
            status = pController->cancel(command, STATUS_SERIAL_CANCEL);
        } else {
            status = process(command);
            //TESTCOUT1("pController->process status:", status);
        }
        break;
    case STATUS_BUSY_EEPROM:
        status = executeEEPROM();
        break;
    case STATUS_BUSY_SETUP: {
        uint8_t c = fireduino::eeprom_read_byte((uint8_t*) 0);
        if (c == '{' || c == '[') {
            status = STATUS_BUSY_EEPROM;
        } else {
			machine.syncHash = machine.hash();
            command.clear();
            status = command.parse("{\"id\":\"\"}", status);
        }
        break;
    }
    case STATUS_OK:
        status = STATUS_WAIT_IDLE;
        if (machine.syncHash != machine.hash()) {
            if (machine.autoSync) {
                TESTCOUT2("STATUS_OK autoSync syncHash:", machine.syncHash, " hash:", machine.hash());
                status = syncConfig();
            } else {
                TESTCOUT2("STATUS_OK syncHash:", machine.syncHash, " hash:", machine.hash());
            }
            if (machine.syncHash != 0 && machine.isEEUserEnabled()) {
                TESTCOUT1("STATUS_OK user EEPROM:", "disabled");
                machine.enableEEUser(false);
            }
        }
        break;
    }

    displayStatus();

    nextLoop.ticks = 0; // Highest priority
}

Status MachineThread::process(JsonCommand& jcmd) {
    Status status = STATUS_OK;
    JsonVariant &jroot = jcmd.requestRoot();

    if (jroot.is<JsonObject&>()) {
        JsonObject& jobj = jroot;
        status = pController->processObj(jcmd, jobj);
    } else if (jroot.is<JsonArray&>()) {
        JsonArray& jarr = jroot;
        if (jcmd.cmdIndex < jarr.size()) {
            JsonObject& jobj = jarr[jcmd.cmdIndex];
            jcmd.jResponseRoot["r"] = jobj;
            status = pController->processObj(jcmd, jobj);
            //TESTCOUT3("JsonController::process(", (int) jcmd.cmdIndex+1,
            //" of ", jarr.size(), ") status:", status);
            if (status == STATUS_OK) {
                bool isLast = jcmd.cmdIndex >= jarr.size()-1;
                if (!isLast && OUTPUT_ARRAYN==(machine.outputMode&OUTPUT_ARRAYN)) {
                    jcmd.setTicks();
                    pController->sendResponse(jcmd, status);
                }
                status = STATUS_BUSY_PARSED;
                jcmd.cmdIndex++;
            }
        } else {
            status = STATUS_OK;
        }
    } else {
        status = STATUS_JSON_CMD;
    }

    jcmd.setTicks();
    jcmd.setStatus(status);

    if (!isProcessing(status)) {
        pController->sendResponse(jcmd,status);
    }

    return status;
}

