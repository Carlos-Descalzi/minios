#ifndef _CACHE_H_
#define _CACHE_H_

#include "stdint.h"
#include "proto.h"

void cache_init             (void);
int  has_mac_for_ip         (IpAddress ip);
int  get_mac_for_ip         (IpAddress ip, uint8_t* mac);
void store_mac_for_ip       (IpAddress ip, uint8_t* mac);
void raise_ip_cache_entry   (int index);

#endif
