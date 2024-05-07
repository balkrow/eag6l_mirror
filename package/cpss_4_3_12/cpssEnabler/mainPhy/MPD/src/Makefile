MODULES := \

FILES := \
mpd.c \
mpdInit.c \
mpdUtils.c \
mpdDebug.c
 
ifeq ($(INCLUDE_MTD),yes)
FILES += mpdmtd.c
endif
ifeq ($(INCLUDE_MYD),yes)
FILES += mpdmyd.c
endif
ifeq ($(INIT_FROM_XML),yes)
MODULES += parser
endif


CWD := $(dir $(lastword $(MAKEFILE_LIST)))
PHY_SRC += $(patsubst %,$(CWD)%,$(FILES))
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

