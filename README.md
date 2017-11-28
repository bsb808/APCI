# APCI

Fork of https://github.com/accesio/APCI

## Installation - specific for Ubuntu 16.04 running on PC104 board

1. Install your default headers  `sudo apt-get install linux-headers-$(uname -r)`
2. Verify that they were installed. They should exist under `/usr/src/linux-headers-$(uname -r )`
3. Make: `make KDIR=/usr/src/linux-headers-$(uname -r)`

If all goes well we should have the `apci.ko` kernel module in the build directory.

Now we need to install the kernel module

## Making the example

Had to make some mods to the original source for this to work.

```
cd apcilib
make
sudo ./tester
```


