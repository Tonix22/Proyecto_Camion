#!/bin/bash

#compile
./gen_misc.sh
if [ $? -eq 0 ]
then
    echo "FLASHING DEVICE ..."
    #esperrase
    sudo esptool.py erase_flash
    #espflash
    sudo esptool.py --port /dev/ttyUSB0 write_flash \
    --flash_mode dio \
    --flash_size 4MB \
    --flash_freq 40m \
    0x0 ../../bin/boot_v1.6.bin \
    0x1000 ../../bin/upgrade/user1.4096.new.4.bin \
    0x3FE000 ../../bin/blank.bin \
    0x3FC000 ../../bin/esp_init_data_default.bin
    sudo python terminal.py
else
    echo "FLASH AVOID DUE TO COMPILATION ERROR: $? "
fi