#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "radio.h"
#include "config.h"

static uint8_t radio_read(uint8_t reg){
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

static void radio_set_bits(uint8_t reg, uint8_t mask){
    radio_write(reg, radio_read(reg) | mask);
}

static void radio_reset_bits(uint8_t reg, uint8_t mask){
    radio_write(reg, radio_read(reg) & (~mask));
}

static void radio_write_mask(uint8_t reg, uint8_t data, uint8_t mask){
    radio_write(reg, (radio_read(reg) & (~mask)) | (data & mask));
}

void radio_init(){
    spi_init(spi1, 10000000);
    gpio_set_function(RADIO_SCK, GPIO_FUNC_SPI);
    gpio_set_function(RADIO_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(RADIO_MISO, GPIO_FUNC_SPI);
    
    gpio_init(RADIO_CS);
    gpio_set_dir(RADIO_CS, GPIO_OUT);
    gpio_put(RADIO_CS, 1);
    
    gpio_init(RADIO_RST);
    gpio_set_dir(RADIO_RST, GPIO_OUT);
    gpio_put(RADIO_RST, 1);
}

void radio_start(){
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

    printf("Radio freq %.3f MHz, ppm %.1f\n", freq/1000000.0, RADIO_PPM_CORR);

    uint16_t dev = (uint16_t)(RADIO_DEV / RADIO_STEP);
    printf("deviation %d: %.0f Hz\n", dev, dev * RADIO_STEP);
    radio_write(REG_FDEV_MSB, (dev >> 8) & 0x3F);
    radio_write(REG_FDEV_LSB, dev & 0xFF);

    radio_set_bits(REG_PA_CONFIG, 0b10000000);
    radio_write_mask(REG_PA_CONFIG, RADIO_PA_LEVEL, 0b00001111);
    radio_write_mask(REG_PA_CONFIG, RADIO_PA_MAX << 4, 0b01110000);
    radio_write(REG_PA_DAC, RADIO_PA_BOOST ? 0x87: 0x84);

    printf("Power settings: level %d/15, max %d/7, boost %d\n", RADIO_PA_LEVEL, RADIO_PA_MAX, RADIO_PA_BOOST);

    radio_reset_bits(REG_PA_RAMP, 0b01100000); // Disable shaping (for now)

    radio_reset_bits(REG_PACKET_CONFIG2, 0b01000000); // Continuous mode

    printf("radio init OK\n");
    sleep_ms(10);
}