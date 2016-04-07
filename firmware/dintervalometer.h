/*
 *  dintervalometer.h
 *
 *  "THE DON'T-CARE LICENCE"
 *  I don't care, do whatever you want with this file.
 *
 *  Author: Daniel Knezevic
 *  E-mail: danielknezevic90<at>gmail.com
 */

#ifndef DINTERVALOMETER_H_
#define DINTERVALOMETER_H_

#define TRUE                    1
#define FALSE                   0

#define ONE_SECOND              1000    // milliseconds

#define BUTTON_HOLD_INTERVAL    1000    // miliseconds to pronounce button as held rather than pressed
#define UNLOCK_INTERVAL         2000    // milliseconds
#define TIMEOUT_INTERVAL        2000    // milliseconds
#define DEBOUNCE_INTERVAL       50      // milliseconds

#define LEFT_BUTTON_DDR         DDRC
#define LEFT_BUTTON_PIN         PINC
#define LEFT_BUTTON_PORT        PORTC
#define LEFT_BUTTON_IO          PC3

#define RIGHT_BUTTON_DDR        DDRC
#define RIGHT_BUTTON_PIN        PINC
#define RIGHT_BUTTON_PORT       PORTC
#define RIGHT_BUTTON_IO         PC2

#define AUTO_CUTOFF_DDR         DDRC
#define AUTO_CUTOFF_PIN         PINC
#define AUTO_CUTOFF_PORT        PORTC
#define AUTO_CUTOFF_IO          PC1

#define SHUTTER_DDR             DDRD
#define SHUTTER_PIN             PIND
#define SHUTTER_PORT            PORTD
#define SHUTTER_IO              PD7

#define FOCUS_DDR               DDRD
#define FOCUS_PIN               PIND
#define FOCUS_PORT              PORTD
#define FOCUS_IO                PD6

#define CHG_DDR                 DDRD
#define CHG_PIN                 PIND
#define CHG_PORT                PORTD
#define CHG_IO                  PD0

#define USB_PWR_DDR             DDRD
#define USB_PWR_PIN             PIND
#define USB_PWR_PORT            PORTD
#define USB_PWR_IO              PD1

#define BACKLIGHT_DDR           DDRD
#define BACKLIGHT_PIN           PIND
#define BACKLIGHT_PORT          PORTD
#define BACKLIGHT_IO            PD3
#define BACKLIGHT_DURATION      10000   // millisecond

#define BLINK_PERIOD           1000     // millisecons
#define ICON_WIDTH              8

#define MAX_BATTERY_VOLTAGE     4200    // mV
#define MIN_BATTERY_VOLTAGE     3600    // mV
#define BATTERY_MONITOR_PERIOD  60000   // milliseconds

#define BACKLIGHT_INTENSITY     63

typedef enum state_e {
    MAIN_MENU = 0,
    INTERVALOMETER_MENU,
    BULB_MODE_MENU,
    SETTINGS,
    OFF,
    INTERVALOMETER,
    BULB_MODE,
    INIT,
    NUM_OF_STATES
} state_t;

typedef state_t state_func_t(void);

typedef struct intervalometer_s {
    uint16_t delay;
    uint16_t period;
    uint16_t shots;
    uint8_t prefocus;
} intervalometer_t;

typedef struct bulb_mode_s {
    uint16_t delay;
    uint16_t duration;
} bulb_mode_t;

typedef struct settings_s {
    uint8_t backlight_intensity;
    uint16_t battery_auto_cutoff;   //mV
} settings_t;

#endif /* DINTERVALOMETER_H_ */
