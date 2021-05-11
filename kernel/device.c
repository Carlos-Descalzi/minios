#include "device.h"
#include "string.h"
#include "pci.h"
#include "ps2.h"
#include "bda.h"
#include "debug.h"

typedef struct DeviceInstance {
    DeviceType* device_type;
    Device* device;
} DeviceInstance;

// TODO: Use some dynamic structure
#define MAX_DEVICE_TYPES    20
#define MAX_DEVICES        40
static DeviceType* device_types[MAX_DEVICE_TYPES];
static DeviceInstance devices[MAX_DEVICES];

void device_init(){
    memset(device_types,0,sizeof(device_types));
    memset(devices,0,sizeof(devices));
}
void device_list_types (DeviceVisitor visitor, void* user_data){
    int i;
    for (i=0;i<MAX_DEVICE_TYPES;i++){
        if (device_types[i]){
            if (visitor(i, device_types[i], user_data)){
                break;
            }
        }
    }
}
int16_t device_register_type (DeviceType* device_type){
    int i;
    for (i=0;i<MAX_DEVICE_TYPES;i++){
        if (!device_types[i]){
            device_types[i] = device_type;
            return 0;
        }
    }
    return -1;
}
void device_init_devices(void){
    int i, j, device_index, device_count;

    device_index = 0;
    for (i=0;i< MAX_DEVICE_TYPES;i++){

        if (device_types[i]){
            device_count = device_types[i]->count_devices(device_types[i]);

            for (j=0;j<device_count;j++){
                devices[device_index].device_type = device_types[i];
                devices[device_index].device = device_types[i]->instantiate(device_types[i], j);
                devices[device_index].device->instance_number = j;
                device_index++;
            }
        }
    }
}
