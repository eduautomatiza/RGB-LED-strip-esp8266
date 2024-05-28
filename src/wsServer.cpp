
#include "wsServer.h"

#include <iostream>
#include <string>
#include <tiny_websockets/client.hpp>
#include <tiny_websockets/server.hpp>

#include "ArduinoJson.h"

// a collection of all connected clients
const size_t maxSocketClients = 16;
websockets::WebsocketsClient socketClients[maxSocketClients];
websockets::WebsocketsServer wsServer;
stripLedRgb *_leds;

#define StaticJsonDocument_length 256
static StaticJsonDocument<StaticJsonDocument_length> doc;

int32_t getFreeSocketClientIndex(void);
void sendConfigToOneClient(websockets::WebsocketsClient *client);
void sendConfigAllClientExceptThis(websockets::WebsocketsClient *client);

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
  sendConfigAllClientExceptThis(&client);
}

int initWsServer(uint16_t ws_port, stripLedRgb *leds) {
  wsServer.listen(ws_port);
  _leds = leds;
  return 0;
}

void handleWsServer(void) {
  // if there is a client that wants to connect
  if (wsServer.poll()) {
    // accept the connection and register callback
    websockets::WebsocketsClient newClient = wsServer.accept();
    int32_t freeIndex = getFreeSocketClientIndex();
    if (freeIndex >= 0) {
      newClient.onMessage(onMessage);
      socketClients[freeIndex] = newClient;
      sendConfigToOneClient(&newClient);
    } else {
      newClient.close();
    }
  }
  for (size_t i = 0; i < maxSocketClients; i++) {
    socketClients[i].poll();
  }
  if (_leds->hasUpdate()) {
    sendConfigAllClientExceptThis(nullptr);
  }
}

int32_t getFreeSocketClientIndex(void) {
  // If a client in our list is not available, it's connection is closed and
  // we can use it for a new client.
  for (size_t i = 0; i < maxSocketClients; i++) {
    if (!socketClients[i].available()) return i;
  }
  return -1;
}

String getConfigToSend(void) {
  doc.clear();

  doc["hsl"]["h"] = _leds->hue();
  doc["hsl"]["s"] = _leds->saturation();
  doc["hsl"]["l"] = _leds->lightness();

  doc["time"] = _leds->time_step();
  doc["angle"] = _leds->angle_step();
  doc["duty"] = _leds->duty_cycle();

  String msg;
  msg.reserve(128);
  serializeJson(doc, msg);
  return msg;
}

void sendConfigToOneClient(websockets::WebsocketsClient *client) {
  String msg = getConfigToSend();
  if (client->available()) {
    client->send(msg);
  }
}

void sendConfigAllClientExceptThis(websockets::WebsocketsClient *client) {
  String msg = getConfigToSend();
  for (size_t i = 0; i < maxSocketClients; i++) {
    websockets::WebsocketsClient *socketClient = &socketClients[i];
    if (client != socketClient && socketClient->available()) {
      socketClient->send(msg);
    }
  }
}
