#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

void UI_INIT();
void DRAW_LABEL(uint32_t color, const char *text);
void DISPLAY_MAIN();

extern uint8_t DSP_LATCH;
extern uint8_t BG_COLOR;

#ifdef __cplusplus
}
#endif