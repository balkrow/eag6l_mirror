
/*******************************************************************************
*              (c), Copyright 2020, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
 ********************************************************************************
 * @file PP-Harrier.c
 *
 * DESCRIPTION:
 *            HARRIER Packet Processor information.
 *
 *
 * FILE REVISION NUMBER:
 *       $Revision: 1 $
 *
*******************************************************************************/
#include <profiles/cpssAppPlatformProfile.h>

/* List of Tx queues to work in generator mode */
static CPSS_APP_PLATFORM_SDMA_QUEUE_STC txGenQueues[] = { {0,3,256,144}, {0,6,256,144} };

/* List of Pha Thread */
/*TODO:HA check these numbers for harrier*/
static CPSS_APP_PLATFORM_PHA_THREAD_LIST phaThread1 = {54, 46};
static CPSS_APP_PLATFORM_PHA_THREAD_LIST phaThread2 = {55, 47};
static CPSS_APP_PLATFORM_PHA_THREAD_LIST phaThread3 = {56, 48};
static CPSS_APP_PLATFORM_PHA_THREAD_LIST* phaThreads[] = {&phaThread1, &phaThread2, &phaThread3};

static CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC harrier_DB_PolarityArray[] =
{
    /* laneNum  invertTx    invertRx */
        { 11,   GT_FALSE,    GT_TRUE }
};

static CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC harrier_mac_ledPos_Arr[] =
{
     /* portMac           ledIf,    ledPosition    */
     {       0           , 0,     0            }
    ,{       1           , 0,     1            }
    ,{       2           , 0,     2            }
    ,{       3           , 0,     3            }
    ,{       4           , 0,     4            }
    ,{       5           , 0,     5            }
    ,{       6           , 0,     6            }
    ,{       7           , 0,     7            }
    ,{       8           , 0,     8            }
    ,{       9           , 0,     9            }
    ,{      10           , 0,    10            }
    ,{      11           , 0,    11            }
    ,{      12           , 0,    12            }
    ,{      13           , 0,    13            }
    ,{      14           , 0,    14            }
    ,{      15           , 0,    15            }
    ,{      16           , 0,     0            }
    ,{      17           , 0,     1            }
    ,{      18           , 0,     2            }
    ,{      19           , 0,     3            }
    ,{      20           , 0,     4            }
    ,{      21           , 0,     5            }
    ,{      22           , 0,     6            }
    ,{      23           , 0,     7            }
    ,{      24           , 0,     8            }
    ,{      25           , 0,     9            }
    ,{      26           , 0,    10            }
    ,{      27           , 0,    11            }
    ,{      28           , 0,    12            }
    ,{      29           , 0,    13            }
    ,{      30           , 0,    14            }
    ,{      31           , 0,    15            }
    ,{      32           , 0,     0            }
    ,{      33           , 0,     1            }
    ,{      34           , 0,     2            }
    ,{      35           , 0,     3            }
    ,{      36           , 0,     4            }
    ,{      37           , 0,     5            }
    ,{      38           , 0,     6            }
    ,{      39           , 0,     7            }
    ,{      40           , 0,     8            }
    ,{      41           , 0,     9            }
    ,{      42           , 0,    10            }
    ,{      43           , 0,    11            }
    ,{      44           , 0,    12            }
    ,{      45           , 0,    13            }
    ,{      46           , 0,    14            }
    ,{      47           , 0,    15            }
    ,{      48           , 0,     0            }
    ,{      49           , 0,     1            }
    ,{      50           , 0,     0            }
    ,{      51           , 0,     1            }
    ,{      52           , 0,     2            }
    ,{      53           , 0,     3            }
    ,{CAP_BAD_VALUE  , CAP_BAD_VALUE, CAP_BAD_VALUE  }
};
/*For WM simulation disable data integrity since this is time consuming */
#if defined ASIC_SIMULATION && defined SHARED_MEMORY
#define    NUM_OF_DATA_INTEGRITY_ELEMENTS_CNS 0x0
#else
#define    NUM_OF_DATA_INTEGRITY_ELEMENTS_CNS 0xFFFFFFFF
#endif

#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_ENABLE \
    _SM_(apEnable                   ) GT_FALSE,\
    _SM_(numOfDataIntegrityElements ) NUM_OF_DATA_INTEGRITY_ELEMENTS_CNS

/* Macro for PP Phase 2 Params */
#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS \
    _SM_(newDevNum                  ) 0, /* to be modified */ \
    _SM_(auDescNum                  ) 2048,\
    _SM_(auMessageLength            ) CPSS_AU_MESSAGE_LENGTH_8_WORDS_E,\
    _SM_(useDoubleAuq               ) GT_FALSE,\
    _SM_(useSecondaryAuq            ) GT_FALSE,\
    _SM_(fuDescNum                  ) 2048,\
    _SM_(fuqUseSeparate             ) CPSS_PP_PHASE2_UPLOAD_QUEUE_SEPARATE_E,\
    _SM_(noTraffic2CPU              ) GT_FALSE,\
    _SM_(cpuPortMode                ) CPSS_NET_CPU_PORT_MODE_SDMA_E,\
    _SM_(useMultiNetIfSdma          ) GT_TRUE,\
    _SM_(netifSdmaPortGroupId       ) 0,\
    _SM_(txDescNum                  ) 75,\
    _SM_(rxDescNum                  ) 75,\
    _SM_(rxAllocMethod              ) CPSS_RX_BUFF_STATIC_ALLOC_E,\
    _SM_(rxBuffersInCachedMem       ) GT_FALSE,\
    _SM_(rxHeaderOffset             ) 0,\
    _SM_(rxBufferPercentage         ) {7,7,7,7,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6, \
                                       6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},\
    _SM_(maxQueueNum                ) 16,\
    _SM_(txGenQueueList             ) txGenQueues,\
    _SM_(txGenQueueNum              ) sizeof(txGenQueues)/sizeof(txGenQueues[0]),\
    _SM_(rxPackets                  ) 75,\
    _SM_(txPackets                  ) 75,\
    _SM_(packetSize                 ) 10240,\
    /* NetIf Lib Init Params */\
    _SM_(miiTxDescNum               ) 0,\
    _SM_(miiTxBufBlockSize          ) 0,\
    _SM_(miiRxBufSize               ) 0,\
    _SM_(miiRxBufBlockSize          ) 0,\
    _SM_(miiRxHeaderOffset          ) 0,\
    _SM_(miiRxBufferPercentage      ) {7,7,7,7,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6, \
                                       6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6}

/* Macro for PP after Phase 2 Params */
#define CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS \
    /* Led Init */\
    _SM_(ledOrganize                ) CPSS_LED_ORDER_MODE_BY_CLASS_E,\
    _SM_(ledClockFrequency          ) CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E, /* 1627 for ledConfig.sip6LedConfig.ledClockFrequency */ \
    _SM_(invertEnable               ) GT_TRUE,\
    _SM_(pulseStretch               ) CPSS_LED_PULSE_STRETCH_1_E,\
    _SM_(blink0Duration             ) CPSS_LED_BLINK_DURATION_1_E,\
    _SM_(blink0DutyCycle            ) CPSS_LED_BLINK_DUTY_CYCLE_1_E,\
    _SM_(blink1Duration             ) CPSS_LED_BLINK_DURATION_1_E,\
    _SM_(blink1DutyCycle            ) CPSS_LED_BLINK_DUTY_CYCLE_1_E,\
    _SM_(disableOnLinkDown          ) GT_FALSE,\
    _SM_(clkInvert                  ) GT_FALSE,\
    _SM_(class5select               ) CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E,\
    _SM_(class13select              ) CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E,\
    _SM_(ledStart                   ) 64,\
    _SM_(ledEnd                     ) 64,\
    _SM_(sip6LedClockFrequency      ) 1627, \
    _SM_(ledClockPosEdgeModeEnable  ) GT_FALSE, \
    _SM_(ledPositionTable           ) harrier_mac_ledPos_Arr,\
    /* Led Class manipulation config */\
    _SM_(invertionEnable            ) GT_FALSE,\
    _SM_(blinkSelect                ) CPSS_LED_BLINK_SELECT_0_E,\
    _SM_(forceEnable                ) GT_FALSE,\
    _SM_(forceData                  ) 0,\
    _SM_(pulseStretchEnable         ) GT_FALSE

/* Macro for PP Lib Init Params */
#define CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS \
    _SM_(initBridge                 ) GT_TRUE,\
    _SM_(initIp                     ) GT_TRUE /*Greg*/,\
    _SM_(initMirror                 ) GT_TRUE /*Greg*/,\
    _SM_(initNetIf                  ) GT_TRUE /*Greg*/,\
    _SM_(initPhy                    ) GT_TRUE,\
    _SM_(initPort                   ) GT_TRUE,\
    _SM_(initTrunk                  ) GT_TRUE, /*Greg*/\
    _SM_(initPcl                    ) GT_TRUE,\
    _SM_(initTcam                   ) GT_TRUE,\
    _SM_(initPolicer                ) GT_TRUE,\
    _SM_(initPha                    ) GT_TRUE,\
    _SM_(initPtp                    ) GT_FALSE,\
    /* Ip Lib Init Params */\
    _SM_(blocksAllocationMethod     ) CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E,\
    _SM_(usePolicyBasedRouting      ) GT_FALSE,\
    _SM_(maxNumOfIpv4Prefixes       ) 1000,\
    _SM_(maxNumOfIpv4McEntries      ) 100,\
    _SM_(maxNumOfIpv6Prefixes       ) 100,\
    _SM_(lpmDbFirstTcamLine         ) 100,\
    _SM_(lpmDbLastTcamLine          ) 1003,\
    _SM_(lpmDbSupportIpv4           ) GT_TRUE,\
    _SM_(lpmDbSupportIpv6           ) GT_TRUE,\
    _SM_(lpmDbPartitionEnable       ) GT_TRUE,\
    /* Trunk Lib Init Params */\
    _SM_(numOfTrunks                ) 0x1000,\
    _SM_(trunkMembersMode           ) CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E,\
    /* Pha Lib Init Params */\
    _SM_(pha_packetOrderChangeEnable) GT_FALSE,\
    _SM_(phaFwImageId               ) 2, /*TODO:HA*/\
    _SM_(phaFwImageIdVersion        ) 554696704, /*TODO:HA*/\
    _SM_(phaThreadList              ) phaThreads, /*TODO:HA*/\
    /* Bridge Lib Init Params */\
    _SM_(policerMruSupported        ) GT_FALSE,\
    /* PTP Lib Init Params */\
    _SM_(outputInterfaceConf        ) 255,\
    _SM_(outputInterfaceMode        ) 0,\
    _SM_(nanoSeconds                ) 0,\
    _SM_(ptpOverEthernetEnable      ) GT_FALSE,\
    _SM_(etherType0value            ) 35063,\
    _SM_(etherType1value            ) 35063,\
    _SM_(ptpOverIpUdpEnable         ) GT_FALSE,\
    _SM_(udpPort0value              ) 319,\
    _SM_(udpPort1value              ) 319,\
    _SM_(wampIpv4Mode               ) 0,\
    _SM_(wampIpv6Mode               ) 1,\
    _SM_(ntpIpv4Mode                ) 0,\
    _SM_(ntpIpv6Mode                ) 1,\
    _SM_(ptpIpv4Mode                ) 0,\
    _SM_(ptpIpv6Mode                ) 1,\
    _SM_(tsTagParseEnable           ) GT_FALSE,\
    _SM_(hybridTsTagParseEnable     ) GT_FALSE,\
    _SM_(tsTagEtherType             ) 0,\
    _SM_(hybridTsTagEtherType       ) 0,\
    _SM_(refClockFrequency          ) 2,\
    _SM_(refClockSource             ) 0,\
    _SM_(ptpCpuCode                 ) 0

/* Macro for PP Phase 1 Params */
#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_128_PHY \
    _SM_(coreClock                  ) CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS,\
    _SM_(ppHAState                  ) CPSS_SYS_HA_MODE_ACTIVE_E,\
    _SM_(serdesRefClock             ) CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,\
    _SM_(isrAddrCompletionRegionsBmp) 0x0,\
    _SM_(appAddrCompletionRegionsBmp) 0x0,\
    _SM_(numOfPortGroups            ) 1,\
    _SM_(maxNumOfPhyPortsToUse      ) 128,\
    _SM_(tcamParityCalcEnable       ) 1,\
    _SM_(cpssWaList                 ) NULL,\
    _SM_(cpssWaNum                  ) 0

/* Macro for PP Logical Init Params */
#define CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS \
    /* Logical Init */\
    _SM_(routingMode                ) CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E, \
    _SM_(maxNumOfPbrEntries         ) 256,\
    _SM_(lpmMemoryMode              ) CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E,\
    _SM_(sharedTableMode            ) CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E,\
    _SM_(ingressDropEnable          ) GT_TRUE,\
    /* General Init */\
    _SM_(ctrlMacLearn               ) GT_TRUE,\
    _SM_(flowControlDisable         ) GT_TRUE,\
    _SM_(modeFcHol                  ) GT_FALSE,\
    _SM_(mirrorAnalyzerPortNum      ) 0,\
    /* Cascade Init */\
    _SM_(cascadePtr                 ) NULL

/** Macro for PP serdes polarity Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS \
    /*Serdes Params*/\
    _SM_(polarityPtr                ) harrier_DB_PolarityArray, \
    _SM_(polarityArrSize            ) sizeof(harrier_DB_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC), \
    _SM_(serdesMapPtr               ) NULL, \
    _SM_(serdesMapArrSize           ) 0

#ifdef MSA_INTCPU
/** Macro for Internal Cpu **/
#define CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS \
    /*Serdes Params*/\
    _SM_(internalCpu                ) GT_TRUE
#else
/** Macro for Internal Cpu **/
#define CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS \
    /*Serdes Params*/\
    _SM_(internalCpu                ) GT_FALSE
#endif

/** Macro for MPD init **/
#define CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS \
    /*Serdes Params*/\
    _SM_(initMpd                ) GT_FALSE

/** Macro for belly2belly handling enable **/
#define CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DEFAULTS \
    _SM_(belly2belly                ) GT_FALSE

#define CPSS_APP_PLATFORM_PP_PROFILE_EZB_XML_LOADED_DEFAULTS \
                _SM_(isEzbIsXmlLoaded                ) GT_FALSE


#define CPSS_APP_PLATFORM_PP_PROFILE_COS_DEFAULTS          \
    _SM_(tailDropWREDEnable                    ) GT_FALSE, \
    _SM_(tailDropDynamicEnable                 ) GT_FALSE, \
    _SM_(tailDropPoolEnable                    ) GT_FALSE, \
    _SM_(globalTailDropLimit                   ) 220000,   \
    _SM_(profileCount                          ) 0,        \
    _SM_(poolTailDropLimitCount                ) 0,        \
    _SM_(poolTailDropQueueCount                ) 0,        \
    _SM_(tailDropProfile                       ) NULL,     \
    _SM_(poolTailDropLimitList                 ) NULL,     \
    _SM_(poolTailDropQueueList                 ) NULL,     \
    _SM_(multicastTailDropLimit                ) 10000,   \
    _SM_(multicastTailDropLowGuaranteedLimit   ) 10000,   \
    _SM_(multicastTailDropHighGuaranteedLimit  ) 10000,    \
    _SM_(multicastTailDropLowDynamicFactor     ) 1,       \
    _SM_(multicastTailDropHighDynamicFactor    ) 1

#define CPSS_APP_PLATFORM_PP_PROFILE_PP_NAME_DEFAULTS \
        _SM_(ppName                ) "Harrier"

CPSS_APP_PLATFORM_PP_PROFILE_STC WM_HARRIER_PP_INFO = {
    CPSS_APP_PLATFORM_PP_PROFILE_PP_NAME_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_128_PHY,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_ENABLE,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_COS_DEFAULTS,
	CPSS_APP_PLATFORM_PP_PROFILE_EZB_XML_LOADED_DEFAULTS
};

