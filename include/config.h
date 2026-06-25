#ifndef _CONFIG_H
#define _CONFIG_H

#define RADIO_SCK       10
#define RADIO_MOSI      11
#define RADIO_MISO      12
#define RADIO_CS        13
#define RADIO_RST       14
#define RADIO_DIO2      15

#define RADIO_SPI_FREQ 1000000UL

#define RADIO_SSTV_FREQ 433400000UL
#define RADIO_PPM_CORR -4.5f

#define RADIO_DEV 1500

#define RADIO_PA_MAX 7 // 0-7
#define RADIO_PA_LEVEL 15 // 0-15
#define RADIO_PA_BOOST 1 // bool
#define RADIO_OCP_ENABLE 1 // bool
#define RADIO_OCP 27 // Over current protection 0-31

#define SSTV_WAIT 50
#define SSTV_DELAY 5000

#endif