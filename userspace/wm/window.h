#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "list.h"
#include "event.h"
#include "gfx.h"

typedef struct Window Window;

typedef struct Component {
    int type;
    Rect rect;
    Window* window;


    void    (*draw)                 (struct Component*);
    void    (*handle_event)         (struct Component*, Event*);
} Component;

#define COMPONENT(c)                ((Component*)(c))

#define component_draw(c)           (COMPONENT(c))->draw(COMPONENT(c))
#define component_handle_event(c,e) (COMPONENT(c))->handle_event(COMPONENT(c),e)

typedef enum {
    WINDOW_STATE_INACTIVE,
    WINDOW_STATE_ACTIVE
} WindowState;

struct Window {
    GfxContext* ctx;
    WindowState state;
    uint32_t internal_state;
    Rect rect;
    Point press_point;
    char* title;
    ListNode* children;
};

void        wmanager_init           (void);
void        wmanager_add_window     (Window* window);
void        wmanager_loop           (void);

void        window_init             (Window* window, char* title);
int         window_child_count      (Window* window);
void        window_add_child        (Window* window, Component* component);
void        window_draw             (Window* window);
/**
 * returns 1 if window was able to handle this event, otherwise 0
 **/
int         window_handle_event     (Window* window, Event* event);

#endif
