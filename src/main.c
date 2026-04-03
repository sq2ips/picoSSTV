#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include "sstv.h"
#include "radio.h"
#include "config.h"

extern const uint8_t _binary_image_bin_start[];
extern const uint8_t _binary_image_bin_end[];
#define BINARY_IMAGE_BIN_LEN (_binary_image_bin_end-_binary_image_bin_start)
#define SSTV_COUNT (BINARY_IMAGE_BIN_LEN/SSTV_BUFF_LEN)

void sleep_wd(int32_t time){
    while(time > 0){
        watchdog_update();
        sleep_ms(25);
        time-=25;
    }
}

int main() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    
    stdio_init_all();
    //while(!stdio_usb_connected()){sleep_ms(10);}
    printf("picoSSTV starting...\n");

    radio_init();

    watchdog_enable(100, 1);
    printf("Watchdog enabled.");

    hard_assert(SSTV_BUFF_LEN*SSTV_COUNT == BINARY_IMAGE_BIN_LEN);

    while (true) {
        for(uint8_t cnt = 0; cnt<SSTV_COUNT; cnt++){
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            printf("Starting SSTV transmission of image %d...\n", cnt);
            radio_write(REG_OP_MODE, MODE_TX);
            sleep_wd(SSTV_WAIT);
            start_sstv(_binary_image_bin_start+cnt*SSTV_BUFF_LEN);
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            while(sstv_running) sleep_wd(100);
            sleep_wd(SSTV_WAIT);
            radio_write(REG_OP_MODE, MODE_SLEEP);
            printf("Transmission ended.");
            sleep_wd(SSTV_DELAY);
        }
    }
}