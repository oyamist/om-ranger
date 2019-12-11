#include <Arduino.h>
#include "ProgMem.h"
#include "Machine.h"

using namespace firestep;

const char firestep::OP_app[] PROGMEM = { "app" };
const char firestep::OP_ch[] PROGMEM = { "ch" };
const char firestep::OP_cmt[] PROGMEM = { "cmt" };
const char firestep::OP_dbg[] PROGMEM = { "dbg" };
const char firestep::OP_dbgfr[] PROGMEM = { "dbgfr" };
const char firestep::OP_dbglp[] PROGMEM = { "dbglp" };
const char firestep::OP_dbgtc[] PROGMEM = { "dbgtc" };
const char firestep::OP_eep[] PROGMEM = { "eep" };
const char firestep::OP_eu[] PROGMEM = { "eu" };
const char firestep::OP_fr[] PROGMEM = { "fr" };
const char firestep::OP_git[] PROGMEM = { "git" };
const char firestep::OP_hp[] PROGMEM = { "hp" };
const char firestep::OP_id[] PROGMEM = { "id" };
const char firestep::OP_idl[] PROGMEM = { "idl" };
const char firestep::OP_io[] PROGMEM = { "io" };
const char firestep::OP_jp[] PROGMEM = { "jp" };
const char firestep::OP_lp[] PROGMEM = { "lp" };
const char firestep::OP_msg[] PROGMEM = { "msg" };
const char firestep::OP_om[] PROGMEM = { "om" };
const char firestep::OP_pgm[] PROGMEM = { "pgm" };
const char firestep::OP_pgmd[] PROGMEM = { "pgmd" };
const char firestep::OP_pgmx[] PROGMEM = { "pgmx" };
const char firestep::OP_pu[] PROGMEM = { "pu" };
const char firestep::OP_sys[] PROGMEM = { "sys" };
const char firestep::OP_syseu[] PROGMEM = { "syseu" };
const char firestep::OP_sysfr[] PROGMEM = { "sysfr" };
const char firestep::OP_sysjp[] PROGMEM = { "sysjp" };
const char firestep::OP_sysom[] PROGMEM = { "sysom" };
const char firestep::OP_systc[] PROGMEM = { "systc" };
const char firestep::OP_sysv[] PROGMEM = { "sysv" };
const char firestep::OP_tc[] PROGMEM = { "tc" };
const char firestep::OP_v[] PROGMEM = { "v" };
const char firestep::OP_ver[] PROGMEM = { "ver" };

const char firestep::OP_test1[] PROGMEM = { "test1" };
const char firestep::OP_test2[] PROGMEM = { "test2" };
const char firestep::OP_test[] PROGMEM = { "test" };
const char firestep::OP_help[] PROGMEM = { "help" };

const char src_help[] PROGMEM = {
    "["
    "{\"msg\":\"https://goo.gl/ZXEN3t\"}"
    "]"
};

const char src_test1[] PROGMEM = {
    "{\"msg\":\"test A\"}"
};

const char src_test2[] PROGMEM = {
    "["
    "{\"msg\":\"test A\"},"
    "{\"msg\":\"test B\"}"
    "]"
};

const char *firestep::prog_src(const char *name) {
    if (false) { // must be sorted reverse alphabetical

    } else if (strcmp_PS(OP_test, name) == 0) {
        return src_test2;
    } else if (strcmp_PS(OP_help, name) == 0) {
        return src_help;
    } else if (strcmp_PS(OP_test1, name) == 0) {
        return src_test1;
    } else if (strcmp_PS(OP_test2, name) == 0) {
        return src_test2;
    }

    return NULL;
}

Status firestep::prog_dump(const char *name) {
    const char *src = prog_src(name);
	if (src == NULL) {
		return STATUS_UNKNOWN_PROGRAM;
	}
    TESTCOUT3("prog_dump:", name, " bytes:", (strlen_P(src)+1), " src:", src);

    for (size_t i = 0; i<MAX_JSON; i++) {
        char c = pgm_read_byte_near(src + i);
        ASSERT(c == 0 || ' ' <= c && c <= '~');
        if (c) {
            fireduino::serial_print(c);
        } else {
            fireduino::serial_println();
            break;
        }
    }

    return STATUS_OK;
}

Status firestep::prog_load_cmd(const char *name, JsonCommand &jcmd) {
    char nameBuf[32];
    snprintf(nameBuf, sizeof(nameBuf), "%s", name); // name is volatile
    Status status = STATUS_OK;
    const char *src = prog_src(nameBuf);
	if (src == NULL) {
		return STATUS_UNKNOWN_PROGRAM;
	}
    TESTCOUT2("prog_load:", nameBuf, " src:", src);

    size_t len = strlen_P(src);
    if (MAX_JSON <= len+1) {
        return STATUS_PROGRAM_SIZE;
    }
    if (len <= 0) {
        return STATUS_UNKNOWN_PROGRAM;
    }

    ///////// DANGER /////////
    // We will replace the currently running command with the program
    // name will no longer be valid
    jcmd.clear();
    ///////// DANGER /////////

    char *buf = jcmd.allocate(len+1);
    ASSERT(buf);
    for (size_t i = 0; i < len; i++) {
        buf[i] = pgm_read_byte_near(src + i);
        ASSERT(' ' <= buf[i] && buf[i] <= '~');
    }
    buf[len] = 0;
    TESTCOUT3("prog_load_cmd:", nameBuf, " buf:", buf, " status:", status);
    if (status != STATUS_OK) {
        TESTCOUT1("prog_load_cmd:", status);
        return status;
    }

    status = jcmd.parse(buf, STATUS_WAIT_IDLE);
    if (status < 0) {
        TESTCOUT2("prog_load_cmd:", nameBuf, " parse error:", status); // should never happen
    } else {
        TESTCOUT2("prog_load_cmd:", nameBuf, " parse status:", status); // STATUS_BUSY_PARSED 10
    }

    return status;
}

