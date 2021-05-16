#include "lib/stdio.h"
#include "lib/stdint.h"
#include "lib/stdlib.h"
#include "lib/string.h"
#include "lib/ctype.h"
#include "misc/debug.h"
#include "lib/errno.h"
#include <stdarg.h>

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

int fprintf(FILE* fp, const char* format, ...){
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

#include "board/console.h"

int fputc(int c, FILE* fp){
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
            fputc('0',fp);
            *written++;
        }
    }
    for (n=0;buffer[n];n++){
        fputc(buffer[n],fp);
        *written++;
    }
}

