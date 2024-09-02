#!/bin/bash

#cd $(dirname $0)

function _clean {
	[ -f Makefile ] && make clean
}

function _config {
	exit 0
}

function _build {
	make || exit -1
}

function _install {
	cp -avf fwimage ${TOPDIR}/scripts
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
