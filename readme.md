
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
lab.flipper.net #chromebrowser webusb   
https://forum.flipper.net/t/cli-command-line-interface-examples/1874/2
