#include "kernel/device.h"
#include "board/ps2.h"
#include "board/pic.h"
#include "lib/heap.h"
#include "kernel/isr.h"
#include "misc/debug.h"

static uint8_t  count_devices       (DeviceType* device_type);
static Device*  instantiate         (DeviceType* device_type, uint8_t device_number);
static int16_t  setopt              (Device* device, uint32_t option, void* data);
static int16_t  read                (CharDevice* device);
static int16_t  write               (CharDevice* device, uint8_t chr);
static void     release             (DeviceType* device_type, Device* device);
static int16_t  read_async          (CharDevice* device,IORequest* request);
static void     handle_mouse_irq    (InterruptFrame* f, void* data);

typedef struct {
    CharDevice device;
    IORequest* request;
} MouseDevice;

#define MOUSE_DEVICE(d)         ((MouseDevice*)d)

static DeviceType DEVICE_TYPE = {
    .kind = MOUSE
};

void module_init(){

    DEVICE_TYPE.count_devices = count_devices;
    DEVICE_TYPE.instantiate = instantiate;
    DEVICE_TYPE.release = release;
    
    device_register_type(&DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    return 1;
}

static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    MouseDevice* device = heap_new(MouseDevice);

    DEVICE(device)->type = DEVICE_TYPE_CHAR;
    DEVICE(device)->async = 1;
    DEVICE(device)->setopt = setopt;
    CHAR_DEVICE(device)->read = read;
    CHAR_DEVICE(device)->read_async = read_async;
    CHAR_DEVICE(device)->write = write;

    ps2_write(PORT_CMD, CMD_ENABLE_2PORT);
    pic_enable(PS2_IRQ);

    ps2_write(PORT_CMD, CMD_WRITE_PS2);
    ps2_write(PORT_DATA, 0xF4);

    isr_install(PIC_IRQ_BASE + PS2_IRQ2, handle_mouse_irq, device);

    return DEVICE(device);
}

static int16_t setopt (Device* device, uint32_t option, void* data){
}

static int16_t read (CharDevice* device){
    return 0;
}

static int16_t write (CharDevice* device, uint8_t chr){
    return 0;
}

static void release (DeviceType* device_type, Device* device){
    heap_free(device);
}

static int16_t read_async (CharDevice* device,IORequest* request){
    MOUSE_DEVICE(device)->request = request;
    return 0;
}

static void handle_mouse_irq (InterruptFrame* f, void* data){
    debug("IRQ!!\n");
}
