# !/bin/sh

MOUNT=/bin/mount
UMOUNT=/bin/umount


domount(){
	M=`mount | grep $1`
	if [ "$M" = "" ]; then
		$MOUNT -t vfat /dev/$1 /mnt/sd_card
		echo "sd card has mount..............."
	else
		$UMOUNT -lf /mnt/sd_card
		echo "umount..."
	fi
}

# do not mount here

#case $1 in
#	"mmcblk0")
#	domount $1
#	;;
#	"mmcblk0p1")
#	domount $1
#	;;
#	*)
#	exit 1
#esac
