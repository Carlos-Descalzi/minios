#include "stdio.h"
#include "fcntl.h"
#include "ioctl.h"
#include "mman.h"
#include "stdint.h"

#define OPT_SET_VIDEO_MODE  4

#define WIDTH               640
#define HEGIHT              480
#define BPP                 32

typedef struct {
    uint8_t     mode;
    uint16_t    width;
    uint16_t    height;
    uint8_t     bpp;
} ModeSettings;

int main(){

    ModeSettings settings;

    settings.mode = 1;
    settings.width = WIDTH;
    settings.height = HEGIHT;
    settings.bpp = BPP;

    int fd = open("video0:", O_RDWR);

    if (ioctl(fd, OPT_SET_VIDEO_MODE, &settings) < 0){
        printf("Cannot set video mode\n");
    }

    uint32_t* buffer = mmap(NULL, WIDTH * HEGIHT * (BPP/4), 0, 0, fd, 0);

    if (!buffer){
        settings.mode = 0;
        printf("Cannot map frame buffer\n");
    }

    for (int i=0;i<HEGIHT;i++){
        buffer[i * WIDTH + i] = 0xFFFFFFFF;
    }
    ioctl(fd, OPT_SET_VIDEO_MODE, &settings);

    return 0;
}
