#ifndef OS_UTIL_H
#define OS_UTIL_H

#ifdef _MSC_VER
#include "winjunk.hpp"
#else
#define CLASS_DECLSPEC
#endif

#ifdef CMAKE
#include <stdint.h>
#include <cstdio>
#include <string>
#endif

#ifdef ARDUINO
#include "Arduino.h"
#define MEMORY_MODEL_TINY
#endif

#ifdef MEMORY_MODEL_TINY
// MEMORY_MODEL_TINY: We have no room for niceties
#define PHVECTOR tinyvector
#define VECTOR_SIZE 5
#define PH5TYPE float
#else
// MEMORY_MODEL_LARGE: do it right
#include <vector>
#include <cstdlib>
#include "FireLog.h"
#include "FireUtils.hpp"
#define PHVECTOR vector
#define PH5TYPE double
#endif

#ifndef ASSERT
#define ASSERT(x)
#define ASSERTFAIL(x)
#define ASSERTEQUALT(a,b,c)
#define ASSERTEQUAL(a,b)
#define ASSERTEQUALS(a,b)
#define firelog_level(x)
#define LOGERROR1(a,b)
#define LOGDEBUG1(a,b)
#define LOGINFO1(a,b)
#define LOGINFO2(a,b,c)
#define LOGINFO3(a,b,c,d)
#endif

#include <math.h>

using namespace std;

#endif // OS_UTIL_H
