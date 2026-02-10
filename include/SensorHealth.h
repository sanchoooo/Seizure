#pragma once
#include "Config.h"
#include "MAX30105.h"
#include "spo2_algorithm.h"

void initHealthSensor();
void updateHealthSensor(SystemState &state);