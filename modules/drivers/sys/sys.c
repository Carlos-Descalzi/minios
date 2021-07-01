#include "kernel/device.h"
#include "lib/heap.h"
/**
 * Placeholder for SYS filesystem
 **/

static DeviceType DEVICE_TYPE = {
    .kind = SYS
};

static uint8_t count_devices(){
    return 1;
}
static int16_t read(BlockDevice* device, uint8_t* buffer, uint16_t size){
    return 0;
}
static int16_t write(BlockDevice* device, uint8_t* buffer, uint16_t size){
    return 0;
}
static void seek(BlockDevice* device, uint32_t pos){
}
static uint32_t pos(BlockDevice* device){
    return 0;
}
int16_t setopt(Device* device, uint32_t option, void* data){
    return 0;
}
static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    BlockDevice* device = heap_alloc(sizeof(BlockDevice));

    device->base.type = DEVICE_TYPE_BLOCK;
    device->base.setopt = setopt;
    device->read = read;
    device->write = write;
    device->seek = seek;
    device->pos = pos;

    return DEVICE(device);
}
static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

void module_init(){
    DEVICE_TYPE.count_devices = count_devices;
    DEVICE_TYPE.instantiate = instantiate;
    DEVICE_TYPE.release = release;
    device_register_type(&DEVICE_TYPE);
}
