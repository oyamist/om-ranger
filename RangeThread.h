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

typedef enum ModeType {
    MODE_SLEEP = 0,   // Inactive
    MODE_SWEEP_FORWARD = 1, // Left-right sweep
    MODE_SWEEP_STEP = 2,    // Floor sweep
    MODE_CAL_FLOOR = 3,     // Calibrate floor height
} ModeType;

typedef class RangeThread : om::Thread {
public:
    RangeThread();
    void setup(uint8_t port=2, uint16_t msLoop=VL53L0X_PERIOD);
    
protected:
    uint8_t port;
    uint16_t msLoop;
    void loop();
    RangeType rng = RNG_UNKNOWN;
    uint32_t minRange = 120L;
    uint32_t maxRange = 2000L;
    int32_t eaDistFast = 0;
    int32_t eaDistSlow = 0;
    int32_t eaDistSleep = 2000L;
    int32_t eaDistErr = 0; 
    uint32_t msIdle = 0;
    int32_t msCalFloor = 0;
    uint32_t msUnsteady = 0;
    int32_t stepFloor = 0;
    int32_t hStick = 0;
    int32_t hCal = 0;
    int32_t h = 0;
    int32_t pitch;
    ModeType mode;
    AxisState * px = &accelThread.xState;
    AxisState * py = &accelThread.yState;
    AxisState * pz = &accelThread.zState;

    void sweepForward(uint16_t dist);
    void sweepStep(uint16_t dist);
    void sweepStepDeprecated(uint16_t dist);
    void calFloor(uint16_t dist);
    void setMode(ModeType mode, bool force=false);
    void updateOledPosition();
    void lraCalibrating(bool done=false);
} RangeThread;

extern RangeThread rangeThread;


#endif
