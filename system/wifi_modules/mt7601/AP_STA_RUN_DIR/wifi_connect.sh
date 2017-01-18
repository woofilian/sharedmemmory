#!/bin/sh
#if [ "'lsmod | grep  mt7601Uap'" != "" ];then
value=$(lsmod | grep mt7601Uap)
if [[ "x${value}" != "x" ]]; then
/mnt/mtd/run/wifi_ap_uinit.sh
fi

ifconfig eth0 down
usleep 500000

value=$(lsmod | grep mtutil7601Usta)
if [[ "x${value}" = "x" ]]; then
    /sbin/insmod /mnt/mtd/grain/ko/mtutil7601Usta.ko
#usleep 200000
fi

value=$(lsmod | grep mt7601Usta) 
if [[ "x${value}" = "x" ]]; then
    /sbin/insmod /mnt/mtd/grain/ko/mt7601Usta.ko
#usleep 200000
fi

value=$(lsmod | grep mtnet7601Usta)
if [[ "x${value}" = "x" ]]; then
    /sbin/insmod /mnt/mtd/grain/ko/mtnet7601Usta.ko
#usleep 100000
fi
mdev -s
ifconfig ra0 172.16.1.3 netmask 255.255.255.0 up
sleep 1
echo 1 > /proc/sys/vm/drop_caches

#uint8_t encrypt_type;				//加密类型0:OPEN 1:SHARED 2:WPAPSK 3:WPA2PSK 
#uint8_t auth_mode;					//认证方式0:NONE  1:EAP 2:PSK 3:OPEN 4:SHARED
#uint8_t secret_key_type;			//密钥管理方式0:NONE 1:WEP 2:TKIP 3:AES */TKIP只对应于加密类型为WPA/WPA2的情况
    


iwpriv ra0 set NetworkType=Infra

# MODE : OPEN
# ----------------------------------------------------------------
if [[ "$1" = "0" ]]; then
    iwpriv ra0 set AuthMode=OPEN
    iwpriv ra0 set EncrypType=NONE
    iwpriv ra0 set SSID=$$_SSID_$$

# MODE : WEP
# ----------------------------------------------------------------
# process WEP IN CODE!
# MODE : WPAPSK_TKIP
# ----------------------------------------------------------------
elif [[ "$1" = "2"&&"$2" = "2" ]]; then

    iwpriv ra0 set AuthMode=WPAPSK
    iwpriv ra0 set EncrypType=TKIP
    iwpriv ra0 set SSID=$$_SSID_$$
    iwpriv ra0 set WPAPSK=$$_PSK_$$

# MODE : WPAPSK_AES
# ----------------------------------------------------------------
elif [[ "$1" = "2" && "$2" = "3" ]]; then

    iwpriv ra0 set AuthMode=WPAPSK
    iwpriv ra0 set EncrypType=AES
    iwpriv ra0 set SSID=$$_SSID_$$
    iwpriv ra0 set WPAPSK=$$_PSK_$$

# MODE : WPA2PSK_TKIP
# ----------------------------------------------------------------
elif [[ "$1" = "3" && "$2" = "2" ]]; then

    iwpriv ra0 set AuthMode=WPA2PSK
    iwpriv ra0 set EncrypType=TKIP
    iwpriv ra0 set SSID=$$_SSID_$$
    iwpriv ra0 set WPAPSK=$$_PSK_$$

# MODE : WPA2PSK_AES
# ----------------------------------------------------------------
elif [[ "$1" = "3" && "$2" = "3" ]]; then

    iwpriv ra0 set AuthMode=WPA2PSK
    iwpriv ra0 set EncrypType=AES
    iwpriv ra0 set SSID=$$_SSID_$$
    iwpriv ra0 set WPAPSK=$$_PSK_$$

fi

iwpriv ra0 radio_on



iwpriv ra0 connStatus

#udhcpc -i ra0 -b -s /etc/dhcp/default.script &
