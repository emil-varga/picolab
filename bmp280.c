/*
SPI communication with the bmp280 device. Most of the code taken from bme280 example of the pico sdk
*/
#include "bmp280.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pins.h"

#define READ_BIT 0x80

int32_t t_fine;
int32_t compensate_temp(int32_t adc_T, const struct bmp280_calibration *cal) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t) cal->dig_T1 << 1))) * ((int32_t) cal->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t) cal->dig_T1)) * ((adc_T >> 4) - ((int32_t) cal->dig_T1))) >> 12) * ((int32_t) cal->dig_T3))
            >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

uint32_t compensate_pressure(int32_t adc_P, const struct bmp280_calibration *cal) {
    int32_t var1, var2;
    uint32_t p;
    var1 = (((int32_t) t_fine) >> 1) - (int32_t) 64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t) cal->dig_P6);
    var2 = var2 + ((var1 * ((int32_t) cal->dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t) cal->dig_P4) << 16);
    var1 = (((cal->dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t) cal->dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t) cal->dig_P1)) >> 15);
    if (var1 == 0)
        return 0;

    p = (((uint32_t) (((int32_t) 1048576) - adc_P) - (var2 >> 12))) * 3125;
    if (p < 0x80000000)
        p = (p << 1) / ((uint32_t) var1);
    else
        p = (p / (uint32_t) var1) * 2;

    var1 = (((int32_t) cal->dig_P9) * ((int32_t) (((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t) (p >> 2)) * ((int32_t) cal->dig_P8)) >> 13;
    p = (uint32_t) ((int32_t) p + ((var1 + var2 + cal->dig_P7) >> 4));

    return p;
}

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(BMP280_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(BMP280_CS, 1);
    asm volatile("nop \n nop \n nop");
}

void write_register(spi_inst_t *spi, uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg & 0x7f;  // remove read bit as this is a write
    buf[1] = data;
    cs_select();
    spi_write_blocking(spi, buf, 2);
    cs_deselect();
    sleep_ms(10);
}

void read_registers(spi_inst_t *spi, uint8_t reg, uint8_t *buf, uint16_t len) {
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.
    reg |= READ_BIT;
    cs_select();
    spi_write_blocking(spi, &reg, 1);
    sleep_ms(10);
    spi_read_blocking(spi, 0, buf, len);
    cs_deselect();
    sleep_ms(10);
}

struct bmp280_calibration cal;
/* This function reads the manufacturing assigned compensation parameters from the device */
void bmp280_read_calibration(spi_inst_t *spi) {
    uint8_t buffer[26];

    read_registers(spi, 0x88, buffer, 24);

    cal.dig_T1 = buffer[0] | (buffer[1] << 8);
    cal.dig_T2 = buffer[2] | (buffer[3] << 8);
    cal.dig_T3 = buffer[4] | (buffer[5] << 8);

    cal.dig_P1 = buffer[6] | (buffer[7] << 8);
    cal.dig_P2 = buffer[8] | (buffer[9] << 8);
    cal.dig_P3 = buffer[10] | (buffer[11] << 8);
    cal.dig_P4 = buffer[12] | (buffer[13] << 8);
    cal.dig_P5 = buffer[14] | (buffer[15] << 8);
    cal.dig_P6 = buffer[16] | (buffer[17] << 8);
    cal.dig_P7 = buffer[18] | (buffer[19] << 8);
    cal.dig_P8 = buffer[20] | (buffer[21] << 8);
    cal.dig_P9 = buffer[22] | (buffer[23] << 8);

    read_registers(spi, 0xE1, buffer, 8);
}

static void bme280_read_raw(spi_inst_t *spi, int32_t *pressure, int32_t *temperature) {
    uint8_t buffer[8];

    read_registers(spi, 0xF7, buffer, 8);
    *pressure = ((uint32_t) buffer[0] << 12) | ((uint32_t) buffer[1] << 4) | (buffer[2] >> 4);
    *temperature = ((uint32_t) buffer[3] << 12) | ((uint32_t) buffer[4] << 4) | (buffer[5] >> 4);
}

int bmp280_read(spi_inst_t *spi, int32_t *temperature, int32_t *pressure)
{
    int32_t adc_P, adc_T;
    bme280_read_raw(spi, &adc_P, &adc_T);

    *temperature = compensate_temp(adc_T, &cal);
    *pressure = compensate_pressure(adc_P, &cal);
}