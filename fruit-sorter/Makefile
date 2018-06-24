KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := fs_hcsr04.o fs_main.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc -o app_fs app_fs.c
	

copy:
	scp fs_main.ko fs_hcsr04.ko app_fs ins.sh rm.sh mknod.sh  pi@10.10.10.13:~/fruit-sorter
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
