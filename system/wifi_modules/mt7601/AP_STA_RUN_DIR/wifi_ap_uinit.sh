#!/bin/sh
killall -9 udhcpd
ifconfig ra0  down
sleep 1
ifconfig ra0  down
sleep 1
rmmod mt7601Uap
sync
sleep 5


