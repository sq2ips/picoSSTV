#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

#include "pico/multicore.h"

#include "sstv.h"

extern const uint8_t _binary_image_bin_start[];
extern const uint32_t _binary_image_bin_size;
uint8_t *b = NULL;

int main() {
    stdio_init_all();

    start_sstv(_binary_image_bin_start, 0);

    while (true) {
        printf("test!\n");
        sleep_ms(100);
    }
}