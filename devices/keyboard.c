#include "keyboard.h"
#include "kernel/device.h"
#include "kernel/isr.h"
#include "board/ps2.h"
#include "board/pic.h"
#include "lib/heap.h"

#define BUFFER_SIZE 256

static uint8_t  count_devices       (DeviceType* device_type);
static Device*  instantiate         (DeviceType* device_type, uint8_t device_number);
static void     release             (DeviceType* device_type, Device* device);
static int16_t  setopt              (Device* device, uint32_t option, void* data);
static int16_t  read                (CharDevice* device);
static void     handle_keyboard_irq (InterruptFrame frame);

typedef struct {
    CharDevice device;
    uint8_t* buffer;
    uint8_t buffptr;
    uint8_t readptr;
    uint32_t ticks;
} KeyboardDevice;

#define KEYBOARD_DEVICE(d)  ((KeyboardDevice*)d)

static DeviceType DEVICE_TYPE = {
    kind: KBD,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};

static KeyboardDevice KBD_DEVICE = {
    device: {
        base: {
            type: DEVICE_TYPE_CHAR,
            setopt: setopt,
        },
        read: read,
        write: NULL
    },
    buffer: NULL,
    ticks: 0
};

void keyboard_register(){
    device_register_type((DeviceType*)&DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    return 1;
}
static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    KBD_DEVICE.buffer = heap_alloc(BUFFER_SIZE);
    KBD_DEVICE.buffptr = 0;

    isr_install(PIC_IRQ_BASE + PS2_IRQ, handle_keyboard_irq);
    return DEVICE(&KBD_DEVICE);
}
static void release(DeviceType* device_type, Device* device){
    heap_free(KBD_DEVICE.buffer);
}
static int16_t setopt(Device* device, uint32_t option, void* data){
    return 0;
}
static int16_t read(CharDevice* device){
    int16_t val;
    if (KBD_DEVICE.readptr != KBD_DEVICE.buffptr){
        val = KBD_DEVICE.buffer[KBD_DEVICE.readptr++];
        KBD_DEVICE.readptr++;

        if (KBD_DEVICE.readptr >= BUFFER_SIZE){
            KBD_DEVICE.readptr = 0;
        }
        return val;
    }
        
    return -1;
}
static void handle_keyboard_irq (InterruptFrame frame){
   uint8_t val;
   cli();

   if (KBD_DEVICE.buffptr != KBD_DEVICE.readptr){
       KBD_DEVICE.buffer[KBD_DEVICE.buffptr++] = val;

       if (KBD_DEVICE.buffptr >= BUFFER_SIZE){
           KBD_DEVICE.buffptr = 0;
       }
   }
   pic_eoi();
   sti();
}