

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
```
minicom -b 115200 -D /dev/ttyACM0
CTRL-A Z Q
```
MacOS
```
ls /dev/cu.usbmodemflip_Poanmur1
```
```
screen /dev/cu.usbmodemflip_Poanmur1
```

```
./fbt
./fbt fap_skeleton
./fbt launch APPSRC=applications_user/skeleton
```
<sup>https://developer.flipper.net/flipperzero/doxygen/apps_on_sd_card.html</sup>



lab.flipper.net #chromebrowser webusb   
https://forum.flipper.net/t/cli-command-line-interface-examples/1874/2
