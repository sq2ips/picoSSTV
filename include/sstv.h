#ifndef INC_SSTV_H_
#define INC_SSTV_H_

#include "pico/stdlib.h"

#define SSTV_PIN 15
#define SSTV_PWM_WRAP 255
#define SSTV_PWM_PSC 8.0f

#define SSTV_SYNC_DURATION 12 // in ms
#define SSTV_LINE_DURRATION 138 // in ms

#define SSTV_TICK 1.5f // in ms

#define SSTV_SYNC_TONE 1200
#define SSTV_TONE_BLACK 1500
#define SSTV_TONE_WHITE 1900

void start_sstv();
void stop_sstv();

#endif