#pragma once
#include <stdint.h>
#include "stripLedRgb.h"

int initWsServer(uint16_t ws_port,stripLedRgb *leds);
void handleWsServer(void);
