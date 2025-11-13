# AI Camera UART Usage

This is a firmware read convert uart command and create a websocket server. It's originally for Arduino or Raspberry Pi Pico to connect to SunFounder Controller. Send command and data over UART in boardrate 115200

## Dependencies

- WebSockets by Markus Sattler
- ArduinoJson

## Update setting page www.

### Get WWW

Unpack setting page www to tools folder as build

### Update WWW

Run command

```bash
python tools/file_to_c_gz.py
```

It will update `ai-camera-firmware/www` folder

## Pack binary

1. In Arduino IDE, click "Sketch" -> "Export Compiled Binary", it will compile firmware to `ai-camera-firmware/build` folder
2. Run `python tools/pack_binary.py`, It will pack the binary file to `ai-camera-firmware/firmware` folder

## Fastory Reset

You may need to reset the module to factory settings, if you forget the AP SSID and password. To do this, Connect the board to power, short IO13 and IO15 with a tweezer or a screwdriver, then press the reset button, and release the reset button. You will see the big bright LED blink twice fast. Then you can release the tweezer or screwdriver, and press the reset button again. The module will reset to factory settings.

## Flash Setting

- Pay attention to enable PSRAM, select "Huge APP" Partition Scheme

    **Board: "ESP32 Dev Module"**
    **CPU Frequency: "240MHz (WiFi/BT)"**
    Core Debug Level: "None"
    Erase All Flash Before Sketch Upload: "Enabled"
    Events Run On: "Core 1"
    Flash Frequency: "80MHz"
    Flash Mode: "QIO"
    **Flash Size: "4MB (32Mb)"**
    Jtag Adapter: "Disabled"
    Arduino Runs On: "Core 1"
    **Partition Scheme: "Minimal SPIFFS APP (1.9MB APP with OTA/190k SPIFFS)"**
    **PSRAM: "Enabled"**
    Upload Speed: "921600"

## Commands

`SET+NAME<name>`: set device name

```
SET+NAMEAI-Camera
```

`SET+TYPE<type>`: set device type

```
SET+TYPEAI-Camera
```

`SET+SSID<ssid>`: set Wi-Fi SSID **DEPRECATED, use APPSK or STASSID instead**

```
SET+SSIDSunFounder
```

`SET+PSK<password>`: set Wi-Fi Password **DEPRECATED, use APPSK or STAPSK instead**

```
SET+PSKsunfounder
```

`SET+PORT<port>`: set Websocket Port

```
SET+PORT8765
```

`SET+MODE<mode>`: set Wi-Fi Mode, STA = 1, AP = 2 **DEPRECATED, now it's dual mode**

```
SET+MODE1
```

`SET+APSSID<ssid>`: set AP SSID

```
SET+APSSIDSunFounder
```

`SET+APPSK<password>`: set AP Password

```
SET+APPSKsunfounder
```

`SET+APCHN<channel>`: set AP Channel

```
SET+APCHN1
```

`SET+STASSID<ssid>`: set STA SSID

```
SET+STASSIDMakerStarsHall
```

`SET+STAPSK<password>`: set STA Password

```
SET+STAPSKsunfounder
```

`SET+START`: set Start, return IP if connected

```
SET+START
```

`SET+LAMP<value>`: set Lamp brightness, 0-100

```
set+LAMP50
```

`SET+RESET`: Reset module

```
SET+RESET
```

`SET+RSTSTA`: Restart STA, reconnect to Wi-Fi

```
SET+RSTSTA
```

`SET+RSTCFG`: Reset configuration Factory Reset

```
SET+RSTCFG
```

## Setting Page

You can access setting page under <http://192.168.4.1>

### Setting Page source code

See [ai-camera-www](https://github.com/sunfounder/ai-camera-www)

### Setting page API

See [Setting Page API](docs/setting-page-api.md)


## Data

`WS+<data>`: send data over websocket

```
WS+{"value": 25}
```

`WSB+<data>`: send binary data over websocket

```
WSB+Hello
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
