obj-m += apci.o

KVERSION        :=      $(shell uname -r)

apci-objs :=          \
        apci_fops.o   \
	apci_dev.o

all:
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean: 
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
