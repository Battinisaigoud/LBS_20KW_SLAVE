//// @file evse_espnow_slave.h
//// @Author: Gopikrishna S
//// @date 07/11/2024
//// @brief ESP-NOW Slave Code
//// @All Rights Reserved EVRE

#pragma once

#include "Arduino.h"
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Channel to be used by the ESP-NOW protocol
#define ESPNOW_WIFI_CHANNEL (2)
#define PACK_DEBUG_EN (0)

void initEspNowSlave(void);
void send_stop_charge_status(void);
void send_start_charge_status(void);
void send_power_status(void);