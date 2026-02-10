#pragma once
#include "Config.h"

// Initialize the OLED using U8x8
void initDisplay();

// Draw Status Screen
void updateDisplay(SystemState &state);