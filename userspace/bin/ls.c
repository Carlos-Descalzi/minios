#include "dirent.h"
#include "stdio.h"
#include "string.h"

static void spc(int n){
    for (int i=0;i<n;i++){
        printf(" ");
    }
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

int main(int argc, char** argv){

    struct dirent* entry;

    DIR* dir = opendir("disk0:/drivers");

    if (!dir){
        printf("No directory\n");
        return 1;
    }
    printf("Listing directory:\n\n");
    int i=0;
    while ((entry = readdir(dir))){
        printf("%s",entry->d_name);
        spc(20-strlen(entry->d_name));
        printf("%s",typename(entry->d_type));
        printf("\n");
        i++;
    }
    printf("%d entries found.\n",i);

    return 0;
}
