#include "kernel/paging.h"
#include "kernel/isr.h"
#include "lib/string.h"
#include "misc/debug.h"
#include "board/console.h"
#include "board/memory.h"
/**
 * Paging
 * https://www.scs.stanford.edu/05au-cs240c/lab/i386/s05_02.htm
 **/

// Use the available RAM region of ~30kb which is behind boot sector,
// for page directory.
// The region starts at 0x500 but I need 4k aligned addresses.
#define KERNEL_PAGE_DIR     ((PageDirectoryEntry*)KERNEL_PAGE_DIR_ADDRESS)
#define KERNEL_PAGE_TABLE   ((PageTableEntry*)KERNEL_PAGE_TABLE_ADDRESS)
#define PD_USED             0x1
#define PD_KERNEL           0x4
#define PAGE_LAST           1023

static void     handle_page_fault   (InterruptFrame frame);
static void     setup_page_table    (PageTableEntry* page_table);

void paging_init(){
    int i;
    memset(KERNEL_PAGE_DIR, 0, sizeof(PageDirectoryEntry)*1024);
    memset(KERNEL_PAGE_TABLE,0, sizeof(PageTableEntry)*1024);

    KERNEL_PAGE_DIR[0].present = 1;
    KERNEL_PAGE_DIR[0].read_write = 1;
    KERNEL_PAGE_DIR[0].user_supervisor = 0;
    KERNEL_PAGE_DIR[0].page_size = 0;
    KERNEL_PAGE_DIR[0].page_table_address = ((uint32_t)KERNEL_PAGE_TABLE) >> 12;
    KERNEL_PAGE_DIR[0].user_data = PD_USED | PD_KERNEL;

    // need 256 pages to cover the first 1Mb
    // These pages are 1:1 with physical memory
    for (i=0;i<256;i++){
        KERNEL_PAGE_TABLE[i].present = 1;
        KERNEL_PAGE_TABLE[i].read_write = 1;
        KERNEL_PAGE_TABLE[i].user_supervisor = 0;
        KERNEL_PAGE_TABLE[i].physical_page_address = i;
    }

    KERNEL_PAGE_DIR[PAGE_LAST].present = 1;
    KERNEL_PAGE_DIR[PAGE_LAST].read_write = 0;
    KERNEL_PAGE_DIR[PAGE_LAST].user_supervisor = 0;
    KERNEL_PAGE_DIR[PAGE_LAST].page_size = 0;
    KERNEL_PAGE_DIR[PAGE_LAST].page_table_address = KERNEL_UPPER_PAGE_TABLE_ADDRESS >> 12;
    KERNEL_PAGE_DIR[PAGE_LAST].user_data = PD_USED | PD_KERNEL;

    setup_page_table((PageTableEntry*)KERNEL_UPPER_PAGE_TABLE_ADDRESS);

    asm volatile (
        "\tmov %0, %%eax\n"
        "\tmov %%eax, %%cr3\n"
        "\tmov %%cr0, %%eax\n"
        "\tor  $0x80000001, %%eax\n"
        "\tmov %%eax, %%cr0"
        :: "Nd"(KERNEL_PAGE_DIR)
    );
    debug("CR3:");debug_i((uint32_t)KERNEL_PAGE_DIR,16);debug("\n");

    trap_install(0xE, handle_page_fault);
    console_print("Paging initialized\n");
}
PageDirectoryEntry* paging_get_current_directory(void){
    PageDirectoryEntry* directory;
    asm volatile( "\tmov %%cr3, %0\n" : "=a"(directory));
    return directory;
}
void paging_set_directory (PageDirectoryEntry* directory){
    asm volatile(
        "\tmov %0, %%eax\n"
        "\tmov %%eax, %%cr3\n"
        :: "Nd"(directory)
    );
}

PageDirectoryEntry* paging_new_page_directory(void){
    PageDirectoryEntry* directory = (PageDirectoryEntry*)memory_alloc_block();
    debug("Allocated page directory at ");debug_i((uint32_t)directory,16);debug("\n");

    if (directory){
        uint32_t table_address = memory_alloc_block();
        if (!table_address){
            memory_free_block((uint32_t)directory);
            return NULL;
        }
        memset(directory, 0, MEMORY_BLOCK_SIZE);
        /**
         * Allocate last page directory entry with one page table
         * for interrupts
         **/
        directory[PAGE_LAST].present = 1;
        directory[PAGE_LAST].read_write = 0;
        directory[PAGE_LAST].user_supervisor = 0;
        directory[PAGE_LAST].page_size = 0;
        directory[PAGE_LAST].page_table_address = table_address >> 12;
        directory[PAGE_LAST].user_data = PD_USED;

        setup_page_table((PageTableEntry*)table_address);


        return directory;
    }
    return NULL;
}
uint32_t paging_alloc_table(PageDirectoryEntry* directory){
    return 0;
}
static void handle_page_fault(InterruptFrame frame){
    debug("Page fault!!!\n");
}

static void setup_page_table(PageTableEntry* page_table){
    // Last page table goes for ISR table
    // All page directories will have this page mapped.
    page_table[PAGE_LAST].present = 1;
    page_table[PAGE_LAST].read_write = 0;
    page_table[PAGE_LAST].user_supervisor = 0;
    page_table[PAGE_LAST].user_data = PD_USED;
    page_table[PAGE_LAST].physical_page_address = ((uint32_t)isr_handlers_start) >> 12;
    debug("ISR handlers physical address:");debug_i((uint32_t)isr_handlers_start,16);debug("\n");
    debug("Last page of table directory used to map ISR handlers: ");
    debug_i((PAGE_LAST << 22) | (PAGE_LAST << 12),16);debug("\n");
}
