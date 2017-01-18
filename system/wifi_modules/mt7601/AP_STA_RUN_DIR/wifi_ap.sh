#!/bin/sh

#/sbin/insmod /mnt/mtd/grain/ko/mt7601Uap.ko
#usleep 300000
#ifconfig ra0 up

#iwpriv ra0 set SSID=$$_SSID_$$


#cp /mnt/mtd/run/udhcpd7601 /etc/udhcpd.conf
#ifconfig ra0 192.168.1.1 up
#sleep 1
#iwpriv ra0 set SSID=AUSDOM_123456791111111111

#iwpriv ra0 set SSID=$$_SSID_$$
#iwpriv ra0 set WPAPSK=12345678

udhcpd -fS /etc/udhcpd.conf& 

