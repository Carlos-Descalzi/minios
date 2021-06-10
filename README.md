# minios
A very basic bootloader and attempt of minimal i386 operating system, just for self learning.
So far these features are implemented (barely):
* bootloader with available memory detection.
* PCI device listing.
* IDE basic disk operations.
* Basic raw console
* base libraries: strings, heap, etc.
* Elf binary support
* ext2 filesystem (readonly so far).
* Paging.
* System calls basic skeleton.
* Context switching.
* Process loading and execution.

![screenshot](screenshot.png)

## Requirements
* gcc
* nasm
* qemu

## Build it
Run ```make```

## Test it
Run ```make test```

## Debug it
Run ```make debug```
From another console, you can connect with gdb at port 1234 of localhost.

## Debug messages
Port 0x08 (COM1) of guest is used for debugging purposes, it is sent to host console (on test target), or to file minios.log (on debug target).
