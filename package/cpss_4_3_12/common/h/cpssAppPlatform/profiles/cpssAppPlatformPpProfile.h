/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cpssAppPlatformPpProfile.h
*
* @brief This file contains Packet processor profile definitions.
*
* @version   1
********************************************************************************
*/
#ifndef __CPSS_APP_PLATFORM_PP_PROFILE_H
#define __CPSS_APP_PLATFORM_PP_PROFILE_H

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManager.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#endif

#define CAP_BAD_VALUE       (GT_U32)(~0)

#define CPSS_APP_PLATFORM_MAX_TRUNKS_CNS      3
#define CPSS_APP_PLATFORM_MAX_TRUNK_PORTS_CNS 128
/* Define the maximum number of RX queues */
#define CPSS_APP_PLATFORM_MAX_RX_QUEUE_CNS    128

typedef struct _CPSS_APP_PLATFORM_SDMA_QUEUE_STC
{
    GT_U32 sdmaPortNum;
    GT_U8  queueNum;
    /* Number of Tx buffers for a tx queue */
    /* Note: num of Tx Buffers == num of Tx descriptors */
    GT_U32 numOfTxBuff;
    GT_U32 txBuffSize;
}CPSS_APP_PLATFORM_SDMA_QUEUE_STC;

typedef struct
{
    GT_U32  portMac;
    GT_U32  ledIf;
    GT_U32  ledPosition;
}CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC;

typedef struct
{
    GT_U32      trunkId;
    GT_U32      numCscdPorts;
#ifdef CHX_FAMILY
    CPSS_CSCD_PORT_TYPE_ENT     cscdPortType;                    /* Enum of cascade port or network port */
#endif
    GT_U32      portList[CPSS_APP_PLATFORM_MAX_TRUNK_PORTS_CNS];
} CPSS_APP_PLATFORM_CSCD_TRUNK_INFO_STC;

/*
 * @struct _CPSS_APP_PLATFORM_PP_PARAM_CASCADE_STC.
 * @brief: Casacade Port info.
 */
typedef struct _CPSS_APP_PLATFORM_PP_PARAM_CASCADE_STC {

    GT_U32  numCscdTrunks;
    GT_U32  srcDevNum;
    GT_U32  trgtDevNum;
    CPSS_APP_PLATFORM_CSCD_TRUNK_INFO_STC   trunkInfo[CPSS_APP_PLATFORM_MAX_TRUNKS_CNS];
} CPSS_APP_PLATFORM_PP_PARAM_CASCADE_STC;

/*
 * @struct: CPSS_APP_PLATFORM_PP_PROFILE_STC
 * @brief: This hold the Phase-1, Phase-2 and post Phase2 related information which
 *         is used to initialize packet processor in a board.
 */

/**
* @struct CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC
 *
 * @brief Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*/
typedef struct{

    /** number of SERDES lane */
    GT_U32 laneNum;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertTx;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertRx;

} CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC;

/**
* @struct CPSS_APP_PLATFORM_PP_PROFILE_TAILDROP_LIMIT_LIST
 *
 * @brief Pool tail drop limit – part of PP profile
*/
typedef struct{
    GT_U8       poolTailDropLimitId;
    GT_U32      poolTailDropLimit;
}CPSS_APP_PLATFORM_PP_PROFILE_TAILDROP_LIMIT_LIST;

/**
* @struct CPSS_APP_PLATFORM_PP_PROFILE_TAILDROP_QUEUE_LIST
 *
 * @brief Pool tail drop queue limit – part of PP profile
*/
typedef struct{
    GT_U8       portTailDropQueue;
    GT_U8       poolTailDropQueueLimitId;
}CPSS_APP_PLATFORM_PP_PROFILE_TAILDROP_QUEUE_LIST;

/**
* @struct CPSS_APP_PLATFORM_TAILDROP_QUEUE_DP_LIST
 *
 * @brief Port tail drop queue dp properties
*/
typedef struct{
    GT_U32      portTailDropQueueDp;
    GT_U32      portTailDropQueueDpGuaranteedLimit;
    GT_U32      portTailDropQueueDpDynamicFactor;
    GT_U32      portTailDropQueueDpProbability;
    GT_U32      portTailDropQueueDpWREDSize;
}CPSS_APP_PLATFORM_TAILDROP_QUEUE_DP_LIST;

/**
* @struct CPSS_APP_PLATFORM_TAILDROP_QUEUE_PROFILE_LIST
 *
 * @brief Tail drop queue profile
*/
typedef struct{
    GT_U32      portTailDropQueue;
    GT_U32      portTailDropQueueMaxLimit;
    GT_U32      dpCount;
    CPSS_APP_PLATFORM_TAILDROP_QUEUE_DP_LIST **portTailDropQueueDp;
}CPSS_APP_PLATFORM_TAILDROP_QUEUE_PROFILE_LIST;

/**
* @struct CPSS_APP_PLATFORM_PP_PROFILE_TAILDROP_PROFILE_LIST
 *
 * @brief Tail drop profile – part of PP profile
*/
typedef struct{
    GT_U8       portTailDropProfileId;
    CPSS_PORTS_BMP_STC      portTailDropPorts;
    GT_U32      portTailDropGuaranteedLimit;
    GT_U32      portTailDropDynamicFactor;
    GT_U32      portTailDropProbability;
    GT_U32      portTailDropWREDSize;
    GT_U32      queueCount;
    CPSS_APP_PLATFORM_TAILDROP_QUEUE_PROFILE_LIST **portTailDropQueue;
}CPSS_APP_PLATFORM_PP_PROFILE_TAILDROP_PROFILE_LIST;

/**
* @struct CPSS_APP_PLATFORM_PHA_THREAD_LIST
 *
 * @brief PHA thread properties
*/
typedef struct{
    GT_U32                             phaThreadId;
    GT_U32                             phaThreadType;
}CPSS_APP_PLATFORM_PHA_THREAD_LIST;

typedef struct _CPSS_APP_PLATFORM_PP_PROFILE_STC
{
    GT_CHAR                                       *ppName;
    /*** PP Phase-1 Parameters ***/
    GT_U32                                         coreClock;
    CPSS_SYS_HA_MODE_ENT                           ppHAState;
#ifdef CHX_FAMILY
    CPSS_DXCH_PP_SERDES_REF_CLOCK_ENT              serdesRefClock;
#endif
    GT_U32                                         isrAddrCompletionRegionsBmp;
    GT_U32                                         appAddrCompletionRegionsBmp;
    GT_U32                                         numOfPortGroups;
    GT_U32                                         maxNumOfPhyPortsToUse;
    GT_U32                                         tcamParityCalcEnable;
#ifdef CHX_FAMILY
    CPSS_DXCH_IMPLEMENT_WA_ENT                     *cpssWaList;
#endif
    GT_U32                                         cpssWaNum;
    GT_BOOL                                        apEnable;
    GT_U32                                         numOfDataIntegrityElements;

    /*** PP Phase-2 Parameters ***/
    GT_U32                                         newDevNum;
    /*** AU queue params ***/
    GT_U32                                         auDescNum;
#ifdef CHX_FAMILY
    CPSS_AU_MESSAGE_LENGTH_ENT                     auMessageLength;
#endif
    GT_BOOL                                        useDoubleAuq;
    GT_BOOL                                        useSecondaryAuq;
    GT_U32                                         fuDescNum;
    CPSS_PP_PHASE2_UPLOAD_QUEUE_TYPE_ENT           fuqUseSeparate;

    GT_BOOL                                        noTraffic2CPU;
    CPSS_NET_CPU_PORT_MODE_ENT                     cpuPortMode;

    /*** SDMA params ***/
    GT_BOOL                                        useMultiNetIfSdma;
    GT_U32                                         netifSdmaPortGroupId;
    GT_U32                                         txDescNum;
    GT_U32                                         rxDescNum;
    CPSS_RX_BUFF_ALLOC_METHOD_ENT                  rxAllocMethod;
    GT_BOOL                                        rxBuffersInCachedMem;
    GT_U32                                         rxHeaderOffset;
    GT_U32                                         rxBufferPercentage[CPSS_APP_PLATFORM_MAX_RX_QUEUE_CNS]; /* Applicable only if non multiNetIfSdma mode */
    GT_U32                                         maxQueueNum;
    CPSS_APP_PLATFORM_SDMA_QUEUE_STC               *txGenQueueList; /* List of Tx queues to work in generator mode */
    GT_U32                                         txGenQueueNum;
    GT_U32                                         rxPackets;
    GT_U32                                         txPackets;
    GT_U32                                         packetSize;

    /*** MII Params ***/
    GT_U32                                         miiTxDescNum;
    GT_U32                                         miiTxBufBlockSize;
    GT_U32                                         miiRxBufSize;
    GT_U32                                         miiRxBufBlockSize;
    GT_U32                                         miiRxHeaderOffset;
    GT_U32                                         miiRxBufferPercentage[CPSS_APP_PLATFORM_MAX_RX_QUEUE_CNS];

    /*** Post Phase-2 Parameters ***/
    /* Led Init*/
    CPSS_LED_ORDER_MODE_ENT                        ledOrganize;
    CPSS_LED_CLOCK_OUT_FREQUENCY_ENT               ledClockFrequency;
    GT_BOOL                                        invertEnable;
    CPSS_LED_PULSE_STRETCH_ENT                     pulseStretch;
    CPSS_LED_BLINK_DURATION_ENT                    blink0Duration;
    CPSS_LED_BLINK_DUTY_CYCLE_ENT                  blink0DutyCycle;
    CPSS_LED_BLINK_DURATION_ENT                    blink1Duration;
    CPSS_LED_BLINK_DUTY_CYCLE_ENT                  blink1DutyCycle;
    GT_BOOL                                        disableOnLinkDown;
    GT_BOOL                                        clkInvert;
#ifdef CHX_FAMILY
    CPSS_LED_CLASS_5_SELECT_ENT                    class5select;
    CPSS_LED_CLASS_13_SELECT_ENT                   class13select;
#endif
    GT_U32                                         ledStart;
    GT_U32                                         ledEnd;
    GT_U32                                         sip6LedClockFrequency;
    GT_BOOL                                        ledClockPosEdgeModeEnable;
    CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC          *ledPositionTable;

    /*Led Class manipulation config*/
    GT_BOOL                                        invertionEnable;
    CPSS_LED_BLINK_SELECT_ENT                      blinkSelect;
    GT_BOOL                                        forceEnable;
    GT_U32                                         forceData;
    GT_BOOL                                        pulseStretchEnable;

    /*Logical Init*/
#ifdef CHX_FAMILY
    CPSS_DXCH_CFG_ROUTING_MODE_ENT                 routingMode;
#endif
    GT_U32                                         maxNumOfPbrEntries;
#ifdef CHX_FAMILY
    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT                 lpmMemoryMode;
    CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT            sharedTableMode;
#endif
    GT_BOOL                                        ingressDropEnable;

    /*General Init*/
    GT_BOOL                                        ctrlMacLearn;
    GT_BOOL                                        flowControlDisable;
    GT_BOOL                                        modeFcHol;
    GT_U32                                         mirrorAnalyzerPortNum;

    /*Cascade Init*/
    CPSS_APP_PLATFORM_PP_PARAM_CASCADE_STC         *cascadePtr;

    /*Lib Init*/
    GT_BOOL                                        initBridge;
    GT_BOOL                                        initIp;
    GT_BOOL                                        initMirror;
    GT_BOOL                                        initNetIf;
    GT_BOOL                                        initPhy;
    GT_BOOL                                        initPort;
    GT_BOOL                                        initTrunk;
    GT_BOOL                                        initPcl;
    GT_BOOL                                        initTcam;
    GT_BOOL                                        initPolicer;
    GT_BOOL                                        initPha;
    GT_BOOL                                        initPtp;

    /** Lib Init Parameters **/
    /* IP Lib Params **/
#ifdef CHX_FAMILY
    CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT blocksAllocationMethod;
#endif
    GT_BOOL                                        usePolicyBasedRouting;
    GT_U32                                         maxNumOfIpv4Prefixes;
    GT_U32                                         maxNumOfIpv4McEntries;
    GT_U32                                         maxNumOfIpv6Prefixes;
    GT_U32                                         lpmDbFirstTcamLine;
    GT_U32                                         lpmDbLastTcamLine;
    GT_BOOL                                        lpmDbSupportIpv4;
    GT_BOOL                                        lpmDbSupportIpv6;
    GT_BOOL                                        lpmDbPartitionEnable;
    /* Trunk Lib Params **/
    GT_U32                                         numOfTrunks;
#ifdef CHX_FAMILY
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT               trunkMembersMode;
#endif
    /* Pha Lib Params **/
    GT_BOOL                                        pha_packetOrderChangeEnable;
    GT_U32                                         phaFwImageId;
    GT_U32                                         phaFwImageIdVersion;
    CPSS_APP_PLATFORM_PHA_THREAD_LIST              **phaThreadList;

    /* Bridge Lib Params **/
    GT_BOOL                                        policerMruSupported;

#ifdef CHX_FAMILY
    /* Ptp Lib Params */
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                   taiNumber;
    CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT    outputInterfaceMode;
    GT_U32                                         nanoSeconds;
    GT_BOOL                                        ptpOverEthernetEnable;
    GT_U32                                         etherType0value;
    GT_U32                                         etherType1value;
    GT_BOOL                                        ptpOverIpUdpEnable;
    GT_U32                                         udpPort0value;
    GT_U32                                         udpPort1value;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT  wampIpv4Mode;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT  wampIpv6Mode;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT  ntpIpv4Mode;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT  ntpIpv6Mode;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT  ptpIpv4Mode;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT  ptpIpv6Mode;
    GT_BOOL                                        tsTagParseEnable;
    GT_BOOL                                        hybridTsTagParseEnable;
    GT_U32                                         tsTagEtherType;
    GT_U32                                         hybridTsTagEtherType;
    CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT               refClockFrequency;
    CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT             refClockSource;
    CPSS_NET_RX_CPU_CODE_ENT                       ptpCpuCode;
#endif

    /* Serdes Polarity Params **/
    CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC     *polarityPtr;
    GT_U32                                         polarityArrSize;

    /* Serdes Mux Params **/
    CPSS_PORT_MAC_TO_SERDES_STC                    *serdesMapPtr;
    GT_U32                                         serdesMapArrSize;

    /*Internal Cpu*/
    GT_BOOL                                        internalCpu;
    /*MPD Init*/
    GT_BOOL                                        initMpd;

    /*belly2belly enable*/
    GT_BOOL                                         belly2belly;

    /* Cos Parameters*/
    GT_BOOL                                         tailDropWREDEnable;
    GT_BOOL                                         tailDropDynamicEnable;
    GT_BOOL                                         tailDropPoolEnable;
    GT_U32                                          globalTailDropLimit;
    GT_U32                                          profileCount;
    GT_U32                                          poolTailDropLimitCount;
    GT_U32                                          poolTailDropQueueCount;
    CPSS_APP_PLATFORM_PP_PROFILE_TAILDROP_PROFILE_LIST  **tailDropProfile;
    CPSS_APP_PLATFORM_PP_PROFILE_TAILDROP_LIMIT_LIST    **poolTailDropLimitList;
    CPSS_APP_PLATFORM_PP_PROFILE_TAILDROP_QUEUE_LIST    **poolTailDropQueueList;
    GT_U32                                          multicastTailDropLimit;
    GT_U32                                          multicastTailDropLowGuaranteedLimit;
    GT_U32                                          multicastTailDropHighGuaranteedLimit;
    GT_U32                                          multicastTailDropLowDynamicFactor;
    GT_U32                                          multicastTailDropHighDynamicFactor;
    GT_BOOL                                         isEzbIsXmlLoaded;
} CPSS_APP_PLATFORM_PP_PROFILE_STC;

#endif /* __CPSS_APP_PLATFORM_PP_PROFILE_H */
