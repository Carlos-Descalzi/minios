#include "board/console.h"
#include "board/pci.h"
#include "board/io.h"
#include "board/pit.h"
#include "board/pic.h"
#include "board/minfo.h"
#include "board/bda.h"
#include "board/ps2.h"
#include "board/memory.h"
#include "kernel/task.h"
#include "kernel/paging.h"
#include "kernel/isr.h"
#include "kernel/device.h"
#include "lib/stdlib.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "misc/debug.h"
#include "bin/elf.h"
#include "fs/ext2.h"

void test_task(){
    Ext2FileSystem* fs;
    Device* device;
    Stream* stream;
    uint32_t task_id;

    console_print("Loading program /hello.elf in memory ...\n");

    device = device_find(DISK, 0);
    if (!device){
        console_print("Device not found\n");
        return;
    }
    fs = ext2_open(BLOCK_DEVICE(device));

    stream = ext2_file_stream_open(fs, "/hello.elf",0);
    task_id = tasks_new(stream);
    stream_close(stream);
    debug("New task id:");debug_i(task_id,10);debug("\n");
    
    stream = ext2_file_stream_open(fs, "/task2.elf",0);
    task_id = tasks_new(stream);
    stream_close(stream);
    debug("New task id:");debug_i(task_id,10);debug("\n");
    
    console_print("Running program ...\n\n");
    sti();
    do{
        tasks_loop();
        //asm volatile("pause");
        debug("loop\n");
    } while (1);
    debug("END\n");
}
