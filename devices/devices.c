#include "screen.h"
#include "serial.h"

void devices_register(){
    screen_register();
    serial_register();
}
