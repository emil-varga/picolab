#ifndef PINS_H
#define PINS_H

#define LED0_PIN 0
#define LED1_PIN 1
#define LED2_PIN 2
#define LED3_PIN 3
#define LED4_PIN 4

#define I2C_SCL 13
#define I2C_SDA 12

#define BMP280_I2C_ADDR 0x76
#define MPU6050_I2C_ADDR 0x68

#define PIEZO_ADC_CHAN 0
#define PIEZO_ADC_PIN (26 + PIEZO_ADC_CHAN)

#endif