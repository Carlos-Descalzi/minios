#define NODEBUG
#include "console.h"
#include "kernel/device.h"
#include "lib/heap.h"
#include "misc/debug.h"
#include "lib/string.h"
#include "kernel/task.h"

typedef struct {
    CharDevice device;
    CharDevice* screen;
    CharDevice* keyboard;
    IORequest* user_request;
    IORequest console_request;
    uint8_t buffer[2];
} ConsoleDevice;

#define CONSOLE_DEVICE(d)   ((ConsoleDevice*)d)

static uint8_t  count_devices   (DeviceType* device_type);
static Device*  instantiate     (DeviceType* device_type, uint8_t device_number);
static void     release         (DeviceType* device_type, Device* device);
static void     request_callback(IORequest* request, void* data);

static DeviceType DEVICE_TYPE = {
    kind: TERM,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};

static int16_t  console_write           (CharDevice* device, uint8_t data);
static int16_t  console_read_async      (CharDevice* device, IORequest* request);
static void     reset_console_request   (ConsoleDevice* device);

void console_register(){
    device_register_type((DeviceType*)&DEVICE_TYPE);
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

static void reset_console_request(ConsoleDevice* device){
    device->console_request.dsize = 0;
    device->console_request.size = 2;
    device->console_request.kernel = 1;
    device->user_request = NULL;
    device->console_request.callback_data = device;
}

static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    ConsoleDevice* device = heap_alloc(sizeof(ConsoleDevice));
    memset(device,0,sizeof(ConsoleDevice));
    device->screen = CHAR_DEVICE(device_find(VIDEO,0));
    device->keyboard = CHAR_DEVICE(device_find(KBD,0));
    device->console_request.callback = request_callback;
    device->console_request.target_buffer = device->buffer;
    reset_console_request(device);
    DEVICE(device)->async = 1;
    CHAR_DEVICE(device)->read_async = console_read_async;
    debug("Char device:");
    debug_i(device,16);
    debug(",");
    debug_i(CHAR_DEVICE(device)->read_async,16);
    debug("\n");
    CHAR_DEVICE(device)->write = console_write;
    return DEVICE(device);
}

static uint8_t count_devices(struct DeviceType* device_type){
    return 1;
}

static int16_t console_write(CharDevice* device, uint8_t data){
    return char_device_write(CONSOLE_DEVICE(device)->screen,data);
}

static int16_t  console_read_async  (CharDevice* device, IORequest* request){
    debug("Console read async\n");
    CONSOLE_DEVICE(device)->user_request = request;
    return char_device_read_async(
        CONSOLE_DEVICE(device)->keyboard, 
        &(CONSOLE_DEVICE(device)->console_request)
    );
}

static void request_callback(IORequest* request, void* data){
    ConsoleDevice* device = data;
    debug("Request callback\n");
    uint16_t code = *((uint16_t*)device->console_request.target_buffer);
    debug_i(code,16);
    debug("\n");

    if (device->console_request.target_buffer[1] & 0x80){
        uint8_t* buffer;
        debug("Key press\n");
        if (device->user_request->kernel){
            buffer = device->user_request->target_buffer;
        } else {
            buffer = tasks_task_to_kernel_adddress(
                device->user_request->tid, 
                device->user_request->target_buffer
            );
        }
        buffer[0] = device->console_request.target_buffer[0];
        device->user_request->dsize = 1;
        device->user_request->result = 0;
        device->user_request->status = TASK_IO_REQUEST_DONE;

        if (device->user_request->callback){
            device->user_request->callback(
                device->user_request,
                device->user_request->callback_data
            );
        }
        reset_console_request(device);
    } else {
        debug("Key release discarded\n");
        
        device->console_request.size = 2;
        device->console_request.dsize = 0;
        memset(device->console_request.target_buffer,0,2);
        char_device_read_async(
            CONSOLE_DEVICE(device)->keyboard,
            &(CONSOLE_DEVICE(device)->console_request)
        );
        
    }

}
