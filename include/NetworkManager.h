#pragma once
#include "Config.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Updated to accept SystemState so we can save the IP address
void initNetwork(SystemState &state);
void updateWebClients(SystemState &state);