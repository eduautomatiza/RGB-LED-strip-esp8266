#include "webServer.h"
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <LittleFS.h>

const char *fsName = "LittleFS";
FS *fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();
static bool fsOK;

ESP8266WebServer server(80);

/*
   Read the given file from the filesystem and stream it back to the client
*/
bool sendFile(String path)
{
  if (path == "/")
  {
    path = "/index.html";
  }

  if (fileSystem->exists(path))
  {
    File file = fileSystem->open(path, "r");
    server.streamFile(file, mime::getContentType(path));
    file.close();
    return true;
  }

  return false;
}


void initWebServer(void)
{
  fileSystemConfig.setAutoFormat(false);
  fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();

  // Use it to read files from filesystem
  server.onNotFound([]() {
    if (!fsOK)
    {
      server.send(500, "text/plain", "File system error!\r\n");
      return;
    }

    if (!sendFile(ESP8266WebServer::urlDecode(server.uri())))
    {
      server.send(404, "text/plain", "File not found!\r\n");
    }
  });

  // Start server
  server.begin();
}

void loopWebServer(void)
{
  server.handleClient();
}