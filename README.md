# TPT1838
TPT183825C

# Good workers on Github
Koshu

jbaiter

# Useful links
[DPavlin's work](https://saturn.ffzg.hr/rot13/index.cgi?lenovo_thinkpad_tablet)

[A500 Info](http://projects.pappkartong.se/a500/)

[XDA](https://forum.xda-developers.com/wiki/Lenovo_ThinkPad_Tablet)

[How to build CM9 for Thinkpad Tablet](https://github.com/jbaiter/thinkpad_tablet_device_tree)

[GSMARENA](https://www.gsmarena.com/lenovo_thinkpad-4444.php)

[Official ROM parts, useless](https://download.lenovo.com/slates/think/tablet1/)

[Official OpenSource](http://download.lenovo.com/lenovo/content/sm/ThinkTabletOpenSource.zip)

[L4T R16](https://developer.nvidia.com/linux-tegra-rel-16)

[Generic SBK Dumper](https://forum.xda-developers.com/showthread.php?t=2071626)

[jevinskie/fusee-launcher](https://github.com/jevinskie/fusee-launcher)

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

**Object**:

| Type	    |      Value 	    |  Reliability		  |
|-----------|:-----------------:|--------------------:|
| Device ID |  d7309f434630840a | From fusee-launcher |
| Chip UID  | ??    			|  ?? 				  |
| SBK		| ??				|  ??				  |

# Try to find out SBK with SBK Dumper
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

```
shell@android:/storage/sdcard1 # uname -a
Linux localhost 2.6.39.4-g40c7636-dirty #1 SMP PREEMPT Thu Feb 7 20:07:37 CET 2013 armv7l GNU/Linux
```
```
shell@android:/storage/sdcard1 # cat /proc/version
Linux version 2.6.39.4-g40c7636-dirty (koshu@koshu-desktop) (gcc version 4.4.3 (GCC) ) #1 SMP PREEMPT Thu Feb 7 20:07:37 CET 2013
```
```
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
```
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

```
shell@android:/ # cat /proc/cmdline                                            
tegraid=20.1.4.0.0 mem=1022M@0M android.commchip=2685344 vmalloc=256M androidboot.serialno=0a843046439f30d7 video=tegrafb no_console_suspend=1 console=none debug_uartport=lsport,-3 usbcore.old_scheme_first=1 lp0_vec=8192@0x1d826000 tegra_fbmem=8197120@0x1d82a000 tegraboot=sdmmc charging=0 isBatteryFail=0 gpt gpt_sector=62519295 
```

```
shell@android:/ # cat /proc/meminfo                                          
MemTotal:         731896 kB
MemFree:           14700 kB
Buffers:           35832 kB
Cached:           312088 kB
SwapCached:            0 kB
Active:           406004 kB
Inactive:         251760 kB
Active(anon):     310108 kB
Inactive(anon):      120 kB
Active(file):      95896 kB
Inactive(file):   251640 kB
Unevictable:         188 kB
Mlocked:               0 kB
HighTotal:        102400 kB
HighFree:            260 kB
LowTotal:         629496 kB
LowFree:           14440 kB
SwapTotal:             0 kB
SwapFree:              0 kB
Dirty:                 0 kB
Writeback:             0 kB
AnonPages:        310040 kB
Mapped:           143072 kB
Shmem:               196 kB
Slab:              17660 kB
SReclaimable:       7384 kB
SUnreclaim:        10276 kB
KernelStack:        6560 kB
PageTables:         9780 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:      365948 kB
Committed_AS:    6184852 kB
VmallocTotal:     253952 kB
VmallocUsed:       61972 kB
VmallocChunk:     170948 kB
DirectMap4k:      110596 kB
DirectMap2M:      544768 kB
```
```
shell@android:/ # cat /proc/partitions                                         
major minor  #blocks  name

 179        0   31259648 mmcblk0
 179        1       6144 mmcblk0p1
 179        2       8192 mmcblk0p2
 179        3     786432 mmcblk0p3
 179        4     921600 mmcblk0p4
 179        5       2048 mmcblk0p5
 179        6     524288 mmcblk0p6
 179        7      20480 mmcblk0p7
 259        0     143360 mmcblk0p8
 259        1      20480 mmcblk0p9
 259        2   28811264 mmcblk0p10
 179        8    1931264 mmcblk1
 179        9    1931196 mmcblk1p1

```
```

```

```
```


Enable support of **su** for adb:

Go to **Settings** -> **Developer options** -> **Root access**, and change option to **"Apps and ADB"**.

Cannot insmod **ss.ko** now, error as

`insmod: init_module 'ss.ko' failed (Exec format error)`

## Recompile **testk.ko** for kernel in the tablet
The **vermagic** of the kernel in the tablet is **2.6.39.4-g40c7636-dirty SMP preempt mod_unload ARMv7**.

[Cross compile kernel module](https://blukat29.github.io/2017/12/cross-compile-arm-kernel-module/)

[Kernel local version](https://nanxiao.me/en/how-to-modify-the-local-version-of-linux-kernel/)

[Build module for precompiled kernel](https://linux.die.net/lkmpg/x380.html)

Present of Thinkpad as Android device with command **lsusb**

`Bus 003 Device 004: ID 17ef:741c Lenovo ThinkPadTablet`

Error for install the module **testk.ko**

`<3>[  183.584018] testk: version magic '2.6.39.4-g40c7636-dirty SMP preempt ARMv7 ' should be '2.6.39.4-g40c7636-dirty SMP preempt mod_unload ARMv7 '
`
Recompile the module tesk.ko with option **mod-unload**

`make ARCH=arm CROSS=/opt/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-`

`make KERNEL=../linux-2.6.39.4 CROSS=/opt/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-`

```
adb shell
shell@android:/ $ su
shell@android:/ # dmesg
<6>[    0.000000] Initializing cgroup subsys cpu
<5>[    0.000000] Linux version 2.6.39.4-g40c7636-dirty (koshu@koshu-desktop) (gcc version 4.4.3 (GCC) ) #1 SMP PREEMPT Thu Feb 7 20:07:37 CET 2013
<4>[    0.000000] CPU: ARMv7 Processor [411fc090] revision 0 (ARMv7), cr=10c5387f
<4>[    0.000000] CPU: VIPT nonaliasing data cache, VIPT aliasing instruction cache
<4>[    0.000000] Machine: ventana
<4>[    0.000000] Ignoring unrecognised tag 0x41000801
<4>[    0.000000] Ignoring unrecognised tag 0x41000801
<4>[    0.000000] Ignoring unrecognised tag 0x41000801
<4>[    0.000000] Ignoring unrecognised tag 0x41000801
<4>[    0.000000] Ignoring unrecognised tag 0x41000801
<4>[    0.000000] Ignoring unrecognised tag 0x41000801
<4>[    0.000000] Ignoring unrecognised tag 0x41000801
<6>[    0.000000] Found tegra_fbmem: 007d1400@1d82a000
<6>[    0.000000] Found charging: 00000000
<6>[    0.000000] Found isBatteryFail: 00000000
<6>[    0.000000] Tegra reserved memory:
<6>[    0.000000] LP0:                    1d826000 - 1d827fff
<6>[    0.000000] Bootloader framebuffer: 1d82a000 - 1dffbfff
<6>[    0.000000] Framebuffer:            2e500000 - 2edfffff
<6>[    0.000000] 2nd Framebuffer:        2ee00000 - 2fdfffff
<6>[    0.000000] Carveout:               2fe00000 - 3fdfffff
<6>[    0.000000] Vpr:                    00000000 - 00000000
<4>[    0.000000] Memory policy: ECC disabled, Data cache writealloc
<7>[    0.000000] On node 0 totalpages: 189440
<7>[    0.000000] free_area_init_node: node 0, pgdat c076b640, node_mem_map c102a000
<7>[    0.000000]   Normal zone: 1280 pages used for memmap
<7>[    0.000000]   Normal zone: 0 pages reserved
<7>[    0.000000]   Normal zone: 162560 pages, LIFO batch:31
<7>[    0.000000]   HighMem zone: 764 pages used for memmap
<7>[    0.000000]   HighMem zone: 24836 pages, LIFO batch:7
<6>[    0.000000] Tegra SKU: 8 Rev: A04 CPU Process: 1 Core Process: 2 Speedo ID: 1
<6>[    0.000000] Tegra Revision: A04 SKU: 0x8 CPU Process: 1 Core Process: 2
<6>[    0.000000] L310 cache controller enabled
<6>[    0.000000] l2x0: 8 ways, CACHE_ID 0x410000c4, AUX_CTRL 0x7e080001, Cache size: 1048576 B
<6>[    0.000000] PERCPU: Embedded 8 pages/cpu @c1833000 s8960 r8192 d15616 u32768
<7>[    0.000000] pcpu-alloc: s8960 r8192 d15616 u32768 alloc=8*4096
<7>[    0.000000] pcpu-alloc: [0] 0 [0] 1 
<4>[    0.000000] Built 1 zonelists in Zone order, mobility grouping on.  Total pages: 187396
<5>[    0.000000] Kernel command line: tegraid=20.1.4.0.0 mem=1022M@0M android.commchip=2685344 vmalloc=256M androidboot.serialno=0a843046439f30d7 video=tegrafb no_console_suspend=1 console=none debug_uartport=lsport,-3 usbcore.old_scheme_first=1 lp0_vec=8192@0x1d826000 tegra_fbmem=8197120@0x1d82a000 tegraboot=sdmmc charging=0 isBatteryFail=0 gpt gpt_sector=62519295 
<6>[    0.000000] PID hash table entries: 4096 (order: 2, 16384 bytes)
<6>[    0.000000] Dentry cache hash table entries: 131072 (order: 7, 524288 bytes)
<6>[    0.000000] Inode-cache hash table entries: 65536 (order: 6, 262144 bytes)
<6>[    0.000000] Memory: 740MB = 740MB total
<5>[    0.000000] Memory: 731520k/731520k available, 315008k reserved, 102400K highmem
<5>[    0.000000] Virtual kernel memory layout:
<5>[    0.000000]     vector  : 0xffff0000 - 0xffff1000   (   4 kB)
<5>[    0.000000]     fixmap  : 0xfff00000 - 0xfffe0000   ( 896 kB)
<5>[    0.000000]     DMA     : 0xff000000 - 0xffe00000   (  14 MB)
<5>[    0.000000]     vmalloc : 0xe8800000 - 0xf8000000   ( 248 MB)
<5>[    0.000000]     lowmem  : 0xc0000000 - 0xe8000000   ( 640 MB)
<5>[    0.000000]     pkmap   : 0xbfe00000 - 0xc0000000   (   2 MB)
<5>[    0.000000]     modules : 0xbf000000 - 0xbfe00000   (  14 MB)
<5>[    0.000000]       .init : 0xc0008000 - 0xc003d000   ( 212 kB)
<5>[    0.000000]       .text : 0xc003d000 - 0xc0709d80   (6964 kB)
<5>[    0.000000]       .data : 0xc070a000 - 0xc076cfc0   ( 396 kB)
<6>[    0.000000] Preemptable hierarchical RCU implementation.
<6>[    0.000000] 	RCU-based detection of stalled CPUs is disabled.
<6>[    0.000000] 	Verbose stalled-CPUs detection is disabled.
<6>[    0.000000] NR_IRQS:480
<6>[    0.000000] sched_clock: 32 bits at 1000kHz, resolution 1000ns, wraps every 4294967ms
<6>[    0.000131] Calibrating delay loop... 996.14 BogoMIPS (lpj=4980736)
<6>[    0.090060] pid_max: default: 32768 minimum: 301
<6>[    0.090203] Mount-cache hash table entries: 512
<6>[    0.090703] Initializing cgroup subsys debug
<6>[    0.090714] Initializing cgroup subsys cpuacct
<6>[    0.090754] Initializing cgroup subsys freezer
<6>[    0.090780] CPU: Testing write buffer coherency: ok
<6>[    0.090981] hw perfevents: enabled with ARMv7 Cortex-A9 PMU driver, 7 counters available
<4>[    0.211286] CPU1: Booted secondary processor
<6>[    0.230960] Brought up 2 CPUs
<6>[    0.230971] SMP: Total of 2 processors activated (1992.29 BogoMIPS).
<6>[    0.233400] print_constraints: dummy: 
<6>[    0.233544] NET: Registered protocol family 16
<6>[    0.234078] host1x bus init
<6>[    5.240695] Selecting UARTD as the debug console
<6>[    5.240710] The debug console clock name is uartd
<6>[    5.241530] ram_console: got buffer at 2e400000, size 100000
<6>[    5.241670] ram_console: uncorrectable error in header
<6>[    5.241679] ram_console: no valid data in buffer (sig = 0x00001400)
<6>[    5.249652] console [ram-1] enabled
<4>[    5.301576] in register_ntrig_spi_devices!!!
<4>[    5.302079] detect LCD panel: LG
<6>[    5.346244] [ Camera ] _____  ventana_camera_init  _____
<7>[    5.346415] gpio_export: gpio172 status -22
<7>[    5.346503] gpio_export: gpio221 status -22
<7>[    5.346588] gpio_export: gpio26 status -22
<7>[    5.346746] gpio_export: gpio172 status -22
<7>[    5.346832] gpio_export: gpio220 status -22
<7>[    5.346918] gpio_export: gpio0 status -22
<4>[    5.347106] ################strap=1 #################
<6>[    5.347194] ventana_emc_init: Hynix 1GB memory found
<6>[    5.347352] tegra_init_emc: Hynix 300MHz memory found
<6>[    5.347537] hw-breakpoint: found 6 breakpoint and 1 watchpoint registers.
<6>[    5.347695] hw-breakpoint: 1 breakpoint(s) reserved for watchpoint single-step.
<6>[    5.347792] hw-breakpoint: maximum watchpoint size is 4 bytes.
<6>[    5.348133] tegra_arb_init: initialized
<6>[    5.348264] tegra_iovmm_register: added iovmm-gart
<6>[    5.355904] bio: create slab <bio-0> at 0
<6>[    5.356418] vgaarb: loaded
<4>[    5.356548] i2c-core: driver [max8907c] using legacy suspend method
<4>[    5.356707] i2c-core: driver [max8907c] using legacy resume method
<5>[    5.357012] SCSI subsystem initialized
<6>[    5.357642] tegra-otg tegra-otg: otg transceiver registered
<6>[    5.357844] usbcore: registered new interface driver usbfs
<6>[    5.358061] usbcore: registered new interface driver hub
<6>[    5.358220] usbcore: registered new device driver usb
<6>[    5.359476] tps6586x 4-0034: VERSIONCRC is 2c
<6>[    5.380962] Wake18 for irq=118
<6>[    5.381049] Enabling wake18
<6>[    5.381904] print_constraints: REG-SM_0: 725 <--> 1500 mV at 1275 mV normal standby
<6>[    5.382878] print_constraints: REG-SM_1: 725 <--> 1500 mV at 1000 mV normal standby
<6>[    5.384316] print_constraints: REG-SM_2: 3000 <--> 4550 mV at 3700 mV normal standby
<6>[    5.384993] print_constraints: REG-LDO_0: 1500 <--> 3300 mV at 1200 mV normal standby
<6>[    5.386097] print_constraints: REG-LDO_1: 725 <--> 1500 mV at 1100 mV normal standby
<6>[    5.386836] print_constraints: REG-LDO_2: 725 <--> 1500 mV at 1275 mV normal standby
<6>[    5.387822] print_constraints: REG-LDO_3: 1250 <--> 3300 mV at 3300 mV normal standby
<6>[    5.388861] print_constraints: REG-LDO_4: 1700 <--> 2475 mV at 1800 mV normal standby
<6>[    5.389462] print_constraints: REG-LDO_5: 1250 <--> 3300 mV at 2850 mV normal standby
<6>[    5.390583] print_constraints: REG-LDO_6: 1800 mV normal standby
<6>[    5.391267] print_constraints: REG-LDO_7: 1250 <--> 3300 mV at 3300 mV normal standby
<6>[    5.392012] print_constraints: REG-LDO_8: 1250 <--> 3300 mV at 1800 mV normal standby
<6>[    5.392627] print_constraints: REG-LDO_9: 1250 <--> 3300 mV at 2850 mV normal standby
<6>[    5.393249] Advanced Linux Sound Architecture Driver Version 1.0.24.
<6>[    5.393754] Bluetooth: Core ver 2.16
<6>[    5.393870] NET: Registered protocol family 31
<6>[    5.394027] Bluetooth: HCI device and connection manager initialized
<6>[    5.394118] Bluetooth: HCI socket layer initialized
<6>[    5.394274] Bluetooth: L2CAP socket layer initialized
<6>[    5.394431] Bluetooth: SCO socket layer initialized
<6>[    5.394761] cfg80211: Calling CRDA to update world regulatory domain
<6>[    5.395071] Switching to clocksource timer_us
<6>[    5.395367] nvmap uc page pool size=22860 pages
<6>[    5.400965] Switched to NOHz mode on CPU #0
<6>[    5.401325] Switched to NOHz mode on CPU #1
<6>[    5.716449] nvmap wc page pool size=22860 pages
<6>[    6.048251] tegra-nvmap tegra-nvmap: created carveout iram (255KiB)
<6>[    6.049535] tegra-nvmap tegra-nvmap: created carveout generic-0 (262144KiB)
<6>[    6.050471] NET: Registered protocol family 2
<6>[    6.050685] IP route cache hash table entries: 32768 (order: 5, 131072 bytes)
<6>[    6.051179] TCP established hash table entries: 131072 (order: 8, 1048576 bytes)
<6>[    6.052836] TCP bind hash table entries: 65536 (order: 7, 786432 bytes)
<6>[    6.053749] TCP: Hash tables configured (established 131072 bind 65536)
<6>[    6.053908] TCP reno registered
<6>[    6.053997] UDP hash table entries: 512 (order: 2, 16384 bytes)
<6>[    6.054176] UDP-Lite hash table entries: 512 (order: 2, 16384 bytes)
<6>[    6.054523] NET: Registered protocol family 1
<6>[    6.054953] RPC: Registered udp transport module.
<6>[    6.055041] RPC: Registered tcp transport module.
<6>[    6.055126] RPC: Registered tcp NFSv4.1 backchannel transport module.
<7>[    6.055305] PCI: CLS 0 bytes, default 32
<6>[    6.055504] Unpacking initramfs...
<6>[    6.064141] Freeing initrd memory: 164K
<6>[    6.064302] PMU: registered new PMU device of type 0
<4>[    6.073481] highmem bounce pool size: 64 pages
<6>[    6.073780] ashmem: initialized
<6>[    6.074595] NTFS driver 2.1.30 [Flags: R/W].
<6>[    6.074743] fuse init (API version 7.16)
<6>[    6.075055] Registering char device tf_driver (122:0)
<6>[    6.078116] io scheduler noop registered (default)
<6>[    6.079599] tegra_grhost tegra_grhost: initialized
<6>[    6.280703] tegradc tegradc.0: probed
<6>[    6.281032] tegradc tegradc.0: probed
<6>[    6.286874] Wake4 for irq=303
<6>[    6.287368] nvhdcp: using "always on" policy.
<6>[    6.287699] tegradc tegradc.1: probed
<6>[    6.288187] tegradc tegradc.1: probed
<6>[    6.288551] Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
<6>[    6.289224] serial8250.0: ttyS0 at MMIO 0x70006300 (irq = 122) is a Tegra
<6>[    6.289544] tegra_uart.1: ttyHS1 at I/O 0x0 (irq = 69) is a unknown
<6>[    6.289796] Registered UART port ttyHS1
<6>[    6.289939] tegra_uart.2: ttyHS2 at I/O 0x0 (irq = 78) is a unknown
<6>[    6.290178] Registered UART port ttyHS2
<6>[    6.290339] Initialized tegra uart driver
<6>[    6.292421] loop: module loaded
<4>[    6.292678] nct1008 4-004c: Error [-19] ingetting the regulator handle for vdd of 4-004c
<6>[    6.294544] nct1008 4-004c: nct1008_probe: initialized
<4>[    6.294854] i2c-core: driver [nct1008] using legacy suspend method
<4>[    6.294945] i2c-core: driver [nct1008] using legacy resume method
<4>[    6.295159] bcm4329_rfkill_probe : can't find reset gpio.
<6>[    6.296119] input: lightsensor-level as /devices/virtual/input/input0
<6>[    6.296430] al3000a_probe: Light Sensor Driver Initialized
<4>[    6.296568] i2c-core: driver [al3000a_ls] using legacy suspend method
<4>[    6.296829] i2c-core: driver [al3000a_ls] using legacy resume method
<6>[    6.398980] input: kxtf9_accel as /devices/virtual/input/input1
<6>[    6.399129] kxtf9 4-000f: kxtf9_probe: Registered /dev/kxtf9
<6>[    6.399468] AKM8975 compass driver: initialize
<6>[    6.399903] input: compass as /devices/virtual/input/input2
<6>[    6.400308] PPP generic driver version 2.4.2
<6>[    6.400560] PPP Deflate Compression module registered
<6>[    6.400649] PPP BSD Compression module registered
<6>[    6.401342] PPP MPPE Compression module registered
<6>[    6.401435] NET: Registered protocol family 24
<6>[    6.401856] tun: Universal TUN/TAP device driver, 1.6
<6>[    6.402018] tun: (C) 1999-2004 Max Krasnyansky <maxk@qualcomm.com>
<6>[    6.402270] usbcore: registered new interface driver asix
<6>[    6.402466] usbcore: registered new interface driver cdc_ether
<6>[    6.402553] cdc_ncm: 01-June-2011-mbm-1
<6>[    6.402737] usbcore: registered new interface driver cdc_ncm
<6>[    6.402851] usbcore: registered new interface driver cdc_subset
<6>[    6.403051] usbcore: registered new interface driver sierra_net
<6>[    6.403143] ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
<6>[    6.436338] tegra-ehci tegra-ehci.1: Tegra EHCI Host Controller
<6>[    6.436506] tegra-ehci tegra-ehci.1: new USB bus registered, assigned bus number 1
<6>[    6.505212] tegra-ehci tegra-ehci.1: irq 53, io mem 0xc5004000
<6>[    6.525200] tegra-ehci tegra-ehci.1: USB 2.0 started, EHCI 1.00
<6>[    6.525343] usb usb1: New USB device found, idVendor=1d6b, idProduct=0002
<6>[    6.525504] usb usb1: New USB device strings: Mfr=3, Product=2, SerialNumber=1
<6>[    6.525593] usb usb1: Product: Tegra EHCI Host Controller
<6>[    6.525751] usb usb1: Manufacturer: Linux 2.6.39.4-g40c7636-dirty ehci_hcd
<6>[    6.525839] usb usb1: SerialNumber: tegra-ehci.1
<6>[    6.526276] hub 1-0:1.0: USB hub found
<6>[    6.526374] hub 1-0:1.0: 1 port detected
<6>[    6.529345] tegra-ehci tegra-ehci.2: Tegra EHCI Host Controller
<6>[    6.529465] tegra-ehci tegra-ehci.2: new USB bus registered, assigned bus number 2
<6>[    6.605209] tegra-ehci tegra-ehci.2: irq 129, io mem 0xc5008000
<6>[    6.625229] tegra-ehci tegra-ehci.2: USB 2.0 started, EHCI 1.00
<6>[    6.625428] usb usb2: New USB device found, idVendor=1d6b, idProduct=0002
<6>[    6.625521] usb usb2: New USB device strings: Mfr=3, Product=2, SerialNumber=1
<6>[    6.625678] usb usb2: Product: Tegra EHCI Host Controller
<6>[    6.625833] usb usb2: Manufacturer: Linux 2.6.39.4-g40c7636-dirty ehci_hcd
<6>[    6.625921] usb usb2: SerialNumber: tegra-ehci.2
<6>[    6.626351] hub 2-0:1.0: USB hub found
<6>[    6.626447] hub 2-0:1.0: 1 port detected
<6>[    6.626817] usbcore: registered new interface driver cdc_acm
<6>[    6.626979] cdc_acm: v0.26-mbm_2:USB Abstract Control Model driver for USB modems and ISDN adapters
<6>[    6.627170] usbcore: registered new interface driver cdc_wdm
<6>[    6.627256] Initializing USB Mass Storage driver...
<6>[    6.627403] usbcore: registered new interface driver usb-storage
<6>[    6.627560] USB Mass Storage support registered.
<6>[    6.627708] usbcore: registered new interface driver libusual
<6>[    6.627955] usbcore: registered new interface driver usbserial
<6>[    6.628042] usbserial: USB Serial Driver core
<6>[    6.628222] USB Serial support registered for GSM modem (1-port)
<6>[    6.628386] usbcore: registered new interface driver option
<6>[    6.628542] option: v0.7.2:USB Driver for GSM modems
<6>[    6.628651] USB Serial support registered for pl2303
<6>[    6.628850] usbcore: registered new interface driver pl2303
<6>[    6.628938] pl2303: Prolific PL2303 USB to serial adaptor driver
<6>[    6.629119] USB Serial support registered for Sierra USB modem
<6>[    6.629247] usbcore: registered new interface driver sierra
<6>[    6.629405] sierra: v.1.7.42_android_generic_2:USB Driver for Sierra Wireless USB modems
<6>[    6.629493] NVidia Tegra High-Speed USB SOC Device Controller driver (Apr 20, 2007)
<3>[    6.635049] fsl-tegra-udc fsl-tegra-udc: can't get charge regulator,err:-19
<6>[    6.635275] tegra-otg tegra-otg: SUSPEND --> PERIPHERAL
<6>[    6.641330] android_usb gadget: Mass Storage Function, version: 2009/09/11
<6>[    6.641494] android_usb gadget: Number of LUNs=1
<6>[    6.641581]  lun0: LUN: removable file: (no medium)
<4>[    6.641859] Gadget Android: controller 'fsl-tegra-udc' not recognized
<6>[    6.641953] android_usb gadget: android_usb ready
<6>[    6.642107] fsl-tegra-udc: bind to driver android_usb
<7>[    6.642554] gpio_set_debounce: gpio-128 status -22
<7>[    6.642693] gpio_set_debounce: gpio-129 status -22
<7>[    6.642889] gpio_set_debounce: gpio-130 status -22
<7>[    6.643008] gpio_set_debounce: gpio-131 status -22
<7>[    6.643196] gpio_set_debounce: gpio-132 status -22
<7>[    6.643314] gpio_set_debounce: gpio-133 status -22
<7>[    6.643502] gpio_set_debounce: gpio-23 status -22
<6>[    6.643591] Wake8 for irq=215
<7>[    6.643708] gpio_set_debounce: gpio-67 status -22
<6>[    6.644030] input: gpio-keys as /devices/platform/gpio-keys.0/input/input3
<6>[    6.765878] using rtc device, tps6586x-rtc, for alarms
<6>[    6.766060] tps6586x-rtc tps6586x-rtc.0: rtc core: registered tps6586x-rtc as rtc0
<6>[    6.766672] i2c /dev entries driver
<6>[    6.767236] Linux video capture interface: v2.00
<6>[    6.767403] usbcore: registered new interface driver uvcvideo
<6>[    6.767562] USB Video Class driver (v1.0.0)
<6>[    6.767713] trpc_sema_init: registered misc dev 10:48
<6>[    6.767865] trpc_node_register: Adding 'local' to node list
<6>[    6.768174] tegra_avp_probe: allocated carveout memory at 2ff00000 for AVP kernel
<6>[    6.768412] trpc_node_register: Adding 'avp-remote' to node list
<6>[    6.768622] tegra_avp_probe: message area 227f6000/227f6110
<6>[    6.768971] tegra_camera tegra_camera: tegra_camera_probe
<6>[    6.769211] yuv sensor_init
<6>[    6.769323] yuv sensor_probe
<6>[    6.769575] yuv5 sensor_init
<6>[    6.769686] yuv5 sensor_probe
<7>[    6.769898] gpio_request: gpio-171 (gpio-charger.0) status -16
<3>[    6.770062] gpio-charger gpio-charger.0: Failed to request gpio pin: -16
<4>[    6.770156] gpio-charger: probe of gpio-charger.0 failed with error -16
<6>[    6.770372] Wake1 for irq=363
<4>[    6.775024] battery capacity: 99
<4>[    6.780251] battery capacity: 99
<6>[    6.845210] usb 1-1: new high speed USB device number 2 using tegra-ehci
<6>[    6.995700] EC_Battery 2-0058: EC_Battery: battery driver registered
<6>[    6.995891] usb 1-1: New USB device found, idVendor=0bdb, idProduct=1911
<6>[    6.995985] usb 1-1: New USB device strings: Mfr=1, Product=2, SerialNumber=3
<6>[    6.996147] usb 1-1: Product: F5521gw
<6>[    6.996235] usb 1-1: Manufacturer: Lenovo
<6>[    6.996397] usb 1-1: SerialNumber: 35ACA18704C53200
<4>[    6.996888] i2c-core: driver [EC_Battery] using legacy suspend method
<4>[    6.997055] i2c-core: driver [EC_Battery] using legacy resume method
<4>[    6.997151] create gpio node for Dock : /sys/devices/virtual/gpio/gpio151/value
<4>[    6.997368] create gpio node for Dock is finished !
<4>[    6.997494] i2c-core: driver [adt7461] using legacy suspend method
<4>[    6.997650] i2c-core: driver [adt7461] using legacy resume method
<6>[    6.997823] device-mapper: uevent: version 1.0.3
<6>[    6.998100] device-mapper: ioctl: 4.20.0-ioctl (2011-02-02) initialised: dm-devel@redhat.com
<6>[    6.998279] Bluetooth: HCI UART driver ver 2.2
<6>[    6.998366] Bluetooth: HCI H4 protocol initialized
<6>[    6.998451] Bluetooth: HCILL protocol initialized
<6>[    6.998604] Bluetooth: BlueSleep Mode Driver Ver 1.1
<6>[    6.998744] Wake7 for irq=358
<6>[    6.999194] cpuidle: using governor ladder
<6>[    6.999513] cpuidle: using governor menu
<6>[    6.999624] sdhci: Secure Digital Host Controller Interface driver
<6>[    6.999782] sdhci: Copyright(c) Pierre Ossman
<4>[    6.999958] mmc0: Invalid maximum block size, assuming 512 bytes
<6>[    7.000453] mmc0: SDHCI controller on platform [sdhci-tegra.3] using ADMA
<6>[    7.000626] Wake23 for irq=261
<6>[    7.000840] Wake23 for irq=261
<6>[    7.000926] Enabling wake23
<6>[    7.001013] gpio bank wake found: wake23 for irq=66
<6>[    7.001099] Enabling wake23
<3>[    7.001262] sdhci sdhci-tegra.2: vddio_sdmmc regulator not found: -19
<3>[    7.001354] sdhci sdhci-tegra.2: vddio_sd_slot regulator not found: -19
<4>[    7.001534] mmc1: Invalid maximum block size, assuming 512 bytes
<6>[    7.001632] mmc1: no vmmc regulator found
<6>[    7.001880] mmc1: SDHCI controller on platform [sdhci-tegra.2] using ADMA
<3>[    7.002006] sdhci sdhci-tegra.0: vddio_sdmmc regulator not found: -19
<3>[    7.002169] sdhci sdhci-tegra.0: vddio_sd_slot regulator not found: -19
<4>[    7.002273] mmc2: Invalid maximum block size, assuming 512 bytes
<6>[    7.002441] mmc2: no vmmc regulator found
<6>[    7.002625] mmc2: SDHCI controller on platform [sdhci-tegra.0] using ADMA
<6>[    7.003389] tegra-aes tegra-aes: registeredDispatcher Driver Version 2.01
<6>[    7.030573] cdc_acm 1-1:1.1: ttyACM0: USB ACM device
<6>[    7.039827] cdc_acm 1-1:1.3: ttyACM1: USB ACM device
<6>[    7.049243] cdc_wdm 1-1:1.5: cdc-wdm0: USB WDM device
<6>[    7.078843] usb 1-1: MAC-Address: 0x02:0x80:0x37:0xec:0x02:0x00
<6>[    7.086236] mmc0: new high speed MMC card at address 0001
<6>[    7.086658] mmcblk0: mmc0:0001 MBG8FA 29.8 GiB 
<4>[    7.106895] Primary GPT is invalid, using alternate GPT.
<6>[    7.107100]  mmcblk0: p1 p2 p3 p4 p5 p6 p7 p8 p9 p10
<6>[    7.135197] usb 1-1: wakeup: enabled
<6>[    7.135666] cdc_ncm 1-1:1.6: rmnet0: register 'cdc_ncm' at usb-tegra-ehci.1-1, CDC NCM, 02:80:37:ec:02:00
<6>[    7.136747] cdc_wdm 1-1:1.8: cdc-wdm1: USB WDM device
<6>[    7.137987] cdc_acm 1-1:1.9: ttyACM2: USB ACM device
<6>[    7.526972] mmc1: new high speed SD card at address 0007
<6>[    7.527424] mmcblk1: mmc1:0007 SD2GB 1.84 GiB 
<6>[    7.530582]  mmcblk1: p1
<6>[    7.625334] input: N-trig Touch as /devices/virtual/input/input4
<6>[    7.625705] input: N-trig Multi Touch as /devices/virtual/input/input5
<6>[    7.625930] Wake14 for irq=366
<6>[    7.628648] usbcore: registered new interface driver usbhid
<6>[    7.628737] usbhid: USB HID core driver
<6>[    7.629169] logger: created 256K log 'log_main'
<6>[    7.629317] logger: created 256K log 'log_events'
<6>[    7.629460] logger: created 256K log 'log_radio'
<6>[    7.629674] logger: created 256K log 'log_system'
<4>[    7.629885] Vibrator initialize
<4>[    7.630208] phj00_lcd: probing sensor.
<4>[    7.630532] psensor: Nothing approach
<6>[    7.630619] psensor probe success
<6>[    7.630738] psensor initial success
<6>[    7.630906] [phj00_diag] PHJ00 diag Driver Initialized
<6>[    7.632969] tegra-snd-wm8903 tegra-snd-wm8903.0: No speaker regulator found
<6>[    7.633134] tegra-snd-wm8903 tegra-snd-wm8903.0: No digital mic regulator found
<6>[    7.633674] wm8903 0-001a: WM8903 revision C
<6>[    7.818315] Wake12 for irq=370
<6>[    7.818528] Wake11 for irq=371
<6>[    7.820067] asoc: wm8903-hifi <-> tegra20-i2s.0 mapping ok
<6>[    7.820899] asoc: dit-hifi <-> tegra20-spdif mapping ok
<6>[    7.821664] asoc: dit-hifi <-> tegra20-i2s.1 mapping ok
<6>[    7.822441] input: tegra-wm8903 Mic Jack as /devices/platform/tegra-snd-wm8903.0/sound/card0/input6
<6>[    7.822785] input: tegra-wm8903 Headphone Jack as /devices/platform/tegra-snd-wm8903.0/sound/card0/input7
<6>[    7.823072] ALSA device list:
<6>[    7.823231]   #0: tegra-wm8903
<6>[    7.823507] oprofile: using arm/armv7-ca9
<6>[    7.823687] GACT probability NOT on
<6>[    7.823851] Mirror/redirect action on
<6>[    7.823938] u32 classifier
<6>[    7.824021]     Actions configured
<6>[    7.824108] Netfilter messages via NETLINK v0.30.
<6>[    7.824308] nf_conntrack version 0.5.0 (11432 buckets, 45728 max)
<6>[    7.824666] NF_TPROXY: Transparent proxy support initialized, version 4.1.0
<6>[    7.824830] NF_TPROXY: Copyright (c) 2006-2007 BalaBit IT Ltd.
<6>[    7.825113] xt_time: kernel timezone is -0000
<6>[    7.825442] ip_tables: (C) 2000-2006 Netfilter Core Team
<6>[    7.825675] arp_tables: (C) 2002 David S. Miller
<6>[    7.825874] TCP cubic registered
<6>[    7.826049] NET: Registered protocol family 10
<6>[    7.826888] Mobile IPv6
<6>[    7.827067] ip6_tables: (C) 2000-2006 Netfilter Core Team
<6>[    7.827270] IPv6 over IPv4 tunneling driver
<6>[    7.828110] NET: Registered protocol family 17
<6>[    7.828290] NET: Registered protocol family 15
<5>[    7.828427] Bridge firewalling registered
<6>[    7.828717] Bluetooth: RFCOMM TTY layer initialized
<6>[    7.828818] Bluetooth: RFCOMM socket layer initialized
<6>[    7.828905] Bluetooth: RFCOMM ver 1.11
<6>[    7.829064] Bluetooth: BNEP (Ethernet Emulation) ver 1.3
<6>[    7.829151] Bluetooth: HIDP (Human Interface Emulation) ver 1.2
<5>[    7.829387] Registering the dns_resolver key type
<6>[    7.829504] VFP support v0.3: implementor 41 architecture 3 part 30 variant 9 rev 1
<5>[    7.829801] Registering SWP/SWPB emulation handler
<4>[    7.834774] Disabling clocks left on by bootloader:
<4>[    7.834936]    audio
<4>[    7.835021]    stat_mon
<4>[    7.835125]    fuse_burn
<4>[    7.835255]    clk_d
<4>[    7.835415]    pll_p_out2
<6>[    7.835848] CPU rate: 1000 MHz
<3>[    7.835971] Error: Driver 'pm_irq_pm_ops' is already registered, aborting...
<3>[    7.838638] Tegra protected aperture disabled because nvmap is using system memory
<6>[    7.840937] GobiNet: 2011-07-29-1026/SWI_2.12_android
<6>[    7.841178] usbcore: registered new interface driver GobiNet
<6>[    7.841578] tps6586x-rtc tps6586x-rtc.0: setting system clock to 2020-03-05 20:25:27 UTC (1583439927)
<4>[    7.841819] Warning: unable to open an initial console.
<6>[    7.842017] Freeing init memory: 212K
<4>[    7.845289] init (1): /proc/1/oom_adj is deprecated, please use /proc/1/oom_score_adj instead.
<4>[    7.855424] battery capacity: 99
<3>[    8.128997] init: could not open /dev/keychord
<3>[    8.129130] init: console_init_action(): console open failed: -1, retrying...
<3>[    8.229383] init: console_init_action(): console open failed: -1, retrying...
<3>[    8.329629] init: console_init_action(): console open failed: -1, retrying...
<3>[    8.429803] init: console_init_action(): console open failed: -1, retrying...
<3>[    8.530047] init: console_init_action(): console open failed: -1, retrying...
<3>[    8.630221] init: console_init_action(): console open failed: -1, retrying...
<3>[    8.730464] init: console_init_action(): console open failed: -1, retrying...
<3>[    8.830710] init: console_init_action(): console open failed: -1, retrying...
<3>[    8.930886] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.031129] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.131381] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.231559] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.331805] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.432046] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.532221] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.632470] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.732647] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.832894] init: console_init_action(): console open failed: -1, retrying...
<3>[    9.933140] init: console_init_action(): console open failed: -1, retrying...
<3>[   10.033315] init: console_init_action(): console open failed: -1, retrying...
<6>[   10.775449] EXT4-fs (mmcblk0p3): mounted filesystem with ordered data mode. Opts: (null)
<7>[   15.528671] EXT4-fs (mmcblk0p10): ext4_orphan_cleanup: deleting unreferenced inode 1098610
<7>[   15.528996] EXT4-fs (mmcblk0p10): ext4_orphan_cleanup: deleting unreferenced inode 1099159
<6>[   15.529290] EXT4-fs (mmcblk0p10): 2 orphan inodes deleted
<6>[   15.529451] EXT4-fs (mmcblk0p10): recovery complete
<6>[   15.610390] EXT4-fs (mmcblk0p10): mounted filesystem with writeback data mode. Opts: journal_async_commit,data=writeback,nodelalloc,errors=panic
<6>[   15.751100] EXT4-fs (mmcblk0p4): recovery complete
<6>[   15.761095] EXT4-fs (mmcblk0p4): mounted filesystem with writeback data mode. Opts: journal_async_commit,data=writeback,nodelalloc,errors=panic
<6>[   15.851305] EXT4-fs (mmcblk0p6): recovery complete
<6>[   15.875623] EXT4-fs (mmcblk0p6): mounted filesystem with writeback data mode. Opts: journal_async_commit,data=writeback,nodelalloc,errors=panic
<6>[   15.926997] EXT4-fs (mmcblk0p9): recovery complete
<6>[   15.927166] EXT4-fs (mmcblk0p9): mounted filesystem with writeback data mode. Opts: journal_async_commit,data=writeback,nodelalloc,errors=panic
<3>[   16.227777] init: cannot find '/system/bin/setup_fs', disabling 'setup_fs'
<3>[   16.228307] init: cannot find '/system/bin/rdd', disabling 'rdd'
<3>[   16.230243] init: cannot find '/system/etc/install-recovery.sh', disabling 'flash_recovery'
<6>[   16.231400] adb_bind_config
<6>[   16.233338] warning: `adbd' uses 32-bit capabilities (legacy support in use)
<6>[   16.233699] adb_open(adbd)
<4>[   16.339612] [USB]wake lock 0
<6>[   16.339717] android_work: did not send uevent (0 0   (null))
<3>[   16.347400] init: service 'console' requires console
<3>[   16.347570] init: cannot find '/system/bin/wwanloader', disabling 'wwanloader'
<6>[   16.348659] adb_release
<6>[   16.348795] mtp_bind_config
<6>[   16.348939] adb_open(adbd)
<6>[   16.349858] adb_bind_config
<4>[   16.350445] [USB]wake lock 0
<6>[   16.350539] android_work: did not send uevent (0 0   (null))
<4>[   16.447711] akmd_open
<4>[   16.466474] [USB]wake lock 1
<6>[   16.466975] android_work: sent uevent USB_STATE=CONNECTED
<6>[   16.486056] android_usb gadget: high speed config #1: android
<4>[   16.486534] [USB]wake lock 1
<6>[   16.486791] android_work: sent uevent USB_STATE=CONFIGURED
<4>[   16.550448] ttyACM1: Entering acm_tty_open.
<6>[   16.558235] usb 1-1: disconnected from network
<6>[   21.555374] AL3000A Light sensor enabled --->
<6>[   21.707114] AL3000A Light sensor disabled --->
<4>[   22.945814] extern1 rate=12000000
<4>[   36.570495] battery capacity: 99
<6>[   36.913763] tegra_uart tegra_uart.2: Setting clk_src pll_p
<6>[   36.936083] tegra_uart tegra_uart.2: Setting clk_src pll_p
<6>[   36.996129] tegra_uart tegra_uart.2: Setting clk_src pll_p
<4>[   37.053641] battery capacity: 99
<4>[   37.107456] wifi_set_power = 1
<4>[   37.277090] battery capacity: 99
<4>[   37.412860] wifi_set_carddetect = 1
<4>[   37.414466] 
<4>[   37.414471] Dongle Host Driver, version 4.218.248.23
<6>[   37.454189] mmc2: new high speed SDIO card at address 0001
<4>[   37.458624] DHD: dongle ram size is set to 294912(orig 294912)
<6>[   37.494467] acc_open
<6>[   37.494576] acc_release
<4>[   37.607968] Firmware version = wl0: Feb 23 2011 14:28:20 version 4.218.248.25
<4>[   37.774868] wlan0: Broadcom Dongle Host Driver mac=e0:b9:a5:ef:57:0b
<4>[   38.249643] wl_iw_set_cscan Ignoring CSCAN : First Scan is not done yet 1
<4>[   38.721101] STA connect received 1
<6>[   38.742306] tegra_uart tegra_uart.2: Setting clk_src pll_m
<6>[   38.799341] Wake7 for irq=358
<6>[   38.799463] Enabling wake7
<6>[   38.799573] gpio bank wake found: wake7 for irq=119
<6>[   38.799777] Enabling wake7
<3>[   39.199062] cdc_wdm 1-1:1.8: wdm_int_callback - 0 bytes
<3>[   39.232048] cdc_wdm 1-1:1.5: wdm_int_callback - 0 bytes
<6>[   40.439766] mtp_open
<3>[   43.191686] init: sys_prop: permission denied uid:1003  name:service.bootanim.exit
<7>[   49.105220] wlan0: no IPv6 routers present
<4>[   67.130281] battery capacity: 99
<4>[   67.347903] battery capacity: 99
<4>[   97.210318] battery capacity: 99
<4>[   97.428069] battery capacity: 99
<6>[  102.099976] binder: release 467:467 transaction 33359 out, still active
<6>[  102.100205] binder: release 467:1446 transaction 33221 in, still active
<6>[  102.100304] binder: send failed reply for transaction 33221 to 603:788
<6>[  102.947304] binder: 268:581 transaction failed 29189, size 352-0
<4>[  127.289812] battery capacity: 99
<4>[  127.507924] battery capacity: 99
shell@android:/ # 
shell@android:/ # cd /storage/sdcard1                                          
shell@android:/storage/sdcard1 # insmod testk.ko
```

Merde, the tablet restarted after the insmod command.

# Try to find SBK with *jevinskie/fusee-launcher*
Download **fusee-launcher** from https://github.com/jevinskie/fusee-launcher

Compile it with **make** command in Linux

Package **python3-pyusb** is necessary

Let tablet enter **APX** mode, confirm it with command **lsusb**
It should be recognized as:

`Bus 003 Device 004: ID 0955:7820 NVIDIA Corp. T20 [Tegra 2] recovery mode`

As there is no USB3 on my laptop, it doesn't work.

I'll try again with another laptop.

Try again with Raspberry Pi 4, with USB3.
command:

`sudo ./fusee-launcher.py -V 0955 -P 7820 --tty dump-sbk-via-usb.bin`

Output : 

```
2021-01-05 22:07:40,426 INFO:usb.core:find(): using backend "usb.backend.libusb1"

Important note: on desktop Linux systems, we currently require an XHCI host controller.
A good way to ensure you're likely using an XHCI backend is to plug your
device into a blue 'USB 3' port.

Identified a Linux system; setting up the appropriate backend.
intermezzo_size: 0x00000078
target_payload_size: 0x00011590
Found a Tegra with Device ID: b'd7309f434630840a'
Traceback (most recent call last):
  File "./fusee-launcher.py", line 720, in <module>
    patched_intermezzo = switch.get_patched_intermezzo(intermezzo, target_payload_size)
  File "./fusee-launcher.py", line 639, in get_patched_intermezzo
    overwrite_len = self.get_overwrite_length()
  File "./fusee-launcher.py", line 593, in get_overwrite_length
    stack_snapshot = self.read_stack()
  File "./fusee-launcher.py", line 588, in read_stack
    return self.backend.read_ep0(0x10)
  File "./fusee-launcher.py", line 140, in read_ep0
    return bytes(self.dev.ctrl_transfer(self.STANDARD_REQUEST_DEVICE_TO_HOST_TO_ENDPOINT, self.GET_STATUS, 0, 0, length))
  File "/usr/local/lib/python3.7/dist-packages/usb/core.py", line 1077, in ctrl_transfer
    self.__get_timeout(timeout))
  File "/usr/local/lib/python3.7/dist-packages/usb/_debug.py", line 62, in do_trace
    return f(*args, **named_args)
  File "/usr/local/lib/python3.7/dist-packages/usb/backend/libusb1.py", line 901, in ctrl_transfer
    timeout))
  File "/usr/local/lib/python3.7/dist-packages/usb/backend/libusb1.py", line 602, in _check
    raise USBTimeoutError(_strerror(ret), ret, _libusb_errno[ret])
usb.core.USBTimeoutError: [Errno 110] Operation timed out
```

## Fuse and 
SBK : 128 bits
Device Key (DK): 32 bits
Unique ID (UID): 64 bits


# tegrarcm
[tegrarcm on Github](https://github.com/NVIDIA/tegrarcm)




