#include "board/console.h"
#include "board/io.h"

#define SCREEN                  ((unsigned short*)0xb8000)
#define SCREEN_WIDTH            80
#define SCREEN_HEIGHT           25
#define SCREEN_SIZE             (SCREEN_WIDTH * SCREEN_HEIGHT)
#define SCREEN_POS_MAX          (SCREEN_SIZE - 1)

static unsigned int pos;
static unsigned char color;
static unsigned char cursor;

static void scroll_up           (void);
static void update_cursor       (void);
static void do_console_put      (const char c);
static void do_clear            (void);

void console_init(){
    pos = 0;
    color = 0xF;
    do_clear();
    console_cursor_on();
    update_cursor();
}
void console_clear_screen(void){
    do_clear();
    update_cursor();
}

void console_color(unsigned char c){
    color = c;
}

void console_gotoxy(unsigned char x, unsigned char y){
    pos = y * 80 + x;
    update_cursor();
}

void console_print(const char* msg){
    for (unsigned char i=0;msg[i];do_console_put(msg[i++]));
    update_cursor();
}

void console_put(const char c){
    do_console_put(c);
    update_cursor();
}

void console_cursor_on(void){
    cursor = 1;
    outb(0x3d4, 0x0a);
    outb(0x3d5, (inb(0x3d5) & 0xC0) | 0x1);
    outb(0x3d4, 0x0b);
    outb(0x3d5, (inb(0x3d5) & 0xE0) | 0xF);
}

void console_cursor_off(void){
    cursor = 0;
    outb(0x3d4, 0x0a);
    outb(0x3d5, 0x20);
}

static void scroll_up(){
    int i;
    for (i=0;i<(SCREEN_SIZE - SCREEN_WIDTH);i++){
        SCREEN[i] = SCREEN[i+SCREEN_WIDTH];
    }
    for (i=0;i<SCREEN_WIDTH;i++){
        SCREEN[(SCREEN_SIZE - SCREEN_WIDTH)+i] = color << 8;
    }
    pos = SCREEN_SIZE-SCREEN_WIDTH;
}

static void do_console_put(const char c){
    if (c == '\n'){
        pos+=SCREEN_WIDTH - (pos % SCREEN_WIDTH);
    } else {
        SCREEN[pos++] = (color << 8) | c;
    }
    if (pos > SCREEN_POS_MAX){
        scroll_up();
    }
}

static void update_cursor(void){
    if (cursor){
        outb(0x3d4,0x0F);
        outb(0x3d5,pos & 0xFF);
        outb(0x3d4,0x0E);
        outb(0x3d5,(pos >> 8) & 0xFF);
    }
}

static void do_clear(){
    int i;
    for (i=0;i<SCREEN_SIZE;i++){
        SCREEN[i] = color << 8;
    }
}
void console_get_cursor_pos(unsigned char* x, unsigned char* y){
    *y = pos / SCREEN_WIDTH;
    *x = pos % SCREEN_HEIGHT;
}
