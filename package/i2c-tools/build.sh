#!/bin/bash

#cd $(dirname $0)
INSTALL_ROOT=${TOPDIR}/rootfs

function _clean {
	[ -f Makefile ] && make clean
}

function _config {
	exit 0
}

function _build {
	CC="${CROSS_COMPILE}gcc" make || exit -1
}

function _install {
	cp -avf tools/i2cdetect ${INSTALL_ROOT}/bin
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/bin/i2cdetect
	cp -avf tools/i2cdump ${INSTALL_ROOT}/bin
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/bin/i2cdump
	cp -avf tools/i2cget ${INSTALL_ROOT}/bin
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/bin/i2cget
	cp -avf tools/i2cset ${INSTALL_ROOT}/bin
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/bin/i2cset

	cp -avf lib/libi2c.so* ${INSTALL_ROOT}/usr/lib
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/usr/lib/libi2c.so*
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
