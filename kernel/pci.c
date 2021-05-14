#include "pci.h"
#include "io.h"
#include "string.h"

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

static void check_bus(uint8_t bus, PCIHeader* header, PciVisitor visitor, void* user_data);
static void check_device(uint8_t bus, uint8_t device, PCIHeader* header, PciVisitor visitor, void* user_data);
static void check_function(uint8_t bus, uint8_t device, uint8_t function, PCIHeader* header, PciVisitor visitor, void* user_data);

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

uint8_t get_header(uint8_t bus, uint8_t device, uint8_t function, PCIHeader* header){
    uint8_t i;
    uint32_t* raw_header = (uint32_t*)header;
    size_t header_fields_size;
    memset(header,0,sizeof(PCIHeader));

    for (i=0;i<4;i++){
        raw_header[i] = pci_config_read_dw(bus, device, function, i*4);
    }
    if (header->base.header_type.type == 0){
        header_fields_size = sizeof(Type00HeaderFields) >> 2;
    } else if (header->base.header_type.type == 1){
        header_fields_size = sizeof(Type01HeaderFields) >> 2;
    } else {
        header_fields_size = sizeof(Type01HeaderFields) >> 2;
    }
    for (;i<header_fields_size;i++){
        raw_header[i] = pci_config_read_dw(bus, device, function, i*4);
    }
    return filled(raw_header);
}

void pci_list_all_buses(PciVisitor visitor, void* user_data){
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    PCIHeader header;

    get_header(0, 0, 0, &header);

    if (header.base.header_type.mf){
        for(function = 0; function < 8;function++){
            get_header(0,0,function,&header);
            if(header.base.vendor_id != 0xFFFF) break;
            check_bus(function,&header,visitor, user_data);
        }
    } else {
        check_bus(0, &header, visitor, user_data);
    }

    //for (bus = 0;bus <= 255;bus ++){ // todo check all buses
    //    check_bus(bus, &header, visitor);
    //}
}


static void check_bus(uint8_t bus, PCIHeader* header, PciVisitor visitor, void* user_data){
    uint8_t device;

    for (device=0;device < 32;device++){
        check_device(bus, device, header, visitor, user_data);
    }
}
static void check_function(uint8_t bus, uint8_t device, uint8_t function, PCIHeader* header, PciVisitor visitor, void* user_data){
    visitor(bus, device, function, header, user_data);

    if (header->base.class == 0x06 && header->base.subclass == 0x04){
        if (header->base.header_type.type == 1){
            check_bus(header->type01.secondary_bus_number, header, visitor, user_data);
        } 
    }
}

static void check_device(uint8_t bus, uint8_t device, PCIHeader* header, PciVisitor visitor, void* user_data){
    uint8_t function = 0;
    if (get_header(bus,device,function, header)){
        if(header->base.vendor_id != 0xFFFF){
            visitor(bus, device, function, header, user_data);
            if (header->base.header_type.mf){
                for(function=0;function<8;function++){
                    if (get_header(bus,device,function, header)){
                        if (header->base.vendor_id != 0xFFFF){
                            visitor(bus,device, function,header, user_data);
                        }
                    }
                }
            }
        }
    }
}

