#!/bin/sh

scene_mode=$1
if [ "$scene_mode" = "day" ]; then
	echo "$1" > /tmp/scene_mode
	echo 1 2 1 2 8 2 1 2 1 > /proc/isp328/ae/win_weight
	echo 0 > /proc/isp328/ae/hi_light_supp
	echo w reload_cfg /mnt/mtd/isp328_sc1135_day.cfg > /proc/isp328/command
fi

if [ "$scene_mode" = "night" ]; then
	echo "$1" > /tmp/scene_mode 
	echo 1 2 1 2 8 2 1 2 1 > /proc/isp328/ae/win_weight
	echo 5 > /proc/isp328/ae/hi_light_supp
	echo w reload_cfg /mnt/mtd/isp328_sc1135_night.cfg > /proc/isp328/command
fi
