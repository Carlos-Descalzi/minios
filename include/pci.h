#ifndef _PCI_H_
#define _PCI_H_
/**
 * PCI
 **/

#define PCI_IO_CONFIG_ADDR  0xCF8
#define PCI_IO_CONFIG_DATA  0xCFC

#include "stdint.h"

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
} HeaderBase;

typedef struct {
    HeaderBase base_header;
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
} Type00Header;

typedef struct {
    HeaderBase base_header;
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

} Type01Header;

typedef void (*PciVisitor)(uint8_t,uint8_t,uint8_t, HeaderBase* header);

uint16_t    pci_config_read_w   (uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint32_t    pci_config_read_dw  (uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
void        pci_list_all_buses  (PciVisitor visitor);

#endif
