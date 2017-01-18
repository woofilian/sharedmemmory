#!/bin/bash

PWD=`pwd`
OUTPUT_IMG=$PWD/output/images/

USER_CFG_DIR=user_cfg_fs
USER_CFG_IMG=user_cfg.jffs2.img


# build version with date
DATE=$(date +%Y%m%d)
MAIN_VER=002
if [ "$1" != "" ]; then
	Descp=$1
else
	Descp=Config
fi
CFG_Version="$MAIN_VER"-"$DATE"-"$Descp"
echo $CFG_Version > $USER_CFG_DIR/ver_cfg

echo "------------build user cfg [Version: $CFG_Version]--------------"
rm -f $OUTPUT_IMG/$USER_CFG_IMG
./utils/mkfs.jffs2 -e 0x10000 -n -d $USER_CFG_DIR -o $USER_CFG_IMG --pad=0x100000
#./utils/mkfs.jffs2 -e 0x10000 -n -d user_cfg_fs -o $USER_CFG_IMG

cp -f $USER_CFG_IMG /tftpboot/share
mv $USER_CFG_IMG $OUTPUT_IMG
