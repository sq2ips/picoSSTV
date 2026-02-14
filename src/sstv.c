#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "pico/multicore.h"

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

static uint sstv_pwm_pin_slice = 0;
static uint sstv_pwm_channel = 0;

volatile static uint32_t phase = 0;
volatile static uint32_t phase_inc = SSTV_FT(1000);
volatile static bool phase_changed = false;

volatile static uint16_t raster_x = 0;
volatile static uint16_t raster_y = 0;
volatile static uint8_t sstv_seq = 0;
static double sstv_time = 0;
static double sstv_next = 0;
static uint8_t vis_sr = SSTV_VIS_CODE;
static uint8_t vis_parity;
static uint8_t header_ptr = 0;
bool sstv_running = false;

static const uint8_t *buff = NULL;

#define linear_map(x, in_min, in_max, out_min, out_max) ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

static void phase_inc_handler(){
    hw_clear_bits(&timer_hw->intr, 1u << SSTV_ALARM_NUM);

    phase += phase_inc;
    phase_changed = true;

    timer_hw->alarm[SSTV_ALARM_NUM] = (uint32_t) (timer_hw->timerawl + SSTV_ALARM_TIME_MS);
}

static void sampling_thread(){
    while(true){
        if(phase_changed){
            phase_changed = false;            
            pwm_set_chan_level(sstv_pwm_pin_slice, PWM_CHAN_B, sine_table[(uint16_t)((phase>>23) & 0x1FF)]); // use last 8 bits of phase for pwm value

            sstv_time += SSTV_TIME_PER_SAMPLE;

            if(!sstv_running) stop_sstv();
            if(sstv_time < sstv_next) continue;

            switch (sstv_seq){
                case 0:
                    sstv_time = 0; header_ptr = 0; vis_parity = 0;
                    // vis_sr = 44;
                    phase_inc = SSTV_HEADER[header_ptr++];
                    sstv_next = (float)SSTV_HEADER[header_ptr++];
                    sstv_seq++;
                    break;
                case 1:
                    if(SSTV_HEADER[header_ptr + 1] == 0){
                        sstv_seq++; header_ptr = 0;
                    } else {
                        phase_inc = SSTV_HEADER[header_ptr++];
                        sstv_next += (float)SSTV_HEADER[header_ptr++];
                    }
                    break;
                case 2:
                    if(header_ptr == 7) {
                        header_ptr = 0;
                        phase_inc = vis_parity ? SSTV_FT(1100) : SSTV_FT(1300);
                        sstv_next += 30.0; sstv_seq++;
                    } else {
                        phase_inc = (vis_sr & 0x01) ? (vis_parity ^= 0x01, SSTV_FT(1100)) : SSTV_FT(1300);
                        vis_sr >>= 1;
                        sstv_next += 30.0; header_ptr++;
                    }
                    break;
                case 3:
                    phase_inc = SSTV_FT(1200);
                    sstv_next += 30.0 + SSTV_H_SYNC_TIME;
                    raster_x = 0; raster_y = 0; sstv_seq = 10;
                    break;
                case 10:
                    if(raster_x == SSTV_WIDTH) {
                        raster_x = 0;
                        phase_inc = SSTV_FT(1500);
                        sstv_next += SSTV_C_SYNC_TIME;
                        sstv_seq++;
                    } else {
                        int G = buff[1 + (raster_x * 3) + (raster_y * SSTV_WIDTH* 3)];
                        int f = linear_map(G, 0, 255, 1500, 2300);
                        phase_inc = SSTV_FT(f);
                        sstv_next += SSTV_PIXEL_TIME; raster_x++;
                    }
                    break;
                case 11:
                    if (raster_x == SSTV_WIDTH) {
                        raster_x = 0;
                        phase_inc = SSTV_FT(1500);
                        sstv_next += SSTV_C_SYNC_TIME;
                        sstv_seq++;
                    } else {
                        int B = buff[(raster_x * 3) + (raster_y * SSTV_WIDTH * 3)];
                        int f = linear_map(B, 0, 255, 1500, 2300);
                        phase_inc = SSTV_FT(f);
                        sstv_next += SSTV_PIXEL_TIME;
                        raster_x++;
                    }
                    break;
                case 12:
                    if (raster_x == SSTV_WIDTH) {
                        raster_x = 0; raster_y++;
                        if(raster_y == SSTV_HEIGHT){
                            sstv_running = false;
                            sstv_seq = 0; phase_inc = 0; phase = 0;
                        } else {
                            phase_inc = SSTV_FT_SYNC;
                            sstv_next += SSTV_V_SYNC_TIME;
                            sstv_seq = 10;
                        }
                    } else {
                        int R = buff[2 + (raster_x * 3) + (raster_y * SSTV_WIDTH * 3)];
                        int f = linear_map(R, 0, 255, 1500, 2300);
                        phase_inc = SSTV_FT(f);
                        sstv_next += SSTV_PIXEL_TIME;
                        raster_x++;
                    }
                    break;
            }
        }
    }
}

void start_sstv(const uint8_t image_buff[])
{
    buff = image_buff;
    
    gpio_set_function(SSTV_PIN, GPIO_FUNC_PWM);
    sstv_pwm_pin_slice = pwm_gpio_to_slice_num(SSTV_PIN);
    sstv_pwm_channel = pwm_gpio_to_channel(SSTV_PIN);

    pwm_set_clkdiv(sstv_pwm_pin_slice, SSTV_PWM_CLKDIV);
    pwm_set_wrap(sstv_pwm_pin_slice, SSTV_PWM_WRAP);

    pwm_set_chan_level(sstv_pwm_pin_slice, PWM_CHAN_B, 0);
    pwm_set_counter(sstv_pwm_pin_slice, 0);

    hw_set_bits(&timer_hw->inte, 1u << SSTV_ALARM_NUM);
    irq_set_exclusive_handler(SSTV_ALARM_IRQ, phase_inc_handler);

    timer_hw->alarm[SSTV_ALARM_NUM] = (uint32_t) (timer_hw->timerawl+SSTV_ALARM_TIME_MS);

    sstv_running = true;
    phase = 0;
    phase_inc = SSTV_FT(1000);

    pwm_set_enabled(sstv_pwm_pin_slice, true);
    irq_set_enabled(SSTV_ALARM_IRQ, true);

    multicore_reset_core1();
    multicore_launch_core1(sampling_thread);
}

void stop_sstv()
{
    pwm_set_enabled(sstv_pwm_pin_slice, false);
    irq_set_enabled(SSTV_ALARM_IRQ, false);
    multicore_reset_core1();
}