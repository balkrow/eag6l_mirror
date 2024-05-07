#!/bin/bash

#cd $(dirname $0)
INSTALL_ROOT=${TOPDIR}/rootfs

function _clean {
	[ -f Makefile ] && make ${MAKEOPT} TARGET=armv7 FAMILY=DX clean || exit -1
	[ -f Makefile ] && make ${MAKEOPT} TARGET=armv7 FAMILY=DX KERNEL_FOLDER=../../build/kernel clean || exit -1
	[ -f Makefile ] && unset CROSS_COMPILE; unset ARCH; make ${MAKEOPT} TARGET=sim64 FAMILY=DX clean  || exit -1
}

function _config {
	exit 0
}

function _build {
	make ${MAKEOPT} TARGET=armv7 FAMILY=DX appDemo || exit -1
	make ${MAKEOPT} TARGET=armv7 FAMILY=DX KERNEL_FOLDER=../../build/kernel modules || exit -1
}

function _demo {
	unset CROSS_COMPILE; unset ARCH; make ${MAKEOPT} TARGET=sim64 FAMILY=DX appDemo || exit -1
}


function _install {
	#cp -avf cpssEnabler/mainExtDrv/src/gtExtDrv/linuxNoKernelModule/drivers/mvEthOpsDrv.ko ${INSTALL_ROOT}/lib/modules
	cp -avf cpssEnabler/mainExtDrv/src/gtExtDrv/linuxNoKernelModule/drivers/mvcpss.ko ${INSTALL_ROOT}/lib/modules
	exit 0
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
	demo)
		_demo
		;;
	*)
		exit -1
esac
