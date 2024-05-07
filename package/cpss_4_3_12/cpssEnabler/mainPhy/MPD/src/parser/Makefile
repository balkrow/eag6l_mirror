MODULES := 

FILES := \
 mpdParserGlobal.c \
 auto/mpdParser.c

CWD := $(dir $(lastword $(MAKEFILE_LIST)))
PHY_SRC += $(patsubst %,$(CWD)%,$(FILES))
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

