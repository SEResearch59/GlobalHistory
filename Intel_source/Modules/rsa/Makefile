obj-m := mychardev.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
ccflags-y := -std=gnu99 -Wno-declartion-after-statement

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

insert: clean all
	sudo rmmod mychardev
	sudo insmod mychardev.ko
