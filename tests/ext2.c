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
#include "fs/ext2.h"
#include "misc/debug.h"
#include "bin/elf.h"

static char buff[32];

void check_e2fs(){
    Ext2FileSystem* fs;
    Device* device;
    char buffer[1024];
    memset(buffer,0,1024);

    device = device_find(DISK, 0);

    console_print("\n\nTesting ext2 filesystem for device kind DISK, instance 0 (disk0)\n");

    if (!device){
        debug("Device not found\n");
    } else {
        fs = ext2_open(BLOCK_DEVICE(device));
        if (!fs){
            debug("Cannot open fs\n");
        } else {
            uint32_t inodenum;
            Ext2Inode inode;
            debug("INIT - Fs Ext2 open\n");

            inodenum = ext2_find_inode(fs, "/");
            console_print("Inode for / :");
            console_print(itoa(inodenum,buff,10));
            console_print("\n");

            inodenum = ext2_find_inode(fs, "/file1.txt");
            console_print("Inode for /file1.txt :");
            console_print(itoa(inodenum,buff,10));
            console_print("\n");

            inodenum = ext2_find_inode(fs, "/folder1");
            console_print("Inode for /folder1 :");
            console_print(itoa(inodenum,buff,10));
            console_print("\n");

            inodenum = ext2_find_inode(fs, "/folderx");
            console_print("Inode for /folderx :");
            console_print(itoa(inodenum,buff,10));
            console_print(" (not found, it's ok)\n");

            inodenum = ext2_find_inode(fs, "/folder1/file2.txt");
            console_print("Inode for /folder1/file2.txt :");
            console_print(itoa(inodenum,buff,10));
            console_print("\n");

            ext2_load_inode(fs, inodenum, &inode);
            console_print("Contents:");
            ext2_load(fs, &inode, buffer);
            console_print(buffer);
            console_print("\n");
        }

    }
}
