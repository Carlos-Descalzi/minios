#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include <stdarg.h>
#include "ctype.h"
#include "debug.h"

struct __file {
};

typedef struct {
    uint8_t digits;
    uint8_t decimals;
    uint8_t padding;
    uint8_t ftype;
} Format;

static void parse_format(const char* format, int* pos, char* buffer, Format* tformat);
static void print_num(int d, Format* tformat, int radix, int* written, char* buffer, FILE* fp);

int kfprintf(FILE* fp, const char* format, ...){
    va_list parameters;
    static char buffer[30];
    int written = 0;
    int i = 0;

    va_start(parameters, format);
    
    if (!format){
        return -1;
    }

    while(format[i]){
        char fmtchar = format[i++];
        if (fmtchar == '%'){
            Format tformat;
            memset(buffer,0, sizeof(buffer));
            memset(&tformat,0,sizeof(Format));
            parse_format(format, &i, buffer, &tformat);

            switch(tformat.ftype){
                case '%':
                    kfputc('%',fp);
                    written++;
                    break;

                case 'd':{
                        int d = va_arg(parameters, int);
                        print_num(d,&tformat, 10, &written, buffer, fp);
                        break;
                    }

                case 'c': {
                        char c = (char) va_arg(parameters, int);
                        kfputc(c,fp);
                        written++;
                        break;
                    }

                case 's':{
                        const char* str = (char*) va_arg(parameters, const char*);
                        int n=0;
                        for (n=0;str[n];n++){
                            kfputc(str[n],fp);
                            written++;
                        }
                        break;
                    }

                case 'x':{
                        int d = va_arg(parameters, int);
                        print_num(d,&tformat, 16, &written, buffer, fp);
                        break;
                    }
            }

        } else {
            kfputc(fmtchar, fp);
            written++;
        }

    }
    

    return written;
}

#include "console.h"

int kfputc(int c, FILE* fp){
    // TODO Use proper FILE implementation
    console_put(c);
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
            kfputc('0',fp);
            *written++;
        }
    }
    for (n=0;buffer[n];n++){
        kfputc(buffer[n],fp);
        *written++;
    }
}

