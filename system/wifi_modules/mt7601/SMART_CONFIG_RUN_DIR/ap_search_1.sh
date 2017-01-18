#!/bin/sh

/sbin/insmod /mnt/mtd/grain/ko/mtutil7601Usta.ko
#usleep 200000
/sbin/insmod /mnt/mtd/grain/ko/mt7601Usta.ko
#usleep 200000
/sbin/insmod /mnt/mtd/grain/ko/mtnet7601Usta.ko
#usleep 200000
ifconfig ra0 up
sleep 2
iwlist ra0 scanning >/mnt/mtd/run/wifilist.sh
sync
