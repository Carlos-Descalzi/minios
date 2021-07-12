#ifndef _NETUTILS_H_
#define _NETUTILS_H_

#include "stdint.h"

int         parse_mac_address   (const char* mac_string, uint8_t* dst_mac);
int         parse_ipv4_address  (const char* ipv4_string, uint8_t* dst_ip);
uint16_t    ipv4_checmsum       (void* header, int len);

#endif
