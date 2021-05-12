#include "screen.h"
#include "serial.h"
#include "rtl8139.h"

void devices_register(){
    screen_register();
    serial_register();
    rtl8139_register();
}
