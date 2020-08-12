/*
fas fa-sync
fas fa-toggle-off
fas fa-toggle-on
*/

var colorPicker = null;
var socket;
var socketStatus;
let connectCounter = 0;
let lastColorSended;

let host = new URL(document.URL).host;
let wsHost = "ws://" + host + ":81";//window.atob(token.substring(1));

var teste;

var syncColor = function() {
    let color = colorPicker.color.hexString;
    if (color == lastColorSended) {
        return;
    }
    if (!socket) {
        connect();
    }
    lastColorSended = color;
    socket.send(color);
}

var setBtnColor = function(btn) {
    colorPicker.color.rgbaString = btn.style.backgroundColor;
    syncColor();
}

// Handle any errors that occur.
var onError = function(error) {
    socketStatus.innerHTML = "Erro." + error.code;
};

// Show a connected message when the WebSocket is opened.
var onOpen = function(event) {
    socketStatus.innerHTML = "Conectado.";
};

var createInterface = function() {
    if (!colorPicker) {
        colorPicker = new iro.ColorPicker(".colorPicker", {
            width: 720,
            borderWidth: 5,
            borderColor: "#fff",
            handleRadius: 50
        });

        colorPicker.on(["input:change"], syncColor);
    }
}

// Handle messages sent by the server.
var onMessage = function(event) {
    colorPicker.color.hexString = event.data;
};

// Show a disconnected message when the WebSocket is closed.
var onClose = function(event) {
    socketStatus.innerHTML = "Desconectado." + (connectCounter++) + " Erro:" + event.code;
    setTimeout(connect(), 5000);
};

var connect = function() {
    // Create a new WebSocket.
    socketStatus.innerHTML = "Conectando...";
    try {
        socket.close();
    } catch (err) {};

    socket = new WebSocket(wsHost);
    socket.onopen = onOpen;
    socket.onmessage = onMessage;
    socket.onerror = onError;
    socket.onclose = onClose;
}

function runScript() {
    socketStatus = document.getElementById("status");
    createInterface();
    connect();
}