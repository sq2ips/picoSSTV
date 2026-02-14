#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

#include "sstv.h"

int main() {
    stdio_init_all();
    
    start_sstv();
    while (true) {
        tight_loop_contents();
    }
}