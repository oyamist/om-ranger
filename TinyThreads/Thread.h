#ifndef THREAD_H
#define THREAD_H

#include "fireduino.h"
extern uint32_t fireduino_timer;
extern void fireduino_timer_handler();

namespace tinythreads {

extern int16_t leastFreeRam;

/**
 * A Tick is 1 millisecond for all TinyThreads implementations. 
 * Clock overflows in about 50 days
 */
typedef uint32_t Ticks;

typedef struct ThreadClock  {
    Ticks ticks;
    ThreadClock() : ticks(0) {}
} ThreadClock, *ThreadClockPtr;

typedef struct Thread {
public:
    Thread() : pNext(NULL), tardies(0), id(0) {
        nextLoop.ticks = 0;
    }
    virtual void setup();
    virtual void loop() {}

    struct Thread *pNext;

    // Threads should increment the loop as desired.
    // Threads with 0 nextLoop will always run ASAP.
    ThreadClock nextLoop;

    uint8_t tardies;
    char id;
}
Thread, *ThreadPtr;

// Binary pulse with variable width
typedef struct PulseThread : Thread {
    virtual void setup(Ticks period, Ticks pulseWidth);
    virtual void loop();

    bool isHigh;

protected:
    Ticks m_LowPeriod;
    Ticks m_HighPeriod;
} PulseThread, *PulseThreadPtr;

typedef class MonitorThread : PulseThread {
    friend class ThreadRunner;
    friend class MachineThread;

private:
    uint8_t	blinkLED;
    int16_t pinLED; /* PRIVATE */

private:
    void 	LED(uint8_t value);
    void setup(int pinLED = NOPIN); /* PRIVATE */
    unsigned int Free(); /* PRIVATE */
    void loop(); /* PRIVATE */

public:
    bool verbose;
public:
    void Error(const char *msg, int value); /* PRIVATE */
} MonitorThread;

void Error(const char *msg, int value);
void ThreadEnable(bool enable);

extern MonitorThread monitor;

extern struct Thread *pThreadList;
extern int nThreads;
extern int32_t nLoops;
extern int32_t nTardies;

typedef class ThreadRunner {
private:
    uint8_t		testTardies;
    int16_t		nHB;
    uint8_t		fast;
public:
    ThreadRunner();
    void clear();
    void setup(int pinLED = NOPIN);

public:
    void run() {
        // outer loop: bookkeeping
        for (;;) {
            outerLoop();
        }
    }
public:
    inline uint8_t get_testTardies() {
        return testTardies;
    }
public:
    inline void outerLoop() {
        if (fast-- && innerLoop()) {
            // do nothing
        } else {
            nLoops += nHB;
            nHB = 0;
            fast = 255;
        }
    }
public:
    inline Ticks ticks() { 
        return millis();
    }
    inline uint8_t innerLoop() {
        // inner loop: run active Threads
        Ticks t = ticks();
        for (ThreadPtr pThread = pThreadList; pThread; pThread = pThread->pNext) {
            if (t < pThread->nextLoop.ticks) {
                continue; // not time yet for scheduled reactivation
            }

            pThread->loop();	// reactivate thread
            nHB++;

            if (testTardies-- == 0) {
                testTardies = 5;	// test intermittently for late Threads
                t = ticks();
                if (t <= pThread->nextLoop.ticks) {
                    continue;    // transient ASAP or future
                }
                if (0 == pThread->nextLoop.ticks) {
                    continue;    // permanent ASAP
                }

                pThread->tardies++;	// thread-specific tardy count
                nTardies++;			// global tardy count
            }
        }
        return 1;
    }
} ThreadRunner;
extern ThreadRunner threadRunner;

extern Ticks ticks();

} // namespace tinythreads

#endif
