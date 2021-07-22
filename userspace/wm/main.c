#include "window.h"

int main(int argc, char** argv){
    Window window;

    wmanager_init();

    window_init(&window, "hello");
    window.rect.x = 100;
    window.rect.y = 100;
    window.rect.w = 500;
    window.rect.h = 400;

    wmanager_add_window(&window);

    wmanager_loop();
}
