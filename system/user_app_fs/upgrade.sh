Linux_img=/mnt/nfs/uImage_8136
RootFs_img=/mnt/nfs/rootfs-cpio.squashfs.img
CFG_img=/mnt/nfs/user_cfg.jffs2.img
APP_img=/mnt/nfs/user_app.jffs2.img

if [ "$1" = "" ]; then
	echo "Usage:"
	echo "Upgrade linux"
	echo "	$0 linux"
	echo ""
	echo "Upgrade filesystem:"
	echo "	$0 fs"
	echo ""
	echo "Upgarde user_cfg"
	echo "	$0 cfg"
	echo ""
	echo "Upgarde user_app"
	echo "	$0 app"
	exit
fi

if [ "$1" = "linux" ]; then
	if [ ! -f "$Linux_img" ]; then
		mount -t nfs -o nolock,rw 192.168.10.10:/tftpboot/share /mnt/nfs
		if [ ! -f "$Linux_img" ]; then
			echo "$Linux_img is not esixt"
			exit
		fi
	fi
	flash_eraseall /dev/mtd1
	flashcp $Linux_img /dev/mtd1
	sync
fi

if [ "$1" = "fs" ]; then
	if [ ! -f "$RootFs_img" ]; then
		mount -t nfs -o nolock,rw 192.168.10.10:/tftpboot/share /mnt/nfs
		if [ ! -f "$RootFs_img" ]; then
			echo "$RootFs_img is not esixt"
			exit
		fi
	fi
	cp /gm/tools/flashcp /var
	cp /gm/bin/flash_eraseall /var
	cp /gm/bin/busybox /var
	/var/flash_eraseall /dev/mtd2
	/var/flashcp $RootFs_img /dev/mtd2
	/var/busybox sync
fi

if [ "$1" = "cfg" ]; then
	if [ ! -f "$CFG_img" ]; then
		mount -t nfs -o nolock,rw 192.168.10.10:/tftpboot/share /mnt/nfs
		if [ ! -f "$CFG_img" ]; then
			echo "$CFG_img is not esixt"
			exit
		fi
	fi
	umount /config
	flash_eraseall /dev/mtd3
	flashcp $CFG_img /dev/mtd3
	sync
fi

if [ "$1" = "app" ]; then
	if [ ! -f "$APP_img" ]; then
		mount -t nfs -o nolock,rw 192.168.10.10:/tftpboot/share /mnt/nfs
		if [ ! -f "$APP_img" ]; then
			echo "$APP_img is not esixt"
			exit
		fi
	fi
	umount /mnt/mtd
	flash_eraseall /dev/mtd4
	flashcp $APP_img /dev/mtd4
	sync
fi
