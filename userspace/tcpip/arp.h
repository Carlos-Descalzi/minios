#ifndef _ARP_H_
#define _ARP_H_

#include "tcpip.h"

void arp_announce        (int fd, uint8_t* tx_buffer);
void arp_handle_packet   (int fd, EthFrame* frame, uint8_t* tx_buffer);
void arp_query           (int fd, uint8_t* ip, uint8_t* tx_buffer);


#endif
