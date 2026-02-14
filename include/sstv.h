#ifndef INC_SSTV_H_
#define INC_SSTV_H_

#include "pico/stdlib.h"

// PWM 200kHz??
// phase increase timer interrupt at 10MHz / F_SAMPLE???
// core 1 sample handler loop

#define SSTV_PIN 15

#define SSTV_PWM_CLKDIV 3 // PWM clock divider value f_pwm = f_clock/(CLKDIV*(WRAP+1))
#define SSTV_PWM_WRAP 255 // PWM counter wrap value, must be max value of sine table

#define SSTV_ALARM_NUM 0
#define SSTV_ALARM_IRQ TIMER_IRQ_0
#define SSTV_SAMPLE_RATE ((1200 * 32) * 0.93)
#define SSTV_ALARM_TIME_MS (1000000/SSTV_SAMPLE_RATE)
#define SSTV_PHASE_INC_1HZ (4294967295 / SSTV_SAMPLE_RATE) // phase_inc wrap value divided by phase increase interrupt rate gives a phase increase value for 1Hz

#define SSTV_FT(f) (uint32_t)(SSTV_PHASE_INC_1HZ * f)

const static uint32_t SSTV_HEADER[] = {SSTV_FT(2300), 100,SSTV_FT(1500), 100, SSTV_FT(2300), 100, SSTV_FT(1500), 100, SSTV_FT(1900), 300, SSTV_FT(1200), 10, SSTV_FT(1900), 300, SSTV_FT(1200), 30, 0, 0};

void start_sstv();
void stop_sstv();

#endif