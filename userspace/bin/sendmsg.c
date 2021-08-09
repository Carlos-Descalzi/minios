#include "stdio.h"
#include "msg.h"
#include "stdlib.h"
#include "unistd.h"
/**
 * Simple program to test message passing, sends
 * a fixed message to a process identified by the
 * task id passed as parameter
 **/

int main(int argc, char** argv){
    Message message;

    if (argc < 2){
        return 0;
    }

    int target = atoi(argv[1]);

    message.header.source = getpid();
    message.header.target = target;
    message.header.number = 0;
    message.header.has_more = 0;
    sprintf(message.body, "HELLO FROM %d", getpid());
    printf("Sending message to %d\n",target);

    if (!msg_send_sync(&message)){
        printf("Answer from %d: \"%s\"\n", message.header.source, message.body);
    } else {
        printf("No message received\n");
    }

    return 0;
}

