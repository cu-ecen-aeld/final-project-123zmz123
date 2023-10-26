obj-m :=led_drv.o
ARCH=arm
COMPILE=arm-linux-gnueabihf-
KERN_DIR=/home/zmz/repo/final-project-123zmz123/env/linux
all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(COMPILE) -C $(KERN_DIR) M=$(PWD) modules
	scp led_drv.ko debian@192.168.7.2:/home/debian

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(COMPILE) -C $(KERN_DIR) M=$(PWD) clean

