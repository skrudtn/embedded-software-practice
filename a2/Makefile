KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m = ku_pir.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc -o writer ku_pir_lib.c ku_pir_writer.c
	arm-linux-gnueabi-gcc -o reader ku_pir_lib.c ku_pir_reader.c
	arm-linux-gnueabi-gcc -o deleter ku_pir_lib.c ku_pir_remover.c
	arm-linux-gnueabi-gcc -o closer ku_pir_lib.c ku_pir_close.c

copy:
	scp ku_pir.ko ku_pir_mknod.sh writer reader deleter closer pi@10.10.10.13:~/a2

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
