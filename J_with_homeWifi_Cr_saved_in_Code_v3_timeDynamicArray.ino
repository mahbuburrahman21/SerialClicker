//=============================================================================Perfect working one==============================================================================

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

const int ledPin = 2; // Onboard LED

// --- WIFI CREDENTIALS ---
const char* my_ssid = "Tp-Link 5.0";
const char* my_password = "Mahbub2u5t";

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
Servo myServo;

const int servoPin = 13;

// --- STATE VARIABLES ---
bool isRunning = false;
int currentClick = 0;
int totalClicks = 0;
int rotationAngle = 90;
unsigned long nextClickTime = 0;

// *******************************************************************
// ***** BUG FIX: Make the JSON document global so data persists *****
// *******************************************************************
StaticJsonDocument<1024> jsonDoc; // Increased size for more time gaps
JsonArray timeGaps;


// --- HTML PAGE ---
// --- (PASTE THIS CODE INTO YOUR .INO FILE) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Keyboard Cliker</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;700&display=swap');
    :root {
      --bg-color: #1a1a2e;
      --card-color: #16213e;
      --primary-color: #0f3460;
      --accent-color: #e94560;
      --text-color: #dcdcdc;
      --label-color: #a0a0a0;
      --border-color: #0f3460;
      --success-color: #16c79a;
      --danger-color: #e94560;
    }
    body {
      font-family: 'Roboto', sans-serif;
      background-color: var(--bg-color);
      color: var(--text-color);
      margin: 0;
      padding: 20px;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .container {
      width: 95%;
      max-width: 700px;
      background: var(--card-color);
      padding: 35px;
      border-radius: 20px;
      box-shadow: 0 15px 40px rgba(0,0,0,0.4);
      border: 1px solid var(--border-color);
    }
    h1 {
      font-size: 2.2rem;
      color: white;
      margin-bottom: 30px;
      text-align: center;
      font-weight: 700;
    }
    .settings-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
      margin-bottom: 30px;
    }
    .input-group {
      text-align: left;
    }
    label {
      display: block;
      margin-bottom: 10px;
      color: var(--label-color);
      font-weight: 400;
      font-size: 0.9rem;
    }
    input[type="number"], input[type="range"] {
      width: 100%;
      padding: 12px;
      background: var(--primary-color);
      border: 1px solid var(--border-color);
      border-radius: 8px;
      box-sizing: border-box;
      font-size: 1rem;
      color: var(--text-color);
      -moz-appearance: textfield;
    }
    input[type="number"]::-webkit-outer-spin-button,
    input[type="number"]::-webkit-inner-spin-button {
        -webkit-appearance: none;
        margin: 0;
    }
    .button-group {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
      margin-top: 30px;
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
      background-size: 200% auto;
    }
    button:hover {
        transform: translateY(-3px);
        box-shadow: 0 8px 20px rgba(0,0,0,0.3);
    }
    #start { background-image: linear-gradient(to right, #16c79a 0%, #28a745 51%, #16c79a 100%); }
    #stop { background-image: linear-gradient(to right, #e94560 0%, #dc3545 51%, #e94560 100%); }
    #time-inputs-container {
        margin-top: 30px;
        border-top: 1px solid var(--border-color);
        padding-top: 20px;
        max-height: 30vh;
        overflow-y: auto;
    }
    .time-input-row {
        display: grid;
        grid-template-columns: 1fr 1fr 1fr;
        gap: 15px;
        align-items: center;
        margin-bottom: 20px;
        background: rgba(15, 52, 96, 0.5);
        padding: 15px;
        border-radius: 10px;
    }
    .time-input-row .row-label {
        grid-column: 1 / -1;
        font-weight: bold;
        margin-bottom: 5px;
        color: white;
    }
    #status {
      margin-top: 30px;
      font-weight: bold;
      font-size: 1.1rem;
      padding: 15px;
      background-color: var(--primary-color);
      border-radius: 8px;
      color: var(--text-color);
      border: 1px solid var(--border-color);
      text-align: center;
    }
    #angleValue {
      font-weight: bold;
      color: var(--accent-color);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Serial Clicker</h1>
    <div class="settings-grid">
      <div class="input-group">
        <label for="num-clicks">Number of Clicks</label>
        <input type="number" id="num-clicks" value="1" min="1">
      </div>
      <div class="input-group">
        <label for="angle">Rotation Angle: <span id="angleValue">90</span>&deg;</label>
        <input type="range" id="angle" min="0" max="180" value="90" oninput="document.getElementById('angleValue').innerText = this.value;">
      </div>
    </div>
    
    <div id="time-inputs-container"></div>
    
    <div class="button-group">
      <button id="start">Start Sequence</button>
      <button id="stop">Stop Immediately</button>
    </div>
    <div id="status">Status: Idle</div>
  </div>
  <script>
    const gateway = `ws://${window.location.hostname}:81/`;
    let websocket;
    
    window.addEventListener('load', () => {
        initWebSocket();
        generateTimeInputs(); 
        document.getElementById('num-clicks').addEventListener('input', generateTimeInputs);
        document.getElementById('start').addEventListener('click', sendData);
        document.getElementById('stop').addEventListener('click', () => websocket.send(JSON.stringify({command: "stop"})));
    });

    function initWebSocket() {
        websocket = new WebSocket(gateway);
        websocket.onopen = () => console.log("WebSocket connected");
        websocket.onclose = () => setTimeout(initWebSocket, 2000);
        websocket.onmessage = (event) => {
            document.getElementById('status').innerHTML = "Status: " + event.data;
        };
    }

    function generateTimeInputs() {
        const numClicks = parseInt(document.getElementById('num-clicks').value, 10) || 1;
        const container = document.getElementById('time-inputs-container');
        container.innerHTML = ''; 

        for (let i = 1; i <= numClicks; i++) {
            const row = document.createElement('div');
            row.className = 'time-input-row';
            row.setAttribute('data-index', i);
            
            let labelText = `Time Gap Before Click #${i}`;
            if (i === 1) {
                labelText = `Initial Delay Before First Click`;
            }

            row.innerHTML = `
                <div class="row-label">${labelText}</div>
                <div class="input-group">
                    <label for="hr${i}">Hours</label>
                    <input type="number" id="hr${i}" min="0" value="0">
                </div>
                <div class="input-group">
                    <label for="min${i}">Minutes</label>
                    <input type="number" id="min${i}" min="0" value="0">
                </div>
                <div class="input-group">
                    <label for="sec${i}">Seconds</label>
                    <input type="number" id="sec${i}" min="0" value="1">
                </div>
            `;
            container.appendChild(row);
        }
    }

    function sendData() {
        const timeRows = document.querySelectorAll('.time-input-row');
        const timeGapsInSeconds = [];
        
        timeRows.forEach(row => {
            const index = row.getAttribute('data-index');
            const hr = parseInt(document.getElementById(`hr${index}`).value, 10) || 0;
            const min = parseInt(document.getElementById(`min${index}`).value, 10) || 0;
            const sec = parseInt(document.getElementById(`sec${index}`).value, 10) || 0;
            const totalSeconds = (hr * 3600) + (min * 60) + sec;
            timeGapsInSeconds.push(totalSeconds);
        });

        const data = {
            command: "start",
            angle: parseInt(document.getElementById('angle').value, 10),
            timeGaps: timeGapsInSeconds
        };

        websocket.send(JSON.stringify(data));
    }
  </script>
</body>
</html>
)rawliteral";

// const char index_html[] PROGMEM = R"rawliteral(...)";


// --- FORWARD DECLARATION ---
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    myServo.attach(servoPin);
    myServo.write(0);

    WiFi.begin(my_ssid, my_password);
    Serial.print("Connecting to Wi-Fi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        Serial.print(".");
        delay(500);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(ledPin, HIGH);
        Serial.println("\nConnected!");
        Serial.print("IP Address: http://");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to Wi-Fi.");
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });
    server.begin();

    webSocket.onEvent(webSocketEvent);
    webSocket.begin();
}

void loop() {
    webSocket.loop();
    
    // Blink LED if not connected to WiFi
    if (WiFi.status() != WL_CONNECTED) {
        digitalWrite(ledPin, !digitalRead(ledPin));
        delay(250);
    } else {
        digitalWrite(ledPin, HIGH);
    }

    if (isRunning) {
        if (millis() >= nextClickTime) {
            if (currentClick < totalClicks) {
                // Perform the click
                myServo.write(rotationAngle);
                delay(200); // Short delay to complete the movement
                myServo.write(0);
                
                String status = "Click " + String(currentClick + 1) + "/" + String(totalClicks) + " executed.";
                webSocket.broadcastTXT(status);
                Serial.println(status);

                currentClick++;

                // If there are more clicks, set the timer for the next one
                if (currentClick < totalClicks) {
                    unsigned long delaySeconds = timeGaps[currentClick].as<unsigned long>();
                    nextClickTime = millis() + delaySeconds * 1000UL;
                    
                    String nextStatus = "Next click in " + String(delaySeconds) + " seconds.";
                    webSocket.broadcastTXT(nextStatus);
                    Serial.println(nextStatus);
                } else {
                    // All clicks are done
                    isRunning = false;
                    webSocket.broadcastTXT("Sequence Finished.");
                    Serial.println("Sequence Finished.");
                }
            }
        }
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
            webSocket.sendTXT(num, "ESP32 Ready");
            break;
        }
        case WStype_TEXT: {
            Serial.printf("[%u] Message: %s\n", num, payload);
            
            // Use the global jsonDoc to deserialize
            DeserializationError error = deserializeJson(jsonDoc, payload);

            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }

            const char* command = jsonDoc["command"];

            if (strcmp(command, "start") == 0) {
                rotationAngle = jsonDoc["angle"];
                timeGaps = jsonDoc["timeGaps"].as<JsonArray>();
                totalClicks = timeGaps.size();
                currentClick = 0;

                if (totalClicks > 0) {
                    isRunning = true;
                    unsigned long initialDelay = timeGaps[0].as<unsigned long>();
                    nextClickTime = millis() + initialDelay * 1000UL;
                    
                    String status = "Sequence started. First click in " + String(initialDelay) + " seconds.";
                    webSocket.broadcastTXT(status);
                    Serial.println(status);
                }

            } else if (strcmp(command, "stop") == 0) {
                isRunning = false;
                myServo.write(0);
                webSocket.broadcastTXT("Stopped by user.");
                Serial.println("Stopped by user.");
            }
            break;
        }
    }
}
