# ESP32-Cam API Documentation

## Overview
The ESP32-Cam's Web server provides multiple API interfaces for configuring and managing the device's WiFi settings, OTA updates, and other camera parameters.

## API Interfaces

### 1. Get Home Page
- **URL**: `/`
- **Method**: `GET`
- **Description**: Returns the HTML of the device's home page.
- **Response**:
    ```html
    <!doctype html><html lang="en"><head><meta charset="UTF-8"><meta http-equiv="X-UA-Compatible" content="IE=edge"><meta name="viewport" content="width=device-width,initial-scale=1"><title>ESP32-Cam OTA</title><style>...</style><script>...</script></head><body>...</body></html>
    ```

### 2. Get Settings
- **URL**: `/settings`
- **Method**: `GET`
- **Description**: Returns the current settings information of the device.
- **Response**:
    ```json
    {
      "version": "xxx.xxx.xxx",
      "name": "device_name",
      "type": "device_type",
      "apSsid": "ap_ssid",
      "apPassword": "ap_password",
      "apChannel": 1,
      "staSsid": "sta_ssid",
      "staPassword": "sta_password",
      "cameraHorizontalMirror": true,
      "cameraVerticalFlip": true,
      "cameraBrightness": 1.0,
      "cameraContrast": 1.0,
      "cameraSaturation": 1.0,
      "cameraSharpness": 1.0,
      "macAddress": "xx:xx:xx:xx:xx:xx",
      "macPrefix": "xx:xx:xx",
      "staConnected": "true",
      "ipAddress": "192.168.1.100"
    }
    ```

### 3. Set Device Name
- **URL**: `/set-name`
- **Method**: `POST`
- **Description**: Set the device name.
- **Request Parameter**:
    - `name` (String): Device name.
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 4. Set Device Type
- **URL**: `/set-type`
- **Method**: `POST`
- **Description**: Set the device type.
- **Request Parameter**:
    - `type` (String): Device type.
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 5. Set AP SSID
- **URL**: `/set-apSsid`
- **Method**: `POST`
- **Description**: Set the SSID in AP mode.
- **Request Parameter**:
    - `apSsid` (String): SSID in AP mode.
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 6. Set AP Password
- **URL**: `/set-apPassword`
- **Method**: `POST`
- **Description**: Set the password in AP mode.
- **Request Parameter**:
    - `apPassword` (String): Password in AP mode.
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 7. Set AP Channel
- **URL**: `/set-apChannel`
- **Method**: `POST`
- **Description**: Set the channel in AP mode.
- **Request Parameter**:
    - `apChannel` (int): Channel number in AP mode (1-11).
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 8. Set Camera Horizontal Mirror
- **URL**: `/set-cameraHorizontalMirror`
- **Method**: `POST`
- **Description**: Set whether the camera image should be horizontally mirrored.
- **Request Parameter**:
    - `cameraHorizontalMirror` (bool): Whether to horizontally mirror (true/false).
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 9. Set Camera Vertical Flip
- **URL**: `/set-cameraVerticalFlip`
- **Method**: `POST`
- **Description**: Set whether the camera image should be vertically flipped.
- **Request Parameter**:
    - `cameraVerticalFlip` (bool): Whether to vertically flip (true/false).
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 10. Set Camera Brightness
- **URL**: `/set-cameraBrightness`
- **Method**: `POST`
- **Description**: Set the brightness of the camera.
- **Request Parameter**:
    - `cameraBrightness` (int): Brightness value (range 0-100).
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 11. Set Camera Contrast
- **URL**: `/set-cameraContrast`
- **Method**: `POST`
- **Description**: Set the contrast of the camera.
- **Request Parameter**:
    - `cameraContrast` (int): Contrast value (range -2-2).
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 12. Set Camera Saturation
- **URL**: `/set-cameraSaturation`
- **Method**: `POST`
- **Description**: Set the saturation of the camera.
- **Request Parameter**:
    - `cameraSaturation` (int): Saturation value (range -2-2).
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 13. Set Camera Sharpness
- **URL**: `/set-cameraSharpness`
- **Method**: `POST`
- **Description**: Set the sharpness of the camera.
- **Request Parameter**:
    - `cameraSharpness` (int): Sharpness value (range -2-2).
- **Response**:
    - `200 OK`: Setting successful.
    - `400 Bad Request`: Request parameter error.

### 14. OTA Update Status
- **URL**: `/update`
- **Method**: `POST`
- **Description**: Returns the status of the OTA update.
- **Response**:
    - `200 OK`: Update successful, returns "OK".
    - `200 OK`: Update failed, returns "FAIL".

### 15. OTA Update
- **URL**: `/update`
- **Method**: `POST`
- **Description**: Upload a firmware file for OTA update.
- **Request Parameter**:
    - `update` (file): Firmware file to upload (.bin format).
- **Response**:
    - `200 OK`: Update successful, returns "OK".
    - `400 Bad Request`: Request parameter error.
    - `200 OK`: Update failed, returns "FAIL".

### 16. Scan WiFi
- **URL**: `/scan-wifi`
- **Method**: `GET`
- **Description**: Scan and return nearby WiFi networks.
- **Response**:
    ```json
    [
      {
        "ssid": "network_name",
        "rssi": -70,
        "secure": true,
        "channel": 1,
        "bssid": "xx:xx:xx:xx:xx:xx"
      },
      ...
    ]
    ```

### 17. Set STA Mode WiFi
- **URL**: `/set-sta`
- **Method**: `POST`
- **Description**: Set the WiFi connection information in STA mode.
- **Request Parameter**:
    - `ssid` (String): SSID in STA mode.
    - `password` (String): Password in STA mode.
- **Response**:
    - `200 OK`: Connection successful, returns the device's STA IP address.
    - `400 Bad Request`: Request parameter error.
    - `400 Bad Request`: Wi-Fi connection failed.

### 18. Restart Device
- **URL**: `/restart`
- **Method**: `POST`
- **Description**: Restart the device.
- **Response**:
    - `200 OK`: Restart successful.
