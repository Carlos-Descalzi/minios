#include "isr.h"

typedef struct {
    uint16_t offset1;
    uint16_t selector;
    uint8_t unused;
    uint8_t 
        gate_type: 4,
        storage_segment: 1,
        privilege_level: 2,
        present: 1;
    uint16_t offset2;
} IDTEntry;

/**
 * Overwrite existing IVR
 **/
#define IDT ((IDTEntry*)0x0)

static void INTERRUPT dummy(InterruptFrame* frame){}

void isr_install(uint16_t interrupt_number, Isr isr){
    IDTEntry* entry = &(IDT[interrupt_number]);
    if (!isr){
        isr = dummy;
    }

    entry->offset1 = ((uint32_t)isr) & 0xFFFF;
    entry->offset2 = ((uint32_t)isr) >> 16;
    entry->selector = 8;
    entry->gate_type = 0xE;
    entry->unused = 0;
    entry->present = 1;
    entry->storage_segment = 0;
}

inline void sti(void){
    asm volatile("sti");
}

inline void cli(void){
    asm volatile("cli");
}

inline void cld(void){
    asm volatile("cld");
}
