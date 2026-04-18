// Backlight : CH422G

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void CH422G_INIT(void);
void CH422G_GPIO_WRITE(uint8_t value);
void CH422G_SET(uint8_t pin, uint8_t level);
void I2C_SCAN(void);

#ifdef __cplusplus
}
#endif