/* *****************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

#ifndef MPD_DEBUG_H_
#define MPD_DEBUG_H_
/**
 * @file mpdDebug.h
 *
 * @brief This file contains types and declarations
 *  required to debug MPD
 */

#include <mpdPrefix.h>
#include <mpdTypes.h>
#include <mpdApi.h>
#include <mpdPrv.h>

#define PRV_MPD_PHY_SKIP_PARAMS_CNS	  (0xFF)

#ifdef MTD_PHY_INCLUDE

#ifdef MTD_DEBUG

extern MTD_DBG_LEVEL mtd_debug_level;
#endif
#endif
/**
 * @enum    MPD_DEBUG_CONVERT_ENT
 *
 * @brief   Enumerator to convert different types to strings for debug.
 */
typedef enum {
    MPD_DEBUG_CONVERT_PHY_TYPE_E,
    MPD_DEBUG_CONVERT_OP_ID_E,
    MPD_DEBUG_CONVERT_MDI_MODE_E,
    MPD_DEBUG_CONVERT_MEDIA_TYPE_E,
    MPD_DEBUG_CONVERT_GREEN_SET_E,
    MPD_DEBUG_CONVERT_PORT_SPEED_E,
    MPD_DEBUG_CONVERT_GREEN_READINESS_E,
    MPD_DEBUG_CONVERT_PHY_KIND_E,
    MPD_DEBUG_CONVERT_PHY_SPEED_E,
    MPD_DEBUG_CONVERT_PHY_EEE_SPEED_E,
    MPD_DEBUG_CONVERT_PHY_EEE_CAPABILITY_E,
    /* must be last */
    MPD_DEBUG_CONVERT_LAST_E
} MPD_DEBUG_CONVERT_ENT;

/**
 * @enum    MPD_DEBUG_REGISTERS_ENT
 *
 * @brief   Enumerator to decide what set of registers to dump
 */
typedef enum {
    MPD_DEBUG_REGISTERS_FULL_DUMP_E,
    MPD_DEBUG_REGISTERS_STRAP_DUMP_E,
    MPD_DEBUG_REGISTERS_LINK_FLAP_DEBUG_DUMP_E,
    /* must be last */
    MPD_DEBUG_REGISTERS_LAST_E
} MPD_DEBUG_REGISTERS_ENT;

typedef enum {
	MPD_DEBUG_TXEQ_EM_POST_CTRL_E ,
	MPD_DEBUG_TXEQ_EM_PEAK_CTRL_E ,
	MPD_DEBUG_TXEQ_EM_PRE_CTRL_E  ,
	MPD_DEBUG_TXEQ_MARGIN_E     ,
	/* must be last */
	MPD_DEBUG_TXEQ_LAST_E
} MPD_DEBUG_TXEQ_PARAM_E;


typedef struct {
    UINT_8  pulseInCounter;
    UINT_32 clockInCounter;
} PRV_MPD_PTP_COUNTERS_STC;

typedef struct {
    UINT_32       num_of_entries;
    const char       ** text_ARR;
} PRV_MPD_DEBUG_CONVERT_STC;

extern const char * prvMpdDebugConvert(
    /*!     INPUTS:             */
    MPD_DEBUG_CONVERT_ENT   conv_type,
    UINT_32                 value
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugPerformPhyOperation(
    /*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    MPD_OP_CODE_ENT                 op,
    MPD_OPERATIONS_PARAMS_UNT     * params_PTR,
    BOOLEAN                         before
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);


extern MPD_RESULT_ENT prvMpdMtdDebugRegDump(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex,
    MPD_DEBUG_REGISTERS_ENT dumpType
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMtdDebugGetEye(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMtdPktGenTHUnit(
	/*     INPUTS:             */
	UINT_32 rel_ifIndex,
	BOOLEAN sendOnTUnit, /* true - send to T unit, false - send to MAC */
	UINT_16	numPktsToSend /* 0x1 - 0xFFFE is fixed number of packets, 0xFFFF to send continuously, 0 - to stop packet generator */
);

extern MPD_RESULT_ENT prvMpdMtdPktGenOnXHUnit(
	/*     INPUTS:             */
	UINT_32 rel_ifIndex,
	BOOLEAN sendOnXUnit, /* true - send to X unit, false - send to H unit */
	UINT_16	numPktsToSend /* 0x1 - 0xFFFE is fixed number of packets, 0xFFFF to send continuously, 0 - to stop packet generator */
);

extern MPD_RESULT_ENT prvMpdMtdDebugGetInterruptsStatus(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMtdDebugRunEchoTest(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugPrintPortDb(
    /*!     INPUTS:             */
    UINT_32    rel_ifIndex,
    BOOLEAN    fullDb
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);


extern MPD_RESULT_ENT prvMpdMdioDebugCl22WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMdioDebugCl22ReadRegister(
    /*     INPUTS:             */
    BOOLEAN readAll,
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMdioDebugCl45WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMdioDebugCl45ReadRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugConvertSpeedToString(
    /*!     INPUTS:             */
    MPD_SPEED_ENT speed,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    char *speedString_PTR
);

extern MPD_RESULT_ENT prvMpdDebugConvertMtdCapabilitiesToString(
    /*!     INPUTS:             */
    UINT_16 capabilities,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    char *capabilitiesString_PTR
);

extern MPD_RESULT_ENT prvMpdDebugPrintPortOperState(
    /*!     INPUTS:             */
    UINT_32    rel_ifIndex
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMtdPktCntChk
(
	/*     INPUTS:             */
	UINT_32 rel_ifIndex
);

extern MPD_RESULT_ENT prvMpdDebugGetFwDetailedBuildInfo (
	/*     INPUTS:             */
	UINT_32    rel_ifIndex
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugMtdSerdesSample28X2PRBS (
	/*     INPUTS:             */
	UINT_32    rel_ifIndex0,
	UINT_32    rel_ifIndex1
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugAPIC28GP4X2GetTxEqParam (
	/*     INPUTS:             */
	UINT_32    rel_ifIndex,
	UINT_32    lane
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugUpdatePortDb(

	/*     INPUTS:             */
	UINT_32         rel_ifIndex,
	MPD_TYPE_ENT	phyType,
	UINT_8			mdioBus,
	UINT_8		    mdioDev,
	UINT_8		    mdioAddress,
	BOOLEAN			doInitSequence
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugAPIC28GP4X2SetTxEqParam (
	/*     INPUTS:             */
	UINT_32					rel_ifIndex,
	MPD_DEBUG_TXEQ_PARAM_E	txeq,
	UINT_32					lane,
	UINT_32					paramValue
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdPtpGetDebugCounters(
    IN UINT_32 phyNumber,
    OUT  PRV_MPD_PTP_COUNTERS_STC *counters_PTR
);

extern MPD_RESULT_ENT mpdDebugPtpTaiRegisterReadAll(
    IN UINT_32 phyNumber
);

extern MPD_RESULT_ENT mpdDebugPtpTaiRegisterRead(
    IN UINT_32 phyNumber,
    IN UINT_8  reg
);

extern MPD_RESULT_ENT mpdDebugPtpTaiRegisterWrite(
    IN UINT_32  phyNumber,
    IN UINT_8   reg,
    IN UINT_32  mask,
    IN UINT_32  data
);

extern MPD_RESULT_ENT prvMpdDebugInit(
    void
);

#endif /* MPD_DEBUG_H_ */



