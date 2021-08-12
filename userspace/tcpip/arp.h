#ifndef _ARP_H_
#define _ARP_H_

#include "tcpip.h"

void arp_announce        (uint8_t* tx_buffer);
void arp_handle_packet   (EthFrame* frame, uint8_t* tx_buffer);
void arp_query           (IpAddress ip, uint8_t* tx_buffer);


#endif
