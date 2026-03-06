# Set python utf-8 encoding
import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

import os
import shutil
import subprocess

# Emoji定义
class Emoji:
    SUCCESS = "✅"
    ERROR = "❌"
    INFO = "ℹ️"
    WARNING = "⚠️"
    BUILD = "🔨"
    COPY = "📋"
    MERGE = "🔄"
    DONE = "🎉"
    FILE = "📁"
    FLASH = "⚡"

SCRIPT_NAME = "ai-camera-firmware"
VERSION_FILE = os.path.join(f"ai-camera-firmware.ino")
BUILD_DIR = os.path.join("build", "esp32.esp32.esp32")
FIRMWARE_DIR = "burn-esp32-cam-bat"
VERSION = ""

# 从VERSION_FILE文件中提取版本号
def get_version():
    global VERSION
    try:
        with open(VERSION_FILE, "r", encoding="utf-8") as f:
            for line in f:
                if line.startswith("#define VERSION"):
                    VERSION = line.split(" ")[2].strip().strip('"')
                    break
        if not VERSION:
            raise ValueError(f"VERSION not found in {VERSION_FILE} file")
        return VERSION
    except Exception as e:
        print(f"{Emoji.ERROR} {e}")
        sys.exit(1)

# 创建固件目录
def create_firmware_dir():
    if not os.path.exists(FIRMWARE_DIR):
        os.makedirs(FIRMWARE_DIR)

# 执行命令并捕获输出
def run_command(cmd, description, show_output=False):
    try:
        if show_output:
            # 直接运行命令并显示输出
            result = os.system(cmd)
            if result != 0:
                print(f"{Emoji.ERROR} {description} failed!")
                sys.exit(1)
        else:
            # 捕获命令输出，只在失败时显示
            output = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"{Emoji.ERROR} {description} failed!")
        print(f"Command: {cmd}")
        print(f"Output: {e.output}")
        sys.exit(1)

# 主函数
def main():
    # 获取版本号
    version = get_version()
    
    # 创建固件目录
    create_firmware_dir()
    
    # 定义文件路径
    OTA_BIN = os.path.join(FIRMWARE_DIR, f"{SCRIPT_NAME}.v{version}-ota.bin")
    FACTORY_BIN = os.path.join(FIRMWARE_DIR, f"{SCRIPT_NAME}.v{version}-factory.bin")
    BOOT_APP = os.path.join("tools", "boot_app0.bin")
    FIRMWARE = os.path.join(BUILD_DIR, f"{SCRIPT_NAME}.ino.bin")
    BOOTLOADER = os.path.join(BUILD_DIR, f"{SCRIPT_NAME}.ino.bootloader.bin")
    PARTITIONS = os.path.join(BUILD_DIR, f"{SCRIPT_NAME}.ino.partitions.bin")
    ESPTOOL = os.path.join(FIRMWARE_DIR, "esptool.exe")
    
    # 打印标题
    print(f"{Emoji.BUILD} {SCRIPT_NAME} Build Tool")
    print(f"Version: {version}")
    # print("Source Files:")
    # print(f"  - Firmware:       {FIRMWARE}")
    # print(f"  - Bootloader:     {BOOTLOADER}")
    # print(f"  - Partitions:     {PARTITIONS}")
    # print(f"  - Boot App:       {BOOT_APP}")
    # print("\nOutput Files:")
    # print(f"  - OTA Binary:     {OTA_BIN}")
    # print(f"  - Factory Binary: {FACTORY_BIN}")
    
    # 复制OTA文件
    print(f"\n{Emoji.COPY} Copying firmware to OTA bin file...")
    try:
        shutil.copy(FIRMWARE, OTA_BIN)
        print(f"{Emoji.SUCCESS} OTA binary created: {OTA_BIN}")
    except Exception as e:
        print(f"{Emoji.ERROR} Failed to copy OTA binary: {e}")
        sys.exit(1)
    
    # 合并固件
    print(f"\n{Emoji.MERGE} Merging firmware to factory bin file...")
    cmd = f"{ESPTOOL} --chip ESP32 merge_bin -o {FACTORY_BIN} --flash_mode dio --flash_size 4MB 0x1000 {BOOTLOADER} 0x8000 {PARTITIONS} 0xe000 {BOOT_APP} 0x10000 {FIRMWARE}"
    if run_command(cmd, "Merging firmware", show_output=False):
        print(f"{Emoji.SUCCESS} Factory binary created: {FACTORY_BIN}")
    
    # 打印完成信息
    print(f"{Emoji.DONE} Build completed successfully!")
    print("Files created:")
    print(f"  - OTA Binary:     {OTA_BIN}")
    print(f"  - Factory Binary: {FACTORY_BIN}")

if __name__ == "__main__":
    main()