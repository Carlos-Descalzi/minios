#include "stdio.h"
#include "msg.h"
#include "sched.h"
#include "string.h"
/**
 * This server is used for testing message passing.
 * Receives a message from another task task and answers it
 **/

int main(int argc,char **argv){
    char buff[1200];
    Message message;

    while(1){
        if (!msg_recv(&message)){
            sprintf(
                buff,
                "Received message from %d %d saying \"%s\"", 
                message.header.source,
                message.header.target,
                message.body
            );
            strcpy(message.body, buff);
            uint32_t t = message.header.source;
            message.header.source = message.header.target;
            message.header.target = t;
            msg_send(&message);
        }
        sched_yield();
    }

    return 0;
}
