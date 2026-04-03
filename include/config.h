#ifndef _CONFIG_H
#define _CONFIG_H

#define RADIO_SCK       10
#define RADIO_MOSI      11
#define RADIO_MISO      12
#define RADIO_CS        13
#define RADIO_RST       14
#define RADIO_DIO2      15

#define RADIO_SSTV_FREQ 435100000UL
#define RADIO_PPM_CORR 0.0f

#define RADIO_DEV 1500

#define RADIO_PA_MAX 4 // 0-7
#define RADIO_PA_LEVEL 1 // 0-15
#define RADIO_PA_BOOST 0 // bool

#define SSTV_WAIT 50
#define SSTV_DELAY 5000

#endif