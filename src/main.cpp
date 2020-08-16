#include <ArduinoOTA.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "Arduino.h"
#include "webServer.h"
#include "wsServer.h"

const char *ssid = WIFI_SSID;      // Enter SSID
const char *password = WIFI_PASS;  // Enter Password

static struct led {
  uint8_t pin;
  uint8_t value;
  bool updated;
  bool inverse;
} led[3];

void initColors(uint8_t pinR = D7, uint8_t pinG = D6, uint8_t pinB = D5);
void loopColors(void);
void setColors(uint8_t R, uint8_t G, uint8_t B);
void setHtmlHexColor(const char *hexColor);  // #FFFFFF
uint32_t getColor();

void initOTA(uint16_t ota_port);
void loopOTA();

void initMDNS(const char *mdnsHost, uint16_t http_port, uint16_t ws_port,
              uint16_t ota_port);
void loopMDNS();

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();

  char mdnsHost[11];
  snprintf(mdnsHost, sizeof(mdnsHost), "led-%06x", ESP.getChipId());
  Serial.println(mdnsHost);

  WiFi.begin(ssid, password);
  initColors();
  initWsServer(81);
  initWebServer(80);
  initOTA(82);
  initMDNS(mdnsHost, 80, 81, 82);
}

void loop() {
  loopColors();
  // if (WiFi.status() == WL_CONNECTED) {
    loopWebServer();
    loopWsServer();
    loopOTA();
    loopMDNS();
  // }
}

void initColors(uint8_t pinR, uint8_t pinG, uint8_t pinB) {
  led[0] = {pin : pinR, value : 0, updated : false, inverse : true};
  led[1] = {pin : pinG, value : 0, updated : false, inverse : true};
  led[2] = {pin : pinB, value : 0, updated : false, inverse : true};

  struct led *pLed = led;
  for (uint8_t i = 0; i < 3; i++, pLed++) {
    pinMode(pLed->pin, OUTPUT);
  }
  analogWriteFreq(400);
  analogWriteRange(255);
}

uint8_t hexToBin(const char hex) {
  if (hex <= '9') {
    return ((hex - '0') & 0x0f);
  }
  if (hex <= 'F') {
    return (((hex - 'A') + 10) & 0x0f);
  }
  return (((hex - 'a') + 10) & 0x0f);
}

uint8_t getHex8ColorFromCharArray(const char *hexColor) {
  uint8_t value = 0x10 * hexToBin(*hexColor);
  return value + hexToBin(*++hexColor);
}

void setHtmlHexColor(const char *hexColor) {
  if (hexColor[0] != '#') return;
  uint32_t length = strnlen(hexColor, 8);
  if (length == 7) {
    setColors(getHex8ColorFromCharArray(hexColor + 1),
              getHex8ColorFromCharArray(hexColor + 3),
              getHex8ColorFromCharArray(hexColor + 5));
  }
}

void setColors(uint8_t R, uint8_t G, uint8_t B) {
  struct led *pLed = led;
  uint8_t rgb[3] = {R, G, B};
  for (uint8_t i = 0; i < 3; i++, pLed++) {
    if (pLed->value != rgb[i]) {
      pLed->updated = false;
      pLed->value = rgb[i];
    }
  }
}

uint32_t getColor() {
  return ((uint32_t)led[0].value * 0x10000 + (uint32_t)led[1].value * 0x100 +
          (uint32_t)led[2].value);
}

void loopColors() {
  struct led *pLed = led;
  for (uint8_t i = 0; i < 3; i++, pLed++) {
    if (!pLed->updated) {
      // Serial.printf("set led[%d] = %d\n", i, pLed->value);
      pLed->updated = true;
      if (pLed->inverse) {
        analogWrite(pLed->pin, (uint8_t)(~pLed->value));
      } else {
        analogWrite(pLed->pin, pLed->value);
      }
    }
  }
}

void initOTA(uint16_t ota_port) {
  ArduinoOTA.setPort(ota_port);
  ArduinoOTA.begin(false);
}

inline void loopOTA() { ArduinoOTA.handle(); }

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

inline void loopMDNS() { MDNS.update(); }