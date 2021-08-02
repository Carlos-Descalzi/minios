#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "stdint.h"

// domain
#define AF_INET     0x03

// type
#define SOCK_DGRAM  0x02

// addresses
#define INADDR_ANY  0x00000000

struct sockaddr {
};

struct in_addr {
    uint32_t s_addr;
};

struct sockaddr_in {
    int16_t         sin_family;
    uint16_t        sin_port;
    struct in_addr  sin_addr;
    char            sin_zero[8];
};

typedef uint32_t socklen_t;

int     socket      (int domain, int type, int protocol);

int     connect     (int sockfd, const struct sockaddr* addr, socklen_t addrlen);

int     inet_aton   (const char* cp, struct in_addr* in_addr);

ssize_t sendto      (int sockfd, const void* buf, size_t size, int flags,
                    const struct sockaddr* dest_addr, socklen_t addrlen);

#endif
