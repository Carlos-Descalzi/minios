#ifndef _ETH_H_
#define _ETH_H_

#include "tcpip.h"

void eth_set (EthFrame* frame, uint16_t ether_type, const uint8_t* target_mac);

#endif
