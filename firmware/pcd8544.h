#ifndef _PCD8544_LCD_H_
#define _PCD8544_LCD_H_

#include <avr/io.h>

#define DC              PB0
#define SCE             PB2
#define RST             PB4
#define SCK             PB5
#define MOSI            PB3

#define SPI_DDR         DDRB
#define SPI_PORT        PORTB

#define SET_DC_PIN      SPI_PORT |= (1 << DC)
#define CLEAR_DC_PIN    SPI_PORT &= ~(1 << DC)
#define SET_SCE_PIN     SPI_PORT |= (1 << SCE)
#define CLEAR_SCE_PIN   SPI_PORT &= ~(1 << SCE)
#define SET_RST_PIN     SPI_PORT |= (1 << RST)
#define CLEAR_RST_PIN   SPI_PORT &= ~(1 << RST)

#define LCD_WIDTH       84  // 84pixels
#define LCD_HEIGHT      6   // 6 cells of 8 pixels -> 6 * 8 = 48

// LCD_putc() "type" parameter options:
#define LCD_TYP_NORM    0b00000000    // normal
#define LCD_TYP_INV     0b10000000    // inverted text (white on black bg)
#define LCD_TYP_NOSPC   0b00000100    // no spacing between characters
#define LCD_TYP_UP1     0b00000001    // moved 1 pixel up
#define LCD_TYP_UP2     0b00000010    // moved 2 pixels up
#define LCD_TYP_ULI     0b00001000    // underlined with black pixel
#define LCD_TYP_ULIW    0b00010000    // underlined with white pixel
#define LCD_TYP_CKNL    0b00100000    // check for automatic new-line (used in in lcd_puts())

extern void TIMER_delay_ms(uint16_t _mS);

/**
 * Name         :  LCD_init
 * Description  :  Initializes the LCD display.
 * Argument(s)  :  None.
 * Return value :  None.
 */
void LCD_init(void);

/**
 * Name         :  LCD_writeCommand
 * Description  :  Sends command to display controller.
 * Argument(s)  :  command -> command to be sent
 * Return value :  None.
 */
void LCD_writeCommand(char command);

/**
 * Name         :  LCD_writeData
 * Description  :  Sends Data to display controller.
 * Argument(s)  :  Data -> Data to be sent
 * Return value :  None.
 */
void LCD_writeData(char data);

/**
 * Name         :  LCD_goto
 * Description  :  Sets cursor location to xy location corresponding to basic font size.
 * Argument(s)  :  x -> range: 0 to 83
 *                 y -> range: 0 to 5
 * Return value :  None.
 */
void LCD_goto(uint8_t x, uint8_t y);

/**
 * Name         :  LCD_clear
 * Description  :  Clears the display
 * Argument(s)  :  None.
 * Return value :  None.
 */
void LCD_clear(void);

/**
 * Name         :  LCD_clearLine
 * Description  :  Clears one line on LCD
 * Argument(s)  :  line -> line to be cleared
 * Return value :  None.
 */
void LCD_clearLine(uint8_t line);

/**
 * Name         :  LCD_putc
 * Description  :  Puts a character on lcd using font defined in .h file. Code borrowed from:
 *                 http://www.elektronika.ba/789/reverse-geocaching-geogame-v1/
 * Argument(s)  :  character -> character to be shown on lcd
 *                 type -> [0]0000000 = 0=normal text,1=inverted text
 *                          000000[00] = 00=no vertical movement, 01=moved 1 pixes up, 10=moved 2 pixels up
 *                          00000[0]00 = 0=with spacing, 1=without character spacing
 *                          000[0]0000 = 0=normal, 1=underlined(bottom pixel set)
 *                          and many more: see .h file for other LCD_TYP... constants
 * Return value :  None.
 */
void LCD_putc(char character , uint8_t type);

/**
 * Name         :  LCD_puts
 * Description  :  Put a null-terminated string on lcd. Code borrowed from:
 *                 http://www.elektronika.ba/789/reverse-geocaching-geogame-v1/
 * Argument(s)  :  s -> string
 *                 type -> [0]0000000 = 0=normal text,1=inverted text
 *                          000000[00] = 00=no vertical movement, 01=moved 1 pixes up, 10=moved 2 pixels up
 *                          00000[0]00 = 0=with spacing, 1=without character spacing
 *                          000[0]0000 = 0=normal, 1=underlined(bottom pixel set)
 *                          and many more: see .h file for other LCD_TYP... constants
 * Return value :  None.
 */
void LCD_puts(const __flash char *s, uint8_t type);

/**
 * Name         :  LCD_putcBig
 * Description  :  Puts a character on lcd using font defined in .h file, but the
 *                 size of the font is doubled.
 * Argument(s)  :  character -> character to be shown on lcd
 *                 type -> [0]0000000 = 0=normal text,1=inverted text
 *                          000000[00] = 00=no vertical movement, 01=moved 1 pixes up, 10=moved 2 pixels up
 *                          00000[0]00 = 0=with spacing, 1=without character spacing
 *                          000[0]0000 = 0=normal, 1=underlined(bottom pixel set)
 *                          and many more: see .h file for other LCD_TYP... constants
 * Return value :  None.
 */
void LCD_putcBig(char character, uint8_t type);

/**
 * Name         :  LCD_putCustomChar
 * Description  :  Puts a custom haracter on the lcd.
 * Argument(s)  :  ch -> character to be shown on lcd
 *                 type -> [0]0000000 = 0=normal text,1=inverted text
 *                          000000[00] = 00=no vertical movement, 01=moved 1 pixes up, 10=moved 2 pixels up
 *                 width -> width of the custom character
 * Return value :  None.
 */
void LCD_putCustomChar(const __flash char *ch, uint8_t width, uint8_t type);

/**
 * Name         :  LCD_writeUnsignedValue
 * Description  :  Puts unsigned value on lcd. Code borrowed from:
 *                 http://www.elektronika.ba/789/reverse-geocaching-geogame-v1/
 * Argument(s)  :  value -> unsigned value to be shown on lcd
 *                 type -> [0]0000000 = 0=normal text,1=inverted text
 *                          000000[00] = 00=no vertical movement, 01=moved 1 pixes up, 10=moved 2 pixels up
 *                          00000[0]00 = 0=with spacing, 1=without character spacing
 *                          000[0]0000 = 0=normal, 1=underlined(bottom pixel set)
 *                          and many more: see .h file for other LCD_TYP... constants
 * Return value :  None.
 */
void LCD_writeUnsignedValue(uint32_t value, uint8_t type);

#endif  // _PCD8544_LCD_H_
