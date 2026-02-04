# ESP32-CAM Video Streaming Project

A complete video streaming solution for ESP32-CAM modules with web interface and Python client support. Stream live video, capture images, and control your ESP32-CAM through multiple interfaces.

![ESP32-CAM](https://img.shields.io/badge/Platform-ESP32--CAM-blue)
![PlatformIO](https://img.shields.io/badge/Framework-PlatformIO-orange)
![License](https://img.shields.io/badge/License-MIT-green)

## Features

- 📹 **Live MJPEG Video Streaming** - Real-time video streaming at ~33 FPS
- 📸 **Single Image Capture** - Capture and download individual frames
- 🌐 **Web Interface** - Built-in HTML interface accessible from any browser
- 🐍 **Python Client** - OpenCV-based client for advanced video processing
- 🎨 **Modern UI** - Responsive web interface with real-time controls
- ⚙️ **Optimized Settings** - Pre-configured camera settings for best quality
- 🔧 **PSRAM Support** - Automatic resolution selection (UXGA/SVGA)
- 🛡️ **Error Handling** - Comprehensive error messages and troubleshooting

## Hardware Requirements

### ESP32-CAM Module
- **ESP32-CAM** (AI-Thinker or compatible board)
- **Camera Module** OV2640 (usually included with ESP32-CAM)
- **PSRAM** (optional, but recommended for higher resolution)

### Programming Hardware
- **USB to Serial Adapter** (FTDI, CP2102, CH340, or similar)
- **Jumper Wires** for connections
- **5V Power Supply** - Stable power is critical for camera operation
  - Minimum 2A recommended
  - USB power may be insufficient for reliable operation

### Pin Connections (Programming Mode)

| FTDI/USB-Serial | ESP32-CAM |
|----------------|-----------|
| 5V             | 5V        |
| GND            | GND       |
| TX             | U0R (RX)  |
| RX             | U0T (TX)  |
| -              | GPIO 0 → GND (for programming mode) |

> **Important**: Connect GPIO 0 to GND only when uploading code. Remove this connection for normal operation.

## Software Requirements

### For ESP32-CAM Development
- **PlatformIO** - [Installation Guide](https://platformio.org/install)
  - Can be installed as VS Code extension or standalone CLI
- **Git** - For cloning the repository

### For Python Client (Optional)
- **Python 3.x** (3.7 or higher recommended)
- **Required Python packages**:
  ```bash
  pip install opencv-python numpy requests
  ```

### For Web Interface (Optional)
- Any modern web browser (Chrome, Firefox, Safari, Edge)

## Installation

### 1. Clone Repository
```bash
git clone https://github.com/stivanov78/camera_esp32.git
cd camera_esp32
```

### 2. Open in PlatformIO
- Open the project folder in VS Code with PlatformIO extension
- Or use PlatformIO CLI: `pio project init`

### 3. Update WiFi Credentials
Edit `src/main.cpp` and update your WiFi credentials:

```cpp
const char* ssid = "YOUR_WIFI_SSID";        // Replace with your WiFi name
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi password
```

### 4. Connect Hardware
1. Connect ESP32-CAM to USB-Serial adapter following the pin connections table above
2. **Important**: Connect GPIO 0 to GND to enable programming mode
3. Ensure stable 5V power supply is connected
4. Press the RESET button on ESP32-CAM

### 5. Upload Firmware
Using PlatformIO:
```bash
# Using PlatformIO CLI
pio run --target upload

# Or use VS Code PlatformIO: Click "Upload" button
```

### 6. Run Normal Mode
1. **Disconnect GPIO 0 from GND**
2. Press the RESET button on ESP32-CAM
3. Open Serial Monitor (115200 baud) to view output and get IP address

## Configuration

### Finding ESP32-CAM IP Address
1. Open Serial Monitor at 115200 baud
2. Press RESET button on ESP32-CAM
3. Wait for WiFi connection
4. Note the IP address displayed (e.g., `192.168.1.100`)

Example Serial Monitor output:
```
ESP32-CAM Video Streaming Server
================================
PSRAM found! Using UXGA resolution
Camera initialized successfully!

Connecting to WiFi: YourWiFiName
...........
WiFi connected successfully!
IP Address: 192.168.1.100

Available endpoints:
- Web Interface:  http://192.168.1.100
- Video Stream:   http://192.168.1.100/stream
- Capture Image:  http://192.168.1.100/capture
```

### Updating Client IP Address
- **Python Client**: Edit IP address in `receive_video_stream.py` or enter when prompted
- **Web Interface**: Open `view_stream.html` and enter IP in the input field

## Usage

### Method 1: Web Browser (Easiest)

#### Option A: Use Built-in Interface
1. Open web browser
2. Navigate to `http://[ESP32-CAM-IP]/`
3. View live stream directly

#### Option B: Use Client HTML Interface
1. Open `client/view_stream.html` in web browser
2. Enter ESP32-CAM IP address
3. Click "Connect"
4. Click "Start Stream" to begin streaming
5. Use "Capture Image" to download a frame

### Method 2: Python Client (Advanced)

```bash
cd client
python receive_video_stream.py
```

Menu options:
1. **Stream video** - Continuous video streaming with OpenCV display
2. **Capture single image** - Capture and save image as `esp32_capture.jpg`
3. **Exit** - Close the application

Keyboard controls during streaming:
- Press `q` to quit stream

### Method 3: Direct API Access

Access endpoints directly via HTTP:

- **Main Interface**: `http://[ESP32-IP]/`
- **Video Stream**: `http://[ESP32-IP]/stream`
- **Capture Image**: `http://[ESP32-IP]/capture`

## API Endpoints

### GET /
Main web interface with embedded video stream and controls.

**Response**: HTML page
```
Content-Type: text/html
Status: 200 OK
```

### GET /stream
MJPEG video stream endpoint.

**Response**: Multipart MJPEG stream
```
Content-Type: multipart/x-mixed-replace; boundary=frame
Status: 200 OK
```

Stream format:
```
--frame
Content-Type: image/jpeg
Content-Length: [size]

[JPEG data]
--frame
Content-Type: image/jpeg
...
```

### GET /capture
Capture a single JPEG image.

**Response**: JPEG image
```
Content-Type: image/jpeg
Content-Length: [size]
Content-Disposition: inline; filename=capture.jpg
Status: 200 OK
```

## Camera Settings

The firmware includes optimized camera settings for best streaming quality:

| Setting | Value | Description |
|---------|-------|-------------|
| **Resolution** | UXGA (1600x1200) with PSRAM<br>SVGA (800x600) without PSRAM | Automatically selected based on PSRAM availability |
| **JPEG Quality** | 10 (with PSRAM)<br>12 (without PSRAM) | Lower = higher quality (0-63 scale) |
| **Frame Buffers** | 2 (with PSRAM)<br>1 (without PSRAM) | Double buffering for smoother streaming |
| **XCLK Frequency** | 20 MHz | Camera clock frequency |
| **Frame Rate** | ~33 FPS | Controlled by 30ms delay |
| **Brightness** | 0 | Range: -2 to 2 |
| **Contrast** | 0 | Range: -2 to 2 |
| **Saturation** | 0 | Range: -2 to 2 |
| **White Balance** | Enabled | Auto white balance |
| **Exposure Control** | Enabled | Auto exposure |
| **Gain Control** | Enabled | Auto gain control |
| **Lens Correction** | Enabled | Corrects lens distortion |

### Modifying Camera Settings

Edit these lines in `src/main.cpp` after camera initialization:

```cpp
sensor_t * s = esp_camera_sensor_get();
s->set_brightness(s, 0);     // -2 to 2
s->set_contrast(s, 0);       // -2 to 2
s->set_saturation(s, 0);     // -2 to 2
// ... other settings
```

## Troubleshooting

### Camera Init Failed
**Symptoms**: Error message "Camera init failed with error 0x..."

**Solutions**:
1. Check camera module is properly connected
2. Verify all pin connections match AI-Thinker board
3. Try pressing RESET button
4. Check for loose cable connections
5. Verify camera module is OV2640

### WiFi Connection Issues
**Symptoms**: Cannot connect to WiFi, timeout errors

**Solutions**:
1. Verify SSID and password are correct
2. Check WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
3. Ensure router is in range
4. Check router allows new device connections
5. Try resetting ESP32-CAM and router

### Brown-out Detector (BOD) Problems
**Symptoms**: Random resets, "Brownout detector was triggered"

**Solutions**:
1. **Use external 5V power supply** (2A minimum)
2. Don't rely on USB-Serial adapter power
3. Use shorter, thicker power cables
4. Add 100µF capacitor between 5V and GND near ESP32-CAM
5. Check power supply stability with multimeter

### Upload Errors
**Symptoms**: "Failed to connect to ESP32", upload timeout

**Solutions**:
1. **Verify GPIO 0 is connected to GND** during upload
2. Press RESET button right before uploading
3. Try lower upload speed: `upload_speed = 115200` in platformio.ini
4. Check TX/RX are not swapped
5. Try different USB cable or port
6. Verify driver is installed for USB-Serial adapter

### Poor Video Quality / Low FPS
**Solutions**:
1. Ensure stable 5V power supply
2. Check if PSRAM is detected (see Serial Monitor)
3. Reduce resolution in code if needed
4. Ensure good WiFi signal strength
5. Close other network-intensive applications

### Stream Not Displaying in Browser
**Solutions**:
1. Verify ESP32-CAM IP address is correct
2. Check firewall settings
3. Ensure devices are on same network
4. Try different browser
5. Check Serial Monitor for errors
6. Try accessing `/capture` endpoint first to test camera

### Python Client Issues
**Solutions**:
1. Install required packages: `pip install opencv-python numpy requests`
2. Verify IP address is correct
3. Check firewall allows Python to access network
4. Ensure OpenCV is properly installed
5. Try using system Python instead of virtual environment

## Advanced Configuration

### Changing Frame Rate
Modify delay in `handleStream()` function:
```cpp
delay(30);  // ~33 FPS (1000ms / 30ms ≈ 33 FPS)
delay(60);  // ~16 FPS (1000ms / 60ms ≈ 16 FPS)
```

### Using Different Resolution
Without PSRAM, you can use these resolutions:
```cpp
config.frame_size = FRAMESIZE_QVGA;    // 320x240
config.frame_size = FRAMESIZE_VGA;     // 640x480
config.frame_size = FRAMESIZE_SVGA;    // 800x600
```

With PSRAM, additional options:
```cpp
config.frame_size = FRAMESIZE_HD;      // 1280x720
config.frame_size = FRAMESIZE_SXGA;    // 1280x1024
config.frame_size = FRAMESIZE_UXGA;    // 1600x1200
```

### Static IP Configuration
To use static IP instead of DHCP, add before `WiFi.begin()`:
```cpp
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  Serial.println("STA Failed to configure");
}
```

## Project Structure

```
camera_esp32/
├── platformio.ini              # PlatformIO configuration
├── README.md                   # This file
├── .gitignore                  # Git ignore rules
├── src/
│   └── main.cpp               # ESP32-CAM firmware
└── client/
    ├── receive_video_stream.py # Python OpenCV client
    └── view_stream.html        # Web-based viewer
```

## Technical Specifications

### ESP32-CAM Pin Configuration (AI-Thinker)
```cpp
PWDN_GPIO_NUM     32
RESET_GPIO_NUM    -1 (not used)
XCLK_GPIO_NUM     0
SIOD_GPIO_NUM     26  (I2C SDA)
SIOC_GPIO_NUM     27  (I2C SCL)

Data Pins:
Y9_GPIO_NUM       35
Y8_GPIO_NUM       34
Y7_GPIO_NUM       39
Y6_GPIO_NUM       36
Y5_GPIO_NUM       21
Y4_GPIO_NUM       19
Y3_GPIO_NUM       18
Y2_GPIO_NUM       5

VSYNC_GPIO_NUM    25
HREF_GPIO_NUM     23
PCLK_GPIO_NUM     22
```

### Network Specifications
- **Protocol**: HTTP/1.1
- **Port**: 80 (default HTTP)
- **Streaming Format**: MJPEG (Motion JPEG)
- **Boundary**: `--frame`
- **Image Format**: JPEG
- **WiFi Mode**: Station (STA)

## Python Requirements

Create `requirements.txt` for Python dependencies:
```txt
opencv-python>=4.5.0
numpy>=1.19.0
requests>=2.25.0
```

Install with:
```bash
pip install -r requirements.txt
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see below for details:

```
MIT License

Copyright (c) 2024 ESP32-CAM Video Streaming Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Acknowledgments

- ESP32 Arduino Core developers
- PlatformIO team
- OpenCV community
- ESP32-CAM community and contributors

## Support

If you find this project helpful, please consider:
- ⭐ Starring the repository
- 🐛 Reporting bugs and issues
- 💡 Suggesting new features
- 🔧 Contributing improvements

## Author

**ESP32-CAM Video Streaming Project**

For questions, issues, or suggestions, please open an issue on GitHub.

---

**Happy Streaming! 📹🚀**