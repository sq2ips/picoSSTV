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

uint8_t image_buff[SSTV_WIDTH*SSTV_HEIGHT*3];

int main() {
    stdio_init_all();
    while(!stdio_usb_connected()){sleep_ms(10);}
    printf("picoSSTV starting...\n");

    radio_init();

    uint8_t images_cnt = get_images_count();
    printf("img cnt: %d\n", images_cnt);

    while (true) {
        for(uint8_t cnt = 0; cnt<images_cnt; cnt++){
            uint8_t *jpeg_buff = NULL;
            uint8_t jpeg_buff_size = get_image_data(cnt, jpeg_buff);
            decode_image(image_buff, jpeg_buff, jpeg_buff_size);
            printf("img size: %lu\n", jpeg_buff_size);
            radio_write(REG_OP_MODE, MODE_TX);
            sleep_ms(SSTV_WAIT);
            start_sstv(image_buff);
            while(sstv_running) sleep_ms(100);
            sleep_ms(SSTV_WAIT);
            radio_write(REG_OP_MODE, MODE_SLEEP);
            sleep_ms(SSTV_DELAY);
        }
    }
}