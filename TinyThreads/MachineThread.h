#ifndef MACHINETHREAD_H
#define MACHINETHREAD_H

#include "JsonController.h"

namespace firestep {

typedef class MachineThread : Thread {
protected:
    void displayStatus();
    char * buildStartupJson();
    Status executeEEPROM();
    size_t readEEPROM(uint8_t *eeprom_addr, char *dst, size_t maxLen);

public:
    Status status;
    Machine machine;
    JsonCommand command;
    JsonController jsonController;
    JsonController *pController;
    bool printBannerOnIdle;

public:
    MachineThread();
    void setController(JsonController &controller);
    void setup(PinConfig pc);
    void loop();
    Status syncConfig();
    Status process(JsonCommand& jcmd);
} MachineThread;

} // namespace firestep

#endif
