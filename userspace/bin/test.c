#include "netutils.h"
#include "stdio.h"
#include "stdlib.h"

char mac_addr[] = "12:34:56:ab:CD:eF";

int main(){
    uint8_t* mac;

    mac = malloc(16384);

    memset(mac,0,16384);

    int ret = parse_mac_address(mac_addr, mac);

    printf("ret:%d\n",ret);

    for (int i=0;i<6;i++){
        if (i > 0){
            printf(":");
        }
        printf("%02x",mac[i]);
    }
    printf("\n");
    free(mac);
    return 0;
}
