###########################################################
# Function definitions
###########################################################

#allow the caller to specify additional CFLAGS
#CPSS_USER_CFLAGS hold multiple 'CFLAGS' , defined as 'export CPSS_USER_CFLAGS="..."'
#example : export CPSS_USER_CFLAGS="-fPIC" before calling the 'make'
CFLAGS += $(CPSS_USER_CFLAGS)
VERSION_FLAGS-y := FAMILY=$(FAMILY)

ifeq ($(OS),Windows_NT)
SHELL = cmd.exe
MKDIR = mkdir
else
SHELL = /bin/bash
AWK = /usr/bin/awk
MKDIR = mkdir -p
SOURCE = source
endif

#---- uppercase function to convert string to uppercase ----------
uppercase_TABLE:=a,A b,B c,C d,D e,E f,F g,G h,H i,I j,J \
    k,K l,L m,M n,N o,O p,P q,Q r,R s,S t,T u,U v,V w,W x,X y,Y z,Z

define uppercase_internal
$(if $1,$$(subst $(firstword $1),$(call uppercase_internal,$(wordlist 2,$(words $1),$1),$2)),$2)
endef

define uppercase
$(eval uppercase_RESULT:=$(call uppercase_internal,$(uppercase_TABLE),$1))$(uppercase_RESULT)
endef
#-----------------------------------------------------------------

SIM_STANDALONE_SRC:= \
########################   Preperations  ##########################
###                                                             ###
###################################################################

# Name of makefile to recursively include in each sub-module
MAKEFILE := _Makefile

DEP :=

ECHO = echo
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip
ELFSIZE = $(CROSS_COMPILE)size
CP = cp

AR_FLAGS=crus

# Prepend a few empty lines gap when calling the makefile
$(info  )
$(info  )

# Verify target is provided: armv7, armv8, sim32, sim64, ia64
# This is relevant only for targets: appDemo
ifeq ("$(TARGET)", "")
ifeq ("$(MAKECMDGOALS)", "$(filter $(MAKECMDGOALS), appDemo modules cpssLib cpssLibExt cpsslibs)")
  $(error TARGET setting missing)
endif
endif

##########################   config    ############################
###                                                             ###
###################################################################
CONFIG ?= .config
# Include configuration file, if exists
-include $(CONFIG)

ifneq ("$(TARGET)", "")
include tools/build/targets/$(TARGET).mk
ifeq ($(OS_RUN), win32)
 TOOL_TYPE ?= vc
else
 TOOL_TYPE ?= gcc
endif
endif

include config.mk

# Dual build is used for compiling  shared library in both aslr and non-aslr mode.
# Not supported for armv7.

ifeq ($(CONFIG_SHARED_MEMORY),y)
 ifeq ($(TARGET),armv7be)
  DUAL_BUILD=n
 else ifeq ($(TARGET),armv7)
  DUAL_BUILD=n
 else
  DUAL_BUILD=y
 endif
endif

######################## compile ##################################
###                                                             ###
###  define compiler commands as per the TOOL_TYPE              ###
###                                                             ###
###################################################################

ifeq ($(OS), Windows_NT)
# compile using msvsc tools
# create list of include headers from VC environment variable "INCLUDE"
# This is used by makedepend
INCLUDE_VC_MD := -I"$(strip $(subst ;," -I",$(INCLUDE)))"

#-----------------------------------------------------------------
# Function compile(dep_cflags, compile_cflags, output_folder)
#
# Compile a .c file into .o file using the defined toolchain, including dependecy .d file.
# Object and dependency files will be located together in output_folder.
# cflags        - CFLAGS to use for creating object file
# output_folder - object file ouptut folder path prefix. Only files
#                 which should be created in this folder, will use
#                 this recipe.
#
# Example: $(call compile $(CFLAGS),/local/objects)
# Targets: /local/objects/foo/bar/baz.o
#          /local/objects/foo/bar/baz.d
# Source file: foo/bar/baz.c
#-----------------------------------------------------------------

define compile
	-if not exist $(dir $@) ($(MKDIR) $(subst /,\,$(dir $@)))
	$(if $(filter-out $(DEPENDENCY),DEP_NO),makedepend -f- -o.o -s# -p$(BUILD_FOLDER)/ -D_M_IX86 -D_WIN32 -- $(INCLUDE_VC_MD) $(CFLAGS) -- $< > $2/$*.d,)
	$(CL) -c $(VC_CFLAGS) $1 $< -$(O)o$@
endef

else
# compile using gcc tools
define compile
@# create folder for dependency file
	$(MKDIR) $(dir $@)
@# create dependency file
	$(if $(filter-out $(DEPENDENCY),DEP_NO),$(CC) -M -c $1 $< -o$2/$*.d,)
@# Duplicate it
	$(if $(filter-out $(DEPENDENCY),DEP_NO),cp $2/$*.d $2/$*.tmp,)
@# Create empty rule per file in dependency rule to avoid "No rule to make target ..." Errors
	$(if $(filter-out $(DEPENDENCY),DEP_NO),sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/d' -e 's/$$/ :/' < $2/$*.d >> $2/$*.tmp,)
@# Change object filename to include full path
	$(if $(filter-out $(DEPENDENCY),DEP_NO),sed -e 's/$(subst .,\.,$(notdir $@))/$(subst /,\/,$@)/'  < $2/$*.tmp > $2/$*.d,)
@# Remove temporary file
	$(if $(filter-out $(DEPENDENCY),DEP_NO),rm $2/$*.tmp,)
@# print compilation msg
	@ $(ECHO) CC $<
@# compile
	$(CL) -c $(VC_CFLAGS) $1 $< -$(O)o$@
endef

endif

ENABLER             := appDemo

# If it is a Cpss ASK(Application Starter kit)  build..
ifeq (y, $(CONFIG_ASK))
  LINUX_CPSS_APP_REF  := YES
  LINUX_CPSS_APP_LIB  := YES
  CFLAGS              += -DPOSIX_SEM -DCPSS_APP_PLATFORM
  ifeq (y, $(CONFIG_MIXED_MODE))
    CFLAGS            += -DMIXED_MODE
  endif

  ifeq ($(CONFIG_SHARED_MEMORY),y)
    BUILD_CPSS_APP_PLAT := EXE_SHARED
  else
    BUILD_CPSS_APP_PLAT := EXE
  endif

  ifeq (y, $(CONFIG_ASK_LIB_BUILD))
    BUILD_CPSS_APP_PLAT := LIB
  else
    CFLAGS += -DCPSS_APP_PLATFORM_REFERENCE
  endif
endif

ifeq ($(CONFIG_DISABLE_CLI_PRINTS), y)
    CFLAGS += -DDISABLE_CLI_SERVICE
endif

ifeq ($(REMOVE_CLI_PRINTS), y)
    CFLAGS += -DREMOVE_CLI_PRINTS
endif
ifeq ($(CONFIG_PIC), y)
    CFLAGS += -DPIC_BUILD
endif

# Figure out paths to various source and target folders
# #####################################################
MY_DIR = $(shell pwd)
VERSION_DATE = $(shell date "+%Y-%m-%d %T")

# List of targets for "help" printout
ifneq ($(OS),Windows_NT)
 targets := $(shell ls tools\/build\/targets | sed 's/\(.*\)\.mk/ \\*   \1/g')
else
 targets := $(shell dir /B tools\build\targets)
 targets := $(basename  $(targets))
endif

# Create build folder name according to build options, unless provided.
# By default, build folder is ./compilation_root
# Create sub-folders in the following format:
# compilation_root/branch/target_family_postfix,
# postfix describes build type (GM, shared, etc)
###############################################################
ifneq ("$(TARGET)", "")
# Default path to cpss build folder (compilation_root), considering git
# In case make is called with -C, compilation_root should be based at caller path
ifneq ($(CALLER_PATH),)
 override CALLER_PATH := $(CALLER_PATH)/
endif

ifeq ($(OS),Windows_NT)
 ISGIT := $(shell if exist .git echo Y)
 BUILD_FOLDER ?= $(CALLER_PATH)compilation_root
else
 ISGIT := $(shell git rev-parse --git-dir 2> /dev/null)
 # Check if freeRTOS folder exist
 IS_FREERTOS := $(shell ls extension/srvCpu/firmware/internal/build_free_rtos.sh 2>/dev/null)
endif

ifneq ($(ISGIT),)
 BRANCH := /$(shell git rev-parse --abbrev-ref HEAD)
endif
ifeq ($(CONFIG_SHARED_MEMORY), y)
 POSTFIX += _SHARED
endif
ifneq ($(CONFIG_GM),)
 POSTFIX += _GM_$(CONFIG_GM)
endif

ifneq ($(OS),Windows_NT)
 BUILD_FOLDER := $(CALLER_PATH)compilation_root$(BRANCH)/$(TARGET)_$(FAMILY)$(POSTFIX)
else
 override BUILD_FOLDER := $(BUILD_FOLDER)$(BRANCH)/$(TARGET)_$(FAMILY)$(POSTFIX)
endif
$(info BUILD_FOLDER = $(BUILD_FOLDER))
endif

# Information about kernel source / headers, and kernel modules.
# Relevant only for building kernel modules and / or kernel image.
KERNEL_FOLDER ?= ../kernel
MODULES_FOLDER := $(MY_DIR)/cpssEnabler/mainExtDrv/src/gtExtDrv/linuxNoKernelModule/drivers


############### Host specific settings ############################
###                                                             ###
###  Set compiler and corresponding flags for different hosts   ###
###                                                             ###
###################################################################

ifeq (y, $(CONFIG_ASIC_SIMULATION))
    simFilesUsed=1
endif
#check if we forbid the use of the simulation files ! even if the ASIC_SIMULATION is used !
ifeq (y, $(CONFIG_ASIC_SIMULATION_ENV_FORBIDDEN))
    simFilesUsed=0
endif

ifeq ($(TOOL_TYPE), vc)
 # Flags to support windows simulation based on Visual Studio
 ############################################################
 # cl is name of VC compiler
 CL := cl
 # cl expects -Fo to denote output (instead of -o)
 O := F
 LD := link
 AR = lib
else
 ##### Linux GNU GCC #####
 ifeq ($(CONFIG_DEBUG_INFO), y)
  CFLAGS += -g -O0
 else
  CFLAGS += -O2
 endif
 ifeq ($(CONFIG_V2_DMA), y)
    ifeq ($(CONFIG_ASIC_SIMULATION), y)
        $(info CONFIG_V2_DMA disabled)
    else
        CFLAGS += -DCONFIG_V2_DMA
    endif
 endif
 ifeq ($(CONFIG_RMON), y)
    ifeq ($(CONFIG_ASIC_SIMULATION), y)
        $(info CONFIG_RMON disabled)
    else
        CFLAGS += -DCONFIG_RMON
    endif
 endif
  ifeq (1, $(simFilesUsed))
  CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare -Wno-old-style-declaration
 endif
 ECHO_CC = $(ECHO)
 CL := $(CC)
 PIE := $(shell $(CC) -dM -E - < /dev/null | grep pie)
 ifneq ($(PIE),)
  PIE := -no-pie
 endif
 CAST := $(shell $(CC) --help=warnings | grep cast-function-type)
 ifneq ($(CAST),)
  CAST := -Wno-cast-function-type
 endif
endif


######################## CPSS #####################################
###                                                             ###
###  Create list of include paths, modules to build and CFLAGS  ###
###  according to provided make flags                           ###
###                                                             ###
###################################################################

CFLAGS += -DMCD_RUN_WITH_CPSS
CFLAGS += -DIMPL_PP -DGT_PCI -D_linux

ifeq ($(WIDTH), 64)
 CFLAGS += -D__WORDSIZE=64 -DIS_64BIT_OS
endif

ifneq ($(CONFIG_GM),)

 ifeq ($(OS),Windows_NT)
    GM_LIB ?= \\\\fileril103\dev\Objects\cpss\bin\lib\simulation\libs\GM\VC10\PpInterfaceLibs\$(GM)\PPInterface.lib
 else
  GM_LIB ?= $(shell find /swdev/fileril103/Objects/cpss/bin/lib/simulation/libs/GM/Linux/PpInterfaceLibs/$(GM) -name "lib*.a")
 endif

endif

CPSS_INC_PATH := common/h
CPSS_INC_PATH += common/h/cpssAppPlatform
CPSS_INC_PATH += common/h/cpss/extServices/os
CPSS_INC_PATH += common/h/cpss/common/labServices/port/gop/port
CPSS_INC_PATH += common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl
CPSS_INC_PATH += common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell/sd28firmware
CPSS_INC_PATH += common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/include
CPSS_INC_PATH += mainExtUtils/h
CPSS_INC_PATH += cpssEnabler/utfWrappers/h
CPSS_INC_PATH += cpssEnabler/demo/h
CPSS_INC_PATH += cpssEnabler/mainOs/h
CPSS_INC_PATH += cpssEnabler/mainSysConfig/h
CPSS_INC_PATH += cpssEnabler/mainExtDrv/h
CPSS_INC_PATH += cpssEnabler/mainExtMac/h
CPSS_INC_PATH += cpssEnabler/mainPhy
CPSS_INC_PATH += cpssEnabler/mainCmd/h
CPSS_INC_PATH += cpssEnabler/mainCmd/src
CPSS_INC_PATH += cpssEnabler/mainSysConfig/src/appDemo/phy
CPSS_INC_PATH += mainExtUtils/src/extUtils/luaCLI/lua-5.1/src
CPSS_INC_PATH += shlibTestClients/h
CPSS_INC_PATH += luaCLI_standalone/src/FS
CPSS_INC_PATH += cpssEnabler/cpssAppPlatform/h
CPSS_INC_PATH += cpssEnabler/ezMpdCommon/h
CPSS_INC_PATH += cpssEnabler/cpssAppUtils/h

ifeq ($(CONFIG_INCLUDE_MPD), y)
CPSS_INC_PATH += cpssEnabler/mainPhy/MPD/h
endif

APPDEMO_MODULES := \
  cpssEnabler    \
  mainExtUtils

CPSS_MODULES := common

###  Optional packages and corresponding CFLAGS  ###

ifneq (,$(findstring DX,$(FAMILY)))
 CPSS_MODULES += mainPpDrv
 CFLAGS += -DCHX_FAMILY

 CPSS_INC_PATH += mainPpDrv/h
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/port/gop/h
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/port/gop/h/port
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Driver
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Driver/ddr3
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Silicon
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/ddr/bap/h
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include/h/msApi
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include/h/platform
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include/h/driver
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/macSec/driver

 ifeq (,$(BUILD_CPSS_APP_PLAT))
  ifneq ($(OS),Windows_NT)
   INCLUDE_EXAMPLES := $(shell if [ -d referenceCode/examples ]; then echo Y; fi)
  else
   INCLUDE_EXAMPLES := $(shell if exist referenceCode\examples (echo Y))
  endif
 endif
endif
ifneq (,$(findstring PX,$(FAMILY)))
 CPSS_MODULES += mainPxDrv
 CFLAGS += -DPX_FAMILY -DGT_SMI
 CPSS_INC_PATH += mainPxDrv/h
 ifneq ($(OS),Windows_NT)
  INCLUDE_REFERENCE_HAL := $(shell if [ -d referenceCode/referenceHal ]; then echo Y; fi)
 else
  INCLUDE_REFERENCE_HAL := $(shell if exist referenceCode\referenceHal (echo Y))
 endif
 ifeq ($(INCLUDE_REFERENCE_HAL), Y)
  CPSS_INC_PATH += referenceCode/referenceHal/h referenceCode/referenceHal/px/bpe/h
  CFLAGS += -DREFERENCEHAL_BPE_EXISTS
 endif
endif

ifneq ($(INCLUDE_EXAMPLES)$(INCLUDE_REFERENCE_HAL),)
 APPDEMO_MODULES += referenceCode
endif

ifeq ($(CONFIG_INCLUDE_UTF), y)
 APPDEMO_MODULES += mainUT
 INCLUDE_ENHANCED_UTF := y
 CPSS_INC_PATH += mainUT/utfTool/h
 CPSS_INC_PATH += mainUT/mainPpDrv/h
 CPSS_INC_PATH += mainUT/utfTraffic/h
 CPSS_INC_PATH += mainUT/mainPxDrv/h
endif

ifeq ($(CONFIG_INCLUDE_STACK_EXTENSION), y)
 ifeq ($(OS_RUN),linux)
  APPDEMO_MODULES += extension
 endif
else
 ifneq ($(EXT_MODULE),)
  APPDEMO_MODULES += extension #Add extension directory for non-empty 'EXT_MODULE'
 endif
endif

ifeq ($(CONFIG_INCLUDE_MACSEC_APP), y)
  APPDEMO_MODULES += extension
endif

ifeq ($(CONFIG_INCLUDE_GALTIS), y)
 APPDEMO_MODULES += mainGaltisWrapper
 CPSS_INC_PATH += mainGaltisWrapper/h
else
 EXCLUDE_GALTIS := EXCLUDE_LIB
 # Needed for TxRx
 CPSS_INC_PATH += mainGaltisWrapper/h
endif

ifeq ($(CONFIG_CMD_LUA_CLI), y)
 APPDEMO_MODULES += mainLuaWrapper
 CPSS_INC_PATH += mainLuaWrapper/h
 ifneq ($(EXT_MODULE),)
  CFLAGS += -DCMDFS_RAM_EMBEDDED_DATA_EXT_XML_EXT
 endif
 ifeq ($(CONFIG_INCLUDE_GALTIS), y)
  ifneq ($(OS),Windows_NT)
   INCLUDE_EMBEDDED_CMD := $(shell if [ -d embeddedCommands ]; then echo Y; else echo N; fi)
  else
   INCLUDE_EMBEDDED_CMD := $(shell if exist embeddedCommands (echo Y))
  endif
  ifeq ($(INCLUDE_EMBEDDED_CMD), Y)
   APPDEMO_MODULES += embeddedCommands
   CPSS_INC_PATH += embeddedCommands/h embeddedCommands/dx/dxEC/h embeddedCommands/px/pxEC/h
   CFLAGS += -DEMBEDDED_COMMANDS_EXISTS
  endif
 endif
endif
ifeq (1, $(simFilesUsed))
 CPSS_MODULES += simulation
ifeq ($(CONFIG_SHARED_MEMORY), y)
 CPSS_MODULES+=simStandalone
endif
CFLAGS += -DASIC_SIMULATION
ifeq ($(CONFIG_INCLUDE_MPD), y)
CFLAGS += -DPHY_SIMULATION
endif
 SIM := Sim
 CPSS_INC_PATH += simulation/simGeneral/h
 CPSS_INC_PATH += simulation/simDevices/h
 CPSS_INC_PATH += simulation/simOs/h
 CPSS_INC_PATH += simulation/simDevices/src/asicSimulation/SKernel/sEmbeddedCpu/firmware/pipe_Pha/inc
 CPSS_INC_PATH += simulation/simGeneral/h/common/SHOST
 CPSS_INC_PATH += simulation/simDevices/src/asicSimulation/SKernel/sEmbeddedCpu/cm3/h
 SHLIB_LDFLAGS := -Xlinker --wrap -Xlinker calloc -Xlinker --wrap -Xlinker free -Xlinker --wrap -Xlinker malloc \
                  -Xlinker --wrap -Xlinker realloc
else
 LINUX_NOKM ?= YES
endif

##
ifeq ($(CONFIG_INCLUDE_EZ_BRINGUP), y)
 CPSS_INC_PATH += cpssEnabler/ez_bringup/pdl/h
 CPSS_INC_PATH += cpssEnabler/ez_bringup/iDbgPdl/h
 CONFIG_INCLUDE_PDLIB=y
endif

ifeq ($(CONFIG_INCLUDE_PDLIB), y)
 CPSS_INC_PATH += cpssEnabler/ez_bringup/PDLIB/h
endif

CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/phy/MPD/h
CFLAGS += -DMPDPREFIX=cpss
##
ifeq ($(CONFIG_INCLUDE_TM), y)
 CPSS_MODULES-$(CONFIG_INCLUDE_TM) += mainTmDrv
 CPSS_INC_PATH += mainTmDrv/h
 CPSS_INC_PATH += mainTmDrv/h/private
 CPSS_INC_PATH += mainTmDrv/h/core
 CPSS_INC_PATH += mainTmDrv/h/platform
endif
ifeq ($(OS_RUN),linux)
 CFLAGS += -DLINUX -DV2L_PTHREADS
endif
ifeq ($(LINUX_NOKM), YES)
 CFLAGS += -DLINUX_NOKM -DNOKM_DRV_EMULATE_INTERRUPTS
endif
ifneq (, $(filter $(STACK_SIZE_LIMIT), Y R))
ifneq ($(shell gcc --help=common|grep stack-usage|wc -l), 0)
CFLAGS += -fstack-usage
STACK_SIZE_LIMIT_REPORT := 4096
else
override STACK_SIZE_LIMIT := N
endif
else
endif

ifeq ($(CONFIG_SHARED_MEMORY), y)
 LUA_MODULES := luaCLI_standalone
 SHARED_CFLAGS := -fPIC -DPIC -D$(SHARED_LIB_LD_ARCH)
 LUACLI_CFLAGS := -DENV_POSIX -DCLI_STANDALONE
 CPSS_SPLIT_LIBRARIES = 1

 SHLIB_LDFLAGS := -shared $(SHLIB_LDFLAGS) $(TARGET_SPECIFIC_CFLAGS)
 LDFLAGS := -Xlinker -z -Xlinker nocopyreloc -rdynamic -lpthread -lrt -ldl -lm $(TARGET_SPECIFIC_CFLAGS)
else
 LDFLAGS := -lrt -lpthread -lm -ldl -rdynamic $(LDFLAGS-y) $(TARGET_SPECIFIC_CFLAGS)
endif

ifeq (LIB,$(BUILD_CPSS_APP_PLAT))
#CAP requires separate libs for os and extdrv - which requires the below flag to be turned on
CPSS_SPLIT_LIBRARIES = 1
endif

CPSS_MODULES += $(CPSS_MODULES-y)

################### Create list of sources ########################
###                                                             ###
###  Recursively dive into source folder tree and return with   ###
###  lists of source *.c files and updated CFLAGS               ###
###                                                             ###
###################################################################
ifneq ("$(MAKECMDGOALS)", "clean")
 ifneq ("$(MAKECMDGOALS)", "cpssLib")
  include $(patsubst %,%/$(MAKEFILE),$(CPSS_MODULES) $(APPDEMO_MODULES) $(LUA_MODULES))
 else
  include $(patsubst %,%/$(MAKEFILE),$(CPSS_MODULES) $(LUA_MODULES))
 endif
endif
ifeq ($(CONFIG_SHARED_MEMORY), y)
 APPDEMO_MODULES := $(filter-out cpssEnabler , $(APPDEMO_MODULES)) # In shared lib mode cpssEnabler is split to sub modules
endif
CONFIG_CFLAGS-$(CONFIG_CMD_FS) += -DCMD_FS

# CFLAGS may be updated by recursive Makefiles, so this is a good location to gather it all up
ifeq ($(TOOL_TYPE), vc)
CFLAGS := $(TARGET_SPECIFIC_CFLAGS) $(addprefix -I, $(CPSS_INC_PATH)) $(CFLAGS) $(CONFIG_CFLAGS-y)
VC_CFLAGS := -nologo -Ot -Zi $(VC_ADD_CFLAGS) -Fd"$(BUILD_FOLDER)/"
ifneq ($(CONFIG_GM),)
  VC_CFLAGS += -MTd
endif
else
CFLAGS := -Wall -Wextra -Werror -Wno-error=cpp $(CAST) -ansi $(TARGET_SPECIFIC_CFLAGS) -fno-builtin -funroll-loops \
           $(SHARED_CFLAGS) $(addprefix -I, $(CPSS_INC_PATH)) $(CFLAGS) $(CONFIG_CFLAGS-y)
#-Wno-cast-function-type  -Wextra
endif

################# Create list of objects and libs #################
###                                                             ###
###################################################################

# List of source file packages, based on modules
# Should look like:  CPSSENABLER_SRC   MAINPPDRV_SRC   MAINEXTUTILS_SRC ...
SRC_PACKAGES := $(addsuffix _SRC,$(call uppercase,$(CPSS_MODULES) $(APPDEMO_MODULES)))

ifeq (LIB,$(BUILD_CPSS_APP_PLAT))
CPSS_MODULES += mainExtUtils \
                mainLuaWrapper

LIB_CPSS_ENABLER_MODULES := cpssAppRef mainCmd demo utfWrappers gtOs gtStack gtUtil mainExtDrv extPhyM
CPSS_OBJ_LIBS := $(foreach M,$(CPSS_MODULES),$(BUILD_FOLDER)/$(M)/libs/$(call uppercase,$(M)).a)
CPSS_ENABLER_OBJ_LIBS := $(foreach M,$(LIB_CPSS_ENABLER_MODULES),$(BUILD_FOLDER)/cpssEnabler/libs/$(call uppercase,$(M)).a)
OBJ_LIBS := $(CPSS_OBJ_LIBS) $(CPSS_ENABLER_OBJ_LIBS)

# List of objects per library
SRC_PACKAGES_SUB_MODULES := $(addprefix cpssEnabler_, $(addsuffix _SRC1, $(LIB_CPSS_ENABLER_MODULES) ))
$(foreach S,$(SRC_PACKAGES_SUB_MODULES),$(eval $(call uppercase, $(patsubst cpssEnabler_%_SRC1,%_OBJ,$(S))) := $(patsubst %.c,$(BUILD_FOLDER)/%.o,$($(S)))))
else
# List of object libraries, full path. Each library corresponds to a source package
# Should look like: $(BUILD_FOLDER)/cpssEnabler/libs/CPSSENABLER.a $(BUILD_FOLDER)/mainPpDrv/libs/MAINPPDRV.a  ...
CPSS_OBJ_LIBS := $(foreach M,$(CPSS_MODULES),$(BUILD_FOLDER)/$(M)/libs/$(call uppercase,$(M)).a)
APPDEMO_OBJ_LIBS := $(foreach M,$(APPDEMO_MODULES),$(BUILD_FOLDER)/$(M)/libs/$(call uppercase,$(M)).a)
OBJ_LIBS := $(CPSS_OBJ_LIBS) $(APPDEMO_OBJ_LIBS)
endif

# Create list of objects per object library, based on corresponding SRC package.
# This evaluates to something like:
# COMMON_OBJ :=  $(BUILD_FOLDER)/common/.../cpssHwInit.o  $(BUILD_FOLDER)/common/.../prvCpssHwMultiPortGroups.o  ...
# MAINPPDRV_OBJ :=  $(BUILD_FOLDER)/mainPpDrv/.../prvCpssGenBrgFdbIsrSig.o  $(BUILD_FOLDER)/mainPpDrv/.../prvCpssGenBrgLog.o  ...
$(foreach S,$(SRC_PACKAGES),$(eval $(patsubst %_SRC,%_OBJ,$(S)) := $(patsubst %.c,$(BUILD_FOLDER)/%.o,$($(S)))))

# Same for shared memory sub modules
ifeq ($(CONFIG_SHARED_MEMORY), y)
 LIB_CPSS_MODULES := mainExtDrv shrMemIntraCPSSdata # For libcpss.a
 LIB_HELPER_MODULES := gtOs gtStack gtUtil mainExtDrvShared # For libhelper.a
 APPDEMO_SHARED_MODULES := utfWrappers demo confi extPhyM luaCLI mainCmd mainPhy mainSysConfig cpssAppRef# dragonite
ifeq ($(CONFIG_INCLUDE_EZ_BRINGUP), y)
 APPDEMO_SHARED_MODULES += ez_bringup ezMpdCommon
 EZB_SRC := $(shell find cpssEnabler/ez_bringup/ -name *.c|grep -v  pdlParserParams.c)
 EZB_SRC += cpssEnabler/ezMpdCommon/src/ezBringupTools.c
ifeq ($(CONFIG_INCLUDE_MPD), y)
 EZB_SRC += cpssEnabler/ezMpdCommon/src/mpdTools.c
endif
 EZB_OBJ := $(patsubst %.c, $(BUILD_FOLDER)/%.o, $(EZB_SRC))
endif
 LIB_CPSS_OBJ_LIBS := $(foreach M,$(LIB_CPSS_MODULES),$(BUILD_FOLDER)/cpssEnabler/libs/$(call uppercase,$(M)).a)
 LIB_HELPER_OBJ_LIBS := $(foreach M,$(LIB_HELPER_MODULES),$(BUILD_FOLDER)/cpssEnabler/libs/$(call uppercase,$(M)).a)
 APPDEMO_SHARED_OBJ_LIBS := $(foreach M,$(APPDEMO_SHARED_MODULES),$(BUILD_FOLDER)/cpssEnabler/libs/$(call uppercase,$(M)).a)

 # List of objects per library
 SRC_PACKAGES_SUB_MODULES := $(addprefix cpssEnabler_, $(addsuffix _SRC1, $(LIB_CPSS_MODULES) $(LIB_HELPER_MODULES) $(APPDEMO_SHARED_MODULES)))
 $(foreach S,$(SRC_PACKAGES_SUB_MODULES),$(eval $(call uppercase, $(patsubst cpssEnabler_%_SRC1,%_OBJ,$(S))) := $(patsubst %.c,$(BUILD_FOLDER)/%.o,$($(S)))))

 LUACLI_STANDALONE_OBJ := $(patsubst %.c, $(BUILD_FOLDER)/%.o, $(LUACLI_STANDALONE_SRC))
 LUACLI_STANDALONE_OBJ += $(patsubst %.c, $(BUILD_FOLDER)/luaCLI_standalone/%.o, $(MAINLUAWRAPPER_LUA_STANDALONE_SRC))
 LUACLI_STANDALONE_OBJ += $(patsubst %.c, $(BUILD_FOLDER)/luaCLI_standalone/%.o, $(EXTENSION_LUA_STANDALONE_SRC))
 #for shell-execute support
 LUACLI_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainCmd/src/cmdShell/os/linux/os_lxExecute.o
ifeq ($(CONFIG_INCLUDE_EZ_BRINGUP), y)
 LUACLI_STANDALONE_OBJ += $(EZB_OBJ)

ifeq ($(CONFIG_INCLUDE_MPD), y)
 LUACLI_STANDALONE_OBJ += $(MAINPHY_OBJ)
endif
endif
 SHARED_MEMORY_REF_APPS := $(BUILD_FOLDER)/luaCLI
 ifneq (,$(findstring DX,$(FAMILY)))
  SHARED_MEMORY_REF_APPS += $(BUILD_FOLDER)/RxTxProcess $(BUILD_FOLDER)/fdbLearning
  ifeq ($(CONFIG_PIC), y)
   ifeq ($(DUAL_BUILD),y)
    SHARED_MEMORY_REF_APPS_NOASLR := $(BUILD_FOLDER)/luaCLI_noaslr
   endif
  endif
 endif
ifeq ($(CONFIG_ASIC_SIMULATION),y)
SHARED_MEMORY_REF_APPS += $(BUILD_FOLDER)/simStandAlone
endif
ifeq ($(CONFIG_INCLUDE_EZ_BRINGUP), y)
$(BUILD_FOLDER)/cpssEnabler/libs/EZ_BRINGUP.a: $(EZB_OBJ)
endif
endif

# Full list of all source files to be compiled
SRC := $(foreach S, $(SRC_PACKAGES) $(SRC_PACKAGES_SUB_MODULES), $($(S)))

# List of all object and dependency files
OBJ := $(patsubst %.c,$(BUILD_FOLDER)/%.o,$(SRC))
ifneq (DEP_NO, $(DEPENDENCY))
DEP += $(patsubst %.o,%.d,$(OBJ))
endif

# Create dependency of each lib.a in corresponding LIB_OBJ list
$(foreach lib,$(OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .A,,$(call uppercase,$(lib))))_OBJ)))
$(foreach lib,$(LIB_CPSS_OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .A,,$(call uppercase,$(lib))))_OBJ)))
$(foreach lib,$(LIB_HELPER_OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .A,,$(call uppercase,$(lib))))_OBJ)))
$(foreach lib,$(APPDEMO_SHARED_OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .A,,$(call uppercase,$(lib))))_OBJ)))
$(BUILD_FOLDER)/luaCLI_standalone/libs/LUACLI_STANDALONE.a: $(LUACLI_STANDALONE_OBJ)

############################ Targets ##############################
###                                                             ###
###################################################################

$(BUILD_FOLDER):
	$(MKDIR) $@

clean_all: clean_cpss clean_modules clean_fws
	rm -f $(BUILD_FOLDER)/*.img

help_fws:
	@echo
	@echo " LINKED_FW_n= options are:"
	@echo
	@echo " BC2, BOBK, msServiceCpuCm3Bc3Fw, mvHwsServiceCpuCm3Ac5Fw, mvHwsServiceCpuCm3AldrinFw"
	@echo " mvHwsServiceCpuCm3Aldrin2Fw, mvHwsServiceCpuCm3RavenFw, mvHwsServiceCpuCm3BobKFw"
	@echo " mvHwsServiceCpuCm3PipeFw, mvHwsServiceCpuCm3Aldrin2Fw"

help:
	@echo
	@echo "CPSS build system"
	@echo "================="
	@echo
	@echo "Usage: make TARGET=<target> FAMILY=<DX / PX / DXPX> [CROSS_COMPILE=<toolchain>] <make_goal>"
	@echo
	@echo "Optional flags for $(ENABLER) build (and default value):"
	@echo "   [DEBUG_INFO=n] [INCLUDE_UTF=n] [INCLUDE_GALTIS=n] [SHARED_MEMORY=n] [INCLUDE_TM=n]"
	@echo "   [CMD_LUA_CLI=y] [LOG_ENABLE=y] [INCLUDE_EZ_BRINGUP=n] [INCLUDE_MPD=n]"
	@echo "   [API_LOCK_PROTECTION=y] [CPSS_USE_MUTEX_PROFILER=n] [GM=falcon/hawk/ironman/seahawk]"
	@echo "   [TRAFFIC_API_LOCK_DISABLE=n] [DEPENDENCY=DEP_NO] [STACK_SIZE_LIMIT=n/[y/r]]"
	@echo "   [LINKED_FW_1 / 2 / 3 /4=[fw_name]]"
	@echo " "
	@echo "Optional flags for kernel modules build:"
	@echo "   [KERNEL_FOLDER=]"
	@echo " "
	@echo "target" represents file name from cpss/tools/build/targets:
	@echo $(targets)
	@echo " "
	@echo "List of make goals:"
	@sh -c "make -p no_targets__ -f Makefile : 2>/dev/null | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$|Makefile|make' | sort"
	@echo


ifeq ($(OS_RUN), win32)

# Check if using VC10 or VC10_64bit to choose correct shost / slan library files
ifeq ($(Platform), X64)
LIB_ARCH := _64bit
endif

# Change name of libs from lib.a to lib.lib
OBJ_LIBS := $(patsubst %.a, %.lib, $(OBJ_LIBS))

# Create dependency of each lib.lib in corresponding LIB_OBJ list
$(foreach lib,$(OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .LIB,,$(call uppercase,$(lib))))_OBJ)))

ifeq ($(TOOL_TYPE), vc)

ifeq ($(OS), Windows_NT)
FILER_LIB_PATH := //fileril103/dev/objects/cpss/bin/lib/simulation/libs
SIM_LIB_PATH := simulation/extern_lib

$(BUILD_FOLDER)/simulation/libs/shost/SHOST.lib:
	@ if not exist $(dir $@) ($(MKDIR) $(subst /,\,$(dir $@)))
	@ if exist $(subst /,\,$(FILER_LIB_PATH)\shost\VC10$(LIB_ARCH)\SHOST.lib) (cp $(FILER_LIB_PATH)/shost/VC10$(LIB_ARCH)/SHOST.lib $@) \
		else (if exist $(subst /,\,$(SIM_LIB_PATH)\shost\VC10$(LIB_ARCH)\SHOST.lib) (cp $(SIM_LIB_PATH)/shost/VC10$(LIB_ARCH)/SHOST.lib $@) \
		else (echo *** Error: cannot find SHOST.lib))

$(BUILD_FOLDER)/simulation/libs/slan/SLAN.lib:
	@ if not exist $(dir $@) ($(MKDIR) $(subst /,\,$(dir $@)))
	@ if exist $(subst /,\,$(FILER_LIB_PATH)\slan\VC10$(LIB_ARCH)\SLAN.lib) (cp $(FILER_LIB_PATH)/slan/VC10$(LIB_ARCH)/SLAN.lib $@) \
		else (if exist $(subst /,\,$(SIM_LIB_PATH)\slan\VC10$(LIB_ARCH)\SLAN.lib) (cp $(SIM_LIB_PATH)/slan/VC10$(LIB_ARCH)/SLAN.lib $@) \
		else (echo *** Error: cannot find SLAN.lib))

%.lib:
	@ $(ECHO) AR $@
	@ if not exist $(dir $@) ($(MKDIR) $(subst /,\,$(dir $@)))
	@ echo "-nologo" -ignore:4006 > $@.list
	@ for %%i in ($^) do @echo %%i >> $@.list
	@ echo "-out:$@" >> $@.list
	@ $(AR) @$@.list
#	echo "dumpbin -symbols $@ > $(dir $@)$(notdir $@).list"

else

#
# Target copy for SLAN and SHOST on Linux Host
#

%.lib:
	@ $(ECHO) AR $@
	$(MKDIR) $(dir $@)
	@ $(AR) /NOLOGO /IGNORE:4006 /OUT:$@ $^
endif

else
# Tool Type GCC
%.lib:
	@ $(ECHO) AR $@
	$(MKDIR) $(dir $@)
	@ $(AR) crs $@ $^

endif

clean:
	rm -rf $(BUILD_FOLDER)

else # Linux GNU

%.a:
	@ $(ECHO) AR $@
	$(MKDIR) $(dir $@)
	$(AR) $(AR_FLAGS) $@ $^

clean:
	@ if [[ $(BUILD_FOLDER) != / ]]; then rm -rf $(BUILD_FOLDER); fi
	@ rm $(CONFIG)

clean_fws:
	@ ./extension/srvCpu/firmware/internal/build_free_rtos.sh clean

endif

.PHONY: $(BUILD_FOLDER)/version_info.c
$(BUILD_FOLDER)/version_info.c:
	echo const char *VERSION_DATE = '"$(VERSION_DATE)"'';'  >  $@
	echo const char *VERSION_FLAGS = '"$(VERSION_FLAGS-y)"'';'  >> $@


ifeq (1, $(simFilesUsed))

 #   Simulation
 ######################
 # generate map file
 # 1. Generate list
 # 2. Create .c file
ifeq ($(OS), Windows_NT)
symtable.c: $(OBJ) $(OBJ_LIBS)
	echo " " > $(BUILD_FOLDER)/symtable.list
	for %%l in ($(OBJ_LIBS)); do dumpbin -nologo -symbols %%l >> $(BUILD_FOLDER)/symtable.list
	awk -f tools/bin/vc_mksymtbl.awk $(BUILD_FOLDER)/symtable.list > symtable.c

clean_exe:
	rm -f $(BUILD_FOLDER)/$(ENABLER).*

$(ENABLER).exe: clean_exe $(BUILD_FOLDER)/symtable.o $(BUILD_FOLDER)/simulation/libs/shost/SHOST.lib $(BUILD_FOLDER)/simulation/libs/slan/SLAN.lib
	@ $(ECHO) LD $@
	$(LD) -nologo -opt:noref -subsystem:console $(OBJ_LIBS) $(filter-out clean_exe,$^) $(GM_LIB) \
	User32.lib WinMM.Lib OLDNAMES.lib WSock32.Lib Shlwapi.lib \
	-LARGEADDRESSAWARE -NODEFAULTLIB:LIBCMT \
	-out:$(BUILD_FOLDER)/$@ -DEBUG -map:$(BUILD_FOLDER)/$(@:.exe=.map)
	@ $(ECHO) EXE file : $(subst /,\,$(BUILD_FOLDER))\$@
#libucrt.lib mincore.lib
cpsslibs: $(OBJ) $(OBJ_LIBS)
	@ $(ECHO) libraries are ready:
	@ for %%l in ($(OBJ_LIBS)); do $(ECHO) %%l
else
#    On a Linux Host
########################
symtable.c: $(OBJ) $(OBJ_LIBS)
	echo " " > $(BUILD_FOLDER)/symtable.list
	for lib in $(OBJ_LIBS); do nm -g $$lib >> $(BUILD_FOLDER)/symtable.list; done
	awk -f tools/bin/gcc_mksymtbl.awk $(BUILD_FOLDER)/symtable.list > symtable.c

$(ENABLER).exe: $(BUILD_FOLDER)/symtable.o $(BUILD_FOLDER)/simulation/libs/shost/SHOST.lib $(BUILD_FOLDER)/simulation/libs/slan/SLAN.lib
	@ $(ECHO) LD $@
	$(LD) -Wl,--start-group $(OBJ_LIBS) -Wl,--end-group $^ $(GM_LIB) \
	-luser32  -lwinmm -lwsock32 -lshlwapi \
	-o $(BUILD_FOLDER)/$@
#libucrt.lib mincore.lib

cpsslibs: $(OBJ) $(OBJ_LIBS)
	@ $(ECHO) libraries are ready:
	@ $(ECHO) $(OBJ_LIBS) | fmt -w 0
endif

else
 #   BM
 ######################
 $(BUILD_FOLDER)/symtable.c: $(OBJ) $(OBJ_LIBS)

fws:
	@./extension/srvCpu/firmware/internal/build_free_rtos.sh;

endif


ifneq ($(CONFIG_LINKED_FW_1),)
  CFLAGS += -DCONFIG_LINKED_FW_1='"'$(CONFIG_LINKED_FW_1).h'"' -DCONFIG_LINKED_FW_1_INFO=$(CONFIG_LINKED_FW_1)_INFO
endif
ifneq ($(CONFIG_LINKED_FW_2),)
  CFLAGS += -DCONFIG_LINKED_FW_2='"'$(CONFIG_LINKED_FW_2).h'"' -DCONFIG_LINKED_FW_2_INFO=$(CONFIG_LINKED_FW_2)_INFO
endif
ifneq ($(CONFIG_LINKED_FW_3),)
  CFLAGS += -DCONFIG_LINKED_FW_3='"'$(CONFIG_LINKED_FW_3).h'"' -DCONFIG_LINKED_FW_3_INFO=$(CONFIG_LINKED_FW_3)_INFO
endif
ifneq ($(CONFIG_LINKED_FW_4),)
  CFLAGS += -DCONFIG_LINKED_FW_4='"'$(CONFIG_LINKED_FW_4).h'"' -DCONFIG_LINKED_FW_4_INFO=$(CONFIG_LINKED_FW_4)_INFO
endif


ifneq ($(CONFIG_SRDS_LINKED_FW_1),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_1='"'$(CONFIG_SRDS_LINKED_FW_1).h'"' -DCONFIG_SRDS_LINKED_FW_1_INFO=$(CONFIG_SRDS_LINKED_FW_1)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_2),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_2='"'$(CONFIG_SRDS_LINKED_FW_2).h'"' -DCONFIG_SRDS_LINKED_FW_2_INFO=$(CONFIG_SRDS_LINKED_FW_2)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_3),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_3='"'$(CONFIG_SRDS_LINKED_FW_3).h'"' -DCONFIG_SRDS_LINKED_FW_3_INFO=$(CONFIG_SRDS_LINKED_FW_3)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_4),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_4='"'$(CONFIG_SRDS_LINKED_FW_4).h'"' -DCONFIG_SRDS_LINKED_FW_4_INFO=$(CONFIG_SRDS_LINKED_FW_4)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_5),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_5='"'$(CONFIG_SRDS_LINKED_FW_5).h'"' -DCONFIG_SRDS_LINKED_FW_5_INFO=$(CONFIG_SRDS_LINKED_FW_5)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_6),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_6='"'$(CONFIG_SRDS_LINKED_FW_6).h'"' -DCONFIG_SRDS_LINKED_FW_6_INFO=$(CONFIG_SRDS_LINKED_FW_6)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_7),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_7='"'$(CONFIG_SRDS_LINKED_FW_7).h'"' -DCONFIG_SRDS_LINKED_FW_7_INFO=$(CONFIG_SRDS_LINKED_FW_7)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_8),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_8='"'$(CONFIG_SRDS_LINKED_FW_8).h'"' -DCONFIG_SRDS_LINKED_FW_8_INFO=$(CONFIG_SRDS_LINKED_FW_8)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_9),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_9='"'$(CONFIG_SRDS_LINKED_FW_9).h'"' -DCONFIG_SRDS_LINKED_FW_9_INFO=$(CONFIG_SRDS_LINKED_FW_9)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_10),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_10='"'$(CONFIG_SRDS_LINKED_FW_10).h'"' -DCONFIG_SRDS_LINKED_FW_10_INFO=$(CONFIG_SRDS_LINKED_FW_10)_INFO
endif
ifneq ($(CONFIG_SRDS_LINKED_FW_11),)
  CFLAGS += -DCONFIG_SRDS_LINKED_FW_11='"'$(CONFIG_SRDS_LINKED_FW_11).h'"' -DCONFIG_SRDS_LINKED_FW_11_INFO=$(CONFIG_SRDS_LINKED_FW_11)_INFO
endif


ifeq ($(CONFIG_INCLUDE_MPD), y)
 MPD_INC_PATH := cpssEnabler/mainPhy/MPD/h
 MPD_INC_PATH += cpssEnabler/mainPhy/MPD/external_phy/mtd/src
 MPD_INC_PATH += cpssEnabler/mainPhy/MPD/external_phy/alaska_c/src
 MPD_INC_PATH += cpssEnabler/mainPhy/MPD/external_phy/mtd/src/serdes/mcesd
 MPD_INC_PATH += cpssEnabler/mainPhy/MPD/external_phy/mtd/src/serdes/mcesd/C28GP4X2
 MPD_INC_PATH += cpssEnabler/mainPhy/MPD/external_phy/mtd/src/LinkCryptPtp
 MPD_FLAGS := $(addprefix -I, $(MPD_INC_PATH)) -DMPD -DINCLUDE_MPD -DMTD_DEBUG -DMTD_DEBUG_FUNC -DMTD_PHY_INCLUDE -DMYD_PHY_INCLUDE
 ifeq (y, $(CONFIG_ASIC_SIMULATION))
  MPD_FLAGS += -DPHY_SIMULATION
 endif
endif


# Build the MPD submodule under cpssEnabler to avoid conflict
$(BUILD_FOLDER)/cpssEnabler/%.o: cpssEnabler/%.c
	$(call compile, $(MPD_FLAGS) $(filter-out -DMPDPREFIX=cpss,$(CFLAGS)), $(BUILD_FOLDER)/cpssEnabler)

$(BUILD_FOLDER)/mainLuaWrapper/src/ez_bringup/%.o: mainLuaWrapper/src/ez_bringup/%.c
	$(call compile, $(MPD_FLAGS) $(filter-out -DMPDPREFIX=cpss,$(CFLAGS)), $(BUILD_FOLDER)/mainLuaWrapper/src/ez_bringup)

ifeq ($(CONFIG_SHARED_MEMORY), y)
$(BUILD_FOLDER)/luaCLI_standalone/%.o: luaCLI_standalone/%.c
	$(call compile, $(filter-out -DENV_MAINCMD,$(CFLAGS)) $(LUACLI_CFLAGS), $(BUILD_FOLDER)/luaCLI_standalone)
$(BUILD_FOLDER)/luaCLI_standalone/%.o: %.c
	$(call compile, $(filter-out -DENV_MAINCMD,$(CFLAGS)) $(LUACLI_CFLAGS), $(BUILD_FOLDER)/luaCLI_standalone)
ifneq ($(CONFIG_PIC), n)
$(ENABLER): $(ENABLER)_aslr $(ENABLER)_noaslr
$(ENABLER)_aslr: $(BUILD_FOLDER)/libcpss.so $(BUILD_FOLDER)/libhelper.so $(APPDEMO_OBJ_LIBS) $(APPDEMO_SHARED_OBJ_LIBS) $(BUILD_FOLDER)/version_info.o $(SHARED_MEMORY_REF_APPS)
	@ $(ECHO) LD $(ENABLER)
	$(LD) -Xlinker --whole-archive $(filter %.a %.o, $^) -pie -fPIE -Xlinker --no-whole-archive \
	$(filter %.so, $^) $(LDFLAGS) -Xlinker -Map -Xlinker $(BUILD_FOLDER)/$@.map -o $(BUILD_FOLDER)/$(ENABLER)
ifeq ($(DUAL_BUILD),y)
$(ENABLER)_noaslr: $(BUILD_FOLDER)/libcpss_noaslr.so $(BUILD_FOLDER)/libhelper_noaslr.so $(APPDEMO_OBJ_LIBS) $(APPDEMO_SHARED_OBJ_LIBS) $(BUILD_FOLDER)/version_info.o $(SHARED_MEMORY_REF_APPS_NOASLR)
	@ $(ECHO) LD $@
	-$(LD) -Xlinker --whole-archive $(filter %.a %.o, $^) -Xlinker --no-whole-archive \
	$(filter %.so, $^) $(LDFLAGS) -Xlinker -Map -Xlinker $(BUILD_FOLDER)/$@.map -o $(BUILD_FOLDER)/$@
else
.PHONY :$(ENABLER)_noaslr
endif

else
$(ENABLER): $(BUILD_FOLDER)/libcpss.so $(BUILD_FOLDER)/libhelper.so $(APPDEMO_OBJ_LIBS) $(APPDEMO_SHARED_OBJ_LIBS) $(BUILD_FOLDER)/version_info.o $(SHARED_MEMORY_REF_APPS)
	@ $(ECHO) LD $@
	$(LD) -Xlinker --whole-archive $(filter %.a %.o, $^) -Xlinker --no-whole-archive \
	$(filter %.so, $^) $(LDFLAGS) -Xlinker -Map -Xlinker $(BUILD_FOLDER)/$@.map -o $(BUILD_FOLDER)/$@
endif
ifeq ($(STACK_SIZE_LIMIT), R)
	@ $(ECHO) "Function with stack size larger than $(STACK_SIZE_LIMIT_REPORT)"
	$(AWK) '$$2 > $(STACK_SIZE_LIMIT_REPORT)' \
		$(shell find $(BUILD_FOLDER)/ -name "*.su")|\
		grep -e "static" -e "dynamic,bounded" || /bin/true
endif
else

$(OBJ): $(CONFIG)

cpssLib: $(CPSS_OBJ_LIBS)
	@ $(ECHO) libraries are ready:
	@ $(ECHO) $^ | fmt -w 0

cpssLibExt: $(OBJ_LIBS)
	@ $(ECHO) libraries are ready:
	@ $(ECHO) $^ | fmt -w 0


# Static link appDemo
ifeq (LIB,$(BUILD_CPSS_APP_PLAT))
$(ENABLER): $(BUILD_FOLDER) $(OBJ) $(OBJ_LIBS) $(BUILD_FOLDER)/version_info.o
else
$(ENABLER): $(BUILD_FOLDER) $(OBJ) $(OBJ_LIBS) $(BUILD_FOLDER)/version_info.o
	@ $(ECHO) LD $@
	$(LD) -Wl,--whole-archive $(OBJ_LIBS) -Wl,--no-whole-archive $(BUILD_FOLDER)/version_info.o $(GM_LIB) \
	$(LDFLAGS) -Xlinker -Map -Xlinker $(BUILD_FOLDER)/$@.map -o $(BUILD_FOLDER)/$@
# Produce stack limit exceed report
ifeq ($(STACK_SIZE_LIMIT), R)
	@ $(ECHO) "Function with stack size larger than $(STACK_SIZE_LIMIT_REPORT)"
	$(AWK) '$$2 > $(STACK_SIZE_LIMIT_REPORT)' \
		$(shell find $(BUILD_FOLDER)/ -name "*.su")|\
		grep -e "static" -e "dynamic,bounded" || /bin/true
endif
endif
ifeq ($(CONFIG_DEBUG_INFO), y)
# create stripped binary for target
	$(STRIP) $(BUILD_FOLDER)/$@ -o $(BUILD_FOLDER)/$@_stripped
endif
endif

	SIZE=`$(ELFSIZE) $(BUILD_FOLDER)/$@ | tail -1|  cut -f 4`; \
	printf "    *** \033[34;1m.text+.data+.bss size:  0x%x \033[0m\n" $$SIZE

$(BUILD_FOLDER)/%.o: %.c
	$(call compile, $(CFLAGS), $(BUILD_FOLDER))

common/h/cpss/common/srvCpu/fw_ids_gen.h : common/h/cpssCommon/private/fw_ids.h
	cat common/h/cpssCommon/private/fw_ids.h | tr -d '\044' > common/h/cpss/common/srvCpu/fw_ids_gen.h

# debug target. usage: make print-var will print value of variable "var"
print-%  : ; @echo $* = $($*)

ifneq (DEP_NO, $(DEPENDENCY))
-include $(DEP)
endif


# related to libhelper.so and libcpss.so

SCRIPTLD_ROOT := cpssEnabler/mainOs/src/gtOs/sharedMemory/scriptsld/linux$(SIM)
SCRIPTLD_CPSS_LIB := $(SCRIPTLD_ROOT)/libcpss_ld_script.$(SHARED_LIB_LD_ARCH)
SCRIPTLD_HELPER_LIB := $(SCRIPTLD_ROOT)/libhelper_ld_script.$(SHARED_LIB_LD_ARCH)
# PIC = position independent code
ifneq ("n", "$(CONFIG_PIC)")
$(BUILD_FOLDER)/libcpss.so: $(CPSS_OBJ_LIBS) $(LIB_CPSS_OBJ_LIBS)
	@ $(ECHO) LD $@
	$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map)  \
	-Xlinker -soname -Xlinker $(@F) -o $@

$(BUILD_FOLDER)/libhelper.so: $(LIB_HELPER_OBJ_LIBS)
	@ $(ECHO) LD $@
	$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map)  \
	-Xlinker -soname -Xlinker $(@F) -o $@

$(BUILD_FOLDER)/libcpss_noaslr.so: $(CPSS_OBJ_LIBS) $(LIB_CPSS_OBJ_LIBS)
	@ $(ECHO) LD $@
	-$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map) -T $(SCRIPTLD_CPSS_LIB) \
	-Xlinker -soname -Xlinker $(@F) -o $@

$(BUILD_FOLDER)/libhelper_noaslr.so: $(LIB_HELPER_OBJ_LIBS)
	@ $(ECHO) LD $@
	-$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map)  -T $(SCRIPTLD_HELPER_LIB) \
	-Xlinker -soname -Xlinker $(@F) -o $@

else
$(BUILD_FOLDER)/libcpss.so: $(CPSS_OBJ_LIBS) $(LIB_CPSS_OBJ_LIBS)
	@ $(ECHO) LD $@
	$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map) -T $(SCRIPTLD_CPSS_LIB) \
	-Xlinker -soname -Xlinker $(@F) -o $@

$(BUILD_FOLDER)/libhelper.so: $(LIB_HELPER_OBJ_LIBS)
	@ $(ECHO) LD $@
	$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map) -T $(SCRIPTLD_HELPER_LIB) \
	-Xlinker -soname -Xlinker $(@F) -o $@


endif
$(BUILD_FOLDER)/RxTxProcess: $(BUILD_FOLDER)/shlibTestClients/src/RxTxProcess.o $(BUILD_FOLDER)/mainExtUtils/src/extUtils/rxEventHandler/rxEventHandler.o  $(BUILD_FOLDER)/libcpss.so $(BUILD_FOLDER)/libhelper.so
	@ $(ECHO) LD $@
	$(LD) $^ $(LDFLAGS) -Xlinker -Map -Xlinker $@.map -o $@

$(BUILD_FOLDER)/fdbLearning: $(BUILD_FOLDER)/shlibTestClients/src/fdbLearning.o $(BUILD_FOLDER)/mainExtUtils/src/extUtils/auEventHandler/auEventHandler.o $(BUILD_FOLDER)/libcpss.so $(BUILD_FOLDER)/libhelper.so
	@ $(ECHO) LD $@
	$(LD) $^ $(LDFLAGS) -Xlinker -Map -Xlinker $@.map -o $@

$(BUILD_FOLDER)/luaCLI: $(BUILD_FOLDER)/libcpss.so $(BUILD_FOLDER)/libhelper.so $(BUILD_FOLDER)/luaCLI_standalone/libs/LUACLI_STANDALONE.a $(BUILD_FOLDER)/mainExtUtils/libs/MAINEXTUTILS.a
	@ $(ECHO) LD $@
	$(LD) -Xlinker --whole-archive $(filter %.a, $^) -pie -fPIE -Xlinker --no-whole-archive \
	$(filter %.so, $^) $(LDFLAGS) -Xlinker -Map -Xlinker $@.map -o $@

ifneq ($(CONFIG_PIC), n)
$(BUILD_FOLDER)/luaCLI_noaslr: $(BUILD_FOLDER)/libcpss_noaslr.so $(BUILD_FOLDER)/libhelper_noaslr.so $(BUILD_FOLDER)/luaCLI_standalone/libs/LUACLI_STANDALONE.a $(BUILD_FOLDER)/mainExtUtils/libs/MAINEXTUTILS.a
	@ $(ECHO) LD $@
	-$(LD) -Xlinker --whole-archive $(filter %.a, $^) -Xlinker --no-whole-archive \
	$(filter %.so, $^) $(LDFLAGS) -Xlinker -Map -Xlinker $@.map -o $@
endif


SIM_STANDALONE_OBJ := $(patsubst %.c, $(BUILD_FOLDER)/%.o, $(SIM_STANDALONE_SRC))
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/simulation/libs/SIMULATION.a

$(BUILD_FOLDER)/simStandAlone: $(SIM_STANDALONE_OBJ)
	@ $(ECHO) LD $@;\
    $(shell  touch $(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  truncate -s 0  $(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "#!/bin/bash">$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "workdir=\`dirname \$$0\`">>$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "export LD_LIBRARY_PATH=\$$workdir">>$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "instanceNumber=\$$((1 +\$$RANDOM % 100))">>$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "ps=\$$((1000 + \$$RANDOM % 2000))">>$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "pa=\$$((3000 + \$$RANDOM % 4000))">>$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "export CPSS_SHR_INSTANCE_NUM=\$$instanceNumber">>$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "\$$workdir/simStandAlone \"\$$@\" -clients 1 -pS \$$ps -pA \$$pa &">>$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "sleep 3">>$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  echo "exec \$$workdir/appDemo \"\$$@\" -client 0 -pS \$$ps -pA \$$pa">>$(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  chmod +x $(BUILD_FOLDER)/$(ENABLER)_dist)\
    $(shell  touch $(BUILD_FOLDER)/$(ENABLER)_legacy)\
    $(shell  truncate -s 0  $(BUILD_FOLDER)/$(ENABLER)_legacy)\
    $(shell  echo "#!/bin/bash">$(BUILD_FOLDER)/$(ENABLER)_legacy)\
    $(shell  echo "workdir=\`dirname \$$0\`">>$(BUILD_FOLDER)/$(ENABLER)_legacy)\
    $(shell  echo "export LD_LIBRARY_PATH=\$$workdir">>$(BUILD_FOLDER)/$(ENABLER)_legacy)\
    $(shell  echo "exec \$$workdir/appDemo_noaslr \"\$$@\"">>$(BUILD_FOLDER)/$(ENABLER)_legacy)\
    $(shell  chmod +x $(BUILD_FOLDER)/$(ENABLER)_legacy)\
    $(LD) $^ $(LDFLAGS) -Xlinker -Map -Xlinker $@.map -o $@

######################## KERNEL MODULES ###############################
modules: $(KERNEL_FOLDER)/.config
	$(MAKE) -C $(KERNEL_FOLDER) LOCALVERSION= M=$(MODULES_FOLDER) clean
	$(MAKE) -C $(KERNEL_FOLDER) LOCALVERSION= M=$(MODULES_FOLDER) $(MODULE_FLAGS) modules

clean_modules:
	$(MAKE) -C $(KERNEL_FOLDER) LOCALVERSION= M=$(MODULES_FOLDER) clean

