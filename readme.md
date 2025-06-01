

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
Ctrl + A → then K → then Y
lsof /dev/ttyACM0
kill -9 1924437
```
<sup> kill with `lsof /dev/ttyACM0` then `kill -9 1924437` </sup>  

```
minicom -b 115200 -D /dev/ttyACM0
CTRL-A Z Q
```
MacOS
```
ls /dev/cu.*
```
```
screen /dev/cu.usbmodemflip_Poanmur1
```

clone and update flipper
```
git clone https://github.com/flipperdevices/flipperzero-firmware.git ~/git
```
```
./fbt
./fbt flash_usb
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




