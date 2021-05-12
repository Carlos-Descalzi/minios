#include "device.h"
#include "serial.h"
#include "bda.h"
#include "heap.h"
#include "io.h"
#include "debug.h"
#include "console.h"
#include "stdlib.h"

static uint8_t count_devices(DeviceType* device_type);
static Device* instantiate(DeviceType* device_type, uint8_t device_number);
int16_t serial_setopt(Device* device, uint32_t option, void* data);
int16_t serial_read(CharDevice* device);
int16_t serial_write(CharDevice* device, uint8_t chr);
static void release(DeviceType* device_type, Device* device);

typedef struct {
    CharDevice device;
    uint16_t port;
} SerialDevice;

#define SERIAL_DEVICE(d)    ((SerialDevice*)d)

static DeviceType SERIAL_DEVICE_TYPE = {
    kind: SER,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};

void serial_register(){
    device_register_type((DeviceType*)&SERIAL_DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    int i;
    for (i=0;BDA->com_ports[i];i++);
    return i;
}

static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    char buff[8];
    uint16_t port = BDA->com_ports[device_number];
    SerialDevice* device = heap_alloc(sizeof(SerialDevice));
    device->device.base.type = DEVICE_TYPE_CHAR;
    device->device.base.setopt = serial_setopt;
    device->device.read = serial_read;
    device->device.write = serial_write;
    device->port = port;
    console_print("Serial port ");
    console_print(itoa(port,buff,16));
    console_print(" initialized\n");
    return DEVICE(device);
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

int16_t serial_setopt(Device* device, uint32_t option, void* data){
}

int16_t serial_read(CharDevice* device){
    return inb(SERIAL_DEVICE(device)->port);
}

int16_t serial_write(CharDevice* device, uint8_t chr){
    outb(SERIAL_DEVICE(device)->port, chr);
    return 0;
}
