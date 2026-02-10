#pragma once
#include "Config.h"
#include <Adafruit_LSM6DSOX.h>

void initMotionSensor();
void calibrateMotionSensor();
void updateMotionSensor(SystemState &state);