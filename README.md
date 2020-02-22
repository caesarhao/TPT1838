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

`NVIDIA Tegra 2 systems have an optional signing process using a Secure Boot Key (SBK). A SBK is a shared-secret AES128 encryption key. The SBK is permanently burned into ROM within the device, and is readable only by a hardware AES engine.

Devices employing the SBK mechanism require nvflash commands to be encrypted with the SBK; nvflash performs the encryption, given the SBK value. If this is not done, or the given SBK is incorrect, nvflash will exit with status 0x4. `

Next step, try to get the SBK in the tablet.

# Find out SBK
Some useful information from the following link:
[SBK Dumper](https://forum.xda-developers.com/showthread.php?t=2071626)

In the linux kernel, there is some protection that block the user try to read out SBK when the device in **odm_production** status.
/sys/firmware/fuse/secure_boot_key

## Fuse and 
SBK : 128 bits
Device Key (DK): 32 bits
Unique ID (UID): 64 bits


# tegrarcm
[tegrarcm on Github](https://github.com/NVIDIA/tegrarcm)




