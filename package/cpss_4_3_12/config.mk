

config_list := \
 CONFIG_TARGET \
 CONFIG_INCLUDE_TM \
 CONFIG_INCLUDE_UTF \
 CONFIG_INCLUDE_GALTIS \
 CONFIG_SHARED_MEMORY \
 CONFIG_ASIC_SIMULATION \
 CONFIG_ASIC_SIMULATION_ENV_FORBIDDEN \
 CONFIG_INCLUDE_EZ_BRINGUP \
 CONFIG_INCLUDE_PDLIB \
 CONFIG_GM \
 CONFIG_DEBUG_INFO \
 CONFIG_ASK \
 CONFIG_ASK_LIB_BUILD \
 CONFIG_MIXED_MODE \
 CONFIG_DISABLE_CLI_PRINTS \
 CONFIG_EXT_MODULE \
 CONFIG_PTP_PLUGIN_SUPPOR \
 CONFIG_TRAFFIC_API_LOCK_DISABLE \
 CONFIG_USE_MUTEX_PROFILER \
 CONFIG_STACK_SIZE_LIMIT \
 CONFIG_LOG_ENABLE \
 CONFIG_INCLUDE_MPD \
 CONFIG_CMD_FS \
 CONFIG_CMD_LUA_CLI \
 CONFIG_API_LOCK_PROTECTION \
 CONFIG_PIC \
 CONFIG_SSE_ENABLED \
 CONFIG_V2_DMA \
 CONFIG_LINKED_FW_1 \
 CONFIG_LINKED_FW_2 \
 CONFIG_LINKED_FW_3 \
 CONFIG_LINKED_FW_4 \
 CONFIG_RMON \
 CONFIG_INCLUDE_MACSEC_APP


# Parse command line parameters:
cli_config_list := $(patsubst CONFIG_%,%,$(config_list))
# If .config does not exist and parameter is set in command line, override defaults
ifeq (,$(wildcard $(CONFIG)))
 $(info $(CONFIG) deleted)
 $(foreach S,$(cli_config_list),$(if $($(S)),$(eval CONFIG_$(S) = $($(S)))))
else
 # Check if configuration has changed since last build
 # First case - item set to 'y' in .config, and declared 'n' in CLI
 cli_config := $(foreach S,$(cli_config_list),$(if $(filter $($(S))$(CONFIG_$(S)),ny),yes))
 # Second case - item not set in .config, and declared 'y' in CLI
 cli_config += $(foreach S,$(cli_config_list),$(if $(filter $($(S)),y),$(if $(filter $($(S))$(CONFIG_$(S)),y),yes)))
 # 3rd case - GM changed
 ifneq ($(CONFIG_GM),$(GM))
  cli_config += yes
 endif
 ifneq ($(CONFIG_TARGET),$(TARGET))
  cli_config += yes
 endif

 ifneq ($(strip $(cli_config)),)
  $(info $(CONFIG) changed)
  $(shell rm $(CONFIG))
 endif
 $(foreach S,$(cli_config_list),$(if $($(S)),$(eval CONFIG_$(S) = $($(S)))))
 # Display list of enabled flags
 config_print := $(shell grep y $(CONFIG))
 $(info $(config_print))
endif

# Constants
value_list := \
 CONFIG_PRV_CPSS_MAX_PP_DEVICES_CNS \
 CONFIG_CPSS_MAX_PORTS_NUM_CNS


# Default options
CONFIG_TARGET ?= $(TARGET)
CONFIG_LOG_ENABLE ?= y
CONFIG_PIC ?= y
CONFIG_CMD_LUA_CLI ?= y
CONFIG_API_LOCK_PROTECTION ?= y
CONFIG_V2_DMA ?= y

# cpss KM capabilities defaults
CONFIG_KM_MVPCI ?= y
CONFIG_KM_MVDMA2 ?= y
CONFIG_KM_MVDMA ?= n
CONFIG_KM_MVINT ?= y
CONFIG_KM_MVMBUS ?= y
CONFIG_KM_MVETH ?= y
MODULE_FLAGS := \
 CONFIG_KM_MVPCI=$(CONFIG_KM_MVPCI) \
 CONFIG_KM_MVDMA2=$(CONFIG_KM_MVDMA2) \
 CONFIG_KM_MVDMA=$(CONFIG_KM_MVDMA) \
 CONFIG_KM_MVINT=$(CONFIG_KM_MVINT) \
 CONFIG_KM_MVMBUS=$(CONFIG_KM_MVMBUS) \
 CONFIG_KM_MVETH=$(CONFIG_KM_MVETH)

# Default values
CONFIG_PRV_CPSS_MAX_PP_DEVICES_CNS ?= 128
CONFIG_CPSS_MAX_PORTS_NUM_CNS ?= 1024
CONFIG_DMA_BLK_SIZE_NUM_MB ?= 2
# Falcon without mvcpss.ko requires to increase the memory pre-allocation
# to at least 6MB. This can be forced from the Makefile by invoking the
# Makefile with additional parameter of CONFIG_DMA_LEN=6
CONFIG_DMA_LEN ?= 4

config := $(foreach S,$(filter-out CONFIG_GM CONFIG_TARGET,$(config_list)),$(if $(filter $($(S)),y),"echo $S=y;", "echo '\#' $S is not set;"))
config += $(foreach S,$(value_list),"echo $S=$($(S));")

# GM requires special care - not a y/n value
ifeq ($(GM),)
 config += "echo '\#' CONFIG_GM is not set;"
else
 config += "echo CONFIG_GM=$(GM);"
endif
config += "echo CONFIG_TARGET=$(TARGET);"

#  Add flag per configuration item
CONFIG_CFLAGS-y :=
CONFIG_CFLAGS-$(CONFIG_INCLUDE_TM) += -DINCLUDE_TM -DIMPL_TM
CONFIG_CFLAGS-$(CONFIG_LOG_ENABLE) += -DCPSS_LOG_ENABLE
CONFIG_CFLAGS-$(CONFIG_API_LOCK_PROTECTION) += -DCPSS_API_LOCK_PROTECTION
CONFIG_CFLAGS-$(CONFIG_SHARED_MEMORY) += -DPOSIX_SEM -DSHARED_MEMORY $(PIE)
CONFIG_CFLAGS-$(CONFIG_TRAFFIC_API_LOCK_DISABLE) += -DCPSS_TRAFFIC_API_LOCK_DISABLE
CONFIG_CFLAGS-$(CONFIG_USE_MUTEX_PROFILER) += -DCPSS_USE_MUTEX_PROFILER
CONFIG_CFLAGS-$(CONFIG_PTP_PLUGIN_SUPPORT) += -DPTP_PLUGIN_SUPPORT
CONFIG_CFLAGS-$(CONFIG_INCLUDE_UTF) += -DINCLUDE_UTF -DIMPL_TGF
CONFIG_CFLAGS-$(CONFIG_INCLUDE_EZ_BRINGUP) += -DEZ_BRINGUP
CONFIG_CFLAGS-$(CONFIG_INCLUDE_PDLIB) += -DPDLIB
CONFIG_CFLAGS-$(CONFIG_INCLUDE_MPD) += -DMPD -DINCLUDE_MPD -DMTD_DEBUG
CONFIG_CFLAGS-$(CONFIG_INCLUDE_GALTIS) += -DIMPL_GALTIS
CONFIG_CFLAGS-$(CONFIG_CMD_LUA_CLI) += -DCMD_LUA_CLI -DCMD_LUA_NET -DCMD_LUA_GLOBAL -DCMD_LUA_GENWRAPPER
CONFIG_CFLAGS-$(CONFIG_CMD_LUA_CLI) += -DCMDFS_RAM_EMBEDDED_DATA_EXT -DCMDFS_RAM_EMBEDDED_DATA_EXT_EXAMPLES
CONFIG_CFLAGS-$(CONFIG_CMD_LUA_CLI) += -DCMDFS_RAM_EMBEDDED_DATA_EXT_XML -DENV_MAINCMD
# Allow to configure the sub-block size allocation by the DMAv2 mechanism:
# e.g. make CONFIG_DMA_BLK_SIZE_NUM_MB=4
CONFIG_CFLAGS-y += -DCONFIG_DMA_BLK_SIZE_NUM_MB=$(CONFIG_DMA_BLK_SIZE_NUM_MB)
# Falcon without mvcpss.ko requires to increase the memory pre-allocation
# to at least 6MB. This can be forced from the Makefile by invoking the
# Makefile with additional parameter of CONFIG_DMA_LEN=6
CONFIG_CFLAGS-y += -DCONFIG_DMA_LEN=$(CONFIG_DMA_LEN)

CONFIG_CFLAGS-$(CONFIG_ASIC_SIMULATION) += -DASIC_SIMULATION
CONFIG_CFLAGS-$(CONFIG_ASIC_SIMULATION_ENV_FORBIDDEN) += -DASIC_SIMULATION_ENV_FORBIDDEN

CONFIG_CFLAGS-$(CONFIG_SSE_ENABLED) += -DSSE_ENABLED

CONFIG_CFLAGS-$(CONFIG_INCLUDE_STACK_EXTENSION) += -DSTACK_EXTENSION

CONFIG_CFLAGS-y += $(foreach S,$(patsubst CONFIG_%,%,$(value_list)),-D$(S)=$(CONFIG_$(S)))

ifneq ($(CONFIG_GM),)
 CONFIG_ASIC_SIMULATION := y
 CONFIG_CFLAGS-y += -DGM_USED
 LDFLAGS-y += -lstdc++
endif
VERSION_FLAGS-$(CONFIG_INCLUDE_TM) += INCLUDE_TM
VERSION_FLAGS-$(CONFIG_INCLUDE_UTF) += INCLUDE_UTF
VERSION_FLAGS-$(CONFIG_INCLUDE_GALTIS) += INCLUDE_GALTIS
VERSION_FLAGS-$(CONFIG_SHARED_MEMORY) += SHARED
VERSION_FLAGS-$(CONFIG_INCLUDE_EZ_BRINGUP) += EZ_BRINGUP
VERSION_FLAGS-$(CONFIG_INCLUDE_PDLIB) += PDLIB
VERSION_FLAGS-$(CONFIG_GM) += GM
VERSION_FLAGS-$(CONFIG_INCLUDE_MPD) += INCLUDE_MPD
VERSION_FLAGS-$(CONFIG_TRAFFIC_API_LOCK_DISABLE) += TRAFFIC_API_LOCK_DISABLE
VERSION_FLAGS-$(CONFIG_USE_MUTEX_PROFILER) += USE_MUTEX_PROFILER
VERSION_FLAGS-$(CONFIG_CMD_LUA_CLI) += CMD_LUA_CLI
VERSION_FLAGS-$(CONFIG_PIC) += PIC
VERSION_FLAGS-$(CONFIG_SSE_ENABLED) += SSE_ENABLED
VERSION_FLAGS-$(CONFIG_V2_DMA) += V2_DMA
VERSION_FLAGS-$(CONFIG_INCLUDE_STACK_EXTENSION) += STACK_EXTENSION
VERSION_FLAGS-$(CONFIG_RMON) += RMON

$(CONFIG):
ifeq ($(OS), Windows_NT)
	@ echo $(subst ',,$(subst ",,$(config))) > tmp$(INDEX).bat
	@ sed -i -e 's/;\s\+/>>$(CONFIG)\n/g' tmp$(INDEX).bat
else
	@ echo $(config) > tmp$(INDEX).bat
	@ sed -i -e 's/;\s*/>>$(CONFIG)\n/g' tmp$(INDEX).bat
endif
	@ rm -f $(CONFIG)
	@ $(SOURCE) tmp$(INDEX).bat
	@ rm -f tmp$(INDEX).bat

