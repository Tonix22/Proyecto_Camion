#!/bin/bash

:<<!
******NOTICE******
MUST set SDK_PATH & BIN_PATH firstly!!!
example:
export SDK_PATH=~/esp_iot_sdk_freertos
export BIN_PATH=~/esp8266_bin
!

export SDK_PATH=~/Share/meine_Tests/ESP8266_RTOS_SDK-2.0.0
export BIN_PATH=~/Share/meine_Tests/ESP8266_RTOS_SDK-2.0.0/bin

echo "gen_misc.sh version 20150911"
echo ""

if [ $SDK_PATH ]; then
    echo "SDK_PATH:"
    echo "$SDK_PATH"
    echo ""
else
    echo "ERROR: Please export SDK_PATH in gen_misc.sh firstly, exit!!!"
    exit
fi

if [ $BIN_PATH ]; then
    echo "BIN_PATH:"
    echo "$BIN_PATH"
    echo ""
else
    echo "ERROR: Please export BIN_PATH in gen_misc.sh firstly, exit!!!"
    exit
fi
echo "PRODUCT_TYPE:"
echo "$PRODUCT_TYPE"
echo "" 


echo "Please follow below steps(1-5) to generate specific bin(s):"
echo "STEP 1: use boot_v1.2+ by default"
boot=new

echo "boot mode: $boot"
echo ""
app=1
echo "generate bin: user1.bin"
echo ""

spi_speed=40
echo "spi speed: $spi_speed MHz"
echo ""

spi_mode=DIO
echo "spi mode: $spi_mode"
echo ""

spi_size_map=6
flash=4096
echo "spi size: 4096KB"
echo "spi ota map:  1024KB + 1024KB"

echo ""

echo "start..."
echo ""

make clean

make BOOT=$boot APP=$app SPI_SPEED=$spi_speed SPI_MODE=$spi_mode SPI_SIZE_MAP=$spi_size_map


BIN_NAME="user${app}.${flash}.${boot}.${spi_size_map}"
echo ""
echo "Generating $BIN_NAME.$PRODUCT_TYPE.bin..."
cp $BIN_PATH/upgrade/$BIN_NAME.bin $BIN_PATH/upgrade/$BIN_NAME.$PRODUCT_TYPE.bin
echo "Binary ready to be flashed: $BIN_NAME.$PRODUCT_TYPE.bin"
echo ""
