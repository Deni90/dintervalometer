#include "pcd8544.h"

static volatile uint8_t cursor_row = 0; /* 0-5 */
static volatile uint8_t cursor_col = 0; /* 0-83 */
static volatile uint8_t lcd_wasnl = 0;

//***** Small fonts (5x7) **********

// Font-code below borrowed and modified from: http://www.instructables.com/id/Connecting-Nokia-3310-LCD-to-USB-using-AVR/step4/Programming-the-ATmega/
// This is the font we use with the function LCD_drawChar.
// Each character has 5 pixels avaliable in width, but it doesn't have to use all 5.
//
// If a character uses less than 5 pixels in width, fill the unused pixeles with 0b10000000 (0x80).
// This is the value for "skip", and simply draws nothing (Not even a blank space,
// if you want a blank space, use 0b00000000).
//
// A blank space is automatically added in between each character, so don't include any character spacing
// in the font.

static const __flash uint8_t font[] = {
// -1-            -2-            -3-            -4-            -5-
        0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b10000000, //SPACE
        0b01011100, 0b10000000, 0b10000000, 0b10000000, 0b10000000, //!
        0b00001100, 0b00000000, 0b00001100, 0b10000000, 0b10000000, //"
        0b00101000, 0b01111100, 0b00101000, 0b01111100, 0b00101000, //#
        0b01010000, 0b01011000, 0b11101100, 0b00101000, 0b10000000, //$
        0b00000100, 0b01100000, 0b00010000, 0b00001100, 0b01000000, //%
        0b00101000, 0b01010100, 0b01010100, 0b00100000, 0b01010000, //&
        0b00001100, 0b10000000, 0b10000000, 0b10000000, 0b10000000, //'
        0b00111000, 0b01000100, 0b10000000, 0b10000000, 0b10000000, //(
        0b01000100, 0b00111000, 0b10000000, 0b10000000, 0b10000000, //)
        0b00010100, 0b00001000, 0b00010100, 0b10000000, 0b10000000, // *
        0b00010000, 0b00111000, 0b00010000, 0b10000000, 0b10000000, //+
        0b11000000, 0b01000000, 0b10000000, 0b10000000, 0b10000000, //,
        0b00010000, 0b00010000, 0b00010000, 0b10000000, 0b10000000, //-
        0b01000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, //.
        0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, ///
        0b00111000, 0b01000100, 0b01000100, 0b00111000, 0b10000000, //0
        0b00000100, 0b01111100, 0b10000000, 0b10000000, 0b10000000, //1
        0b01100100, 0b01010100, 0b01010100, 0b01001000, 0b10000000, //2
        0b01000100, 0b01010100, 0b01010100, 0b00101000, 0b10000000, //3
        0b00110000, 0b00101000, 0b01111100, 0b00100000, 0b10000000, //4
        0b01011100, 0b01010100, 0b01010100, 0b00100100, 0b10000000, //5
        0b00111000, 0b01010100, 0b01010100, 0b00100000, 0b10000000, //6
        0b00000100, 0b01100100, 0b00010100, 0b00001100, 0b10000000, //7
        0b00101000, 0b01010100, 0b01010100, 0b00101000, 0b10000000, //8
        0b00001000, 0b01010100, 0b01010100, 0b00111000, 0b10000000, //9
        0b00000000, 0b00101000, 0b00000000, 0b10000000, 0b10000000, //:
        0b01101000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, //;
        0b00010000, 0b00101000, 0b01000100, 0b10000000, 0b10000000, //<
        0b00101000, 0b00101000, 0b00101000, 0b10000000, 0b10000000, //=
        0b01000100, 0b00101000, 0b00010000, 0b10000000, 0b10000000, //>
        0b00000100, 0b01010100, 0b00010100, 0b00001000, 0b10000000, //?
        0b00111000, 0b01000100, 0b01110100, 0b01010100, 0b00111000, //@
        0b01111000, 0b00100100, 0b00100100, 0b01111000, 0b10000000, //A
        0b01111100, 0b01010100, 0b01010100, 0b00101000, 0b10000000, //B
        0b00111000, 0b01000100, 0b01000100, 0b10000000, 0b10000000, //C
        0b01111100, 0b01000100, 0b01000100, 0b00111000, 0b10000000, //D
        0b01111100, 0b01010100, 0b01010100, 0b01000100, 0b10000000, //E
        0b01111100, 0b00010100, 0b00010100, 0b00000100, 0b10000000, //F
        0b00111000, 0b01000100, 0b01010100, 0b01110100, 0b10000000, //G
        0b01111100, 0b00010000, 0b00010000, 0b01111100, 0b10000000, //H
        0b01000100, 0b01111100, 0b01000100, 0b10000000, 0b10000000, //I
        0b00100000, 0b01000000, 0b01000100, 0b00111100, 0b10000000, //J
        0b01111100, 0b00010000, 0b00101000, 0b01000100, 0b10000000, //K
        0b01111100, 0b01000000, 0b01000000, 0b10000000, 0b10000000, //L
        0b01111100, 0b00001000, 0b00010000, 0b00001000, 0b01111100, //M
        0b01111100, 0b00001000, 0b00010000, 0b01111100, 0b10000000, //N
        0b00111000, 0b01000100, 0b01000100, 0b00111000, 0b10000000, //O
        0b01111100, 0b00100100, 0b00100100, 0b00011000, 0b10000000, //P
        0b00111000, 0b01000100, 0b01000100, 0b10111000, 0b10000000, //Q
        0b01111100, 0b00100100, 0b00100100, 0b01011000, 0b10000000, //R
        0b01001000, 0b01010100, 0b01010100, 0b00100100, 0b10000000, //S
        0b00000100, 0b01111100, 0b00000100, 0b10000000, 0b10000000, //T
        0b00111100, 0b01000000, 0b01000000, 0b00111100, 0b10000000, //U
        0b00111100, 0b01000000, 0b00110000, 0b00001100, 0b10000000, //V
        0b00111100, 0b01000000, 0b00111000, 0b01000000, 0b00111100, //W
        0b01101100, 0b00010000, 0b00010000, 0b01101100, 0b10000000, //X
        0b00001100, 0b01010000, 0b01010000, 0b00111100, 0b10000000, //Y
        0b01100100, 0b01010100, 0b01001100, 0b10000000, 0b10000000, //Z
        0b01111100, 0b01000100, 0b10000000, 0b10000000, 0b10000000, // [
        0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, /* \ */
        0b01000100, 0b01111100, 0b10000000, 0b10000000, 0b10000000, // ]
        0b00001000, 0b00000100, 0b00001000, 0b10000000, 0b10000000, // ^
        0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b10000000, // _
        0b00000100, 0b00001000, 0b10000000, 0b10000000, 0b10000000, // `
        0b00110000, 0b01001000, 0b01001000, 0b01111000, 0b10000000, // a
        0b01111100, 0b01001000, 0b01001000, 0b00110000, 0b10000000, // b
        0b00110000, 0b01001000, 0b01001000, 0b10000000, 0b10000000, // c
        0b00110000, 0b01001000, 0b01001000, 0b01111100, 0b10000000, // d
        0b00110000, 0b01101000, 0b01011000, 0b00010000, 0b10000000, // e
        0b00010000, 0b01111000, 0b00010100, 0b10000000, 0b10000000, // f
        0b00011000, 0b10100100, 0b10100100, 0b01111100, 0b10000000, // g
        0b01111100, 0b00001000, 0b00001000, 0b01110000, 0b10000000, // h
        0b01110100, 0b10000000, 0b10000000, 0b10000000, 0b10000000, // i
        0b01000000, 0b00110100, 0b10000000, 0b10000000, 0b10000000, // j
        0b01111100, 0b00100000, 0b00110000, 0b01001000, 0b10000000, // k
        0b01111100, 0b10000000, 0b10000000, 0b10000000, 0b10000000, // l
        0b01111000, 0b00001000, 0b01111000, 0b00001000, 0b01110000, // m
        0b01111000, 0b00001000, 0b00001000, 0b01110000, 0b10000000, // n
        0b00110000, 0b01001000, 0b01001000, 0b00110000, 0b10000000, // o
        0b11111000, 0b01001000, 0b01001000, 0b00110000, 0b10000000, // p
        0b00110000, 0b01001000, 0b01001000, 0b11111000, 0b10000000, // q
        0b01111000, 0b00010000, 0b00001000, 0b10000000, 0b10000000, // r
        0b01010000, 0b01011000, 0b01101000, 0b00101000, 0b10000000, // s
        0b00001000, 0b00111100, 0b01001000, 0b10000000, 0b10000000, // t
        0b00111000, 0b01000000, 0b01000000, 0b01111000, 0b10000000, // u
        0b00111000, 0b01000000, 0b00100000, 0b00011000, 0b10000000, // v
        0b00011000, 0b01100000, 0b00011000, 0b01100000, 0b00011000, // w
        0b01001000, 0b00110000, 0b01001000, 0b10000000, 0b10000000, // x
        0b00011000, 0b10100000, 0b10100000, 0b01111000, 0b10000000, // y
        0b01001000, 0b01101000, 0b01011000, 0b01001000, 0b10000000, // z
        0b00010000, 0b01101100, 0b01000100, 0b10000000, 0b10000000, // {
        0b01111110, 0b10000000, 0b10000000, 0b10000000, 0b10000000, // |
        0b01000100, 0b01101100, 0b00010000, 0b10000000, 0b10000000, // }
        0b00001000, 0b00000100, 0b00001000, 0b00000100, 0b10000000, // ~
        };

/*
 // Bigger - nicer font definition - but can't fit much on screen
 static const uint8_t font[] PROGMEM =
 {
 0x00, 0x00, 0x80, 0x80, 0x80,    // sp
 0x2f, 0x80, 0x80, 0x80, 0x80,    // !
 0x07, 0x00, 0x07, 0x80, 0x80,    // "
 0x14, 0x7f, 0x14, 0x7f, 0x14,    // #
 0x24, 0x2a, 0x7f, 0x2a, 0x12,    // $
 0x62, 0x64, 0x08, 0x13, 0x23,    // %
 0x36, 0x49, 0x55, 0x22, 0x50,    // &
 0x05, 0x03, 0x80, 0x80, 0x80,    // '
 0x1c, 0x22, 0x41, 0x80, 0x80,    // (
 0x41, 0x22, 0x1c, 0x80, 0x80,    // )
 0x14, 0x08, 0x3E, 0x08, 0x14,    // *
 0x08, 0x08, 0x3E, 0x08, 0x08,    // +
 0xA0, 0x60, 0x80, 0x80, 0x80,    // ,
 0x08, 0x08, 0x08, 0x08, 0x08,    // -
 0x60, 0x60, 0x80, 0x80, 0x80,    // .
 0x20, 0x10, 0x08, 0x04, 0x02,    // /
 0x3E, 0x51, 0x49, 0x45, 0x3E,    // 0
 0x42, 0x7F, 0x40, 0x80, 0x80,    // 1
 0x42, 0x61, 0x51, 0x49, 0x46,    // 2
 0x21, 0x41, 0x45, 0x4B, 0x31,    // 3
 0x18, 0x14, 0x12, 0x7F, 0x10,    // 4
 0x27, 0x45, 0x45, 0x45, 0x39,    // 5
 0x3C, 0x4A, 0x49, 0x49, 0x30,    // 6
 0x01, 0x71, 0x09, 0x05, 0x03,    // 7
 0x36, 0x49, 0x49, 0x49, 0x36,    // 8
 0x06, 0x49, 0x49, 0x29, 0x1E,    // 9
 0x36, 0x36, 0x80, 0x80, 0x80,    // :
 0x56, 0x36, 0x80, 0x80, 0x80,    // ;
 0x08, 0x14, 0x22, 0x41, 0x80,    // <
 0x14, 0x14, 0x14, 0x14, 0x14,    // =
 0x41, 0x22, 0x14, 0x08, 0x80,    // >
 0x02, 0x01, 0x51, 0x09, 0x06,    // ?
 0x32, 0x49, 0x59, 0x51, 0x3E,    // @
 0x7C, 0x12, 0x11, 0x12, 0x7C,    // A
 0x7F, 0x49, 0x49, 0x49, 0x36,    // B
 0x3E, 0x41, 0x41, 0x41, 0x22,    // C
 0x7F, 0x41, 0x41, 0x22, 0x1C,    // D
 0x7F, 0x49, 0x49, 0x49, 0x41,    // E
 0x7F, 0x09, 0x09, 0x09, 0x01,    // F
 0x3E, 0x41, 0x49, 0x49, 0x7A,    // G
 0x7F, 0x08, 0x08, 0x08, 0x7F,    // H
 0x41, 0x7F, 0x41, 0x80, 0x80,    // I
 0x20, 0x40, 0x41, 0x3F, 0x01,    // J
 0x7F, 0x08, 0x14, 0x22, 0x41,    // K
 0x7F, 0x40, 0x40, 0x40, 0x40,    // L
 0x7F, 0x02, 0x0C, 0x02, 0x7F,    // M
 0x7F, 0x04, 0x08, 0x10, 0x7F,    // N
 0x3E, 0x41, 0x41, 0x41, 0x3E,    // O
 0x7F, 0x09, 0x09, 0x09, 0x06,    // P
 0x3E, 0x41, 0x51, 0x21, 0x5E,    // Q
 0x7F, 0x09, 0x19, 0x29, 0x46,    // R
 0x46, 0x49, 0x49, 0x49, 0x31,    // S
 0x01, 0x01, 0x7F, 0x01, 0x01,    // T
 0x3F, 0x40, 0x40, 0x40, 0x3F,    // U
 0x1F, 0x20, 0x40, 0x20, 0x1F,    // V
 0x3F, 0x40, 0x38, 0x40, 0x3F,    // W
 0x63, 0x14, 0x08, 0x14, 0x63,    // X
 0x07, 0x08, 0x70, 0x08, 0x07,    // Y
 0x61, 0x51, 0x49, 0x45, 0x43,    // Z
 0x7F, 0x41, 0x41, 0x80, 0x80,    // [
 0x55, 0x2A, 0x55, 0x2A, 0x55,    // 55 - should be /
 0x41, 0x41, 0x7F, 0x00, 0x80,    // ]
 0x04, 0x02, 0x01, 0x02, 0x04,    // ^
 0x40, 0x40, 0x40, 0x40, 0x40,    // _
 0x01, 0x02, 0x04, 0x80, 0x80,    // '
 0x20, 0x54, 0x54, 0x54, 0x78,    // a
 0x7F, 0x48, 0x44, 0x44, 0x38,    // b
 0x38, 0x44, 0x44, 0x44, 0x20,    // c
 0x38, 0x44, 0x44, 0x48, 0x7F,    // d
 0x38, 0x54, 0x54, 0x54, 0x18,    // e
 0x08, 0x7E, 0x09, 0x01, 0x02,    // f
 0x18, 0xA4, 0xA4, 0xA4, 0x7C,    // g
 0x7F, 0x08, 0x04, 0x04, 0x78,    // h
 0x44, 0x7D, 0x40, 0x80, 0x80,    // i
 0x40, 0x80, 0x84, 0x7D, 0x80,    // j
 0x7F, 0x10, 0x28, 0x44, 0x80,    // k
 0x00, 0x41, 0x7F, 0x40, 0x80,    // l
 0x7C, 0x04, 0x18, 0x04, 0x78,    // m
 0x7C, 0x08, 0x04, 0x04, 0x78,    // n
 0x38, 0x44, 0x44, 0x44, 0x38,    // o
 0xFC, 0x24, 0x24, 0x24, 0x18,    // p
 0x18, 0x24, 0x24, 0x18, 0xFC,    // q
 0x7C, 0x08, 0x04, 0x04, 0x08,    // r
 0x48, 0x54, 0x54, 0x54, 0x20,    // s
 0x04, 0x3F, 0x44, 0x40, 0x20,    // t
 0x3C, 0x40, 0x40, 0x20, 0x7C,    // u
 0x1C, 0x20, 0x40, 0x20, 0x1C,    // v
 0x3C, 0x40, 0x30, 0x40, 0x3C,    // w
 0x44, 0x28, 0x10, 0x28, 0x44,    // x
 0x1C, 0xA0, 0xA0, 0xA0, 0x7C,    // y
 0x44, 0x64, 0x54, 0x4C, 0x44,    // z
 };
 */

void LCD_init(void) {
    SPI_DDR |= (1 << DC) | (1 << SCE) | (1 << RST) | (1 << SCK) | (1 << MOSI);
    SET_RST_PIN;

    SPCR = 0x50; //setup SPI

    TIMER_delay_ms(100);

    CLEAR_SCE_PIN;    //Enable LCD

    CLEAR_RST_PIN;    //reset LCD
    TIMER_delay_ms(100);
    SET_RST_PIN;

    SET_SCE_PIN;    //disable LCD

    LCD_writeCommand(0x21);  // LCD Extended Commands.
    LCD_writeCommand(0xE0);  // Set LCD Vop (Contrast).
    LCD_writeCommand(0x04);  // Set Temp coefficent.
    LCD_writeCommand(0x13);  // LCD bias mode 1:48.
    LCD_writeCommand(0x20); // LCD Standard Commands, Horizontal addressing mode.
    LCD_writeCommand(0x0c);  // LCD in normal mode.

    LCD_clear();
}

void LCD_writeCommand(char command) {
    CLEAR_SCE_PIN;      //enable LCD

    CLEAR_DC_PIN;      //set LCD in command mode

    //  Send data to display controller.
    SPDR = command;

    //  Wait until Tx register empty.
    while (!(SPSR & (1 << SPIF)))
        ;

    SET_SCE_PIN;        //disable LCD
}

void LCD_writeData(char data) {
    CLEAR_SCE_PIN;      //enable LCD

    SET_DC_PIN;      //set LCD in Data mode

    //  Send data to display controller.
    SPDR = data;

    //  Wait until Tx register empty.
    while (!(SPSR & (1 << SPIF)))
        ;

    SET_SCE_PIN;        //disable LCD

    cursor_col = (cursor_col + 1) % LCD_WIDTH;
    if (cursor_col == 0)
        cursor_row = (cursor_row + 1) % LCD_HEIGHT;
}

void LCD_clear(void) {
    uint8_t i, j;

    LCD_goto(0, 0);  //start with (0,0) position

    for (i = 0; i < LCD_HEIGHT; i++)
        for (j = 0; j < LCD_WIDTH; j++) {
            LCD_writeData(0x00);
        }

    LCD_goto(0, 0);    //bring the XY position back to (0,0)

}

void LCD_clearLine(uint8_t line) {
    uint8_t j;

    if (line >= LCD_HEIGHT)
        return;

    LCD_goto(0, line);

    for (j = 0; j < LCD_WIDTH; j++) {
        LCD_writeData(0x00);
    }

    LCD_goto(0, line);    //go to the begining of the line
}

void LCD_goto(uint8_t x, uint8_t y) {
    LCD_writeCommand(0x80 | x);   //column
    LCD_writeCommand(0x40 | y);   //row

    cursor_row = y;
    cursor_col = x;
}

void LCD_puts(const __flash char *s, uint8_t type) {
    lcd_wasnl = 0;
    while (*s) {
        LCD_putc(*s, type | LCD_TYP_CKNL); // force new-line check here
        s++;
    }
    // if the function ended before it had the chance to goto next line when it had to:
    if (lcd_wasnl == 1)
        LCD_goto(0, ++cursor_row); // we will do it
    lcd_wasnl = 0;
}

void LCD_putc(char character, uint8_t type) {
    // Code below borrowed and modified from: http://www.instructables.com/id/Connecting-Nokia-3310-LCD-to-USB-using-AVR/step4/Programming-the-ATmega/

    // if we've been forced to a new line and it requires handling, go to a new line since we went back up
    if ((type & LCD_TYP_CKNL) && lcd_wasnl == 1) {
        LCD_goto(0, ++cursor_row);
        lcd_wasnl = 0;

        if (character == ' ')
            return; // if space is next, just ignore it
        if (character == '\n')
            return; // if the next in line is NEWLINE, ignore it also since we just created a newline here
    }

    // Newline \n = 0x0A in hex, if the char we want to draw is this,
    // we call LCD_goto and increase Y by 1
    if (character == '\n') {
        LCD_goto(0, ++cursor_row);
        return;    // We don't have anything more to do, return
    }

    // Sometimes the character we want to draw is too close to the edge of the screen
    // to be drawn normally. To prevent the character from being partly drawn over 2 lines
    // we check if we have enough room to draw the character, and if we don't we simply
    // draw it on the next line instead

    // If there are less than 5 pixels left of this,
    // Count how many pixels this character needs
    if (LCD_WIDTH - cursor_col < 5) {
        uint8_t width = 0;
        for (uint8_t j = 0; j < 5; j++) {
            // Count if this is not a skip
            if (font[(character-32)*5 + j] != 0b10000000) {
                width++;
            }
        }
        // If the width of this character is over the amount of pixels left on the X axis
        if (width > LCD_WIDTH - cursor_col) {
            LCD_goto(0, ++cursor_row);    // Goto next line
        }
    }

    // For 5 pixels (Our font maximum width)
    for (uint8_t j = 0; j < 5; j++) {
        // The first draw-able character we have is space (32 in decimal)
        // Our font do not use the first 32 ASCII characters, so we remove 32 from the character
        // we want to draw, and we get the same character as in out font!

        // As some of the characters don't use all 5 pixels, we can make it look nicer by not
        // drawing the empty space. 0b10000000 (0x80) is the value for not drawing anything in our font, aka "skip"
        // so, we don't print the 0x80 columns, we ignore them
        if (font[(character-32)*5 + j] != 0b10000000) {
            // Remove 32 from the character we want to draw, and then draw it.
            uint8_t tmp = font[(character-32)*5 + j];

            // inverted?
            if (type & LCD_TYP_INV) {
                tmp = 255 - tmp; // invert
            }

            // shift character up
            for (uint8_t t = 0; t < (type & 0b00000011); t++) {
                tmp = tmp >> 1; // shift up "type & 0b00000011" many times (0-3)
            }

            // underlined with BLACK pixel?
            if (type & LCD_TYP_ULI) {
                tmp = tmp | 0b10000000; // set bottom pixel
            }

            // underlined with WHITE pixel?
            if (type & LCD_TYP_ULIW) {
                tmp = tmp & 0b01111111; // clear bottom pixel
            }

            // write to LCD
            LCD_writeData(tmp);
        }
    }

    // Draw one pixel of spacing between the character and the next one
    // but only if we were not moved to new line right after printing the character above
    if (cursor_col > 0) // this is how we know that :)
            {
        uint8_t tmp = 0x00;

        // inverted?
        if (type & LCD_TYP_INV) {
            tmp = 255 - tmp; // invert
        }

        // shift character up
        for (uint8_t t = 0; t < (type & 0b00000011); t++) {
            tmp = tmp >> 1; // shift up "type & 0b00000011" many times (0-3)
        }

        // underlined with BLACK pixel?
        if (type & LCD_TYP_ULI) {
            tmp = tmp | 0b10000000; // set bottom pixel
        }

        // underlined with WHITE pixel?
        if (type & LCD_TYP_ULIW) {
            tmp = tmp & 0b01111111; // clear bottom pixel
        }

        // write to LCD
        LCD_writeData(tmp);
    }

    // if we've been forced to a new line, and it requires handling, move back above
    if (cursor_col == 0 && (type & LCD_TYP_CKNL)) {
        LCD_goto(0, --cursor_row); // return up one line, and at the beginning
        lcd_wasnl = 1; // mark that we need to move below right after this
    }
}

void LCD_writeUnsignedValue(uint32_t value, uint8_t type) {
    // code borrowed and modified from: http://www.jave.de/blog2/?p=71
    uint32_t dividend;
    for (dividend = 1000000000; dividend > 0; dividend /= 10) {
        if (dividend == 1 || value >= dividend) {
            LCD_putc(value / dividend % 10 + '0', type);
        }
    }
}

void LCD_putcBig(char character, uint8_t type) {
    uint8_t i, j;
    uint8_t font_element, big_font_element = 0x00, temp_col, temp_row;
    // backup the LCD cursor position
    temp_col = cursor_col;
    temp_row = cursor_row;
    for (j = 0; j < 10; j++) {
        font_element = font[(character-32)*5 + j/2];
        // skip emty space
        if (font_element == 0x80)
            continue;
        // inverted?
        if (type & LCD_TYP_INV) {
            font_element = 255 - font_element; // invert
        }
        // shift character up
        for (uint8_t t = 0; t < (type & 0b00000011); t++) {
            font_element = font_element >> 1; // shift up "type & 0b00000011" many times (0-3)
        }
        // underlined with BLACK pixel?
        if (type & LCD_TYP_ULI) {
            font_element = font_element | 0b10000000; // set bottom pixel
        }
        // underlined with WHITE pixel?
        if (type & LCD_TYP_ULIW) {
            font_element = font_element & 0b01111111; // clear bottom pixel
        }
        // interpolate the upper part of the character element
        for (i = 0; i < 8; i++) {
            if ((1 << i / 2) & font_element)
                big_font_element |= (1 << i);
            else
                big_font_element &= ~(1 << i);
        }
        LCD_writeData(big_font_element);
        temp_col = cursor_col;  // store the column of the upper part
        // interpolate the bottom part of the character element
        for (i = 0; i < 8; i++) {
            if ((1 << (4 + i / 2)) & font_element)
                big_font_element |= (1 << i);
            else
                big_font_element &= ~(1 << i);
        }
        LCD_goto(cursor_col - 1, temp_row + 1); // prepare to write to the bottom part
        LCD_writeData(big_font_element);
        LCD_goto(temp_col, temp_row);   // go back to the saved position for upper part
    }

    LCD_goto(cursor_col, temp_row);
}

void LCD_putCustomChar(const __flash char *ch, uint8_t width, uint8_t type){
    uint8_t i, line;
    for(i = 0; i < width; i++){
        line = ch[i];
        (type == LCD_TYP_INV) ? LCD_writeData(~line) : LCD_writeData(line);
    }
}
