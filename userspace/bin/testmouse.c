#include "fcntl.h"
#include "stdio.h"
#include "stdint.h"
#include "unistd.h"
#include "sched.h"
#include "mouse.h"

int main(){
    MouseState state;
    int x = 0;
    int y = 0;

    int fd = open("mouse0:",O_RDONLY | O_NONBLOCK);

    while(1){
        int result = read(fd,&state, sizeof(MouseState));
        if (result){

            x += state.xm;
            y += state.ym;

            printf("%d %d %d\n",
                x,
                y,
                state.bl);
        } else {
            sched_yield();
        }
    }
}
