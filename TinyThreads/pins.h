#ifndef PINS_H
#define PINS_H

#define NOPIN 255
typedef uint8_t PinType;		// pin specification

enum PinConfig {
    PC0_NOPIN = 0,
};

#define MAX_PIN				   69

///////////////////// PC0_NOPIN ////////////////
// No pin assignments
////////////////////////////////////////////////

#define PC0_X_STEP_PIN          NOPIN
#define PC0_X_DIR_PIN           NOPIN
#define PC0_X_ENABLE_PIN        NOPIN
#define PC0_X_MIN_PIN           NOPIN
#define PC0_X_MAX_PIN           NOPIN

#define PC0_Y_STEP_PIN          NOPIN
#define PC0_Y_DIR_PIN           NOPIN
#define PC0_Y_ENABLE_PIN        NOPIN
#define PC0_Y_MIN_PIN           NOPIN
#define PC0_Y_MAX_PIN           NOPIN

#define PC0_Z_STEP_PIN          NOPIN
#define PC0_Z_DIR_PIN           NOPIN
#define PC0_Z_ENABLE_PIN        NOPIN
#define PC0_Z_MIN_PIN           NOPIN
#define PC0_Z_MAX_PIN           NOPIN

#define PC0_STATUS_PIN			NOPIN

#endif
