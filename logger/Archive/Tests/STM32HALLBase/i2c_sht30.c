uint8_t cmd[2] = {0x2C, 0x06};
HAL_I2C_Master_Transmit(&hi2c1, 0x44 << 1, cmd, 2, 100);
HAL_Delay(15);

uint8_t buf[6];
HAL_I2C_Master_Receive(&hi2c1, 0x44 << 1, buf, 6, 100);

uint16_t t_raw  = (buf[0] << 8) | buf[1];
uint16_t rh_raw = (buf[3] << 8) | buf[4];

float t = -45.0f + 175.0f * (t_raw / 65535.0f);
float h = 100.0f * (rh_raw / 65535.0f);