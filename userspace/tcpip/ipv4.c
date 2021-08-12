#include "ipv4.h"
#include "stdlib.h"
#include "string.h"
#include "cache.h"
#include "buffer.h"

#define STATUS_ACTIVE           0x01
#define STATUS_SERVER           0x02
#define STATUS_ACCEPTING        0x04
#define STATUS_RECEIVING        0x08
#define STATUS_WAITING_MORE     0x10
#define STATUS_WAITING_ADDRESS  0x20
#define STATUS_ARP_RESOLVING    0x40


typedef struct {
    uint8_t status;
    pid_t pid;
    IpAddress remote_address;
    uint16_t port;
    Buffer rx_buffer;
    Buffer tx_buffer;
} UdpSocket;

typedef struct {
    uint8_t status;
    pid_t pid;
    uint16_t seq_number;
    uint16_t ack_number;
    IpAddress remote_address;
    uint16_t remote_port;
    uint8_t remote_mac[6];
    Buffer rx_buffer;
    Buffer tx_buffer;
} TcpSocket;

static UdpSocket udp_sockets[MAX_SOCKETS];
static TcpSocket tcp_sockets[MAX_SOCKETS];

static void handle_udp_packet   (Ipv4Packet* ipv4_packet);
static void handle_tcp_packet   (Ipv4Packet* ipv4_packet);
static int  allocate_tcp_socket (int pid);
static int  is_same_network     (IpAddress address);

void ipv4_init (void){
    memset(udp_sockets,0, sizeof(udp_sockets));
    memset(tcp_sockets,0, sizeof(tcp_sockets));
}

void ipv4_handle_packet(EthFrame* frame){

    log("\t\tIPv4 Packet\n\t\tSource address: ");
    print_ip(frame->ipv4_packet.ipv4.source_address);
    log("\n");

    store_mac_for_ip(frame->ipv4_packet.ipv4.source_address, frame->source_mac);

    switch(frame->ipv4_packet.ipv4.protocol){
        case IPV4_TYPE_UDP:
            handle_udp_packet(&(frame->ipv4_packet));
            break;
        case IPV4_TYPE_TCP:
            handle_tcp_packet(&(frame->ipv4_packet));
            break;

        default:
            log("\t\tUnknown IPv4 protocol %x\n", frame->ipv4_packet.ipv4.protocol);
    }

}

int ipv4_tcp_socket_close  (int pid, int port){
    if (tcp_sockets[port].status & STATUS_ACTIVE
        && tcp_sockets[port].pid == pid){
        log("Closing TCP socket %d used by pid %d\n", port, pid);

        tcp_sockets[port].status = 0;
        tcp_sockets[port].pid = 0;

        if (tcp_sockets[port].rx_buffer){
            buffer_free(&(tcp_sockets[port].rx_buffer));
        }
        if (tcp_sockets[port].tx_buffer){
            buffer_free(&(tcp_sockets[port].tx_buffer));
        }
    } else {
        log("TCP socket %d not active or invalid owner pid %d\n", port, pid)
    }
    return 0;
}

int ipv4_udp_socket_close  (int pid, int port){
    if (udp_sockets[port].status & STATUS_ACTIVE
        && udp_sockets[port].pid == pid){
        log("Closing UDP socket %d used by pid %d\n", port, pid);

        udp_sockets[port].status = 0;
        udp_sockets[port].pid = 0;

        if (udp_sockets[port].rx_buffer){
            buffer_free(&(udp_sockets[port].rx_buffer));
        }
        if (udp_sockets[port].tx_buffer){
            buffer_free(&(udp_sockets[port].tx_buffer));
        }
    } else {
        log("UDP socket %d not active or invalid owner pid %d\n", port, pid)
    }
    return 0;
}

int ipv4_tcp_socket_accept (int pid, int port){
    if (!(tcp_sockets[port].status & STATUS_ACTIVE)){
        log("TCP socket %d not active\n", port);
        return -1;
    }
    if (!(tcp_sockets[port].status & STATUS_SERVER)){
        log("TCP socket not set as server\n");
        return -1;
    }

    tcp_sockets[port].status |= STATUS_ACCEPTING;

    return 0;
}
int ipv4_udp_socket_open (int pid, int server, int port){
    if (udp_sockets[port].status & STATUS_ACTIVE){
        debug("UDP port %d already active\n", port);
        return -1;
    } else {
        debug("Opening UDP socket port %d for pid %d\n", port, pid);
        udp_sockets[port].status |= STATUS_ACTIVE;
        if (server){
            udp_sockets[port].status |= STATUS_SERVER;
            udp_sockets[port].rx_buffer = NULL;
            udp_sockets[port].rx_buffer_size = 0;
            udp_sockets[port].tx_buffer = NULL;
            udp_sockets[port].tx_buffer_size = 0;
        }
        udp_sockets[port].pid = pid;
    }
    return 0;
}

int ipv4_tcp_socket_open (int pid, int server, int port){
    if (tcp_sockets[port].status & STATUS_ACTIVE){
        debug("TCP port %d already active\n", port);
        return -1;
    } else {
        debug("Opening TCP socket port %d for pid %d\n", port, pid);
        tcp_sockets[port].status |= STATUS_ACTIVE;
        tcp_sockets[port].pid = pid;
        tcp_sockets[port].remote_port = 0;
        tcp_sockets[port].remote_address.l_address = 0;
        if (server){
            tcp_sockets[port].status |= STATUS_SERVER;
            tcp_sockets[port].buffer = NULL;
        } else {
            tcp_sockets[port].buffer = malloc(BUFFER_SIZE);
        }
    }
    return 0;
}

int ipv4_udp_socket_receive(int pid, int port){
    if (!(udp_sockets[port].status & STATUS_ACTIVE)){
        log("UDP socket %d not active\n", port);
        return -1;
    }
    if (!(udp_sockets[port].status & STATUS_SERVER)){
        log("UDP socket not server\n");
        return -1;
    }
    udp_sockets[port].status |= STATUS_RECEIVING;
    return 0;
}
/*
int ipv4_udp_socket_send (int pid, int port, 
    IpAddress remote_address,
    uint16_t remote_port,
    void* data,
    uint16_t length,
    int flush){

    if (!(udp_sockets[port].status & STATUS_ACTIVE)){
        log("UDP socket %d not active\n", port);
    }
    if (udp_sockets[port].status & STATUS_SERVER){
        log("Port %d This is a server UDP socket!", port);
    }
    if (!(udp_sockets[port].status & STATUS_SENDING)){
        udp_sockets[port].status |= STATUS_SENDING;
        udp_sockets[port].remote_address = remote_address;
        udp_sockets[port].remote_port = remote_port;
    }

    memcpy(udp_sockets[port].tx_buffer + udp_sockets[port].rx_index, data, length);

    


}*/

int ipv4_socket_udp_send_prepare (int pid, uint16_t port, 
    IpAddress remote_address, 
    uint16_t remote_port){

    udp_sockets[port].status |= STATUS_SENDING;
    udp_sockets[port].remote_address = remote_address;
    udp_sockets[port].remote_port = remote_port;

    buffer_free(&(udp_sockets[port].tx_buffer));
    udp_sockets[port].tx_buffer_size = 0;

    if (is_same_network(remote_address)){
        if (!get_mac_for_ip(remote_address, udp_sockets[port].remote_mac)){
            arp_query(remote_address, udp_sockets[port].remote_mac);
            udp_sockets[port].status |= STATUS_ARP_RESOLVING;
        }
    } 

    return 0;
}

int ipv4_socket_udp_send_add (int pid, uint16_t port,
    void* data, 
    uint16_t length){

    buffer_write(&(udp_sockets[port].tx_buffer), data, length);
    
    return 0;
}
int ipv4_socket_udp_send_commit (int pid, uint16_t port){

    return 0; 
}

static void handle_udp_packet (Ipv4Packet* ipv4_packet){

    log("\t\tUDP packet:\n");
    log("\t\tSource port: %d\n", htons2(ipv4_packet->udp.header.source_port));
    log("\t\tTarget port: %d\n", htons2(ipv4_packet->udp.header.target_port));
    log("\t\tPacket length: %d\n", htons2(ipv4_packet->udp.header.length));

    uint16_t target_port = htons2(ipv4_packet->udp.header.target_port);

    if (udp_sockets[target_port].status & STATUS_ACTIVE){

        if (udp_sockets[target_port].status & STATUS_RECEIVING){

            uint16_t total_length = htons2(ipv4_packet->udp.header.length);
            uint16_t to_send = total_length;

            uint16_t status = SEND_STATUS_START;
            int l=0;

            while (to_send > 0){
                
                uint16_t chunk_size = min(BUFFER_SIZE, to_send);

                debug("\t\t\tChunk size: %d",chunk_size);

                memcpy(
                    udp_sockets[target_port].rx_buffer + udp_sockets[target_port].rx_index, 
                    ipv4_packet->udp.payload, 
                    chunk_size
                );

                udp_sockets[target_port].rx_index += chunk_size;

                to_send -= chunk_size;

                if (to_send == 0 && l > 0){
                    status = SEND_STATUS_END;
                }

                udp_received(
                    udp_sockets[target_port].pid,
                    SOCKET_TYPE_UDP | SOCKET_TYPE_SERVER,
                    target_port,
                    ipv4_packet->ipv4.source_address.l_address,
                    ipv4_packet->udp.header.source_port,
                    chunk_size,
                    status,
                    udp_sockets[target_port].rx_buffer
                );

                udp_sockets[target_port].rx_index = 0;
                //if (!ipv4_packet->ipv4.more_fragments){
                //}
                l++;
            }
        } else {
            log("\t\tUDP socket NOT receiving\n");
        }

    } else {
        log("\t\tSocket %d not active\n", target_port);
    }
}

static void handle_tcp_packet (Ipv4Packet* ipv4_packet){

    log("\t\tTCP packet\n");
    log("\t\tSource port: %d\n", htons2(ipv4_packet->tcp.header.source_port));
    log("\t\tTarget port: %d\n", htons2(ipv4_packet->tcp.header.target_port));

    uint16_t target_port = htons2(ipv4_packet->tcp.header.target_port);

    if (tcp_sockets[target_port].status & STATUS_ACTIVE){
        if (!(tcp_sockets[target_port].status & STATUS_ACCEPTING)){
            log("\t\tSocket %d not accepting\n", target_port);
            /*
            uint16_t local_port;
            // find existing local tcp port
            local_port = find_local_tcp_port(
                tcp_sockets[target_port].pid,
                ipv4_packet->ipv4.source_address,
                ipv4_packet->tcp.header.source_port
            );

            if (!local_port){
                local_port = allocate_tcp_socket(tcp_sockets[target_port].pid);

            }
            */
        }
    } else {
        log("\t\tSocket %d not active\n", target_port);
    }
}

static int allocate_tcp_socket(int pid){
    for (int i=0;i < MAX_SOCKETS;i++){
        if (!(tcp_sockets[i].status & STATUS_ACTIVE)){
            tcp_sockets[i].status |= STATUS_ACTIVE;
            tcp_sockets[i].pid = pid;

            return i;
        }
    }

    return 0;
}

static int is_same_network(IpAddress address){
    return (address.l_address & mask.l_address) 
        == (ip_address.l_address & mask.l_address);
}

#define active_resolving(s) ((s & (STATUS_ARP_RESOLVING | STATUS_ACTIVE)) == (STATUS_ARP_RESOLVING | STATUS_ACTIVE))

void ipv4_arp_address_resolved(IpAddress* address, uin8_t* mac){
    for (int i=0;i<MAX_SOCKETS;i++){
        if (active_resolving(udp_sockets[i].status)){
            memcpy(udp_sockets[i].remote_mac, mac);
            udp_sockets[i].status &= ~ STATUS_ARP_RESOLVING;
        }
    }
}
