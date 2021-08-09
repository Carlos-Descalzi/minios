#include "cache.h"
#include "string.h"

#define IP_CACHE_SIZE           100

typedef struct {
    uint8_t used;
    uint8_t mac_address[6];
    uint8_t ip_address[4];
} IpCacheEntry;

static IpCacheEntry     ip_cache[IP_CACHE_SIZE];

void cache_init (void){
    memset(ip_cache,0, sizeof(ip_cache));
}

int get_mac_for_ip (uint8_t* ip, uint8_t* mac){
    for (int i=0;i<IP_CACHE_SIZE;i++){
        if (ip_cache[i].used
            && !memcmp(ip_cache[i].ip_address, ip, 4)){
            memcpy(mac, ip_cache[i].mac_address, 6);

            if (i > 0){
                raise_ip_cache_entry(i);
            }
            return 1;
        }
    }
    return 0;
}

void store_mac_for_ip(uint8_t* ip, uint8_t* mac){
    int i;
    for (i=0;i<IP_CACHE_SIZE-1;i++){
        if (!ip_cache[i].used){
            break;
        }
    }
    ip_cache[i].used = 1;
    memcpy(ip_cache[i].ip_address, ip, 4);
    memcpy(ip_cache[i].mac_address, mac, 6);
}

void raise_ip_cache_entry(int index){
    IpCacheEntry tmp;
    memcpy(&tmp, &(ip_cache[index-1]), sizeof(IpCacheEntry));
    memcpy(&(ip_cache[index-1]), &(ip_cache[index]), sizeof(IpCacheEntry));
    memcpy(&(ip_cache[index]), &tmp, sizeof(IpCacheEntry));
}
