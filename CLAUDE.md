# AI Camera Firmware

ESP32-CAM firmware for Sunfounder Controller — communicates via Serial/WebSocket and streams MJPEG video.

## Project Overview

- **Target**: ESP32-CAM (OV3660 / OV2640 camera sensor)
- **Build System**: PlatformIO with Arduino framework (`platformio.ini`)
- **Version**: 1.5.4 (`src/main.cpp`, `#define VERSION`)

## Development Workflow

```
Dev:  x.y.z.a1 → .a2 → ...   (alpha suffix, each modify+upload)
Release: x.y+1.z              (strip .aN, close debug, bump version)
```

## Architecture

```
Serial / WS  ──►  main.cpp (loop)
                   ├── wsServerCameraHandler() → cameraInit() → camera.cpp (HW init + frame task on core 1)
                   ├── serialReceivedHandler() → SET+/WS+/WSB+ commands
                   ├── settingsLoop()          → WebServer (port 80) handleClient
                   └── ledStatusHandler()

Port 80   → settings.cpp   (WebServer: settings UI, OTA /update, /restart, etc.)
Port 9000 → camera_server.cpp (MJPEG stream + capture endpoints)
Port 8765 → ws_server.cpp  (WebSocket for Sunfounder Controller APP)
```

## Key Files

| File | Role |
|------|------|
| `src/main.cpp` | Entry point, command routing (SET+, WS+, WSB+) |
| `src/camera.cpp` | Camera HW init (`esp_camera_init`), creates frame-grab task on core 1 (stack 2KB) |
| `src/camera_server.cpp` | HTTP MJPEG stream (port 9000), capture endpoint |
| `src/settings.cpp` | WebServer (port 80): config UI, WiFi scan, **OTA update** (`/update` POST) |
| `src/ws_server.cpp` | WebSocket server (port 8765) for APP, multi-client (max 5) |
| `src/wifi_helper.cpp` | WiFi AP+STA management |
| `src/led_status.cpp` | Status LED patterns |
| `include/pins.h` | Pin definitions for ESP32-CAM / ESP32-S3-CAM |
| `include/defaults.h` | Default settings, frame size (96x96), fb_count (2) |

## OTA Update Flow

1. Client POSTs firmware binary to `http://<ip>/update`
2. `handleUpdate()` suspends camera task (`camera_stop()`), then writes via `Update` library
3. Error handling via flags — response sent from `handleUpdateReturn()` (avoids HTTP protocol errors)
4. On success, user calls `/restart` to boot into new firmware

## Camera Task

- `task_process_handler` runs on **core 1** with **1KB stack**
- Continuously calls `esp_camera_fb_get()` and `xQueueSend()` to the HTTP stream queue
- `camera_stop()` suspends the task via `vTaskSuspend()` before OTA to prevent flash write conflicts

## Build

```bash
pio run -e esp32cam
```

## Serial Protocol

Commands over Serial (115200 baud):
- `SET+KEY=VALUE` — set configuration
- `WS+{json}` — forward to WebSocket as text
- `WSB+{binary}` — forward to WebSocket as binary
