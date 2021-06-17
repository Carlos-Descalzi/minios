#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "errno.h"
#include "unistd.h"
#include <stdarg.h>

typedef struct {
    uint8_t digits;
    uint8_t decimals;
    uint8_t padding;
    uint8_t ftype;
} Format;

static void parse_format(const char* format, int* pos, char* buffer, Format* tformat);
static void print_num(int d, Format* tformat, int radix, int* written, char* buffer, FILE* fp);


int printf(const char* format, ...){
    va_list parameters;

    va_start(parameters, format);

    return vfprintf(stdout, format, parameters);
}

int vprintf(const char* format, va_list parameters){
    return vfprintf(stdout, format, parameters);
}

int fprintf(FILE* fp, const char* format, ...){
    va_list parameters;

    va_start(parameters, format);

    return vfprintf(fp, format, parameters);
}

int vfprintf(FILE* fp, const char* format, va_list parameters){ 
    static char buffer[30];
    int written = 0;
    int i = 0;

    while(format[i]){
        char fmtchar = format[i++];
        if (fmtchar == '%'){
            Format tformat;
            memset(buffer,0, sizeof(buffer));
            memset(&tformat,0,sizeof(Format));
            parse_format(format, &i, buffer, &tformat);

            switch(tformat.ftype){
                case '%':
                    fputc('%',fp);
                    written++;
                    break;

                case 'd':{
                        int d = va_arg(parameters, int);
                        print_num(d,&tformat, 10, &written, buffer, fp);
                        break;
                    }

                case 'c': {
                        char c = (char) va_arg(parameters, int);
                        fputc(c,fp);
                        written++;
                        break;
                    }

                case 's':{
                        const char* str = (char*) va_arg(parameters, const char*);
                        int n=0;
                        if (!str){
                            str = "(null)";
                        }
                        for (n=0;str[n];n++){
                            fputc(str[n],fp);
                            written++;
                        }
                        break;
                    }

                case 'x':{
                        int d = va_arg(parameters, int);
                        print_num(d,&tformat, 16, &written, buffer, fp);
                        break;
                    }

                default:
                    break;
            }

        } else {
            fputc(fmtchar, fp);
            written++;
        }

    }
    

    return written;
}
int puts(const char* str){
    for (int i=0;str[i];i++){
        fputc(str[i],stdout);
    }
    return 0;
}

struct _FILE {
    int fd;
};

int fputc(int c, FILE* fp){
    write(fp->fd,&c,1);
    return 0;
}

static void parse_format(const char* format, int* pos, char* buffer, Format* tformat){
    int i=0;
    
    if(format[*pos] == '0'){
        tformat->padding=1;
        (*pos)++;
    }
    while(isdigit(format[*pos])){
        buffer[i++] = format[(*pos)++];
    }
    buffer[i] = '\0';
    tformat->digits = atoi(buffer);

    if (format[*pos] == '.'){

        while(isdigit(format[*pos])){
            buffer[i++] = format[(*pos)++];
        }
        buffer[i] = '\0';
        tformat->decimals = atoi(buffer);
    }
    tformat->ftype = format[(*pos)++];
}

static void print_num(int d, Format* tformat, int radix, int* written, char* buffer, FILE* fp){
    int n;
    itoa(d, buffer, 10);
    if (tformat->padding){
        for (n=strlen(buffer);n<tformat->digits;n++){
            fputc('0',fp);
            (*written)++;
        }
    }
    for (n=0;buffer[n];n++){
        fputc(buffer[n],fp);
        (*written)++;
    }
}

