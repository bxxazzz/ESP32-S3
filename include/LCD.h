// LCD Interface : 16Bit RGB 병렬 방식.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

void LCD_INIT(void);
void LCD_FILL(uint8_t R, uint8_t G, uint8_t B);

#ifdef __cplusplus
}
#endif