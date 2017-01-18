#!/bin/sh
echo "mount..."
mount -t vfat /dev/mmcblk0p1 /mnt/sd_card
mount -t exfat /dev/mmcblk0p1 /mnt/sd_card
[ -d /var/log ] || mkdir /var/log
echo "" > /var/log/has_sd_card
