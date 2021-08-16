#include "tcpip.h"
#include "stdio.h"
#include "sched.h"
#include "minios.h"

int main(int argc, char** argv){

    if (read_config() < 0){
        return 1;
    }

    if (stack_init() < 0){
        return 1;
    }

    WaitCondition conditions[2];

    conditions[0].cond_type = COND_TYPE_FD;
    conditions[0].fd = eth_fd;
    conditions[1].cond_type = COND_TYPE_MSG;

    while (1){
        sched_wait_conditions(2, conditions);

        stack_loop();

        if (handle_user_message()){
            printf("Exiting \n");
            break;
        }
    }

    return 0;
}
