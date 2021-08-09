#ifndef _CACHE_H_
#define _CACHE_H_

#include "stdint.h"

void cache_init             (void);
int  get_mac_for_ip         (uint8_t* ip, uint8_t* mac);
void store_mac_for_ip       (uint8_t* ip, uint8_t* mac);
void raise_ip_cache_entry   (int index);

#endif
