
# insmod wifi driver [RTL8188]
insmod /lib/modules/8188eu.ko
echo 3 > /proc/net/rtl8188eu/log_level

ifconfig wlan0 up
MacAddr=`ifconfig wlan0 | grep HWaddr | awk -F" " '{print $5}'`

Byte5=`echo $MacAddr | cut -d ':' -f 5`
Byte6=`echo $MacAddr | cut -d ':' -f 6`

SSID="JooanIpc-""$Byte5""$Byte6"

echo "ctrl_interface=/var/run/hostapd" > /etc/hostapd.conf
echo "interface=wlan0" >> /etc/hostapd.conf
echo "ssid=$SSID" >> /etc/hostapd.conf
echo "channel=6" >> /etc/hostapd.conf
echo "hw_mode=g" >> /etc/hostapd.conf
echo "driver=nl80211" >> /etc/hostapd.conf

ifconfig wlan0 down
ifconfig wlan0 up
ifconfig wlan0 10.68.68.22 netmask 255.255.255.0

hostapd /etc/hostapd.conf -B -P /var/run/hostapd.pid &

killall udhcpd
udhcpd -S /etc/udhcpd.conf &

GoAhead &
extract_webs.sh
sleep 2
