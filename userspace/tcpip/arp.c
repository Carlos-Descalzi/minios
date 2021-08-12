/**
 * Test code for eth and ip stack
 **/
#include "tcpip.h"
#include "string.h"
#include "arp.h"
#include "minios.h"
#include "cache.h"
#include "fcntl.h"
#include "unistd.h"
#include "eth.h"

static void arp_set             (EthFrame* frame, uint16_t proto_type, uint16_t opcode, 
                                const uint8_t* target_hw_address, const IpAddress target_proto_address);
static void arp_answer_request  (EthFrame* frame, uint8_t* tx_buffer);
static void arp_handle_reply    (EthFrame* frame, uint8_t* tx_buffer);

void arp_handle_packet(EthFrame* frame, uint8_t* tx_buffer){
    switch(htons2(frame->arp_packet.opcode)){
        case ARP_REQUEST:
            log("\t\tARP Request:\n");

            if (frame->arp_packet.target_proto_addr.l_address == ip_address.l_address){
                arp_answer_request(frame, tx_buffer);
            }
            break;
        case ARP_REPLY:
            log("\t\tARP Reply\n");
            arp_handle_reply(frame, tx_buffer);
            break;
    }

}


void arp_announce(uint8_t* tx_buffer){

    log("Announcing my IP\n");

    EthFrame* eth_frame = (EthFrame*) tx_buffer;

    eth_set(
        eth_frame, 
        ETHER_TYPE_ARP, 
        broadcast_mac
    );

    arp_set(
        eth_frame,
        ARP_PROTO_TYPE_IPV4,
        ARP_REQUEST,
        null_mac,
        ip_address
    );

    write(eth_fd, eth_frame, ARP_PACKET_SIZE);
}

static void arp_handle_reply (EthFrame* frame, uint8_t* tx_buffer){
    // TODO
    log("ARP reply: sender IP");
    print_ip(frame->arp_packet.sender_proto_addr);
    log("\n\tSender MAC:");
    print_mac(frame->arp_packet.sender_hw_address);
    log("\n");
    store_mac_for_ip(
        frame->arp_packet.sender_proto_addr, 
        frame->arp_packet.sender_hw_address
    );
}

void arp_query(IpAddress ip, uint8_t* tx_buffer){
    log("Doing ARP query for ip address\n");

    EthFrame* eth_frame = (EthFrame*) tx_buffer;

    eth_set(
        eth_frame,
        ETHER_TYPE_ARP,
        broadcast_mac
    );

    arp_set(
        eth_frame,
        ARP_PROTO_TYPE_IPV4,
        ARP_REQUEST,
        null_mac,
        ip
    );

    write(eth_fd, eth_frame, ARP_PACKET_SIZE);
}

static void arp_answer_request(EthFrame* frame, uint8_t* tx_buffer){

    log("\t\tAnswering ARP request\n");

    EthFrame* answer = (EthFrame*) tx_buffer;

    eth_set(
        answer,
        ETHER_TYPE_ARP,
        frame->source_mac
    );

    arp_set(
        answer,
        frame->arp_packet.proto_type,
        ARP_REPLY,
        frame->arp_packet.sender_hw_address,
        frame->arp_packet.sender_proto_addr
    );

    write(eth_fd, answer, ARP_PACKET_SIZE);
}

void arp_set(
        EthFrame* frame, 
        uint16_t proto_type, 
        uint16_t opcode, 
        const uint8_t* target_hw_address,
        const IpAddress target_proto_address){

    frame->arp_packet.proto_type = proto_type;
    frame->arp_packet.proto_addr_len = 4;
    frame->arp_packet.opcode = htons2(opcode);

    memcpy(frame->arp_packet.sender_hw_address, device_mac,6);
    frame->arp_packet.sender_proto_addr = ip_address;
    memcpy(frame->arp_packet.target_hw_address, target_hw_address, 6);
    frame->arp_packet.target_proto_addr = target_proto_address;

}

