/**
 * Test code for eth and ip stack
 **/
#include "stdio.h"
#include "fcntl.h"
#include "stdint.h"
#include "string.h"
#include "net.h"
#include "netutils.h"


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
    packet->ipv4.total_len = htons2(sizeof(Ipv4Header) + sizeof(UdpHeader) + strlen(message) +1);
    packet->ipv4.ttl = 10;
    packet->ipv4.flags = 0;
    packet->ipv4.fragment_offset = 0;
    packet->ipv4.protocol = IPV4_TYPE_UDP;
    memcpy(packet->ipv4.source_address, ip_addr, 4);
    memcpy(packet->ipv4.target_address, dest_ip, 4);
    packet->ipv4.checksum = ipv4_checmsum(&(packet->ipv4), sizeof(Ipv4Header));

    packet->udp.source_port = htons2(1025);
    packet->udp.target_port = htons2(8888);
    packet->udp.length = htons2(sizeof(UdpHeader) + strlen(message) +1);
    strcpy(packet->payload,message);
    packet->udp.checksum = ipv4_checmsum(&(packet->udp), sizeof(UdpHeader) + strlen(message)+1);

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
