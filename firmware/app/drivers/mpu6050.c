#include "mpu6050.h"

#if defined(__arm__) || defined(__thumb__)
static HAL_StatusTypeDef i2c_write_reg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(hi2c, MPU6050_I2C_ADDR_DEFAULT, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

static HAL_StatusTypeDef i2c_read_reg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *data) {
    return HAL_I2C_Mem_Read(hi2c, MPU6050_I2C_ADDR_DEFAULT, reg, I2C_MEMADD_SIZE_8BIT, data, 1, 100);
}

bool mpu6050_init(I2C_HandleTypeDef *hi2c) {
    uint8_t who_am_i = 0;
    if (i2c_read_reg(hi2c, MPU6050_REG_WHO_AM_I, &who_am_i) != HAL_OK) {
        return false;
    }

    if (who_am_i != MPU6050_WHO_AM_I_VAL) {
        return false;
    }

    /* 1. Wake up device: PWR_MGMT_1 = 0x00 (internal 8MHz oscillator) */
    if (i2c_write_reg(hi2c, MPU6050_REG_PWR_MGMT_1, 0x00) != HAL_OK) return false;

    /* 2. Sample Rate Divider: SMPLRT_DIV = 7 -> 1kHz / (1 + 7) = 125 Hz internal */
    if (i2c_write_reg(hi2c, MPU6050_REG_SMPLRT_DIV, 0x07) != HAL_OK) return false;

    /* 3. DLPF Config: CONFIG = 0x03 -> DLPF 44 Hz (accel delay 4.9ms) */
    if (i2c_write_reg(hi2c, MPU6050_REG_CONFIG, 0x03) != HAL_OK) return false;

    /* 4. Gyro Config: +/- 250 deg/s */
    if (i2c_write_reg(hi2c, MPU6050_REG_GYRO_CONFIG, 0x00) != HAL_OK) return false;

    /* 5. Accel Config: +/- 2g */
    if (i2c_write_reg(hi2c, MPU6050_REG_ACCEL_CONFIG, 0x00) != HAL_OK) return false;

    return true;
}

bool mpu6050_read_all(I2C_HandleTypeDef *hi2c,
                      int16_t *ax, int16_t *ay, int16_t *az,
                      int16_t *gx, int16_t *gy, int16_t *gz) {
    uint8_t raw_buf[14];

    /* Burst read 14 bytes starting at 0x3B: Accel(6) + Temp(2) + Gyro(6) */
    if (HAL_I2C_Mem_Read(hi2c, MPU6050_I2C_ADDR_DEFAULT, MPU6050_REG_ACCEL_XOUT_H,
                         I2C_MEMADD_SIZE_8BIT, raw_buf, sizeof(raw_buf), 50) != HAL_OK) {
        return false;
    }

    if (ax) *ax = (int16_t)((raw_buf[0] << 8) | raw_buf[1]);
    if (ay) *ay = (int16_t)((raw_buf[2] << 8) | raw_buf[3]);
    if (az) *az = (int16_t)((raw_buf[4] << 8) | raw_buf[5]);
    /* raw_buf[6..7] is temperature */
    if (gx) *gx = (int16_t)((raw_buf[8] << 8) | raw_buf[9]);
    if (gy) *gy = (int16_t)((raw_buf[10] << 8) | raw_buf[11]);
    if (gz) *gz = (int16_t)((raw_buf[12] << 8) | raw_buf[13]);

    return true;
}
#else
bool mpu6050_init(void *hi2c) { (void)hi2c; return true; }
bool mpu6050_read_all(void *hi2c, int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx, int16_t *gy, int16_t *gz) {
    (void)hi2c;
    if (ax) *ax = 0; if (ay) *ay = 0; if (az) *az = 16384;
    if (gx) *gx = 0; if (gy) *gy = 0; if (gz) *gz = 0;
    return true;
}
#endif
