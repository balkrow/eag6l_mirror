#!/bin/bash

OS_IMG=$1
FPGA_IMAGE=$2

echo "OS_IMG = $OS_IMG"
echo "FPGA_IMG = $FPGA_IMAGE"
#PKG_VERSION=${MAJOR}.${MINOR}.${REV}


if ! [ -f "$OS_IMG" ]; then
	echo "$OS_IMG not exist"
	exit 0
fi

if [ -f "$FPGA_IMAGE" ]; then
	#echo "$FPGA_IMAGE file exist"
  zip $PRODUCT_NAME-PKG.zip $OS_IMG $FPGA_IMAGE
  FILESIZE=`du -sb $PRODUCT_NAME-PKG.zip | awk '{print $1}'`
  echo "total size = $FILESIZE"
  echo "PKG_VERSION = $PKG_VERSION"
  mv $PRODUCT_NAME-PKG.zip $PRODUCT_NAME-PKG-v$PKG_VERSION.PKG
 ./fwimage -C eag6l -v $PKG_VERSION -n hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION} -d \
  $PRODUCT_NAME-PKG-v$PKG_VERSION.PKG hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION}.bin
else
	echo "$FPGA_IMAGE not exist"
 ./fwimage -C eag6l -v $PKG_VERSION -n hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION} -d \
  $OS_IMG hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION}.bin
fi

mv hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION}.bin ..
ls -al ${TOPDIR}/hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION}.bin 
cp -avf ${TOPDIR}/hsn8000na-eag6l-us-${IMG_TYPE}-v${PKG_VERSION}.bin $HOME 

exit 0
