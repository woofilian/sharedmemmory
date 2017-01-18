#!/bin/sh
echo "umount......"
[ -d /var/log ] && [ -f /var/log/has_sd_card ] && rm -f /var/log/has_sd_card
sync
umount -lf /mnt/sd_card
