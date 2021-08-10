/**
 * Test code for eth and ip stack
 **/
#include "tcpip.h"
#include "stdio.h"
#include "fcntl.h"
#include "stdint.h"
#include "string.h"
#include "netutils.h"
#include "sched.h"
#include "msg.h"
#include "unistd.h"
#include "stdlib.h"
#include "ioctl.h"
#include "minios.h"
#include "arp.h"
#include "ipv4.h"
#include "cache.h"

static uint8_t rx_buffer[BUFFER_SIZE];
static uint8_t tx_buffer[BUFFER_SIZE];

int stack_init (void){

    ipv4_init();
    cache_init();

    int fd = open(network_device, O_RDWR | O_NONBLOCK); 

    if (fd <= 0){
        log("Unable to open device %d\n", -fd);
        return -1;
    }

    ioctl(fd, IOCTL_NET_MAC, device_mac);

    log("Mac address   :");
    print_mac(device_mac);
    log("\n");

    arp_announce(fd, tx_buffer);

    return fd;
}

int stack_loop(int fd){
    memset(rx_buffer, 0, BUFFER_SIZE);
    int received = read(fd, rx_buffer, BUFFER_SIZE);

    if (received > 0){
        debug("Received %d bytes\n", received);

        EthFrame* eth_frame = (EthFrame*) rx_buffer;
        log("\tsource:");
        print_mac(eth_frame->source_mac);
        log("\n\ttarget:");
        print_mac(eth_frame->target_mac);
        log("\n");

        switch(htons2(eth_frame->ether_type)){
            case ETHER_TYPE_ARP:
                log("\tARP packet\n");
                arp_handle_packet(fd, eth_frame, tx_buffer);
                break;
            case ETHER_TYPE_IP:
                log("\tIPv4 packet\n");
                ipv4_handle_packet(fd, eth_frame);
                break;
            default:
                log("\tUnknown packet type %x\n", eth_frame->ether_type);
                break;
        }
    }

    return 0;
}


int stack_open_socket (int pid, int socket_type, uint16_t port){

    if (socket_type & SOCKET_TYPE_UDP){
        return ipv4_udp_socket_open(pid, socket_type & SOCKET_TYPE_SERVER, port);
    } else {
        return ipv4_tcp_socket_open(pid, socket_type & SOCKET_TYPE_SERVER, port);
    }

    return 0;
}

int stack_close_socket (int pid, int socket_type, uint16_t port){
    if (socket_type & SOCKET_TYPE_UDP){
        ipv4_udp_socket_close(pid, port);
    } else {
        ipv4_tcp_socket_close(pid, port);
    }
    return 0;
}

int stack_socket_accept (int pid, int socket_type, uint16_t port){
    if (socket_type & SOCKET_TYPE_TCP){
        return ipv4_tcp_socket_accept(pid, port);
    } 
    log("Cannot accept with socket type UDP\n");

    return -1;
}

int stack_socket_receive (int pid, int socket_type, uint16_t port){
    if (socket_type & SOCKET_TYPE_UDP){
        return ipv4_udp_socket_receive(pid, port);
    } else {
    }
    
    return 0;
}

int stack_socket_send(int pid, int socket_type, uint16_t port, void* data, uint16_t length){
    if (socket_type & SOCKET_TYPE_UDP){

    } else {

    }
}
