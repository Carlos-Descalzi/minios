#include "ipv4.h"
#include "stdlib.h"
#include "string.h"
#include "cache.h"

#define STATUS_ACTIVE           0x01
#define STATUS_SERVER           0x02
#define STATUS_ACCEPTING        0x04
#define STATUS_RECEIVING        0x08
#define STATUS_WAITING_MORE     0x10
#define STATUS_WAITING_ADDRESS  0x20

static UdpSocket udp_sockets[MAX_SOCKETS];
static TcpSocket tcp_sockets[MAX_SOCKETS];

static void handle_udp_packet   (int fd, Ipv4Packet* ipv4_packet);
static void handle_tcp_packet   (int fd, Ipv4Packet* ipv4_packet);
static int  allocate_tcp_socket (int pid);

void ipv4_init (void){
    memset(udp_sockets,0, sizeof(udp_sockets));
    memset(tcp_sockets,0, sizeof(tcp_sockets));
}

void ipv4_handle_packet(int fd, EthFrame* frame){

    log("\t\tIPv4 Packet\n\t\tSource address: ");
    print_ip(frame->ipv4_packet.ipv4.source_address);
    log("\n");

    store_mac_for_ip(frame->ipv4_packet.ipv4.source_address, frame->source_mac);

    switch(frame->ipv4_packet.ipv4.protocol){
        case IPV4_TYPE_UDP:
            handle_udp_packet(fd, &(frame->ipv4_packet));
            break;
        case IPV4_TYPE_TCP:
            handle_tcp_packet(fd, &(frame->ipv4_packet));
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

        if (tcp_sockets[port].buffer){
            free(tcp_sockets[port].buffer);
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
            free(udp_sockets[port].rx_buffer);
        }
        if (udp_sockets[port].tx_buffer){
            free(udp_sockets[port].tx_buffer);
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
            udp_sockets[port].rx_buffer = malloc(BUFFER_SIZE);
            udp_sockets[port].rx_index = 0;
            udp_sockets[port].tx_buffer = malloc(BUFFER_SIZE);
            udp_sockets[port].tx_index = 0;
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

static void handle_udp_packet (int fd, Ipv4Packet* ipv4_packet){

    log("\t\tUDP packet:\n");
    log("\t\tSource port: %d\n", htons2(ipv4_packet->udp.header.source_port));
    log("\t\tTarget port: %d\n", htons2(ipv4_packet->udp.header.target_port));

    uint16_t target_port = htons2(ipv4_packet->udp.header.target_port);

    if (udp_sockets[target_port].status & STATUS_ACTIVE){

        if (udp_sockets[target_port].status & STATUS_RECEIVING){

            memcpy(
                udp_sockets[target_port].rx_buffer, 
                ipv4_packet->udp.payload, 
                ipv4_packet->udp.header.length
            );

            udp_sockets[target_port].rx_index += ipv4_packet->udp.header.length;

            if (!ipv4_packet->ipv4.more_fragments){
                udp_received(
                    udp_sockets[target_port].pid,
                    SOCKET_TYPE_UDP | SOCKET_TYPE_SERVER,
                    target_port,
                    ip_caddr_to_l(ipv4_packet->ipv4.source_address),
                    ipv4_packet->udp.header.source_port,
                    udp_sockets[target_port].rx_index,
                    udp_sockets[target_port].rx_buffer
                );
            }
        }
    } else {
        log("\t\tSocket %d not active\n", target_port);
    }
}

static void handle_tcp_packet (int fd, Ipv4Packet* ipv4_packet){

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

