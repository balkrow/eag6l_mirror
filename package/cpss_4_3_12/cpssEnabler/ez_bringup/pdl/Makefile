# Main Makefile for Ros Over linux (BM + WM)
#
SHELL := /bin/bash

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURR_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))
ROOT_DIR := $(CURR_DIR)

ECHO = echo
MKDIR = mkdir -p
RM = rm -rf


IDBGPDL_DIR := $(shell cd ..; pwd)/iDbgPdl
PDLIB_DIR := $(shell cd ..; pwd)/PDLIB

export FAST_COMPILATION_FLAG := -j$(shell awk '/^processor/{n+=1}END{print n}' /proc/cpuinfo)

.PHONY: all
all: wm bm gm


wm: 
	$(MAKE) -f Makefile_wm $(FAST_COMPILATION_FLAG) lib
	
gm: 
	$(MAKE) -f Makefile_bm $(FAST_COMPILATION_FLAG) GREY_MODE=yes

bm: 
	$(MAKE) -f Makefile_bm $(FAST_COMPILATION_FLAG)
	
clean:
	$(MAKE) -f Makefile_wm clean
	$(MAKE) -f Makefile_bm clean
	$(MAKE) -f Makefile_bm GREY_MODE=yes clean

.PHONY: no_targets__ list
no_targets__:
list:
	@sh -c "make -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | egrep -v '__\$$|Makefile|make|\%' | sort"

############################################################
bm_%::
	$(MAKE) -f Makefile_bm $(patsubst bm_%,%,$@)
	
wm_%::
	$(MAKE) -f Makefile_wm $(patsubst wm_%,%,$@)
############################################################

