#include "fs/fs.h"
#include "misc/debug.h"
#include "lib/heap.h"
#include "lib/string.h"
#include "kernel/task.h"
/**
 * Ethernet filesystem module.
 * Allow send and receive packets by referencing devices in the following way:
 *
 * net0:/00:00:00:00:00:00/arp
 *       \---------------/ \-/
 *               |          |
 *               v          v
 *             target    protocol
 *            address
 **/
#define ETHER_TYPE_ARP  0x0806
#define ETHER_TYPE_IP   0x0800

#define ETHFS_INODE_SELF    1

#define htons(v)    ((v >> 8) | ((v & 0xFF)<<8))

typedef struct {
    uint8_t target[6];
    uint8_t source[6];
    uint8_t payload[];
} EthFrame;

typedef struct {
    Inode           inode;
    uint8_t         in_use;
    char            mac_address[6];
    //uint16_t        protocol;
} EthInode;

#define MAX_ETH_INODES  20

typedef struct {
    FileSystem fs;
    char mac_address[6];
    char mac_addresses[MAX_ETH_INODES][6];
    EthInode work_inodes[MAX_ETH_INODES];
    EthInode self;
} EthFileSystem;

typedef struct {
    Stream          stream;
    EthFileSystem*  fs;
    EthInode        inode;
    IORequest       stream_io_request;
    IORequest*      user_io_request;
    uint8_t         data_available;
    uint8_t         buffer[1536];
} EthStream;


#define OPT_GET_HWADDR  0x01

#define ETHFS(fs)       ((EthFileSystem*)fs)
#define INODE(i)        ((Inode*)i)
#define ETHINODE(i)     ((EthInode*)i)
#define ETHSTREAM(s)    ((EthStream*)s)


static const char       FS_TYPE_NAME[] = "eth";
static FileSystemType   FS_TYPE;

static FileSystem*      create_fs           (FileSystemType* fs_type, BlockDevice* device);
static void             list_inodes         (FileSystem* fs, InodeVisitor visitor, void*data);
static void             close               (FileSystem* fs);
static int32_t          load_inode          (FileSystem* fs, uint32_t inodenum, Inode* inode);
static uint32_t         find_inode          (FileSystem* fs, const char* path);
static int32_t          load                (FileSystem* fs, Inode* inode, void* dest);
static uint32_t         read_block          (FileSystem* fs, Inode* inode, 
                                            uint32_t b_index, void* dest, 
                                            uint32_t length);
static int32_t          get_direntry        (FileSystem* fs, Inode* inode, 
                                            uint32_t* offset, DirEntry* direntry);
static Stream*          open_stream         (FileSystem* fs, uint32_t inodenum, uint32_t flags);
int16_t                 eth_read_byte       (Stream*);
int16_t                 eth_write_byte      (Stream*,uint8_t);
int16_t                 eth_read_bytes      (Stream*,uint8_t*,int16_t);
int16_t                 eth_write_bytes     (Stream*,uint8_t*,int16_t);
uint32_t                eth_pos             (Stream*);
int16_t                 eth_seek            (Stream*,uint32_t);
uint32_t                eth_size            (Stream*);
void                    eth_close           (Stream*);
static void             release_resources   (FileSystem* fs);
static Inode*           alloc_inode         (FileSystem* fs);
static void             free_inode          (FileSystem* fs, Inode* inode);
static int16_t          eth_read_async      (Stream* stream, IORequest* request);
static void             reset_request       (EthStream* stream);
static void             request_callback    (IORequest* request, void* data);

void module_init(){
    FS_TYPE.type_name = FS_TYPE_NAME;
    FS_TYPE.create = create_fs;
    fs_register_type(&FS_TYPE);
}


static FileSystem* create_fs(FileSystemType* fs_type, BlockDevice* device){

    if (!device){
        debug("No device");
        return NULL;
    }
    if (DEVICE(device)->kind != NET){
        debug("Invalid device type\n");
        return NULL;
    }

    EthFileSystem* fs = heap_new(EthFileSystem);
    fs->self.inode.uid = ETHFS_INODE_SELF;

    FILE_SYSTEM(fs)->type = fs_type;
    FILE_SYSTEM(fs)->device = device;
    FILE_SYSTEM(fs)->close = close;
    FILE_SYSTEM(fs)->find_inode = find_inode;
    FILE_SYSTEM(fs)->open_stream = open_stream;
    FILE_SYSTEM(fs)->alloc_inode = alloc_inode;
    FILE_SYSTEM(fs)->free_inode = free_inode;
    FILE_SYSTEM(fs)->release_resources = release_resources;
    FILE_SYSTEM(fs)->get_direntry = get_direntry;
    FILE_SYSTEM(fs)->list_inodes = list_inodes;
    FILE_SYSTEM(fs)->read_block = read_block;
    FILE_SYSTEM(fs)->load = load;
    FILE_SYSTEM(fs)->inode_size = sizeof(Inode);
    FILE_SYSTEM(fs)->block_size = 1024;

    device_setopt(device, OPT_GET_HWADDR, fs->mac_address);

    return FILE_SYSTEM(fs);
}

static int32_t get_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){
    if (*offset == 0){
        (*offset)++;
        return 0;
    } else if (*offset == 1){
        (*offset)++;
        return 0;
    }
    return 1;
}

static uint8_t to_hex(char* token, int len){
    uint8_t val;
    for (int i=len-1,n=0;i>=0;i--,n++){
        uint8_t digit;
        if (token[i] >= '0' && token[i] <= '9'){
            digit = token[i] - '0';
        } else if (token[i] >= 'a' && token[i] <= 'f'){
            digit = 10 + token[i] - 'a';
        } else {
            digit = 10 + token[i] - 'A';
        }
        val |= digit << (4 * n);
    }
    return val;
}

static const char MAC_SELF[] = "self";

static int parse_path(const char* path, char* mac){
    
    char* start = strchr(path, '/');
    if (!start){
        return -1;
    }
    start += 1;

    if (!strncmp(start, MAC_SELF ,4)){
        strcpy(mac, MAC_SELF);
        return 0;
    }

    int i=0;
    while(start && i < 6){
        char* to = strchr(start,':');
        if (!to){
            to = start+strlen(start);
        }
        if (!to){
            break;
        }
        mac[i++] = to_hex(start, ((uint32_t)to)-((uint32_t)start));
        start = to+1;
    }
    return 0;
}

static uint32_t mkinodenum(uint16_t index){
    return index + 3;
}

static int breakinodenum(uint16_t inodenum){
    return inodenum -3;
}

static uint32_t find_inode (FileSystem* fs, const char* path){
    char mac[6];

    debug("Finding inode for path ");debug(path);debug("\n");
    if (!strcmp(path,"/")){
        return FS_INODE_ROOT_DIR;
    }

    if (parse_path(path,mac)){
        return 0;
    }

    if (!strcmp(mac, MAC_SELF)){
        return ETHFS_INODE_SELF;
    }

    for (int i=0;i<MAX_ETH_INODES;i++){
        if (!strlen(ETHFS(fs)->mac_addresses[i])){
            memcpy(ETHFS(fs)->mac_addresses[i], mac, 6);
            return mkinodenum(i);
        }
    }
    return 0;
}

static int32_t load_inode (FileSystem* fs, uint32_t inodenum, Inode* inode){

    if (inodenum == ETHFS_INODE_SELF){
        memcpy(inode, &(ETHFS(fs)->self),sizeof(EthInode));
        return 0;
    }

    inode->uid = inodenum;

    uint16_t mac_index = breakinodenum( inodenum );

    memcpy(ETHINODE(inode)->mac_address, ETHFS(fs)->mac_addresses[mac_index],6);

    return 0;
}

static Stream* open_stream(FileSystem* fs, uint32_t inodenum, uint32_t flags){

    EthStream* stream = heap_new(EthStream);
    load_inode(fs,inodenum, INODE(&(stream->inode)));

    stream->fs = ETHFS(fs);
    stream->user_io_request = NULL;
    stream->data_available = 0;
    stream->stream_io_request.type = TASK_IO_REQUEST_READ;
    stream->stream_io_request.kernel = 1;
    stream->stream_io_request.status = TASK_IO_REQUEST_PENDING;
    stream->stream_io_request.size = 1536;
    stream->stream_io_request.target_buffer = stream->buffer;
    stream->stream_io_request.callback = request_callback;
    stream->stream_io_request.callback_data = stream;
    STREAM(stream)->async = DEVICE(fs->device)->async;
    STREAM(stream)->read_byte = eth_read_byte;
    STREAM(stream)->read_bytes = eth_read_bytes;
    STREAM(stream)->write_byte = eth_write_byte;
    STREAM(stream)->write_bytes = eth_write_bytes;
    STREAM(stream)->read_async = eth_read_async;
    STREAM(stream)->pos = eth_pos;
    STREAM(stream)->seek = eth_seek;
    STREAM(stream)->size = eth_size;
    STREAM(stream)->close = eth_close;

    reset_request(stream);
    
    return STREAM(stream);
}

int16_t eth_read_byte(Stream* stream){
    return 0;
}

int16_t eth_read_bytes(Stream* stream,uint8_t* bytes,int16_t size){
    return 0;
}
static int16_t eth_read_async(Stream* stream, IORequest* request){
    debug("Read ethfs async\n");
    
    if (ETHSTREAM(stream)->inode.inode.uid == ETHFS_INODE_SELF){

        handle_io_request(request, 
            (uint8_t*) ETHSTREAM(stream)->fs->mac_address,
            6,
            TASK_IO_REQUEST_DONE);

        return 0;
    }

    if (ETHSTREAM(stream)->data_available){
        debug("already data available\n");
        ETHSTREAM(stream)->data_available = 0;

        handle_io_request(request,
            ETHSTREAM(stream)->buffer,
            1536,
            TASK_IO_REQUEST_DONE);

    } else {
        debug("\tSetting request\n");
        ETHSTREAM(stream)->user_io_request = request;
    }

    return 0;
}


static int broadcast_mac(const uint8_t* mac){
    return (mac[0] == 0x00 || mac[0] == 0xFF)
        && mac[0] == mac[1]
        && mac[1] == mac[2]
        && mac[2] == mac[3]
        && mac[3] == mac[4]
        && mac[4] == mac[5];
}

static void print_mac(const uint8_t* mac){
    for (int i=0;i<6;i++){
        if (i>0){
            debug(":");
        }
        debug_i(mac[i],16);
    }
    debug("\n");
}

static int valid_dst_mac(EthStream* stream, EthFrame* frame){
   int val =  
        !memcmp(stream->fs->mac_address, frame->target, 6)
        || !memcmp(stream->fs->mac_address, frame->source, 6)
        || broadcast_mac(frame->source)
        || broadcast_mac(frame->target);

   return val;
}

static void request_callback(IORequest* request, void* data){
    debug("Packet received\n");
    if (ETHSTREAM(data)->user_io_request){
        debug("Attending user reqquest\n");

        IORequest* user_io_request = ETHSTREAM(data)->user_io_request;

        EthFrame* frame = (EthFrame*) request->target_buffer;

        if (valid_dst_mac(ETHSTREAM(data), frame)){
            ETHSTREAM(data)->user_io_request = NULL;

            handle_io_request(
                user_io_request,
                frame->payload,
                request->dsize - 12,
                TASK_IO_REQUEST_DONE
            );

        } else {
            debug("Not a valid packet\n");
        }
        ETHSTREAM(data)->data_available = 0;
    } else {
        ETHSTREAM(data)->data_available = 1;
    }
    reset_request(ETHSTREAM(data));
}

static void reset_request(EthStream* stream){
    stream->stream_io_request.status = TASK_IO_REQUEST_PENDING;
    stream->stream_io_request.dsize = 0;
    stream->stream_io_request.result = 0;

    block_device_read_async(
        FILE_SYSTEM(stream->fs)->device, 
        &(stream->stream_io_request));
}

int16_t eth_write_byte(Stream* stream,uint8_t byte){
    return 0;
}

int16_t eth_write_bytes(Stream* stream,uint8_t* bytes,int16_t size){

    if (ETHSTREAM(stream)->inode.inode.uid == ETHFS_INODE_SELF){
        // read only
        return 0;
    }
    
    int frame_size = sizeof(EthFrame) + size;
    EthFrame* frame = heap_alloc(frame_size);
    debug("write bytes:");debug_i(size,10);debug("\n");
    print_mac(ETHSTREAM(stream)->inode.mac_address);debug("\n");

    memcpy(frame->target, ETHSTREAM(stream)->inode.mac_address, 6);
    memcpy(frame->source, ETHSTREAM(stream)->fs->mac_address, 6);
    memcpy(frame->payload, bytes, size);

    int16_t result = block_device_write(
        FILE_SYSTEM(ETHSTREAM(stream)->fs)->device, 
        (uint8_t*)frame, 
        frame_size);

    heap_free(frame);

    return result;
}

uint32_t eth_pos(Stream* stream){
    return 0;
}

int16_t eth_seek(Stream* stream,uint32_t pos){
    return 0;
}

uint32_t eth_size(Stream* stream){
    return 0;
}

void eth_close(Stream* stream){
    heap_free(stream);
}

static void release_resources(FileSystem* fs){
    for (int i=0;i<MAX_ETH_INODES;i++){
        memset(ETHFS(fs)->mac_addresses[i],0,6);
    }
    memset(ETHFS(fs)->work_inodes,0,sizeof(EthInode)*MAX_ETH_INODES);
}

static Inode* alloc_inode(FileSystem* fs){
    for (int i=0;i<MAX_ETH_INODES;i++){
        if (!ETHFS(fs)->work_inodes[i].in_use){
            ETHFS(fs)->work_inodes[i].in_use = 1;
            return INODE(&(ETHFS(fs)->work_inodes[i]));
        }
    }
    return NULL;
}

static void free_inode(FileSystem* fs, Inode* inode){
    for (int i=0;i<MAX_ETH_INODES;i++){
        if (INODE(&(ETHFS(fs)->work_inodes[i])) == inode){
            ETHFS(fs)->work_inodes[i].in_use = 0;
            break;
        }
    }
}

static void list_inodes(FileSystem* fs, InodeVisitor visitor, void*data){
}

static void close(FileSystem* fs){
    heap_free(fs);
}
static int32_t load(FileSystem* fs, Inode* inode, void* dest){
    return 0;
}
static uint32_t read_block(FileSystem* fs, Inode* inode, uint32_t b_index, void* dest, uint32_t length){
    return 0;
}
