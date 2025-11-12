# Set python utf-8 encoding


import os
import shutil

SCRIPT_NAME = "ai-camera-firmware"
INO_NAME = os.path.join(SCRIPT_NAME, f"{SCRIPT_NAME}.ino")
BUILD_DIR = os.path.join(SCRIPT_NAME, "build", "esp32.esp32.esp32")
FIRMWARE_DIR = "firmware"
VERSION = ""

with open(INO_NAME, "r", encoding="utf-8") as f:
    for line in f:
        if line.startswith("#define VERSION"):
            VERSION = line.split(" ")[2].strip().strip('"')
            break
if not VERSION:
    raise ValueError("VERSION not found in ino file")

OTA_BIN = os.path.join(FIRMWARE_DIR, f"{SCRIPT_NAME}.v{VERSION}-ota.bin")
FACTORY_BIN = os.path.join(FIRMWARE_DIR, f"{SCRIPT_NAME}.v{VERSION}-factory.bin")
BOOT_APP = os.path.join("tools", "boot_app0.bin")
MAIN = os.path.join(BUILD_DIR, f"{SCRIPT_NAME}.ino.bin")
BOOTLOADER = os.path.join(BUILD_DIR, f"{SCRIPT_NAME}.ino.bootloader.bin")
PARTITIONS = os.path.join(BUILD_DIR, f"{SCRIPT_NAME}.ino.partitions.bin")
ESPTOOL = os.path.join(FIRMWARE_DIR, "esptool.exe")

def main():
    # 从ino文件中提取版本号
    print(f"VERSION: {VERSION}")
    shutil.copy(MAIN, OTA_BIN)

    cmd = f"{ESPTOOL} --chip ESP32 merge_bin -o {FACTORY_BIN} --flash_mode dio --flash_size 4MB 0x1000 {BOOTLOADER} 0x8000 {PARTITIONS} 0xe000 {BOOT_APP} 0x10000 {MAIN}"

    print(cmd)
    os.system(cmd)

if __name__ == "__main__":
    main()