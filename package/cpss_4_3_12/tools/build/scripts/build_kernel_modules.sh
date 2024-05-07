#!/usr/bin/env bash
##################################################################################
##  build_kernel_modules.sh
##################################################################################
##
##  Build Kernel Modules for various Linux Kernel versions and architectures
##
##################################################################################
## ARGUMENTS         :  Can be defined in any order
##
## No arguments: use defaults
##
## --h: Help
##
##################################################################


####################
# HELP option
####################
if [ "$1" = "--h" ]; then
    echo ""
    echo ""
    echo "Build Kernel Modules for various Linux Kernel versions and architectures"
    echo ""
    echo "=========="
    echo "LK 3.10.70 Src MUST be at /local/store/LINUX/linux-cpss-3.10.70"
    echo "LK 4.4.52 Src MUST be at /local/store/LINUX/linux-cpss-4.4.52"
    echo "LK 4.14.76 Src MUST be at /local/store/LINUX/linux-cpss-4.14.76"
    echo "LK 5.4.74 Src MUST be at /local/store/LINUX/linux-cpss-5.4.74"
    echo "LK 5.15.76 Src MUST be at /local/store/LINUX/linux-cpss-5.15.76"
    echo "=========="
    exit 0
fi

#Exit on first error
set -e

#Check if LK 3.10.70 source directory exists, if not exit
LK_3_10_70_SRC=/local/store/LINUX/linux-cpss-3.10.70
if [ -d ${LK_3_10_70_SRC} ]; then : ; else echo ${LK_3_10_70_SRC} does not exist!!! exiting...; exit; fi

#Check if LK 4.4.52 source directory exists, if not exit
LK_4_4_52_SRC=/local/store/LINUX/linux-cpss-4.4.52
if [ -d ${LK_4_4_52_SRC} ]; then : ; else echo ${LK_4_4_52_SRC} does not exist!!! exiting...; exit; fi

#Check if LK 4.14.76 source directory exists, if not exit
LK_4_14_76_SRC=/local/store/LINUX/linux-cpss-4.14.76
if [ -d ${LK_4_14_76_SRC} ]; then : ; else echo ${LK_4_14_76_SRC} does not exist!!! exiting...; exit; fi

#Check if LK 5.4.74 source directory exists, if not exit
LK_5_4_74_SRC=/local/store/LINUX/linux-cpss-5.4.74
if [ -d ${LK_5_4_74_SRC} ]; then : ; else echo ${LK_5_4_74_SRC} does not exist!!! exiting...; exit; fi

#Check if LK 5.15.76 source directory exists, if not exit
LK_5_15_76_SRC=/local/store/LINUX/linux-cpss-5.15.76
if [ -d ${LK_5_15_76_SRC} ]; then : ; else echo ${LK_5_15_76_SRC} does not exist!!! exiting...; exit; fi

#Check if CPSS WS exists
CPSS_WS=$(pwd)
if [ -d ${CPSS_WS} ]; then cd ${CPSS_WS}; else echo ${CPSS_WS} "doesn't exist!!!" && exit; fi

#Store CPSS KO directory path for later use
CPSS_KO_DIR=${CPSS_WS}/cpssEnabler/mainExtDrv/src/gtExtDrv/linuxNoKernelModule/drivers/

#Fetch GIT TAGS
git fetch --all --tags

#Get last CPSS commit hash
CPSS_TAG_COMMIT_HASH=$(git describe --tag)

#Check if destination directory exists, if not create one
DEST_DIR=/${CPSS_WS}/compilation_root/CPSS_LKM_${CPSS_TAG_COMMIT_HASH}
if [ -d ${DEST_DIR} ]; then : ; else mkdir ${DEST_DIR} || exit; fi

#Save git history -20 for reference
git hist -20 > ${DEST_DIR}/history.log

#Create info file
echo "Build Date    : " $(date)  		> 	${DEST_DIR}/info.log
echo "Build User    : " $LOGNAME 		>> 	${DEST_DIR}/info.log
echo "Build Mach    : " $(hostname) 	>> 	${DEST_DIR}/info.log
echo "Build WS      : " $CPSS_WS  		>> 	${DEST_DIR}/info.log

#Final KO to be copied to DEST DIR
LKM=mvcpss.ko

#Check if destination directory exists, if not create one
ARMv7_LE_DIR=${DEST_DIR}/armv7_le
if [ -d ${ARMv7_LE_DIR} ]; then : ; else mkdir ${ARMv7_LE_DIR} || exit; fi

#Check if destination directory exists, if not create one
ARMv7_BE_DIR=${DEST_DIR}/armv7_be
if [ -d ${ARMv7_BE_DIR} ]; then : ; else mkdir ${ARMv7_BE_DIR} || exit; fi

#Check if destination directory exists, if not create one
ARMv8_LE_DIR=${DEST_DIR}/armv8_le
if [ -d ${ARMv8_LE_DIR} ]; then : ; else mkdir ${ARMv8_LE_DIR} || exit; fi

#Check if destination directory exists, if not create one
ARMv8_BE_DIR=${DEST_DIR}/armv8_be
if [ -d ${ARMv8_BE_DIR} ]; then : ; else mkdir ${ARMv8_BE_DIR} || exit; fi

####################################################################################################
#Build LK 3.10.70 + Armv7 + LE
cd ${LK_3_10_70_SRC}
make mrproper 
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- mvebu_lsp_defconfig
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- msys-ac3-db.dtb
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- Image -j 12

#Check if destination directory exists, if not create one
LK_3_10_70_DIR=${ARMv7_LE_DIR}/3.10.70
if [ -d ${LK_3_10_70_DIR} ]; then : ; else mkdir ${LK_3_10_70_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 3.10.70 + Armv7 + LE
cd ${CPSS_WS}
export CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-
make FAMILY=DX TARGET=armv7 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-3.10.70 modules
unset CROSS_COMPILE
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_3_10_70_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_3_10_70_DIR}/${LKM}
####################################################################################################

###################################################################################################
#Build LK 4.4.52 + Armv8 + LE
cd ${LK_4_4_52_SRC}
make mrproper
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabi/bin/arm-linux-gnueabi- marvell_v8_aarch32_sdk_defconfig
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabi/bin/arm-linux-gnueabi- ac5_db.dtb
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabi/bin/arm-linux-gnueabi- Image -j 12

#Check if destination directory exists, if not create one
LK_4_4_52_DIR=${ARMv8_LE_DIR}/4.4.52
if [ -d ${LK_4_4_52_DIR} ]; then : ; else mkdir ${LK_4_4_52_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 4.4.52 + Armv8 + LE
cd ${CPSS_WS}
make FAMILY=DX TARGET=armv7 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-4.4.52 modules
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_4_4_52_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_4_4_52_DIR}/${LKM}
###################################################################################################

####################################################################################################
#Build LK 4.14.76 + Armv8 + LE
cd ${LK_4_14_76_SRC}
make mrproper
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell_v8_sdk_defconfig
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell/ac5_db.dtb
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- Image.gz -j 12

#Check if destination directory exists, if not create one
LK_4_14_76_DIR=${ARMv8_LE_DIR}/4.14.76
if [ -d ${LK_4_14_76_DIR} ]; then : ; else mkdir ${LK_4_14_76_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 4.14.76 + Armv8 + LE
cd ${CPSS_WS}
make FAMILY=DX TARGET=armv8 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-4.14.76 modules
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_4_14_76_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_4_14_76_DIR}/${LKM}
####################################################################################################

####################################################################################################
#Build LK 4.14.76 + Armv8 + BE
cd ${LK_4_14_76_SRC}
make mrproper
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell_v8_sdk_defconfig
./scripts/config --set-val CONFIG_CPU_BIG_ENDIAN y
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell/ac5_db.dtb
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- Image.gz -j 12

#Check if destination directory exists, if not create one
LK_4_14_76_DIR=${ARMv8_BE_DIR}/4.14.76-19.0.0
if [ -d ${LK_4_14_76_DIR} ]; then : ; else mkdir ${LK_4_14_76_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 4.14.76 + Armv8 + BE
cd ${CPSS_WS}
make FAMILY=DX TARGET=armv8 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-4.14.76 modules
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_4_14_76_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_4_14_76_DIR}/${LKM}
####################################################################################################

####################################################################################################
#Build LK 4.14.76 + Armv7 + BE
cd ${LK_4_14_76_SRC}
make mrproper
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- mvebu_v7_defconfig
./scripts/config --set-val CONFIG_CPU_BIG_ENDIAN y
./scripts/config --set-val CONFIG_CPU_ENDIAN_BE8 y
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- msys-ac3-db.dtb
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- Image -j 12

#Check if destination directory exists, if not create one
LK_4_14_76_DIR=${ARMv7_BE_DIR}/4.14.76-19.0.0
if [ -d ${LK_4_14_76_DIR} ]; then : ; else mkdir ${LK_4_14_76_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 4.14.76 + Armv8 + BE
cd ${CPSS_WS}
make FAMILY=DX TARGET=armv7 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-4.14.76 modules
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_4_14_76_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_4_14_76_DIR}/${LKM}
###################################################################################################

####################################################################################################
#Build LK 4.14.76 + Armv7 + LE
cd ${LK_4_14_76_SRC}
make mrproper
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- mvebu_v7_defconfig
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- msys-ac3-db.dtb
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- Image -j 12

#Check if destination directory exists, if not create one
LK_4_14_76_DIR=${ARMv7_LE_DIR}/4.14.76-19.0.0
if [ -d ${LK_4_14_76_DIR} ]; then : ; else mkdir ${LK_4_14_76_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 4.14.76 + Armv8 + LE
cd ${CPSS_WS}
make FAMILY=DX TARGET=armv7 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-4.14.76 modules
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_4_14_76_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_4_14_76_DIR}/${LKM}
###################################################################################################

###################################################################################################
#Build LK 5.15.76 + Armv8 + LE
cd ${LK_5_15_76_SRC}
make mrproper
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell_v8_sdk_defconfig
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell/ac5_db.dtb
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- Image.gz -j 12
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- modules -j 12

#Check if destination directory exists, if not create one
LK_5_15_76_DIR=${ARMv8_LE_DIR}/5.15.76
if [ -d ${LK_5_15_76_DIR} ]; then : ; else mkdir ${LK_5_15_76_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 5.4.74 + Armv8 + LE
cd ${CPSS_WS}
make FAMILY=DX TARGET=armv8 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-5.15.76 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- modules
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_5_15_76_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_5_15_76_DIR}/${LKM}
###################################################################################################

###################################################################################################
#Build LK 5.4.74 + Armv7 + LE
cd ${LK_5_4_74_SRC}
make mrproper
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- mvebu_v7_defconfig
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- msys-ac3-db.dtb
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- Image -j 12

#Check if destination directory exists, if not create one
LK_5_4_74_DIR=${ARMv7_LE_DIR}/5.4.74
if [ -d ${LK_5_4_74_DIR} ]; then : ; else mkdir ${LK_5_4_74_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 5.4.74 + Armv7 + LE
cd ${CPSS_WS}
make FAMILY=DX TARGET=armv7 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-5.4.74 modules
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_5_4_74_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_5_4_74_DIR}/${LKM}
###################################################################################################

###################################################################################################
#Build LK 5.4.74 + Armv8 + LE
cd ${LK_5_4_74_SRC}
make mrproper
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell_v8_sdk_defconfig
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell/ac5_db.dtb
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- Image.gz -j 12

#Check if destination directory exists, if not create one
LK_5_4_74_DIR=${ARMv8_LE_DIR}/5.4.74
if [ -d ${LK_5_4_74_DIR} ]; then : ; else mkdir ${LK_5_4_74_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 5.4.74 + Armv8 + LE
cd ${CPSS_WS}
make FAMILY=DX TARGET=armv8 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-5.4.74 modules
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_5_4_74_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_5_4_74_DIR}/${LKM}
###################################################################################################

###################################################################################################
#Build LK 5.4.74 + Armv8 + BE
cd ${LK_5_4_74_SRC}
make mrproper
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell_v8_sdk_defconfig
echo "********************************************************************************************"
echo 'Go to "Kernel Features"  ---> Select "[*] Build big-endian kernel"'
echo "Press ENTER to continue..."
echo "********************************************************************************************"
read
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- menuconfig
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- marvell/ac5_db.dtb
make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu- Image.gz -j 12

#Check if destination directory exists, if not create one
LK_5_4_74_DIR=${ARMv8_BE_DIR}/5.4.74
if [ -d ${LK_5_4_74_DIR} ]; then : ; else mkdir ${LK_5_4_74_DIR} || exit; fi

#Cross Compile CPSS KOs for LK 5.4.74 + Armv8 + BE
cd ${CPSS_WS}
make FAMILY=DX TARGET=armv8 KERNEL_FOLDER=/local/store/LINUX/linux-cpss-5.4.74 modules
echo "cp ${CPSS_KO_DIR}/${LKM} ${LK_5_4_74_DIR}/${LKM}"
cp ${CPSS_KO_DIR}/${LKM} ${LK_5_4_74_DIR}/${LKM}
###################################################################################################

###################################################################################################
echo
echo
echo "Cross Compiled ${LKM} is copied to ${DEST_DIR}"
###################################################################################################