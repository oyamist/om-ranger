#ifndef JSONCONTROLLER_H
#define JSONCONTROLLER_H

#include "version.h"
#include "Thread.h"
#include "Status.h"
#include "Machine.h"
#include "JsonCommand.h"

namespace firestep {

typedef class JsonController {
private:
protected:
    Machine &machine;

protected:
    virtual Status processDebug(JsonCommand& jcmd, JsonObject& jobj, const char* key);
    virtual Status processEEPROM(JsonCommand& jcmd, JsonObject& jobj, const char* key);
    virtual Status processEEPROMValue(JsonCommand& jcmd, JsonObject& jobj, const char* key, const char *addr);
    virtual Status processIO(JsonCommand& jcmd, JsonObject& jobj, const char* key, bool pullUp=false);
    virtual Status processIOPin(JsonCommand& jcmd, JsonObject& jobj, const char* key, bool pullUp);
    virtual Status processPin(JsonObject& jobj, const char *key, PinType &pin, int16_t mode, int16_t value = LOW);
    virtual Status processSys(JsonCommand& jcmd, JsonObject& jobj, const char* key);
    virtual Status process_id(JsonCommand& jcmd, JsonObject& jobj, const char* key);

public:
    JsonController(Machine& machine);

public:
    void sendResponse(JsonCommand& jcmd, Status status, bool final=true);
    Status processObj(JsonCommand& jcmd, JsonObject&jobj);
    JsonController& operator=(JsonController& that);
    Status cancel(JsonCommand &jcmd, Status cause);
    const char *name();

} JsonController;


} // namespace firestep

#endif
