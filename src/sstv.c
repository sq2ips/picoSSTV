#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

#include "sstv.h"

static const uint8_t sine_table[] = {
    127, 129, 130, 132, 133, 135, 136, 138, 139, 141, 143, 144, 146, 147, 149, 150, 152, 153, 155, 156, 158, 159, 161, 163, 164, 166, 167, 168, 170,
    171, 173, 174, 176, 177, 179, 180, 182, 183, 184, 186, 187, 188, 190, 191, 193, 194, 195, 197, 198, 199, 200, 202, 203, 204, 205, 207, 208, 209,
    210, 211, 213, 214, 215, 216, 217, 218, 219, 220, 221, 223, 224, 225, 226, 227, 228, 228, 229, 230, 231, 232, 233, 234, 235, 236, 236, 237, 238,
    239, 239, 240, 241, 242, 242, 243, 244, 244, 245, 245, 246, 247, 247, 248, 248, 249, 249, 249, 250, 250, 251, 251, 251, 252, 252, 252, 253, 253,
    253, 253, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 255, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 253, 253, 253, 253, 252, 252,
    252, 251, 251, 251, 250, 250, 249, 249, 249, 248, 248, 247, 247, 246, 245, 245, 244, 244, 243, 242, 242, 241, 240, 239, 239, 238, 237, 236, 236,
    235, 234, 233, 232, 231, 230, 229, 228, 228, 227, 226, 225, 224, 223, 221, 220, 219, 218, 217, 216, 215, 214, 213, 211, 210, 209, 208, 207, 205,
    204, 203, 202, 200, 199, 198, 197, 195, 194, 193, 191, 190, 188, 187, 186, 184, 183, 182, 180, 179, 177, 176, 174, 173, 171, 170, 168, 167, 166,
    164, 163, 161, 159, 158, 156, 155, 153, 152, 150, 149, 147, 146, 144, 143, 141, 139, 138, 136, 135, 133, 132, 130, 129, 127, 125, 124, 122, 121,
    119, 118, 116, 115, 113, 111, 110, 108, 107, 105, 104, 102, 101, 99, 98, 96, 95, 93, 91, 90, 88, 87, 86, 84, 83, 81, 80, 78, 77,
    75, 74, 72, 71, 70, 68, 67, 66, 64, 63, 61, 60, 59, 57, 56, 55, 54, 52, 51, 50, 49, 47, 46, 45, 44, 43, 41, 40, 39,
    38, 37, 36, 35, 34, 33, 31, 30, 29, 28, 27, 26, 26, 25, 24, 23, 22, 21, 20, 19, 18, 18, 17, 16, 15, 15, 14, 13, 12,
    12, 11, 10, 10, 9, 9, 8, 7, 7, 6, 6, 5, 5, 5, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4,
    4, 5, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 43, 44, 45, 46, 47, 49, 50, 51, 52, 54, 55,
    56, 57, 59, 60, 61, 63, 64, 66, 67, 68, 70, 71, 72, 74, 75, 77, 78, 80, 81, 83, 84, 86, 87, 88, 90, 91, 93, 95, 96,
    98, 99, 101, 102, 104, 105, 107, 108, 110, 111, 113, 115, 116, 118, 119, 121, 122, 124, 125};

static const uint16_t SINE_TABLE_SIZE = sizeof(sine_table);
static const uint16_t SSTV_SAMPLE_RATE = SYS_CLK_HZ / ((SSTV_PWM_WRAP + 1) * (SSTV_PWM_PSC + 1));

static const uint16_t PHASE_INC_SYNC = ((SINE_TABLE_SIZE * SSTV_SYNC_TONE) << 7) / SSTV_SAMPLE_RATE;
static const uint16_t PHASE_INC_BLACK = ((SINE_TABLE_SIZE * SSTV_TONE_BLACK) << 7) / SSTV_SAMPLE_RATE;
static const uint16_t PHASE_INC_WHITE = ((SINE_TABLE_SIZE * SSTV_TONE_WHITE) << 7) / SSTV_SAMPLE_RATE;

static const uint32_t SAMPLES_PER_TICK = (SSTV_SAMPLE_RATE << 8) * (SSTV_TICK / 1000.0f);

volatile static uint16_t phase = 0;
volatile static uint16_t phase_inc = 0;
volatile static uint32_t sample = 0;
volatile static uint8_t tick_count = 0;

static uint sstv_pin_slice = 0;
bool in_line = false;

static void sstv_pwm_irq_handler()
{
    // Clear interrupt
    pwm_clear_irq(sstv_pin_slice);
    pwm_set_chan_level(sstv_pin_slice, PWM_CHAN_B, sine_table[(phase >> 7) + ((phase & (1 << 6)) >> 6)]);

    phase += phase_inc;
    if (phase >= (SINE_TABLE_SIZE << 7)) phase -= (SINE_TABLE_SIZE << 7);
    
    if (sample < (1 << 8)){
        if(tick_count==7) phase_inc=PHASE_INC_BLACK;
        else if(tick_count==10) phase_inc=PHASE_INC_WHITE;
        if(tick_count==100){
            phase_inc=PHASE_INC_SYNC;
            tick_count = 0;
        }
        tick_count++;
    }
   //phase_inc = PHASE_INC_WHITE;

    sample += (1 << 8);
    if (sample >= SAMPLES_PER_TICK) sample -= SAMPLES_PER_TICK;

}

void start_sstv()
{
    gpio_set_function(SSTV_PIN, GPIO_FUNC_PWM);
    sstv_pin_slice = pwm_gpio_to_slice_num(SSTV_PIN);

    pwm_set_clkdiv(sstv_pin_slice, SSTV_PWM_PSC);
    pwm_set_wrap(sstv_pin_slice, SSTV_PWM_WRAP);

    // pwm_set_chan_level(sstv_pin_slice, PWM_CHAN_B, 0);
    pwm_set_counter(sstv_pin_slice, 0);

    pwm_clear_irq(sstv_pin_slice);
    pwm_set_irq_enabled(sstv_pin_slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, sstv_pwm_irq_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_set_enabled(sstv_pin_slice, true);

    sstv_pwm_irq_handler();
}

void stop_sstv()
{
    pwm_set_enabled(sstv_pin_slice, false);
    pwm_set_irq_enabled(sstv_pin_slice, false);
    irq_set_enabled(PWM_IRQ_WRAP, false);
}