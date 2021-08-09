#include "socket.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"

char buffer[4096];

int main(int argc, char**argv){
    uint32_t remote_address;
    uint16_t remote_port;
    size_t size;

    if (argc < 2){
        printf("Missing pid\n");
        return 1;
    }

    int pid = atoi(argv[1]);

    printf("Init sockets %d\n", pid);
    socket_init(pid);

    int socket = socket_server_udp_open(67);
    if (socket <= 0){
        printf("Unable to open socket\n");
        return 1;
    }
    printf("Opened socket %x\n", socket);

    socket_server_udp_recv(socket, &remote_address, &remote_port, buffer, &size);
    printf("Received data \"%s\"\n",buffer);

    socket_close(socket);
    printf("Socket closed\n");

    return 0;
}
