#ifndef _TCPIP_H_
#define _TCPIP_H_

#include "stdint.h"
#include "msg.h"
#include "minios.h"
#include "proto.h"

#define log(...)                {debug(__VA_ARGS__);}

#define BUFFER_SIZE             8192
#define MAX_SOCKETS             65536

#define IOCTL_NET_MAC           1

#define SOCKET_TYPE_UDP     0x01
#define SOCKET_TYPE_TCP     0x02
#define SOCKET_TYPE_SERVER  0x10

#define mk_ip_addr(a,b,c,d)     ((uint32_t)((a << 24) | (b << 16) | (c << 8) | d))
#define ip_caddr_to_l(c)        mk_ip_addr(c[0],c[1],c[2],c[3])
#define ip_laddr_to_c(l,c)      {c[0] = l & 0xFF;\
                                c[1] = (l >> 8) & 0xFF;\
                                c[2] = (l >> 16) & 0xFF;\
                                c[2] = (l >> 24) & 0xFF;}

#define SEND_STATUS_START       1
#define SEND_STATUS_CONT        2
#define SEND_STATUS_END         3

typedef int (*StackRequest)(void* data);

extern int          eth_fd;
extern char         network_device[];
extern IpAddress    ip_address;
extern IpAddress    mask;
extern IpAddress    gateway;
extern uint8_t      device_mac[];
extern uint8_t      gateway_mac[];
extern const uint8_t broadcast_mac[];
extern const uint8_t null_mac[];

int  read_config                    (void);
void print_mac                      (uint8_t* mac);
void print_ip                       (IpAddress ip);
void parse_ip                       (const char* ip_str, uint8_t* ip);
int  stack_init                     (void);
int  stack_loop                     (void);
int  stack_open_socket              (int pid, int socket_type, uint16_t port);
int  stack_close_socket             (int pid, int socket_type, uint16_t port);
int  stack_socket_accept            (int pid, int socket_type, uint16_t port);
int  stack_socket_receive           (int pid, int socket_type, uint16_t port);
int  handle_user_message            (void);
void udp_received                   (int pid, int socket_type, uint16_t port,
                                    uint32_t remote_address,
                                    uint16_t remote_port,
                                    uint16_t chunk_size,
                                    uint16_t status,
                                    uint8_t* payload);
int stack_socket_udp_send_prepare   (int pid, uint16_t port, 
                                    uint32_t remote_address, 
                                    uint16_t remote_port);
int stack_socket_udp_send_add       (int pid, uint16_t port,
                                    void* data, 
                                    uint16_t length);
int stack_socket_udp_send_commit    (int pid, uint16_t port);

void stack_add_request              (StackRequest request);

#endif
