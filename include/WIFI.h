#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define WIFI_DISCONNECTED 0
#define WIFI_CONNECTED    1

extern uint8_t WIFI_STATUS;

void WIFI_INIT();

#ifdef __cplusplus
}
#endif