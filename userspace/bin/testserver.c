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
            strcpy(buff,"Hello from server! replying to \"");
            strcat(buff,message.body);
            strcat(buff,"\"");
            /*FIXME sprintf*/
            /*sprintf(
                buff,
                "Received message from %d %d saying \"%s\"", 
                message.source,
                message.target,
                message.body
            );*/
            strcpy(message.body, buff);
            uint32_t t = message.source;
            message.source = message.target;
            message.target = t;
            msg_send(&message);
        }
        sched_yield();
    }

    return 0;
}
