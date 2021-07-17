#include "gfx.h"
#include "mouse.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"

#define WIDTH           1024
#define HEIGHT          768
#define BGCOLOR         0x00555555

#define MOUSE_LPRESS    0x0001
#define MOUSE_MPRESS    0x0002
#define MOUSE_RPRESS    0x0004
#define MOUSE_MOVE      0x0040
#define MOUSE_DRAG      0x0080

typedef struct {
    int fd;
    int x; 
    int y;
    MouseEvent prev_event;
    uint32_t state;
} Mouse;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;

    char* title;

} Window;

static uint32_t cursor[] = {
    0xFF000000,0x00010101,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0x00010101,0x00FFFFFF,0x00010101,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0x00010101,0x00FFFFFF,0x00FFFFFF,0x00010101,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0x00010101,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00010101,0xFF000000,0xFF000000,0xFF000000,
    0x00010101,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00010101,0xFF000000,0xFF000000,
    0x00010101,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00010101,0xFF000000,
    0x00010101,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00010101,0x00010101,0xFF000000,0xFF000000,
    0x00010101,0x00010101,0x00010101,0x00FFFFFF,0x00010101,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0x00010101,0x00FFFFFF,0x00010101,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0x00010101,0xFF000000,0xFF000000,0xFF000000 
};

#define CURSOR_WIDTH        8
#define CURSOR_HEIGHT       10

static uint32_t cursor_buffer[8 * 10];
static uint32_t px, py;

static void mouse_init      (Mouse* mouse);
static int  mouse_check     (Mouse* mouse);
static void cursor_update   (GfxContext* ctx, Mouse* event, int init);

static void window_init     (Window* window, int x, int y, int w, int h, char* title);
static void window_draw     (GfxContext* ctx, Window* window);

int main(int argc, char** argv){

    GfxContext ctx;
    Mouse mouse;
    Window window;

    gfx_init(&ctx, WIDTH, HEIGHT, 0);
    mouse_init(&mouse);
    window_init(&window, 100,100,500,300, "Hello world");
    //mouse_check(&mouse);


    gfx_fillscreen(&ctx, BGCOLOR);
    window_draw(&ctx, &window);
    cursor_update(&ctx, &mouse, 1);
    while(1){
        mouse_check(&mouse);
        cursor_update(&ctx, &mouse, 0);
        gfx_flush(&ctx);
    }
}
static void window_init(Window* window, int x, int y, int w, int h, char* title){
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
    window->title = title;
}
static void window_draw (GfxContext* ctx, Window* window){

    gfx_fillbox(ctx, 
        window->x,
        window->y,
        window->x + window->w -1,
        window->y + 15,
        0x0
    );

    gfx_hline(ctx, 
        window->x,
        window->x + window->w -1,
        window->y,
        0x00FFFFFF
    );
    gfx_hline(ctx, 
        window->x,
        window->x + window->w -1,
        window->y + 15,
        0x00FFFFFF
    );
    gfx_hline(ctx, 
        window->x,
        window->x + window->w -1,
        window->y + window->h -1,
        0x00FFFFFF
    );

    gfx_vline(ctx,
        window->x,
        window->y,
        window->y + window->h -1,
        0x00FFFFFF
    );
    gfx_vline(ctx,
        window->x + window->w -1,
        window->y,
        window->y + window->h -1,
        0x00FFFFFF
    );

}

static void mouse_init(Mouse* mouse){

    memset(mouse,0, sizeof(Mouse));
    mouse->x = 512;
    mouse->y = 384;

    mouse->fd = open("mouse0:",O_RDONLY);

}
static void cursor_update (GfxContext* ctx, Mouse* mouse, int init){

    if (!init){
        gfx_copy_from(
            ctx, 
            px, 
            py, 
            CURSOR_WIDTH, 
            CURSOR_HEIGHT, 
            cursor_buffer,
            0xFF000000
        );
    } 

    gfx_copy_to(
        ctx, 
        mouse->x, 
        mouse->y, 
        CURSOR_WIDTH, 
        CURSOR_HEIGHT, 
        cursor_buffer
    );

    gfx_copy_from(
        ctx, 
        mouse->x, 
        mouse->y, 
        CURSOR_WIDTH, 
        CURSOR_HEIGHT, 
        cursor, 
        0xFF000000
    );

    px = mouse->x;
    py = mouse->y;
}

static int mouse_check (Mouse* mouse){
    int has_event = 0;
    MouseEvent mouse_event;

    read(mouse->fd, &mouse_event, sizeof(MouseEvent));

    if (mouse_event.bl){
        mouse->state |= MOUSE_LPRESS;
    } else {
        mouse->state &= ~MOUSE_LPRESS;
    }

    if (mouse_event.bm){
        mouse->state |= MOUSE_MPRESS;
    } else {
        mouse->state &= ~MOUSE_MPRESS;
    }

    if (mouse_event.br){
        mouse->state |= MOUSE_RPRESS;
    } else {
        mouse->state &= ~MOUSE_RPRESS;
    }

    int x = mouse->x + mouse_event.xm;
    int y = mouse->y - mouse_event.ym;

    if (x != mouse->x || y != mouse->y){
        mouse->state |= MOUSE_MOVE;
        
        if (mouse_event.bl == mouse->prev_event.bl 
            && mouse_event.bl){
            mouse->state |= MOUSE_DRAG;
        } else {
            mouse->state &= ~MOUSE_DRAG;
        }

    } else {
        mouse->state &= ~MOUSE_MOVE;
    }

    mouse->x = x;
    mouse->y = y;

    memcpy(&(mouse->prev_event), &mouse_event, sizeof(MouseEvent));

    return has_event;
}
