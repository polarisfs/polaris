obj-m += init.o
kernelpath = /usr/src/linux-headers-$(shell uname -r)

all:
	make -C $(kernelpath) M=$(PWD) modules

clean:
	make -C $(kernelpath) M=$(PWD) clean
