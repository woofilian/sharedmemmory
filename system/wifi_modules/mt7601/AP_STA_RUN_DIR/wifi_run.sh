#!/bin/sh
/sbin/insmod /mnt/mtd/grain/ko/mtutil7601Usta.ko
usleep 500000
/sbin/insmod /mnt/mtd/grain/ko/mt7601Usta.ko
usleep 500000
/sbin/insmod /mnt/mtd/grain/ko/mtnet7601Usta.ko
usleep 500000
ifconfig ra0 up
sleep 1
iwpriv ra0 set NetworkType=Infra
iwpriv ra0 set AuthMode=WPA2PSK
iwpriv ra0 set EncrypType=AES
iwpriv ra0 set SSID=JOOAN-Office
iwpriv ra0 set WPAPSK=jooan@office
iwpriv ra0 radio_on
sleep 1
iwpriv ra0 connStatus 

/sbin/udhcpc -i  ra0 -b -s   /etc/dhcp/default.script

sleep 30

ifconfig ra0  down
sleep 1
rmmod mtnet7601Usta
usleep 500000
rmmod mt7601Usta
usleep 500000
rmmod mtutil7601Usta
usleep 500000








ifconfig ra0  down
rmmod mtnet7601Usta
rmmod mt7601Usta
rmmod mtutil7601Usta
