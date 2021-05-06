#include "pci.h"
#include "io.h"

#define HEADER_TYPE_STANDARD        0x00
#define HEADER_TYPE_PCI_PCI         0x01
#define HEADER_TYPE_CARDBUS_BRIDGE  0x02
#define HEADER_TYPE_MULTI_FUNCTION  0x80

typedef union PciAddress {
    struct __attribute__((__packed__)) {
        uint32_t 
            offset: 8,
            function: 3,
            device: 5,
            bus: 8,
            reserved: 7,
            enable: 1;
    };
    uint32_t address;
} PciAddress;

uint16_t pci_config_read_w  (uint8_t bus, uint8_t device, uint8_t func, uint8_t offset){

    PciAddress address = {
        .offset = offset & 0xFC,
        .function = func,
        .device = device,
        .bus = bus,
        .reserved = 0,
        .enable = 1
    };

    outdw(PCI_IO_CONFIG_ADDR, address.address);

    return (uint16_t) ((indw(PCI_IO_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}

uint32_t pci_config_read_dw(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset){
    PciAddress address = {
        .offset = offset & 0xFC,
        .function = func,
        .device = device,
        .bus = bus,
        .reserved = 0,
        .enable = 1
    };

    outdw(PCI_IO_CONFIG_ADDR, address.address);

    return indw(PCI_IO_CONFIG_DATA);
}

#define filled(a) (a[0] != 0 || a[1] != 0 || a[2] != 0 || a[3] != 0)

uint8_t get_header_base(uint8_t bus, uint8_t device, uint8_t function, HeaderBase* header){
    uint8_t i;
    uint32_t* raw_header = (uint32_t*)header;

    for (i=0;i<4;i++){
        raw_header[i] = pci_config_read_dw(bus, device, function, i*4);
    }
    return filled(raw_header);
}



static void check_function(uint8_t bus, uint8_t device, uint8_t function, PciVisitor visitor){
    HeaderBase header;

    if (get_header_base(bus,device,function, &header)){
        if(header.vendor_id != 0xFFFF){
            visitor(bus, device, function, &header);
        }
    }
}

static void check_device(uint8_t bus, uint8_t device, PciVisitor visitor){
    uint8_t function = 0;

    check_function(bus, device, 0, visitor);

}

void pci_list_all_buses(PciVisitor visitor){
    uint8_t bus;
    uint8_t device;

    for (bus = 0;bus <= 32;bus ++){ // todo check all buses
        for (device = 0;device < 32;device ++){
            check_device(bus, device, visitor);
        }
    }
}

