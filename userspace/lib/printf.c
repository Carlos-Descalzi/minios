#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "errno.h"
#include "unistd.h"
#include <stdarg.h>

typedef struct {
    int16_t digits;
    uint8_t decimals;
    uint8_t padding;
    uint8_t ftype;
} Format;

typedef struct Writer {
    int (*putc) (struct Writer*, char);
    int (*puts) (struct Writer*, const char*);
    char* str;
    FILE* fp;
    int pos;
} Writer;

#define min(a,b)                ((a) < (b) ? (a) : (b))

#define writer_putc(w,c)        (w)->putc(w,c)
#define writer_puts(w,s)        (w)->puts(w,s)

static void parse_format        (const char* format, int* pos, char* buffer, Format* tformat);
static void print_num           (char* buffer, Format* tformat, int radix, int* written, Writer* w);
static void print_str           (const char*str, Format* tformat, int* written, Writer* writer);
static int  do_vfprintf         (Writer* writer, const char* format, va_list parameters);
static void setup_str_writer    (Writer* writer, char* str);
static void setup_fp_writer     (Writer* writer, FILE* fp);
static int  str_putc            (Writer* writer, char c);
static int  str_puts            (Writer* writer, const char* str);
static int  fp_putc             (Writer* writer, char c);
static int  fp_puts             (Writer* writer, const char* str);


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
int sprintf (char* str, const char* format, ...){
    va_list parameters;

    va_start(parameters, format);

    int ret = vsprintf(str, format, parameters);

    va_end(parameters);

    return ret;
}
int vsprintf (char* str, const char* format, va_list ap){
    Writer writer;

    setup_str_writer(&writer, str);

    return do_vfprintf(&writer, format, ap);
}
int vfprintf(FILE* fp, const char* format, va_list parameters){ 
    Writer writer;

    setup_fp_writer(&writer, fp);

    return do_vfprintf(&writer, format, parameters);
}

int puts(const char* str){
    int r = fputs(str,stdout);
    fputc('\n',stdout);
    return r;
}

int putchar(int c){
    return fputc(c,stdout);
}

struct _FILE {
    int fd;
};

int fputc(int c, FILE* fp){
    return write(fp->fd,&c,1);
}

int fputs(const char* c, FILE* fp){
    int l = strlen(c);
    write(fp->fd,c,l);
    return l;
}

static void parse_format(const char* format, int* pos, char* buffer, Format* tformat){
    int i=0;
    int neg = 0;
    
    if (format[*pos] == '-'){
        neg = 1;
        (*pos)++;
    }
    if(format[*pos] == '0'){
        tformat->padding=1;
        (*pos)++;
    }
    while(isdigit(format[*pos])){
        buffer[i++] = format[(*pos)++];
    }
    buffer[i] = '\0';
    tformat->digits = atoi(buffer) * ( neg ? -1 : 1);

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

static void print_num(char* buffer, Format* tformat, int radix, int* written, Writer* writer){
    int l = strlen(buffer);
    int printable = tformat->digits ? min(tformat->digits,l) : l;
    int n;
    if (tformat->digits > 0){
        if (tformat->padding){
            for (n=printable;n<tformat->digits;n++){
                writer_putc(writer, '0');
                (*written)++;
            }
        } else {
            int padding = tformat->digits - l;
            for (int i=0;i<padding;i++){
                writer_putc(writer,' ');
            }

            (*written) += padding;
        }
    }
    if (l > printable){
        buffer+=(l-printable);
    }
    (*written) += writer_puts(writer, buffer);
}
static void print_str(const char*str, Format* tformat, int* written, Writer* writer){

    if (!str){
        str = "(null)";
    }

    int l = strlen(str);

    if (tformat->digits > 0){
        if (tformat->digits > l){
            int padding = tformat->digits - l;
            for (int i=0;i<padding;i++){
                writer_putc(writer,' ');
            }

            (*written) += padding;
        } 
    }

    (*written) += writer_puts(writer,str);

    if (tformat->digits < 0){
        if (-tformat->digits > l){
            int padding = -tformat->digits - l;
            for (int i=0;i<padding;i++){
                writer_putc(writer,' ');
            }

            (*written) += padding;
        } 
    }
}

static int do_vfprintf(Writer* writer, const char* format, va_list parameters){ 
    char buffer[30];
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
                    writer_putc(writer,'%');
                    written++;
                    break;

                case 'd':{
                        int d = va_arg(parameters, int);
                        print_num(
                            itoa(d, buffer, 10),
                            &tformat, 10, &written, writer);
                        break;
                    }
                case 'u':{
                        unsigned int d = va_arg(parameters, unsigned int);
                        print_num(
                            utoa(d, buffer, 10),
                            &tformat, 10, &written, writer);
                        break;
                    }

                case 'c': {
                        char c = (char) va_arg(parameters, int);
                        writer_putc(writer,c);
                        written++;
                        break;
                    }

                case 's':{
                        const char* str = va_arg(parameters, const char*);
                        print_str(str,&tformat, &written, writer);
                        break;
                    }

                case 'x':{
                        unsigned int d = va_arg(parameters, unsigned int);
                        print_num(
                            utoa(d,buffer,16),
                            &tformat, 16, &written, writer);
                        break;
                    }
                case 'o':{
                        unsigned int d = va_arg(parameters, unsigned int);
                        print_num(
                            utoa(d,buffer,8),
                            &tformat, 8, &written, writer);
                        break;
                    }

                default:
                    break;
            }

        } else {
            writer_putc(writer, fmtchar);
            written++;
        }
    }

    return written;
}
static void setup_str_writer (Writer* writer, char* str){
    writer->putc = str_putc;
    writer->puts = str_puts;
    writer->str = str;
    writer->str[0] = '\0';
    writer->pos = 0;
}
static void setup_fp_writer (Writer* writer, FILE* fp){
    writer->putc = fp_putc;
    writer->puts = fp_puts;
    writer->fp = fp;
}
static int str_putc (Writer* writer, char c){
    writer->str[writer->pos++] = c;
    return 1;
}
static int str_puts (Writer* writer, const char* str){
    int l = strlen(str);
    strcpy(writer->str + writer->pos,str);
    writer->pos += l;
    return l;
}
static int fp_putc (Writer* writer, char c){
    return fputc(c, writer->fp);
}
static int fp_puts (Writer* writer, const char* str){
    return fputs(str, writer->fp);
}
