rootdev : 
	mmcblk0p1 ---- internal eMMC.
        mmcblk1p1 ---- external SDCARD.
        sda1 --------- external USB devices. (USB memory stick, HDD)
        eth0 --------- nfsroot via external USB Ethernet interface.

odmdata : 0x30098011

miniloader base address : 0x40008000, in iRAM
fastboot base address : 0x40008000, in iRAM
U-Boot base address : 0x01008000, in SDRAM

