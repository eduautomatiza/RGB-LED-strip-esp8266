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

ESP8266WiFiMulti wifiMulti;
stripLedRgb leds(D7, D8, D5);

uint16_t httpPort = 80;
uint16_t websocketPort = 81;
uint16_t arduinoOtaPort = 82;
char hostName[] = "led-XXXXXX";

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();

  digitalWrite(LED_BUILTIN, true);
  pinMode(LED_BUILTIN, OUTPUT);

  snprintf(hostName, sizeof(hostName), "led-%06x", ESP.getChipId());
  Serial.println(hostName);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostName);

  wifiMulti.addAP(ssid, password);

  leds.begin();

  initWsServer(websocketPort, &leds);
  initWebServer(httpPort);
  ArduinoOTA.setPort(arduinoOtaPort);
  ArduinoOTA.begin(false);
}

void handleMDNS() {
  unsigned long now = millis();
  static unsigned long nextAnnounce = now;

  if (MDNS.isRunning()) {
    MDNS.update();
    if (now > nextAnnounce) {
      nextAnnounce = now + 100000;
      MDNS.announce();
    }
  } else {
    if (MDNS.begin(hostName)) {
      MDNS.addService("http", "tcp", httpPort);
      MDNS.addService("ws", "tcp", websocketPort);
      MDNS.enableArduino(arduinoOtaPort);
      Serial.printf("Open http://%s.local:%u or http://%s:%u\n", hostName,
                    httpPort, WiFi.localIP().toString().c_str(), httpPort);
    }
  }
}

void loop() {
  auto status = wifiMulti.run();
  digitalWrite(LED_BUILTIN, !(status == WL_CONNECTED));
  leds.handle();
  handleWebServer();
  handleWsServer();
  ArduinoOTA.handle();
  handleMDNS();
}
