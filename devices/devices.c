#include "screen.h"
#include "serial.h"
#include "rtl8139.h"
#include "ide.h"
#include "console.h"
#include "keyboard.h"

void devices_register(){
    screen_register();
    serial_register();
//    rtl8139_register();
    ide_register();
    keyboard_register();
    console_register();
}
