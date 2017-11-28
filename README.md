# APCI

Fork of https://github.com/accesio/APCI

## Installation - specific for Ubuntu 16.04 running on ADL PC104 with Access I/O 4-port relay board

1. Install your default headers  `sudo apt-get install linux-headers-$(uname -r)`
2. Verify that they were installed. They should exist under `/usr/src/linux-headers-$(uname -r )`
3. Make: `make KDIR=/usr/src/linux-headers-$(uname -r)`

If all goes well we should have the `apci.ko` kernel module in the build directory.

Now we need to install the kernel module

## Checking

Once we have the kernel module loaded, you should see that the module is associated with the board...
```
lspci -vn

01:00.0 ff00: 494f:0100 (rev 70)
	Subsystem: 494f:0100
	Flags: bus master, fast devsel, latency 0, IRQ 16
	I/O ports at ac00 [size=256]
	Memory at fd4ff000 (32-bit, non-prefetchable) [size=256]
	I/O ports at af00 [size=64]
	Capabilities: <access denied>
	Kernel driver in use: acpi
	Kernel modules: apci
```

## Making the example

Had to make some mods to the original source for this to work.

```
cd apcilib
make
sudo ./tester
```

The example should be able to access the board and give us the BAR (bass address register)
```
sudo ./tester 
Devices Found:1
Device 0
status:0
Device ID: 0100
BAR[0]:0
BAR[1]:0
BAR[2]:AF00
BAR[3]:0
BAR[4]:0
BAR[5]:0
```


