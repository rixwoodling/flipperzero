```
              _.-------.._                    -,
          .-"```"--..,,_/ /`-,               -,  \ 
       .:"          /:/  /'\  \     ,_...,  `. |  |
      /       ,----/:/  /`\ _\~`_-"`     _;
     '      / /`"""'\ \ \.~`_-'      ,-"'/ 
    |      | |  0    | | .-'      ,/`  /
   |    ,..\ \     ,.-"`       ,/`    /
  ;    :    `/`""\`           ,/--==,/-----,
  |    `-...|        -.___-Z:_______J...---;
  :         `                           _-'
 _L_  _     ___  ___  ___  ___  ____--"`___  _     ___
| __|| |   |_ _|| _ \| _ \| __|| _ \   / __|| |   |_ _|
| _| | |__  | | |  _/|  _/| _| |   /  | (__ | |__  | |
|_|  |____||___||_|  |_|  |___||_|_\   \___||____||___|

Welcome to Flipper Zero Command Line Interface!
Read the manual: https://docs.flipper.net/development/cli
Run `help` or `?` to list available commands

Firmware version: 1.3.4 1.3.4 (ad2a8004 built on 22-04-2025)
```

Linux
```
ls -l /dev/ttyACM0
```
```
groups
sudo usermod -aG dialout $USER
newgrp dialout
```
```
screen /dev/ttyACM0 115200
# Ctrl + A → then K → then Y
# Ctrl + A → :kill
```
<sup> kill with `lsof /dev/ttyACM0` then `kill -9 1924437` </sup>  

MacOS
```
ls /dev/cu.*
```
```
screen /dev/cu.usbmodemflip_Poanmur1
```

clone and update flipper
```
git clone https://github.com/flipperdevices/flipperzero-firmware.git
git fetch --all
git branch -r
git checkout -b release origin/release
```
```
./fbt # build firmware
./fbt flash_usb # flash firmware through usb
./fbt fap_skeleton
./fbt launch APPSRC=applications_user/skeleton
```

```
rm -rf ./build/f7-firmware-D/.extapps/skeleton*
./fbt flash_usb
```
<sup>https://developer.flipper.net/flipperzero/doxygen/apps_on_sd_card.html</sup>



lab.flipper.net #chromebrowser webusb   
https://forum.flipper.net/t/cli-command-line-interface-examples/1874/2

```
brew install avrdude
```
```
ls /dev/tty.*
```
```
avrdude -c arduino -P /dev/tty.usbmodemflip_Poanmur3 -b 19200 -p attiny85 -U flash:r:dump.hex:i
```
```
xxd dump.hex | less
```
```
avrdude -c arduino -P /dev/tty.usbmodemflip_Poanmur3 -b 19200 -p attiny85 -U flash:w:dump.hex:i
```




