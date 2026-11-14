#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

#define SP_AP_NAME "ESP Config"
#define SP_AP_IP 192, 168, 1, 1

#define SP_ERROR 0
#define SP_SUBMIT 1
#define SP_SWITCH_AP 2
#define SP_SWITCH_LOCAL 3
#define SP_EXIT 4
#define SP_TIMEOUT 5

struct PortalCfg {
  char SSID[32] = "";
  char pass[32] = "";
  uint8_t mode = WIFI_AP;
};

extern PortalCfg portalCfg;

void portalStart();
void portalStop();
bool portalTick();
void portalRun(uint32_t prd = 60000);
byte portalStatus();
