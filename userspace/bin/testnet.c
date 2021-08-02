/**
 * Test code for eth and ip stack
 **/
#include "stdio.h"
#include "fcntl.h"
#include "stdint.h"
#include "string.h"
#include "net.h"
#include "netutils.h"
#include "sched.h"
#include "msg.h"

/*

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


*/

char rx_buffer[8192];
char tx_buffer[8192];
char my_ip[] = {192,168,76,9};
uint8_t my_mac[] = {0x32,0x2F,0x67,0x52,0xAB,0xBD};

static void print_mac(uint8_t* mac){
    printf ("%02x:%02x:%02x:%02x:%02x:%02x", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void print_ip(char* ip){
    printf("%u.%u.%u.%u", 
        ip[0] & 0xFF, ip[1] & 0xFF , ip[2] & 0xFF, ip[3] & 0xFF);
}

static void print_arp_packet(EthFrame* frame){
    printf("\t\tSender proto address:");
    print_ip(frame->arp_packet.sender_proto_addr);
    printf("\n");
    printf("\t\tSender hw address:");
    print_mac(frame->arp_packet.sender_hw_address);
    printf("\n");
    printf("\t\tTarget proto address:");
    print_ip(frame->arp_packet.target_proto_addr);
    printf("\n");
    printf("\t\tTarget hw address:");
    print_mac(frame->arp_packet.target_hw_address);
    printf("\n");
}

static void answer_arp_request(int fd, EthFrame* frame){

    printf("\t\tAnswering ARP request\n");

    EthFrame* answer = (EthFrame*) tx_buffer;
    memcpy(answer->source_mac, my_mac, 6);
    memcpy(answer->target_mac, frame->source_mac, 6);
    answer->ether_type = htons2(ETHER_TYPE_ARP);
    answer->arp_packet.hw_type = htons2(ARP_HW_ETH);
    answer->arp_packet.proto_type = frame->arp_packet.proto_type;
    answer->arp_packet.proto_addr_len = frame->arp_packet.proto_addr_len;
    answer->arp_packet.opcode = htons2(ARP_REPLY);

    memcpy(answer->arp_packet.sender_hw_address, my_mac,6);
    memcpy(answer->arp_packet.sender_proto_addr, my_ip, 4);
    memcpy(answer->arp_packet.target_hw_address, frame->arp_packet.sender_hw_address, 6);
    memcpy(answer->arp_packet.target_proto_addr, frame->arp_packet.sender_proto_addr, 4);

    write(fd, answer, ARP_PACKET_SIZE);
}

static void make_arp_announcement(int fd){
    EthFrame* eth_frame = (EthFrame*) tx_buffer;

    eth_frame->ether_type = htons2(ETHER_TYPE_ARP);
    eth_frame->arp_packet.hw_type = htons2(ARP_HW_ETH);
    eth_frame->arp_packet.proto_type = htons2(ARP_PROTO_TYPE_IPV4);
    eth_frame->arp_packet.hw_addr_len = 6;
    eth_frame->arp_packet.proto_addr_len = 4;
    eth_frame->arp_packet.opcode = htons2(ARP_REQUEST);
    memcpy(eth_frame->arp_packet.sender_hw_address, my_mac, 6);
    memcpy(eth_frame->arp_packet.sender_proto_addr, my_ip, 4);
    memset(eth_frame->arp_packet.target_hw_address, 0, 6);
    memcpy(eth_frame->arp_packet.target_proto_addr, my_ip, 4);

    write(fd, &tx_buffer, ARP_PACKET_SIZE);
}

static void handle_arp_request(int fd, EthFrame* frame){
    switch(htons2(frame->arp_packet.opcode)){
        case ARP_REQUEST:
            printf("\t\tARP Request:\n");
            print_arp_packet(frame);

            if (!memcmp(frame->arp_packet.target_proto_addr, my_ip, 4)){
                answer_arp_request(fd, frame);
            }
            break;
        case ARP_REPLY:
            printf("\t\tARP Reply\n");
            break;
    }

}

static void handle_ipv4_packet(int fd, EthFrame* frame){
    printf("\t\tSource address: ");
    print_ip(frame->ipv4_packet.ipv4.source_address);
    printf("\n");
    switch(htons2(frame->ipv4_packet.ipv4.protocol)){
        case IPV4_TYPE_UDP:
            printf("\t\tUDP packet:\n");
            printf("\t\tSource port: %d\n", htons2(frame->ipv4_packet.udp.source_port));
            printf("\t\tTarget port: %d\n", htons2(frame->ipv4_packet.udp.target_port));
            printf("\t\tPayload: %s\n", frame->ipv4_packet.payload);
            break;
        default:
            printf("\t\tUnknown IPv4 protocol %x\n", frame->ipv4_packet.ipv4.protocol);
    }
}

static void handle_user_message(Message* message){
}

int main(int argc, char** argv){

    Message message;

    int fd = open("net0:", O_RDWR | O_NONBLOCK);

    if (fd < 0){
        printf("Unable to open device\n");
        return 1;
    }

    make_arp_announcement(fd);

    while (1){
        memset(rx_buffer, 0, 8192);
        int received = read(fd, rx_buffer, 8192);

        if (received > 0){
            printf("Received %d bytes\n", received);

            EthFrame* eth_frame = (EthFrame*) rx_buffer;
            printf("\tsource:");
            print_mac(eth_frame->source_mac);
            printf("\n\ttarget:");
            print_mac(eth_frame->target_mac);
            printf("\n");

            switch(htons2(eth_frame->ether_type)){
                case ETHER_TYPE_ARP:
                    printf("\tARP packet\n");
                    handle_arp_request(fd, eth_frame);
                    break;
                case ETHER_TYPE_IP:
                    printf("\tIPv4 packet\n");
                    handle_ipv4_packet(fd, eth_frame);
                    break;
                default:
                    printf("\tUnknown packet type %x\n", eth_frame->ether_type);
                    break;
            }
        }

        if (!msg_recv(&message)){
            printf("Received user request\n");
            handle_user_message(&message);
        }

        sched_yield();
    }

    return 0;
}
