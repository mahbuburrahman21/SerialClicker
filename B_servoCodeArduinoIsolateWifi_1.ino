#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>

// Create an Access Point
const char* ssid = "RoboticArmControl";
const char* password = "12345678";

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

Servo myServo;
const int servoPin = 15; // GPIO 15 on ESP32

int clickDelay = 1000;
int numberOfClicks = 10;
bool isRunning = false;
bool isPaused = false;
int clicksDone = 0;
unsigned long lastClickTime = 0;

// The HTML and JavaScript for the GUI (no changes needed here)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Robotic Arm Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin: 0 auto; padding-top: 30px; background-color: #f2f2f2; }
    h1 { color: #333; }
    .container { display: flex; flex-direction: column; align-items: center; background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px 0 rgba(0,0,0,0.2); }
    .input-group { margin: 10px; width: 80%; }
    .input-group label { display: block; margin-bottom: 5px; color: #555; }
    .input-group input { width: 100%; padding: 8px; box-sizing: border-box; border: 1px solid #ccc; border-radius: 4px; }
    .button-group { margin-top: 20px; }
    button { border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block;
             font-size: 16px; margin: 4px 2px; cursor: pointer; border-radius: 8px; transition: background-color 0.3s; }
    #set { background-color: #4CAF50; } /* Green */
    #set:hover { background-color: #45a049; }
    #stop { background-color: #f44336; } /* Red */
    #stop:hover { background-color: #da190b; }
    #pause { background-color: #ff9800; } /* Orange */
    #pause:hover { background-color: #e68a00; }
    #reset { background-color: #555555; } /* Grey */
    #reset:hover { background-color: #333; }
    #status { margin-top: 20px; font-size: 1.2em; color: #333; font-weight: bold; }
  </style>
</head>
<body>
  <h1>ESP32 Robotic Arm Control</h1>
  <div class="container">
    <div class="input-group">
      <label for="delay">Set Delay (ms):</label>
      <input type="number" id="delay" value="1000">
    </div>
    <div class="input-group">
      <label for="clicks">Number of Clicks:</label>
      <input type="number" id="clicks" value="10">
    </div>
    <div class="button-group">
      <button id="set">Set and Start</button>
      <button id="stop">Stop</button>
      <button id="pause">Pause</button>
      <button id="reset">Reset</button>
    </div>
    <div id="status">Status: Idle</div>
  </div>
  <script>
    var gateway = `ws://${window.location.hostname}:81/`;
    var websocket;
    window.addEventListener('load', onLoad);

    function onLoad(event) {
      initWebSocket();
    }

    function initWebSocket() {
      console.log('Trying to open a WebSocket connection...');
      websocket = new WebSocket(gateway);
      websocket.onopen    = onOpen;
      websocket.onclose   = onClose;
      websocket.onmessage = onMessage;
    }

    function onOpen(event) {
      console.log('Connection opened');
      websocket.send("Hi from client");
    }

    function onClose(event) {
      console.log('Connection closed');
      setTimeout(initWebSocket, 2000);
    }

    function onMessage(event) {
      console.log(event.data);
      document.getElementById('status').innerHTML = "Status: " + event.data;
    }

    document.getElementById('set').addEventListener('click', () => {
      var delay = document.getElementById('delay').value;
      var clicks = document.getElementById('clicks').value;
      var message = `set,${delay},${clicks}`;
      websocket.send(message);
    });

    document.getElementById('stop').addEventListener('click', () => {
      websocket.send('stop');
    });

    document.getElementById('pause').addEventListener('click', () => {
      var pauseButton = document.getElementById('pause');
      websocket.send('pause');
      if (pauseButton.innerText === 'Pause') {
        pauseButton.innerText = 'Resume';
      } else {
        pauseButton.innerText = 'Pause';
      }
    });

    document.getElementById('reset').addEventListener('click', () => {
      document.getElementById('delay').value = 1000;
      document.getElementById('clicks').value = 10;
      document.getElementById('pause').innerText = 'Pause';
      websocket.send('reset');
    });
  </script>
</body>
</html>
)rawliteral";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      webSocket.sendTXT(num, "Connected to ESP32");
    }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*) payload);

      if (message.startsWith("set")) {
        int firstComma = message.indexOf(',');
        int secondComma = message.indexOf(',', firstComma + 1);
        clickDelay = message.substring(firstComma + 1, secondComma).toInt();
        numberOfClicks = message.substring(secondComma + 1).toInt();
        clicksDone = 0;
        isRunning = true;
        isPaused = false;
        webSocket.sendTXT(num, "Started");
      } else if (message == "stop") {
        isRunning = false;
        isPaused = false;
        webSocket.sendTXT(num, "Stopped");
      } else if (message == "pause") {
        isPaused = !isPaused;
        webSocket.sendTXT(num, isPaused ? "Paused" : "Resumed");
      } else if (message == "reset") {
        isRunning = false;
        isPaused = false;
        clicksDone = 0;
        webSocket.sendTXT(num, "Reset");
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  myServo.attach(servoPin);
  myServo.write(0); // Initial position

  // Create Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();

  if (isRunning && !isPaused) {
    if (millis() - lastClickTime >= clickDelay) {
      if (clicksDone < numberOfClicks) {
        // Perform the 'click' action
        myServo.write(90); // Click position
        delay(200); // Hold the click for a moment
        myServo.write(0); // Return to initial position
        
        lastClickTime = millis();
        clicksDone++;
        
        String status = "Clicks: " + String(clicksDone) + "/" + String(numberOfClicks);
        webSocket.broadcastTXT(status);
      } else {
        isRunning = false;
        webSocket.broadcastTXT("Finished All Clicks");
      }
    }
  }
}
