# AI Camera UART Usage

This is a firmware read convert uart command and create a websocket server. It's originally for Arduino or Raspberry Pi Pico to connect to SunFounder Controller. Send command and data over UART in boardrate 115200

## Update setting page www.

### Get WWW

Unpack setting page www to tools folder as build

### Update WWW

Run command

```bash
python tools/file_to_c_gz.py
```

It will update `ai-camera-firmware/www` folder

## Build binary

Run command to build firmware

```bash
python tools/build.py
```

It will build the firmware to `ai-camera-firmware/firmware` folder

## Factory Reset

You may need to reset the module to factory settings, if you forget the AP SSID and password. To do this, Connect the board to power, short IO13 and IO15 with a tweezer or a screwdriver, then press the reset button, and release the reset button. You will see the big bright LED blink twice fast. Then you can release the tweezer or screwdriver, and press the reset button again. The module will reset to factory settings.

## LED Status Indicators

The ESP32-CAM has a built-in LED (pin 33) that indicates the device status.

| LED Signal | Meaning |
|---|---|
| Slow blink (1s cycle) | Waiting for commands (disconnected) |
| LED on (solid) | Connected and camera running normally |
| 3 fast blinks, pause, repeat | Camera not detected (init failed or sensor not found) |
| 5 fast blinks, pause, repeat | Frame capture failure (camera stopped responding) |

## Flash Settings

Flash configuration is managed by PlatformIO in `platformio.ini`. No manual Arduino IDE settings needed.

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
