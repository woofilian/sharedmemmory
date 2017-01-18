#!/bin/bash

# for GM8136S/GM8135S, set PLATFORMName = GM8136
PLATFORMName=GM8136
NSBootName=./images/nsboot.bin
uBootName=./images/u-boot.bin
LinuxName=./images/uImage_8136
RootfsName=./images/rootfs-cpio.squashfs.img
UserCfgName=./images/user_cfg.jffs2.img
UserAppName=./images/user_app.jffs2.img

echo "[IMAGE_INFO] PLATFORMName: ${PLATFORMName}"
echo "[IMAGE_INFO] NSBootName  : ${NSBootName}"
echo "[IMAGE_INFO] uBootName   : ${uBootName}"
echo "[IMAGE_INFO] LinuxName   : ${LinuxName}"
echo "[IMAGE_INFO] RootfsName  : ${RootfsName}"
echo "[IMAGE_INFO] UserCfgName : ${UserCfgName}"
echo "[IMAGE_INFO] UserAppName : ${UserAppName}"

echo ""

ls -la \
"$NSBootName" \
"$uBootName" \
"$LinuxName" \
"$RootfsName" \
"$UserCfgName" \
"$UserAppName" \

echo ""

if [ ! -f "$NSBootName" ] ||
    [ ! -f "$uBootName" ] ||
    [ ! -f "$LinuxName" ] ||
    [ ! -f "$RootfsName" ] ||
    [ ! -f "$UserCfgName" ] ||
    [ ! -f "$UserAppName" ];
then
    echo "error: Image configuration error."
    exit 1
fi

# merge_spi_image Usage:
# merge_spi_image [platform_name] [chip size] [file number M] [assign partition number N]
#                 [partition 1 address] [partition 1 offset] [partition 1 name]
#                 [partition N address] [partition N offset] [partition N name]
#                 [file 1 name] [file 2 name] ... [file M name]

./bin/merge_spi_image \
$PLATFORMName \
0x1000000 \
6 \
5 \
0x10000 \
0x70000 \
UBOOT \
0x80000 \
0x240000 \
LINUX \
0x2C0000 \
0x4A0000 \
FS \
0x760000 \
0x100000 \
CONFIG \
0x860000 \
0x7A0000 \
APP \
"$NSBootName" \
"$uBootName" \
"$LinuxName" \
"$RootfsName" \
"$UserCfgName" \
"$UserAppName" \

echo ""

echo "Output image: all.img"
ls -la all.img

