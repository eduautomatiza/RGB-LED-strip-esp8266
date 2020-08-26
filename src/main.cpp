#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <math.h>

#include "rgb_t.h"
#include "stripLedRgb.h"
#include "webServer.h"
#include "wsServer.h"

const char *ssid = WIFI_SSID;      // Enter SSID
const char *password = WIFI_PASS;  // Enter Password

void initMDNS(const char *mdnsHost, uint16_t http_port, uint16_t ws_port,
              uint16_t ota_port);

ESP8266WiFiMulti wifiMulti;
stripLedRgb leds(D7, D8, D5);

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();

  digitalWrite(LED_BUILTIN, true);
  pinMode(LED_BUILTIN, OUTPUT);

  char mdnsHost[11];
  snprintf(mdnsHost, sizeof(mdnsHost), "led-%06x", ESP.getChipId());
  Serial.println(mdnsHost);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(mdnsHost);

  wifiMulti.addAP(ssid, password);

  leds.begin();

  initWsServer(81, &leds);
  initWebServer(80);
  ArduinoOTA.setPort(82);
  ArduinoOTA.begin(false);
  initMDNS(mdnsHost, 80, 81, 82);
}

void loop() {
  auto status = wifiMulti.run();

  if (status == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, false);
  } else {
    digitalWrite(LED_BUILTIN, true);
  }

  leds.handle();
  handleWebServer();
  handleWsServer();
  ArduinoOTA.handle();
  MDNS.update();
}

void initMDNS(const char *mdnsHost, uint16_t http_port, uint16_t ws_port,
              uint16_t ota_port) {
  String host = mdnsHost;
  static auto onGotIP =
      WiFi.onStationModeGotIP([host, http_port, ws_port, ota_port](
                                  const WiFiEventStationModeGotIP &event) {
        if (MDNS.begin(host)) {
          MDNS.addService("http", "tcp", http_port);
          MDNS.addService("ws", "tcp", ws_port);
          MDNS.enableArduino(ota_port);
          Serial.printf("Open http://%s.local\n", host.c_str());
        } else {
          MDNS.notifyAPChange();
        }
        Serial.printf("Connected(RSSI=%d, CH=%d, BSSID=%s, IP=%s)\n",
                      WiFi.RSSI(), WiFi.channel(), WiFi.BSSIDstr().c_str(),
                      WiFi.localIP().toString().c_str());
      });

  static auto onLostIP = WiFi.onStationModeDisconnected(
      [](const WiFiEventStationModeDisconnected &event) {
        MDNS.notifyAPChange();
        Serial.println("Disconnected");
      });
}
