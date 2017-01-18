##
## Run Station Mode:
## kill hostapd and udhcpd, then run wpa_supplicant
##
if [ -f "/var/run/hostapd.pid" ]; then
	pid=`cat /var/run/hostapd.pid`
	kill $pid
fi

if [ -f "/var/run/udhcpd.pid" ]; then
	pid=`cat /var/run/udhcpd.pid`
	kill $pid
fi

ifconfig wlan0 down
ifconfig wlan0 up
ifconfig wlan0 up 172.16.1.18 netmask 255.255.255.0 up

echo "ctrl_interface=/var/run/wpa_supplicant" > /etc/wpa_supplicant.conf
echo "ap_scan=1" >> /etc/wpa_supplicant.conf

wpa_supplicant -Dwext -iwlan0 -c/etc/wpa_supplicant.conf -B -P/var/run/wpa_supplicant.pid &


#udhcpc -i wlan0 -t 15  -p /tmp/udhcpc_wlan0_pid.txt &
