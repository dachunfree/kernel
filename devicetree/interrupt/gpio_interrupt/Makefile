KERN_DIR = /home/liuchunming/weige/kernel/linux-4.19-rc3

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= button.o
