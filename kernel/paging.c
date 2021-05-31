#include "kernel/paging.h"
#include "kernel/isr.h"
#include "lib/string.h"
#include "misc/debug.h"
#include "board/console.h"
#include "board/memory.h"
#include "bin/elf.h"
/**
 * Paging
 * https://www.scs.stanford.edu/05au-cs240c/lab/i386/s05_02.htm
 **/

// Use the available RAM region of ~30kb which is behind boot sector,
// for page directory.
// The region starts at 0x500 but I need 4k aligned addresses.
#define KERNEL_PAGE_DIR         ((PageDirectoryEntry*)KERNEL_PAGE_DIR_ADDRESS)
#define KERNEL_PAGE_TABLE       ((PageTableEntry*)KERNEL_PAGE_TABLE_ADDRESS)
#define PD_USED                 0x1
#define PAGE_LAST               1023
#define KERNEL_EXCHANGE_PAGE    (KERNEL_PAGE_TABLE[PAGE_LAST])
#define TO_PAGES(s)    ((s / PAGE_SIZE) + (s % PAGE_SIZE ? 1 : 0))

static void     handle_page_fault       (InterruptFrame frame);
static void     setup_page_table        (PageTableEntry* page_table,uint8_t rw, uint8_t us);
static void     setup_page_table_user   (PageTableEntry* page_table,uint8_t rw, uint8_t us);

void paging_init(){
    int i;
    memset(KERNEL_PAGE_DIR, 0, sizeof(PageDirectoryEntry)*1024);
    memset(KERNEL_PAGE_TABLE,0, sizeof(PageTableEntry)*1024);

    KERNEL_PAGE_DIR[0].present = 1;
    KERNEL_PAGE_DIR[0].read_write = 1;
    KERNEL_PAGE_DIR[0].user_supervisor = 0;
    KERNEL_PAGE_DIR[0].page_size = 0;
    KERNEL_PAGE_DIR[0].page_table_address = ((uint32_t)KERNEL_PAGE_TABLE) >> 12;
    KERNEL_PAGE_DIR[0].user_data = PD_USED;

    // need 256 pages to cover the first 1Mb
    // These pages are 1:1 with physical memory
    for (i=0;i<256;i++){
        KERNEL_PAGE_TABLE[i].present = 1;
        KERNEL_PAGE_TABLE[i].read_write = 1;
        KERNEL_PAGE_TABLE[i].user_supervisor = 0;
        KERNEL_PAGE_TABLE[i].physical_page_address = i;
    }

    // Use this last page of first table
    // For moving data into the physical address
    // of a given user page, so physycal address
    // will be replaced when needed.
    KERNEL_PAGE_TABLE[PAGE_LAST].present = 1;
    KERNEL_PAGE_TABLE[PAGE_LAST].read_write = 1;
    KERNEL_PAGE_TABLE[PAGE_LAST].user_supervisor = 0;
    KERNEL_PAGE_TABLE[PAGE_LAST].physical_page_address = 0;

    KERNEL_PAGE_DIR[PAGE_LAST].present = 1;
    KERNEL_PAGE_DIR[PAGE_LAST].read_write = 0;
    KERNEL_PAGE_DIR[PAGE_LAST].user_supervisor = 0;
    KERNEL_PAGE_DIR[PAGE_LAST].page_size = 0;
    KERNEL_PAGE_DIR[PAGE_LAST].page_table_address = KERNEL_UPPER_PAGE_TABLE_ADDRESS >> 12;
    KERNEL_PAGE_DIR[PAGE_LAST].user_data = PD_USED;

    setup_page_table((PageTableEntry*)KERNEL_UPPER_PAGE_TABLE_ADDRESS,0,0);

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

PageDirectoryEntry* paging_new_page_directory(uint8_t us){
    PageDirectoryEntry* directory = (PageDirectoryEntry*)memory_alloc_block();
    debug("Allocated page directory at ");debug_i((uint32_t)directory,16);debug("\n");

    if (directory){
        debug("1\n");
        uint32_t table_address = memory_alloc_block();
        if (!table_address){
            memory_free_block((uint32_t)directory);
            return NULL;
        }
        debug("2\n");
        memset(directory, 0, MEMORY_BLOCK_SIZE);
        debug("3\n");
        /**
         * Allocate last page directory entry with one page table
         * for interrupts
         **/
        directory[PAGE_LAST].present = 1;
        directory[PAGE_LAST].read_write = 0;
        directory[PAGE_LAST].user_supervisor = us;
        directory[PAGE_LAST].page_size = 0;
        directory[PAGE_LAST].page_table_address = table_address >> 12;
        directory[PAGE_LAST].user_data = PD_USED;
        debug("4\n");

        setup_page_table_user((PageTableEntry*)table_address,1,us);
        debug("5\n");


        return directory;
    }
    return NULL;
}
uint32_t paging_alloc_table(PageDirectoryEntry* dir_entry){
    dir_entry->present = 1;
    dir_entry->read_write = 1;
    dir_entry->user_supervisor = 1;
    dir_entry->user_data = PD_USED;
    dir_entry->page_table_address = memory_alloc_block() >> 12;
    return 0;
}

uint32_t physical_address(uint32_t page_dir, uint32_t address){
    VirtualAddress v_address = { .address= address };
    PageTableEntry* page_table = (PageTableEntry*)(((PageDirectoryEntry*)page_dir)[v_address.page_dir_index].page_table_address << 12);
    return page_table[v_address.page_index].physical_page_address << 12;
}

static void handle_page_fault(InterruptFrame frame){
    debug("Page fault!!!\n");
}

static void setup_page_table(PageTableEntry* page_table, uint8_t rw, uint8_t us){
    // Last page table goes for ISR table
    // All page directories will have this page mapped.
    page_table[PAGE_LAST].present = 1;
    page_table[PAGE_LAST].read_write = rw;
    page_table[PAGE_LAST].user_supervisor = us;
    page_table[PAGE_LAST].user_data = PD_USED;
    page_table[PAGE_LAST].physical_page_address = ((uint32_t)isr_handlers_start) >> 12;
    debug("ISR handlers physical address:");debug_i((uint32_t)isr_handlers_start,16);debug("\n");
    debug("Last page of table directory used to map ISR handlers: ");
    debug_i((PAGE_LAST << 22) | (PAGE_LAST << 12),16);debug("\n");
}
static void setup_page_table_user(PageTableEntry* page_table, uint8_t rw, uint8_t us){
    // Last page table goes for ISR table
    // All page directories will have this page mapped.
    VirtualAddress v_address;
    KERNEL_EXCHANGE_PAGE.physical_page_address = ((uint32_t)page_table) >> 12;

    v_address.page_dir_index = 0;
    v_address.page_index = PAGE_LAST;
    v_address.offset = 0;

    setup_page_table((PageTableEntry*)v_address.paddress, rw, us);
}
/**
 * TODO: Move somewhere else
 **/
void paging_load_code(Stream* stream, PageTableEntry* start){
    ElfHeader header;
    ElfProgramHeader prg_header;
    uint32_t blocks;
    uint32_t size;
    uint32_t i;
    VirtualAddress v_address;

    elf_read_header(stream, &header);
    elf_read_program_header(stream, &header, 0, &prg_header);

    size = prg_header.segment_mem_size;
    blocks = TO_PAGES(size);

    v_address.page_dir_index = 0;
    v_address.page_index = PAGE_LAST;
    v_address.offset = 0;

    debug("Reading ");debug_i(blocks,10);debug(" blocks\n");

    for (i=0;i<blocks;i++){
        uint32_t phys_block = memory_alloc_block();
        KERNEL_EXCHANGE_PAGE.physical_page_address = phys_block;
        elf_read_program_page(stream, &prg_header, v_address.paddress, i, PAGE_SIZE);
        // Place the physical address on the target page table
        ((PageTableEntry*)v_address.paddress)[i].physical_page_address = phys_block;
        ((PageTableEntry*)v_address.paddress)[i].present = 1;
        ((PageTableEntry*)v_address.paddress)[i].read_write = 0;
    }


}
