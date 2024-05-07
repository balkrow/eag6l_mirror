MODULES := \
 init \
 button \
 lib \
 sensor \
 interface \
 packet_processor \
 fan \
 serdes \
 phy \
 sfp \
 power \
 cpu \
 oob_port \
 led \
 parser \
 cpld

FILES := 

CWD := $(dir $(lastword $(MAKEFILE_LIST)))
PDL_SRC += $(patsubst %,$(CWD)%,$(FILES))
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

