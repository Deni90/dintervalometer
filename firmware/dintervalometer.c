/*
 *  dintervalometer.c
 *
 *  "THE DON'T-CARE LICENCE"
 *  I don't care, do whatever you want with this file.
 *
 *  Author: Daniel Knezevic
 *  E-mail: danielknezevic90<at>gmail.com
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <math.h>

#include "dintervalometer.h"
#include "pcd8544.h"

// String constants and icons stored in flash
const __flash char turn_on_message_str[]
= "Hold buttons for 2s\n     to turn on\n";
const __flash char main_menu_title_str[] = "MAIN MENU\n";
const __flash char intervalometer_menu_title_str[] = "INTERVALOMETER\n";
const __flash char bulb_mode_menu_title_str[] = "BULB MODE\n";
const __flash char settings_menu_title_str[] = "SETTINGS\n";
const __flash char intervalometer_option_str[] = " Intervalometer\n";
const __flash char bulb_mode_option_str[] = " Bulb mode\n";
const __flash char settings_option_str[] = " Settings\n";
const __flash char turn_off_option_str[] = " Turn off\n";
const __flash char delay_str[] = " delay  ";
const __flash char interval_str[] = " period ";
const __flash char shots_str[] = " shots       ";
const __flash char prefocus_str[] = " prefocus    ";
const __flash char start_str[] = " start";
const __flash char duration_str[] = " bulb   ";
const __flash char on_str[] = " on";
const __flash char off_str[] = "off";
const __flash char done_str[] = "Done";
const __flash char help_bar_1_str[] = "down/set        up";
const __flash char help_bar_2_str[] = "down/set  up/back";
const __flash char help_bar_3_str[] = "++/set       --/set";
const __flash char abort_str[] = "/abort";
const __flash char back_str[] = "/back";
const __flash char cleanup_message_str[] = "    Turning off\n    bye, bye :)\n";
const __flash char backlight_str[] = " backlight    ";
const __flash char auto_cutoff_str[] = " auto cutoff ";
const __flash char low_battery_str[] = "Low battery!";
const __flash char charging_str[] = "Charging";
const __flash char full_battery_str[] = "Battery is full";

const __flash char clock_icon[] = {0x00, 0x18, 0x24, 0x42, 0x5e, 0x52, 0x24, 0x18};
const __flash char bulb_icon[] = {0x00, 0x00, 0x02, 0x2d, 0x69, 0x2d, 0x02,
    0x00};
const __flash char settings_icon[] = {0x00, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x24,
    0x24};
const __flash char turn_off_icon[] = {0x00, 0x18, 0x24, 0x40, 0x4e, 0x40, 0x24,
    0x18};
const __flash char delay_icon[] = {0x00, 0x41, 0x63, 0x75, 0x6d, 0x75, 0x63,
    0x41};
const __flash char focus_icon[] = {0x00, 0x63, 0x41, 0x1c, 0x14, 0x1c, 0x41,
    0x63};
const __flash char image_icon[] = {0x00, 0x7f, 0x41, 0x41, 0x4f, 0x49, 0x4a,
    0x7c};
const __flash char start_icon[] = {0x00, 0x00, 0x7f, 0x7f, 0x3e, 0x1c, 0x08,
    0x00};
const __flash char battery_icon_elements[] = {0x1c, 0x63, 0x41, 0x7f};
const __flash char image_icon_top[] = {0x00, 0x00, 0x00, 0x00, 0xfe, 0x02, 0x02,
    0x02, 0x02, 0x7e, 0x42, 0x44, 0x48, 0x50, 0xe0, 0x00, 0x00, 0x00,
    0x00};
const __flash char image_icon_bottom[] = {0x00, 0x00, 0x00, 0x00, 0x3f, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3f, 0x00, 0x00, 0x00,
    0x00};
const __flash char delay_icon_top[] = {0x00, 0x00, 0x00, 0x02, 0x06, 0x0A,
    0x12, 0x62, 0xE2, 0xE2, 0xE2, 0x62, 0x12, 0x0A, 0x06, 0x02, 0x00, 0x00,
    0x00};
const __flash char delay_icon_bottom[] = {0x00, 0x00, 0x00, 0x40, 0x60, 0x70,
    0x78, 0x76, 0x71, 0x73, 0x71, 0x76, 0x78, 0x70, 0x60, 0x40, 0x00, 0x00,
    0x00};
const __flash char bulb_icon_top[] = {0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x82,
    0x02, 0x32, 0xD2, 0x32, 0x02, 0x82, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00};
const __flash char bulb_icon_bottom[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x0E, 0x38, 0x7F, 0x38, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00};
const __flash char battery_icon[] = {0x1c, 0x36, 0x22, 0x22, 0x22, 0x3e, 0x3e,
    0x3e};
const __flash char battery_tray_icon[] = {0x0e, 0x1f, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x1f};
const __flash char backlight_icon[] = {0x08, 0x2a, 0x1c, 0x7f, 0x1c, 0x2a, 0x08,
    0x00};

// RAM varibles used for measuring/counting time for various actions
volatile unsigned int clock;                    // used for global timer
volatile unsigned int blink_timer = 0;         //used for blinking in menus
volatile unsigned int button_hold_timer = 0;  // counter for press/hold decision
volatile unsigned int unlock_timer = 0;         // timer to unlock the device
volatile unsigned int timeout_timer = 0; // timer to turn off the device on startup when there is no input from the user
volatile unsigned int delay_timer = 0; // used for interrupt based delay function
volatile unsigned int debounce_timer = 0;       // used for button debunce
volatile unsigned int backlight_timer = 0; // backlight is on until the value reaches 0
volatile unsigned int battery_check_timer = 0;  // timer for battery management

// RAM variables used to store information related to states of the buttons
volatile uint8_t button_press = 0x00;
volatile uint8_t button_hold = 0x00;
volatile uint8_t last_button_state = 0x00;

// Settings with initial values stored in eeprom
settings_t EEMEM settings_data_backup = { 0, MIN_BATTERY_VOLTAGE };

// Global variables
state_t cur_state = INIT;
uint8_t selected_menu_item = 0;
uint8_t is_prop_selected = 0;
uint8_t is_blinking = 0;
intervalometer_t intervalometer_data = { 0, 0, 0, 0 };
intervalometer_t intervalometer_temp_data = { 0, 0, 0, 0 };
uint8_t intervalometer_pre_start = FALSE;
uint16_t remaining_time = 0; // seconds
uint8_t update_image_icon = 0;
bulb_mode_t bulb_mode_data = { 0, 0 };
bulb_mode_t bulb_mode_temp_data = { 0, 0 };
uint8_t bulb_mode_pre_start = FALSE;
uint8_t number_part = 0;
settings_t settings_data;

// State functions
state_t STATE_init(void);
state_t STATE_main_menu(void);
state_t STATE_intervalometer_menu(void);
state_t STATE_bulb_mode_menu(void);
state_t STATE_settings_menu(void);
state_t STATE_intervalometer(void);
state_t STATE_bulb_mode(void);
state_t STATE_turn_off(void);

// state functions must be in same order as states in state_t enum (look in dintervalometer.h)
state_func_t* const state_table[NUM_OF_STATES] = { STATE_main_menu,
        STATE_intervalometer_menu, STATE_bulb_mode_menu, STATE_settings_menu,
        STATE_turn_off, STATE_intervalometer, STATE_bulb_mode, STATE_init };

state_t STATE_run_state(state_t cur_state) {
    return state_table[cur_state]();
}

// Misc
/*
 * Initializes timer1
 */
void TIMER_init(void);

/*
 * Interrupt based delay function 1-65536ms (65sec max)
 */
void TIMER_delay_ms(uint16_t _mS);

/*
 * Initializes ADC on ADC0(PC0) for battery voltage monitoring
 */
void BATTERY_init_adc(void);

/*
 * Reads battery voltage by using ADC.
 * return value: measured value in mV
 */
uint16_t BATTERY_read_voltage(void);

/*
 * Periodically measures battery voltage and if the voltage is under the
 * margin notifies the user with some message and turns off the device
 */
void BATTERY_monitor(void);

/*
 * Initializes timer2 for PWM on OC2B(PD3), PWM frewuency should be few kHz
 */
void BACKLIGHT_init_pwm(void);

/*
 * Updates backlight status. It will be turned on if brightness > 0
 */
void BACKLIGHT_update(uint8_t brightness);

/*
 * Basic GPIO initialization
 */
void GPIO_init(void);

/*
 * Prints hh:mm:ss time format from seconds
 */
void LCD_print_time(uint16_t _value, uint8_t size);

/*
 * Draws a scroll bar on LCD based on number of items and on currently
 * selected item
 */
void LCD_update_scroll_bar(uint8_t selected_item, uint8_t total_items);

/*
 * Draws a battery icon on top right corner of the LCD based on the measured
 * battery voltage
 */
void LCD_update_battery_icon(uint16_t voltage);

/*
 * Shows low battery notification
 */
void LCD_show_low_battery_alert(void);

/*
 * Timer1 "Compare Match" ISR
 */
ISR(TIMER1_COMPA_vect, ISR_NOBLOCK) {
    clock++;
    blink_timer++;
    delay_timer++;
    debounce_timer++;
    battery_check_timer++;
    if (button_hold_timer)
        button_hold_timer--;   // to detect button holds, we shouldn't overflow!
    if (unlock_timer)
        unlock_timer--;         // to unlock the device, we shouldn't overflow!
    if (timeout_timer)
        timeout_timer--;       // to turn off the device, we shouldn't overflow!
    if (backlight_timer)
        backlight_timer--;
    else
        OCR2B = 0; // turn off the backlight
}

/*
 * Interrupt based button sensing
 * Code borrowed and modified from:
 * http://www.elektronika.ba/789/reverse-geocaching-geogame-v1/
 */
ISR(PCINT1_vect, ISR_NOBLOCK) {
    PCICR &= ~(1 << PCIE1);     // ..disable interrupts for the entire section

    /* up/set button handling part */
    if (!(LEFT_BUTTON_PIN & (1 << LEFT_BUTTON_IO))) {   // if is button pressed
        // see if it wasn't pressed at all
        if (!(last_button_state & (1 << LEFT_BUTTON_IO))) {
            TIMER_delay_ms(DEBOUNCE_INTERVAL);                       // debounce
            // if it is still not VCC after DEBOUNCE_INTERVAL, this was an error
            if ((LEFT_BUTTON_PIN & (1 << LEFT_BUTTON_IO))) {
                PCICR |= (1 << PCIE1); // ..re-enable interrupts for the entire section
                return;
            }
            last_button_state |= (1 << LEFT_BUTTON_IO);
            button_hold_timer = BUTTON_HOLD_INTERVAL; // load for hold detection
            while (button_hold_timer > 0) {
                // if button released in meantime
                if ((LEFT_BUTTON_PIN & (1 << LEFT_BUTTON_IO))) {
                    button_press = (1 << LEFT_BUTTON_IO); // this was just a press!
                    PCICR |= (1 << PCIE1); // ..re-enable interrupts for the entire section
                    return;
                }
            }
            // if we get to this line, the button is still pressed, so "it is held"
            button_hold = (1 << LEFT_BUTTON_IO);
        }
    } else {
        last_button_state &= ~(1 << LEFT_BUTTON_IO);
    }

    /* down/back button handling part */
    if (!(RIGHT_BUTTON_PIN & (1 << RIGHT_BUTTON_IO))) { // if button is pressed
        // see if it wasn't pressed at all
        if (!(last_button_state & (1 << RIGHT_BUTTON_IO))) {
            TIMER_delay_ms(DEBOUNCE_INTERVAL);                       // debounce
            // if it is still not VCC after DEBOUNCE_INTERVAL, this was an error
            if ((RIGHT_BUTTON_PIN & (1 << RIGHT_BUTTON_IO))) {
                PCICR |= (1 << PCIE1); // ..re-enable interrupts for the entire section
                return;
            }
            last_button_state |= (1 << RIGHT_BUTTON_IO);
            button_hold_timer = BUTTON_HOLD_INTERVAL; // load for hold detection
            while (button_hold_timer > 0) {
                // if button released in meantime
                if ((RIGHT_BUTTON_PIN & (1 << RIGHT_BUTTON_IO))) {
                    button_press = (1 << RIGHT_BUTTON_IO); // this was just a press!
                    PCICR |= (1 << PCIE1); // ..re-enable interrupts for the entire section
                    return;
                }
            }
            // if we get to this line, the button is still pressed, so "it is held"
            button_hold = (1 << RIGHT_BUTTON_IO);
        }
    } else {
        last_button_state &= ~(1 << RIGHT_BUTTON_IO);
    }

    PCICR |= (1 << PCIE1);  // ..re-enable interrupts for the entire section
}

int main(void) {
    TIMER_init();
    BATTERY_init_adc();
    BACKLIGHT_init_pwm();
    GPIO_init();
    // Enable Global Interrupts
    sei();
    LCD_init();

    // restore settings from eeprom
    eeprom_read_block(&settings_data, &settings_data_backup,
            sizeof(settings_t));

    while (1) {
        cur_state = STATE_run_state(cur_state);   //start the state machine
    }   // while
    return 0;
}

state_t STATE_init(void) {
    state_t next_state = cur_state;
    PCICR &= ~(1 << PCIE1); // Disable PIN Change Interrupt 1 - This disables interrupts on pins PCINT14...8 see p73 of datasheet

    BACKLIGHT_update(settings_data.backlight_intensity);

    LCD_clear();

    // First check are the device powered with USB (Charging the battery)
    if (USB_PWR_PIN & (1 << USB_PWR_IO)) {
        if (CHG_PIN & (1 << CHG_IO)) {
            // charging in progress
            LCD_goto(25, 2);
            LCD_puts(charging_str, LCD_TYP_NORM);
        }
        // block until the battery is full -> CHG = 0
        while (CHG_PIN & (1 << CHG_IO))
            ;
        // battery is full
        LCD_clear();
        LCD_goto(12, 2);
        LCD_puts(full_battery_str, LCD_TYP_NORM);

    }
    // block until the USB is connected
    while (USB_PWR_PIN & (1 << USB_PWR_IO))
        ;

    LCD_update_battery_icon(BATTERY_read_voltage());
    LCD_goto(0, 2);
    // show help message to turn on the device
    LCD_puts(turn_on_message_str, LCD_TYP_NORM);
    timeout_timer = TIMEOUT_INTERVAL;   // start timer
    AUTO_CUTOFF_PORT |= (1 << AUTO_CUTOFF_IO);  // pull up the port

    while (1) {
        // if it reaches timeout turn off the device
        if (timeout_timer == 0) {
            next_state = OFF;
            break;
        }
        // check the buttons. If the buttons are held until the UNLOCK_INTERVAL unlock the device.
        if (!(LEFT_BUTTON_PIN & (1 << LEFT_BUTTON_IO))
                && !(RIGHT_BUTTON_PIN & (1 << RIGHT_BUTTON_IO))) {
            TIMER_delay_ms(DEBOUNCE_INTERVAL);    // debounce
            // if some of the buttons are released in the meantime, turn off the device
            if ((LEFT_BUTTON_PIN & (1 << LEFT_BUTTON_IO))
                    || (RIGHT_BUTTON_PIN & (1 << RIGHT_BUTTON_IO))) {
                next_state = OFF;
                break;
            }
            last_button_state |= (1 << LEFT_BUTTON_IO) | (1 << RIGHT_BUTTON_IO);
            uint8_t i;
            // Show initial progress bar and battery icon
            LCD_clear();
            LCD_update_battery_icon(BATTERY_read_voltage());
            LCD_goto(10, 2);
            for (i = 0; i < 64; i++) {
                if (i == 0 || i == 63)
                    LCD_writeData(0xff);
                else
                    LCD_writeData(0x81);
            }
            // prepare for successful unlock
            next_state = MAIN_MENU;
            selected_menu_item = 0;
            // start unlock_timer
            unlock_timer = UNLOCK_INTERVAL;
            while (unlock_timer > 0) {
                // Update progress bar
                LCD_goto(10, 2);
                for (i = 0; i < 64; i++) {
                    if (i < 64 - unlock_timer / 31 || i == 63)
                        LCD_writeData(0xff);
                    else
                        LCD_writeData(0x81);
                }
                // check if the buttons are released in the meantime, if yes turn off the device
                if ((LEFT_BUTTON_PIN & (1 << LEFT_BUTTON_IO))
                        || (RIGHT_BUTTON_PIN & (1 << RIGHT_BUTTON_IO))) {
                    last_button_state &= ~(1 << LEFT_BUTTON_IO)
                            & ~(1 << RIGHT_BUTTON_IO);
                    next_state = OFF;
                    break;
                }
            }
            break;
        }
    }
    return next_state;
}

state_t STATE_main_menu(void) {
    uint8_t menu_size;
    state_t next_state = cur_state;
    PCICR &= ~(1 << PCIE1); // Disable PIN Change Interrupt 1 - This disables Interrupt 1 - This enables interrupts on pins PCINT14...8 see p73 of datasheet
    // main menu
    LCD_clear();
    LCD_update_battery_icon(BATTERY_read_voltage());
    menu_size = 4;
    // menu title
    LCD_puts(main_menu_title_str, LCD_TYP_UP2);
    // intervalometer menu item (icon + name)
    LCD_putCustomChar(clock_icon, ICON_WIDTH,
            (selected_menu_item == 0) ? LCD_TYP_INV : LCD_TYP_NORM);
    LCD_puts(intervalometer_option_str,
            (selected_menu_item == 0) ? LCD_TYP_INV : LCD_TYP_NORM);
    // bulb mode menu item  (icon + name)
    LCD_putCustomChar(bulb_icon, ICON_WIDTH,
            (selected_menu_item == 1) ? LCD_TYP_INV : LCD_TYP_NORM);
    LCD_puts(bulb_mode_option_str,
            (selected_menu_item == 1) ? LCD_TYP_INV : LCD_TYP_NORM);
    // settings menu item  (icon + name)
    LCD_putCustomChar(settings_icon, ICON_WIDTH,
            (selected_menu_item == 2) ? LCD_TYP_INV : LCD_TYP_NORM);
    LCD_puts(settings_option_str,
            (selected_menu_item == 2) ? LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putCustomChar(turn_off_icon, ICON_WIDTH,
            (selected_menu_item == 3) ? LCD_TYP_INV : LCD_TYP_NORM);
    LCD_puts(turn_off_option_str,
            (selected_menu_item == 3) ? LCD_TYP_INV : LCD_TYP_NORM);
    // help bar on the bottom
    LCD_goto(0, 5);
    LCD_puts(help_bar_1_str, LCD_TYP_NORM);
    // draw scroll bar
    LCD_update_scroll_bar(selected_menu_item, menu_size);

    PCICR |= (1 << PCIE1); // Enable PIN Change Interrupt 1 - This enables interrupts on pins PCINT14...8 see p73 of datasheet
    // wait button press or hold
    while (!(button_press | button_hold)) {
        // periodically check the battery status
        BATTERY_monitor();
    }

    // turn on the backlight (if enabled) when some button is pressed/held
    BACKLIGHT_update(settings_data.backlight_intensity);

    if (button_press & (1 << LEFT_BUTTON_IO)) {
        button_press &= ~(1 << LEFT_BUTTON_IO); // button press handled
        button_hold &= ~(1 << LEFT_BUTTON_IO); // also clear the hold flag if any...
        selected_menu_item++;
        selected_menu_item %= menu_size;
    } else if (button_press & (1 << RIGHT_BUTTON_IO)) {
        button_press &= ~(1 << RIGHT_BUTTON_IO); // button press handled
        button_hold &= ~(1 << RIGHT_BUTTON_IO); // also clear the hold flag if any...
        if (selected_menu_item)
            selected_menu_item--;
        else
            selected_menu_item = menu_size - 1;
    } else if (button_hold & (1 << LEFT_BUTTON_IO)) {
        button_hold &= ~(1 << LEFT_BUTTON_IO);  // button hold handled
        button_press &= ~(1 << LEFT_BUTTON_IO); // also clear the press flag if any...
        next_state = selected_menu_item + 1; // next state is a selected submenu item
        selected_menu_item = 0;
    } else if ((button_press & (1 << RIGHT_BUTTON_IO))
            || (button_hold & (1 << RIGHT_BUTTON_IO))) {
        // clear the press/hold flag
        button_hold &= ~(1 << RIGHT_BUTTON_IO);
        button_press &= ~(1 << RIGHT_BUTTON_IO);
    }
    return next_state;
}

state_t STATE_intervalometer_menu(void) {
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
    uint8_t menu_size = 5;
    state_t next_state = cur_state;
    PCICR &= ~(1 << PCIE1); // Disable PIN Change Interrupt 1 - This disables interrupts on pins PCINT14...8 see p73 of datasheet
    LCD_clear();
    // draw battery icon with actual battery state
    LCD_update_battery_icon(BATTERY_read_voltage());
    // menu title
    LCD_puts(intervalometer_menu_title_str,
    LCD_TYP_UP2);
    if (selected_menu_item < 4) {
        // delay property (icon + name + hh:mm:ss)
        LCD_putCustomChar(delay_icon, ICON_WIDTH,
                (selected_menu_item == 0) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_puts(delay_str,
                (selected_menu_item == 0) ? LCD_TYP_INV : LCD_TYP_NORM);
        hours = intervalometer_data.delay / 3600;
        minutes = (intervalometer_data.delay / 60) % 60;
        seconds = intervalometer_data.delay % 60;
        LCD_putc(hours / 10 + '0',
                (selected_menu_item == 0 && !(is_blinking && number_part == 2)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(hours % 10 + '0',
                (selected_menu_item == 0 && !(is_blinking && number_part == 2)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(':', (selected_menu_item == 0) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(minutes / 10 + '0',
                (selected_menu_item == 0 && !(is_blinking && number_part == 1)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(minutes % 10 + '0',
                (selected_menu_item == 0 && !(is_blinking && number_part == 1)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(':', (selected_menu_item == 0) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(seconds / 10 + '0',
                (selected_menu_item == 0 && !(is_blinking && number_part == 0)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(seconds % 10 + '0',
                (selected_menu_item == 0 && !(is_blinking && number_part == 0)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc('\n', LCD_TYP_NORM);
        // period property (icon + name + hh:mm:ss)
        LCD_putCustomChar(clock_icon, ICON_WIDTH,
                (selected_menu_item == 1) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_puts(interval_str,
                (selected_menu_item == 1) ? LCD_TYP_INV : LCD_TYP_NORM);
        hours = intervalometer_data.period / 3600;
        minutes = (intervalometer_data.period / 60) % 60;
        seconds = intervalometer_data.period % 60;
        LCD_putc(hours / 10 + '0',
                (selected_menu_item == 1 && !(is_blinking && number_part == 2)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(hours % 10 + '0',
                (selected_menu_item == 1 && !(is_blinking && number_part == 2)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(':', (selected_menu_item == 1) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(minutes / 10 + '0',
                (selected_menu_item == 1 && !(is_blinking && number_part == 1)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(minutes % 10 + '0',
                (selected_menu_item == 1 && !(is_blinking && number_part == 1)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(':', (selected_menu_item == 1) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(seconds / 10 + '0',
                (selected_menu_item == 1 && !(is_blinking && number_part == 0)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(seconds % 10 + '0',
                (selected_menu_item == 1 && !(is_blinking && number_part == 0)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc('\n', LCD_TYP_NORM);
        // number of images property (icon + name + [num][num][num])
        LCD_putCustomChar(image_icon, ICON_WIDTH,
                (selected_menu_item == 2) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_puts(shots_str,
                (selected_menu_item == 2) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(intervalometer_data.shots / 100 + '0',
                (selected_menu_item == 2 && !(is_blinking && number_part == 2)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc((intervalometer_data.shots / 10) % 10 + '0',
                (selected_menu_item == 2 && !(is_blinking && number_part == 1)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc(intervalometer_data.shots % 10 + '0',
                (selected_menu_item == 2 && !(is_blinking && number_part == 0)) ?
                        LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc('\n', LCD_TYP_NORM);
        // prefocus property (icon + name + [on\off])
        LCD_putCustomChar(focus_icon, ICON_WIDTH,
                (selected_menu_item == 3) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_puts(prefocus_str,
                (selected_menu_item == 3) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_puts((intervalometer_data.prefocus == 0) ? off_str : on_str,
                (selected_menu_item == 3) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_putc('\n', LCD_TYP_NORM);
    } else {
        // start (icon + name)
        LCD_putCustomChar(start_icon, ICON_WIDTH,
                (selected_menu_item == 4) ? LCD_TYP_INV : LCD_TYP_NORM);
        LCD_puts(start_str,
                (selected_menu_item == 4) ? LCD_TYP_INV : LCD_TYP_NORM);
    }
    // help bar on the bottom
    LCD_goto(0, 5);
    LCD_puts(is_prop_selected ? help_bar_3_str : help_bar_2_str,
    LCD_TYP_NORM);
    // draw scroll bar
    LCD_update_scroll_bar(selected_menu_item, menu_size);

    PCICR |= (1 << PCIE1); // Enable PIN Change Interrupt 1 - This enables interrupts on pins PCINT14...8 see p73 of datasheet
    // wait button press or hold
    while (!(button_press | button_hold)) {
        // periodically check the battery status
        BATTERY_monitor();
        // blinking...
        if (is_prop_selected && (blink_timer >= BLINK_PERIOD)) {
            blink_timer = 0; // reset timer
            is_blinking ^= 1; // toggle the value
            break;
        }
    }

    // turn on the backlight (if enabled) when some button is pressed/held
    BACKLIGHT_update(settings_data.backlight_intensity);

    if (button_press & (1 << LEFT_BUTTON_IO)) {
        button_press &= ~(1 << LEFT_BUTTON_IO); // button press handled
        button_hold &= ~(1 << LEFT_BUTTON_IO); // also clear the hold flag if any...
        // check is the property already selected
        if (is_prop_selected) { // yes, start blinking and increment selected value
            switch (selected_menu_item) {
            // Delay
            case 0:
                switch (number_part) {
                // Hours
                case 2:
                    if ((intervalometer_data.delay / 3600) < 9)
                        intervalometer_data.delay += 3600;
                    else
                        intervalometer_data.delay %= 3600;
                    break;
                    // Minutes
                case 1:
                    if (((intervalometer_data.delay % 3600) / 60) < 59)
                        intervalometer_data.delay += 60;
                    else
                        intervalometer_data.delay -= ((intervalometer_data.delay
                                % 3600) / 60) * 60;
                    break;
                    // Seconds
                case 0:
                    if (((intervalometer_data.delay % 3600) % 60) < 59)
                        intervalometer_data.delay++;
                    else
                        intervalometer_data.delay -= intervalometer_data.delay
                                % 60;
                    break;
                }
                break;
                // Period
            case 1:
                switch (number_part) {
                // Hours
                case 2:
                    if ((intervalometer_data.period / 3600) < 9)
                        intervalometer_data.period += 3600;
                    else
                        intervalometer_data.period %= 3600;
                    break;
                    // Minutes
                case 1:
                    if (((intervalometer_data.period % 3600) / 60) < 59)
                        intervalometer_data.period += 60;
                    else
                        intervalometer_data.period -=
                                ((intervalometer_data.period % 3600) / 60) * 60;
                    break;
                    // Seconds
                case 0:
                    if (((intervalometer_data.period % 3600) % 60) < 59)
                        intervalometer_data.period++;
                    else
                        intervalometer_data.period -= intervalometer_data.period
                                % 60;
                    break;
                }
                break;
                // Number of images/shots
            case 2:
                switch (number_part) {
                case 2:
                    if ((intervalometer_data.shots / 100) < 9)
                        intervalometer_data.shots += 100;
                    else
                        intervalometer_data.shots %= 100;
                    break;
                case 1:
                    if (((intervalometer_data.shots / 10) % 10) < 9)
                        intervalometer_data.shots += 10;
                    else
                        intervalometer_data.shots -= ((intervalometer_data.shots
                                / 10) % 10) * 10;
                    break;
                case 0:
                    if ((intervalometer_data.shots % 10 < 9))
                        intervalometer_data.shots++;
                    else
                        intervalometer_data.shots -= intervalometer_data.shots
                                % 10;
                    break;
                }
                break;
            }
        } else {    // no, select the next property from menu
            selected_menu_item++;
            selected_menu_item %= menu_size;
            number_part = 0; // reset
        }
    } else if (button_press & (1 << RIGHT_BUTTON_IO)) {
        button_press &= ~(1 << RIGHT_BUTTON_IO); // button press handled
        button_hold &= ~(1 << RIGHT_BUTTON_IO); // also clear the hold flag if any...
        // check is the property already selected
        if (is_prop_selected) { // yes, start blinking and decrement selected value
            switch (selected_menu_item) {
            // Delay
            case 0:
                switch (number_part) {
                // Hours
                case 2:
                    if (intervalometer_data.delay >= 3600)
                        intervalometer_data.delay -= 3600;
                    else
                        intervalometer_data.delay += 32400;
                    break;
                    // Minutes
                case 1:
                    if (((intervalometer_data.delay % 3600) / 60) > 0)
                        intervalometer_data.delay -= 60;
                    else
                        intervalometer_data.delay += 3540;
                    break;
                    // Seconds
                case 0:
                    if (((intervalometer_data.delay % 3600) % 60) > 0)
                        intervalometer_data.delay--;
                    else
                        intervalometer_data.delay += 59;
                    break;
                }
                break;
                // Period
            case 1:
                switch (number_part) {
                // Hours
                case 2:
                    if (intervalometer_data.period >= 3600)
                        intervalometer_data.period -= 3600;
                    else
                        intervalometer_data.period += 32400;
                    break;
                    // Minutes
                case 1:
                    if (((intervalometer_data.period % 3600) / 60) > 0)
                        intervalometer_data.period -= 60;
                    else
                        intervalometer_data.period += 3540;
                    break;
                    // Seconds
                case 0:
                    if (((intervalometer_data.period % 3600) % 60))
                        intervalometer_data.period--;
                    else
                        intervalometer_data.period += 59;
                    break;
                }
                break;
                // Number of images/shots
            case 2:
                switch (number_part) {
                case 2:
                    if (intervalometer_data.shots >= 100)
                        intervalometer_data.shots -= 100;
                    else
                        intervalometer_data.shots += 900;
                    break;
                case 1:
                    if (((intervalometer_data.shots / 10) % 10) > 0)
                        intervalometer_data.shots -= 10;
                    else
                        intervalometer_data.shots += 90;
                    break;
                case 0:
                    if ((intervalometer_data.shots % 10) > 0)
                        intervalometer_data.shots--;
                    else
                        intervalometer_data.shots += 9;
                    break;
                }
                break;
            }
        } else {    // no, select the previous property from menu
            if (selected_menu_item)
                selected_menu_item--;
            else
                selected_menu_item = menu_size - 1;
            number_part = 0; // reset
        }
    } else if (button_hold & (1 << LEFT_BUTTON_IO)) {
        button_hold &= ~(1 << LEFT_BUTTON_IO); // button hold handled
        button_press &= ~(1 << LEFT_BUTTON_IO); // also clear the press flag if any...
        if (selected_menu_item < 3) {
            is_prop_selected ^= 1;
            is_blinking = 0;
            if (is_prop_selected == 0) {
                number_part++;
                number_part %= 3;
            }
        } else if (selected_menu_item == 3) {
            intervalometer_data.prefocus ^= 1;  // toggle prefocus value
        } else if (selected_menu_item == 4 && (intervalometer_data.period > 0)
                && (intervalometer_data.shots > 0)) { // if start is selected go to new state
            next_state = INTERVALOMETER;
            intervalometer_pre_start = TRUE;
        }
    } else if (button_hold & (1 << RIGHT_BUTTON_IO)) {
        button_hold &= ~(1 << RIGHT_BUTTON_IO); // button hold handled
        button_press &= ~(1 << RIGHT_BUTTON_IO); // also clear the press flag if any...
        // check is some property selected
        if (is_prop_selected) { // yes, stop blinking -> deselect it
            is_prop_selected = 0;
            is_blinking = 0;
        } else {    // no, go back to main menu
            next_state = MAIN_MENU; // go back to main menu
            selected_menu_item = 0;
        }
    }
    return next_state;
}

state_t STATE_intervalometer(void) {
    state_t next_state = cur_state;
    if (!timeout_timer) {
        SHUTTER_PORT &= ~(1 << SHUTTER_IO);
        FOCUS_PORT &= ~(1 << FOCUS_IO);
    }

    if (intervalometer_pre_start) {
        intervalometer_pre_start = FALSE;
        // calculate remaining time
        remaining_time = intervalometer_data.shots * intervalometer_data.period;
        intervalometer_temp_data = intervalometer_data;
        // print initial data to LCD
        LCD_clear();
        // draw battery icon with actual battery state
        LCD_update_battery_icon(BATTERY_read_voltage());
        // menu title
        LCD_puts(intervalometer_menu_title_str, LCD_TYP_UP2);
        if (intervalometer_temp_data.delay) {
            // image icon + [num][num][num]
            LCD_goto(0, 1);
            LCD_putCustomChar(image_icon, 8, LCD_TYP_NORM);
            LCD_putc(' ', LCD_TYP_NORM);
            LCD_putc('0' + intervalometer_temp_data.shots / 100,
            LCD_TYP_NORM);
            LCD_putc('0' + (intervalometer_temp_data.shots / 10) % 10,
            LCD_TYP_NORM);
            LCD_putc('0' + intervalometer_temp_data.shots % 10,
            LCD_TYP_NORM);
        } else {
            // big image icon + [NUM][NUM][NUM]
            LCD_goto(20, 1);
            LCD_putcBig('0' + intervalometer_temp_data.shots / 100,
            LCD_TYP_NORM);
            LCD_goto(30, 1);
            LCD_putcBig('0' + (intervalometer_temp_data.shots / 10) % 10,
            LCD_TYP_NORM);
            LCD_goto(40, 1);
            LCD_putcBig('0' + intervalometer_temp_data.shots % 10,
            LCD_TYP_NORM);
            LCD_goto(0, 1);
            LCD_putCustomChar(image_icon_top, 19,
            LCD_TYP_NORM);
            LCD_goto(0, 2);
            LCD_putCustomChar(image_icon_bottom, 19,
            LCD_TYP_NORM);
            // delay icon + hh:mm:ss
            LCD_goto(0, 3);
            LCD_putCustomChar(delay_icon, ICON_WIDTH,
            LCD_TYP_NORM);
            LCD_putc(' ', LCD_TYP_NORM);
            if (intervalometer_temp_data.delay) {
                LCD_print_time(intervalometer_temp_data.delay, 0);
            } else {
                LCD_print_time(remaining_time, 0);
            }
        }
        // duration icon + hh:mm:ss
        LCD_goto(0, 4);
        LCD_putCustomChar(clock_icon, ICON_WIDTH, LCD_TYP_NORM);
        LCD_putc(' ', LCD_TYP_NORM);
        LCD_print_time(intervalometer_temp_data.period, 0);
        // abort string
        LCD_goto(55, 5);
        LCD_puts(abort_str, LCD_TYP_NORM);
    }

    if (intervalometer_temp_data.prefocus
            && ((intervalometer_temp_data.period - 1) == 0))
        FOCUS_PORT |= (1 << FOCUS_IO);

    if ((intervalometer_temp_data.shots > 0)
            && (intervalometer_temp_data.period == 0)) {
        // execute shooting
        SHUTTER_PORT |= (1 << SHUTTER_IO);
        timeout_timer = 100;
        if (intervalometer_temp_data.shots) {
            intervalometer_temp_data.shots--;
            intervalometer_temp_data.period = intervalometer_data.period; // reset period timer
            // update the number of remaining shots
            // big image icon + [NUM][NUM][NUM]
            LCD_clearLine(1);
            LCD_clearLine(2);
            LCD_goto(20, 1);
            LCD_putcBig('0' + intervalometer_temp_data.shots / 100,
            LCD_TYP_NORM);
            LCD_goto(30, 1);
            LCD_putcBig('0' + (intervalometer_temp_data.shots / 10) % 10,
            LCD_TYP_NORM);
            LCD_goto(40, 1);
            LCD_putcBig('0' + intervalometer_temp_data.shots % 10,
            LCD_TYP_NORM);
            LCD_goto(0, 1);
            LCD_putCustomChar(image_icon_top, 19,
            LCD_TYP_NORM);
            LCD_goto(0, 2);
            LCD_putCustomChar(image_icon_bottom, 19,
            LCD_TYP_NORM);
        }
    }

    if (clock >= ONE_SECOND) {
        clock = 0;
        if (intervalometer_temp_data.delay) {
            // decrement delay every second
            intervalometer_temp_data.delay--;
            // update LCD with new delay value (big delay icon + HH:MM:SS)
            LCD_clearLine(2);
            LCD_putCustomChar(delay_icon_top, 19, LCD_TYP_NORM);
            LCD_clearLine(3);
            LCD_putCustomChar(delay_icon_bottom, 19,
            LCD_TYP_NORM);
            LCD_goto(20, 2);
            LCD_print_time(intervalometer_temp_data.delay, 1);
            update_image_icon = 1;
        } else {
            if (update_image_icon) {
                update_image_icon = 0;
                // images icon + [NUM][NUM][NUM]
                LCD_clearLine(1);
                LCD_clearLine(2);
                LCD_goto(20, 1);
                LCD_putcBig('0' + intervalometer_temp_data.shots / 100,
                LCD_TYP_NORM);
                LCD_goto(30, 1);
                LCD_putcBig('0' + (intervalometer_temp_data.shots / 10) % 10,
                LCD_TYP_NORM);
                LCD_goto(40, 1);
                LCD_putcBig('0' + intervalometer_temp_data.shots % 10,
                LCD_TYP_NORM);
                LCD_goto(0, 1);
                LCD_putCustomChar(image_icon_top, 19,
                LCD_TYP_NORM);
                LCD_goto(0, 2);
                LCD_putCustomChar(image_icon_bottom, 19,
                LCD_TYP_NORM);
            }
            // decrement remaining time and period every second
            if (remaining_time)
                remaining_time--;
            if (intervalometer_temp_data.period)
                intervalometer_temp_data.period--;
            // update LCD with new values
            LCD_clearLine(3);
            LCD_putCustomChar(delay_icon, ICON_WIDTH,
            LCD_TYP_NORM);
            LCD_putc(' ', LCD_TYP_NORM);
            LCD_print_time(remaining_time, 0);
            LCD_clearLine(4);
            LCD_putCustomChar(clock_icon, ICON_WIDTH,
            LCD_TYP_NORM);
            LCD_putc(' ', LCD_TYP_NORM);
            LCD_print_time(intervalometer_temp_data.period, 0);
        }
    }

    // show "Done" message if intervalometer finished
    if (intervalometer_temp_data.shots == 0) {
        SHUTTER_PORT &= ~(1 << SHUTTER_IO); //release shutter
        FOCUS_PORT &= ~(1 << FOCUS_IO); //release focus
        // turn on the backlight (if enabled) when intervalometer finished
        BACKLIGHT_update(settings_data.backlight_intensity);
        // clear LCD and show "Done" message
        LCD_clearLine(1);
        LCD_clearLine(2);
        LCD_goto(33, 2);
        LCD_puts(done_str, LCD_TYP_NORM);
        LCD_clearLine(3);
        LCD_clearLine(4);
        LCD_clearLine(5);
        LCD_goto(59, 5);
        LCD_puts(back_str, LCD_TYP_NORM);
        // wait for button  press or hold
        while (!(button_press | button_hold))
            ;
    }

    // turn on the backlight (if enabled) when some button is pressed/held
    if (button_press || button_hold) {
        BACKLIGHT_update(settings_data.backlight_intensity);
    }

    if (button_hold & (1 << RIGHT_BUTTON_IO)) {
        button_hold &= ~(1 << RIGHT_BUTTON_IO); // button hold handled
        button_press &= ~(1 << RIGHT_BUTTON_IO); // also clear the press flag if any...
        next_state = MAIN_MENU; // go back to main menu
        selected_menu_item = 0;
        SHUTTER_PORT &= ~(1 << SHUTTER_IO); //release shutter if abort is pressed
        FOCUS_PORT &= ~(1 << FOCUS_IO); //release focus if abort is pressed
        PCICR |= (1 << PCIE1); // Enable PIN Change Interrupt 1 - This enables interrupts on pins PCINT14...8 see p73 of datasheet
    } else {
        button_press = 0x00;
        button_hold = 0x00;
    }
    return next_state;
}

state_t STATE_bulb_mode_menu(void) {
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
    uint8_t menu_size = 0;
    state_t next_state = cur_state;

    PCICR &= ~(1 << PCIE1); // Disable PIN Change Interrupt 1 - This disables interrupts on pins PCINT14...8 see p73 of datasheet
    LCD_clear();
    // draw battery icon with actual battery state
    LCD_update_battery_icon(BATTERY_read_voltage());
    // menu title
    LCD_puts(bulb_mode_menu_title_str,
    LCD_TYP_UP2);
    // delay property (icon + name + hh:mm:ss)
    LCD_putCustomChar(delay_icon, ICON_WIDTH,
            (selected_menu_item == 0) ?
            LCD_TYP_INV :
                                        LCD_TYP_NORM);
    LCD_puts(delay_str, (selected_menu_item == 0) ?
    LCD_TYP_INV :
                                                    LCD_TYP_NORM);
    hours = bulb_mode_data.delay / 3600;
    minutes = (bulb_mode_data.delay / 60) % 60;
    seconds = bulb_mode_data.delay % 60;
    LCD_putc(hours / 10 + '0',
            (selected_menu_item == 0 && !(is_blinking && number_part == 2)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(hours % 10 + '0',
            (selected_menu_item == 0 && !(is_blinking && number_part == 2)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(':', (selected_menu_item == 0) ?
    LCD_TYP_INV :
                                              LCD_TYP_NORM);
    LCD_putc(minutes / 10 + '0',
            (selected_menu_item == 0 && !(is_blinking && number_part == 1)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(minutes % 10 + '0',
            (selected_menu_item == 0 && !(is_blinking && number_part == 1)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(':', (selected_menu_item == 0) ?
    LCD_TYP_INV :
                                              LCD_TYP_NORM);
    LCD_putc(seconds / 10 + '0',
            (selected_menu_item == 0 && !(is_blinking && number_part == 0)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(seconds % 10 + '0',
            (selected_menu_item == 0 && !(is_blinking && number_part == 0)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc('\n', LCD_TYP_NORM);
    menu_size++;
    // duration property (icon + name + hh:mm:ss)
    LCD_putCustomChar(clock_icon, ICON_WIDTH,
            (selected_menu_item == 1) ?
            LCD_TYP_INV :
                                        LCD_TYP_NORM);
    LCD_puts(duration_str, (selected_menu_item == 1) ?
    LCD_TYP_INV :
                                                       LCD_TYP_NORM);
    hours = bulb_mode_data.duration / 3600;
    minutes = (bulb_mode_data.duration / 60) % 60;
    seconds = bulb_mode_data.duration % 60;
    LCD_putc(hours / 10 + '0',
            (selected_menu_item == 1 && !(is_blinking && number_part == 2)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(hours % 10 + '0',
            (selected_menu_item == 1 && !(is_blinking && number_part == 2)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(':', (selected_menu_item == 1) ?
    LCD_TYP_INV :
                                              LCD_TYP_NORM);
    LCD_putc(minutes / 10 + '0',
            (selected_menu_item == 1 && !(is_blinking && number_part == 1)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(minutes % 10 + '0',
            (selected_menu_item == 1 && !(is_blinking && number_part == 1)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(':', (selected_menu_item == 1) ?
    LCD_TYP_INV :
                                              LCD_TYP_NORM);
    LCD_putc(seconds / 10 + '0',
            (selected_menu_item == 1 && !(is_blinking && number_part == 0)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc(seconds % 10 + '0',
            (selected_menu_item == 1 && !(is_blinking && number_part == 0)) ?
                    LCD_TYP_INV : LCD_TYP_NORM);
    LCD_putc('\n', LCD_TYP_NORM);
    menu_size++;
    // start (icon + name)
    LCD_putCustomChar(start_icon, ICON_WIDTH,
            (selected_menu_item == 2) ?
            LCD_TYP_INV :
                                        LCD_TYP_NORM);
    LCD_puts(start_str, (selected_menu_item == 2) ?
    LCD_TYP_INV :
                                                    LCD_TYP_NORM);
    menu_size++;
    // help bar on the bottom
    LCD_goto(0, 5);
    LCD_puts(is_prop_selected ? help_bar_3_str : help_bar_2_str,
    LCD_TYP_NORM);
    // draw scroll bar
    LCD_update_scroll_bar(selected_menu_item, menu_size);

    PCICR |= (1 << PCIE1); // Enable PIN Change Interrupt 1 - This enables interrupts on pins PCINT14...8 see p73 of datasheet
    // wait button press or hold
    while (!(button_press | button_hold)) {
        // periodically check the battery status
        BATTERY_monitor();
        // blinking...
        if (is_prop_selected && (blink_timer >= BLINK_PERIOD)) {
            blink_timer = 0; // reset timer
            is_blinking ^= 1; // toggle the value
            break;
        }
    }

    // turn on the backlight (if enabled) when some button is pressed/held
    BACKLIGHT_update(settings_data.backlight_intensity);

    if (button_press & (1 << LEFT_BUTTON_IO)) {
        button_press &= ~(1 << LEFT_BUTTON_IO); // button press handled
        button_hold &= ~(1 << LEFT_BUTTON_IO); // also clear the hold flag if any...
        // check is the property already selected
        if (is_prop_selected) { // yes, start blinking and increment selected value
            switch (selected_menu_item) {
            // Delay
            case 0:
                switch (number_part) {
                // Hours
                case 2:
                    if ((bulb_mode_data.delay / 3600) < 9)
                        bulb_mode_data.delay += 3600;
                    else
                        bulb_mode_data.delay %= 3600;
                    break;
                    // Minutes
                case 1:
                    if (((bulb_mode_data.delay % 3600) / 60) < 59)
                        bulb_mode_data.delay += 60;
                    else
                        bulb_mode_data.delay -= ((bulb_mode_data.delay % 3600)
                                / 60) * 60;
                    break;
                    // Seconds
                case 0:
                    if (((bulb_mode_data.delay % 3600) % 60) < 59)
                        bulb_mode_data.delay++;
                    else
                        bulb_mode_data.delay -= bulb_mode_data.delay % 60;
                    break;
                }
                break;
                // Duration
            case 1:
                switch (number_part) {
                // Hours
                case 2:
                    if ((bulb_mode_data.duration / 3600) < 9)
                        bulb_mode_data.duration += 3600;
                    else
                        bulb_mode_data.duration %= 3600;
                    break;
                    // Minutes
                case 1:
                    if (((bulb_mode_data.duration % 3600) / 60) < 59)
                        bulb_mode_data.duration += 60;
                    else
                        bulb_mode_data.duration -= ((bulb_mode_data.duration
                                % 3600) / 60) * 60;
                    break;
                    // Seconds
                case 0:
                    if (((bulb_mode_data.duration % 3600) % 60) < 59)
                        bulb_mode_data.duration++;
                    else
                        bulb_mode_data.duration -= bulb_mode_data.duration % 60;
                    break;
                }
                break;
            }
        } else {    // no, select the next property from menu
            selected_menu_item++;
            selected_menu_item %= menu_size;
            number_part = 0; // reset
        }
    } else if (button_press & (1 << RIGHT_BUTTON_IO)) {
        button_press &= ~(1 << RIGHT_BUTTON_IO); // button press handled
        button_hold &= ~(1 << RIGHT_BUTTON_IO); // also clear the hold flag if any...
        // check is the property already selected
        if (is_prop_selected) { // yes, start blinking and decrement selected value
            switch (selected_menu_item) {
            // Delay
            case 0:
                switch (number_part) {
                // Hours
                case 2:
                    if (bulb_mode_data.delay >= 3600)
                        bulb_mode_data.delay -= 3600;
                    else
                        bulb_mode_data.delay += 32400;
                    break;
                    // Minutes
                case 1:
                    if (((bulb_mode_data.delay % 3600) / 60) > 0)
                        bulb_mode_data.delay -= 60;
                    else
                        bulb_mode_data.delay += 3540;
                    break;
                    // Seconds
                case 0:
                    if (((bulb_mode_data.delay % 3600) % 60) > 0)
                        bulb_mode_data.delay--;
                    else
                        bulb_mode_data.delay += 59;
                    break;
                }
                break;
                // Duration
            case 1:
                switch (number_part) {
                // Hours
                case 2:
                    if (bulb_mode_data.duration >= 3600)
                        bulb_mode_data.duration -= 3600;
                    else
                        bulb_mode_data.duration += 32400;
                    break;
                    // Minutes
                case 1:
                    if (((bulb_mode_data.duration % 3600) / 60) > 0)
                        bulb_mode_data.duration -= 60;
                    else
                        bulb_mode_data.duration += 3540;
                    break;
                    // Seconds
                case 0:
                    if (((bulb_mode_data.duration % 3600) % 60) > 0)
                        bulb_mode_data.duration--;
                    else
                        bulb_mode_data.duration += 59;
                    break;
                }
                break;
            }
        } else {    // no, select the previous property from menu
            if (selected_menu_item)
                selected_menu_item--;
            else
                selected_menu_item = menu_size - 1;
            number_part = 0; // reset
        }
    } else if (button_hold & (1 << LEFT_BUTTON_IO)) {
        button_hold &= ~(1 << LEFT_BUTTON_IO); // button hold handled
        button_press &= ~(1 << LEFT_BUTTON_IO); // also clear the press flag if any...
        if (selected_menu_item < 2) {
            is_prop_selected ^= 1;   //
            is_blinking = 0;
            if (is_prop_selected == 0) {
                number_part++;
                number_part %= 3;
            }
        } else if (selected_menu_item == 2) { // if start is selected goto new state
            if (bulb_mode_data.duration > 0) {
                next_state = BULB_MODE;
                bulb_mode_pre_start = TRUE;
            }
        }
    } else if (button_hold & (1 << RIGHT_BUTTON_IO)) {
        button_hold &= ~(1 << RIGHT_BUTTON_IO); // button hold handled
        button_press &= ~(1 << RIGHT_BUTTON_IO); // also clear the press flag if any...
        if (is_prop_selected) {
            is_prop_selected = 0;
            is_blinking = 0;
        } else {
            next_state = MAIN_MENU; // go back to main menu
            selected_menu_item = 1;
        }
    }
    return next_state;
}

state_t STATE_bulb_mode(void) {
    state_t next_state = cur_state;
    if (bulb_mode_pre_start) {
        bulb_mode_pre_start = FALSE;
        bulb_mode_temp_data = bulb_mode_data;
        LCD_clear();
        // draw battery icon with actual battery state
        LCD_update_battery_icon(BATTERY_read_voltage());
        // menu title
        LCD_puts(bulb_mode_menu_title_str,
        LCD_TYP_UP2);
        // print initial data on LCD
        LCD_goto(0, 1);
        if (bulb_mode_temp_data.delay > 0) {
            LCD_putCustomChar(delay_icon_top, 19, LCD_TYP_NORM);
            LCD_goto(0, 2);
            LCD_putCustomChar(delay_icon_bottom, 19,
            LCD_TYP_NORM);
            LCD_goto(20, 1);
            LCD_print_time(bulb_mode_temp_data.delay, 1);
            LCD_goto(0, 3);
            LCD_putCustomChar(bulb_icon, ICON_WIDTH,
            LCD_TYP_NORM);
            LCD_putc(' ', LCD_TYP_NORM);
            LCD_print_time(bulb_mode_temp_data.duration, 0);
        } else {
            LCD_putCustomChar(delay_icon, ICON_WIDTH,
            LCD_TYP_NORM);
            LCD_putc(' ', LCD_TYP_NORM);
            LCD_print_time(bulb_mode_temp_data.delay, 0);
            LCD_goto(0, 2);
            LCD_putCustomChar(bulb_icon_top, 19, LCD_TYP_NORM);
            LCD_goto(0, 3);
            LCD_putCustomChar(bulb_icon_bottom, 19,
            LCD_TYP_NORM);
            LCD_goto(20, 2);
            LCD_print_time(bulb_mode_temp_data.duration, 1);
        }
        LCD_goto(55, 5);
        LCD_puts(abort_str, LCD_TYP_NORM);
    }

    if (clock >= ONE_SECOND) {
        clock = 0;
        // start delay timer
        if (bulb_mode_temp_data.delay) {
            bulb_mode_temp_data.delay--;
            LCD_clearLine(1);
            LCD_putCustomChar(delay_icon_top, 19, LCD_TYP_NORM);
            LCD_clearLine(2);
            LCD_putCustomChar(delay_icon_bottom, 19,
            LCD_TYP_NORM);
            LCD_goto(20, 1);
            LCD_print_time(bulb_mode_temp_data.delay, 1);
            LCD_clearLine(3);
            LCD_putCustomChar(bulb_icon, ICON_WIDTH,
            LCD_TYP_NORM);
            LCD_putc(' ', LCD_TYP_NORM);
            LCD_print_time(bulb_mode_temp_data.duration, 0);
        } else {
            // hold shutter
            SHUTTER_PORT |= (1 << SHUTTER_IO);
            // decrement duration
            if (bulb_mode_temp_data.duration) {
                bulb_mode_temp_data.duration--;
                LCD_clearLine(1);
                LCD_putCustomChar(delay_icon, ICON_WIDTH,
                LCD_TYP_NORM);
                LCD_putc(' ', LCD_TYP_NORM);
                LCD_print_time(bulb_mode_temp_data.delay, 0);
                LCD_clearLine(2);
                LCD_putCustomChar(bulb_icon_top, 19, LCD_TYP_NORM);
                LCD_clearLine(3);
                LCD_putCustomChar(bulb_icon_bottom, 19,
                LCD_TYP_NORM);
                LCD_goto(20, 2);
                LCD_print_time(bulb_mode_temp_data.duration, 1);
            } else {
                SHUTTER_PORT &= ~(1 << SHUTTER_IO); // release shutter
                // turn on the backlight (if enabled) when bulb finished
                BACKLIGHT_update(settings_data.backlight_intensity);
                // clear LCD and show "Done" message
                LCD_clearLine(1);
                LCD_clearLine(2);
                LCD_goto(33, 2);
                LCD_puts(done_str, LCD_TYP_NORM);
                LCD_clearLine(3);
                LCD_clearLine(4);
                LCD_clearLine(5);
                LCD_goto(59, 5);
                LCD_puts(back_str, LCD_TYP_NORM);
                // wait for button  press or hold
                while (!(button_press | button_hold))
                    ;
            }
        }
    }

    // turn on the backlight (if enabled) when some button is pressed/held
    if (button_press || button_hold) {
        BACKLIGHT_update(settings_data.backlight_intensity);
    }

    if (button_hold & (1 << RIGHT_BUTTON_IO)) {
        button_hold &= ~(1 << RIGHT_BUTTON_IO); // button hold handled
        button_press &= ~(1 << RIGHT_BUTTON_IO); // also clear the press flag if any...
        next_state = MAIN_MENU; // go back to main menu
        selected_menu_item = 1;
        SHUTTER_PORT &= ~(1 << SHUTTER_IO); // release shutter if abort is pressed
        PCICR |= (1 << PCIE1); // Enable PIN Change Interrupt 1 - This enables interrupts on pins PCINT14...8 see p73 of datasheet
    } else {
        button_hold = 0x00;
        button_press = 0x00;
    }
    return next_state;
}

state_t STATE_settings_menu(void) {
    uint8_t menu_size;
    state_t next_state = cur_state;

    PCICR &= ~(1 << PCIE1); // Disable PIN Change Interrupt 1 - This disables interrupts on pins PCINT14...8 see p73 of datasheet
    LCD_clear();
    // draw battery icon with actual battery state
    LCD_update_battery_icon(BATTERY_read_voltage());
    menu_size = 2;
    // menu title
    LCD_puts(settings_menu_title_str,
    LCD_TYP_UP2);
    // backlight property (icon + name + value)
    LCD_goto(0, 1);
    LCD_putCustomChar(backlight_icon, 8,
            (selected_menu_item == 0) ?
            LCD_TYP_INV :
                                        LCD_TYP_NORM);
    LCD_puts(backlight_str, (selected_menu_item == 0) ?
    LCD_TYP_INV :
                                                        LCD_TYP_NORM);
    if (settings_data.backlight_intensity == 0) {
        LCD_puts(off_str,
                (selected_menu_item == 0 && (is_blinking == 0)) ?
                LCD_TYP_INV :
                                                                  LCD_TYP_NORM);
    } else {
        LCD_putc(' ',
                (selected_menu_item == 0 && (is_blinking == 0)) ?
                LCD_TYP_INV :
                                                                  LCD_TYP_NORM);
        LCD_putc(' ',
                (selected_menu_item == 0 && (is_blinking == 0)) ?
                LCD_TYP_INV :
                                                                  LCD_TYP_NORM);
        LCD_writeUnsignedValue((settings_data.backlight_intensity + 1) / 64,
                (selected_menu_item == 0 && (is_blinking == 0)) ?
                LCD_TYP_INV :
                                                                  LCD_TYP_NORM);
    }
    // battery cutoff property (icon + name + value [mV])
    LCD_goto(0, 2);
    LCD_putCustomChar(battery_icon, 8,
            (selected_menu_item == 1) ?
            LCD_TYP_INV :
                                        LCD_TYP_NORM);
    LCD_puts(auto_cutoff_str, (selected_menu_item == 1) ?
    LCD_TYP_INV :
                                                          LCD_TYP_NORM);
    LCD_writeUnsignedValue(settings_data.battery_auto_cutoff / 1000,
            (selected_menu_item == 1 && (is_blinking == 0)) ?
            LCD_TYP_INV :
                                                              LCD_TYP_NORM);
    LCD_putc('V',
            (selected_menu_item == 1 && (is_blinking == 0)) ?
            LCD_TYP_INV :
                                                              LCD_TYP_NORM);
    LCD_writeUnsignedValue((settings_data.battery_auto_cutoff % 1000) / 100,
            (selected_menu_item == 1 && (is_blinking == 0)) ?
            LCD_TYP_INV :
                                                              LCD_TYP_NORM);
    // help bar on the bottom
    LCD_goto(0, 5);
    LCD_puts(is_prop_selected ? help_bar_3_str : help_bar_2_str,
    LCD_TYP_NORM);
    // draw scroll bar
    LCD_update_scroll_bar(selected_menu_item, menu_size);

    PCICR |= (1 << PCIE1); // Enable PIN Change Interrupt 1 - This enables interrupts on pins PCINT14...8 see p73 of datasheet

    // wait button press or hold
    while (!(button_press | button_hold)) {
        // periodically check the battery status
        BATTERY_monitor();
        // blinking...
        if (is_prop_selected && (blink_timer >= BLINK_PERIOD)) {
            blink_timer = 0; // reset timer
            is_blinking ^= 1; // toggle the value
            break;
        }
    }

    // turn on the backlight (if enabled) when some button is pressed/held
    BACKLIGHT_update(settings_data.backlight_intensity);

    if (button_press & (1 << LEFT_BUTTON_IO)) {
        button_press &= ~(1 << LEFT_BUTTON_IO); // button press handled
        button_hold &= ~(1 << LEFT_BUTTON_IO); // also clear the hold flag if any...
        if (is_prop_selected) {
            switch (selected_menu_item) {
            // backlight
            case 0:
                if (settings_data.backlight_intensity == 255) {
                    settings_data.backlight_intensity = 0;
                } else {
                    settings_data.backlight_intensity +=
                            (settings_data.backlight_intensity == 0) ? 63 : 64;
                }
                break;
            case 1:
                // auto cutoff
                if (settings_data.battery_auto_cutoff == 3600) {
                    settings_data.battery_auto_cutoff = 3200;
                } else {
                    settings_data.battery_auto_cutoff += 100;
                }
                break;
            }
        } else {
            selected_menu_item++;
            selected_menu_item %= menu_size;
        }
    } else if (button_press & (1 << RIGHT_BUTTON_IO)) {
        button_press &= ~(1 << RIGHT_BUTTON_IO); // button press handled
        button_hold &= ~(1 << RIGHT_BUTTON_IO); // also clear the hold flag if any...
        if (is_prop_selected) {
            switch (selected_menu_item) {
            // backlight
            case 0:
                if (settings_data.backlight_intensity == 0) {
                    settings_data.backlight_intensity = 255;
                } else {
                    settings_data.backlight_intensity -=
                            (settings_data.backlight_intensity == 63) ? 63 : 64;
                }
                break;
                // auto cutoff
            case 1:
                if (settings_data.battery_auto_cutoff == 3200) {
                    settings_data.battery_auto_cutoff = 3600;
                } else {
                    settings_data.battery_auto_cutoff -= 100;
                }
                break;
            }
        } else {
            if (selected_menu_item)
                selected_menu_item--;
            else
                selected_menu_item = menu_size - 1;
        }
    } else if (button_hold & (1 << LEFT_BUTTON_IO)) {
        button_hold &= ~(1 << LEFT_BUTTON_IO); // button hold handled
        button_press &= ~(1 << LEFT_BUTTON_IO); // also clear the press flag if any...
        is_prop_selected ^= 1;   //
        is_blinking = 0;
    } else if (button_hold & (1 << RIGHT_BUTTON_IO)) {
        button_hold &= ~(1 << RIGHT_BUTTON_IO); // button hold handled
        button_press &= ~(1 << RIGHT_BUTTON_IO); // also clear the press flag if any...
        // check is some property selected
        if (is_prop_selected) { // yes, stop blinking -> deselect it
            is_prop_selected = 0;
            is_blinking = 0;
        } else {    // no, go back to main menu
            // backup settings, save them to eeprom
            eeprom_write_block(&settings_data, &settings_data_backup,
                    sizeof(settings_t));
            next_state = MAIN_MENU; // go back to main menu
            selected_menu_item = 3;
        }
    }
    return next_state;
}

/*
 * Turns off the device
 */
state_t STATE_turn_off(void) {
    state_t next_state = cur_state;
    LCD_clear();
    BACKLIGHT_update(settings_data.backlight_intensity);
    LCD_goto(0, 2);
    LCD_puts(cleanup_message_str, LCD_TYP_NORM); // print message
    TIMER_delay_ms(500);  // wait some time
    AUTO_CUTOFF_PORT &= ~(1 << AUTO_CUTOFF_IO); // and then turn off the device
    return next_state;
}

void TIMER_init(void) {
    TCCR1B |= (1 << WGM12); // CTC mode of Timer1
    OCR1A = 124;           // OCR1A Timer1's TOP value for 1mS @8Mhz
    TCCR1B |= (1 << CS11) | (1 << CS10); // start Timer1, clkI/O/64 (From prescaler) 125kHz
    TIMSK1 |= (1 << OCIE1A); // Timer/Counter1 Output Compare Match A Interrupt Enable
}

void TIMER_delay_ms(uint16_t _mS) {
    delay_timer = 0;
    while (delay_timer < _mS)
        ;
    return;
}

void BATTERY_init_adc(void) {
    DIDR0 |= (1 << ADC0D);                  // pin 0 disabled
    ADMUX |= (1 << REFS1) | (1 << REFS0); // Internal 1.1V Voltage Reference with external capacitor at AREF pin
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // ADC Prescaler Select Bits: 125kHz, prescaler: 64
    ADCSRA |= (1 << ADEN);                  // ADC Enable
}

uint16_t BATTERY_read_voltage(void) {
    uint16_t read_value = 0;
    ADMUX &= 0xF8;           // clear existing bottom 3 bits. 000 -> ADC0
    ADCSRA |= (1 << ADSC);   // start conversion
// wait for completion
    while (ADCSRA & (1 << ADSC))
        ;
    read_value = (ADCL | (ADCH << 8));
// voltage divider R1 = 10k, R2 = 3k3, ADC: 1v1/1024=1.0742mV
    return floor((13300 * 1.0742 * read_value) / 3300);
}

void BATTERY_monitor(void) {
    uint16_t battery_voltage = 0;

    if (battery_check_timer >= BATTERY_MONITOR_PERIOD) {
        battery_check_timer = 0;    // reset timer
        battery_voltage = BATTERY_read_voltage(); // read battery status
        LCD_update_battery_icon(battery_voltage);   // draw battery icon
        if (battery_voltage <= settings_data.battery_auto_cutoff) {
            LCD_show_low_battery_alert();
//            turn_off();   // TODO uncomment me
        }
    }
}

void BACKLIGHT_init_pwm(void) {
    TCCR2A |= (1 << COM2B1); // Clear OC2B on Compare Match when up-counting. Set OC2B on Compare Match when down-counting
    TCCR2A |= (1 << WGM20); // PWM, Phase Correct mode of Timer2, TOP=255
    OCR2B = 0;              // backlight intensity 0-255
    TCCR2B |= (1 << CS21); //Start Timer2 from prescaler clkT2S/8 (10MHz/8/(2*256)=2,441KHz)
}

void BACKLIGHT_update(uint8_t brightness) {
// start timer
    backlight_timer = BACKLIGHT_DURATION;
//turn backlight on if brightness != 0
    OCR2B = brightness;
}

void GPIO_init(void) {
    PCMSK1 |= (1 << PCINT10); // Set the mask on Pin change interrupt 1 so that only PCINT10 (PC2) triggers /the interrupt. see p85 of datasheet
    PCMSK1 |= (1 << PCINT11); // Set the mask on Pin change interrupt 1 so that only PCINT08 (PC3) triggers /the interrupt. see p85 of datasheet

    AUTO_CUTOFF_PORT &= ~(1 << AUTO_CUTOFF_IO); // pull down AUTO_CUTOFF_IO
    AUTO_CUTOFF_DDR |= (1 << AUTO_CUTOFF_IO); // configure AUTO_CUTOFF_IO as output

    SHUTTER_PORT &= ~(1 << SHUTTER_IO); // pull down SHUTTER_IO
    SHUTTER_DDR |= (1 << SHUTTER_IO);   // configure SHUTTER_IO as output

    FOCUS_PORT &= ~(1 << FOCUS_IO); // pull down FOCUS_IO
    FOCUS_DDR |= (1 << FOCUS_IO);   // configure FOCUS_IO as output

    LEFT_BUTTON_PORT |= (1 << LEFT_BUTTON_IO); // pull up LEFT_BUTTON_IO
    LEFT_BUTTON_DDR &= ~(1 << LEFT_BUTTON_IO); // configure LEFT_BUTTON_IO as input

    RIGHT_BUTTON_PORT |= (1 << RIGHT_BUTTON_IO); // pull up the LEFT_BUTTON_IO
    RIGHT_BUTTON_DDR &= ~(1 << RIGHT_BUTTON_IO); // configure LEFT_BUTTON_IO as input

    CHG_PORT &= ~(1 << CHG_IO); // pull down CHG_IO
    CHG_DDR &= ~(1 << CHG_IO); // configure CHG_IO as input

    USB_PWR_PORT &= ~(1 << USB_PWR_IO); // pull down CHG_IO
    USB_PWR_DDR &= ~(1 << USB_PWR_IO); // configure CHG_IO as input

    BACKLIGHT_DDR |= (1 << BACKLIGHT_IO); // configure BACKLIGHT_IO as output
}

void LCD_print_time(uint16_t _value, uint8_t size) {
    uint8_t i;
    uint8_t time[3];    // [0] -> hours, [1] -> minutes, [2] -> seconds
    time[0] = _value / 3600;
    time[1] = (_value / 60) % 60;
    time[2] = _value % 60;

    for (i = 0; i < 3; i++) {
        if (size == 0) {
            LCD_putc(time[i] / 10 + '0', LCD_TYP_NORM);
            LCD_putc(time[i] % 10 + '0', LCD_TYP_NORM);
            (i < 2) ? LCD_putc(':', LCD_TYP_NORM) : 0;
        } else {
            LCD_putcBig(time[i] / 10 + '0', LCD_TYP_NORM);
            LCD_putcBig(time[i] % 10 + '0', LCD_TYP_NORM);
            (i < 2) ? LCD_putcBig(':', LCD_TYP_NORM) : 0;
        }
    }
}

void LCD_update_scroll_bar(uint8_t selected_item, uint8_t total_items) {
    uint8_t scroll_bar_size = 0;
    uint8_t scroll_bar_position = 0;
    uint8_t i, j, pos = 0;
    uint8_t bank = 0x00;

    scroll_bar_size = 32 / total_items;
    scroll_bar_position = scroll_bar_size * selected_item;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            if (pos >= scroll_bar_position
                    && pos < scroll_bar_position + scroll_bar_size) {
                bank |= (1 << j);
            } else {
                bank &= ~(1 << j);
            }
            pos++;
        }
        LCD_goto(82, i + 1);
        LCD_writeData(bank);
        LCD_goto(83, i + 1);
        LCD_writeData(bank);
    }
}

void LCD_update_battery_icon(uint16_t voltage) {
    int16_t result = 0;
    uint8_t resolution, i;
    LCD_goto(71, 0);    // go to the top right corner of the LCD
    resolution = (MAX_BATTERY_VOLTAGE - 100 - settings_data.battery_auto_cutoff)
            / 10;
// draw the skeleton of the battery
    LCD_putCustomChar(battery_tray_icon, 13, LCD_TYP_NORM);
    result = (voltage - settings_data.battery_auto_cutoff) / resolution;
    if (result > 10) {
        result = 10;
    }
// draw the actual status of the battery
    for (i = 0; i < result; i++) {
        LCD_goto(82 - i, 0);
        LCD_writeData(0x1f);
    }
    LCD_goto(0, 0);
}

void LCD_show_low_battery_alert(void) {
    uint8_t i;
    LCD_clear();
    for (i = 0; i < 3; i++) {
        LCD_update_battery_icon(BATTERY_read_voltage());
        LCD_goto(20, 2);
        LCD_puts(low_battery_str, LCD_TYP_NORM);
        TIMER_delay_ms(500);
        LCD_clear();
        TIMER_delay_ms(500);
    }
}
