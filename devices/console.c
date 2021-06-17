#include "console.h"
#include "kernel/device.h"
#include "lib/heap.h"

typedef struct {
    CharDevice device;
    CharDevice* screen;
    CharDevice* keyboard;
} ConsoleDevice;

#define CONSOLE_DEVICE(d)   ((ConsoleDevice*)d)

static uint8_t  count_devices   (DeviceType* device_type);
static Device*  instantiate     (DeviceType* device_type, uint8_t device_number);
static void     release         (DeviceType* device_type, Device* device);

static DeviceType DEVICE_TYPE = {
    kind: TERM,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};

static int16_t  console_write   (CharDevice* device, uint8_t data);
static int16_t  console_read    (CharDevice* device);

void console_register(){
    device_register_type((DeviceType*)&DEVICE_TYPE);
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    ConsoleDevice* device = heap_alloc(sizeof(ConsoleDevice));
    device->screen = CHAR_DEVICE(device_find(VIDEO,0));
    device->screen = CHAR_DEVICE(device_find(KBD,0));
    CHAR_DEVICE(device)->read = console_read;
    CHAR_DEVICE(device)->write = console_write;
    return DEVICE(device);
}

static uint8_t count_devices(struct DeviceType* device_type){
    return 1;
}

static int16_t console_write(CharDevice* device, uint8_t data){
    return char_device_write(CONSOLE_DEVICE(device)->screen,data);
}

static int16_t console_read(CharDevice* device){
    int16_t data = char_device_read(CONSOLE_DEVICE(device)->keyboard);


    return data;
}
