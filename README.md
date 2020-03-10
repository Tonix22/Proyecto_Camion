# Bustech a proyect with ESP8266

For making work this repo follow the steps

1. Install Lubuntu OVA file in virutal box 6 or higher with gest aditions. [Virtual Machine](https://drive.google.com/file/d/1IUcwQMMozoiYh4vhLy_iwbzmVj-vMFqK/view?usp=sharing)


2. Download [RTOS_SDK_folder](https://drive.google.com/drive/folders/1v_t7_RB9f1m5O2nrJXazsrjHLy7q6xAi?usp=sharing)

3. Go to linux home and link a sync folder, this one is called Share. It will be usefull for sync folders in your Host OS.
```console
esp8266@esp8266-VirtualBox:~$ ./mount.sh
esp8266@esp8266-VirtualBox:~$ cd Share
esp8266@esp8266-VirtualBox:~/Share$ 
```
4. Copy the RTOS_SDK_folder inside Share. You can do this in the HOST OS with the shared folder.
```console
esp8266@esp8266-VirtualBox:~/Share/ESP8266_RTOS_SDK-2.0.0$
```
The directory should look like this.

```
ESP8266_RTOS_SDK-2.0.0/
  ├── bin/
  ├── documents
  ├── driver_lib
  ├── examples
  |── extra_include
  |── include
  ├── ld
  ├── lib
  ├── third_party
```
5. Clone repo in this folder
6. Go to  ESP8266_RTOS_SDK-2.0.0/Proyecto_Camion/project_template and compile your first proyect. 
```console
esp8266@esp8266-VirtualBox:~/Share/ESP8266_RTOS_SDK-2.0.0/Proyecto_Camion/project_template$ ./gen_misc.sh
```

## Flash
For flash esp8266 in linux we used esptool.
1. Check that your usb is detected (ls usb)
2. Run the following script 

```console
esp8266@esp8266-VirtualBox:~/Share/ESP8266_RTOS_SDK-2.0.0/Proyecto_Camion/project_template$ ./setup.sh
```

## Tools


b. **Socket Test 3**
  This tool can emulate TCP or UDP servers, usefull for testing purposes.
  
## Notes
Some **script** files could fail for **\r\n** chars. To fix it just rewrite with\n

# Pending stuff

- [ ] Add ADC and battery coin to save data when system is turn off. 
- [ ] Check that back bars are working well and refactor it.
- [ ] Integral validation back bars traseras and front system
- [ ] Integrate FOTA
- [ ] Make documentation and flow diagrams
- [ ] Integrate web Rest APIS GET/POST for printer updates.
