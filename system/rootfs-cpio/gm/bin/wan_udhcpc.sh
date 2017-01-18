ifname=$1

pid_file=/var/run/udhcpc_"$ifname"_pid.txt

if [ -f "$pid_file" ]; then
	PID=`cat $pid_file`
	kill $PID
	kill -9 $PID
fi
sleep 2
hostname="JA-Module"
udhcpc -i "$ifname" -H "$hostname" -b -p "$pid_file" &
