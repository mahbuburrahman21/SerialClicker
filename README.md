<div align="center">
  <img src="https://i.imgur.com/8x2gYq2.png" alt="Project Banner" width="800"/>
  <h1>Serial Clicker</h1>
  <p>A Web-Controlled Servo Sequencer for Precision Automation</p>

  <a href="https://github.com/arduino/arduino-cli">
    <img src="https://img.shields.io/badge/Framework-Arduino-00979D?style=for-the-badge&logo=arduino" alt="Framework">
  </a>
  <a href="https://www.espressif.com/en/products/socs/esp32">
    <img src="https://img.shields.io/badge/Platform-ESP32-E7352C?style=for-the-badge&logo=espressif" alt="Platform">
  </a>
   <a href="#">
    <img src="https://img.shields.io/badge/License-MIT-blue?style=for-the-badge" alt="License">
  </a>

---

![Typing SVG](https://readme-typing-svg.herokuapp.com?font=Fira+Code&size=22&pause=1000&color=36BCF7&center=true&vCenter=true&width=800&lines=Automate+physical+actions+with+precision.;Control+from+any+device+via+a+modern+web+UI.;Easy+setup+with+an+on-demand+WiFi+portal.)

</div>

---

## 📖 Overview

The ESP32 Serial Clicker is a powerful and versatile tool for automating physical interactions. At its core, it's a servo motor controlled by an ESP32, but its power lies in the sophisticated web interface and robust backend. You can define complex sequences of "clicks" with precise, user-defined time gaps—from seconds to hours.

Whether you need to automate keyboard presses for AFK tasks, conduct long-term product testing, or create timed mechanical interactions for a project, this solution offers a reliable, network-controlled alternative to manual intervention. The intuitive UI and easy WiFi setup mean you can get your automation sequence running in minutes from any device on your network.

---

## ✨ Key Features

-   **📱 Modern & Responsive Web UI**: A sleek, dark-themed interface built to work flawlessly on your desktop, tablet, or smartphone.
-   **🌐 Effortless WiFi Configuration**: Features **WiFiManager** for a pain-free setup. No hardcoded credentials—simply connect to the ESP32's access point on first boot to configure your network.
-   **⏱️ Advanced Sequence Programming**:
    -   Define the total number of clicks.
    -   Set unique time gaps (in hours, minutes, and seconds) before each individual click.
    -   Specify an initial delay before the first click.
-   **⚙️ Real-Time Servo Control**: Calibrate the servo's rotation angle directly from the UI to perfectly match your physical setup.
-   **📈 Live Status Dashboard**: Get instant feedback via WebSockets. The UI provides:
    -   **Current Status**: `Idle`, `Running`, `Finished`, or `Stopped`.
    -   **Click Progress**: A live count of executed clicks (e.g., `5 / 100`).
    -   **Countdown Timer**: A precise countdown to the next scheduled click.
-   **👆 On-Demand Configuration Portal**: Press a physical button at any time to re-launch the WiFi configuration portal without needing to re-flash the firmware.
-   **🔗 Zero-Configuration Networking**: Uses **mDNS**, allowing you to access the web interface via a friendly address like `http://serialclicker.local` instead of hunting for an IP.

---

## 📸 Web Interface Preview

<div align="center">
  <img src="Screenshot 2025-09-30 205354.png" width="750"/>
  <p><em>Control everything from one clean interface.</em></p>
</div>

---

## 🛠️ Hardware Requirements

| Component                  | Quantity | Notes                                          |
| -------------------------- | :------: | ---------------------------------------------- |
| ESP32 Development Board    |    1     | Any model with WiFi will work.                 |
| Servo Motor                |    1     | e.g., SG90 Micro Servo.                        |
| Momentary Push Button      |    1     | For triggering the on-demand WiFi portal.      |
| Breadboard                 |    1     | For prototyping.                               |
| Jumper Wires               |   Set    | To connect the components.                     |
| 5V Power Supply (Optional) |    1     | Recommended for stable servo operation.        |

---

## 🔌 Wiring Diagram

Connect the components as follows. Ensure your ESP32 is **not** connected to USB when wiring to avoid shorts.

| ESP32 Pin | Component       | Connection Point  |
| :-------- | :-------------- | :---------------- |
| **`GPIO 13`** | Servo Motor     | `Signal Pin`      |
| **`GPIO 0`**  | Push Button     | `One Leg`         |
| **`GPIO 2`**  | Onboard LED     | _(Internal)_      |
| **`5V / VIN`**| Servo Motor     | `VCC (Red Wire)`  |
| **`GND`**     | Servo Motor     | `GND (Brown Wire)`|
| **`GND`**     | Push Button     | `Other Leg`       |

*Note: For best performance, power the servo from a separate 5V power supply, making sure to connect its ground to the ESP32's ground.*

---

## 🚀 Getting Started

### 1. Software & Libraries

Ensure you have the latest version of the **Arduino IDE** installed with the **ESP32 Board Manager**. The following libraries must be installed via the Arduino Library Manager (`Sketch > Include Library > Manage Libraries...`):

-   `WiFi`
-   `WiFiManager` by tzapu
-   `ESPAsyncWebServer`
-   `WebSockets` by Markus Sattler
-   `ArduinoJson` by Benoit Blanchon
-   `ESP32Servo`
-   `ESPmDNS`

### 2. Flashing the Firmware

1.  **Clone the Repository**: Download the `ino` file from this project.
2.  **Open in Arduino IDE**: Open the main project file.
3.  **Configure Pins**: If you wired your components to different pins, update the `#define` section at the top of the code.
4.  **Upload**: Select your ESP32 board and COM port, then click the "Upload" button.

### 3. First-Time WiFi Setup

1.  **Power On**: Power the ESP32 via USB or an external supply.
2.  **Connect to the AP**: On your phone or computer, search for a new WiFi network named **`AutoConnectAP`** and connect to it. The password is **`password`**.
3.  **Captive Portal**: A configuration page should automatically open in your browser. If not, open a browser and go to `192.168.4.1`.
4.  **Configure WiFi**: Click "Configure WiFi", select your home network (SSID), enter your password, and click "Save".
5.  **Restart**: The ESP32 will restart and automatically connect to your network. The onboard LED will turn solid `HIGH` to indicate a successful connection.
6.  **Find the IP**: Open the Arduino Serial Monitor at `115200` baud to see the IP address, or simply navigate to **`http://serialclicker.local`** in your browser.

### 4. Operation

1.  **Access the UI**: Open the web interface using the IP address or mDNS hostname.
2.  **Set Parameters**:
    -   Enter the desired **Number of Clicks**.
    -   Adjust the **Rotation Angle** slider to calibrate the servo's movement.
    -   The time gap input fields will generate automatically. Set the desired delay (**Hours, Minutes, Seconds**) for each click interval.
3.  **Start Sequence**: Click the **Start** button. The status panel will update in real-time.
4.  **Stop Sequence**: Click the **Stop** button at any time to immediately halt the sequence.

---

## 💡 How It Works

This project masterfully combines several technologies:

-   **ESPAsyncWebServer**: Serves the main HTML, CSS, and JavaScript file that constitutes the user interface.
-   **WebSockets**: Creates a persistent, two-way communication channel between the web browser and the ESP32. This allows the server to push status updates (like countdowns and click counts) to the client instantly without requiring the page to be reloaded.
-   **ArduinoJson**: Efficiently parses JSON data sent from the web UI (start/stop commands, angle, time gaps) and serializes status data to be sent back to the UI.
-   **WiFiManager**: Simplifies network setup by handling WiFi credential storage in the ESP32's non-volatile memory, eliminating the need to hardcode them. The on-demand trigger on `TRIGGER_PIN` provides a user-friendly way to reconfigure networking if the environment changes.

---

## 🤝 Contributing

Contributions, issues, and feature requests are welcome! Feel free to check the [issues page](https://github.com/your-username/your-repo/issues).



---
<div align="center">
  <p>All the right goes to Md Mahbubur Rahman.</p>
</div>
