#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "radio.h"
#include "config.h"

uint8_t radio_read(uint8_t reg){
    uint8_t data_tx[2] = {reg & 0x7F, 0};
    uint8_t data_rx[2];
    gpio_put(RADIO_CS, 0);
    spi_write_read_blocking(spi1, data_tx, data_rx, 2);
    gpio_put(RADIO_CS, 1);
    return data_rx[1];
}

void radio_write(uint8_t reg, uint8_t data){
    uint8_t data_tx[2] = {reg | 0x80, data};
    gpio_put(RADIO_CS, 0);
    spi_write_blocking(spi1, data_tx, 2);
    gpio_put(RADIO_CS, 1);
}

void radio_init(){
    gpio_put(RADIO_RST, 0);
    sleep_ms(10);
    gpio_put(RADIO_RST, 1);
    sleep_ms(10);

    printf("SX1276 version: 0x%02X\n", radio_read(REG_VERSION));

    radio_write(REG_OP_MODE, MODE_SLEEP);
    sleep_ms(10);

    double freq = (double)RADIO_SSTV_FREQ * (1.0 + RADIO_PPM_CORR / 1000000.0);
    uint32_t freq_r = (uint32_t)(freq / RADIO_STEP);
    radio_write(REG_FRF_MSB, (freq_r >> 16) & 0xFF);
    radio_write(REG_FRF_MID, (freq_r >> 8) & 0xFF);
    radio_write(REG_FRF_LSB, freq_r & 0xFF);

    printf("Radio freq %.3f MHz, ppm %.1f\n");

    uint16_t dev = (uint16_t)(RADIO_DEV / RADIO_STEP);
    printf("deviation %d, %.0f Hz", dev, dev * RADIO_STEP);
    radio_write(REG_FDEV_MSB, (dev >> 8) & 0x3F);
    radio_write(REG_FDEV_LSB, dev & 0xFF);

    radio_write(REG_PA_CONFIG, (0b10000000 | RADIO_PA_LEVEL) | (RADIO_PA_MAX << 4));
    radio_write(REG_PA_DAC, RADIO_PA_BOOST ? 0x87: 0x84);

    radio_write(REG_PA_RAMP, 0x00); // Disable shaping (for now)

    radio_write(REG_PACKET_CONFIG2, 0X00); // Continuous mode

    radio_write(REG_OP_MODE, MODE_STDBY);
    sleep_ms(10);

    printf("init OK");
}