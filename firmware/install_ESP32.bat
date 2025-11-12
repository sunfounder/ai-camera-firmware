@REM esptool.exe -help

set baud=921600
set bin_file="ai-camera-firmware-v1.5.3.12-factory.bin"

esptool.exe --chip esp32 --baud %baud% --before default_reset erase_flash
esptool.exe --chip esp32 --baud %baud% --before default_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0000 %bin_file%

pause
