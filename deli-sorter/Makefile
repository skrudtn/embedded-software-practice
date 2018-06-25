KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := fs_hcsr04.o fs_main.o fs_tcs.o fs_decision.o
fs_tcs-objs := fs_tcs_dev.o fs_tcs_control.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc -o app_fs app_fs.c
	arm-linux-gnueabi-gcc -o app_ppt app_ppt.c

copy:
	scp fs_tcs.ko fs_main.ko fs_hcsr04.ko fs_decision.ko app_fs app_ppt ins.sh rm.sh mknod.sh  pi@10.10.10.13:~/deli-service
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
