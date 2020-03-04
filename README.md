# TPT1838
TPT183825C

# Useful links
[DPavlin's work](https://saturn.ffzg.hr/rot13/index.cgi?lenovo_thinkpad_tablet)

[A500 Info](http://projects.pappkartong.se/a500/)

[XDA](https://forum.xda-developers.com/wiki/Lenovo_ThinkPad_Tablet)

[GSMARENA](https://www.gsmarena.com/lenovo_thinkpad-4444.php)

[Official ROM parts, useless](https://download.lenovo.com/slates/think/tablet1/)

[Official OpenSource](http://download.lenovo.com/lenovo/content/sm/ThinkTabletOpenSource.zip)

[L4T R16](https://developer.nvidia.com/linux-tegra-rel-16)

[Generic SBK Dumper](https://forum.xda-developers.com/showthread.php?t=2071626)

# Android Recovery mode
## How to enter Recovery mode?
1. Turn off the tablet and disconnect it from PC
2. Press the **Volume Up** button and **Power** button together for 2 seconds
3. The tablet comes into Recovery mode and the screen shows Android logo.

# APX mode
## What's APX mode?
[XDA APX](https://forum.xda-developers.com/wiki/APX_mode)

APX mode is a special low-level diagnostic and device-programming mode for NVIDIA Tegra–based devices. The firmware implementing APX mode is stored in the boot ROM and hence can never be changed.
## How to enter APX mode?
1. Turn off the tablet and disconnect it from PC
2. Press the **Rotation** button and hold it
4. Press the **Power** button for approximately 2 seconds
5. The tablet screen keeps black
6. Connect the tablet via Micro-USB cable to PC
7. The tablet will recognize the tablet as APX device
In Linux, it's recognized as:
`Bus 003 Device 004: ID 0955:7820 NVIDIA Corp. T20 [Tegra 2] recovery mode`

## How to exit APX mode?
Press and hold the power button for about six seconds. The device should immediately boot into its normal mode. 
## What can be done in APX mode?
* NVIDIA's **nvflash** program can be used for controlling/manipulating a device in APX mode.
* NVIDIA's **tegrarcm** program can be used to send code to Tegra devices.

# nvflash, first try, failed caused by SBK
[nvflash download](http://developer.download.nvidia.com/mobile/tegra/l4t/r16.5.0/ventana_release_armhf/Tegra20_Linux_R16.5_armhf.tbz2)

[nvflash on XDA](https://forum.xda-developers.com/showthread.php?t=1745450)

APX mode is recognized in Linux:
`Bus 003 Device 004: ID 0955:7820 NVIDIA Corp. T20 [Tegra 2] recovery mode`

**nvflash** file is renamed as **nvflash_t20** and moved into */usr/bin/*.
A file **52-nvflash.rules** has been created in */etc/udev/rules.d/* with the following content:

`# APX mode of NVIDIA Tegra 2
SUBSYSTEM=="usb", ATTR{idVendor}=="0955", ATTR{idProduct}=="7820", MODE="0660"`

The purpose is to use nvflash without in **root** mode.

After the connection of the tablet in APX mode, try to call nvflash command.
My God, **nvflash** doesn't work, the tablet gets disconnected quickly, because **Secure Boot Key (SBK)** has been enabled on the device.

```NVIDIA Tegra 2 systems have an optional signing process using a Secure Boot Key (SBK). A SBK is a shared-secret AES128 encryption key. The SBK is permanently burned into ROM within the device, and is readable only by a hardware AES engine.

Devices employing the SBK mechanism require nvflash commands to be encrypted with the SBK; nvflash performs the encryption, given the SBK value. If this is not done, or the given SBK is incorrect, nvflash will exit with status 0x4. 
```

Next step, try to get the SBK in the tablet.

# Find out SBK
Some useful information from the following link:
[SBK Dumper](https://forum.xda-developers.com/showthread.php?t=2071626)

In the linux kernel, there is some protection that block the user try to read out SBK when the device in **odm_production** status.
/sys/firmware/fuse/secure_boot_key

Start the tablet and enable the Debug mode
Confirmation in shell:

`Bus 003 Device 004: ID 17ef:741c Lenovo ThinkPadTablet`
List of devices with **adb**:

```adb devices
List of devices attached
* daemon not running; starting now at tcp:5037
* daemon started successfully
MP0A99W	device
```

```shell@android:/storage/sdcard1 # uname -a
Linux localhost 2.6.39.4-g40c7636-dirty #1 SMP PREEMPT Thu Feb 7 20:07:37 CET 2013 armv7l GNU/Linux
shell@android:/storage/sdcard1 # cat /proc/version
Linux version 2.6.39.4-g40c7636-dirty (koshu@koshu-desktop) (gcc version 4.4.3 (GCC) ) #1 SMP PREEMPT Thu Feb 7 20:07:37 CET 2013
shell@android:/storage/sdcard1 # cat /proc/cpuinfo
Processor		: ARMv7 Processor rev 0 (v71)
processor		: 0
BogoMIPS		: 996.14

processor		: 1
BogoMIPS		: 996.14

Features		: swp half thumb fastmult vfp edsp vfpv3 vfpv3d16
CPU implementer	: 0x41
CPU architecture : 7
CPU variant		: 0x1
CPU part		: Oxc09
CPU revision	: 0

Hardware		: ventana
Revision		: 0000
Serial			: 0000000000000000
```
```adb shell
shell@android:/ $ df
Filesystem             Size   Used   Free   Blksize
/dev                   357M    32K   357M   4096
/mnt/asec              357M     0K   357M   4096
/mnt/obb               357M     0K   357M   4096
/sqlite_stmt_journals     4M     0K     4M   4096
/system                755M   310M   445M   4096
/data                   27G     6G    20G   4096
/cache                 885M    81M   804M   4096
/mnt/pia               503M   329M   174M   4096
/mnt/persdata           19M     8M    10M   4096
/storage/sdcard0        27G     6G    20G   4096
/storage/sdcard1         1G     1G   727M   4096
```

Enable support of **su** for adb:

Go to **Settings** -> **Developer options** -> **Root access**, and change option to **"Apps and ADB"**.

Cannot insmod **ss.ko** now, error as

`insmod: init_module 'ss.ko' failed (Exec format error)`

## Recompile **testk.ko** for kernel in the tablet
The **vermagic** of the kernel in the tablet is **2.6.39.4-g40c7636-dirty #1 SMP PREEMPT**.

[Cross compile kernel module](https://blukat29.github.io/2017/12/cross-compile-arm-kernel-module/)

[Kernel local version](https://nanxiao.me/en/how-to-modify-the-local-version-of-linux-kernel/)

[Build module for precompiled kernel](https://linux.die.net/lkmpg/x380.html)


## Fuse and 
SBK : 128 bits
Device Key (DK): 32 bits
Unique ID (UID): 64 bits


# tegrarcm
[tegrarcm on Github](https://github.com/NVIDIA/tegrarcm)




