@REM esptool.exe -help

set baud=921600
set boot_app0="boot_app0.bin"
set bootloader="ai-camera-firmware-v1.4.2-bootloader.bin"
set partitions="ai-camera-firmware-v1.4.2-partitions.bin"
set bin_file="ai-camera-firmware-v1.4.2-ota.bin"

esptool.exe --chip esp32 --baud %baud% --before default_reset erase_flash
esptool.exe --chip esp32 --baud %baud% --before default_reset write_flash -z 
--flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 %bootloader% 0x8000 %partitions% 0xe000 %boot_app0% 0x10000 %bin_file%

pause
