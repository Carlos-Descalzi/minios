#include "dirent.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"
/**
 * Simple directory listing
 **/

static const char COLOR_NONE[] = "\e[15;0m";
static const char COLOR_EXEC[] = "\e[2;0m";
static const char COLOR_DIR[] = "\e[9;0m";
static const char COLOR_CHR[] = "\e[6;0m";
static const char COLOR_BLK[] = "\e[1;0m";
static const char COLOR_FIFO[] = "\e[4;0m";

static char         typename(uint8_t type);
static const char*  color   (uint8_t type, struct stat* statbuf);

int main(int argc, char* argv[]){
    char tmppath[256];
    struct stat statbuf;
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

    int i=0;

    while ((entry = readdir(dir))){
        sprintf(tmppath,"%s/%s",path, entry->d_name);
        stat(tmppath, &statbuf);
        printf(
            "%c%c%c%c%c%c%c%c%c%c %8d %s%-20s %s\n",
            typename(entry->d_type),
            statbuf.st_mode & S_IRUSR ? 'r' : '-',
            statbuf.st_mode & S_IWUSR ? 'w' : '-',
            statbuf.st_mode & S_IXUSR ? 'x' : '-',
            statbuf.st_mode & S_IRGRP ? 'r' : '-',
            statbuf.st_mode & S_IWGRP ? 'w' : '-',
            statbuf.st_mode & S_IXGRP ? 'x' : '-',
            statbuf.st_mode & S_IROTH ? 'r' : '-',
            statbuf.st_mode & S_IWOTH ? 'w' : '-',
            statbuf.st_mode & S_IXOTH ? 'x' : '-',
            statbuf.st_size,
            color(entry->d_type, &statbuf), 
            entry->d_name,
            COLOR_NONE
        );
        i++;
    }

    printf("%d entries found.\n",i);

    return 0;
}

static const char* color(uint8_t type, struct stat* statbuf){
    switch(type){
        case DT_DIR:
            return COLOR_DIR;
        case DT_CHR:
            return COLOR_CHR;
        case DT_BLK:
            return COLOR_BLK;
        case DT_FIFO:
            return COLOR_FIFO;
        case DT_REG:
            if (statbuf->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)){
                return COLOR_EXEC;
            }
            return COLOR_NONE;
    }
    return COLOR_NONE;
}

static char typename(uint8_t type){
    switch(type){
        case DT_FIFO:
            return 'f';
        case DT_CHR:
            return 'c';
        case DT_DIR:
            return 'd';
        case DT_BLK:
            return 'b';
        case DT_REG:
            return '-';
        case DT_LNK:
            return 'l';
        case DT_SOCK:
            return 's';
        default:
            return 'x';
    }
}

