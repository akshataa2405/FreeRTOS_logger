#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include <stdbool.h>

#if defined(__arm__) || defined(__thumb__)
#include "stm32f4xx_hal.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MPU6050_I2C_ADDR_DEFAULT    0xD0U   /* (0x68 << 1) when AD0 is GND */
#define MPU6050_WHO_AM_I_VAL        0x68U

/* Registers */
#define MPU6050_REG_SMPLRT_DIV      0x19U
#define MPU6050_REG_CONFIG          0x1AU
#define MPU6050_REG_GYRO_CONFIG     0x1BU
#define MPU6050_REG_ACCEL_CONFIG    0x1CU
#define MPU6050_REG_ACCEL_XOUT_H    0x3BU
#define MPU6050_REG_PWR_MGMT_1      0x6BU
#define MPU6050_REG_WHO_AM_I        0x75U

/**
 * @brief Initializes MPU6050 sensor via I2C:
 * 1. Checks WHO_AM_I
 * 2. Wakes from sleep (PWR_MGMT_1 = 0)
 * 3. Sets DLPF (Digital Low Pass Filter) to 44 Hz
 * 4. Sets sample rate divider
 *
 * @param hi2c Pointer to HAL I2C handle
 * @return true on success, false on communication failure or bad WHO_AM_I
 */
#if defined(__arm__) || defined(__thumb__)
bool mpu6050_init(I2C_HandleTypeDef *hi2c);
bool mpu6050_read_all(I2C_HandleTypeDef *hi2c,
                      int16_t *ax, int16_t *ay, int16_t *az,
                      int16_t *gx, int16_t *gy, int16_t *gz);
#else
bool mpu6050_init(void *hi2c);
bool mpu6050_read_all(void *hi2c,
                      int16_t *ax, int16_t *ay, int16_t *az,
                      int16_t *gx, int16_t *gy, int16_t *gz);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MPU6050_H */
