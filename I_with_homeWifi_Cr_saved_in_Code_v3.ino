#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// ----------- ADD THIS LINE -----------
const int ledPin = 2; // Onboard LED for most ESP32 boards is on GPIO2
// ------------------------------------

// ----------- CHANGE THESE LINES -----------
const char* my_ssid = "Tp-Link 5.0";       // Your home Wi-Fi network name
const char* my_password = "Mahbub2u5t"; // Your home Wi-Fi password
//GUI IP: http://192.168.0.164/ 
// ------------------------------------------

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
Servo myServo;

const int servoPin = 13; // Servo on ESP32 (GPIO13)

unsigned long clickDelay = 1000;
int numberOfMovements = 10;
int numberOfCycles = 1;
int rotationAngle = 90; // Default rotation angle
bool isRunning = false;
bool isPaused = false;
int movementsDone = 0;
int cyclesDone = 0;
unsigned long lastMoveTime = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 Servo Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root {
      --primary-color: #007bff;
      --secondary-color: #6c757d;
      --success-color: #28a745;
      --danger-color: #dc3545;
      --warning-color: #ffc107;
      --light-color: #f8f9fa;
      --dark-color: #343a40;
    }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
      text-align: center;
      background-color: #e9ecef;
      margin: 0;
      padding: 20px;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .container {
      margin: auto;
      width: 95%;
      max-width: 500px;
      background: white;
      padding: 30px;
      border-radius: 15px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.1);
    }
    h1 {
      font-size: 2rem;
      color: var(--dark-color);
      margin-bottom: 25px;
    }
    .input-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
      gap: 15px;
      margin-bottom: 20px;
    }
    .input-group {
      text-align: left;
    }
    label {
      display: block;
      margin-bottom: 8px;
      color: var(--secondary-color);
      font-weight: bold;
    }
    input[type="number"], input[type="range"] {
      width: 100%;
      padding: 12px;
      border: 1px solid #ced4da;
      border-radius: 8px;
      box-sizing: border-box;
      font-size: 1rem;
    }
    .button-group {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 15px;
      margin-top: 25px;
    }
    button {
      border: none;
      color: white;
      padding: 15px;
      border-radius: 8px;
      cursor: pointer;
      font-size: 1rem;
      font-weight: bold;
      transition: all 0.3s ease;
    }
    button:hover {
        transform: translateY(-2px);
        box-shadow: 0 4px 15px rgba(0,0,0,0.15);
    }
    #set { background: var(--success-color); }
    #stop { background: var(--danger-color); }
    #pause { background: var(--warning-color); color: var(--dark-color); }
    #reset { background: var(--secondary-color); }
    #status {
      margin-top: 30px;
      font-weight: bold;
      font-size: 1.1rem;
      padding: 15px;
      background-color: var(--light-color);
      border-radius: 8px;
      color: var(--dark-color);
      border: 1px solid #dee2e6;
    }
    .slider-group {
        grid-column: 1 / -1; /* Span full width in grid */
    }
    #angleValue {
        font-weight: bold;
        color: var(--primary-color);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 Servo Control</h1>
    <div class="input-grid">
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
        <label for="movements">Movements:</label>
        <input type="number" id="movements" value="10" min="1">
      </div>
      <div class="input-group">
        <label for="cycles">Cycles:</label>
        <input type="number" id="cycles" value="1" min="1">
      </div>
      <div class="input-group slider-group">
        <label for="angle">Rotation Angle: <span id="angleValue">90</span>&deg;</label>
        <input type="range" id="angle" min="0" max="180" value="90" oninput="document.getElementById('angleValue').innerText = this.value;">
      </div>
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
      var movements = document.getElementById('movements').value;
      var cycles = document.getElementById('cycles').value;
      var angle = document.getElementById('angle').value;
      websocket.send(`set,${hr},${min},${sec},${movements},${cycles},${angle}`);
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

  // --- MODIFICATION START ---
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Turn LED off initially
  // --- MODIFICATION END ---

  myServo.attach(servoPin);
  myServo.write(0);

  WiFi.begin(my_ssid, my_password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // --- MODIFICATION START ---
  digitalWrite(ledPin, HIGH); // Turn LED on when connected
  // --- MODIFICATION END ---
  
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  server.begin();

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
          int p6 = message.indexOf(',', p5 + 1);

          int hr = message.substring(p1 + 1, p2).toInt();
          int min = message.substring(p2 + 1, p3).toInt();
          int sec = message.substring(p3 + 1, p4).toInt();
          numberOfMovements = message.substring(p4 + 1, p5).toInt();
          numberOfCycles = message.substring(p5 + 1, p6).toInt();
          rotationAngle = message.substring(p6 + 1).toInt();


          clickDelay = ((hr * 3600UL) + (min * 60UL) + sec) * 1000UL;
          movementsDone = 0;
          cyclesDone = 0;
          isRunning = true;
          isPaused = false;
          webSocket.sendTXT(num, "Started");
        } 
        else if (message == "stop") {
          isRunning = false;
          isPaused = false;
          myServo.write(0);
          webSocket.sendTXT(num, "Stopped");
        } 
        else if (message == "pause") {
          isPaused = !isPaused;
          webSocket.sendTXT(num, isPaused ? "Paused" : "Resumed");
        } 
        else if (message == "reset") {
          isRunning = false;
          isPaused = false;
          movementsDone = 0;
          cyclesDone = 0;
          myServo.write(0);
          webSocket.sendTXT(num, "Reset");
        }
      } break;
    }
  });

  webSocket.begin();
}

void loop() {
  // --- MODIFICATION START ---
  // Continuously check Wi-Fi status and update LED
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(ledPin, HIGH); // If connected, LED is ON
  } else {
    digitalWrite(ledPin, LOW);  // If not connected, LED is OFF
  }
  // --- MODIFICATION END ---
  
  webSocket.loop();
  if (isRunning && !isPaused) {
    if (millis() - lastMoveTime >= clickDelay) {
      if (cyclesDone < numberOfCycles) {
        if (movementsDone < numberOfMovements) {
          myServo.write(rotationAngle);
          delay(500);
          myServo.write(0);

          lastMoveTime = millis();
          movementsDone++;

          String status = "Cycle: " + String(cyclesDone + 1) + "/" + String(numberOfCycles) + " - Movements: " + String(movementsDone) + "/" + String(numberOfMovements);
          webSocket.broadcastTXT(status);
        } else {
          movementsDone = 0;
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
