#include "window.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"
#include "sched.h"
#include "mouse.h"
#include "stdlib.h"
#include "minios.h"

typedef struct {
    int fd;
    int x;
    int y;
    MouseState prev_state;
} Mouse;

typedef struct {
    ListNode head;
    Window* window;
} WindowNode;

#define WINDOW_NODE(n)  ((WindowNode*)(n))

static void mouse_init();
static void keyboard_init();
static int check_events(Event* event);
static int handle_mouse(Event* event);
static int handle_keyboard(Event* event);
static void redraw();
static void update_cursor(int init);

static int active;
static GfxContext ctx;
static Mouse mouse;
static ListNode* window_list;

static struct {
    EventType event_type;
    int (*handler) (Event*);
} event_handlers[] = {
    {   EVENT_TYPE_MOUSE,   handle_mouse    },
    {   EVENT_TYPE_KEY,     handle_keyboard },
    {   EVENT_TYPE_NONE,    NULL            }
};

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
static uint32_t cursor_buffer[8 * 10];
static uint32_t px, py;
#define CURSOR_WIDTH        8
#define CURSOR_HEIGHT       10
#define WIDTH           1024
#define HEIGHT          768
#define BGCOLOR         0x00555555

void wmanager_init (void){
    window_list = NULL;
    gfx_init(&ctx, WIDTH, HEIGHT, 0);
    mouse_init();
    keyboard_init();
    active = 1;
}
void wmanager_add_window (Window* window){
    WindowNode* node = malloc(sizeof(WindowNode));
    node->window = window;
    node->window->ctx = &ctx;
    node->window->state = WINDOW_STATE_ACTIVE;
    window_list = list_add(window_list, LIST_NODE(node));
}
void wmanager_loop (void){
    Event event;

    redraw();

    while(active){

        if (check_events(&event)){
            int handled = 0;
            for (ListNode* node = window_list; node; node = node->next){
                if (window_handle_event(WINDOW_NODE(node)->window, &event)){
                    redraw();
                    handled = 1;
                    break;
                }
            }

            if (!handled){
                update_cursor(0);
            }
        }

        sched_yield();
    }
}

static void redraw(){
    gfx_fillscreen(&ctx, BGCOLOR);

    for (ListNode* node = window_list; node; node = node->next){
        window_draw(WINDOW_NODE(node)->window);
    }

    update_cursor(1);
}

static void mouse_init(){

    memset(&mouse,0, sizeof(Mouse));
    mouse.x = 512;
    mouse.y = 384;

    mouse.fd = open("mouse0:",O_RDONLY | O_NONBLOCK);
}
static void keyboard_init(){
}


static int check_events(Event* event){

    for (int i=0;event_handlers[i].handler;i++){
        int result = event_handlers[i].handler(event);

        if (result){
            event->type = event_handlers[i].event_type;
            return result;
        }
    }
    return 0;
}
static int handle_mouse(Event* event){
    MouseState mouse_state;

    int result = read(mouse.fd, &mouse_state, sizeof(MouseState));

    if (result){

        int x = mouse.x + mouse_state.xm;
        int y = mouse.y - mouse_state.ym;

        if (mouse_state.bl && !mouse.prev_state.bl){
            event->mouse.type = MOUSE_EVENT_PRESS;
            event->mouse.button = 1;
        } else if (!mouse_state.bl && mouse.prev_state.bl){
            event->mouse.type = MOUSE_EVENT_RELEASE;
            event->mouse.button = 1;
        } else if (mouse_state.bm && !mouse.prev_state.bm){
            event->mouse.type = MOUSE_EVENT_PRESS;
            event->mouse.button = 2;
        } else if (!mouse_state.bm && mouse.prev_state.bm){
            event->mouse.type = MOUSE_EVENT_RELEASE;
            event->mouse.button = 2;
        } else if (mouse_state.br && !mouse.prev_state.br){
            event->mouse.type = MOUSE_EVENT_PRESS;
            event->mouse.button = 3;
        } else if (!mouse_state.br && mouse.prev_state.br){
            event->mouse.type = MOUSE_EVENT_RELEASE;
            event->mouse.button = 3;
        } else if (x != mouse.x || y != mouse.y){
            event->mouse.type = MOUSE_EVENT_MOVE;
        }
        event->mouse.pos.x = x;
        event->mouse.pos.y = y;

        mouse.x = x;
        mouse.y = y;
        memcpy(&(mouse.prev_state), &mouse_state, sizeof(MouseState));
        return 1;
    }
    return 0;
}
static int handle_keyboard(Event* event){
    return 0;
}
static void update_cursor (int init){

    if (!init){
        gfx_copy_from(
            &ctx, 
            px, 
            py, 
            CURSOR_WIDTH, 
            CURSOR_HEIGHT, 
            cursor_buffer,
            0xFF000000
        );
    } 

    gfx_copy_to(
        &ctx, 
        mouse.x, 
        mouse.y, 
        CURSOR_WIDTH, 
        CURSOR_HEIGHT, 
        cursor_buffer
    );

    gfx_copy_from(
        &ctx, 
        mouse.x, 
        mouse.y, 
        CURSOR_WIDTH, 
        CURSOR_HEIGHT, 
        cursor, 
        0xFF000000
    );

    px = mouse.x;
    py = mouse.y;
}
