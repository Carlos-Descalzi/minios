/**
 * Test code for eth and ip stack
 **/
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "tcpip.h"

void print_mac(uint8_t* mac){
    log ("%02x:%02x:%02x:%02x:%02x:%02x", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void print_ip(IpAddress ip){
    log("%u.%u.%u.%u", 
        ip.c_address[0], ip.c_address[1], ip.c_address[2], ip.c_address[3]);
}

void parse_ip(const char* ip_str, uint8_t* ip){
    char buff[20];
    strcpy(buff,ip_str);
    char* pos = NULL;

    int i = 0;
    char* token = strtok_r(buff,".",&pos);

    while(token){
        ip[i++] = atoi(token);
        token = strtok_r(NULL,".",&pos);
    }
}
