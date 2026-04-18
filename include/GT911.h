// LCD Interface : 16Bit RGB 병렬 방식.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "esp_err.h"

typedef struct 
{
    bool pressed;
    uint8_t count;
    uint16_t x;
    uint16_t y;
} TOUCH_POINT;

extern TOUCH_POINT LCD_TOUCH;

esp_err_t GT911_INIT();
void TOUCH_READ();

#ifdef __cplusplus
}
#endif