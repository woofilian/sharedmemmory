#!/bin/sh
#====================================================#
#| Security |  0   |  1   |  2     |    3           |#
#|----------|------|------|--------|----------------|#
#| Authmode | OPEN | OPEN | SHARED | WPAPSK/WPA2PSK |#
#|Encryption| NONE | WEP  | WEP    | TKIP/AES       |#
#====================================================#

security=$1
myssid=$2
mypsk=$3
NWID=`wpa_cli -iwlan0 add_network`
echo "Security=${security}, SSID=${myssid}, PSK=${mypsk}, network_id=${NWID}"

wpa_cli -iwlan0 set_network ${NWID} scan_ssid 1
wpa_cli -iwlan0 set_network ${NWID} ssid '"@SSID@"'

# security = 0 // NONE/OPEN
if [ "$security" = "0" ]; then
	wpa_cli -iwlan0 set_network ${NWID} key_mgmt NONE
fi

# security = 1 // WEP/OPEN
if [ "$security" = "1" ]; then
	wpa_cli -iwlan0 set_network ${NWID} key_mgmt NONE
	wpa_cli -iwlan0 set_network ${NWID} wep_key0 '"@PWD@"'
	wpa_cli -iwlan0 set_network ${NWID} wep_tx_keyidx 0
fi

# security = 2 // WEP/SHARED
if [ "$security" = "2" ]; then
	wpa_cli -iwlan0 set_network ${NWID} key_mgmt NONE
	wpa_cli -iwlan0 set_network ${NWID} auth_alg SHARED
	wpa_cli -iwlan0 set_network ${NWID} wep_key0 '"@PWD@"'
	wpa_cli -iwlan0 set_network ${NWID} wep_tx_keyidx 0
fi

# security = 3 // WPA
if [ "$security" = "3" ]; then
	wpa_cli -iwlan0 set_network ${NWID} key_mgmt WPA-PSK
	wpa_cli -iwlan0 set_network ${NWID} psk '"@PWD@"'
fi

wpa_cli -iwlan0 enable_network ${NWID}
wpa_cli -iwlan0 select_network ${NWID}
