#define NODEBUG
#include "kernel/paging.h"
#include "kernel/isr.h"
#include "lib/string.h"
#include "misc/debug.h"
#include "board/console.h"
#include "board/memory.h"
#include "bin/elf.h"
/**
 * Paging
 * reference: https://www.scs.stanford.edu/05au-cs240c/lab/i386/s05_02.htm
 *
 *
 * Yeah this implementation of paging and memory management I made sucks, I know.
 *
 **/

// Use the available RAM region of ~30kb which is behind boot sector,
// for page directory.
// The region starts at 0x500 but I need 4k aligned addresses.
#define mkvaddr(d,p,o)          (((d) << 22)|((p) << 12)|(o))
#define KERNEL_PAGE_DIR         ((PageDirectoryEntry*)KERNEL_PAGE_DIR_ADDRESS)
#define KERNEL_PAGE_TABLE       ((PageTableEntry*)KERNEL_PAGE_TABLE_ADDRESS)
#define PAGES_MAX               1024
#define PAGE_LAST               ((PAGES_MAX)-1)
/**
 * I use this page as a kind of window to map to other physical addresses
 * belonging to task pages when I need to operate them
 **/
#define KERNEL_EXCHANGE_PAGE    (KERNEL_PAGE_TABLE[PAGE_LAST])
#define KERNEL_EXCHANGE_ADDRESS 0x3FF000

#define TO_PAGES(s)             ((s / PAGE_SIZE) + (s % PAGE_SIZE ? 1 : 0))

#define set_exchange_page(addr) {   \
    KERNEL_EXCHANGE_PAGE.physical_page_address = ((uint32_t)addr) >> 12; \
    KERNEL_EXCHANGE_PAGE.present = 1; \
    paging_invalidate_cache(); \
}
// Convenience macros, map the page I use for exchange to different structures
// which I use dpending the situation
#define local_page_dir          ((PageDirectoryEntry*)KERNEL_EXCHANGE_ADDRESS)
#define local_table             ((PageTableEntry*)KERNEL_EXCHANGE_ADDRESS)
#define local_ptr               ((void*)KERNEL_EXCHANGE_ADDRESS)

static void     handle_page_fault           (InterruptFrame *frame, void* data);
static void     setup_isr_page              (PageTableEntry* page_table,uint8_t us);
static uint32_t paging_kernel_alloc_pages   (uint32_t nblocks, uint8_t rw);


void paging_init(){
    int i;
    memset(KERNEL_PAGE_DIR, 0, sizeof(PageDirectoryEntry)*1024);
    memset(KERNEL_PAGE_TABLE,0, sizeof(PageTableEntry)*1024);

    KERNEL_PAGE_DIR[0].present = 1;
    KERNEL_PAGE_DIR[0].read_write = 1;
    KERNEL_PAGE_DIR[0].user_supervisor = 0;
    KERNEL_PAGE_DIR[0].page_size = 0;
    KERNEL_PAGE_DIR[0].page_table_address = ((uint32_t)KERNEL_PAGE_TABLE) >> 12;
    KERNEL_PAGE_DIR[0].user_data = 0;

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
    KERNEL_PAGE_DIR[PAGE_LAST].user_data = 0;

    setup_isr_page((PageTableEntry*)KERNEL_UPPER_PAGE_TABLE_ADDRESS,0);

    asm volatile (
        "\tmov %0, %%eax\n"
        "\tmov %%eax, %%cr3\n"
        "\tmov %%cr0, %%eax\n"
        "\tor  $0x80000001, %%eax\n"
        "\tmov %%eax, %%cr0"
        :: "Nd"(KERNEL_PAGE_DIR_ADDRESS)
    );
    debug("PAGING - CR3:");debug_i((uint32_t)KERNEL_PAGE_DIR,16);debug("\n");

    trap_install(0xE, handle_page_fault, NULL);
    console_print("PAGING - Paging initialized\n");
}

uint32_t physical_address(uint32_t page_dir, uint32_t address){
    VirtualAddress v_address = { .address= address };
    PageTableEntry* page_table = (PageTableEntry*)(((PageDirectoryEntry*)page_dir)[v_address.page_dir_index].page_table_address << 12);
    return page_table[v_address.page_index].physical_page_address << 12;
}

static void setup_isr_page(PageTableEntry* page_table, uint8_t us){
    // Last page table goes for ISR table
    // All page directories will have this page mapped.

    page_table[PAGE_LAST].present = 1;
    page_table[PAGE_LAST].read_write = 0;
    page_table[PAGE_LAST].user_supervisor = us;
    page_table[PAGE_LAST].user_data = 0;
    page_table[PAGE_LAST].physical_page_address = ((uint32_t)isr_handlers_start) >> 12;
    debug("PAGING - ISR handlers physical address:");debug_i((uint32_t)isr_handlers_start,16);debug("\n");
    debug("PAGING - Last page of table directory used to map ISR handlers: ");
    debug_i((PAGE_LAST << 22) | (PAGE_LAST << 12),16);debug("\n");
}

static void setup_page(PageDirectoryEntry* dir, VirtualAddress vaddress, uint32_t physical_address, uint8_t rw){
    uint32_t index = vaddress.page_dir_index;

    debug("PAGING - Setup page for vaddress:");debug_i(vaddress.address,16);debug(",rw:");debug_i(rw,10);debug("\n");

    set_exchange_page(dir);

    if (!local_page_dir[index].present){
        uint32_t page_table_address = memory_alloc_block();
        debug("PAGING - Setup page directory entry\n");
        local_page_dir[index].present = 1;
        local_page_dir[index].read_write = 1;
        local_page_dir[index].user_supervisor = 1;
        local_page_dir[index].page_table_address = page_table_address >> 12;
    }

    set_exchange_page(local_page_dir[index].page_table_address << 12);

    index = vaddress.page_index;

    if (!local_table[index].present){
        local_table[index].present = 1;
        local_table[index].read_write = rw;
        local_table[index].user_supervisor = 1;
        local_table[index].physical_page_address = physical_address >> 12;
    }
}


/**
 * TODO: Move somewhere else
 **/
uint32_t paging_load_code(Stream* stream, PageDirectoryEntry* dir){
    ElfHeader header;
    ElfProgramHeader prg_header;
    uint32_t blocks;
    uint32_t i;
    uint32_t j;

    debug("PAGING - Reading elf header\n");
    elf_read_header(stream, &header);

    for (i=0;i< header.program_header_table_entry_count;i++){

        elf_read_program_header(stream, &header, i, &prg_header);

        if (prg_header.segment_type == ELF_PH_PT_LOAD){

            blocks = TO_PAGES(prg_header.segment_mem_size);

            for (j=0;j<blocks;j++){
                uint32_t phys_block = memory_alloc_block();
                debug("PAGING - Alloc physical block:");debug_i(phys_block,16);debug("\n");
                set_exchange_page(phys_block);
                elf_read_program_page(
                    stream, 
                    &prg_header, 
                    local_ptr,
                    j, 
                    PAGE_SIZE);
                setup_page(
                    dir, 
                    (VirtualAddress)(prg_header.virtual_address + ( j * PAGE_SIZE )), 
                    phys_block,
                    (prg_header.flags & 0x2) != 0);
            }
        } else {
            //debug("PAGING - Segment type:");debug_i(prg_header.segment_type,16);debug("\n");
        }
    }

    return header.program_entry_position;
}

uint32_t paging_kernel_load_code(Stream* stream){
    ElfHeader header;
    ElfProgramHeader prg_header;
    ElfSectionHeader section_header;
    uint32_t blocks;
    uint32_t size;
    uint32_t i;
    uint32_t j;

    debug("PAGING - Loading code in kernel space\n");
    elf_read_header(stream, &header);

    if (header.magic_number != MAGIC_NUMBER_ELF){
        debug("Not ELF\n");
        return 0;
    }

    uint32_t target_address = 0;

    for (i=0;i< header.program_header_table_entry_count;i++){
        elf_read_program_header(stream, &header, i, &prg_header);

        if (prg_header.segment_type == ELF_PH_PT_LOAD){
            debug("PAGING - Segment type pt load\n");

            if (prg_header.segment_mem_size > 0){
                blocks = TO_PAGES(prg_header.segment_mem_size);

                uint32_t address = paging_kernel_alloc_pages(
                    blocks, 
                    (prg_header.flags & ELF_PRG_HEADER_W) != 0
                );

                if (!target_address && (prg_header.flags & ELF_PRG_HEADER_X)) {
                    target_address = address;
                }
    
                for (j=0;j<blocks;j++){
                    debug("PAGING - Writing page at :");debug_i(address + j * PAGE_SIZE,16);debug("\n");
                    elf_read_program_page(
                        stream, 
                        &prg_header, 
                        address + j * PAGE_SIZE,
                        j, 
                        PAGE_SIZE);
                }
            }

        } else if (prg_header.segment_type == ELF_PH_PT_DYNAMIC){
            debug("PAGING - Segment type dynamic\n");
        } else {
            debug("PAGING - Segment type:");debug_i(prg_header.segment_type,16);debug("\n");
        }
    }

    return target_address + header.program_entry_position;
}

PageDirectoryEntry* paging_new_task_space(void){
    uint32_t directory;
    uint32_t table_address_1;
    uint32_t table_address_2;
    uint32_t stack_block;
    uint32_t env_block;
    int i;

    directory = memory_alloc_block();
    if (!directory){
        return NULL;
    }

    debug("PAGING - Allocated page directory at ");debug_i(directory,16);debug("\n");

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

    stack_block = memory_alloc_block();
    if (!stack_block){
        memory_free_block(directory);
        memory_free_block(table_address_1);
        memory_free_block(table_address_2);
        return NULL;
    }
    env_block = memory_alloc_block();
    if (!env_block){
        memory_free_block(directory);
        memory_free_block(table_address_1);
        memory_free_block(table_address_2);
        memory_free_block(stack_block);
    }

    debug("PAGING - Allocated page table 0 at ");debug_i(table_address_1,16);debug("\n");
    debug("PAGING - Allocated page table 1 at ");debug_i(table_address_2,16);debug("\n");

    set_exchange_page(directory);

    memset(local_page_dir,0,sizeof(PageDirectoryEntry)*1024);

    local_page_dir[0].present = 1;
    local_page_dir[0].read_write = 0;
    local_page_dir[0].user_supervisor = 0;
    local_page_dir[0].page_table_address = table_address_1 >> 12;

    local_page_dir[PAGE_LAST].present = 1;
    local_page_dir[PAGE_LAST].read_write = 1;
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
    }

    set_exchange_page(table_address_2);
    // Page 1021 of last table is 2k for arguments + 2k for environment
    local_table[PAGE_LAST-2].present = 1;
    local_table[PAGE_LAST-2].read_write = 1;
    local_table[PAGE_LAST-2].user_supervisor = 1;
    local_table[PAGE_LAST-2].physical_page_address = env_block >> 12;
    // Page 1022 of last table maps to stack.
    local_table[PAGE_LAST-1].present = 1;
    local_table[PAGE_LAST-1].read_write = 1;
    local_table[PAGE_LAST-1].user_supervisor = 1;
    local_table[PAGE_LAST-1].physical_page_address = stack_block >> 12;
    // Last page (1023) of last table maps to ISRs
    setup_isr_page(local_table, 1);

    return (PageDirectoryEntry*)(directory | 3);
}
inline void paging_invalidate_cache(void){
    asm volatile(
        "\tmov %cr3, %eax\n"
        "\tmov %eax, %cr3\n"
    );
}
uint32_t current_page_dir(){
    uint32_t value;
    asm volatile(
        "mov %%cr3, %0":"=a"(value)
    );
    return value;
}

void paging_release_task_space(PageDirectoryEntry* page_directory){
    int i,j;
    for (i=0;i<PAGES_MAX;i++){
        set_exchange_page(page_directory);
        if (local_page_dir[i].present){
            local_page_dir[i].present = 0;
            set_exchange_page(local_page_dir[i].page_table_address << 12);
            for (j=0;j<PAGES_MAX;j++){
                if (!(
                    (i == 0 && j < 256) 
                    || (i == PAGE_LAST && j >= (PAGE_LAST-1)))){
                    // skip the shared pages
                    if (local_table[j].present){
                        memory_free_block(local_table[j].physical_page_address);
                    }
                }
            }
        }
    }
    memory_free_block((uint32_t)page_directory);
}
uint32_t paging_physical_address(PageDirectoryEntry* page_dir, void *address){
    uint32_t addr;

    VirtualAddress vaddress = {.paddress = address };
    set_exchange_page(page_dir);
    addr = local_page_dir[vaddress.page_dir_index].page_table_address;
    set_exchange_page(addr << 12);
    return (local_table[vaddress.page_index].physical_page_address << 12) | vaddress.offset;
    return addr;
}

void* paging_to_kernel_space(uint32_t physical_address){
    set_exchange_page(physical_address);

    return (void*) (KERNEL_EXCHANGE_ADDRESS | (physical_address % PAGE_SIZE));
}

static void handle_page_fault(InterruptFrame *frame, void* data){
    debug("PAGING - Page fault\n");
    debug("PAGING - \tCR2:");          debug_i(frame->cr2,16);debug("\n");
    debug("PAGING - \tCR3:");          debug_i(frame->cr3,16);debug("\n");

    set_exchange_page(frame->cr3);
    VirtualAddress address = {.address = frame->cr2};

    if (!local_page_dir[address.page_dir_index].present){
        debug("PAGING - \tPage table not present "); debug_i(address.page_dir_index,16); debug("\n");

        local_page_dir[address.page_dir_index].page_table_address = memory_alloc_block() >> 12;
        local_page_dir[address.page_dir_index].user_supervisor = 1;
        local_page_dir[address.page_dir_index].read_write = 1;
        local_page_dir[address.page_dir_index].present = 1;
        paging_invalidate_cache();
        debug("PAGING - \tAllocated page directory\n");
    } else {
        set_exchange_page(local_page_dir[address.page_dir_index].page_table_address << 12);

        if (!local_table[address.page_index].present){
            debug("PAGING - \tPage not present ");
            debug_i(address.page_dir_index,16);
            debug(":");
            debug_i(address.page_index,16);
            debug("\n");

            local_table[address.page_index].physical_page_address = memory_alloc_block() >> 12;
            local_table[address.page_index].user_supervisor = 1;
            local_table[address.page_index].read_write = 1;
            local_table[address.page_index].present = 1;
            paging_invalidate_cache();
            debug("PAGING - \tAllocated page\n");
        } else {
            debug("PAGING - \tPage present ");
            debug_i(address.page_dir_index,16);
            debug(":");
            debug_i(address.page_index,16);
            debug(",");
            debug_i(local_table[address.page_index].physical_page_address,16);
            debug(",");
            debug_i(local_table[address.page_index].user_supervisor,10);
            debug(",");
            debug_i(local_table[address.page_index].read_write,10);
            debug("\n");
        }

    }
}

static uint32_t paging_kernel_alloc_pages   (uint32_t nblocks, uint8_t rw){
    // skip first page dir
    VirtualAddress start_address;
    debug("Allocating ");debug_i(nblocks,10);debug(" pages for kernel\n");
    for (int page_dir_index = 1;page_dir_index<PAGES_MAX;page_dir_index++){
        if (KERNEL_PAGE_DIR[page_dir_index].present == 0){
            uint32_t address = memory_alloc_block();
            KERNEL_PAGE_DIR[page_dir_index].present = 1;
            KERNEL_PAGE_DIR[page_dir_index].read_write = 1;
            KERNEL_PAGE_DIR[page_dir_index].user_supervisor = 0;
            KERNEL_PAGE_DIR[page_dir_index].page_table_address = address >> 12;
            KERNEL_PAGE_DIR[page_dir_index].page_size = 0;
            
            set_exchange_page(address);
            memset(local_ptr,0,sizeof(PageDirectoryEntry)*PAGES_MAX);
        } else {
            set_exchange_page(KERNEL_PAGE_DIR[page_dir_index].page_table_address << 12);
        }
        for (int page_index = 0;page_index<PAGES_MAX;page_index++){
            if (!local_table[page_index].present){
                start_address.page_dir_index = page_dir_index;
                start_address.page_index = page_index;
                start_address.offset = 0;

                for (int i=0;i<nblocks;i++){

                    local_table[page_index+i].physical_page_address = memory_alloc_block() >> 12;
                    local_table[page_index+i].read_write = rw;
                    local_table[page_index+i].user_supervisor = 0;
                    local_table[page_index+i].present = 1;
                }

                return start_address.address;
            }
        }
    }
    return 0;
}

void paging_write_env(PageDirectoryEntry* dir,
    TaskParams* args,
    TaskParams* env){
    
    if (args || env){
        set_exchange_page(dir);
    
        uint32_t address = local_page_dir[PAGE_LAST].page_table_address << 12;
        set_exchange_page(address);
    
        address = local_table[PAGE_LAST-2].physical_page_address << 12;
        set_exchange_page(address);

        if (args){
            debug("Setting up arguments\n");
            uint32_t offset = mkvaddr(PAGE_LAST, PAGE_LAST-2, 0);
            task_params_copy_with_offset(args, local_ptr, offset);
        }
        if (env){
            debug("Setting up environment\n");
            uint32_t offset = mkvaddr(PAGE_LAST, PAGE_LAST-2, PAGE_SIZE / 2);
            task_params_copy_with_offset(env, local_ptr + PAGE_SIZE / 2, offset);
        }
    }
}
