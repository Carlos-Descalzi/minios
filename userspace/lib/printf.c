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
static void print_num(char* buffer, Format* tformat, int radix, int* written, FILE* fp);


int printf(const char* format, ...){
    va_list parameters;

    va_start(parameters, format);

    int ret = vfprintf(stdout, format, parameters);

    va_end(parameters);

    return ret;
}

int vprintf(const char* format, va_list parameters){
    return vfprintf(stdout, format, parameters);
}

int fprintf(FILE* fp, const char* format, ...){
    va_list parameters;

    va_start(parameters, format);

    int ret = vfprintf(fp, format, parameters);

    va_end(parameters);

    return ret;
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
                        print_num(
                            itoa(d, buffer, 10),
                            &tformat, 10, &written, fp);
                        break;
                    }

                case 'c': {
                        char c = (char) va_arg(parameters, int);
                        fputc(c,fp);
                        written++;
                        break;
                    }

                case 's':{
                        const char* str = va_arg(parameters, const char*);
                        int n=0;
                        if (!str){
                            str = "(null)";
                        }
                        written+=fputs(str,fp);
                        break;
                    }

                case 'x':{
                        unsigned int d = va_arg(parameters, unsigned int);
                        print_num(
                            utoa(d,buffer,16),
                            &tformat, 16, &written, fp);
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
    int r = fputs(str,stdout);
    fputc('\n',stdout);
    return r;
}

int putchar(char c){
    return fputc(c,stdout);
}

struct _FILE {
    int fd;
};

int fputc(int c, FILE* fp){
    if (fp && fp->fd){
        write(fp->fd,&c,1);
        return 0;
    }
    return -1;
}

int fputs(const char* c, FILE* fp){
    if (fp && fp->fd){
        int l = strlen(c);
        write(fp->fd,c,l);
        return l;
    }
    return -1;
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
        (*pos)++;

        while(isdigit(format[*pos])){
            buffer[i++] = format[(*pos)++];
        }
        buffer[i] = '\0';
        tformat->decimals = atoi(buffer);
    }
    tformat->ftype = format[(*pos)++];
}

static void print_num(char* buffer, Format* tformat, int radix, int* written, FILE* fp){
    int n;
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

