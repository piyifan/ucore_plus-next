	cd ucore
	make ARCH=i386 menuconfig
	make kernel
	make sfsimg
