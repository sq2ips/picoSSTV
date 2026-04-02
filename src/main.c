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

#define SSTV_COUNT ((_binary_image_bin_end-_binary_image_bin_start) / SSTV_BUFF_LEN)

int main() {
    stdio_init_all();
    //while(!stdio_usb_connected()){sleep_ms(10);}
    printf("picoSSTV starting...\n");

    radio_init();
    
    radio_write(REG_OP_MODE, MODE_TX);

    hard_assert(_binary_image_bin_end-_binary_image_bin_start == (SSTV_BUFF_LEN * SSTV_COUNT));
    
    radio_write(REG_OP_MODE, MODE_TX);
    sleep_ms(500);
    while (true) {
        for(uint8_t cnt = 0; cnt<SSTV_COUNT; cnt++){
            start_sstv(_binary_image_bin_start+cnt*SSTV_BUFF_LEN);
            while(sstv_running) sleep_ms(1000);
        }
        //radio_write(REG_OP_MODE, MODE_TX);
        //sleep_ms(500);
        //radio_write(REG_OP_MODE, MODE_SLEEP);
        //sleep_ms(500);
    }
}