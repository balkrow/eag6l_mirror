# Main Makefile for Ros Over linux (BM + WM)
#
SHELL := /bin/bash

BASE_DIR := $(shell pwd)
PDL_PATH := pdl
IDBG_PDL_PATH := iDbgPdl
PDLIB_LIB := PDLIB

.PHONY: all wm bm gm clean 
all: wm bm gm

wm: wm_pdl wm_iDbgPdl wm_PDLIB
bm: bm_pdl bm_iDbgPdl bm_PDLIB
gm: gm_pdl gm_iDbgPdl gm_PDLIB

wm_%: 
	$(MAKE) -C $(patsubst wm_%,%,$@) wm
	
bm_%: 
	$(MAKE) -C $(patsubst bm_%,%,$@) bm
	
gm_%: 
	$(MAKE) -C $(patsubst gm_%,%,$@) gm

clean: 
	$(MAKE) -C $(PDL_PATH) clean
	$(MAKE) -C $(PDLIB_LIB) clean
	$(MAKE) -C $(IDBG_PDL_PATH) clean

.PHONY: no_targets__ list
no_targets__: 
list:
	@sh -c "make -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | egrep -v '__\$$|Makefile|make|\%' | sort"
		
############################################################
