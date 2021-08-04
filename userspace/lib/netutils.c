#include "netutils.h"
#include "string.h"
#include "ctype.h"


static uint8_t to_hexd(char cdigit){
    if (cdigit >= '0' && cdigit <= '9'){
        return cdigit - '0';
    } else if (cdigit >= 'a' && cdigit <= 'f'){
        return 10 + cdigit - 'a';
    } 
    return 0;
}

static uint8_t to_hex(char* token){
    uint8_t val = 0;
    int len = strlen(token);
    for (int i=len-1,n=0;i>=0;i--,n++){
        val |= to_hexd(token[i]) << (4 * n);
    }
    return val;
}

int parse_mac_address(const char* mac_string, uint8_t* dst_mac){
    char hex[5];
    int j=0;

    memset(hex,0,3);

    for (int i=0, k=0; i<strlen(mac_string); i++){
        char c = tolower(mac_string[i]);

        if (isxdigit(c)){
            hex[k++] = c;
        } else if (c == ':'){
            if (strlen(hex) != 2){
                return -1;
            }
            dst_mac[j++] = to_hex(hex);
            k = 0;
        } else {
            return -1;
        }
    }
    if (strlen(hex) != 2){
        return -1;
    }
    dst_mac[j++] = to_hex(hex);

    return 0;
}

int parse_ipv4_address(const char* ipv4_string, uint8_t* dst_ip){
    return 0;
}

uint16_t ipv4_checmsum(void* header, int len){
    uint32_t checksum = 0;

    for (int i=0;i<len/2;i++){
        checksum += ((uint16_t*)header)[i];
    }
    checksum += checksum >> 16;
    return ~checksum & 0xFFFF;
}
