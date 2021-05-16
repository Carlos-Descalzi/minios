#ifndef _PCI_H_
#define _PCI_H_
/**
 * PCI
 **/

#define PCI_IO_CONFIG_ADDR  0xCF8
#define PCI_IO_CONFIG_DATA  0xCFC

#include "lib/stdint.h"

typedef struct {
    uint8_t type:7,
            mf: 1;
} HeaderType;

typedef struct {
    uint8_t 
        completion_code: 3,
        reserved: 2,
        start_bist: 1,
        bist_capable: 1;
} BistRegister;

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    HeaderType header_type;
    BistRegister bist;
} HeaderBaseFields;

typedef struct {
    uint32_t base_addresses[6];
    uint32_t cardbus_cis_pointer;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base_address;
    struct {
        uint32_t 
            capabilities_pointer: 8,
            reserved1: 24;
    };
    uint32_t reserved2;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
} Type00HeaderFields;

typedef struct {
    uint32_t base_addresses[2];
    uint8_t primary_bus_number;
    uint8_t secondary_bus_number;
    uint8_t subordinate_bus_number;
    uint8_t secondary_latency_timer;
    uint8_t io_base;
    uint8_t io_limit;
    uint16_t secondary_status;
    uint16_t memory_base;
    uint16_t memory_limit;
    uint16_t prefetchable_memory_base;
    uint16_t prefetchable_memory_limit;
    uint32_t prefetchable_base_upper;
    uint32_t prefetchable_limit_upper;
    uint16_t io_base_upper;
    uint16_t io_limit_upper;
    struct {
        uint32_t capabilites_pointer:8,
            reserved:24;
    };
    uint32_t expansion_rom_address;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
} Type01HeaderFields;

typedef struct {
    uint32_t cardbus_exca_base_address;
    uint8_t offset_cap_list;
    uint8_t reserved1;
    uint16_t secondary_status;
    uint8_t pci_bus_number;
    uint8_t cardbus_bus_number;
    uint8_t subordinate_bus_number;
    uint8_t cardbus_latency_timer;
    uint32_t memory_base_0;
    uint32_t memory_limit_0;
    uint32_t memory_base_1;
    uint32_t memory_limit_1;
    uint32_t io_base_0;
    uint32_t io_limit_0;
    uint32_t io_base_1;
    uint32_t io_limit_1;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
    uint16_t subsystem_device_id;
    uint16_t subsystem_vendor_id;
    uint32_t legacy_mode_base_address;
} Type02HeaderFields;

typedef struct {
    HeaderBaseFields base;
    union {
        Type00HeaderFields type00;
        Type01HeaderFields type01;
        Type02HeaderFields type02;
    };
} PCIHeader;

typedef void (*PciVisitor)(uint8_t,uint8_t,uint8_t, PCIHeader* header, void* user_data);

uint16_t    pci_config_read_w   (uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint32_t    pci_config_read_dw  (uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
void        pci_list_all_buses  (PciVisitor visitor, void* user_data);

#endif
