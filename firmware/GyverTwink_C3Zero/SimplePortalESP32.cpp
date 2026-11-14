#include "SimplePortalESP32.h"

static DNSServer dnsServer;
static WebServer webServer(80);
static bool portalStarted = false;
static byte currentStatus = SP_ERROR;

PortalCfg portalCfg;

static const char connectPage[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head><meta name="viewport" content="width=device-width, initial-scale=1"></head><body>
<style>input[type=text],input[type=password]{margin-bottom:8px;font-size:20px}input[type=submit]{width:180px;height:60px;margin-bottom:8px;font-size:20px}</style>
<center><h3>WiFi settings</h3>
<form action="/connect" method="POST"><input type="text" name="ssid" placeholder="SSID"><br>
<input type="password" name="pass" placeholder="Pass"><br><input type="submit" value="Submit"></form>
<h3>Switch WiFi mode</h3>
<form action="/ap" method="POST"><input type="submit" value="Access Point"></form>
<form action="/local" method="POST"><input type="submit" value="Local Mode"></form>
<form action="/exit" method="POST"><input type="submit" value="Exit Portal"></form></center></body></html>)rawliteral";

static void sendPage() {
  webServer.send(200, "text/html", FPSTR(connectPage));
}

static void handleConnect() {
  webServer.arg("ssid").toCharArray(portalCfg.SSID, sizeof(portalCfg.SSID));
  webServer.arg("pass").toCharArray(portalCfg.pass, sizeof(portalCfg.pass));
  portalCfg.mode = WIFI_STA;
  currentStatus = SP_SUBMIT;
  sendPage();
}

static void handleAP() {
  portalCfg.mode = WIFI_AP;
  currentStatus = SP_SWITCH_AP;
  sendPage();
}

static void handleLocal() {
  portalCfg.mode = WIFI_STA;
  currentStatus = SP_SWITCH_LOCAL;
  sendPage();
}

static void handleExit() {
  currentStatus = SP_EXIT;
  sendPage();
}

void portalStart() {
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true, true);
  const IPAddress apIP(SP_AP_IP);
  const IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, subnet);
  WiFi.softAP(SP_AP_NAME);
  dnsServer.start(53, "*", apIP);

  webServer.onNotFound(sendPage);
  webServer.on("/connect", HTTP_POST, handleConnect);
  webServer.on("/ap", HTTP_POST, handleAP);
  webServer.on("/local", HTTP_POST, handleLocal);
  webServer.on("/exit", HTTP_POST, handleExit);
  webServer.begin();
  portalStarted = true;
  currentStatus = SP_ERROR;
}

void portalStop() {
  webServer.stop();
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  portalStarted = false;
}

bool portalTick() {
  if (!portalStarted) return false;
  dnsServer.processNextRequest();
  webServer.handleClient();
  if (currentStatus != SP_ERROR) {
    portalStop();
    return true;
  }
  return false;
}

void portalRun(uint32_t period) {
  const uint32_t startedAt = millis();
  portalStart();
  while (!portalTick()) {
    if (millis() - startedAt > period) {
      currentStatus = SP_TIMEOUT;
      portalStop();
      break;
    }
    delay(1);
  }
}

byte portalStatus() {
  return currentStatus;
}
