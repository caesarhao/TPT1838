# TPT1838
TPT183825C

# Useful links
[DPavlin's work](https://saturn.ffzg.hr/rot13/index.cgi?lenovo_thinkpad_tablet)
[XDA](https://forum.xda-developers.com/wiki/Lenovo_ThinkPad_Tablet)
[GSMARENA](https://www.gsmarena.com/lenovo_thinkpad-4444.php)
[Official ROM parts, useless](https://download.lenovo.com/slates/think/tablet1/)
[Official OpenSource](http://download.lenovo.com/lenovo/content/sm/ThinkTabletOpenSource.zip)
[L4T R16](https://developer.nvidia.com/linux-tegra-rel-16)

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
5. The tablet vibrates and the screen keeps black
6. Connect the tablet via Micro-USB cable to PC
7. The tablet will recognize the tablet as APX device
## How to exit APX mode?
Press and hold the power button for about six seconds. The device should immediately boot into its normal mode. 
## What can be done in APX mode?
* NVIDIA's **nvflash** program can be used for controlling/manipulating a device in APX mode.
* NVIDIA's **tegrarcm** program can be used to send code to Tegra devices.

# nvflash
[nvflash download](http://developer.download.nvidia.com/mobile/tegra/l4t/r16.5.0/ventana_release_armhf/Tegra20_Linux_R16.5_armhf.tbz2)
[nvflash on XDA](https://forum.xda-developers.com/showthread.php?t=1745450)

# tegrarcm
[tegrarcm on Github](https://github.com/NVIDIA/tegrarcm)




