#pragma once
#include "Config.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void initNetwork();
void updateWebClients(SystemState &state);