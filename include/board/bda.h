#ifndef _BDA_H_
#define _BDA_H_

#include "lib/stdint.h"

typedef struct {
    uint16_t com_ports[4];
    uint16_t lpt_ports[3];
    uint16_t ebda_address;
    uint16_t detected_hardware;
    uint16_t kb_before_ebda;
    uint16_t keyboard_state;
    uint8_t keyboard_buffer[32];
    uint8_t display_mode;
    uint16_t n_columns_text_mode;
    uint16_t video_port;
    uint16_t ticks_since_boot;
    uint8_t hard_disk_count;
    uint16_t keyboard_buffer_start;
    uint16_t keyboard_buffer_end;
    uint8_t last_keyboard_state;
} BiosDataArea;

extern const BiosDataArea* BDA;

#endif
