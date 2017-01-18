# !/bin/sh

mounted_dev=""
while true
do

mmcs=`ls /dev/mmcblk* 2>/dev/null`
                                                               
if [ ! -f /var/log/has_sd_card -a "$mmcs" != "" ]; then
for i in $mmcs                                 
do                              
	mount -t vfat $i /mnt/sd_card              
#	if [ "$?" = "0" ]; then # do not check result anymore            
		echo $i                     
		mounted_dev="$i"
		mkdir -p /var/log		
		echo "$mounted_dev" > /var/log/has_sd_card
		break                        
#	fi
done                                         
fi                                           
                                                                              
if [ -f /var/log/has_sd_card -a "$mmcs" = "" ]; then
	echo "umount $mounted_dev"  
	umount -lf /mnt/sd_card     
	rm -rf /var/log
fi                  
                                                                            
#check every seconds
sleep 2
done
