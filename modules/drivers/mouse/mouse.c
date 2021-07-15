#include "kernel/device.h"
#include "board/ps2.h"
#include "board/pic.h"
#include "board/io.h"
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

    //ps2_write(PORT_CMD, CMD_READ_BYTE);
    
    uint8_t v = ps2_read_data();

    //ps2_write(PORT_CMD, CMD_WRITE_BYTE);
    //ps2_write(PORT_CMD, v | 2);

    ps2_write_data(v | 2);

    v = ps2_read_data();

    ps2_write(PORT_CMD, CMD_ENABLE_2PORT);

    pic_enable(PS2_IRQ2);
    isr_install(PIC_IRQ_BASE + PS2_IRQ2, handle_mouse_irq, device);

    /*ps2_write(PORT_CMD, CMD_WRITE_PS2);
    ps2_write(PORT_DATA, 0xA9);
    debug_i(ps2_read(PORT_DATA),16);debug("\n");*/

    /*
    ps2_write_data_ack(0xa9);
    ps2_write_data_ack(0xf4);
    ps2_write_data_ack(0xea);
    ps2_write_data_ack(0xf6);
    */
    return DEVICE(device);
}

static int16_t setopt (Device* device, uint32_t option, void* data){
    return 0;
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
    debug_i(pic_get_irq(),16);
    debug("\n");
    pic_eoi2();
}
