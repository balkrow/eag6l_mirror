#!/bin/bash

#export CFLAGS="${CROSS_CFLAGS} ${PLATFORM_CFLAGS}"
#export LDFLAGS="${CROSS_LDFLAGS}"
INSTALL_ROOT=${TOPDIR}/rootfs

function _clean {
	make distclean || exit -1
}

function _config {
	make ${BUILD_BOARD_NAME}_defconfig || exit -1
}

function _build {
	make ${CONFIG_DEBUG} ${MAKEOPT} V=1 || exit -1
	echo "$TARGET_APPS_MANUAL" | grep -qw busybox || exit 0
}

function _install {
	make CONFIG_PREFIX=${INSTALL_ROOT} install
	if [ "${CONFIG_DEBUG}" == "CONFIG_DEBUG=y" ] ; then
		cp -avf busybox_unstripped ${INSTALL_ROOT}/bin/busybox
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
	debug)
		_dbg
		;;
	rebuild)
		_clean
		_config
		_build
		;;
	install)
		_install
		;;
	dbg_install)
		_dbg_install
		;;
	*)
		exit -1
esac
