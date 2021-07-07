#include "kernel/device.h"

typedef struct {
    uint8_t preamble[7];
    uint8_t start;
    uint8_t mac_destination[6];
    uint8_t mac_source[6];
    uint8_t tag[4];

} EthernetFrameHeader;

typedef struct {
    BlockDevice device;
    BlockDevice adapter;
} EthDevice;

#define ETH_DEVICE(d)       ((EthDevice*)(d))

static uint8_t  count_devices   (DeviceType* device_type);
static Device*  instantiate     (DeviceType* device_type, uint8_t device_number);
static void     release         (DeviceType* device_type, Device* device);
static int16_t  read_async      (BlockDevice* device, IORequest* request);
static int16_t  write           (BlockDevice* device, uint8_t* buffer, uint16_t size);

static DeviceType DEVICE_TYPE;

void module_init(){
    DEVICE_TYPE.kind = NET;
    DEVICE_TYPE.count_devices = count_devices;
    DEVICE_TYPE.instantiate = instantiate;
    DEVICE_TYPE.release = release;
    device_register_type(&DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    return 1;
}
static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    EthDevice* device = heap_alloc(sizeof(EthDevice));

    device->adapter = device_find(NET,0);

    BLOCK_DEVICE(device)->read_async = read_async;
    BLOCK_DEVICE(device)->write = write;

    DEVICE(device)->async = 1;

    return DEVICE(device);
}
static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}
static int16_t read_async(BlockDevice* device, IORequest* request){
}
static int16_t write(BlockDevice* device, uint8_t* buffer, uint16_t size){
}
