obj-m := hellom.o
ARCH=arm
COMPILE=arm-linux-gnueabihf-
KERN_DIR=/home/zmz/repo/final-project-123zmz123/env/linux
all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(COMPILE) -C $(KERN_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(COMPILE) -C $(KERN_DIR) M=$(PWD) clean

