#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// Create Access Point
const char* ssid = "LEDBlinkControl";
const char* password = "12345678";

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

//const int ledPin = 2; // Built-in LED on ESP32 (GPIO2)
const int ledPin = 13; // outside LED on ESP32 (GPIO13)

unsigned long clickDelay = 1000; // Blink delay (ms)
int numberOfBlinks = 10;
int numberOfCycles = 1;
bool isRunning = false;
bool isPaused = false;
int blinksDone = 0;
int cyclesDone = 0;
unsigned long lastBlinkTime = 0;

// HTML Page with improved UI and loop feature
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 LED Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      text-align: center;
      background-color: #f0f2f5;
      margin: 0;
      padding: 20px;
    }
    .container {
      margin: auto;
      width: 90%;
      max-width: 400px;
      background: white;
      padding: 25px;
      border-radius: 15px;
      box-shadow: 0 10px 25px rgba(0,0,0,0.1);
    }
    h1 {
      font-size: 24px;
      color: #333;
      margin-bottom: 20px;
    }
    .input-group {
      margin-bottom: 15px;
      text-align: left;
    }
    label {
      display: block;
      margin-bottom: 5px;
      color: #555;
      font-weight: bold;
    }
    input[type="number"] {
      width: 100%;
      padding: 10px;
      border: 1px solid #ddd;
      border-radius: 8px;
      box-sizing: border-box;
    }
    .button-group {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      margin-top: 20px;
    }
    button {
      border: none;
      color: white;
      padding: 12px;
      border-radius: 8px;
      cursor: pointer;
      font-size: 16px;
      transition: background-color 0.3s;
    }
    #set { background: #28a745; } #set:hover { background: #218838; }
    #stop { background: #dc3545; } #stop:hover { background: #c82333; }
    #pause { background: #ffc107; color: #333; } #pause:hover { background: #e0a800; }
    #reset { background: #6c757d; } #reset:hover { background: #5a6268; }
    #status {
      margin-top: 25px;
      font-weight: bold;
      font-size: 18px;
      padding: 10px;
      background-color: #e9ecef;
      border-radius: 8px;
      color: #495057;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 LED Control</h1>
    <div class="input-group">
      <label for="hr">Hours:</label>
      <input type="number" id="hr" value="0" min="0">
    </div>
    <div class="input-group">
      <label for="min">Minutes:</label>
      <input type="number" id="min" value="0" min="0">
    </div>
    <div class="input-group">
      <label for="sec">Seconds:</label>
      <input type="number" id="sec" value="1" min="1">
    </div>
    <div class="input-group">
      <label for="blinks">Number of Blinks:</label>
      <input type="number" id="blinks" value="10" min="1">
    </div>
    <div class="input-group">
      <label for="cycles">Number of Cycles:</label>
      <input type="number" id="cycles" value="1" min="1">
    </div>
    <div class="button-group">
      <button id="set">Set & Start</button>
      <button id="stop">Stop</button>
      <button id="pause">Pause/Resume</button>
      <button id="reset">Reset</button>
    </div>
    <div id="status">Status: Idle</div>
  </div>
  <script>
    var gateway = `ws://${window.location.hostname}:81/`;
    var websocket;
    window.addEventListener('load', onLoad);

    function onLoad() {
      initWebSocket();
    }

    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onopen = () => console.log("Connected");
      websocket.onclose = () => setTimeout(initWebSocket, 2000);
      websocket.onmessage = (event) => {
        document.getElementById('status').innerHTML = "Status: " + event.data;
      };
    }

    document.getElementById('set').onclick = () => {
      var hr = document.getElementById('hr').value;
      var min = document.getElementById('min').value;
      var sec = document.getElementById('sec').value;
      var blinks = document.getElementById('blinks').value;
      var cycles = document.getElementById('cycles').value;
      websocket.send(`set,${hr},${min},${sec},${blinks},${cycles}`);
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

  // WebSocket Event Handler
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
          int p2 = message.indexOf(',', p1 + 1);
          int p3 = message.indexOf(',', p2 + 1);
          int p4 = message.indexOf(',', p3 + 1);
          int p5 = message.indexOf(',', p4 + 1);

          int hr = message.substring(p1 + 1, p2).toInt();
          int min = message.substring(p2 + 1, p3).toInt();
          int sec = message.substring(p3 + 1, p4).toInt();
          numberOfBlinks = message.substring(p4 + 1, p5).toInt();
          numberOfCycles = message.substring(p5 + 1).toInt();

          clickDelay = ((hr * 3600UL) + (min * 60UL) + sec) * 1000UL;
          blinksDone = 0;
          cyclesDone = 0;
          isRunning = true;
          isPaused = false;
          webSocket.sendTXT(num, "Started");
        } 
        else if (message == "stop") {
          isRunning = false;
          isPaused = false;
          digitalWrite(ledPin, LOW);
          webSocket.sendTXT(num, "Stopped");
        } 
        else if (message == "pause") {
          isPaused = !isPaused;
          webSocket.sendTXT(num, isPaused ? "Paused" : "Resumed");
        } 
        else if (message == "reset") {
          isRunning = false;
          isPaused = false;
          blinksDone = 0;
          cyclesDone = 0;
          digitalWrite(ledPin, LOW);
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
      if (cyclesDone < numberOfCycles) {
        if (blinksDone < numberOfBlinks) {
          digitalWrite(ledPin, HIGH);
          delay(200); // LED ON duration
          digitalWrite(ledPin, LOW);

          lastBlinkTime = millis();
          blinksDone++;

          String status = "Cycle: " + String(cyclesDone + 1) + "/" + String(numberOfCycles) + " - Blinks: " + String(blinksDone) + "/" + String(numberOfBlinks);
          webSocket.broadcastTXT(status);
        } else {
          blinksDone = 0;
          cyclesDone++;
          if (cyclesDone >= numberOfCycles) {
            isRunning = false;
            webSocket.broadcastTXT("Finished All Cycles");
          }
        }
      }
    }
  }
}
