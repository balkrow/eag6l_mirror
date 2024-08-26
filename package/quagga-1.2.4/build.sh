#!/bin/bash

#cd $(dirname $0)
INSTALL_ROOT=${TOPDIR}/rootfs
QUAGGA=`pwd`

function _clean {
	[ -f Makefile ] && make distclean && rm -fr install || exit -1
}

#[#31]DB-98DX7312-8SFP1QSFP 용 setup.env 추가 및 개발환경 수정
function _config {
  if [ ${BUILD_BOARD_NAME} == "eag6l" ] ; then
    automake
	./configure											\
		--host=${CROSS_COMPILE_PFX}							\
		--prefix=$QUAGGA/install \
		--sysconfdir=/etc \
		--disable-doc --disable-bgpd --disable-ripd --disable-ripngd --disable-ospfd \
		--disable-ospf6d --disable-nhrpd --disable-isisd --disable-pimd --disable-bgp-announce \
		--disable-ospfapi --disable-ospfclient --enable-sysmon \
		|| exit -1
   elif [ ${BUILD_BOARD_NAME} == "ac5x_db" ] ; then
   CC=${CROSS_COMPILE}gcc \
   AR=${CROSS_COMPILE}ar \
   LD=${CROSS_COMPILE}ld \
   RANLIB=${CROSS_COMPILE}ranlib \
   CFLAGS="-DMVDEMO -g -O2" \
	./configure											\
		--prefix=$QUAGGA/install \
		--host=arm-linux \
		--sysconfdir=/etc \
		--disable-doc --disable-bgpd --disable-ripd --disable-ripngd --disable-ospfd \
		--disable-ospf6d --disable-nhrpd --disable-isisd --disable-pimd --disable-bgp-announce \
		--disable-ospfapi --disable-ospfclient --enable-sysmon \
		|| exit -1
   fi
}

function _build {
	make ${MAKEOPT} V=1 || exit -1
}

function _install {

  if [ ${BUILD_BOARD_NAME} == "eag6l" ] ; then

	cp -avf zebra/.libs/zebra $INSTALL_ROOT/sbin
	${CROSS_COMPILE}strip $INSTALL_ROOT/sbin/zebra 

	cp -avf vtysh/.libs/vtysh $INSTALL_ROOT/bin
	${CROSS_COMPILE}strip $INSTALL_ROOT/bin/vtysh 

	cp -avf sysmon/.libs/sysmon $INSTALL_ROOT/sbin
	${CROSS_COMPILE}strip $INSTALL_ROOT/sbin/sysmon 

	cp -avf lib/.libs/libzebra.so* $INSTALL_ROOT/lib
	${CROSS_COMPILE}strip $INSTALL_ROOT/lib/libzebra.so*

	cp -avf vtysh.conf $INSTALL_ROOT/etc
	cp -avf zebra.conf $INSTALL_ROOT/etc
	cp -avf sysmon.conf $INSTALL_ROOT/etc

  elif [ ${BUILD_BOARD_NAME} == "ac5x_db" ] ; then
	cp -avf sysmon/.libs/sysmon ~
	cp -avf vtysh/.libs/vtysh ~
	#${CROSS_COMPILE}strip ~/sysmon
	#${CROSS_COMPILE}strip ~/vtysh
  fi
}

if [ $# -eq 0 ]; then
	cmd="build"
else
	cmd="$1"
	shift
fi

case "$cmd" in
	clean)
		_clean
		;;
	config)
		_config
		;;
	build)
		_build
		;;
	rebuild)
		_clean
		_build
		;;
	install)
		_install
		;;
	*)
		exit -1
esac
