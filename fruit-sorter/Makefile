KBUILD_EXTRA_SYMBOLS := ./Module.symvers
KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

fruit_sorter-objs := fs_main.o fs_hcsr04.o
obj-m := fruit_sorter.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules

copy:
	scp fruit_sorter.ko ins.sh rm.sh pi@10.10.10.13:~/fruit-sorter
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
