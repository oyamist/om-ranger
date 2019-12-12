#ifndef PROGMEM_H
#define PROGMEM_H

#ifdef CMAKE
#include <cstring>
#include <cstdio>
#endif

#ifdef ARDUINO
#define strcmp_PS(a,b) strcmp_P(b,a)
#else
#define PROGMEM
#define strcpy_P(dst,src) strcpy(dst,src)
#define strlen_P(src) strlen(src)
#define strcmp_PS(a,b) strcmp(a,b)
#define pgm_read_byte_near(src) (*(const char *)(src))
#endif

#include "Status.h"
#include "JsonCommand.h"

namespace firestep {

const char *prog_src(const char *name);
Status prog_dump(const char *name);
Status prog_load_cmd(const char *name, JsonCommand &jcmd);


extern const char OP_app[];
extern const char OP_ch[];
extern const char OP_cmt[];
extern const char OP_dbg[];
extern const char OP_dbgfr[];
extern const char OP_dbglp[];
extern const char OP_dbgtc[];
extern const char OP_eep[];
extern const char OP_eu[];
extern const char OP_fr[];
extern const char OP_git[];
extern const char OP_hp[];
extern const char OP_id[];
extern const char OP_idl[];
extern const char OP_io[];
extern const char OP_jp[];
extern const char OP_lp[];
extern const char OP_msg[];
extern const char OP_om[];
extern const char OP_pgm[];
extern const char OP_pgmd[];
extern const char OP_pgmx[];
extern const char OP_pu[];
extern const char OP_sys[];
extern const char OP_syseu[];
extern const char OP_sysfr[];
extern const char OP_sysjp[];
extern const char OP_sysom[];
extern const char OP_systc[];
extern const char OP_sysv[];
extern const char OP_tc[];
extern const char OP_v[];
extern const char OP_ver[];
extern const char OP_test1[];
extern const char OP_test2[];
extern const char OP_test[];
extern const char OP_help[];

} // namespace firestep


////////////////////// PROGMEM_H //////////////
#endif
