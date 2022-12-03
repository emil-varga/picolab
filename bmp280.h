#include "hardware/spi.h"

//calibration data
struct bmp280_calibration {
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
};

extern struct bmp280_calibration cal;

int bmp280_read(spi_inst_t *spi, int32_t *temperature, int32_t *pressure);
void bmp280_read_calibration(spi_inst_t *spi);
void write_register(spi_inst_t *spi, uint8_t reg, uint8_t data);
void read_registers(spi_inst_t *spi, uint8_t reg, uint8_t *buf, uint16_t len);