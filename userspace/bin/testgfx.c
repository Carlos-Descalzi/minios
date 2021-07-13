#include "stdio.h"
#include "fcntl.h"
#include "ioctl.h"
#include "mman.h"
#include "stdint.h"
#include "string.h"

#define OPT_SET_VIDEO_MODE  4

#define WIDTH               1024
#define HEIGHT              768
#define BPP                 32
#define FB_SIZE             (WIDTH * HEIGHT * (BPP/4))

typedef struct {
    uint8_t     mode;
    uint16_t    width;
    uint16_t    height;
    uint8_t     bpp;
} ModeSettings;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;

} Window;

static void fillscreen(void* buffer, uint32_t val){
    memsetdw(buffer,val,FB_SIZE / 4);
}

static void hline(void* buffer, int xs, int xe, int y, uint32_t color){
    memsetdw(buffer+(y*WIDTH+xs)*4,color,(xe-xs));
}

static void vline(void* buffer, int x, int ys, int ye, uint32_t color){
    for (int i=ys;i<ye;i++){
        ((uint32_t*)buffer)[x+i*WIDTH] = color;
    }
}

static void fillbox(void* buffer, int x, int y, int mx, int my, uint32_t color){
    // TODO: make it in assembly for speed.
    for (int i = y;i<=my;i++){
        hline(buffer,x,mx,i,color);
    }
}

static void window_draw(void* buffer, Window* w){
    
    fillbox(buffer, w->x, w->y, w->x + w->w -1, w->y + 16, 0x00888888);
    fillbox(buffer, w->x, w->y + 16, w->x + w->w -1, w->y + w->h - 1, 0x0);

    hline(buffer, w->x,w->x + w->w, w->y, 0x00FFFFFF);
    hline(buffer, w->x,w->x + w->w, w->y + w->h, 0x00FFFFFF);
    hline(buffer, w->x,w->x + w->w, w->y + 16 , 0x00FFFFFF);
    vline(buffer, w->x, w->y, w->y + w->h, 0x00FFFFFF);
    vline(buffer, w->x + w->w, w->y, w->y + w->h, 0x00FFFFFF);
}

int main(){

    ModeSettings settings;

    settings.mode = 1;
    settings.width = WIDTH;
    settings.height = HEIGHT;
    settings.bpp = BPP;

    int fd = open("video0:", O_RDWR);

    if (ioctl(fd, OPT_SET_VIDEO_MODE, &settings) < 0){
        printf("Cannot set video mode\n");
    }

    uint32_t* buffer = mmap(NULL, FB_SIZE, 0, 0, fd, 0);

    if (!buffer){
        settings.mode = 0;
        printf("Cannot map frame buffer\n");
    }

    Window window = {
        .x = 10,
        .y = 10,
        .w = 640,
        .h = 480
    };

    fillscreen(buffer, 0x00555555);

    window_draw(buffer, &window);

    return 0;
}
