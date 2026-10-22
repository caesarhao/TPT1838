# Download ubuntu base for RootFS.
# wget http://cdimage.ubuntu.com/ubuntu-base/releases/18.04/release/ubuntu-base-18.04.5-base-armhf.tar.gz
wget http://cdimage.ubuntu.com/ubuntu-base/releases/26.10/release/ubuntu-base-26.10-base-armhf.tar.gz

# Extracting and Preparing the RootFS
# 1. Mount your SD card ext4 partition (e.g., /media/user/rootfs)
# 2. Extract the archive (replace with your downloaded file name)
sudo tar -xvpzf ubuntu-base-26.10-base-armhf.tar.gz -C /media/user/rootfs/
# 3. Ensure all changes are physically written to the card before unplugging
sync

# Install LXDE on Ubuntu Base
## Install on the tablet
# Update your package repositories
sudo apt update

# Install LXDE (Lightest and fastest)
sudo apt install lxde xorg lightdm

## Install on the SD Card
# 1. Install QEMU static emulator so your PC can run ARM instructions
sudo apt install qemu-user-static

# 2. Copy the emulator into your tablet's RootFS
sudo cp /usr/bin/qemu-arm-static /media/user/rootfs/usr/bin/

# 3. Copy your DNS settings so the tablet filesystem can access the internet
sudo cp /etc/resolv.conf /media/user/rootfs/etc/resolv.conf

# 4. Mount system virtual directories and log into the tablet filesystem
sudo mount --bind /dev /media/user/rootfs/dev
sudo mount --bind /sys /media/user/rootfs/sys
sudo mount --bind /proc /media/user/rootfs/proc
sudo chroot /media/user/rootfs

# 5. You are now virtually inside your tablet's operating system!
apt update
# 6. Create user "thinkpad"
adduser thinkpad

# 7. Add user to sudo group
usermod -aG sudo thinkpad

# 8. Install the core graphics engine and LXDE desktop
apt install -y xorg lxde lightdm

# 9. Install Input Packages for N-Trig Stylus Pen
apt install xserver-xorg-input-wacom xinput xinput_calibrator
type /etc/X11/xorg.conf.d/50-thinkpad-pen.conf
Insert content :
Section "InputClass"
    Identifier "Wacom N-Trig class"
    MatchProduct "N-Trig Pen|N-Trig DuoSense"
    MatchDevicePath "/dev/input/event*"
    Driver "wacom"
    Option "Button2" "3"   # Maps the second button on the barrel to act as a Right Click
EndSection

# 9.1 Calibrate the touchscreen
xinput_calibrator
nano /etc/X11/xorg.conf.d/99-calibration.conf
Section "InputClass"
    Identifier "calibration"
    MatchProduct "N-Trig DuoSense"
    Option "Calibration" "YOUR_MIN_X YOUR_MAX_X YOUR_MIN_Y YOUR_MAX_Y"
EndSection

# 9.2 Restart login manager
sudo systemctl restart lightdm

# 10. (Optional) Install a lightweight browser and network manager
apt install -y network-manager wicd-cli leafpad



# 1. Exit the virtual tablet filesystem
exit

# 2. Cleanly unmount the system folders
sudo umount /media/user/rootfs/dev
sudo umount /media/user/rootfs/sys
sudo umount /media/user/rootfs/proc

# 3. Flush written data to the card
sync

