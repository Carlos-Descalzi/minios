#include "paging.h"
#include "string.h"
#include "isr.h"
#include "console.h"

// Use the available RAM region of ~30kb which is behind boot sector,
// for page directory.
// The region starts at 0x500 but I need 4k aligned addresses.
#define PAGE_DIRECTORY  ((PageDirectoryEntry*)0x1000)
// Kernel page table, is table#0 in page directory
#define KERNEL_PAGE_TABLE ((PageTableEntry*)0x2000)

#define PD_USED     0x1
#define PD_KERNEL   0x4

static void handle_page_fault(InterruptFrame* frame);

void paging_init(){
    int i;
    memset(PAGE_DIRECTORY, 0, sizeof(PageDirectoryEntry)*1024);
    memset(KERNEL_PAGE_TABLE,0, sizeof(PageTableEntry)*1024);

    PAGE_DIRECTORY[0].present = 1;
    PAGE_DIRECTORY[0].read_write = 1;
    PAGE_DIRECTORY[0].user_supervisor = 0;
    PAGE_DIRECTORY[0].page_size = 0;
    PAGE_DIRECTORY[0].page_table_address = ((uint32_t)KERNEL_PAGE_TABLE) >> 12;
    PAGE_DIRECTORY[0].user_data = PD_USED | PD_KERNEL;

    // need 256 pages to cover the first 1Mb
    // These pages are 1:1 with physical memory
    for (i=0;i<256;i++){
        KERNEL_PAGE_TABLE[i].present = 1;
        KERNEL_PAGE_TABLE[i].read_write = 1;
        KERNEL_PAGE_TABLE[i].user_supervisor = 0;
        KERNEL_PAGE_TABLE[i].physical_page_address = i;
    }

    asm volatile (
        "\tmov %0, %%eax\n"
        "\tmov %%eax, %%cr3\n"
        "\tmov %%cr0, %%eax\n"
        "\tor  $0x80000001, %%eax\n"
        "\tmov %%eax, %%cr0"
        :: "Nd"(PAGE_DIRECTORY)
    );

    isr_install(0xE, handle_page_fault);
}

int16_t paging_alloc_table(PageDirectoryEntry** entry_ptr){
    uint16_t i;
    for (i=0;i<1024;i++){
        if (PAGE_DIRECTORY[i].user_data & PD_USED == 0){
            PAGE_DIRECTORY[i].user_data |= PD_USED;
            *entry_ptr = &(PAGE_DIRECTORY[i]);
            return i;
        }
    }
    return -1;
}

void paging_free_table(uint16_t page_table_index){
    PAGE_DIRECTORY[page_table_index].user_data &= ~PD_USED;
}
static void INTERRUPT handle_page_fault(InterruptFrame* frame){
    console_init("Page fault!!!\n");
}
