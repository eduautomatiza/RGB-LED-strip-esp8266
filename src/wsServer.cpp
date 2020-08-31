
#include "wsServer.h"

#include <iostream>
#include <string>
#include <tiny_websockets/client.hpp>
#include <tiny_websockets/server.hpp>
#include <vector>

#include "ArduinoJson.h"

// a collection of all connected clients
std::vector<websockets::WebsocketsClient> allClients;
websockets::WebsocketsServer wsServer;
stripLedRgb *_leds;

#define StaticJsonDocument_length 256
static StaticJsonDocument<StaticJsonDocument_length> doc;

// this method goes thrugh every client and polls for new messages and events
void pollAllClients();
void sendConfig(websockets::WebsocketsClient *client);

// this callback is common for all clients, the client that sent that
// message is the one that gets the echo response
void onMessage(websockets::WebsocketsClient &client, websockets::WebsocketsMessage message) {
  DeserializationError error = deserializeJson(doc, message.data());
  if (error) {
    return;
  }

  if (doc.containsKey("hsl")) {
    JsonObject hsl = doc["hsl"];
    if (hsl.containsKey("h") && hsl.containsKey("s") && hsl.containsKey("l")) {
      _leds->hsl(hsl["h"], hsl["s"], hsl["l"]);
    }
  }

  if (doc.containsKey("time")) {
    _leds->time_step(doc["time"]);
  }

  if (doc.containsKey("angle")) {
    _leds->angle_step(doc["angle"]);
  }

  if (doc.containsKey("duty")) {
    _leds->duty_cycle(doc["duty"]);
  }
}

int initWsServer(uint16_t ws_port, stripLedRgb *leds) {
  wsServer.listen(ws_port);
  _leds = leds;
  return 0;
}

void handleWsServer(void) {
  // while the server is alive
  if (wsServer.available()) {
    // if there is a client that wants to connect
    if (wsServer.poll()) {
      // accept the connection and register callback
      websockets::WebsocketsClient client = wsServer.accept();
      client.onMessage(onMessage);
      sendConfig(&client);
      // store it for later use
      allClients.push_back(client);
    }
    // check for updates in all clients
    pollAllClients();
  }
}

void pollAllClients() {
  if (allClients.empty()) {
    return;
  }

  for (auto &client : allClients) {
    client.poll();
  }
}

void sendConfig(websockets::WebsocketsClient *client){

  doc.clear();

  doc["hsl"]["h"] = _leds->hue();
  doc["hsl"]["s"] = _leds->saturation();
  doc["hsl"]["l"] = _leds->lightness();

  doc["time"] = _leds->time_step();
  doc["angle"] = _leds->angle_step();
  doc["duty"] = _leds->duty_cycle();

  String msg;
  msg.reserve(128);
  serializeJson(doc,msg);
  client->send(msg);
}
