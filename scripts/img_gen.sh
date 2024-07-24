#!/bin/bash

#variable
IMG_DIR="/tftpboot/"
FW_INFO=".pkg_info"

#check image 

if [ -d $IMG_DIR ]; then
	echo "directory exist"
else
	echo "directory not exist.. image directory is current directory"
	IMG_DIR=$PWD
fi

OS_IMG=$1
FPGA_OS_IMG=$2
FPGA_IMG=$3

MF_VERSION=$4
MF_VENDOR=$5
MF_CODE=$6
MF_NAME=$7
MF_BLDID=$8
MF_BLDNAME=$9


rm *PKG

cp $IMG_DIR$1 .
cp $IMG_DIR$2 .
cp $IMG_DIR$3 .

if [ -f "$OS_IMG" ]; then
	echo "$OS_IMG file exist"
else
	echo "$OS_IMG not exist"
	exit 0
fi

if [ -f "$FPGA_OS_IMG" ]; then
	echo "$FPGA_OS_IMG file exist"
else
	echo "$FPGA_OS_IMG not exist"
	exit 0
fi

if [ -f "$FPGA_IMG" ]; then
	echo "$FPGA_IMG file exist"
else
	echo "$FPGA_IMG not exist"
	exit 0
fi

# create fw_info
./mkfwimage -j $MAJOR -i $MINOR -r $REV -X -d $OS_IMG:$FPGA_OS_IMG:$FPGA_IMG
mv .fw_info $FW_INFO

zip EAG6L-PKG.zip $OS_IMG $FPGA_OS_IMG $FPGA_IMG $FW_INFO

FILESIZE=`du -sb EAG6L-PKG.zip | awk '{print $1}'`

echo "total size = $FILESIZE"
echo "MAJOR = $MAJOR"
echo "MINOR = $MINOR"

### DO NOT ADD PKG HEADER. MCU WILL ADD FW HEADER.
###./mkfwimage -m h -n HFR -F 0 -f EAG6L-PKG- -j $MAJOR \
###	-i $MINOR -r $REV -M 0 -d EAG6L-PKG.zip EAG6L-PKG-$MAJOR$MINOR.PKG
### JUST RENAME ZIP TO PKG.
mv EAG6L-PKG.zip ../EAG6L-PKG-$MAJOR$MINOR.PKG
