#
SHELL := /bin/bash

DEBUG_MODE ?= yes
INCLUDE_MYD ?= no
INCLUDE_MTD ?= yes
INIT_FROM_XML ?= yes
INCLUDE_PDLIB ?= no
PDLIB_LIB := PDLIB

ifeq ("$(DEBUG_MODE)", "yes")
export DEBUG_MODE=yes
endif

ifeq ("$(INCLUDE_MYD)", "yes")
export INCLUDE_MYD=yes
endif

ifeq ("$(INCLUDE_MTD)", "yes")
export INCLUDE_MTD=yes
endif

ifeq ("$(INIT_FROM_XML)", "yes")
export INIT_FROM_XML=yes
endif

ifeq ("$(INCLUDE_PDLIB)", "yes")
export INCLUDE_PDLIB=yes
else
export INCLUDE_PDLIB=no
endif

.PHONY: all doc
all:  bm gm 
#	wm

clean: wm_clean bm_clean gm_clean 	
ifeq ("$(INCLUDE_PDLIB)", "yes")
	$(MAKE) -C $(PDLIB_LIB) clean
endif

ifeq ("$(INCLUDE_PDLIB)", "yes")
bm:
	$(MAKE) -C $(PDLIB_LIB) bm
else
bm:
endif
	$(MAKE) -f Makefile_bm

gm:
	$(MAKE) -f Makefile_bm GREY_MODE=yes

ifeq ("$(INCLUDE_PDLIB)", "yes")
wm:
	$(MAKE) -C $(PDLIB_LIB) wm
else
wm:
endif
	$(MAKE) -f Makefile_wm
	
wm_clean:
	$(MAKE) -f Makefile_wm clean

bm_clean:
	$(MAKE) -f Makefile_bm clean

gm_clean:
	$(MAKE) -f Makefile_bm GREY_MODE=yes clean

gm_sample:
	$(MAKE) -C $(PDLIB_LIB) gm
	$(MAKE) -f Makefile_bm GREY_MODE=yes INCLUDE_PDLIB=yes sample

baseline:
	@version=`git describe --abbrev=0 --always --tags` && sed -i -E 's/(\w+\s+MPD_GIT_VERSION\s*).*/\1'$$version'/g' h/mpdVersion.h

doc:
	doxygen Doxyfile
	cp doc_resources/*.* doc/html/.

.PHONY: no_targets__ list
no_targets__:
list:
	@sh -c "make -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | egrep -v '__\$$|Makefile|make|\%' | sort"

