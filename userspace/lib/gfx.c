#include "gfx.h"
#include "string.h"
#include "mman.h"
#include "ioctl.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

#define OPT_SET_VIDEO_MODE  4
#define BPP                 32

typedef struct {
    uint8_t     mode;
    uint16_t    width;
    uint16_t    height;
    uint8_t     bpp;
} ModeSettings;

void gfx_init (GfxContext* ctx, uint16_t width, uint16_t height, int double_buffered){
    ctx->fd = open("video0:", O_RDWR);
    ctx->screen_width = width;
    ctx->screen_height = height;
    ctx->double_buffered = double_buffered;

    ModeSettings settings = {
        .mode = 1,
        .width = width,
        .height = height,
        .bpp = BPP
    };

    ioctl(ctx->fd, OPT_SET_VIDEO_MODE, &settings);

    uint32_t fb_size = ctx->screen_width * ctx->screen_height * 4;

    ctx->frame_buffer = mmap(NULL, fb_size, 0, 0, ctx->fd, 0);

    if (ctx->double_buffered){
        ctx->buffer = malloc(fb_size);
    }
}
void gfx_plot (GfxContext* ctx, uint16_t x, uint16_t y, uint32_t color){

    uint32_t* frame_buffer = ctx->double_buffered 
        ? ctx->buffer
        : ctx->frame_buffer;

    frame_buffer[x + y * ctx->screen_width] = color;
}
void gfx_fillscreen (GfxContext* ctx, uint32_t val){

    uint32_t* frame_buffer = ctx->double_buffered 
        ? ctx->buffer
        : ctx->frame_buffer;

    memsetdw(
        frame_buffer,
        val, 
        ctx->screen_width * ctx->screen_height
    );

}

void gfx_hline (GfxContext* ctx, uint16_t xs, uint16_t xe, uint16_t y, uint32_t color){

    uint32_t* frame_buffer = ctx->double_buffered 
        ? ctx->buffer
        : ctx->frame_buffer;

    memsetdw(
        frame_buffer+ ( y * ctx->screen_width + xs ),
        color,
        (xe-xs)
    );

}

void gfx_vline (GfxContext* ctx, uint16_t x, uint16_t ys, uint16_t ye, uint32_t color){

    uint32_t* frame_buffer = ctx->double_buffered 
        ? ctx->buffer
        : ctx->frame_buffer;

    for (int i=ys;i<ye;i++){
        frame_buffer[ x + i * ctx->screen_width ] = color;
    }

}

void gfx_fillbox (GfxContext* ctx, uint16_t x, uint16_t y, uint16_t mx, uint16_t my, uint32_t color){

    for (int i = y;i<=my;i++){
        gfx_hline(ctx,x,mx,i,color);
    }

}

void gfx_exit (GfxContext* ctx){
    ModeSettings settings = {
        .mode = 0,
    };

    ioctl(ctx->fd, OPT_SET_VIDEO_MODE, &settings);
}

void gfx_copy_to (
    GfxContext* ctx, 
    uint16_t x, uint16_t y, 
    uint16_t w, uint16_t h, 
    uint32_t* target){

    uint32_t* frame_buffer = ctx->double_buffered 
        ? ctx->buffer
        : ctx->frame_buffer;

    for (uint16_t i=0; i<w; i++){
        for (uint16_t j=0; j<h; j++){

            target[i + j * w] = frame_buffer[ x + i + ( y + j ) * ctx->screen_width ];

        }
    }
}

void gfx_copy_from (
        GfxContext* ctx, 
        uint16_t x, uint16_t y, 
        uint16_t w, uint16_t h, 
        uint32_t* source, 
        uint32_t key_color){

    uint32_t* frame_buffer = ctx->double_buffered 
        ? ctx->buffer
        : ctx->frame_buffer;

    for (uint16_t i=0; i<w; i++){
        for (uint16_t j=0; j<h; j++){

            uint32_t data = source[i + j * w];

            if (data != key_color){
                frame_buffer[ x + i + ( y + j ) * ctx->screen_width ] = data;
            }
        }
    }
}

void gfx_flush(GfxContext* ctx){
    if (ctx->double_buffered){
        memcpydw(ctx->frame_buffer, ctx->buffer, ctx->screen_width * ctx->screen_height);
    }
}
