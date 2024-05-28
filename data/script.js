/*
fas fa-sync
fas fa-toggle-off
fas fa-toggle-on
*/

var colorPicker = null;
var socket;
let element;

const table = {
  time: [0, 1000, 500, 200, 100],
  angle: [0, 180, 120, 90, 60, 1],
  duty: [1, 0.5, 0.1],
};

var setBtnColor = function (btn) {
  colorPicker.color.rgbString = btn.style.backgroundColor;
  sendToDevice({ hsl: colorPicker.color.hsl });
};

// Handle any errors that occur.
var onError = function (error) {
  element.status.innerHTML = "Erro." + error.code;
};

// Show a connected message when the WebSocket is opened.
var onOpen = function (event) {
  element.status.innerHTML = "Conectado.";
};

var createInterface = function () {
  element = {
    status: document.getElementById("status"),
    colorPicker: document.getElementById("colorPicker"),
    time: document.getElementById("timeSlider"),
    angle: document.getElementById("angleSlider"),
    duty: document.getElementById("dutySlider"),
  };

  colorPicker = new iro.ColorPicker(element.colorPicker, {
    id: "elementColorPicker",
    width: 300,
    borderWidth: 2,
    borderColor: "#fff",
    handleRadius: 15,
    layout: [
      {
        component: iro.ui.Wheel,
        options: {
          wheelLightness: false,
        },
      },
      {
        component: iro.ui.Slider,
        options: {
          sliderType: "value",
        },
      },
    ],
  });

  colorPicker.on(["input:change"], function () {
    sendToDevice({ hsl: colorPicker.color.hsl });
  });
  element.time.addEventListener("input", function () {
    sendToDevice({ time: Number(table.time[this.value]) });
  });
  element.angle.addEventListener("input", function () {
    sendToDevice({ angle: Number(table.angle[this.value]) });
  });
  element.duty.addEventListener("input", function () {
    sendToDevice({ duty: Number(table.duty[this.value]) });
  });

  element.time.max = table.time.length - 1;
  element.angle.max = table.angle.length - 1;
  element.duty.max = table.duty.length - 1;
};

// Handle messages sent by the server.
var onMessage = function (event) {
  try {
    let msg = JSON.parse(event.data);
    try {
      colorPicker.color.hsl = msg.hsl;
    } catch (e) {}
    try {
      element.time.value = table.time.indexOf(msg.time);
    } catch (e) {}
    try {
      element.angle.value = table.angle.indexOf(msg.angle);
    } catch (e) {}
    try {
      element.duty.value = table.duty.indexOf(msg.duty);
    } catch (e) {}
  } catch (e) {}
};

// Show a disconnected message when the WebSocket is closed.
var onClose = function (event) {
  element.status.innerHTML = "Desconectado.";
  setTimeout(connect(), 5000);
};

var connect = function () {
  // Create a new WebSocket.
  element.status.innerHTML = "Conectando...";
  try {
    socket.close();
  } catch (err) {}

  if (!wsHost) {
    if (document.location.protocol === "https:") {
      var wsHost = "wss://" + document.location.hostname + ":81";
    } else {
      var wsHost = "ws://" + document.location.hostname + ":81";
    }
  }

  socket = new WebSocket(wsHost, "json");
  socket.onopen = onOpen;
  socket.onmessage = onMessage;
  socket.onerror = onError;
  socket.onclose = onClose;
};

var sendToDevice = function (msg) {
  if (!socket) {
    connect();
  }
  if (socket) {
    socket.send(JSON.stringify(msg));
  }
};

function runScript() {
  createInterface();
  connect();
}
