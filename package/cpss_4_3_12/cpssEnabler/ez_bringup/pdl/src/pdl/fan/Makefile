MODULES := 

FILES := \
 pdlFan.c \
 pdlFanAdt7476.c \
 pdlFanTc654.c \
 pdlFanEMC2305.c \
 pdlFanPWM.c \
 pdlFanNCT7802.c

CWD := $(dir $(lastword $(MAKEFILE_LIST)))
PDL_SRC += $(patsubst %,$(CWD)%,$(FILES))
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

