#!/bin/sh

mdev -s
#iifconfig wlan0 172.16.1.3 netmask 255.255.255.0 up
#sleep 10
echo 1 > /proc/sys/vm/drop_caches


# 0:init   1:start   2:stop   3:result   4:uinit  
# smart_config  init
# ----------------------------------------------------------------
if [[ "$1" = "0" ]]; then
	cd /mnt/mtd/run/
	cp RT2870STA_7601.dat /etc/Wireless/RT2870STA/
	insmod /mnt/mtd/grain/ko/mt7601Usta.ko
	ifconfig  wlan0 up

# smart_config  start
# ----------------------------------------------------------------
elif [[ "$1" = "1" ]]; then
 	iwpriv wlan0 elian start		


# smart_config  stop
# ----------------------------------------------------------------
elif [[ "$1" = "2" ]]; then
	iwpriv wlan0 elian clear
	iwpriv wlan0 elian stop
					

# smart_config  result
# ----------------------------------------------------------------
elif [[ "$1" = "3" ]]; then
	iwpriv wlan0 elian result >/tmp/wifi_info.sh
	

# smart_config  uinit                                             
# ----------------------------------------------------------------
elif [[ "$1" = "4" ]]; then                         
	ifconfig wlan0  down
	rmmod mt7601Usta							
fi
