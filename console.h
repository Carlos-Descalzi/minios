#ifndef _CONSOLE_H_
#define _CONSOLE_H_
/**
 * Basic raw console interface
 **/

#define CONSOLE_COLOR_BLACK         0x00
#define CONSOLE_COLOR_BLUE          0x01
#define CONSOLE_COLOR_GREEN         0x02
#define CONSOLE_COLOR_CYAN          0x03
#define CONSOLE_COLOR_RED           0x04
#define CONSOLE_COLOR_PURPLE        0x05
#define CONSOLE_COLOR_BROWN         0x06
#define CONSOLE_COLOR_GRAY          0x07
#define CONSOLE_COLOR_DARK_GRAY     0x08
#define CONSOLE_COLOR_LIGHT_BLUE    0x09
#define CONSOLE_COLOR_LIGHT_GREEN   0x0A
#define CONSOLE_COLOR_LIGHT_CYAN    0x0B
#define CONSOLE_COLOR_LIGHT_RED     0x0C
#define CONSOLE_COLOR_LIGHT_PURPLE  0x0D
#define CONSOLE_COLOR_YELLOW        0x0E
#define CONSOLE_COLOR_WHITE         0x0F

/**
 * Console initialization
 **/
void console_init();
/**
 * Set current color attributes
 **/
void console_color(unsigned char color);
/**
 * Moves cursor to the given x/y position on screen
 **/
void console_gotoxy(unsigned char x, unsigned char y);
/**
 * Prints a string in console.
 * does not handle escape characters
 **/
void console_print(const char* msg);
/**
 * Puts a character in console in actual position x/y.
 **/
void console_put(const char c);
/**
 * Enable cursor
 **/
void console_cursor_on(void);
/**
 * Disable cursor
 **/
void console_cursor_off(void);
#endif
