#include "tcpip.h"
#include "unistd.h"
#include "tcpipd.h"
#include "minios.h"
#include "string.h"

static void handle_open     (UserMessage* message);
static void handle_close    (UserMessage* message);
static void handle_accept   (UserMessage* message);
static void handle_recv     (UserMessage* message);


static UserMessage message;
static UserMessage answer;

int handle_user_message(){

    memset(&message, 0, sizeof(UserMessage));

    if (!msg_recv(MESSAGE(&message))){
        debug("Received user message from %d\n",message.header.source);
        switch(message.type){
            case MSG_OPEN:
                handle_open(&message);
                break;
            case MSG_CLOSE:
                handle_close(&message);
                break;
            case MSG_LISTEN:
                break;
            case MSG_ACCEPT:
                handle_accept(&message);
                break;
            case MSG_RECV:
                handle_recv(&message);
                break;
            case MSG_SEND:
                break;

        }
    }
    return 0;
}

static void handle_open(UserMessage* message){
    debug("Open message\n");

    
    int result = stack_open_socket(
        message->header.source,
        message->open_request.socket_type, 
        message->open_request.port
    );
    debug("Answer open message\n");

    UserMessage answer = {
        .header = {
            .source = getpid(),
            .target = message->header.source
        },
        .type = MSG_RESPONSE,
        .int_response = result
    };
    debug("Answer: %d - %d\n",answer.header.source, answer.header.target);

    msg_send(MESSAGE(&answer));
    debug("Message answered\n");
}

static void handle_close (UserMessage* message){
    debug("Close message\n");
    int result = stack_close_socket(
        message->header.source,
        message->close_request.socket_type, 
        message->close_request.port
    );
    debug("Socket closed\n");
}

static void handle_accept(UserMessage* message){
    stack_socket_accept(
        message->header.source,
        message->accept_request.socket_type,
        message->accept_request.port
    );
}
static void handle_recv (UserMessage* message){
    stack_socket_receive(
        message->header.source,
        message->recv_request.socket_type,
        message->recv_request.port
    );
}
void udp_received (int pid, int socket_type, uint16_t port,
    uint32_t remote_address,
    uint16_t remote_port,
    uint16_t length,
    uint8_t* payload){

    message.header.source = getpid();
    message.header.target = pid;
    message.type = MSG_RECV;
    message.recv_header_response.remote_address = remote_address;
    message.recv_header_response.remote_port = remote_port;

    message.recv_header_response.size = length;

    memcpy(message.recv_header_response.payload, payload, length);

    msg_send(MESSAGE(&message));

}
