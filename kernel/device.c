#include "kernel/device.h"
#include "lib/string.h"
#include "board/pci.h"
#include "board/ps2.h"
#include "board/bda.h"
#include "misc/debug.h"
#include "kernel/paging.h"

static const char* kinds[] = {
    "video",
    "ser",
    "disk",
    "net",
    "kbd",
    "mouse",
    "term"
};

typedef struct DeviceInstance {
    DeviceType* device_type;
    Device* device;
} DeviceInstance;

// TODO: Use some dynamic structure
#define MAX_DEVICE_TYPES    20
#define MAX_DEVICES         40
static DeviceType* device_types[MAX_DEVICE_TYPES];
static DeviceInstance devices[MAX_DEVICES];
static int initialized = 0;

static void init_devices        (DeviceType* device_type);
static void do_init_device      (DeviceType* device_type, int*device_index);

void device_init(){
    memset(device_types,0,sizeof(device_types));
    memset(devices,0,sizeof(devices));
}
void device_list_types (DeviceTypeVisitor visitor, void* user_data){
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

            if (initialized){
                init_devices(device_type);
            }
            return 0;
        }
    }
    return -1;
}
void device_init_devices(void){
    int i, device_index;

    device_index = 0;
    for (i=0;i< MAX_DEVICE_TYPES;i++){
        if (device_types[i]){
            do_init_device(device_types[i], &device_index);
        }
    }
    initialized = 1;
}

void device_list (DeviceVisitor visitor, void *data){
    int i;

    for (i=0;devices[i].device;i++){
        if(visitor(i,devices[i].device->kind,devices[i].device,data)){
            break;
        }
    }
}

Device* device_find(uint8_t kind, uint8_t instance){
    int i;

    for (i=0;devices[i].device;i++){
        if (devices[i].device->kind == kind
            && devices[i].device->instance_number == instance){
            return devices[i].device;
        }
    }
    return NULL;
}

static void init_devices(DeviceType* device_type){
    int device_index;

    for (device_index = 0;devices[device_index].device;device_index++);

    do_init_device(device_type,&device_index);
}

static void do_init_device(DeviceType* device_type, int*device_index){
    int j;
    char buff[10];
    int device_count = device_type->count_devices(device_type);

    for (j=0;j<device_count;j++){
        devices[*device_index].device_type = device_type;
        devices[*device_index].device = device_type->instantiate(device_type, j);
        if (devices[*device_index].device){
            devices[*device_index].device->kind = device_type->kind;
            devices[*device_index].device->instance_number = j;
            console_print(kinds[device_type->kind]);
            console_print(itoa(devices[*device_index].device->instance_number,buff,10));
            console_print("\n");
            (*device_index)++;
        }
    }
}

