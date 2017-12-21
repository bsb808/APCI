#!/bin/bash
sudo cp apci.ko /lib/modules/`uname -r`
sudo depmod -a 
sudo modprobe apci
lsmod | grep apci
echo "Now  add a file in etc/modules-load.d/"