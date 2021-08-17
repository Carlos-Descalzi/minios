#include "stdio.h"

void xx(char* x){
    strcpy(x, "01234567");

}
int main(int argc, char** argv){
    char x[8];
    xx(x);
    return 0;
}
