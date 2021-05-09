# minios
A very basic bootloader and attempt of minimal i386 operating system, just for self learning.
So far these features are implemented (barely):
* bootloader with available memory detection.
* PCI device listing.
* time based task switching
* memory paging
* basic raw console
* base libraries: strings, heap, etc.

## Requirements
* gcc
* nasm
* qemu
