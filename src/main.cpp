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

#if PCI_VERSION_1
#define PIN_RED_LED D7
#define PIN_GREEN_LED D6
#define PIN_BLUE_LED D5
#define RBG_INVERT_LOGIC true
#elif PCI_VERSION_2
#define PIN_RED_LED D7
#define PIN_GREEN_LED D8
#define PIN_BLUE_LED D5
#define RBG_INVERT_LOGIC false
#else
#error "Define a PCI Version"
#endif

#ifdef WIFI_SSID
const char *ssid = WIFI_SSID;
#else
const char *ssid =
    "wifi";  // Digite no lugar da palavra wifi, o nome da rede wifi.
#endif

#ifdef WIFI_PASS
const char *password = WIFI_PASS;
#else
const char *password =
    "senha";  // Digite no lugar da palavra senha, a senha do wifi.
#endif

ESP8266WiFiMulti wifiMulti;
stripLedRgb leds(PIN_RED_LED, PIN_GREEN_LED, PIN_BLUE_LED, RBG_INVERT_LOGIC);

uint16_t httpPort = 80;
uint16_t websocketPort = 81;
uint16_t arduinoOtaPort = 82;
char hostName[] = "led-XXXXXX";

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();

  leds.begin();

  digitalWrite(LED_BUILTIN, true);
  pinMode(LED_BUILTIN, OUTPUT);

  snprintf(hostName, sizeof(hostName), "led-%06x", ESP.getChipId());
  Serial.println(hostName);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostName);

  wifiMulti.addAP(ssid, password);

  initWsServer(websocketPort, &leds);
  initWebServer(httpPort);
  ArduinoOTA.setPort(arduinoOtaPort);
  ArduinoOTA.begin(false);
}

void handleMDNS() {
  unsigned long now = millis();
  static unsigned long nextAnnounce = now;
  static unsigned long nextBegin = now;

  static uint8_t countAnnounce = 0;

  if (MDNS.isRunning()) {
    MDNS.update();

    if (now > nextAnnounce) {
      if (++countAnnounce < 3) {
        nextAnnounce = now + (countAnnounce * 1000);
      } else {
        countAnnounce = 0;
        nextAnnounce = now + 119000;
      }
      MDNS.announce();
    }
  } else {
    if (now > nextBegin) {
      nextBegin = now + 1000;
      if (MDNS.begin(hostName)) {
        MDNS.addService("http", "tcp", httpPort);
        MDNS.addService("ws", "tcp", websocketPort);
        MDNS.enableArduino(arduinoOtaPort);
        Serial.printf("Open http://%s.local:%u or http://%s:%u\n", hostName,
                      httpPort, WiFi.localIP().toString().c_str(), httpPort);
        nextAnnounce = now + 119000;
      }
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
