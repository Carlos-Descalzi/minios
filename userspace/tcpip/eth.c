#include "eth.h"
#include "string.h"

void eth_set(
        EthFrame* frame, 
        uint16_t ether_type, 
        const uint8_t* target_mac){

    memcpy(frame->source_mac, device_mac, 6);
    memcpy(frame->target_mac, target_mac, 6);
    frame->ether_type = htons2(ether_type);

}
