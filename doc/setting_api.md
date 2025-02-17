# ESP32-Cam API 文档

## 概述
ESP32-Cam 的 Web 服务器提供了多种 API 接口，用于配置和管理设备的 WiFi 设置、OTA 更新以及其他相机参数。

## API 接口

### 1. 获取主页
- **URL**: `/`
- **Method**: `GET`
- **Description**: 返回设备的主页 HTML。
- **Response**:
    ```html
    <!doctype html><html lang=en><head><meta charset=UTF-8><meta http-equiv=X-UA-Compatible content="IE=edge"><meta name=viewport content="width=device-width,initial-scale=1"><title>ESP32-Cam OTA</title><style>...</style><script>...</script></head><body>...</body></html>
    ```

### 2. 获取设置
- **URL**: `/settings`
- **Method**: `GET`
- **Description**: 返回设备的当前设置信息。
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
      "macPrefix": "xx:xx:xx"
    }
    ```

### 3. 设置设备名称
- **URL**: `/set-name`
- **Method**: `POST`
- **Description**: 设置设备名称。
- **Request Parameter**:
    - `name` (String): 设备名称。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 4. 设置设备类型
- **URL**: `/set-type`
- **Method**: `POST`
- **Description**: 设置设备类型。
- **Request Parameter**:
    - `type` (String): 设备类型。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 5. 设置 AP SSID
- **URL**: `/set-apSsid`
- **Method**: `POST`
- **Description**: 设置 AP 模式的 SSID。
- **Request Parameter**:
    - `apSsid` (String): AP 模式的 SSID。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 6. 设置 AP 密码
- **URL**: `/set-apPassword`
- **Method**: `POST`
- **Description**: 设置 AP 模式的密码。
- **Request Parameter**:
    - `apPassword` (String): AP 模式的密码。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 7. 设置 AP 通道
- **URL**: `/set-apChannel`
- **Method**: `POST`
- **Description**: 设置 AP 模式的通道。
- **Request Parameter**:
    - `apChannel` (int): AP 模式的通道号（1-11）。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 8. 设置相机水平镜像
- **URL**: `/set-cameraHorizontalMirror`
- **Method**: `POST`
- **Description**: 设置相机是否水平镜像。
- **Request Parameter**:
    - `cameraHorizontalMirror` (bool): 是否水平镜像（true/false）。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 9. 设置相机垂直翻转
- **URL**: `/set-cameraVerticalFlip`
- **Method**: `POST`
- **Description**: 设置相机是否垂直翻转。
- **Request Parameter**:
    - `cameraVerticalFlip` (bool): 是否垂直翻转（true/false）。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 10. 设置相机亮度
- **URL**: `/set-cameraBrightness`
- **Method**: `POST`
- **Description**: 设置相机的亮度。
- **Request Parameter**:
    - `cameraBrightness` (float): 亮度值。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 11. 设置相机对比度
- **URL**: `/set-cameraContrast`
- **Method**: `POST`
- **Description**: 设置相机的对比度。
- **Request Parameter**:
    - `cameraContrast` (float): 对比度值。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 12. 设置相机饱和度
- **URL**: `/set-cameraSaturation`
- **Method**: `POST`
- **Description**: 设置相机的饱和度。
- **Request Parameter**:
    - `cameraSaturation` (float): 饱和度值。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 13. 设置相机锐度
- **URL**: `/set-cameraSharpness`
- **Method**: `POST`
- **Description**: 设置相机的锐度。
- **Request Parameter**:
    - `cameraSharpness` (float): 锐度值。
- **Response**:
    - `200 OK`: 设置成功。
    - `400 Bad Request`: 请求参数错误。

### 14. OTA 更新返回状态
- **URL**: `/update`
- **Method**: `POST`
- **Description**: 返回 OTA 更新的状态。
- **Response**:
    - `200 OK`: 更新成功，返回 "OK"。
    - `200 OK`: 更新失败，返回 "FAIL"。

### 15. OTA 更新
- **URL**: `/update`
- **Method**: `POST`
- **Description**: 上传 firmware 文件进行 OTA 更新。
- **Request Parameter**:
    - `update` (file): 要上传的 firmware 文件（.bin 格式）。
- **Response**:
    - `200 OK`: 更新成功，返回 "OK"。
    - `400 Bad Request`: 请求参数错误。
    - `200 OK`: 更新失败，返回 "FAIL"。

### 16. 扫描 WiFi
- **URL**: `/scan-wifi`
- **Method**: `GET`
- **Description**: 扫描并返回附近的 WiFi 网络。
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

### 17. 设置 STA 模式 WiFi
- **URL**: `/set-sta`
- **Method**: `POST`
- **Description**: 设置 STA 模式的 WiFi 连接信息。
- **Request Parameter**:
    - `ssid` (String): STA 模式的 SSID。
    - `password` (String): STA 模式的密码。
- **Response**:
    - `200 OK`: 连接成功，返回设备的 STA IP 地址。
    - `400 Bad Request`: 请求参数错误。
    - `400 Bad Request`: Wi-Fi 连接失败。
