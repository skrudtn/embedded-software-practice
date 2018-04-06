KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m = ku_ipc.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc -o ku_ipc_sender ku_ipc_sender.c ku_ipc_lib.c
	arm-linux-gnueabi-gcc -o ku_ipc_receiver ku_ipc_receiver.c ku_ipc_lib.c

	
copy:
	scp ku_ipc.ko ku_ipc_mknod.sh ku_ipc_sender ku_ipc_receiver pi@10.10.10.13:~/a1/

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
