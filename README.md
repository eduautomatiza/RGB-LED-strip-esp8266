# RGB-LED-strip-esp8266

This is a project to drive RGB led strips.

The activation is done through a local web interface.

On devices that work with a ".local" domain, the page can be loaded by name. For example: led-10c3b4.local. For other cases, the interface is accessed by IP. Ex .: 192.168.0.101.

The web page is served by ESP8266 on TCP port 80.

To ensure agility, the interaction of the color selection is done using websockts on TCP port 81.

The firmware accepts updating using OTA on port 82 or via the serial interface.

The color selection disc uses the lib available at [https://github.com/jaames/iro.js](https://github.com/jaames/iro.js)

[![Operation 1](http://img.youtube.com/vi/vLKXMp9dhpw/0.jpg)](https://youtu.be/vLKXMp9dhpw)

[![Operation 2](http://img.youtube.com/vi/u9i37xKtStI/0.jpg)](https://youtu.be/u9i37xKtStI)

![Browser Interface](./doc/interface.png)

![botton](./doc/botton.jpg)

![top1](./doc/top1.jpg)

![top2](./doc/top2.jpg)

![connections](./doc/connections.jpg)
