<div align="center">
<img src="https://i.imgur.com/g0gM4eT.png" alt="Project Banner" width="700"/>
<h1>ESP32 Serial Clicker</h1>
<p>
A web-controlled, precision servo-based automated clicker with a dynamic UI, smart WiFi provisioning, and mDNS support.
</p>
<p>
<a href="https://github.com/your-username/esp32-serial-clicker/blob/main/LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License"></a>
<a href="https://www.arduino.cc/"><img src="https://img.shields.io/badge/Framework-Arduino-00979D.svg" alt="Framework"></a>
<a href="https://www.espressif.com/en/products/socs/esp32"><img src="https://img.shields.io/badge/Platform-ESP32-E7352C.svg" alt="Platform"></a>
</p>
</div>
✨ Key Features
The ESP32 Serial Clicker isn't just a simple servo controller; it's a full-featured automation tool designed for ease of use and flexibility.
🌐 Modern Web Interface: Control everything from a sleek, responsive, and real-time web UI that works on any device with a browser.
🔮 Smart WiFi Provisioning: Thanks to WiFiManager, the device automatically creates a setup portal (AutoConnectAP) on its first boot. No more hardcoding WiFi credentials!
👆 On-Demand Configuration: Need to change WiFi networks? Just press the onboard BOOT button to launch a temporary configuration portal (OnDemandAP) anytime.
DNS Friendly .local Address: No need to hunt for IP addresses. Access the control panel from any device on the same network by simply navigating to http://serialclicker.local.
🦾 Complex Sequence Programming: Define sequences with multiple clicks, each with its own custom time gap (hours, minutes, seconds).
📈 Real-Time Status Dashboard: The UI provides live feedback on the current status (Idle, Running, Finished), click progress, and a countdown to the next action.
⚙️ Adjustable Servo Control: Fine-tune the servo's rotation angle directly from the web interface for precise physical interaction.
📸 Sneak Peek: The Web UI
<div align="center">
<img src="https://i.imgur.com/V7uB5F1.png" alt="Web UI Screenshot" width="800"/>
<p><em>The control panel gives you full command over your clicking sequences.</em></p>
</div>
🛠️ Hardware Requirements
1 x ESP32 Development Board
1 x SG90 Micro Servo (or similar)
Jumper Wires
A physical setup/mount for the servo to perform the click action.
📦 Software & Library Dependencies
This project is built using the Arduino Framework. Before uploading the code, ensure you have installed the following libraries through your IDE's Library Manager:
ESPAsyncWebServer
AsyncTCP
WebSockets by Markus Sattler
WiFiManager by tzapu
ArduinoJson
ESP32Servo
🚀 Getting Started: Installation & Setup
Follow these steps to get your Serial Clicker up and running.
1. Clone the Repository
code
Bash
git clone https://github.com/your-username/esp32-serial-clicker.git
cd esp32-serial-clicker
2. Configure Your Environment
Open the .ino file in your Arduino IDE or PlatformIO.
Install all the libraries listed in the dependencies section above.
3. Upload the Code
Connect your ESP32 board to your computer.
Select the correct board and COM port in your IDE.
Hit the Upload button.
🕹️ How to Use
First-Time WiFi Setup (Auto-Portal)
Power on the ESP32. The onboard LED will blink, indicating it's not connected to WiFi.
On your computer or smartphone, scan for WiFi networks.
Connect to the access point named AutoConnectAP. The password is password.
A captive portal should automatically open in your browser. If not, open a browser and go to 192.168.4.1.
Select your home WiFi network (SSID), enter its password, and click Save.
The ESP32 will connect to your network and restart. The onboard LED will turn solid, indicating a successful connection.
Normal Operation
Once connected to your network, open any web browser on a device connected to the same network.
Navigate to http://serialclicker.local.
Use the web interface to configure your desired click sequences and start the process!
Changing WiFi Credentials (On-Demand Portal)
While the device is running, press and hold the BOOT button on the ESP32 board for a second.
The device will launch a new, temporary access point named OnDemandAP (password: password).
Follow the same steps as the "First-Time Setup" to connect to a new network. The device will restart and connect to the newly configured network.
🤝 Contributing
Contributions, issues, and feature requests are welcome! Feel free to check the issues page.
📄 License
This project is licensed under the MIT License. See the LICENSE file for details.
<div align="center">
Made with ❤️ and a lot of clicks.
</div>
