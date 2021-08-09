#include "socket.h"

int main(int argc, char**argv){

    int socket = socket_client_udp_open();

    socket_close(socket);

    return 0;
}
