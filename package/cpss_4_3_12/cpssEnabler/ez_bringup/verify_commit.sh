#!/bin/bash 

declare -a TARGETS_LIST=( "/swtools/devtools/gnueabi/arm_be/aarch64ebv8-marvell-linux-gnu/bin/aarch64_be-linux-gnu-" 
							 "" 
							 "/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20151110/bin/aarch64-marvell-linux-gnu-"
							 "/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-arm-10.1-2020.6-x86_64-aarch64-marvell-linux/bin/aarch64-marvell-linux-gnu-"
							 "/swtools/devtools/gnueabi/arm_le/gcc-arm-12.04-2022.12-x86_64-aarch64-marvell-linux/bin/aarch64-marvell-linux-gnu-"
							 "/swtools/devtools/gnueabi/arm_le/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabi/bin/arm-linux-gnueabi-"							
							 );

ERROR_CODE=0

for i in "${!TARGETS_LIST[@]}" 
do
	make clean > /dev/null;
	make bm CROSS_COMPILE=${TARGETS_LIST[$i]} -s -j >> log 2>&1
	if [ "$?" -ne 0 ]; then
		echo "*** [Build failed on ${TARGETS_LIST[$i]}] ***"
		
		ERROR_CODE=1
	fi
done

if [ $ERROR_CODE = 1 ]; 
then
	cat log |grep "error\:" -A3 -B3
fi

rm log
exit $ERROR_CODE

