#include "kernel/device.h"
#include "board/pci.h"
#include "board/console.h"
#include "board/io.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "lib/stdlib.h"
#include "misc/debug.h"

#define REG_RBSTART 0x30
#define REG_CR      0x37
#define REG_IMR     0x3C

#define IMR_ROK     (1 << 0)
#define IMR_RER     (1 << 1)
#define IMR_TOK     (1 << 2)
#define IMR_TER     (1 << 3)
#define IMR_RXOVW   (1 << 4)
#define IMR_PLNKCH  (1 << 5)
#define IMR_FOVW    (1 << 6)
#define IMR_LENCH   (1 << 13)
#define IMR_TIMOUT  (1 << 14)
#define IMR_SERR    (1 << 15)

#define REG_RCR     0x44
#define REG_CONFIG1 0x52

static uint8_t  count_devices   (DeviceType* device_type);
static Device*  instantiate     (DeviceType* device_type, uint8_t device_number);
static void     release         (DeviceType* device_type, Device* device);
static int      check_pci       (uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header, void* data);
static int      count_pci       (uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header, void* data);

typedef struct {
    BlockDevice device;
    int8_t device_number;
    uint16_t iobase;
    uint8_t irq;
    uint8_t* rx_buffer;
} NetDevice;

#define NET_DEVICE(d) ((NetDevice*)d)

static DeviceType DEVICE_TYPE;

void module_init(){
    DEVICE_TYPE.kind = NET;
    DEVICE_TYPE.count_devices = count_devices;
    DEVICE_TYPE.instantiate = instantiate;
    DEVICE_TYPE.release = release;
    device_register_type(&DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    int device_count = 0;
    pci_list_all_buses(count_pci, &device_count);
    return device_count ? 1 : 0; // TODO: Support multiple devices
}

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t func;
    PCIHeader header;
} PCIData;

static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    //PCIHeader pci_header;
    PCIData pci_data;

    memset(&pci_data, 0, sizeof(PCIData));

    pci_list_all_buses(check_pci, &pci_data);

    if (pci_data.header.base.device_id != 0x8139){
        debug("No device found\n");
        return NULL;
    }

    NetDevice* device = heap_alloc(sizeof(NetDevice));

    if (!device){
        console_print("Error: No memory for initializing device\n");
        return NULL;
    } 
    device->device_number = device_number;
    
    if (pci_data.header.base.header_type.type == 0){
        debug("header type 0\n");
        device->iobase = pci_data.header.type00.base_addresses[0] & ~0x0001;
    } else {
        debug("header type 1\n");
        device->iobase = pci_data.header.type01.base_addresses[0] & ~0x0001;
    }
    device->rx_buffer = heap_alloc(8192+16);
    
    debug("RTL8139 ethernet controller initialized, IOBase: ");
    debug_i(device->iobase,16);
    debug(",");debug("IRQ:");debug_i(pci_data.header.type00.interrupt_line,10);
    debug(",");debug("IRQ Pin:");debug_i(pci_data.header.type00.interrupt_pin,10);
    debug("\n");

    uint32_t irq = pci_config_read_dw(pci_data.bus, pci_data.device, pci_data.func, 0x3c);
    if (irq != 0xFFFFFFFF){
        device->irq = irq & 0xFF;
        debug("IRQ:");debug_i(irq,16);debug("\n");
    }

    outb(device->iobase + REG_CONFIG1,0x0); // wake up device.
    outb(device->iobase + REG_CR, 0x10);    // reset
    while( (inb(device->iobase + REG_CR) & 0x10) != 0) { }

    outdw(device->iobase + REG_RBSTART, (uint32_t)device->rx_buffer); // send rx buffer address
    outw(device->iobase + REG_IMR, 0xFFFF);//IMR_ROK | IMR_TOK); 
    outdw(device->iobase + REG_RCR, 0x8f); 
    outb(device->iobase + REG_CR, 0x0c);
    
    return DEVICE(device);
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

static int valid_pci_dev(PCIHeader* header){
    return header->base.class == 0x02 
        && header->base.subclass == 0x00 
        && header->base.vendor_id == 0x10EC
        && header->base.device_id == 0x8139;
}

static int count_pci(uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header, void* data){
    if (valid_pci_dev(header)){
        (*((int*)data))++;
        return 1;
    }
    return 0;
}
static int check_pci(uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header, void* data){
    if (valid_pci_dev(header)){
        PCIData* pci_data = data;
        pci_data->bus = bus;
        pci_data->device = device;
        pci_data->func = func;
        memcpy(&(pci_data->header), header, sizeof(PCIHeader));
        return 1;
    }
    return 0;
}
