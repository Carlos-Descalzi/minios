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
/**
 * I use this page as a kind of window to map to other physical addresses
 * belonging to task pages when I need to operate them
 **/
#define KERNEL_EXCHANGE_PAGE    (KERNEL_PAGE_TABLE[PAGE_LAST])
#define KERNEL_EXCHANGE_ADDRESS 0x3FF000

#define TO_PAGES(s)             ((s / PAGE_SIZE) + (s % PAGE_SIZE ? 1 : 0))

#define set_exchange_page(addr) {   \
    KERNEL_EXCHANGE_PAGE.physical_page_address = ((uint32_t)addr) >> 12; \
    invalidate_cache(); \
}

static void     handle_page_fault       (InterruptFrame frame);
static void     setup_isr_page          (PageTableEntry* page_table,uint8_t us);
static void     invalidate_cache        (void);

void paging_init(){
    int i;
    memset(KERNEL_PAGE_DIR, 0, sizeof(PageDirectoryEntry)*1024);
    memset(KERNEL_PAGE_TABLE,0, sizeof(PageTableEntry)*1024);

    KERNEL_PAGE_DIR[0].present = 1;
    KERNEL_PAGE_DIR[0].read_write = 1;
    KERNEL_PAGE_DIR[0].user_supervisor = 0;
    KERNEL_PAGE_DIR[0].page_size = 0;
    KERNEL_PAGE_DIR[0].page_table_address = ((uint32_t)KERNEL_PAGE_TABLE) >> 12;
    KERNEL_PAGE_DIR[0].user_data = 0;//PD_USED;

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
    KERNEL_PAGE_DIR[PAGE_LAST].user_data = 0;//PD_USED;

    setup_isr_page((PageTableEntry*)KERNEL_UPPER_PAGE_TABLE_ADDRESS,0);

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

uint32_t physical_address(uint32_t page_dir, uint32_t address){
    VirtualAddress v_address = { .address= address };
    PageTableEntry* page_table = (PageTableEntry*)(((PageDirectoryEntry*)page_dir)[v_address.page_dir_index].page_table_address << 12);
    return page_table[v_address.page_index].physical_page_address << 12;
}

static void handle_page_fault(InterruptFrame frame){
    debug("Page fault!!!\n");
}

static void setup_isr_page(PageTableEntry* page_table, uint8_t us){
    // Last page table goes for ISR table
    // All page directories will have this page mapped.

    page_table[PAGE_LAST].present = 1;
    page_table[PAGE_LAST].read_write = 0;
    page_table[PAGE_LAST].user_supervisor = us;
    page_table[PAGE_LAST].user_data = 0;//PD_USED;
    page_table[PAGE_LAST].physical_page_address = ((uint32_t)isr_handlers_start) >> 12;
    debug("ISR handlers physical address:");debug_i((uint32_t)isr_handlers_start,16);debug("\n");
    debug("Last page of table directory used to map ISR handlers: ");
    debug_i((PAGE_LAST << 22) | (PAGE_LAST << 12),16);debug("\n");
}
// Convenience macros, map the page I use for exchange to different structures
// which I use dpending the situation
#define local_page_dir ((PageDirectoryEntry*)KERNEL_EXCHANGE_ADDRESS)
#define local_table  ((PageTableEntry*)KERNEL_EXCHANGE_ADDRESS)
#define local_ptr    ((void*)KERNEL_EXCHANGE_ADDRESS)

static void setup_page(PageDirectoryEntry* dir, VirtualAddress vaddress, uint32_t physical_address){
    uint32_t index = vaddress.page_dir_index;

    debug("Setup page for vaddress:");debug_i(vaddress.address,16);debug("\n");

    set_exchange_page(dir);

    if (!local_page_dir[index].present){
        uint32_t page_table_address = memory_alloc_block();
        local_page_dir[index].present = 1;
        local_page_dir[index].read_write = 0;
        local_page_dir[index].user_supervisor = 1;
        local_page_dir[index].page_table_address = page_table_address >> 12;
    }

    set_exchange_page(local_page_dir[index].page_table_address << 12);

    index = vaddress.page_index;

    if (!local_table[index].present){
        local_table[index].present = 1;
        local_table[index].read_write = 0;
        local_table[index].user_supervisor = 1;
        local_table[index].physical_page_address = physical_address >> 12;
    }
}


/**
 * TODO: Move somewhere else
 **/
void paging_load_code(Stream* stream, PageDirectoryEntry* dir, uint32_t vaddress){
    ElfHeader header;
    ElfProgramHeader prg_header;
    uint32_t blocks;
    uint32_t size;
    uint32_t i;

    elf_read_header(stream, &header);
    elf_read_program_header(stream, &header, 0, &prg_header);

    size = prg_header.segment_mem_size;
    blocks = TO_PAGES(size);

    debug("Reading ");debug_i(blocks,10);debug(" blocks\n");

    for (i=0;i<blocks;i++){
        uint32_t phys_block = memory_alloc_block();
        set_exchange_page(phys_block);
        elf_read_program_page(stream, &prg_header, local_ptr, i, PAGE_SIZE);
        setup_page(dir, (VirtualAddress)(vaddress + ( i * PAGE_SIZE )), phys_block);
    }
}

PageDirectoryEntry* paging_new_task_space(void){
    uint32_t directory;
    uint32_t table_address_1;
    uint32_t table_address_2;
    int i;

    directory = memory_alloc_block();
    if (!directory){
        return NULL;
    }

    debug("Allocated page directory at ");debug_i(directory,16);debug("\n");

    // Allocate page table 0 which maps 1:1 with kernel page 0
    table_address_1 = memory_alloc_block();
    if (!table_address_1){
        memory_free_block(directory);
        return NULL;
    }
    // Allocate last page for mapping ISRs
    table_address_2 = memory_alloc_block();
    if (!table_address_2){
        memory_free_block(directory);
        memory_free_block(table_address_1);
        return NULL;
    }
    debug("Allocated page table 0 at ");debug_i(table_address_1,16);debug("\n");
    debug("Allocated page table 1 at ");debug_i(table_address_2,16);debug("\n");

    set_exchange_page(directory);

    memset(local_page_dir,0,sizeof(PageDirectoryEntry)*1024);

    local_page_dir[0].present = 1;
    local_page_dir[0].read_write = 0;
    local_page_dir[0].user_supervisor = 0;
    local_page_dir[0].page_table_address = table_address_1 >> 12;

    local_page_dir[PAGE_LAST].present = 1;
    local_page_dir[PAGE_LAST].read_write = 0;
    local_page_dir[PAGE_LAST].user_supervisor = 1;
    local_page_dir[PAGE_LAST].page_table_address = table_address_2 >> 12;

    set_exchange_page(table_address_1);

    memset(local_table,0,sizeof(PageTableEntry)*1024);
    // First 256 entries of page table 1 maps 1:1 to lower 1mb RAM.
    for (i=0;i<256;i++){
        local_table[i].present = 1;
        local_table[i].read_write = 0;
        local_table[i].user_supervisor = 1;
        local_table[i].physical_page_address = i;
        //debug("Page ");debug_i(i,10);debug(" - Address:");debug_i(i << 12,16);debug("\n");
    }

    set_exchange_page(table_address_2);
    // Last page (1023) of last table maps to ISRs
    setup_isr_page((PageTableEntry*)KERNEL_EXCHANGE_ADDRESS, 1);
    // Page 1022 of last table maps to stack.
    local_table[PAGE_LAST-1].present = 1;
    local_table[PAGE_LAST-1].read_write = 1;
    local_table[PAGE_LAST-1].user_supervisor = 1;
    local_table[PAGE_LAST-1].physical_page_address = 0x3;

    return (PageDirectoryEntry*)(directory | 3);
}
static inline void invalidate_cache(void){
    asm volatile(
        "\tmov %cr3, %eax\n"
        "\tmov %eax, %cr3\n"
    );
}
