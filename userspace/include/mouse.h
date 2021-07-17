#ifndef _MOUSE_H_
#define _MOUSE_H_

typedef struct __attribute__((__packed__)) {
    uint8_t bl:1,
            br:1,
            bm:1,
            ao:1,
            xs:1,
            ys:1,
            xo:1,
            yo:1;
    int8_t  xm;
    int8_t  ym;
} MouseEvent;

#endif
