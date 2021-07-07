#include "kernel/device.h"
#include "board/pci.h"
#include "board/console.h"
#include "board/io.h"
#include "board/pic.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "lib/stdlib.h"
#include "misc/debug.h"
#include "kernel/isr.h"
#define BUFFER_SIZE 8192+16+1500

#define REG_RBSTART 0x30
#define REG_CR      0x37

#define CR_BUFE     0x01
#define CR_TE       0x04
#define CR_RE       0x08
#define CR_RST      0x10

#define REG_IMR     0x3C

#define IMR_ROK     0x0001
#define IMR_RER     0x0002
#define IMR_TOK     0x0004
#define IMR_TER     0x0008
#define IMR_RXOVW   0x0010
#define IMR_PLNKCH  0x0020
#define IMR_FOVW    0x0040
#define IMR_LENCH   0x2000
#define IMR_TIMOUT  0x4000
#define IMR_SERR    0x8000

#define REG_RCR     0x44

#define RCR_AAP     0x01
#define RCR_APM     0x02
#define RCR_AM      0x04
#define RCR_AB      0x08
#define RCR_AER     0x10
#define RCR_WRAP    0x80

#define REG_CONFIG1 0x52

#define REG_ISR     0x3e

#define ISR_ROK     0x01
#define ISR_RER     0x02
#define ISR_TOK     0x04
#define ISR_TER     0x08

#define REG_TX_STAT 0x10
#define REG_TX_ADDR 0x20

typedef struct {
    uint32_t status;
    uint32_t start;
} Descriptor;

typedef struct {
    BlockDevice device;
    int8_t      device_number;
    uint16_t    iobase;
    uint8_t     irq;
    uint8_t     mac_address[6];
    uint8_t*    rx_buffer;
    Descriptor  descritors[4];
    uint8_t     descritor_index;
} NetDevice;

static uint8_t  count_devices   (DeviceType* device_type);
static Device*  instantiate     (DeviceType* device_type, uint8_t device_number);
static void     release         (DeviceType* device_type, Device* device);
static int      check_pci       (uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header, void* data);
static int      count_pci       (uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header, void* data);
static void     get_mac_address (NetDevice* device, uint8_t* address);
static int16_t  read            (BlockDevice* device, uint8_t* buffer, uint16_t size);
static int16_t  read_async      (BlockDevice* device, IORequest* request);
static int16_t  write           (BlockDevice* device, uint8_t* buffer, uint16_t size);
static void     isr_handler     (InterruptFrame* frame, void* data);
static void     handle_receive  (NetDevice* device);
static int16_t  close           (BlockDevice* device);
static void     seek            (BlockDevice* device, uint32_t pos);
static void     flush           (BlockDevice* device);
static uint32_t pos             (BlockDevice* device);

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
    device->rx_buffer = heap_alloc(BUFFER_SIZE);
    
    debug("RTL8139 ethernet controller initialized, IOBase: ");
    debug_i(device->iobase,16);
    debug(",");debug("IRQ:");debug_i(pci_data.header.type00.interrupt_line,10);
    debug(",");debug("IRQ Pin:");debug_i(pci_data.header.type00.interrupt_pin,10);
    debug("\n");

    uint32_t irq = pci_config_read_dw(pci_data.bus, pci_data.device, pci_data.func, 0x3c);
    if (irq != 0xFFFFFFFF){
        device->irq = irq & 0xFF;
        debug("IRQ:");debug_i(device->irq,16);debug("\n");
    }


    outb(device->iobase + REG_CONFIG1,0x0); // wake up device.
    outb(device->iobase + REG_CR, CR_RST);    // reset
    while((inb(device->iobase + REG_CR) & CR_RST) != 0);

    outdw(device->iobase + REG_RBSTART, (uint32_t)device->rx_buffer); // send rx buffer address
    outw(device->iobase + REG_IMR, IMR_ROK | IMR_TOK);//IMR_ROK | IMR_TOK); 
    outdw(device->iobase + REG_RCR, RCR_AAP | RCR_APM | RCR_AM | RCR_AB |RCR_WRAP); 
    outb(device->iobase + REG_CR, CR_TE | CR_RE); // enable transmitter and receiver

    get_mac_address(device, device->mac_address);

    debug("Mac address:");
    debug_i(device->mac_address[0],16);debug(":");
    debug_i(device->mac_address[1],16);debug(":");
    debug_i(device->mac_address[2],16);debug(":");
    debug_i(device->mac_address[3],16);debug(":");
    debug_i(device->mac_address[4],16);debug(":");
    debug_i(device->mac_address[5],16);
    debug("\n");
    DEVICE(device)->type = DEVICE_TYPE_BLOCK;

    BLOCK_DEVICE(device)->read = read;
    BLOCK_DEVICE(device)->read_async = read_async;
    BLOCK_DEVICE(device)->write = write;
    BLOCK_DEVICE(device)->close = close;
    BLOCK_DEVICE(device)->seek = seek;
    BLOCK_DEVICE(device)->flush = flush;
    BLOCK_DEVICE(device)->pos = pos;

    device->descritor_index = 0;

    DEVICE(device)->async = 1;

    isr_install(irq + PIC_IRQ_BASE , isr_handler, device);
    
    return DEVICE(device);
}
static void get_mac_address (NetDevice* device, uint8_t* address){
    uint32_t mac1 = indw(device->iobase + 0x00);
    uint16_t mac2 = inw(device->iobase + 0x04);
    address[0] = (mac1 >> 0) & 0xFF;
    address[1] = (mac1 >> 8) & 0xFF;
    address[2] = (mac1 >> 16) & 0xFF;
    address[3] = (mac1 >> 24) & 0xFF;
    address[4] = (mac2 >> 0) & 0xFF;
    address[5] = (mac2 >> 8) & 0xFF;
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
static int16_t read_async(BlockDevice* device, IORequest* request){
    return 0;
}
static int16_t write (BlockDevice* device, uint8_t* buffer, uint16_t size){

    debug("RTL8139 - Write\n");
    int index = NET_DEVICE(device)->descritor_index;

    NET_DEVICE(device)->descritors[index].start = (uint32_t) buffer;
    NET_DEVICE(device)->descritors[index].status = size;

    outdw(NET_DEVICE(
        device)->iobase + 
        REG_TX_ADDR + index * 4,
        (uint32_t) buffer
    );

    outdw(NET_DEVICE(
        device)->iobase + 
        REG_TX_STAT + index * 4,
        size
    );

    NET_DEVICE(device)->descritor_index++;

    return 0;
}
static void isr_handler(InterruptFrame* frame, void* data){
    uint16_t status = inw(NET_DEVICE(data)->iobase + REG_ISR);

    debug("RTL interrupt\n");

    if (status & ISR_TOK){
        debug("Packet sent\n");
    }
    if (status & ISR_ROK){
        debug("Packet received\n");
        handle_receive(NET_DEVICE(data));
    }
    if (status & ISR_TER){
        debug("Transmit error\n");
    }
    if (status & ISR_RER){
        debug("Receive error\n");
    }

}
static void handle_receive(NetDevice* device){
}
static int16_t close(BlockDevice* device){
    isr_install(NET_DEVICE(device)->irq + PIC_IRQ_BASE, NULL, NULL);
    heap_free(NET_DEVICE(device)->rx_buffer);
    heap_free(device);
    return 0;
}
static void seek(BlockDevice* device, uint32_t pos){}
static void flush(BlockDevice* device){}
static uint32_t pos (BlockDevice* device){
    return 0;
}
static int16_t read (BlockDevice* device, uint8_t* buffer, uint16_t size){
    return 0;
}
