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
#define BINARY_IMAGE_BIN_LEN (_binary_image_bin_end-_binary_image_bin_start)
#define SSTV_COUNT (BINARY_IMAGE_BIN_LEN/SSTV_BUFF_LEN)

int main() {
    stdio_init_all();
    //while(!stdio_usb_connected()){sleep_ms(10);}
    printf("picoSSTV starting...\n");

    radio_init();

    hard_assert(SSTV_BUFF_LEN*SSTV_COUNT == BINARY_IMAGE_BIN_LEN);

    while (true) {
        for(uint8_t cnt = 0; cnt<SSTV_COUNT; cnt++){
            radio_write(REG_OP_MODE, MODE_TX);
            sleep_ms(SSTV_WAIT);
            start_sstv(_binary_image_bin_start+cnt*SSTV_BUFF_LEN);
            while(sstv_running) sleep_ms(100);
            sleep_ms(SSTV_WAIT);
            radio_write(REG_OP_MODE, MODE_SLEEP);
            sleep_ms(SSTV_DELAY);
        }
    }
}