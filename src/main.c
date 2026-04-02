#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

#include "pico/multicore.h"

#include "sstv.h"
#include "radio.h"
#include "config.h"
#include "images.h"

extern const uint8_t _binary_image_bin_start[];
extern const uint8_t _binary_image_bin_end[];
size_t _binary_image_bin_len = 0;
#define SSTV_COUNT (_binary_image_bin_len / SSTV_BUFF_LEN)

uint8_t image_buff[SSTV_WIDTH*SSTV_HEIGHT*3];

int main() {
    stdio_init_all();
    //while(!stdio_usb_connected()){sleep_ms(10);}
    printf("picoSSTV starting...\n");

    radio_init();
    
    //radio_write(REG_OP_MODE, MODE_TX);

    //hard_assert(_binary_image_bin_len == (SSTV_BUFF_LEN * SSTV_COUNT));
    _binary_image_bin_len = _binary_image_bin_end-_binary_image_bin_start;
    printf("%lu\n",_binary_image_bin_len);

    decode_image(image_buff, _binary_image_bin_start, _binary_image_bin_len);
    printf("%d\n", image_buff[sizeof(image_buff)-1]);
    /*for(size_t i = 0; i<sizeof(image_buff); i++){
        printf("%c", image_buff[i]);
    }*/
    
    radio_write(REG_OP_MODE, MODE_TX);
    sleep_ms(SSTV_WAIT);
    start_sstv(image_buff);
    while(sstv_running) sleep_ms(100);
    sleep_ms(SSTV_WAIT);
    radio_write(REG_OP_MODE, MODE_SLEEP);
    sleep_ms(SSTV_DELAY);

    while (true) {

        /*for(uint8_t cnt = 0; cnt<SSTV_COUNT; cnt++){
            radio_write(REG_OP_MODE, MODE_TX);
            sleep_ms(SSTV_WAIT);
            start_sstv(_binary_image_bin_start+cnt*SSTV_BUFF_LEN);
            while(sstv_running) sleep_ms(100);
            sleep_ms(SSTV_WAIT);
            radio_write(REG_OP_MODE, MODE_SLEEP);
            sleep_ms(SSTV_DELAY);
        }*/
    }
}