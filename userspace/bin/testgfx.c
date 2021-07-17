#include "stdio.h"
#include "stdint.h"
#include "gfx.h"
#include "mouse.h"
#include "fcntl.h"

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

static void window_draw(GfxContext* ctx, Window* w){
    
    gfx_fillbox(ctx, w->x, w->y, w->x + w->w -1, w->y + 16, 0x00888888);
    gfx_fillbox(ctx, w->x, w->y + 16, w->x + w->w -1, w->y + w->h - 1, 0x0);

    gfx_hline(ctx, w->x,w->x + w->w, w->y, 0x00FFFFFF);
    gfx_hline(ctx, w->x,w->x + w->w, w->y + w->h, 0x00FFFFFF);
    gfx_hline(ctx, w->x,w->x + w->w, w->y + 16 , 0x00FFFFFF);
    gfx_vline(ctx, w->x, w->y, w->y + w->h, 0x00FFFFFF);
    gfx_vline(ctx, w->x + w->w, w->y, w->y + w->h, 0x00FFFFFF);
}


static char cursor[8][8] = {
    {   0,7,0,0,0,0,0,0 },
    {   0,7,7,0,0,0,0,0 },
    {   0,7,7,7,0,0,0,0 },
    {   0,7,7,7,7,0,0,0 },
    {   0,7,7,7,7,7,0,0 },
    {   0,7,7,7,0,0,0,0 },
    {   0,0,0,7,0,0,0,0 },
    {   0,0,0,0,7,0,0,0 }
};
static uint32_t buff[8][8];
static uint32_t px, py;

static void update_cursor(GfxContext* ctx, int x, int y, int update){
    
    uint32_t* frame_buffer = ctx->frame_buffer;

    if (update){
        for (int i=0;i<7;i++){
            for (int j=0;j<7;j++){
                frame_buffer[(px+j)+(py+i)*1024] = buff[i][j];
            }
        }
    }
    for (int i=0;i<7;i++){
        for (int j=0;j<7;j++){
            buff[i][j] = frame_buffer[ (x+j) + (y+i) * 1024];
            if (cursor[i][j]){
                frame_buffer[(x+j)+(y+i)*1024] = 0x00FFFFFF;
            }
        }
    }
    px = x;
    py = y;
}

#define DRAG    0x10
#define LPRESS  0x01

int main(){


    GfxContext ctx;
    MouseEvent event;
    MouseEvent event2;
    int status = 0;

    gfx_init(&ctx, WIDTH, HEIGHT,0);

    gfx_fillscreen(&ctx, 0x00555555);

    Window window = {
        .x = 10,
        .y = 10,
        .w = 640,
        .h = 480
    };

    window_draw(&ctx, &window);

    int fd = open("mouse0:",O_RDONLY);

    uint32_t x = 512;
    uint32_t y = 300;
    px = 512;
    py = 300;
    update_cursor(&ctx,x,y,0);

    while(1){

        read(fd,&event,sizeof(MouseEvent));

        int xm = event.xm;
        int ym = -event.ym;

        x += xm;
        y += ym;

        update_cursor(&ctx,x,y,1);

        if (event.bl){
            status |= LPRESS;
            if (!event2.bl){
                status |= DRAG;
            }
        } else {
            status &= ~LPRESS;
            if (event2.bl){
                status &= ~DRAG;
            }
        }


        memcpy(&event2, &event, sizeof(MouseEvent));

    }

    return 0;
}
