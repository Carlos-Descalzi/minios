#include "console.h"
#include "pci.h"
#include "io.h"
#include "pit.h"
#include "pic.h"
#include "isr.h"
#include "stdlib.h"

const char BANNER[] = "*************\n** MINI OS **\n*************\n";

static uint8_t timer_count;
static char buff[16];

static void timer_handler();
static void show_pci_entry(uint8_t bus, uint8_t device, uint8_t func, HeaderBase* header);

void init(){
    console_init();
    console_print(BANNER);
    console_gotoxy(0,4);
    console_print("PCI Devices:\n");

    pci_list_all_buses(show_pci_entry);

    console_print("------\nTesting timer:\n");

    pit_init();
    pit_set_freq(1);
    timer_count = 0;
    isr_install(8, timer_handler); // TODO: Remap PIC
    sti();
    while(timer_count < 3);
    cli();
        
}

static void show_pci_entry(uint8_t bus, uint8_t device, uint8_t func, HeaderBase* header){
    console_print("Bus:");
    console_print(itoa(bus,buff,16));
    console_print(" Dev:");
    console_print(itoa(device,buff,16));
    console_print(" Vendor:");
    console_print(itoa(header->vendor_id,buff,16));
    console_print(" Device:");
    console_print(itoa(header->device_id,buff,16));
    console_print(" Class:");
    console_print(itoa(header->class,buff,16));
    console_print(" Subclass:");
    console_print(itoa(header->subclass,buff,16));
    console_put('\n');
}

static void INTERRUPT timer_handler(InterruptFrame* frame){
    console_print("Timer called ");
    timer_count++;
    console_print(itoa(timer_count,buff,10));
    console_put('\n');
    eoi();
}
