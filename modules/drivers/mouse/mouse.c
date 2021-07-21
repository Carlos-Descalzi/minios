//#define NODEBUG
#include "kernel/device.h"
#include "board/ps2.h"
#include "board/pic.h"
#include "board/io.h"
#include "lib/heap.h"
#include "kernel/isr.h"
#include "misc/debug.h"

typedef struct {
    CharDevice device;
    IORequest* request;
    uint8_t buffer[3];
    uint8_t bufindex;
    uint8_t reset;
} MouseDevice;

#define MOUSE_DEVICE(d)             ((MouseDevice*)d)

#define OPT_MOUSE_ENABLE            0

static uint8_t  count_devices       (DeviceType* device_type);
static Device*  instantiate         (DeviceType* device_type, uint8_t device_number);
static int16_t  setopt              (Device* device, uint32_t option, void* data);
static int16_t  read                (CharDevice* device);
static int16_t  write               (CharDevice* device, uint8_t chr);
static void     release             (DeviceType* device_type, Device* device);
static int16_t  read_async          (CharDevice* device,IORequest* request);
static void     handle_mouse_irq    (InterruptFrame* f, void* data);
static void     set_enabled         (MouseDevice* device, uint32_t enabled);

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
    
    uint8_t v = ps2_read_data();
    ps2_write_data(v | 2);

    ps2_write_2_ack(0xf6);

    ps2_write_2_ack(0xf4);
    
    device->reset = 1;
    device->bufindex = 0;

    isr_install(PIC_IRQ_BASE + 0x0C, handle_mouse_irq, device);
    pic_enable(0xC);

    return DEVICE(device);
}

static int16_t setopt (Device* device, uint32_t option, void* data){
    switch (option){
        case OPT_MOUSE_ENABLE:
            set_enabled(MOUSE_DEVICE(device), (uint32_t) data);
            break;
    }
    return 0;
}

static void set_enabled (MouseDevice* device, uint32_t enabled){
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

static void handle_request (MouseDevice* device, IORequest* request){
    char buff[3];

    memcpy(buff, MOUSE_DEVICE(device)->buffer,3);
    MOUSE_DEVICE(device)->bufindex  = 0;
    handle_io_request(request, buff, 3, TASK_IO_REQUEST_DONE);

}
static void handle_empty_request (MouseDevice* device, IORequest* request){
    char buff;
    handle_io_request(request, &buff,0, TASK_IO_REQUEST_DONE);
}

static int16_t read_async (CharDevice* device,IORequest* request){

    if (MOUSE_DEVICE(device)->bufindex == 3){
        handle_request(MOUSE_DEVICE(device), request);
    } else if (request->type & TASK_IO_NOBLOCK){
        handle_empty_request(device, request);
    } else {
        MOUSE_DEVICE(device)->request = request;
    }

    return 0;
}

static void handle_mouse_irq (InterruptFrame* f, void* data){
    cli();

    MouseDevice* device = data;

    uint8_t val = ps2_read(PORT_DATA);

    if (device->reset) {
        device->reset = 0;
    } else if(device->bufindex < 3) {
        device->buffer[device->bufindex++] = val;
    } // else: overflow - discard

    if (device->bufindex == 3) {
        if (device->request){

            IORequest* request = device->request;
            device->request = NULL;
            handle_request(device, request);

        } 
    }


    pic_eoi1();
    pic_eoi2();
    sti();
}
