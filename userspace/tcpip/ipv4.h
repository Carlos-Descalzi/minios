#ifndef _IPV4_H_
#define _IPV4_H_

#include "tcpip.h"

void ipv4_init                      (void);
void ipv4_handle_packet             (EthFrame* frame);
int  ipv4_udp_socket_close          (int pid, int port);
int  ipv4_tcp_socket_close          (int pid, int port);
int  ipv4_udp_socket_open           (int pid, int server, int port);
int  ipv4_tcp_socket_open           (int pid, int server, int port);
int  ipv4_tcp_socket_accept         (int pid, int port);
int  ipv4_udp_socket_receive        (int pid, int port);

int  ipv4_socket_udp_send_prepare   (int pid, uint16_t port, 
                                    IpAddress remote_address, 
                                    uint16_t remote_port);

int ipv4_socket_udp_send_add        (int pid, uint16_t port,
                                    void* data, 
                                    uint16_t length);
int  ipv4_socket_udp_send_commit    (int pid, uint16_t port);
void ipv4_arp_address_resolved      (IpAddress* address, uin8_t* mac);
#endif
