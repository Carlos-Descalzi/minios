#define NODEBUG
#include "ide.h"
#include "kernel/device.h"
#include "board/io.h"
#include "board/pci.h"
#include "lib/heap.h"
#include "lib/stdlib.h"
#include "lib/string.h"
#include "misc/debug.h"

/**
 * I just copied these chunks from here
 * https://wiki.osdev.org/PCI_IDE_Controller#Detecting_a_PCI_IDE_Controller
 * TODO: Figure out what is this about
 **/

#define IDE_TYPE_BUS_MASTER 5

#define pci_base_address(pci, n) \
    (pci->base.header_type.type == 0 ? \
     pci->type00.base_addresses[n] : \
     pci->type01.base_addresses[n])

#define ATA_STATUS_BUSY         0x80
#define ATA_STATUS_READY        0x40
#define ATA_STATUS_WRITE_FAIL   0x20
#define ATA_STATUS_SEEK_READY   0x10
#define ATA_STATUS_REQ_READY    0x08
#define ATA_STATUS_CORR_DATA    0x04
#define ATA_STATUS_INDEX        0x02
#define ATA_STATUS_ERROR        0x01


#define ATA_ERROR_BAD_BLOCK     0x80
#define ATA_ERROR_UNR_DATA      0x40
#define ATA_ERROR_MEDIA_CHG     0x20
#define ATA_ERROR_MID_NOT_FOUND 0x10
#define ATA_ERROR_MEDIA_CHG_REQ 0x08
#define ATA_ERROR_ABOR          0x04
#define ATA_ERROR_T0_NOT_FOUND  0x02
#define ATA_ERROR_NO_ADDR_MARK  0x01

#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_READ_DMA        0xc8
#define ATA_CMD_READ_DMA_EXT    0x25
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_WRITE_DMA       0xca
#define ATA_CMD_WRITE_DMA_EXT   0x35
#define ATA_CMD_CACHE_FLUSH     0xe7
#define ATA_CMD_CACHE_FLUSH_EXT 0xea
#define ATA_CMD_PACKET          0xa0
#define ATA_CMD_IDENTIFY_PACKET 0xa1
#define ATA_CMD_IDENTIFY        0xec

#define ATA_REG_DATA            0x00
#define ATA_REG_SECCOUNT0       0x02
#define ATA_REG_HDDEVSEL        0x06
#define ATA_REG_COMMAND         0x07
#define ATA_REG_STATUS          0x07
#define ATA_REG_SECCOUNT1       0x08
#define ATA_REG_CONTROL         0x0C
#define ATA_REG_LBA0            0x03
#define ATA_REG_LBA1            0x04
#define ATA_REG_LBA2            0x05
#define ATA_REG_LBA3            0x09
#define ATA_REG_LBA4            0x0a
#define ATA_REG_LBA5            0x0b
#define ATA_REG_ALTSTATUS       0x0c

#define IDE_TYPE_ATA            1
#define IDE_TYPE_ATAPI          2

#define ATA_IDENTIFY_DEVICETYPE     0
#define ATA_IDENTIFY_MODEL          54
#define ATA_IDENTIFY_CAPABILITIES   98
#define ATA_IDENTIFY_COMMANDSETS    164
#define ATA_IDENTIFY_MAX_LBA_EXT    200
#define ATA_IDENTIFY_MAX_LBA        120

#define IDE_ACTION_READ         0
#define IDE_ACTION_WRITE        1

#define IDE_DEVICE(d)   ((IDEDevice*)d)

#define SECTOR_SIZE     512

typedef struct {
    uint16_t base;
    uint16_t ctrl;
    uint16_t master;
    uint8_t irq;
    uint8_t irq_disabled;
} IDEChannel;


typedef struct {
    IDEChannel channel;
    uint8_t drive;
    uint16_t type;
    uint32_t size;
    uint16_t signature;
    uint16_t capabilities;
    uint32_t command_sets;
    uint32_t size_in_sectors;
    char model[41];
} IDEDrive;


typedef struct IDEDevice{
    BlockDevice device;
    IDEDrive drive;
    uint8_t sector_buffer[SECTOR_SIZE];
    uint32_t current_pos;
} IDEDevice;

typedef void (*ReadCallback)(IDEDevice*,uint8_t*, void*);

static uint8_t  count_devices               (DeviceType* device_type);
static Device*  instantiate                 (DeviceType* device_type, uint8_t device_number);
static void     release                     (DeviceType* device_type, Device* device);
static void     check_pci                   (uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header, void* user_data);
static uint8_t  ide_read_reg                (IDEChannel* channel, uint8_t reg);
static void     ide_write_reg               (IDEChannel* channel, uint8_t reg, uint8_t data);
static void     get_controller_info         (PCIHeader* header);
static void     irq_set_enabled             (IDEChannel* channel, int enabled);
static void     ide_read_reg_buffer         (IDEChannel* channel, uint8_t reg, uint32_t* buffer, uint8_t wcount);
static int16_t  ide_read                    (BlockDevice* device, uint8_t* buffer, uint16_t size);
static int16_t  ide_write                   (BlockDevice* device, uint8_t* buffer, uint16_t size);
static void     ide_seek                    (BlockDevice* device, uint32_t pos);
static uint32_t ide_pos                     (BlockDevice* device);
static uint8_t  get_ata_access_cmd          (int action, int dma, uint8_t lbamode);
static int8_t   ide_ata_access              (IDEDevice* device, uint32_t lba, uint8_t nsectors, 
                                            int action, ReadCallback callback, void* callback_data);
//static void show_sector(uint8_t *sector);

static uint8_t device_count;
static IDEDrive devices[4];
static DeviceType DEVICE_TYPE = {
    kind: DISK,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};

void ide_register(){
    device_register_type(&DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    device_count = 0;
    memset(&devices,0,sizeof(devices));
    pci_list_all_buses(check_pci, NULL);
    return device_count;
}

static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    IDEDevice* device = heap_alloc(sizeof(IDEDevice));
    memset(device,0,sizeof(IDEDevice));
    memcpy(&(device->drive),&(devices[device_number]),sizeof(IDEDrive));
    device->device.base.type = DEVICE_TYPE_BLOCK;
    device->device.read = ide_read;
    device->device.write = ide_write;
    device->device.seek = ide_seek;
    device->device.pos = ide_pos;
    return DEVICE(device);
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

static void get_controller_info(PCIHeader* header){
    int i, j, k;
    uint8_t prog_if = header->base.prog_if;
    IDEChannel channels[2];
    uint8_t buffer[SECTOR_SIZE];
    int status;

    if (prog_if & 0x80){
        uint16_t master_port = pci_base_address(header, 4) & ~1;
        debug("\tbus master controller, port:");
        debug_i(master_port,16);
        debug("\n");
        channels[0].master = master_port;
        channels[1].master = master_port;
    }
    if (prog_if & 0x01){
        debug("\tprimary channel in PCI native mode\n");
    } else {
        debug("\tprimary channel in compatibility mode\n");
        channels[0].base = 0x1F0;
        channels[0].ctrl = 0x3F6;
        channels[0].irq = 0x14;
    }
    if (prog_if & 0x04){
        debug("\tsecondary channel in PCI native mode\n");
    } else {
        debug("\tsecondary channel in compatibility mode\n");
        channels[1].base = 0x170;
        channels[1].ctrl = 0x376;
        channels[1].irq = 0x15;
    }
    irq_set_enabled(&(channels[0]), 0);
    irq_set_enabled(&(channels[1]), 0);
    channels[0].irq_disabled = 1;
    channels[1].irq_disabled = 1;

    for (i=0;i<2;i++){
        IDEChannel* channel = &(channels[i]);
        for (j=0;j<2;j++){
            int error = 0;
            uint16_t type;
            ide_write_reg(channel, ATA_REG_HDDEVSEL, 0xa0 | (j << 4));
            // sleep
            ide_write_reg(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            // sleep

            if (ide_read_reg(channel, ATA_REG_STATUS) != 0){

                while (1){
                    status = ide_read_reg(channel, ATA_REG_STATUS);
                    if (status & ATA_STATUS_ERROR){
                        error = 1;
                        break;
                    }
                    if (!(status & ATA_STATUS_BUSY) && (status & ATA_STATUS_REQ_READY)){
                        break;
                    }
                }

                if (error){
                    uint8_t cl = ide_read_reg(channel, ATA_REG_LBA1);
                    uint8_t ch = ide_read_reg(channel, ATA_REG_LBA2);

                    if (cl == 0x14 && ch == 0xeb){
                        type = IDE_TYPE_ATAPI;
                        debug("IDE - Found ATAPI device, not supported yet.\n");
                        continue; 
                    } else if (cl == 0x69 && ch == 0x96){
                        type = IDE_TYPE_ATA;
                    } else {
                        debug("IDE - Unknown device found, skipping\n");
                        continue;
                    }

                    ide_write_reg(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                    // sleep
                }

                ide_read_reg_buffer(channel, ATA_REG_DATA, (uint32_t*) buffer, 128);

                memcpy(&(devices[device_count].channel),channel,sizeof(IDEChannel));
                devices[device_count].drive = j;
                devices[device_count].type = type; // TODO add ATAPI.
                devices[device_count].signature = *((uint16_t*)buffer + ATA_IDENTIFY_DEVICETYPE);
                devices[device_count].capabilities = *((uint16_t*)buffer + ATA_IDENTIFY_CAPABILITIES);
                devices[device_count].command_sets = *((uint32_t*)buffer + ATA_IDENTIFY_COMMANDSETS);
                
                if (devices[device_count].command_sets & (1<<26)){
                    devices[device_count].size = *((uint32_t*)(buffer + ATA_IDENTIFY_MAX_LBA_EXT));
                } else {
                    devices[device_count].size = *((uint32_t*)(buffer + ATA_IDENTIFY_MAX_LBA));
                }
                for (k=0;k<40;k+=2){
                    devices[device_count].model[k] = buffer[ATA_IDENTIFY_MODEL + k + 1];
                    devices[device_count].model[k+1] = buffer[ATA_IDENTIFY_MODEL + k];
                }
                for (k=39;k>=0;k--){
                    if (devices[device_count].model[k] == ' '){
                        devices[device_count].model[k] = '\0';
                    } else {
                        break;
                    }
                }
                devices[device_count].model[40] = '\0';
                debug("New IDE disk found: ");
                debug(devices[device_count].model);
                debug(", # sectors: ");
                debug_i(devices[device_count].size,10);
                debug(", Size: ");
                debug_i(devices[device_count].size * SECTOR_SIZE, 10);
                debug("\n");
                memcpy(&(devices[device_count].channel),channel, sizeof(IDEChannel));

                device_count++;
            }
        }
    }
}

static void inline irq_set_enabled(IDEChannel* channel, int enabled){
    channel->irq_disabled = enabled ? 0 : 1;
    ide_write_reg(channel, ATA_REG_CONTROL, enabled ? 0 : 2);
}

static void check_pci(uint8_t bus,uint8_t device,uint8_t func, PCIHeader* header, void* user_data){
    if (header->base.class == 0x01
        && header->base.subclass == 0x01){
        debug("Detected PCI IDE device\n");
        get_controller_info(header);
    }
}

static uint16_t get_reg(IDEChannel* channel, uint8_t reg){
    if (reg < 0x08){
        return channel->base + reg;
    } else if (reg < 0x0C){
        return channel->base + reg - 0x06;
    } else if (reg < 0x0E) {
        return channel->ctrl + reg - 0x0A;
    } else {
        return channel->master + reg - 0x0E;
    }
}

static uint8_t ide_read_reg(IDEChannel* channel, uint8_t reg){
    uint8_t value;

    if (reg > 0x07 && reg < 0x0c){
        ide_write_reg(channel, ATA_REG_CONTROL, 0x80| channel->irq_disabled);
    }

    value = inb(get_reg(channel,reg));

    if (reg > 0x07 && reg < 0x0c){
        ide_write_reg(channel, ATA_REG_CONTROL, channel->irq_disabled);
    }

    return value;
}

static void ide_write_reg(IDEChannel* channel, uint8_t reg, uint8_t data){
    if (reg > 0x07 && reg < 0x0c){
        ide_write_reg(channel, ATA_REG_CONTROL, 0x80| channel->irq_disabled);
    }

    outb(get_reg(channel,reg), data);

    if (reg > 0x07 && reg < 0x0c){
        ide_write_reg(channel, ATA_REG_CONTROL, channel->irq_disabled);
    }
}
static void ide_read_reg_buffer(IDEChannel* channel, uint8_t reg, uint32_t* buffer, uint8_t wcount){
    int i;
    if (reg > 0x07 && reg < 0x0c){
        ide_write_reg(channel, ATA_REG_CONTROL, 0x80| channel->irq_disabled);
    }

    for (i=0;i<wcount;i++){
        buffer[i] = indw(get_reg(channel, reg));
    }

    if (reg > 0x07 && reg < 0x0c){
        ide_write_reg(channel, ATA_REG_CONTROL, channel->irq_disabled);
    }
}

static int8_t ide_poll(IDEChannel* channel, uint32_t check){
    int i;

    for (i=0;i<4;i++){
        ide_read_reg(channel, ATA_REG_ALTSTATUS);
    }

    while (ide_read_reg(channel, ATA_REG_STATUS) & ATA_STATUS_BUSY);

    if (check){

        uint8_t state = ide_read_reg(channel, ATA_REG_STATUS);

        if (state & ATA_STATUS_ERROR){
            return 2;
        }
        if (state & ATA_STATUS_WRITE_FAIL){
            return 1;
        }
        if (!(state & ATA_STATUS_REQ_READY)){
            return 3;
        }
    }
    return 0;

}


static int8_t ide_ata_access(IDEDevice* device, uint32_t lba, uint8_t nsectors, int action, ReadCallback callback, void *callback_data){
    uint8_t lbamode;
    uint8_t lbaio[6];
    uint8_t head;
    uint8_t sector, cylinder;
    int dma;
    uint8_t cmd;
    int i, j;
    int8_t err;

    irq_set_enabled(&(device->drive.channel), 0);
    memset(lbaio,0,6);

    if (lba >= 0x10000000){
        lbamode = 2;
        lbaio[0] = lba & 0xFF;
        lbaio[1] = (lba & 0xFF00) >> 8;
        lbaio[2] = (lba & 0xFF0000) >> 16;
        lbaio[3] = (lba & 0xFF000000) >> 24;
        head = 0;
    } else if (device->drive.capabilities & 0x200){
        lbamode = 1;
        lbaio[0] = lba & 0xFF;
        lbaio[1] = (lba & 0xFF00) >> 8;
        lbaio[2] = (lba & 0xFF0000) >> 16;
        head = (lba & 0xFF000000) >> 24;
    } else {
        lbamode = 0;
        sector = (lba % 63) +1;
        cylinder = (lba + 1 - sector) / (16 * 63);
        lbaio[0] = sector;
        lbaio[1] = cylinder & 0xFF;
        lbaio[2] = (cylinder >> 8) & 0xFF;
        head = ((lba + 1 - sector) % (16 * 63)) / 63;
        //debug("IDE - read, lba:");debug_i(lba,10);
        //debug(",head:");debug_i(head,10);
        //debug(",cylinder:");debug_i(cylinder,10);
        //debug(",sector:");debug_i(sector,10);debug("\n");
    }
    while (ide_read_reg(&(device->drive.channel), ATA_REG_STATUS) & ATA_STATUS_BUSY);

    // TODO slavebit
    if (lbamode == 0){
        ide_write_reg(&(device->drive.channel), ATA_REG_HDDEVSEL, 0xA0 | (0 << 4) | head);
    } else {
        ide_write_reg(&(device->drive.channel), ATA_REG_HDDEVSEL, 0xE0 | (0 << 4) | head);
    }

    if (lbamode == 2){
        ide_write_reg(&(device->drive.channel),ATA_REG_SECCOUNT1, 0);
        ide_write_reg(&(device->drive.channel),ATA_REG_LBA3, lbaio[3]);
        ide_write_reg(&(device->drive.channel),ATA_REG_LBA4, lbaio[4]);
        ide_write_reg(&(device->drive.channel),ATA_REG_LBA5, lbaio[5]);
    }
    ide_write_reg(&(device->drive.channel),ATA_REG_SECCOUNT0, nsectors);
    ide_write_reg(&(device->drive.channel),ATA_REG_LBA2, lbaio[2]);
    ide_write_reg(&(device->drive.channel),ATA_REG_LBA1, lbaio[1]);
    ide_write_reg(&(device->drive.channel),ATA_REG_LBA0, lbaio[0]);

    dma = 0; // TODO DMA

    cmd = get_ata_access_cmd(action, dma, lbamode);
    
    ide_write_reg(&(device->drive.channel), ATA_REG_COMMAND, cmd);

    if (dma){
    } else {
        if (action == IDE_ACTION_READ){
            if ((err = ide_poll(&(device->drive.channel),1))){
                return err;
            }
            for (i=0;i<nsectors;i++){
                for (j=0;j<256;j++){
                    ((uint16_t*)device->sector_buffer)[j] = inw(device->drive.channel.base);
                }
                //show_sector(device->sector_buffer);
                if (callback){
                    callback(device, device->sector_buffer, callback_data);
                }
            }
        } else {
            for (i=0;i<nsectors;i++){
                // TODO: Write
            }
        }
    }
    return 0;
}
/*
static void show_sector(uint8_t *sector){
    int i;
    int j;
    debug("Sector read:\n");
    for (i=0;i<8;i++){
        for (j=0;j<64;j++){
            debug_i(sector[i*64+j],16);debug(" ");
        }
        debug("\n");
    }
}
*/
static uint8_t get_ata_access_cmd(int action, int dma, uint8_t lbamode){
    if (action == IDE_ACTION_READ){
        if (dma){
        } else {
            if (lbamode == 2){
                return ATA_CMD_READ_PIO_EXT;
            } else {
                return ATA_CMD_READ_PIO;
            }
        }
    } else {
        if (dma){
        } else {
            if (lbamode == 2){
                return ATA_CMD_WRITE_PIO_EXT;
            } else {
                return ATA_CMD_WRITE_PIO;
            }
        }
    }
    return 0;
}

typedef struct {
    uint8_t* buffer;
    uint32_t pos;
    uint32_t remaining;
} ReadRequest;


static void read_callback(IDEDevice* device, uint8_t* sector, ReadRequest* request){
    uint32_t to_read = min(SECTOR_SIZE,request->remaining);
    memcpy(request->buffer+request->pos,sector, to_read);
    request->pos+=to_read;
    request->remaining-=to_read;
}

static int16_t ide_read(BlockDevice* device, uint8_t* buffer, uint16_t size){
    ReadRequest read_request = {
        .buffer=buffer, 
        .pos=0, 
        .remaining=size
    };
    uint32_t sector = IDE_DEVICE(device)->current_pos >> 9;
    uint8_t nsectors = size / 512 + (size % 512 ? 1 : 0);
    
    ide_ata_access(
        IDE_DEVICE(device),
        sector,
        nsectors,
        IDE_ACTION_READ, 
        (ReadCallback)read_callback, 
        &read_request
    );
    IDE_DEVICE(device)->current_pos+=SECTOR_SIZE * nsectors;
    return 0;
}

static int16_t ide_write(BlockDevice* device, uint8_t* buffer, uint16_t size){
    return 0;
}

static void ide_seek(BlockDevice* device, uint32_t pos){
    debug("IDE - Ide device seek "); debug_i(pos,10); debug("\n");
    IDE_DEVICE(device)->current_pos = pos;
}
static uint32_t ide_pos (BlockDevice* device){
    return IDE_DEVICE(device)->current_pos;
}
