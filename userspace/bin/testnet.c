/**
 * Test code for eth and ip stack
 **/
#include "stdio.h"
#include "fcntl.h"
#include "stdint.h"
#include "string.h"

typedef struct __attribute__((__packed__)){
    uint16_t    hw_type;
    uint16_t    proto_type;
    uint8_t     hw_addr_len;
    uint8_t     proto_addr_len;
    uint16_t    opcode;
    uint8_t     sender_hw_address[6];
    uint8_t     sender_proto_addr[4];
    uint8_t     target_hw_address[6];
    uint16_t    target_proto_addr[4];
} Arp;

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
} Ipv4;

typedef struct __attribute__((__packed__)){
    uint16_t    source_port;
    uint16_t    target_port;
    uint16_t    length;
    uint16_t    checksum;
} Udp;

typedef struct __attribute__((__packed__)){
    uint8_t     op_code;
    uint8_t     hw_type;
    uint8_t     hw_addr_len;
    uint8_t     hops;
    uint32_t    tx_id;
    uint16_t    seconds;
    uint16_t    flags;
    uint8_t     ciaddr[4];
    uint8_t     yiaddr[4];
    uint8_t     siaddr[4];
    uint8_t     giaddr[4];
    uint8_t     chaddr[16];
    uint8_t     bootp[192];
    uint32_t    cookie;
} Dhcp;

typedef struct {
    uint16_t ether_type;
    Arp arp;
} ArpPacket;

typedef struct {
    uint16_t ether_type;
    Ipv4 ipv4;
    Udp udp;
    char payload[];
} Ipv4Packet;

typedef union {
    uint16_t ether_type;
    ArpPacket arp_packet;
    Ipv4Packet ipv4_packet;
} EthFrame;

typedef struct {
    uint16_t ether_type;
    Ipv4 ipv4_header;
    Udp udp_header;
    Dhcp dhcp;
} DhcpPacket;

#define htons2(v)    (((v & 0xFF00) >> 8) | ((v & 0xFF)<<8))

#define ETHER_TYPE_ARP  0x0806
#define ETHER_TYPE_IP   0x0800

#define IPV4_TYPE_UDP   0x11

#define DHCP_PACKET_LEN (sizeof(Ipv4)+sizeof(Udp)+sizeof(Dhcp))

static uint16_t calc_ipv4_checmsum(void* header, int len){
    uint32_t checksum = 0;

    for (int i=0;i<len/2;i++){
        checksum += ((uint16_t*)header)[i];
    }
    checksum += checksum >> 16;
    return ~checksum & 0xFFFF;
}

static void read_mac(char* mac){
    int fd = open("net0:/self",O_RDONLY);
    read(fd, mac,6);
    close(fd);
    printf("MAC address:");
    for (int i=0;i<6;i++){
        if (i>0) printf(":");
        printf("%2x",mac[i]);
    }
    printf("\n");
}

#define ARP_REQUEST         0x0001
#define ARP_REPLY           0x0002
#define ARP_HW_ETH          0x0001
#define ARP_PROTO_TYPE_IPV4 0x0800

static void make_arp_announcement(char* source_mac, char* ip, int fd){
    ArpPacket arp;

    arp.ether_type = htons2(ETHER_TYPE_ARP);
    arp.arp.hw_type = htons2(ARP_HW_ETH);
    arp.arp.proto_type = htons2(ARP_PROTO_TYPE_IPV4);
    arp.arp.hw_addr_len = 6;
    arp.arp.proto_addr_len = 4;
    arp.arp.opcode = htons2(ARP_REQUEST);
    memcpy(arp.arp.sender_hw_address, source_mac, 6);
    memcpy(arp.arp.sender_proto_addr, ip, 4);
    memcpy(arp.arp.target_proto_addr, ip, 4);
    memset(arp.arp.target_hw_address, 0, 6);

    write(fd, &arp, sizeof(ArpPacket));
}
ArpPacket arp;
EthFrame eth_frame;

static void answer_arp(int fd, char* source_mac, char* source_ip, EthFrame* frame){
    arp.ether_type = htons2(ETHER_TYPE_ARP);
    arp.arp.hw_type = htons2(ARP_HW_ETH);
    arp.arp.proto_type = htons2(ARP_PROTO_TYPE_IPV4);
    arp.arp.hw_addr_len = 6;
    arp.arp.proto_addr_len = 4;
    arp.arp.opcode = htons2(ARP_REPLY);
    memcpy(arp.arp.sender_hw_address, source_mac, 6);
    memcpy(arp.arp.sender_proto_addr, source_ip,4);
    memcpy(arp.arp.target_hw_address, frame->arp_packet.arp.sender_hw_address, 6);
    memcpy(arp.arp.target_proto_addr, frame->arp_packet.arp.sender_proto_addr, 4);
    write(fd, &arp, sizeof(ArpPacket));
}

static void make_arp_query(char* source_mac, char* source_ip, char* dest_mac, char* dest_ip, int fd){

    arp.ether_type = htons2(ETHER_TYPE_ARP);
    arp.arp.hw_type = htons2(ARP_HW_ETH);
    arp.arp.proto_type = htons2(ARP_PROTO_TYPE_IPV4);
    arp.arp.hw_addr_len = 6;
    arp.arp.proto_addr_len = 4;
    arp.arp.opcode = htons2(ARP_REQUEST);
    memcpy(arp.arp.sender_hw_address, source_mac, 6);
    memcpy(arp.arp.sender_proto_addr, source_ip,4);
    memset(arp.arp.target_hw_address,0, 6);
    memcpy(arp.arp.target_proto_addr, dest_ip, 4);

    write(fd, &arp, sizeof(ArpPacket));

    memset(&arp, 0, sizeof(ArpPacket));

    while(1){
        int result = read(fd, &eth_frame,sizeof(EthFrame));

        printf("Received packet of type: %04x, result: %d\n",htons2(eth_frame.ether_type),result);

        if (eth_frame.ether_type == htons2(ETHER_TYPE_IP)){
            printf("\tReceived IP packet\n");
            if (eth_frame.ipv4_packet.ipv4.protocol == IPV4_TYPE_UDP){
                printf("\t  Received UDP packet, payload: %s\n",
                        eth_frame.ipv4_packet.payload);
            } else {
                printf("\t  Packet of protocol %04x\n", eth_frame.ipv4_packet.ipv4.protocol);
            }

        } else if (eth_frame.ether_type == htons2(ETHER_TYPE_ARP)){
            printf("\tReceived ARP packet\n");
            if (eth_frame.arp_packet.arp.opcode == htons2(ARP_REQUEST)){
                printf("\t  Answering ARP\n");
                answer_arp(fd, source_mac, source_ip, &eth_frame);
            } else {
                printf("\t  ARP opcode: %04x\n",eth_frame.arp_packet.arp.opcode);
            }
        } else {
            printf("Recevied packet %04x\n", htons2(eth_frame.ether_type));
        }
        printf("Done\n");
    }
}

static char ip_addr[] = {192,168,102,3};
char dest_ip[] = {192,168,100,4};
char gateway_ip[] = {192,168,102,1};
char message[] = "HOLA!\n";
char udp_message[256];

static void send_udp_message(int fd){

    Ipv4Packet* packet = (Ipv4Packet*) udp_message;

    packet->ether_type = htons2(ETHER_TYPE_IP);

    packet->ipv4.version = 0x45;
    packet->ipv4.service_type = 0x0;
    packet->ipv4.total_len = htons2(sizeof(Ipv4) + sizeof(Udp) + strlen(message) +1);
    packet->ipv4.ttl = 10;
    packet->ipv4.flags = 0;
    packet->ipv4.fragment_offset = 0;
    packet->ipv4.protocol = IPV4_TYPE_UDP;
    memcpy(packet->ipv4.source_address, ip_addr, 4);
    memcpy(packet->ipv4.target_address, dest_ip, 4);
    packet->ipv4.checksum = calc_ipv4_checmsum(&(packet->ipv4), sizeof(Ipv4));

    packet->udp.source_port = htons2(1025);
    packet->udp.target_port = htons2(8888);
    packet->udp.length = htons2(sizeof(Udp) + strlen(message) +1);
    strcpy(packet->payload,message);
    packet->udp.checksum = calc_ipv4_checmsum(&(packet->udp), sizeof(Udp) + strlen(message)+1);

    write(fd, packet, sizeof(Ipv4Packet) + strlen(message) + 1);
}

static char dest_mac[] = {0x1c, 0x39, 0x47, 0xb0, 0x39, 0x1f};

int main(int argc,char **argv){
    char mac[6];

    read_mac(mac);
    int fd = open("net0:/00:00:00:00:00:00", O_RDWR| O_NONBLOCK);

    if (fd < 0){
        printf("Unable to open device\n");
        return 1;
    }

    make_arp_announcement(mac, ip_addr, fd);
    //make_arp_query(mac, ip_addr, NULL, dest_ip, fd);
    printf("Closing\n");
    close(fd);
    printf("Closed\n");
    /*
    int fd = open(dest_addr, O_RDWR | O_NONBLOCK);

    if (fd < 0){
        printf("Unable to open device\n");
    }
    send_udp_message(fd);

    close(fd);
    */
    return 0;
}
