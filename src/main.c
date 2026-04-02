#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

#include "pico/multicore.h"

#include "sstv.h"
#include "radio.h"
#include "config.h"

extern const uint8_t _binary_image_bin_start[];
extern const uint8_t _binary_image_bin_end[];
uint8_t *b = NULL;

int main() {
    stdio_init_all();
    while(!stdio_usb_connected()){sleep_ms(10);}
    printf("picoSSTV starting...\n");

    radio_init();
    
    radio_write(REG_OP_MODE, MODE_TX);

    hard_assert(_binary_image_bin_end-_binary_image_bin_start == SSTV_WIDTH * SSTV_HEIGHT * 3);
    
    radio_write(REG_OP_MODE, MODE_TX);
    sleep_ms(500);
    while (true) {
        start_sstv(_binary_image_bin_start);
        while(sstv_running) sleep_ms(1000);
        //radio_write(REG_OP_MODE, MODE_TX);
        //sleep_ms(500);
        //radio_write(REG_OP_MODE, MODE_SLEEP);
        //sleep_ms(500);
    }
}