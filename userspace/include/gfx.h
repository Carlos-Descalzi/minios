#ifndef _GFX_H_
#define _GFX_H_

#include "stdint.h"

// always 32 bpp
typedef struct {
    int fd;
    void* frame_buffer;
    uint16_t screen_width;
    uint16_t screen_height;
    uint8_t* font;
    uint8_t font_width;
    uint8_t font_height;
    void* buffer;
    int double_buffered;
} GfxContext;

void gfx_init       (GfxContext* ctx, uint16_t width, uint16_t height, int double_buffered);
void gfx_fillscreen (GfxContext* ctx, uint32_t val);
void gfx_hline      (GfxContext* ctx, uint16_t xs, uint16_t xe, uint16_t y, uint32_t color);
void gfx_vline      (GfxContext* ctx, uint16_t x, uint16_t ys, uint16_t ye, uint32_t color);
void gfx_fillbox    (GfxContext* ctx, uint16_t x, uint16_t y, uint16_t mx, uint16_t my, uint32_t color);
void gfx_plot       (GfxContext* ctx, uint16_t x, uint16_t y, uint32_t color);
void gfx_copy_to    (GfxContext* ctx, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t* target);
void gfx_copy_from  (GfxContext* ctx, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t* target, 
                    uint32_t key_color);
void gfx_flush      (GfxContext* ctx);
void gfx_exit       (GfxContext* ctx);

#endif 

