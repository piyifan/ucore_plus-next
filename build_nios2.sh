	cd ucore
	make ARCH=nios2 menuconfig
	make kernel
	make sfsimg
