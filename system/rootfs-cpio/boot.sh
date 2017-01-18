#Transfer vg_boot.sh spec.cfg param.cfg from master to slave site (/mnt/mtd)
boot_actor=single

echo "if you want run app by youself,please press 'q'"
read -t 2 -p "   Press q -> ENTER to exit boot procedure? " exit_boot
if [ "$exit_boot" == "q" ] ; then
	exit
fi

# create udhcpd.leases
mkdir -p /var/lib/misc
touch /var/lib/misc/udhcpd.leases

# cp udhcpc default script to default path
mkdir -p /usr/share/udhcpc
cp -f /gm/etc_src/default.script /usr/share/udhcpc/

if [ -e /mnt/mtd/startapp ] ; then
	sh /mnt/mtd/startapp
	echo "================ startapp is ok ====================";
else
	echo "================ cannot find startapp ,now run GoAhead only......";
	ethtool eth0
	linked=`ethtool eth0 | grep Link | grep yes`
	if [ "$linked" != "" ]; then
		echo "detect ethernet..."
		ifconfig eth0 172.16.1.100 netmask 255.255.255.0 up
		wan_udhcpc.sh eth0
	fi
	telnetd &
	only_goAhead.sh &
fi

