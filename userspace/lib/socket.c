#include "socket.h"
#include "stdlib.h"
#include "unistd.h"
#include "tcpipd.h"
#include "minios.h"
#include "string.h"

static pid_t        service_pid = 0;
static UserMessage  message;

static pid_t    get_tcpipd_pid  (void);
static int      socket_open     (uint16_t type, uint16_t port);

int socket_init (pid_t _service_pid){

    if (!_service_pid){
        service_pid = get_tcpipd_pid();
    } else {
        service_pid = _service_pid;
    }
    return 0;
}

int socket_client_udp_open (){
    return socket_open(SOCKET_TYPE_UDP, 0);
}

int socket_server_udp_open (uint16_t port){
    return socket_open(SOCKET_TYPE_UDP |SOCKET_TYPE_SERVER, port);
}

static void prepare_message (void){
    memset(&message,0,sizeof(UserMessage));
    message.header.source = getpid();
    message.header.target = service_pid;
}

int socket_server_tcp_accept (int sockd, uint32_t* address, uint16_t* port){

    if (sockd <= 0){
        return -1;
    }

    if (!service_pid){
        return -1;
    }

    prepare_message();
    message.type = MSG_ACCEPT;
    message.accept_request.socket_type = sockd >> 16;
    message.accept_request.port = sockd & 0xFFFF;

    msg_send_sync(MESSAGE(&message));

    if (message.accept_response.result <= 0){
        return message.accept_response.result;
    }

    int new_sock_d = message.accept_response.socket_type << 16 | message.accept_response.port;

    *address = message.accept_response.remote_address;
    *port = message.accept_response.remote_port;

    return new_sock_d;
}

void socket_close (int sockd){

    if (sockd <= 0){
        debug("1\n");
        return;
    }

    if (!service_pid){
        debug("2   %d \n",service_pid);
        debug("pos1 : %x\n",&service_pid);
        debug("pos 2: %x\n",&message);
        return;
    }
    debug("Closing socket %d\n",sockd);

    uint16_t socket_type = sockd >> 16;
    uint16_t port = sockd & 0xFFFF;

    prepare_message();
    message.type = MSG_CLOSE;
    message.close_request.socket_type = socket_type;
    message.close_request.port = port;

    msg_send(MESSAGE(&message));
}

int socket_server_udp_recv (int sockd, uint32_t* address, uint16_t* port, void* data, size_t size){

    if (sockd <= 0){
        return -1;
    }
    if (!service_pid){
        return -1;
    }

    prepare_message();
    message.type = MSG_RECV;
    message.recv_request.socket_type = sockd >> 16;
    message.recv_request.port = sockd & 0xFFFF;
    
    if (msg_send_sync(MESSAGE(&message)) < 0){
        debug("recv failed\n");
        return -1;
    }
    printf("**** Received response \n");
    int pos = 0;

    *address = message.recv_header_response.remote_address;
    *port = message.recv_header_response.remote_port;

    memcpy(data, message.recv_header_response.payload, message.recv_header_response.size);
    pos+=message.recv_header_response.size;

    while(message.header.has_more && pos < size){
        printf("Response - more parts %d %d\n",pos,size);
        msg_recv_wait(MESSAGE(&message));

        int to_copy = min(message.recv_response.size, size - pos);
        memcpy(data + pos, message.recv_response.payload, to_copy);
        pos+= to_copy;
        printf("Response - more parts 2 - %d %d\n",pos,size);
    }

    return pos;
}

static pid_t get_tcpipd_pid(){
    char* pid = getenv("TCPIPD");

    if (pid){
        return atoi(pid);
    }

    return 0;
}

static int socket_open(uint16_t type, uint16_t port){
    debug("Opening socket at port %d\n",port);

    if (!service_pid){
        debug("12\n");
        return -1;
    }

    prepare_message();
    message.type = MSG_OPEN;
    message.open_request.socket_type = type;
    message.open_request.port = port;

    msg_send_sync(MESSAGE(&message));

    if (message.type == MSG_RESPONSE
        && message.int_response == 0){
        return SOCKET_TYPE_UDP << 16 | port;
    }

    return -1;
}
