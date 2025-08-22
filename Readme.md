# AI Camera UART Usage

This is a firmware read convert uart command and create a websocket server. It's originally for Arduino or Raspberry Pi Pico to connect to SunFounder Controller. Send command and data over UART in boardrate 115200

## Dependencies
 - ESP32 Arduino 2.0.17 (not working for 3.x for some reason, keeps disconnecting from websocket)
 - WebSockets by Markus Sattler (2.4.0)
 - ArduinoJson (7.4.2)

## Flash Setting
- Pay attention to enable PSRAM, select "Huge APP" Partition Scheme

    Board: "ESP32 Dev Module"
    Upload Speed: "921600"
    CPU Frequency: "240MHz (WiFi/BT)"
    Flash Frequency: "80MHz"
    Flash Mode: "QIO"
    Flash Size: "4MB (32Mb)"
    Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
    Core Debug Level: "None"
    PSRAM: "Enabled"
    Arduino Runs On: "Core 1"
    Events Run On: "Core 1"

## Commands

`SET+SSID<ssid>`: set Wi-Fi SSID

```
SET+SSIDSunFounder
```

`SET+PSK<password>`: set Wi-Fi Password

```
SET+PSKsunfounder
```

`SET+PORT<port>`: set Websocket Port

```
SET+PORT8765
```

`SET+MODE<mode>`: set Wi-Fi Mode, STA = 1, AP = 2

```
SET+MODE1
```

`SET+START`: set Start, return IP if connected

```
SET+START
```

`SET+RESET`: Reset module

```
SET+RESET
```

`SET+CAMERA_MODE`: set Camera pixel format, AI = 0, Stream = 1, Both = 2. AI need RGB565 format, Stream need JPEG format. In Stream-Only mode, camera output JPEG format directly, makes fps higher. In AI-only camera output RGB565, and doestn't need to convert to JPEG and stream, makes fps a little bit higher to 3 fps. In Both mode, camera output RGB565 format for AI than convert to JPEG for stream, makes fps low to 2 fps.

```
SET+CAMERA_MODE2
```

## Data

`WS+<data>`: send data over websocket

```
WS+{"value": 25}
```

`AI+<data>`: AI camera data [left, top, right, bottom]

```
AI+[93, 59, 208, 176]
```

## Example

Here's an example connecting to Wi-Fi with ssid: SunFounder and password: sunfounder. Then echos what it reads from websocket client. `[tx]` is what you need to send over UART, and `[rx]` is what you get from rx. Under `[rx]`, `[DEBUG]` will only appear in debug mode, and without it, is what you receive for your code.

```
[tx] SET+SSIDMakerStarsHall  // Set ssid to MakerStarsHall
[rx] [DEBUG] RX Receive: SET+SSIDMakerStarsHall
[rx] [DEBUG] Set SSID: SunFounder
[rx] [OK]

[tx] SET+PSKsunfounder   // Set password to sunfounder
[rx] [DEBUG] RX Receive: SET+PSKsunfounder
[rx] [DEBUG] Set password: sunfounder
[rx] [OK]

[tx] SET+MODE1   // Set mode to STA
[rx] [DEBUG] RX Receive: SET+MODE1
[rx] [DEBUG] Set mode: 1
[rx] [OK]

[tx] SET+MODE2   // Set mode to AP
[rx] [DEBUG] RX Receive: SET+MODE1
[rx] [DEBUG] Set mode: 1
[rx] [OK]

[tx] SET+PORT8765   // Set websocket server port to 8765
[rx] [DEBUG] RX Receive: SET+PORT8765
[rx] [DEBUG] Set port: 8765
[rx] [OK]

[tx] SET+START   // Start connecting and start websocket server
[rx] [DEBUG] Connecting  // it will automaticaly try to connect to Wi-Fi as you fill in both ssid and password
[rx] [DEBUG] WiFi connected
[rx] [DEBUG] IP address:
[rx] 192.168.43.145
[rx] [DEBUG] Is server live? 1
[rx] [DEBUG] Websocker on!    // Now Websocket is on!

[rx] [DEBUG] RX Receive: Hello    // Websocket receives a hello.
[tx] WS+Hello    // Send out a Hello back to the Websocket client.
```

```
SET+SSIDbuibuibui
SET+PSKsunfounder
SET+MODE1
SET+PORT8765
SET+START

SET+SSIDaaa
SET+PSKsunfounder
SET+MODE2
SET+PORT8765
SET+START

SET+SSIDMakerStarsHall
SET+PSKsunfounder
SET+MODE1
SET+PORT8765
SET+START

SET+LAMP0
SET+LAMP5
SET+LAMP10
```
