
#include "gd32e23x.h"
#include "bsp.h"

void main() {
    bsp_init();
    for (;;) {
        bsp_systick_delay_await_ms(1000);
        bsp_pa15_led_toggle();
    }
}