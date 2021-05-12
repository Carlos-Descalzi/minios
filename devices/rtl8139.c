#include "device.h"
#include "rtl8139.h"
#include "string.h"
#include "pci.h"
#include "console.h"
#include "heap.h"
#include "stdlib.h"
#include "io.h"

static uint8_t count_devices(DeviceType* device_type);
static Device* instantiate(DeviceType* device_type, uint8_t device_number);
static void release(DeviceType* device_type, Device* device);
static void check_pci(uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header);

typedef struct {
    BlockDevice device;
    int8_t device_number;
    uint16_t iobase;
    uint8_t* rx_buffer;
} NetDevice;

#define MAX_DEVICES 10

static int device_count;

static struct {
    uint8_t bus;    // -1 means no more 
    uint8_t device;
    uint8_t func;
    PCIHeader pci;
} avail_net_devices[MAX_DEVICES];

#define NET_DEVICE(d) ((NetDevice*)d)

static DeviceType DEVICE_TYPE = {
    kind: NET,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};

void rtl8139_register(){
    device_register_type((DeviceType*)&DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    device_count = 0;
    memset(avail_net_devices,0,sizeof(avail_net_devices));
    pci_list_all_buses(check_pci);
    return device_count;
}
static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    char buff[8];
    NetDevice* device = heap_alloc(sizeof(NetDevice));
    if (!device){
        console_print("Error: No memory for initializing device\n");
        return NULL;
    } else {
        PCIHeader* pci = &(avail_net_devices[device_number].pci);
        device->device_number = device_number;
        
        if (pci->base.header_type.type == 0){
            device->iobase = pci->type00.base_addresses[0] & ~0x0001;
        } else {
            device->iobase = pci->type01.base_addresses[1] & ~0x0001;
        }
        device->rx_buffer = heap_alloc(8192+16);

        outb(device->iobase + 0x52,0x0); // wake up device.
        outdw(device->iobase + 0x30, (uint32_t)device->rx_buffer); // send rx buffer address
        
        console_print("RTL8139 ethernet controller initialized, IOBase: ");
        console_print(itoa(device->iobase,buff,16));
        console_print("\n");
        return DEVICE(device);
    }
}
static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}
static void check_pci(uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header){
    if (header->base.class == 0x02 
        && header->base.subclass == 0x00 
        && header->base.vendor_id == 0x10EC
        && header->base.device_id == 0x8139){
        memcpy(&(avail_net_devices[device_count].pci),header,sizeof(PCIHeader));
        device_count++;
    }
}
