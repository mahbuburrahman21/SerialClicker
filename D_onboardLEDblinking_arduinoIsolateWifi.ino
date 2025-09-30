fully working

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <WebSockets.h>

// Create Access Point
const char* ssid = "LEDBlinkControl";
const char* password = "12345678";

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const int ledPin = 2; // Built-in LED on ESP32 (GPIO2)

unsigned long clickDelay = 1000; // Blink delay (ms)
int numberOfBlinks = 10;
bool isRunning = false;
bool isPaused = false;
int blinksDone = 0;
unsigned long lastBlinkTime = 0;

// HTML Page with hr, min, sec input
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 LED Blink Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: #f2f2f2; }
    .container { margin: auto; margin-top: 30px; width: 300px; background: white; padding: 20px; border-radius: 10px;
                 box-shadow: 0 4px 8px rgba(0,0,0,0.2); }
    h1 { font-size: 20px; }
    label { display: block; margin: 10px 0 5px; text-align: left; }
    input { width: 100%; padding: 8px; margin-bottom: 10px; }
    button { border: none; color: white; padding: 10px; margin: 5px; width: 45%; border-radius: 8px; cursor: pointer; }
    #set { background: #4CAF50; } #set:hover { background: #45a049; }
    #stop { background: #f44336; } #stop:hover { background: #da190b; }
    #pause { background: #ff9800; } #pause:hover { background: #e68a00; }
    #reset { background: #555; } #reset:hover { background: #333; }
    #status { margin-top: 15px; font-weight: bold; }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 LED Blink Control</h1>
    <label>Hours:</label>
    <input type="number" id="hr" value="0" min="0">
    <label>Minutes:</label>
    <input type="number" id="min" value="0" min="0">
    <label>Seconds:</label>
    <input type="number" id="sec" value="1" min="1">
    <label>Number of Blinks:</label>
    <input type="number" id="blinks" value="10" min="1">
    <br>
    <button id="set">Set & Start</button>
    <button id="stop">Stop</button>
    <button id="pause">Pause</button>
    <button id="reset">Reset</button>
    <div id="status">Status: Idle</div>
  </div>
  <script>
    var gateway = `ws://${window.location.hostname}:81/`;
    var websocket;
    window.addEventListener('load', onLoad);

    function onLoad() { initWebSocket(); }
    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onopen = () => console.log("Connected");
      websocket.onclose = () => setTimeout(initWebSocket, 2000);
      websocket.onmessage = (event) => document.getElementById('status').innerHTML = "Status: " + event.data;
    }

    document.getElementById('set').onclick = () => {
      var hr = document.getElementById('hr').value;
      var min = document.getElementById('min').value;
      var sec = document.getElementById('sec').value;
      var blinks = document.getElementById('blinks').value;
      websocket.send(`set,${hr},${min},${sec},${blinks}`);
    };

    document.getElementById('stop').onclick = () => websocket.send('stop');
    document.getElementById('pause').onclick = () => websocket.send('pause');
    document.getElementById('reset').onclick = () => websocket.send('reset');
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Start WiFi AP
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.println(WiFi.softAPIP());

  // Serve the HTML UI
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  server.begin();

  // WebSocket Event Handler (lambda)
  webSocket.onEvent([&](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
      case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;

      case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        webSocket.sendTXT(num, "Connected to ESP32");
      } break;

      case WStype_TEXT: {
        String message = String((char*) payload);
        Serial.printf("[%u] Message: %s\n", num, payload);

        if (message.startsWith("set")) {
          int p1 = message.indexOf(',');
          int p2 = message.indexOf(',', p1+1);
          int p3 = message.indexOf(',', p2+1);
          int p4 = message.indexOf(',', p3+1);

          int hr = message.substring(p1+1, p2).toInt();
          int min = message.substring(p2+1, p3).toInt();
          int sec = message.substring(p3+1, p4).toInt();
          numberOfBlinks = message.substring(p4+1).toInt();

          clickDelay = ((hr * 3600UL) + (min * 60UL) + sec) * 1000UL; // convert to ms
          blinksDone = 0;
          isRunning = true;
          isPaused = false;
          webSocket.sendTXT(num, "Started");
        } 
        else if (message == "stop") {
          isRunning = false; isPaused = false;
          webSocket.sendTXT(num, "Stopped");
        } 
        else if (message == "pause") {
          isPaused = !isPaused;
          webSocket.sendTXT(num, isPaused ? "Paused" : "Resumed");
        } 
        else if (message == "reset") {
          isRunning = false; isPaused = false; blinksDone = 0;
          webSocket.sendTXT(num, "Reset");
        }
      } break;
    }
  });

  webSocket.begin();
}

void loop() {
  webSocket.loop();
  if (isRunning && !isPaused) {
    if (millis() - lastBlinkTime >= clickDelay) {
      if (blinksDone < numberOfBlinks) {
        digitalWrite(ledPin, HIGH);
        delay(200); // LED ON duration
        digitalWrite(ledPin, LOW);

        lastBlinkTime = millis();
        blinksDone++;

        String status = "Blinks: " + String(blinksDone) + "/" + String(numberOfBlinks);
        webSocket.broadcastTXT(status);
      } else {
        isRunning = false;
        webSocket.broadcastTXT("Finished All Blinks");
      }
    }
  }
}
