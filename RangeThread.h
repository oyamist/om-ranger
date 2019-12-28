#ifndef RANGE_THREAD_H
#define RANGE_THREAD_H

#include "src/omilli/Thread.h"
#include "Accel3Thread.h"

#define VL53L0X_PERIOD 33

typedef enum RangeType {
  RNG_UNKNOWN = 0,   // Center flash white
  RNG_TOUCH = 1,     // Steady red
  RNG_CLOSE = 2,     // Quick flash red
  RNG_BODY = 3,      // Slow flash red
  RNG_NEAR = 4,      // Steady green
  RNG_FAR = 5,       // Slow flash green
} RangeType;

typedef class RangeThread : om::Thread {
public:
    RangeThread();
    void setup(uint8_t port=2, uint16_t msLoop=VL53L0X_PERIOD);
    
protected:
    uint8_t port;
    uint16_t msLoop;
    void loop();
    RangeType rng = RNG_UNKNOWN;
    uint32_t lastDist = 8192L;
    uint32_t minRange = 120L;
    uint32_t maxRange = 2000L;
} RangeThread;

extern RangeThread rangeThread;


#endif
