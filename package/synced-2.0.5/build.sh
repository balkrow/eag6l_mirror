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
  make synced ESMC_STACK=renesas PLATFORM=${ARCH} DEVICE=rsmu SYNCED_DEBUG_MODE=1 || exit -1
}

function _install {
	cp -avf build/bin/synced ${INSTALL_ROOT}/sbin
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/sbin/synced
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
	*)
		exit -1
esac
