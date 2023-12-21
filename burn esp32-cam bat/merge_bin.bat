@REM esptool.exe -help

set boot_app0="boot_app0.bin"
set bootloader="ai-camera-firmware-v1.4.0-bootloader.bin"
set partitions="ai-camera-firmware-v1.4.0-partitions.bin"
set app="ai-camera-firmware-v1.4.0.bin"

set out_merged_app="ai-camera-firmware-v1.4.0-merged.bin"

esptool.exe --chip ESP32 merge_bin -o %out_merged_app% --flash_mode dio --flash_size 4MB 0x1000 %bootloader% 0x8000 %partitions% 0xe000 %boot_app0% 0x10000 %app%

pause
