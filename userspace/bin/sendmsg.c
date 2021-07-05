#include "stdio.h"
#include "msg.h"

int main(int argc, char** argv){
    Message message;

    if (argc == 0){
        return 0;
    }

    int target = atoi(argv[0]);

    message.source = getpid();
    message.target = target;
    message.number = 0;
    message.has_more = 0;
    sprintf(message.body, "HELLO FROM %d", getpid());
    printf("Sending message to %d\n",target);

    if (!msg_send_sync(&message)){
        printf("Answer from %d: \"%s\"\n", message.source, message.body);
    } else {
        printf("No message received\n");
    }

    return 0;
}

