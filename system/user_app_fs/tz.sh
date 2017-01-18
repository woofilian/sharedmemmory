# !/bin/sh


############################### timezone #############################
TZ=$1
if [ "$TZ" = "" ]; then
	TZ="Asia/Shanghai"
fi
	    
cur_dir=`pwd`
mkdir -p /tmp/zoneinfo
cd /tmp/zoneinfo
cp /home/zoneinfo.tar.bz2 /tmp/zoneinfo
	    
bunzip2 zoneinfo.tar.bz2
tar -xf zoneinfo.tar
rm -f zoneinfo.tar
cp -f /tmp/zoneinfo/$TZ /etc/localtime
	    
rm -rf /tmp/zoneinfo
echo $TZ
cd $cur_dir
