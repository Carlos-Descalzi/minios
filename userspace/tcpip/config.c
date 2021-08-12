#include "tcpip.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

char    network_device[10];
IpAddress ip_address;
IpAddress mask;
IpAddress gateway;
uint8_t device_mac[6];
uint8_t gateway_mac[6];
const uint8_t broadcast_mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
const uint8_t null_mac[6] = {0x00,0x00,0x00,0x00,0x00,0x00};


int read_config(){
    char buffer[256];
    char* pos = NULL;
    FILE* fp = fopen("disk0:/etc/net.conf","r");

    if (!fp){
        log("Unable to read config, exiting\n");
        return -1;
    }
    memset(network_device,0,10);
    
    while (fgets(buffer,256,fp)){
        if (!strlen(buffer)){
            break;
        }
        char* opt = strtok_r(buffer,"=",&pos);
        char* val = strtok_r(NULL,"=",&pos);

        if (!strcmp(opt,"dev")){
            strcpy(network_device, val);
        } else if (!strcmp(opt,"ip")){
            parse_ip(val, ip_address.c_address);
        } else if (!strcmp(opt,"mask")){
            parse_ip(val, mask.c_address);
        } else if (!strcmp(opt,"gw")){
            parse_ip(val, gateway.c_address);
        }

        pos = NULL;
        memset(buffer,0,256);
    }
    fclose(fp);

    log("Network device:%s\n",network_device);

    log("IP address    :");
    print_ip(ip_address);
    log("\n");

    log("Network mask  :");
    print_ip(mask);
    log("\n");

    log("Gateway       :");
    print_ip(gateway);
    log("\n");


    return 0;
}

