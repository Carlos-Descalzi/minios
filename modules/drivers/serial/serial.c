#include "kernel/device.h"
#include "board/bda.h"
#include "board/io.h"
#include "lib/heap.h"
#include "lib/stdlib.h"
#include "misc/debug.h"
#include "kernel/isr.h"
#include "board/pic.h"

static uint8_t count_devices(DeviceType* device_type);
static Device* instantiate(DeviceType* device_type, uint8_t device_number);
int16_t serial_setopt(Device* device, uint32_t option, void* data);
int16_t serial_read(CharDevice* device);
int16_t serial_write(CharDevice* device, uint8_t chr);
static void release(DeviceType* device_type, Device* device);
//static void handle_irq (InterruptFrame* f, void* data);

typedef struct {
    CharDevice device;
    uint16_t port;
} SerialDevice;

#define SERIAL_DEVICE(d)    ((SerialDevice*)d)

static DeviceType SERIAL_DEVICE_TYPE = {
    .kind = SER
};

void module_init(){
    SERIAL_DEVICE_TYPE.count_devices = count_devices;
    SERIAL_DEVICE_TYPE.instantiate = instantiate;
    SERIAL_DEVICE_TYPE.release = release;
    device_register_type((DeviceType*)&SERIAL_DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    int i;
    for (i=0;BDA->com_ports[i];i++);
    return i;
}

static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    uint16_t port = BDA->com_ports[device_number];
    SerialDevice* device = heap_alloc(sizeof(SerialDevice));
    device->device.base.type = DEVICE_TYPE_CHAR;
    device->device.base.setopt = serial_setopt;
    device->device.read = serial_read;
    device->device.write = serial_write;
    device->port = port;
    //isr_install(PIC_IRQ_BASE + 11, handle_irq, device);
    return DEVICE(device);
}
/*
static void handle_irq (InterruptFrame* f, void* data){
    cli();
    debug("serial irq\n");
    pic_eoi2();
    sti();
}
*/
static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

int16_t serial_setopt(Device* device, uint32_t option, void* data){
    return 0;
}

int16_t serial_read(CharDevice* device){
    return inb(SERIAL_DEVICE(device)->port);
}

int16_t serial_write(CharDevice* device, uint8_t chr){
    outb(SERIAL_DEVICE(device)->port, chr);
    return 0;
}
