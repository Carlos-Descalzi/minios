#include "dirent.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
/**
 * Simple directory listing
 **/

static const char*  typename(uint8_t type);
static const char*  color   (uint8_t type);

int main(int argc, char* argv[]){
    char* path = NULL;
    struct dirent* entry = NULL;

    if (argc > 1){
        path = argv[1];
    } else {
        path = getenv("PWD");
    }

    DIR* dir = opendir(path);

    if (!dir){
        printf("Directory not found\n");
        return 1;
    }
    printf("Listing directory %s:\n",path);

    int i=0;

    while ((entry = readdir(dir))){
        printf(
            "%s%-20s%s%s\n",
            color(entry->d_type), 
            entry->d_name,
            typename(entry->d_type),
            color(0)
        );
        i++;
    }

    printf("%d entries found.\n",i);

    return 0;
}

static const char* color(uint8_t type){
    switch(type){
        case DT_DIR:
            return "\e[9;0m";
        case DT_CHR:
            return "\e[6;0m";
        case DT_BLK:
            return "\e[1;0m";
        case DT_FIFO:
            return "\e[4;0m";
    }
    return "\e[15;0m";
}

static const char* typename(uint8_t type){
    switch(type){
        case DT_FIFO:
            return "fifo";
        case DT_CHR:
            return "char";
        case DT_DIR:
            return "dir";
        case DT_BLK:
            return "block";
        case DT_REG:
            return "file";
        case DT_LNK:
            return "link";
        case DT_SOCK:
            return "sock";
        default:
            return "unknown";
    }
}

