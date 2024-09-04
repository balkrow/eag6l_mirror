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
FPGA_IMG=$2

echo "OS_IMG = $OS_IMG"
echo "FPGA_IMG = $FPGA_IMG"

#cp $IMG_DIR$1 .
#cp $IMG_DIR$2 .
###cp $IMG_DIR$3 .

if [ -f "$OS_IMG" ]; then
	echo "$OS_IMG file exist"
else
	echo "$OS_IMG not exist"
	exit 0
fi

if [ -f "$FPGA_IMG" ]; then
	echo "$FPGA_IMG file exist"
else
	echo "$FPGA_IMG not exist"
	exit 0
fi

###if [ -f "$FPGA_IMG" ]; then
###	echo "$FPGA_IMG file exist"
###else
###	echo "$FPGA_IMG not exist"
###	exit 0
###fi

# create fw_info
#./mkfwimage -j $MAJOR -i $MINOR -r $REV -X -d $OS_IMG:$FPGA_OS_IMG:
#mv .fw_info $FW_INFO
PKG_VERSION=${MAJOR}.${MINOR}.${REV}

#zip $PRODUCT_NAME-PKG.zip $OS_IMG $FPGA_OS_IMG $FW_INFO
zip $PRODUCT_NAME-PKG.zip $OS_IMG $FPGA_IMG

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
cp -avf ${TOPDIR}/hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION}.bin $HOME 

exit 0
