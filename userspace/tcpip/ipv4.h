#ifndef _IPV4_H_
#define _IPV4_H_

#include "tcpip.h"

void ipv4_init              (void);
void ipv4_handle_packet     (int fd, EthFrame* frame);
int  ipv4_udp_socket_close  (int pid, int port);
int  ipv4_tcp_socket_close  (int pid, int port);
int  ipv4_udp_socket_open   (int pid, int server, int port);
int  ipv4_tcp_socket_open   (int pid, int server, int port);
int  ipv4_tcp_socket_accept (int pid, int port);
#endif
