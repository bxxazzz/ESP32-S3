// LCD Interface : 16Bit RGB 병렬 방식.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define LVGL_USE    

void LCD_INIT();
void LCD_FILL(uint8_t R, uint8_t G, uint8_t B);
void LVGL_INIT();

#ifdef __cplusplus
}
#endif