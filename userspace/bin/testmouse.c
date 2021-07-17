#include "fcntl.h"
#include "stdio.h"
#include "stdint.h"
#include "unistd.h"

typedef struct __attribute__((__packed__)) {
    union {
        uint8_t bl:1,
                br:1,
                bm:1,
                ao:1,
                xs:1,
                ys:1,
                xo:1,
                yo:1;
        uint8_t b1;
    };
    int8_t xm;
    int8_t ym;
} MouseEvent;

int main(){
    MouseEvent event;
    int x = 0;
    int y = 0;

    int fd = open("mouse0:",O_RDONLY);

    while(1){
        read(fd,&event, sizeof(MouseEvent));

        x += event.xm ;//* (event.xs ? 1 : -1);
        y += event.ym ;//* (event.ys ? 1 : -1);

        printf("%d %d - %d %d : (%d %d) --- (%02x) \n", 
            event.xm, event.xs, 
            event.ym, event.ys, 
            x,
            y,
            event.b1);
    }
}
