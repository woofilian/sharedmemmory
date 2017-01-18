#!/bin/sh
ifconfig ra0  down
sleep 1
rmmod mtnet7601Usta
usleep 500000
rmmod mt7601Usta
usleep 500000
rmmod mtutil7601Usta
usleep 500000
