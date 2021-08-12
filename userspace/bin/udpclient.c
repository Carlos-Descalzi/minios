#include "socket.h"
#include "stdio.h"
#include "stdlib.h"


char data[] = "hola\n";
int data_size = 5;

int main(int argc, char**argv){

    int pid = atoi(argv[1]);

    printf("Init sockets %d\n", pid);
    socket_init(pid);

    int socket = socket_client_udp_open();

    socket_client_udp_send(socket, mk_ip_addr(192,168,100,4), 8067, data, 5);

    socket_close(socket);

    return 0;
}
