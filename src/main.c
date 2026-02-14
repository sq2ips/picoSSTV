#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

#include "pico/multicore.h"

#include "sstv.h"

extern const uint8_t _binary_image_bin_start[];
extern const uint8_t _binary_image_bin_end[];
uint8_t *b = NULL;

int main() {
    stdio_init_all();

    hard_assert(_binary_image_bin_end-_binary_image_bin_start == SSTV_WIDTH * SSTV_HEIGHT * 3);
    start_sstv(_binary_image_bin_start);

    while (true) {
        sleep_ms(100);
    }
}