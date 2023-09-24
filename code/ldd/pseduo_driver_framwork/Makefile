obj-m :=psdfrm.o
ARCH=arm
COMPILE=arm-linux-gnueabihf-
KERN_DIR=/home/zmz/repo/final-project-123zmz123/env/linux
HOST_KERN_DIR = /lib/modules/$(shell uname -r)/build/
all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(COMPILE) -C $(KERN_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(COMPILE) -C $(KERN_DIR) M=$(PWD) clean

host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules

host_clean:
	make -C $(HOST_KERN_DIR) M=$(PWD) clean