#define NODEBUG
#include "kernel/isr.h"
#include "misc/debug.h"
#include "lib/string.h"
#include "board/pic.h"
#include "kernel/task.h"
#include "board/memory.h"

typedef void (*LowLevelIsr)(InterruptFrame*);

typedef struct {
    Isr isr;
    void* callback_data;
} IsrHandler;

static IsrHandler isr_handlers[96];

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

static void dummy_trap_handler(InterruptFrame* frame, void* data);
static void dummy_isr_pic_handler(InterruptFrame* frame, void* data);
static void dummy_isr_handler(InterruptFrame* frame, void* data);
static void handle_isr(uint32_t isr_num, InterruptFrame* frame);

void* handle_isr_ref = handle_isr; // FIXME, used by assembly code.

static void do_isr_install(uint16_t interrupt_number, uint32_t isr, uint8_t type){
    IDTEntry* entry = &(IDT[interrupt_number]);
    entry->privilege_level = 3;
    entry->offset1 = isr & 0xFFFF;
    entry->offset2 = isr >> 16;
    entry->selector = 8;
    entry->gate_type = type;
    entry->unused = 0;
    entry->present = 1;
    entry->storage_segment = 0;
}
void isr_install(uint16_t interrupt_number, Isr isr, void* callback_data){
    if (interrupt_number < 0x20){
        // < 32 is for traps
        return;
    }
    if (!isr){
        if (interrupt_number >= 0x20 && interrupt_number <= 0x2F){
            isr = dummy_isr_pic_handler;
        } else {
            isr = dummy_isr_handler;
        }
    }
    isr_handlers[interrupt_number].isr = isr;
    isr_handlers[interrupt_number].callback_data = callback_data;
}
void trap_install(uint16_t interrupt_number, Isr isr, void* callback_data){
    if (interrupt_number < 32){
        if (!isr){
            isr = dummy_trap_handler;
        }
        isr_handlers[interrupt_number].isr = isr;
        isr_handlers[interrupt_number].callback_data = callback_data;
    }
}
static void handle_isr(uint32_t isr_num, InterruptFrame* frame){
    cli();
    int user_space = (frame->cr3 & 3) != 0;
    if (user_space){
        asm volatile(
            "\tmov %0, %%eax\n"
            "\tmov %%eax,%%cr3\n"
            "\tmov $0x10, %%eax\n"
            "\tmov %%ax, %%ds\n"
            "\tmov %%ax, %%es\n"
            "\tmov %%ax, %%gs\n"
            "\tmov %%ax, %%fs\n"
            ::"Nd"(KERNEL_PAGE_DIR_ADDRESS)
        );
        //debug(">>>>");debug_i(frame->cr3 & 3,16);debug("\n");
        tasks_update_current(frame);
    } else {
        debug("Interrupt in kernel space\n");
    }
    sti();

    isr_handlers[isr_num].isr(frame, isr_handlers[isr_num].callback_data);
    cli();
    if (user_space){
        asm volatile(
            "\tmov %0, %%eax\n"
            "\tmov %%eax, %%cr3\n"
            "\tmov $0x2b, %%eax\n"
            "\tmov %%ax, %%ds\n"
            "\tmov %%ax, %%es\n"
            "\tmov %%ax, %%fs\n"
            "\tmov %%ax, %%gs\n"
            ::"Nd"(frame->cr3)
        );
    } else {
        debug("Exit from interrupt in kernel space\n");
    }
    sti();
}

inline void sti(void){
    //debug("int enabled\n");
    asm volatile("sti");
}

inline void cli(void){
    //debug("int disabled\n");
    asm volatile("cli");
}
inline void cld(void){
    asm volatile("cld");
}

/**
 * I use this macro to adjust handler addresses to the virtual
 * address mapped on paging (last page of last table)
 **/
#define ADJUST_PAGING(address) (0xFFFFF000 - ((uint32_t)isr_handlers_start) + address)

#define ISR_SETUP(n) {\
    extern LowLevelIsr handle_isr_##n(InterruptFrame);\
    do_isr_install(n, ADJUST_PAGING((uint32_t)handle_isr_##n), 0xe); \
}

void isr_init(){
    int i;

    for (i=0;i<32;i++){
        isr_handlers[i].isr = dummy_trap_handler;
        isr_handlers[i].callback_data = NULL;
    }
    for (;i<48;i++){
        isr_handlers[i].isr = dummy_isr_pic_handler;
        isr_handlers[i].callback_data = NULL;
    }
    for (;i<96;i++){
        isr_handlers[i].isr = dummy_isr_handler;
        isr_handlers[i].callback_data = NULL;
    }
    ISR_SETUP(0x00);
    ISR_SETUP(0x01);
    ISR_SETUP(0x02);
    ISR_SETUP(0x03);
    ISR_SETUP(0x04);
    ISR_SETUP(0x05);
    ISR_SETUP(0x06);
    ISR_SETUP(0x07);
    ISR_SETUP(0x08);
    ISR_SETUP(0x09);
    ISR_SETUP(0x0a);
    ISR_SETUP(0x0b);
    ISR_SETUP(0x0c);
    ISR_SETUP(0x0d);
    ISR_SETUP(0x0e);
    ISR_SETUP(0x0f);
    ISR_SETUP(0x10);
    ISR_SETUP(0x11);
    ISR_SETUP(0x12);
    ISR_SETUP(0x13);
    ISR_SETUP(0x14);
    ISR_SETUP(0x1e);
    ISR_SETUP(0x1f);
    ISR_SETUP(0x20);
    ISR_SETUP(0x21);
    ISR_SETUP(0x22);
    ISR_SETUP(0x23);
    ISR_SETUP(0x24);
    ISR_SETUP(0x25);
    ISR_SETUP(0x26);
    ISR_SETUP(0x27);
    ISR_SETUP(0x28);
    ISR_SETUP(0x29);
    ISR_SETUP(0x2a);
    ISR_SETUP(0x2b);
    ISR_SETUP(0x2c);
    ISR_SETUP(0x2d);
    ISR_SETUP(0x2e);
    ISR_SETUP(0x2f);
    ISR_SETUP(0x30);
    ISR_SETUP(0x31);
    ISR_SETUP(0x32);
    ISR_SETUP(0x33);
    ISR_SETUP(0x34);
    ISR_SETUP(0x35);
    ISR_SETUP(0x36);
    ISR_SETUP(0x37);
    ISR_SETUP(0x38);
    ISR_SETUP(0x39);
    ISR_SETUP(0x3a);
    ISR_SETUP(0x3b);
    ISR_SETUP(0x3c);
    ISR_SETUP(0x3d);
    ISR_SETUP(0x3e);
    ISR_SETUP(0x3f);
    ISR_SETUP(0x40);
    ISR_SETUP(0x41);
    ISR_SETUP(0x42);
    ISR_SETUP(0x43);
    ISR_SETUP(0x44);
    ISR_SETUP(0x45);
    ISR_SETUP(0x46);
    ISR_SETUP(0x47);
    ISR_SETUP(0x48);
    ISR_SETUP(0x49);
    ISR_SETUP(0x4a);
    ISR_SETUP(0x4b);
    ISR_SETUP(0x4c);
    ISR_SETUP(0x4d);
    ISR_SETUP(0x4e);
    ISR_SETUP(0x4f);
    ISR_SETUP(0x50);
    ISR_SETUP(0x51);
    ISR_SETUP(0x52);
    ISR_SETUP(0x53);
    ISR_SETUP(0x54);
    ISR_SETUP(0x55);
    ISR_SETUP(0x56);
    ISR_SETUP(0x57);
    ISR_SETUP(0x58);
    ISR_SETUP(0x59);
    ISR_SETUP(0x5a);
    ISR_SETUP(0x5b);
    ISR_SETUP(0x5c);
    ISR_SETUP(0x5d);
    ISR_SETUP(0x5e);
    ISR_SETUP(0x5f);
}

static void dummy_trap_handler(InterruptFrame* frame, void* data){
    debug("Trap!\n");
    asm volatile("cli");
    asm volatile("hlt");
}

static void dummy_isr_pic_handler(InterruptFrame* frame, void* data){
    uint8_t val = pic_get_irq_reg();
    debug("PIC interrupt:");debug_i(val,16);debug("\n");
    if (val & 0xF){
        pic_eoi1();
    } 
    if (val & 0xF0){
        pic_eoi2();
    }
}

static void dummy_isr_handler(InterruptFrame* frame, void* data){
    debug("ISR\n");
}

