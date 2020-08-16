/*
  Single threaded Echo Server that can handle multiple clients.

  After running this demo, there will be a websockets server running
  on port SERVER_PORT (default 8080), that server will accept connections
  and will respond with "echo" messages for every received message.

  The code:
    1. Sets up a server
    2. While possible, checks if there is a client wanting to connect
      2-1. If there is, accept the connection
      2-2. Set up callbacks (for incoming messages)
      2-3. Store the client in a collection for future use
    3. For every client in the collection, poll for incoming events

    When a message is received: respond to that message with `"Echo: " +
  message.data()`
*/

#include <iostream>
#include <string>
#include <tiny_websockets/client.hpp>
#include <tiny_websockets/server.hpp>
#include <vector>

extern void setHtmlHexColor(const char *hexColor);
extern uint32_t getColor();

using namespace websockets;

// a collection of all connected clients
std::vector<WebsocketsClient> allClients;
String lastColor;
bool sendColor = false;
WebsocketsClient *senderColor = nullptr;

WebsocketsServer wsServer;

// this method goes thrugh every client and polls for new messages and events
void pollAllClients() {
  for (auto &client : allClients) {
    client.poll();
  }

  for (auto &client : allClients) {
    if (sendColor) {
      if ((WebsocketsClient *)&client != senderColor) {
        client.send(lastColor.c_str());
      }
    }
  }
  sendColor = false;
}

void colorToSend(WebsocketsClient *client, String color) {
  if (!client || (color != lastColor)) {
    if (client) {
      setHtmlHexColor(color.c_str());
    }
    lastColor = color;
    sendColor = true;
    senderColor = client;
  }
}

// this callback is common for all clients, the client that sent that
// message is the one that gets the echo response
void onMessage(WebsocketsClient &client, WebsocketsMessage message) {
  colorToSend(&client, message.data());
}

int initWsServer(uint16_t ws_port) {
  wsServer.listen(ws_port);
  return 0;
}

String CurrentColor() {
  char color[8];
  snprintf(color, 8, "#%06x", getColor());
  return color;
}

void loopWsServer(void) {
  // while the server is alive
  if (wsServer.available()) {
    // if there is a client that wants to connect
    if (wsServer.poll()) {
      // accept the connection and register callback
      WebsocketsClient client = wsServer.accept();
      client.onMessage(onMessage);

      colorToSend(NULL, CurrentColor());

      // store it for later use
      allClients.push_back(client);
    }

    // check for updates in all clients
    pollAllClients();
  }
}