#include "hardware/i2c.h"

//calibration data
struct bmp280_calibration {
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
};

typedef struct {
    int16_t accel[3];
    uint16_t temp;
    int16_t gyro[3];
} mpu6050_reading;

extern struct bmp280_calibration cal;

int bmp280_read(i2c_inst_t *i2c, int32_t *temperature, int32_t *pressure);
void bmp280_read_calibration(i2c_inst_t *i2c);
int write_register(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes);
int read_registers(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes);

int mpu6050_read_all(i2c_inst_t *i2c, mpu6050_reading *reading);