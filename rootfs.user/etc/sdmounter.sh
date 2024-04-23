#!/bin/sh
 
#destdir=/mnt/sd

MNT_FLASH_DIR=/mnt/flash
MNT_SD_DIR=/mnt/sd
MEDIA_DIR=/media

P3_DEV_NAME=mmcblk0p3
P3_DEV=/dev/${P3_DEV_NAME}
P3_DIR=${MEDIA_DIR}/boot

P4_DEV_NAME=mmcblk0p4
P4_DEV=/dev/${P4_DEV_NAME}
P4_DIR=${MEDIA_DIR}/factory

#P5_DEV_NAME=mmcblk0p5
#P5_DEV=/dev/${P5_DEV_NAME}
#P5_DIR=${MEDIA_DIR}/pm

#P6_DEV_NAME=mmcblk0p6
#P6_DEV=/dev/${P6_DEV_NAME}
#P6_DIR=${MEDIA_DIR}/config

P5_DEV_NAME=mmcblk0p5
P5_DEV=/dev/${P5_DEV_NAME}
P5_DIR=${MEDIA_DIR}/log

P6_DEV_NAME=mmcblk0p6
P6_DEV=/dev/${P6_DEV_NAME}
P6_DIR=${MNT_FLASH_DIR}
#P8_DIR=${MEDIA_DIR}/data

eMMC_mount()
{
	if [ $1 = ${P3_DEV_NAME} ]; then
		EMMC_DIR=${P3_DIR}
		EMMC_DEV=${P3_DEV}
	elif [ $1 = ${P4_DEV_NAME} ]; then
		EMMC_DIR=${P4_DIR}
		EMMC_DEV=${P4_DEV}
	elif [ $1 = ${P5_DEV_NAME} ]; then
		EMMC_DIR=${P5_DIR}
		EMMC_DEV=${P5_DEV}
	elif [ $1 = ${P6_DEV_NAME} ]; then
		EMMC_DIR=${P6_DIR}
		EMMC_DEV=${P6_DEV}
	#elif [ $1 = ${P7_DEV_NAME} ]; then
	#	EMMC_DIR=${P7_DIR}
	#	EMMC_DEV=${P7_DEV}
	#elif [ $1 = ${P8_DEV_NAME} ]; then
	#	EMMC_DIR=${P8_DIR}
	#	EMMC_DEV=${P8_DEV}
	else
		#echo "Unknown Device : $1"
		exit;
	fi

	#echo "eMMC Device : ${EMMC_DEV}"

	if [ ! -d ${EMMC_DIR} ]; then
		#echo "Make directory ${EMMC_DIR}"
		mkdir -p ${EMMC_DIR}
		#sleep 1
	fi

	if grep -q "^/dev/${MDEV} " /proc/mounts ; then
			# Already mounted
			echo "${MDEV} Already mounted"
			exit 0
	fi

	#mount -t ext4 -o defaults,noatime,nodiratime,nobarrier,nodelalloc,data=writeback ${EMMC_DEV} ${EMMC_DIR}
	#mount -t auto -o sync ${EMMC_DEV} ${EMMC_DIR} > /dev/null 2>&1
	mount -t auto -o sync ${EMMC_DEV} ${EMMC_DIR}
	if [ $? -ne 0 ]; then # mount success
		#echo "eMMC Device : ${EMMC_DEV}. mount SUCCESS! (Result : $?)"
	#else
		echo "eMMC Device : ${EMMC_DEV}. Need format : vfat (Result : $?)"
		mkfs.ext4 -F ${EMMC_DEV}
		if [ $? -eq 0 ]; then # format success
			mount -t auto -o sync ${EMMC_DEV} ${EMMC_DIR}
			if [ $? -eq 0 ]; then # mount success
				echo "eMMC Device : ${EMMC_DEV}. Remount SUCCESS!"
			else
				echo "eMMC Device : ${EMMC_DEV}. Remount FAIL!"
			fi
		else
			echo "eMMC Device : ${EMMC_DEV}. Format FAIL!"
		fi
	fi
	sleep 1
}

sdcard_umount()
{
	rm -f /etc/conf
	ln -sf /mnt/flash /etc/conf
	rm -f /var/coredump
	ln -sf /mnt/flash/coredump /var/coredump
	rm -f /var/misc
	ln -sf /mnt/flash/misc /var/misc
	if grep -qs "^/dev/$1 " /proc/mounts ; then
		umount "${destdir}";
		rm -rf "${destdir}"
	fi
}
 
sdcard_mount()
{
	mkdir -p "${destdir}/$1" || exit 1
	echo "parameter 1:$1, destdir:${destdir}" #RyongHa TEST
 
	if ! mount -t auto -o sync "/dev/$1" "${destdir}"; then
		# failed to mount, clean up mountpoint
		mkfs.vfat /dev/$1
			if ! mount -t auto -o sync "/dev/$1" "${destdir}"; then
				# failed to mount, clean up mountpoint
				exit 1
			fi
		#exit 1
	fi

	if [ ! -d ${destdir}/coredump ]
	then
	mkdir ${destdir}/coredump
	fi
	rm -f /var/coredump
	ln -sf ${destdir}/coredump /var/coredump
	
	if [ ! -d ${destdir}/log ]
	then
	mkdir ${destdir}/log
	fi

	if [ ! -d ${destdir}/firmware ]
	then
	mkdir ${destdir}/firmware
	mkdir ${destdir}/firmware/atu10f
	mkdir ${destdir}/firmware/otu4g
	mkdir ${destdir}/firmware/ttu4g
	mkdir ${destdir}/firmware/dcu
	fi

	if [ ! -d ${destdir}/firmware/atu10f ]
	then
	mkdir ${destdir}/firmware/atu10f
	fi

	if [ ! -d ${destdir}/firmware/otu4g ]
	then
	mkdir ${destdir}/firmware/otu4g
	fi

	if [ ! -d ${destdir}/firmware/ttu4g ]
	then
	mkdir ${destdir}/firmware/ttu4g
	fi

	if [ ! -d ${destdir}/firmware/dcu ]
	then
	mkdir ${destdir}/firmware/dcu
	fi

#modified by balkrow : auto-download 
	if [ ! -d ${destdir}/firmware/dcue ]
	then
	mkdir ${destdir}/firmware/dcue
	fi

	if [ ! -d ${destdir}/firmware/otu4go ]
	then
	mkdir ${destdir}/firmware/otu4go
	fi

	if [ ! -d ${destdir}/firmware/ttu4go ]
	then
	mkdir ${destdir}/firmware/ttu4go
	fi

	if [ ! -d ${destdir}/firmware/m0dhx ]
	then
	mkdir ${destdir}/firmware/m0dhx
	fi

	if [ ! -d ${destdir}/firmware/m0ah1 ]
	then
	mkdir ${destdir}/firmware/m0ah1
	fi

	if [ ! -d ${destdir}/firmware/msu_dcm ]
	then
	mkdir ${destdir}/firmware/msu_dcm
	fi

	if [ ! -d ${destdir}/firmware/msu0_dcm ]
	then
	mkdir ${destdir}/firmware/msu0_dcm
	fi
#end auto-download

	if [ ! -d ${destdir}/misc ]
	then
	mkdir ${destdir}/misc
	mkdir ${destdir}/misc/slot-data
	fi

	if [ ! -d ${destdir}/misc/slot-data ]
	then
	mkdir ${destdir}/misc/slot-data
	fi
	rm -f /var/misc
	ln -sf ${destdir}/misc /var/misc
	
	if [ ! -d ${destdir}/config ]
	then
	mkdir ${destdir}/config
	fi
	
	if [ ! -f ${destdir}/misc/udhcpd-vrf.leases ]
	then
		if [ -f /mnt/flash/misc/udhcpd-vrf.leases ]
		then
			cp -f /mnt/flash/misc/udhcpd-vrf.leases  ${destdir}/misc/udhcpd-vrf.leases  
		fi	
	fi

	if [ ! -f ${destdir}/misc/udhcpd-lan.leases ]
	then
		if [ -f /mnt/flash/misc/udhcpd-lan.leases ]
		then
			cp -f /mnt/flash/misc/udhcpd-lan.leases  ${destdir}/misc/udhcpd-lan.leases  
		fi	
	fi
	
	if [ ! -f ${destdir}/misc/system_random_id ]
	then
		if [ -f /mnt/flash/misc/system_random_id ]
		then
			cp -f /mnt/flash/misc/system_random_id  ${destdir}/misc/system_random_id  
		fi	
	fi
	
	if [ ! -d ${destdir}/tmp ]
	then
	mkdir ${destdir}/tmp
	else
	rm -f ${destdir}/tmp/*
	fi


	
	rm -f /etc/conf
	ln -sf ${destdir}/config /etc/conf

#modified by balkrow
	if [ -f /mnt/sd/config/VWMIUG.conf ]
	then
		if grep -Fxq "VWMVER" /mnt/sd/config/VWMIUG.conf 
		then
			if ! grep -Fxq "END" /mnt/sd/config/VWMIUG.conf 
			then
				cp -f /mnt/sd/config/VWMIUG.conf /mnt/sd/config/VWMIUG.conf
			fi
		fi
	fi
}
 
case "${ACTION}" in
add|"")
	eMMC_mount ${MDEV}
	#sdcard_umount ${MDEV}
	#sdcard_mount ${MDEV}
	;;
remove)
	#sdcard_umount ${MDEV}
	;;
esac
