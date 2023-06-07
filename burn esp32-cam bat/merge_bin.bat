@REM esptool.exe -help

set boot_app0="boot_app0.bin"
set bootloader="ai-camera-firmware_v1.2.0_bootloader.bin"
set partitions="ai-camera-firmware_v1.2.0_partitions.bin"
set app="ai-camera-firmware_v1.2.0.bin"

set out_merged_app="ai-camera-firmware_v1.2.0_merged.bin"

esptool.exe --chip ESP32 merge_bin -o %out_merged_app% --flash_mode dio --flash_size 4MB 0x1000 %bootloader% 0x8000 %partitions% 0xe000 %boot_app0% 0x10000 %app%

pause
