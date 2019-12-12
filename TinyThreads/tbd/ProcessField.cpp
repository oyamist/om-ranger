#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#include <cstdio>
#endif
#include "ProcessField.h"

using namespace firestep;

namespace firestep {
template<>
Status processField<int32_t, int32_t>(JsonObject& jobj, const char *key, int32_t& field, bool *pUpdated) {
    Status status = STATUS_OK;
    const char *s;
    if ((s = jobj[key]) && *s == 0) { // query
        status = (jobj[key] = field).success() ? status : STATUS_FIELD_ERROR;
        if (pUpdated) {
            *pUpdated = false;
        }
    } else {
        field = jobj[key];
        if (pUpdated) {
            *pUpdated = true;
        }
    }
    return status;
}
}


