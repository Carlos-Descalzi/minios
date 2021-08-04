#ifndef _NET_H_
#define _NET_H_

#include "stdint.h"

typedef struct __attribute__((__packed__)){
    uint16_t    hw_type;
    uint16_t    proto_type;
    uint8_t     hw_addr_len;
    uint8_t     proto_addr_len;
    uint16_t    opcode;
    uint8_t     sender_hw_address[6];
    uint8_t     sender_proto_addr[4];
    uint8_t     target_hw_address[6];
    uint8_t     target_proto_addr[4];
} ArpPacket;

typedef struct __attribute__((__packed__)){
    uint8_t     version;
    uint8_t     service_type;
    uint16_t    total_len;
    uint16_t    identifier;
    uint16_t    flags:3,
                fragment_offset:13;
    uint8_t     ttl;
    uint8_t     protocol;
    uint16_t    checksum;
    uint8_t     source_address[4];
    uint8_t     target_address[4];
} Ipv4Header;

typedef struct __attribute__((__packed__)){
    uint16_t    source_port;
    uint16_t    target_port;
    uint16_t    length;
    uint16_t    checksum;
} UdpHeader;

typedef struct {
    Ipv4Header ipv4;
    UdpHeader udp;
    char payload[];
} Ipv4Packet;

typedef struct __attribute__((__packed__)){
    uint8_t target_mac[6];
    uint8_t source_mac[6];
    uint16_t ether_type;
    union {
        ArpPacket arp_packet;
        Ipv4Packet ipv4_packet;
    };
} EthFrame;

#define ARP_PACKET_SIZE     (12+sizeof(ArpPacket))
#define IPV4_PACKET_SIZE    (12+sizeof(Ipv4Packet))

#define ETHER_TYPE_ARP      0x0806
#define ETHER_TYPE_IP       0x0800

#define ARP_REQUEST         0x0001
#define ARP_REPLY           0x0002
#define ARP_HW_ETH          0x0001
#define ARP_PROTO_TYPE_IPV4 0x0800

#define IPV4_TYPE_UDP       0x1100

#define htons2(v)           ((((v) & 0xFF00) >> 8) | (((v) & 0xFF)<<8))

#endif
