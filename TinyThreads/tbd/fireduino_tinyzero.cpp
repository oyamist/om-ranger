#include <Arduino.h>
#include "fireduino_tinyzero.h"
#include "Adafruit_ZeroTimer.h"

#define TC_THREAD 4  // TC timer for Threads 0..4
#ifndef TC_INSTANCE_OFFSET
#define TC_INSTANCE_OFFSET 3
#endif

extern "C" {
#define TC_MAX_CALLBACKS (TC_CALLBACK_BITS*3)
#define TC_CALLBACK_BITS 6
static void(* __cb[TC_MAX_CALLBACKS]) (void);
static uint32_t _register_callback_mask = 0;
static uint32_t _enable_callback_mask = 0;
};

static inline bool tc_is_syncing(Tc *const hw) {
#if defined(__SAMD51__)
  return hw->COUNT8.SYNCBUSY.reg > 0;
#else
  return (hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
#endif
}

namespace fireduino {

Adafruit_ZeroTimer aztThread(TC_THREAD);
/*
  Adafruit_ZeroTimer(uint8_t tn);
  boolean PWMout(boolean pwmout, uint8_t channum, uint8_t pin);
  void setPeriodMatch(uint32_t period, uint32_t match, uint8_t channum = 1);
  void enable(boolean en);
  void configure(tc_clock_prescaler prescale, 
                 tc_counter_size countersize, 
                 tc_wave_generation wavegen, 
                 tc_count_direction countdir = TC_COUNT_DIRECTION_UP);
  void setCompare(uint8_t channum, uint32_t compare);
  void invertWave(uint8_t invert);
  void setCallback(boolean enable, tc_callback cb_type, void(* callback_func) (void) = NULL);

  static void timerHandler(uint8_t timerNum);
*/

    @param countersize Can be TC_COUNTER_SIZE_8BIT, TC_COUNTER_SIZE_16BIT or TC_COUNTER_SIZE_32BIT 
    @param wavegen Can be TC_WAVE_WAVEGEN_NFRQ, TC_WAVE_WAVEGEN_MFRQ, TC_WAVE_WAVEGEN_NPWM or TC_WAVE_WAVEGEN_MPWM
    @param countdir Can be TC_COUNT_DIRECTION_UP or TC_COUNT_DIRECTION_DOWN

	uint32_t get_timer250us() {
		return (* (volatile uint16_t *) &TCC1);
	}

	void enable_timer250us(bool enable) {
        aztThread.enable(1);
    }

	void setup_timer250us() {
        tc_clock_prescaler prescale = 1; 
        tc_counter_size countersize = TC_COUNTER_SIZE_16BIT; 
        tc_wave_generation wavegen = TC_WAVE_WAVEGEN_NFRQ;
        aztThread.configure(prescale, countersize, wavegen);
        aztThread.setCallback(1, cb_type, fireduino_timer_handler);

/*
        REG_GCLK_GENDIV =                   // Configure GCLK divider
          //GCLK_GENDIV_DIV(8) |            // 32kHz/8 => 4kHz @ 250us
            GCLK_GENDIV_DIV(80) |           // 8MHz/80 => 100kHz @ 10us
            GCLK_GENDIV_ID(TC_THREAD);      // Select GCLK divider
        while (GCLK->STATUS.bit.SYNCBUSY);  // Wait for synchronization

        REG_GCLK_GENCTRL =                  // Configure GCLK
            GCLK_GENCTRL_IDC |              // Set the duty cycle to 50%
            GCLK_GENCTRL_GENEN |            // Enable GCLK
          //GCLK_GENCTRL_SRC_OSCULP32K |    // Set the 32kHz clock source
            GCLK_GENCTRL_SRC_OSC8M |        // Set the 8MHz clock source
          //GCLK_GENCTRL_SRC_DFLL48M |      // Set the 48MHz clock source
            GCLK_GENCTRL_ID(TC_THREAD);      Select GCLK Control
        while (GCLK->STATUS.bit.SYNCBUSY);  // Wait for synchronization

        REG_GCLK_CLKCTRL =                  // Feed GCLK4 to TCC2 (and TC3)
            GCLK_CLKCTRL_CLKEN |            // Enable GCLK to TCC2 (and TC3)
            GCLK_CLKCTRL_GEN_GCLK4 |        // Select GCLK4 TC_THREAD
            GCLK_CLKCTRL_ID_TCC2_TC3;       // Feed GCLK to TCC2 (and TC3)
        while (GCLK->STATUS.bit.SYNCBUSY);  // Wait for synchronization
        */
	}

	void clear_timer250us() {
        // TODO
	}

} // fireduino
