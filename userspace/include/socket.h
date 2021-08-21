#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "sys/types.h"
#include "stdint.h"

#define mk_ip_addr(a,b,c,d)         ((uint32_t)((a << 24) | (b << 16) | (c << 8) | d))

int     socket_init                 (pid_t service_pid);

int     socket_client_udp_open      (void);
int     socket_server_udp_open      (uint16_t port);
int     socket_server_udp_recv      (int sockd, uint32_t* address, uint16_t* port, void* data, size_t size);
int     socket_client_udp_send      (int sockd, uint32_t address, uint16_t port, void* data, size_t size);

int     socket_client_tcp_open      (uint32_t address, uint16_t port);
int     socket_server_tcp_open      (uint16_t port);
int     socket_server_tcp_accept    (int sockd, uint32_t* address, uint16_t* port);

void    socket_close                (int sockd);

#endif
