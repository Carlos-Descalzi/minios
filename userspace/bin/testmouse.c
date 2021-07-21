#include "fcntl.h"
#include "stdio.h"
#include "stdint.h"
#include "unistd.h"
#include "sched.h"

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

    int fd = open("mouse0:",O_RDONLY | O_NONBLOCK);

    while(1){
        int result = read(fd,&event, sizeof(MouseEvent));
        if (result){

            x += event.xm ;
            y += event.ym ;

            printf("%d %d %d\n",
                x,
                y,
                event.bl);
        } else {
            sched_yield();
        }
    }
}
