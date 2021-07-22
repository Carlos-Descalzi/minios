#include "window.h"
#include "stdlib.h"
#include "minios.h"

typedef struct ComponentNode {
    ListNode head;
    Component* component;
} ComponentNode;

#define COMPONENT_NODE(c)       ((ComponentNode*)c)

void window_init (Window* window, char* title){
    window->rect.x = 0;
    window->rect.y = 0;
    window->rect.w = 0;
    window->rect.h = 0;
    window->title = title;
    window->children = NULL;
    window->press_point.x = 0;
    window->press_point.y = 0;
}
int window_child_count (Window* window){
    int count = 0;

    for (
        ListNode* child = window->children; 
        child; 
        child = child->next, count++
    );

    return count;
}
void window_add_child (Window* window, Component* component){

    ComponentNode* node = malloc(sizeof(ComponentNode));
    node->component = component;

    window->children = list_add(window->children, LIST_NODE(node));
}
void window_draw (Window* window){
    if (!window->ctx){
        return;
    }
    gfx_fillbox(window->ctx, 
        window->rect.x,
        window->rect.y,
        window->rect.x + window->rect.w -1,
        window->rect.y + 15,
        0x0
    );

    gfx_hline(window->ctx, 
        window->rect.x,
        window->rect.x + window->rect.w -1,
        window->rect.y,
        0x00FFFFFF
    );
    gfx_hline(window->ctx, 
        window->rect.x,
        window->rect.x + window->rect.w -1,
        window->rect.y + 15,
        0x00FFFFFF
    );
    gfx_hline(window->ctx, 
        window->rect.x,
        window->rect.x + window->rect.w -1,
        window->rect.y + window->rect.h -1,
        0x00FFFFFF
    );

    gfx_vline(window->ctx,
        window->rect.x,
        window->rect.y,
        window->rect.y + window->rect.h -1,
        0x00FFFFFF
    );
    gfx_vline(window->ctx,
        window->rect.x + window->rect.w -1,
        window->rect.y,
        window->rect.y + window->rect.h -1,
        0x00FFFFFF
    );
}

#define W_STATE_DRAGGING    1

static int handle_mouse_event(Window* window, Event* event){
    Rect header_rect = {
        .x = window->rect.x,
        .y = window->rect.y,
        .w = window->rect.w,
        .h = 16
    };
    Rect body_rect = {
        .x = window->rect.x,
        .y = window->rect.y + 16,
        .w = window->rect.w,
        .h = window->rect.h -16
    };

    if (rect_contains_point(&header_rect, &(event->mouse.pos))){
        if (event->mouse.type == MOUSE_EVENT_PRESS 
            && event->mouse.button == 1){
            window->press_point = event->mouse.pos;
            window->internal_state |= W_STATE_DRAGGING;

            debug("Start drag\n");
            return 1;
        } else if (event->mouse.type == MOUSE_EVENT_RELEASE
            && event->mouse.button == 1){
            window->internal_state &= ~ W_STATE_DRAGGING;
            return 1;
        } else if (window->internal_state & W_STATE_DRAGGING
            && event->mouse.type == MOUSE_EVENT_MOVE){

            int dx = event->mouse.pos.x - window->press_point.x;
            int dy = event->mouse.pos.y - window->press_point.y;

            window->press_point = event->mouse.pos;

            window->rect.x += dx;
            window->rect.y += dy;

            debug("Dragging ....\n");

            return 1;
        }
    } else {
        if (rect_contains_point(&body_rect, &(event->mouse.pos))){
            for (ListNode* node = window->children; node; node = node->next){
                if (rect_contains_point(&(COMPONENT_NODE(node)->component->rect), &(event->mouse.pos))){
                    component_handle_event(COMPONENT_NODE(node)->component, event);
                    return 1;
                }
            }
        } else {
            debug("Unhandled event\n");
        }
    }
    return 0;
}

int window_handle_event (Window* window, Event* event){
    if (window->state == WINDOW_STATE_ACTIVE){

        if (event->type == EVENT_TYPE_MOUSE
            && rect_contains_point(&(window->rect), &(event->mouse.pos))){
            return handle_mouse_event(window, event);
        }
    }
    return 0;
}
