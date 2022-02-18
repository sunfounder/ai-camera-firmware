## Build Guide

## Get ESP-IDF

ESP-WHO runs on ESP-IDF. For details on getting ESP-IDF, please refer to ESP-IDF Programming Guide.

Please use the latest version of ESP-IDF on the master branch.

## Get ESP-WHO

Run the following commands in your terminal to download ESP-WHO:

```bash
git clone --recursive https://github.com/espressif/esp-who.git
```

Remember to use `git submodule update --recursive --init` to pull and update submodules of ESP-WHO.

## Change EXTRA_COMPONENT_DIRS

open `CMakeLists.txt` and change the `EXTRA_COMPONENT_DIRS` to where your esp-who is located.

```CMake
set(EXTRA_COMPONENT_DIRS /Users/cavon/esp/esp-who/components)
```

## Set the target chip

Open the terminal and go to any folder that stores examples (e.g. examples/human_face_detection/lcd). Run the following command to set the target chip:

```bash
idf.py set-target [SoC]
```

Replace [SoC] with your target chip, e.g. esp32, esp32s2, esp32s3.

## Configure the camera

If not using the Espressif development boards mentioned in [Hardware](#Hardware), configure the camera pins manually. Enter `idf.py menuconfig` in the terminal and click (Top) -> Component config -> ESP-WHO Configuration to enter the ESP-WHO configuration interface, as shown below:

![](./docs/img/esp-who_config.png)

Click Camera Configuration to select the pin configuration of the camera according to the development board you use, as shown in the following figure:

![](./docs/img/esp-who_config_camera_config_select_pinout.png)

If the board you are using is not shown in the figure above, please select ``Custom Camera Pinout`` and configure the corresponding pins correctly, as shown in the following figure: 

![](./docs/img/esp-who_config_camera_config_custom.png)

## Configure the Wi-Fi

If the output of example is displayed on web server, click Wi-Fi Configuration to configure Wi-Fi password and other parameters, as shown in the following figure: 

![](./docs/img/esp-who_config_wifi_config.png)

## Launch and monitor

Flash the program and launch IDF Monitor:

```bash
idf.py flash monitor
```
