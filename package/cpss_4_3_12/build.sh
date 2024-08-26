#!/bin/bash

#cd $(dirname $0)
INSTALL_ROOT=${TOPDIR}/rootfs
branch_name=`git symbolic-ref --short HEAD` 

function _clean {

  if [ ${BUILD_BOARD_NAME} == "eag6l" ] ; then
	[ -f Makefile ] && make ${MAKEOPT} TARGET=armv7 FAMILY=DX clean || exit -1
	[ -f Makefile ] && make ${MAKEOPT} TARGET=armv7 FAMILY=DX KERNEL_FOLDER=../../build/kernel clean || exit -1
	[ -f Makefile ] && unset CROSS_COMPILE; unset ARCH; make ${MAKEOPT} TARGET=sim64 FAMILY=DX clean  || exit -1
  elif [ ${BUILD_BOARD_NAME} == "ac5x_db" ] ; then
	[ -f Makefile ] && make ${MAKEOPT} TARGET=armv8 FAMILY=DX clean || exit -1
	[ -f Makefile ] && make ${MAKEOPT} TARGET=armv8 FAMILY=DX KERNEL_FOLDER=../../build/kernel clean || exit -1
	[ -f Makefile ] && unset CROSS_COMPILE; unset ARCH; make ${MAKEOPT} TARGET=sim64 FAMILY=DX clean  || exit -1
  fi
}

function _config {
	exit 0
}

function _build {
	#make ${MAKEOPT} TARGET=armv7 FAMILY=DX CONFIG_PRV_CPSS_MAX_PP_DEVICES_CNS=2 CONFIG_CPSS_MAX_PORTS_NUM_CNS=128 LOG_ENABLE=n CMD_LUA_CLI=n appDemo || exit -1
  if [ ${BUILD_BOARD_NAME} == "eag6l" ] ; then
	make ${MAKEOPT} TARGET=armv7 FAMILY=DX CONFIG_LOG_ENABLE=N CONFIG_CMD_LUA_CLI=N appDemo || exit -1
	make ${MAKEOPT} TARGET=armv7 FAMILY=DX KERNEL_FOLDER=../../build/kernel modules || exit -1
  elif [ ${BUILD_BOARD_NAME} == "ac5x_db" ] ; then
	make ${MAKEOPT} TARGET=armv8 FAMILY=DX appDemo || exit -1
	make ${MAKEOPT} TARGET=armv8 FAMILY=DX KERNEL_FOLDER=../../build/kernel modules || exit -1
  fi
}

function _demo {
	unset CROSS_COMPILE; unset ARCH; make ${MAKEOPT} TARGET=sim64 FAMILY=DX appDemo || exit -1
}


function _install {
  if [ ${BUILD_BOARD_NAME} == "eag6l" ] ; then
	cp -avf ./compilation_root/${branch_name}/armv7_DX/appDemo ${INSTALL_ROOT}/sbin
	${CROSS_COMPILE}strip ${INSTALL_ROOT}/sbin/appDemo
	cp -avf cpssEnabler/mainExtDrv/src/gtExtDrv/linuxNoKernelModule/drivers/mvcpss.ko ${INSTALL_ROOT}/lib/modules
  elif [ ${BUILD_BOARD_NAME} == "ac5x_db" ] ; then
	cp -avf ./compilation_root/${branch_name}/armv8_DX/appDemo ~
	${CROSS_COMPILE}strip ~/appDemo
	cp -avf cpssEnabler/mainExtDrv/src/gtExtDrv/linuxNoKernelModule/drivers/mvcpss.ko ~
  fi
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
