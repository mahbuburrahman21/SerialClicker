//=============================================================================
//================== Final Corrected Code (No MDNS.update) ==================
//=============================================================================

// No need to have any IP address. Just Connect your device with the same network. Then Open any web browser. 
// And Search ================(serialclicker.local)====================

// --- REQUIRED LIBRARIES ---
// Make sure you have these installed from the Arduino Library Manager:
// 1. WebSockets by Markus Sattler
// 2. ESPAsyncWebServer by me-no-dev
// 3. AsyncTCP by me-no-dev

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h> // This library contains the definition for WStype_t
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <ESPmDNS.h> // Include the mDNS library

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
unsigned long remainingTimeAtUpdate = 0; 

StaticJsonDocument<1024> jsonDoc;
JsonArray timeGaps;


// --- HTML PAGE (with new UI elements and JavaScript logic) ---
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
    #status-container {
      margin-top: 30px;
      padding: 20px;
      background-color: var(--primary-color);
      border-radius: 8px;
      border: 1px solid var(--border-color);
      text-align: center;
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 15px;
    }
    .status-item {
        background-color: var(--card-color);
        padding: 15px;
        border-radius: 8px;
    }
    .status-item h3 {
        margin: 0 0 8px 0;
        font-size: 0.9rem;
        color: var(--label-color);
        font-weight: 400;
    }
    .status-item p {
        margin: 0;
        font-size: 1.4rem;
        font-weight: 700;
        color: white;
    }
    #status-text { color: var(--success-color); }
    #countdown-timer { color: var(--accent-color); }
    .full-width {
        grid-column: 1 / -1;
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

    <div id="status-container">
      <div class="status-item full-width">
        <h3>STATUS</h3>
        <p id="status-text">Idle</p>
      </div>
      <div class="status-item">
        <h3>NEXT CLICK</h3>
        <p id="next-click-info">N/A</p>
      </div>
      <div class="status-item">
        <h3>TIME REMAINING</h3>
        <p id="countdown-timer">--:--:--</p>
      </div>
    </div>

  </div>
  <script>
    const gateway = `ws://${window.location.hostname}:81/`;
    let websocket;
    let countdownInterval;

    window.addEventListener('load', () => {
        initWebSocket();
        generateTimeInputs(); 
        document.getElementById('num-clicks').addEventListener('input', generateTimeInputs);
        document.getElementById('start').addEventListener('click', sendData);
        document.getElementById('stop').addEventListener('click', () => {
            websocket.send(JSON.stringify({command: "stop"}));
            resetStatusUI();
        });
    });

    function initWebSocket() {
        websocket = new WebSocket(gateway);
        websocket.onopen = () => console.log("WebSocket connected");
        websocket.onclose = () => setTimeout(initWebSocket, 2000);
        websocket.onmessage = (event) => {
            let data;
            try {
                data = JSON.parse(event.data);
            } catch (e) {
                console.log("Received non-JSON message: ", event.data);
                if (typeof event.data === 'string') {
                    document.getElementById('status-text').textContent = event.data;
                }
                return;
            }
            
            updateStatusUI(data);
        };
    }

    function resetStatusUI() {
        clearInterval(countdownInterval);
        document.getElementById('status-text').textContent = 'Idle';
        document.getElementById('next-click-info').textContent = 'N/A';
        document.getElementById('countdown-timer').textContent = '--:--:--';
    }

    function updateStatusUI(data) {
        const statusTextEl = document.getElementById('status-text');
        const nextClickInfoEl = document.getElementById('next-click-info');
        
        if (!data.status) return;

        statusTextEl.textContent = data.status;

        if (data.status === "Running") {
            nextClickInfoEl.textContent = `${data.currentClick} / ${data.totalClicks}`;
            startCountdown(data.countdown);
        } else if (data.status === "Finished" || data.status.includes("Stopped")) {
            clearInterval(countdownInterval);
            nextClickInfoEl.textContent = "N/A";
            document.getElementById('countdown-timer').textContent = "00:00:00";
        } else if (data.status === "Connected") {
            resetStatusUI();
            statusTextEl.textContent = 'Connected & Idle';
        }
    }

    function startCountdown(durationInSeconds) {
        clearInterval(countdownInterval);
        let remainingTime = durationInSeconds;

        const updateTimer = () => {
            if (remainingTime < 0) {
                clearInterval(countdownInterval);
                document.getElementById('countdown-timer').textContent = "00:00:00";
                return;
            }

            const hours = Math.floor(remainingTime / 3600);
            const minutes = Math.floor((remainingTime % 3600) / 60);
            const seconds = remainingTime % 60;

            document.getElementById('countdown-timer').textContent = 
                `${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
            
            remainingTime--;
        };

        updateTimer(); 
        countdownInterval = setInterval(updateTimer, 1000);
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


// Forward Declarations
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void broadcastStatus(); 

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

        // --- Start mDNS ---
        if (!MDNS.begin("serialclicker")) { // Set the hostname for .local address
            Serial.println("Error setting up MDNS responder!");
        } else {
            Serial.println("mDNS responder started. Access at http://serialclicker.local");
            MDNS.addService("http", "tcp", 80);
        }
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
    // The line MDNS.update() has been removed as it is not needed for ESP32.
    
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
                delay(200);
                myServo.write(0);
                
                Serial.printf("Click %d/%d executed.\n", currentClick + 1, totalClicks);
                currentClick++;

                if (currentClick < totalClicks) {
                    unsigned long delaySeconds = timeGaps[currentClick].as<unsigned long>();
                    nextClickTime = millis() + delaySeconds * 1000UL;
                    broadcastStatus();
                } else {
                    // This was the last click
                    isRunning = false;
                    broadcastStatus();
                }
            }
        }
    }
}

void broadcastStatus() {
    StaticJsonDocument<200> statusDoc;
    String jsonString;

    if (isRunning) {
        statusDoc["status"] = "Running";
        statusDoc["currentClick"] = currentClick + 1;
        statusDoc["totalClicks"] = totalClicks;
        // Calculate remaining time accurately for the countdown
        unsigned long timeNow = millis();
        if (nextClickTime > timeNow) {
            statusDoc["countdown"] = (nextClickTime - timeNow) / 1000;
        } else {
            statusDoc["countdown"] = 0;
        }
    } else {
        // Check if the sequence finished naturally vs being stopped
        if (currentClick >= totalClicks && totalClicks > 0) {
            statusDoc["status"] = "Finished";
        } else {
             statusDoc["status"] = "Stopped by user";
        }
        statusDoc["currentClick"] = 0;
        statusDoc["totalClicks"] = 0;
        statusDoc["countdown"] = 0;
        // Reset state for the next run
        totalClicks = 0; 
        currentClick = 0;
    }

    serializeJson(statusDoc, jsonString);
    webSocket.broadcastTXT(jsonString);
    Serial.println("Sent status: " + jsonString);
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
            webSocket.sendTXT(num, "{\"status\":\"Connected\"}");
            break;
        }
        case WStype_TEXT: {
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
                    broadcastStatus();
                }

            } else if (strcmp(command, "stop") == 0) {
                isRunning = false;
                myServo.write(0);
                broadcastStatus();
            }
            break;
        }
        // Safely handle other WebSocket event types
        case WStype_BIN:
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }
}
