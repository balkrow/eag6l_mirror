#!/bin/bash

#cd $(dirname $0)
INSTALL_ROOT=${TOPDIR}/rootfs
DROPBEAR=`pwd`

function _clean {
	[ -f Makefile ] && make distclean
}

function _config {
	./configure											\
		CC="${CROSS_COMPILE}gcc"						\
		--host=${CROSS_COMPILE_PFX}						\
		--prefix=${DROPBEAR}/install					\
		|| exit -1
}

function _build {
	make ${MAKEOPT} PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" || exit -1
}

function _install {

	make PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" install

	cp -vf dropbear ${INSTALL_ROOT}/usr/sbin/dropbear
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/usr/sbin/dropbear

	cp -vf dbclient ${INSTALL_ROOT}/usr/sbin/dbclient
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/usr/sbin/dbclient

	cp -vf dropbearkey ${INSTALL_ROOT}/usr/sbin/dropbearkey
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/usr/sbin/dropbearkey

	cp -vf dropbearconvert ${INSTALL_ROOT}/usr/sbin/dropbearconvert
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/usr/sbin/dropbearconvert

	cp -vf scp ${INSTALL_ROOT}/usr/sbin/scp
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/usr/sbin/scp

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
