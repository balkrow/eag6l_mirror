#!/bin/bash

#cd $(dirname $0)
INSTALL_ROOT=${TOPDIR}/rootfs
QUAGGA=`pwd`

function _clean {
	[ -f Makefile ] && make distclean && rm -fr install || exit -1
}

function _config {
	./configure											\
		--host=${CROSS_COMPILE_PFX}							\
		--prefix=$QUAGGA/install \
		--sysconfdir=/etc \
		--disable-doc --disable-bgpd --disable-ripd --disable-ripngd --disable-ospfd \
		--disable-ospf6d --disable-nhrpd --disable-isisd --disable-pimd --disable-bgp-announce \
		--disable-ospfapi --disable-ospfclient --enable-sysmon \
		|| exit -1
}

function _build {
	make ${MAKEOPT} || exit -1
}

function _install {
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
