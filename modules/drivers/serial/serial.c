#include "kernel/device.h"
#include "board/bda.h"
#include "board/io.h"
#include "lib/heap.h"
#include "lib/stdlib.h"
#include "lib/string.h"
#include "misc/debug.h"
#include "kernel/isr.h"
#include "board/pic.h"
#define abs(a)  ((a) < 0 ? -(a) : (a))

#define RX_BUFFER_SIZE  1024
#define PIC_IRQ_SERIAL  4

#define SERIAL_DATA 0
#define SERIAL_IER  1
#define SERIAL_FIFO 2
#define SERIAL_LCR  3
#define SERIAL_MCR  4
#define SERIAL_LSR  5
#define SERIAL_MSR  6
#define SERIAL_SR   7

typedef struct {
    CharDevice device;
    uint16_t port;
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    uint16_t rx_write_pos;
    uint16_t rx_read_pos;
    uint16_t read;
    IORequest* request;
} SerialDevice;

static uint8_t  count_devices   (DeviceType* device_type);
static Device*  instantiate     (DeviceType* device_type, uint8_t device_number);
static int16_t  setopt          (Device* device, uint32_t option, void* data);
static int16_t  read            (CharDevice* device);
static int16_t  write           (CharDevice* device, uint8_t chr);
static int16_t  read_async      (CharDevice* device,IORequest* request);
static void     release         (DeviceType* device_type, Device* device);
static void     handle_irq      (InterruptFrame* f, void* data);
static uint32_t available       (CharDevice *device);
static int      recv_size       (SerialDevice* device);
static int16_t handle_read      (SerialDevice* device, IORequest* request);


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

    SerialDevice* device = heap_new(SerialDevice);

    DEVICE(device)->type = DEVICE_TYPE_CHAR;
    DEVICE(device)->kind = SER;
    DEVICE(device)->async = 1;
    DEVICE(device)->setopt = setopt;
    CHAR_DEVICE(device)->read = read;
    CHAR_DEVICE(device)->read_async = read_async;
    CHAR_DEVICE(device)->write = write;
    CHAR_DEVICE(device)->available = available;

    uint16_t port = BDA->com_ports[device_number];

    device->port = port;
    device->rx_write_pos = 0;
    device->rx_read_pos = 0;
    device->read = 0;
    memset(device->rx_buffer,0, RX_BUFFER_SIZE);

    outb(device->port + SERIAL_IER,     0x00);
    outb(device->port + SERIAL_LCR,     0x80);
    outb(device->port + SERIAL_DATA,    0x03);
    outb(device->port + SERIAL_IER,     0x00);
    outb(device->port + SERIAL_LCR,     0x03);
    outb(device->port + SERIAL_FIFO,    0xC7);
    outb(device->port + SERIAL_MCR,     0x0B);

    outb(device->port + SERIAL_IER,     0x01);

    isr_install(PIC_IRQ_BASE + PIC_IRQ_SERIAL, handle_irq, device);

    pic_enable(PIC_IRQ_SERIAL);
    pic_eoi1();

    return DEVICE(device);
}

static void handle_irq (InterruptFrame* f, void* data){
    SerialDevice* device = data;

    uint8_t read_data = inb(device->port + SERIAL_DATA);

    if (device->rx_read_pos != device->rx_write_pos || device->read == 0){
        device->rx_buffer[device->rx_write_pos++] = read_data;
        device->read++;
    
        if (device->rx_write_pos == RX_BUFFER_SIZE){
            device->rx_write_pos = 0;
        }
        if (device->request){
            if (handle_read(device, device->request)){
                device->request = NULL;
            }
        }
    }
    pic_eoi1();
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

static int16_t setopt(Device* device, uint32_t option, void* data){
    return 0;
}

static int16_t read(CharDevice* device){
    if (SERIAL_DEVICE(device)->read == 0){
        return -1;
    }
    int16_t data = SERIAL_DEVICE(device)->rx_buffer[SERIAL_DEVICE(device)->rx_read_pos++];

    if (SERIAL_DEVICE(device)->rx_read_pos == RX_BUFFER_SIZE){
        SERIAL_DEVICE(device)->rx_read_pos = 0;
    }
    SERIAL_DEVICE(device)->read--;

    return data;
}

static int recv_size(SerialDevice* device){
    return abs(((int)device->rx_read_pos) - ((int)device->rx_write_pos));
}

static int16_t handle_read(SerialDevice* device, IORequest* request){

    int amount = device->read;

    if (amount > 0){
        int to_read = min(amount, request->size);

        for (int i=0;i<to_read;i++){
            uint8_t data = (uint8_t) read(CHAR_DEVICE(device));

            handle_io_request(
                request, 
                &data,1,
                TASK_IO_REQUEST_PENDING
            );
        }
        if (amount >= request->size){
            handle_io_request(
                request,
                NULL,
                0,
                TASK_IO_REQUEST_DONE
            );
            return 1;
        } 
    } 
    return 0;
}

static int16_t read_async (CharDevice* device, IORequest* request){
    if (!handle_read(SERIAL_DEVICE(device), request)){
        SERIAL_DEVICE(device)->request = request;
    }
    return 0;
}

static int16_t write(CharDevice* device, uint8_t chr){
    outb(SERIAL_DEVICE(device)->port + SERIAL_DATA, chr);
    return 0;
}

static uint32_t available (CharDevice *device){
    return SERIAL_DEVICE(device)->read > 0;
}
