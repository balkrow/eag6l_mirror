#!/bin/bash

#variable
## [#96] Adding option bit after downloading FPGA, dustin, 2024-08-19
##       added OWNER to update recent image binary.
OWNER=`whoami`
IMG_DIR="/tftpboot/$OWNER/"
FW_INFO=".pkg_info"

#check image 

if [ -d $IMG_DIR ]; then
	echo "$IMG_DIR directory exist"
else
	echo "directory not exist.. image directory is current directory"
	IMG_DIR=$PWD
fi

OS_IMG=$1
FPGA_OS_IMG=$2
FPGA_IMG=$3
EVAL_VERSION=$4
PRODUCT_VENDOR=$5
PRODUCT_CODE=$6
PRODUCT_NAME=$7
BUILD_ID=$8
BUILD_NAME=$9

echo "OS_IMG = $OS_IMG"
echo "FPGA_OS_IMG = $FPGA_OS_IMG"
echo "FPGA_IMG = $FPGA_IMG"
echo "EVAL_VERSION = $EVAL_VERSION"
echo "PRODUCT_VENDOR = $PRODUCT_VENDOR"
echo "PRODUCT_CODE = $PRODUCT_CODE"
echo "PRODUCT_NAME = $PRODUCT_NAME"
echo "BUILD_ID = $BUILD_ID"
echo "BUILD_NAME = $BUILD_NAME"

cp $IMG_DIR$1 .
cp $IMG_DIR$2 .
###cp $IMG_DIR$3 .

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

###if [ -f "$FPGA_IMG" ]; then
###	echo "$FPGA_IMG file exist"
###else
###	echo "$FPGA_IMG not exist"
###	exit 0
###fi

# create fw_info
./mkfwimage -j $MAJOR -i $MINOR -r $REV -X -d $OS_IMG:$FPGA_OS_IMG:
mv .fw_info $FW_INFO

zip $PRODUCT_NAME-PKG.zip $OS_IMG $FPGA_OS_IMG $FW_INFO

FILESIZE=`du -sb $PRODUCT_NAME-PKG.zip | awk '{print $1}'`

echo "total size = $FILESIZE"
echo "PKG_VERSION = $PKG_VERSION"

# rename zip to PKG file.
mv $PRODUCT_NAME-PKG.zip $PRODUCT_NAME-PKG-v$PKG_VERSION.PKG

# add header (fw_image_header_t)
./fwimage -C eag6l -v $PKG_VERSION -n hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION} -d \
$PRODUCT_NAME-PKG-v$PKG_VERSION.PKG hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION}.bin

mv hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION}.bin ..
ls -al ${TOPDIR}/hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION}.bin 

exit 0
