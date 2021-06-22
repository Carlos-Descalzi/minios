#include "dirent.h"

const char* types[] = {
    "block",
    "char",
    "dir",
    "fifo",
    "link",
    "file",
    "socket",
    "unknown"
};

int main(int argc, char** argv){

    struct dirent entry;

    DIR* dir = opendir("/");

    while ((entry = readdir(dir))){
        print("%s\t%s\n",entry->d_name,types[entry->d_type]);
    }

    return 0;
}
