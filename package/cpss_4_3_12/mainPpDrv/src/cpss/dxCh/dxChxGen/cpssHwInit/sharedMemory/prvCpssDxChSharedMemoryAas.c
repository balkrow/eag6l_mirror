/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvCpssDxChSharedMemoryAas.c
*
* @brief This file implement SHM unit (shared-memory) of the AAS.
*       Shared tables target is to enable increase of one engine tables size on
*       the expense of other engine without increase of total area.
*       1.SHM uses two types of memories:
*           a. Type 1: LPM dedicate memory (for smallest LPM bank option)
*           b. Type 2: Shared SBM - for LPM, FDB, ARP, TS, EM, CNC, PLRs, HF
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*Those values 1..19 means the same in all SBMs*/
/*1 == em0/4 Low (SBM 0..3 - EM0 , SBM 4..7 - EM4) */
/*2 == em0/4, High (high means the value >= 8) */
/*3 == em1/5, Low                              */
/*4 == em1/5, High                             */
/*5 == em2/6, Low                              */
/*6 == em2/6, High                             */
/*7 == em3/7, Low                              */
/*8 == em3/7, High                             */
/*9  NA                                        */
/*10 == CNC 2-3                                */
/*11 == HF0                                    */
/*12 == HF1                                    */
/*13 == LPM                                    */
/*14 == ARP                                    */
/*15 == TS                                     */
/*16 == PLR CNT                                */
/*17 == PLR MET                                */
/*18 == CNC 0-1                                */
/*19 == FDB                                    */

/* AAS : enum for the clients that may occupy the SBMs */
typedef enum
{
    /* value 0 is not valid in the regValue_0,regValue_1 */
    AAS_UNUSED_HW_VALUE                     = 0,
    AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E    = 1,
    AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E   = 2,
    AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E    = 3,
    AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E   = 4,
    AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E    = 5,
    AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E   = 6,
    AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E    = 7,
    AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E   = 8,
    AAS_SBM_CLIENT_TYPE_NOT_USED_E          = 9,/* NOT USED */
    AAS_SBM_CLIENT_TYPE_CNC_2_3_E           = 10,
    AAS_SBM_CLIENT_TYPE_HF0_E               = 11,
    AAS_SBM_CLIENT_TYPE_HF1_E               = 12,
    AAS_SBM_CLIENT_TYPE_LPM_E               = 13,
    AAS_SBM_CLIENT_TYPE_ARP_E               = 14,
    AAS_SBM_CLIENT_TYPE_TS_E                = 15,
    AAS_SBM_CLIENT_TYPE_PLR_CNT_E /*PLR0*/  = 16,
    AAS_SBM_CLIENT_TYPE_PLR_MET_E /*PLR1*/  = 17,
    AAS_SBM_CLIENT_TYPE_CNC_0_1_E           = 18,
    AAS_SBM_CLIENT_TYPE_FDB_E               = 19,

    /* next not relate to HW , but needed for WM only */
    WM__SPECIAL_MESSAGE_TYPE__FIRST__E   = 100,

    WM__SPECIAL_MESSAGE_TYPE_CNC_0_1_BLOCKS_BMP_E   ,
    WM__SPECIAL_MESSAGE_TYPE_CNC_2_3_BLOCKS_BMP_E   ,
    WM__SPECIAL_MESSAGE_TYPE_SHARING_PAIRS_INFO_E   ,
    WM__SPECIAL_MESSAGE_TYPE_LPM_BLOCKS_BMP_E  ,

    WM__SPECIAL_MESSAGE_TYPE__LAST__E,

    AAS_SBM_CLIENT_TYPE__LAST__E

}AAS_SBM_CLIENT_TYPE_ENT;


/**
 * @struct AAS_CLIENT_INFO_STC
 *
 * @brief struct to hold info about the client that occupy the UBM
 *  (APPLICABLE DEVICES: AAS.)
 *
 */
typedef struct
{
    /**@brief the name (for debug prints)
     */
    GT_CHAR             *clientName;
    /**@brief the HW value into port0 or port 1
     */
    GT_U32                hwValue;
    /**@brief when GT_TRUE port1 connecting to the other tile (port 0 is 'local')
     */
    GT_BOOL               supportShareWithOtherTile;
    /**@brief FDB use it for SA and DA that use port 0,1 also CNC need it
     */
    GT_BOOL               use2PortsInSingleTile;
    /**@brief the ARP and TS and HF need 3 consecutive SBMs
     *        the PLR-cnt and PLR-meter need 2 consecutive SBMs
     */
    GT_U32                numOfConsecutiveSbmsNeeded;
    /**@brief the ARP and TS and HF need 3 consecutive SBMs    : so the value is per 3 SBMs
     *        the PLR-cnt and PLR-meter need 2 consecutive SBMs : so the value is per 2 SBMs
     *        for FDB for example : 32K per SBM
     */
    GT_U32                numEntriesPerSbm;
    /**@brief per CP , (min) number of SBMs that the client occupy (connected to) even if the memory not needed
     */
    GT_U32                minNumSbms;
    /**@brief per CP , max number of SBMs that the client can occupy
     */
    GT_U32                maxNumSbms;

}AAS_CLIENT_INFO_STC;

/* for most clients the 32K lines (133bits width) are used for 32K entries */
/* for CNC , there are design needs to use only 1/2 the SBM for the client */
#define SBM_NUM_LINES   _32K

#define DEFAULT_NUM_LINES SBM_NUM_LINES
#define CNC_NUM_LINES     (SBM_NUM_LINES / 2)
/* the HF need 6 SBMs , for 'full line' of 784 bits */
/* but it supports the ability to use only 3 SBMs instead of 6 , on the last 3 (instead of 6) SBMs */
#define HF_NUM_LINES      DEFAULT_NUM_LINES

static const AAS_CLIENT_INFO_STC aasShmClientsInfoArr[AAS_SBM_CLIENT_TYPE__LAST__E+1] =
{                                            /*supportShareWithOtherTile*/    /*numOfConsecutiveSbmsNeeded*/
                                            /*hwValue*/         /*use2PortsInSingleTile*/ /*numEntriesPerSbm*/
                                                                                                              /* min , max ,connectingOrder */
     {STR(AAS_UNUSED_HW_VALUE                   )  , 0,      GT_FALSE,  GT_FALSE,     0     , 0                 , 0,  0}
    ,{STR(AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E  )  , 1,      GT_TRUE ,  GT_FALSE,     0     , DEFAULT_NUM_LINES , 0, 64}
    ,{STR(AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E )  , 2,      GT_TRUE ,  GT_FALSE,     0     , DEFAULT_NUM_LINES , 0, 64}
    ,{STR(AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E  )  , 3,      GT_TRUE ,  GT_FALSE,     0     , DEFAULT_NUM_LINES , 0, 64}
    ,{STR(AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E )  , 4,      GT_TRUE ,  GT_FALSE,     0     , DEFAULT_NUM_LINES , 0, 64}
    ,{STR(AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E  )  , 5,      GT_TRUE ,  GT_FALSE,     0     , DEFAULT_NUM_LINES , 0, 64}
    ,{STR(AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E )  , 6,      GT_TRUE ,  GT_FALSE,     0     , DEFAULT_NUM_LINES , 0, 64}
    ,{STR(AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E  )  , 7,      GT_TRUE ,  GT_FALSE,     0     , DEFAULT_NUM_LINES , 0, 64}
    ,{STR(AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E )  , 8,      GT_TRUE ,  GT_FALSE,     0     , DEFAULT_NUM_LINES , 0, 64}
    ,{STR(AAS_SBM_CLIENT_TYPE_NOT_USED_E        )  , 9,      GT_FALSE,  GT_FALSE,     0     , 0                 , 0,  0}
    ,{STR(AAS_SBM_CLIENT_TYPE_CNC_2_3_E         )  , 10,     GT_FALSE,  GT_TRUE ,     0     , CNC_NUM_LINES     , 0, 16}/*min is 0 unlike CNC0_1 with 6*/
    ,{STR(AAS_SBM_CLIENT_TYPE_HF0_E             )  , 11,     GT_TRUE ,  GT_FALSE,  3/*not6*/, HF_NUM_LINES/*per 6 SBMs*/, 3, 24}
    ,{STR(AAS_SBM_CLIENT_TYPE_HF1_E             )  , 12,     GT_TRUE ,  GT_FALSE,  3/*not6*/, HF_NUM_LINES/*per 6 SBMs*/, 3, 24}
    ,{STR(AAS_SBM_CLIENT_TYPE_LPM_E             )  , 13,     GT_TRUE ,  GT_FALSE,     0     , DEFAULT_NUM_LINES ,32/2,154/2}
    ,{STR(AAS_SBM_CLIENT_TYPE_ARP_E             )  , 14,     GT_FALSE,  GT_FALSE,     3     , DEFAULT_NUM_LINES/*lines*/*8/*ARPS in line*//*per 3 SBMs*/, 0, 12}
    ,{STR(AAS_SBM_CLIENT_TYPE_TS_E              )  , 15,     GT_TRUE ,  GT_FALSE,     3     , DEFAULT_NUM_LINES/*per 3 SBMs*/, 3, 24}
    ,{STR(AAS_SBM_CLIENT_TYPE_PLR_CNT_E         )  , 16,     GT_FALSE,  GT_FALSE,     2     , DEFAULT_NUM_LINES/*per 2 SBMs*/, 2, 32}
    ,{STR(AAS_SBM_CLIENT_TYPE_PLR_MET_E         )  , 17,     GT_FALSE,  GT_FALSE,     2     , DEFAULT_NUM_LINES/*per 2 SBMs*/, 2, 32}
    ,{STR(AAS_SBM_CLIENT_TYPE_CNC_0_1_E         )  , 18,     GT_FALSE,  GT_TRUE ,     0     , CNC_NUM_LINES     , 6, 16}
    ,{STR(AAS_SBM_CLIENT_TYPE_FDB_E             )  , 19,     GT_FALSE,  GT_TRUE ,     0     , DEFAULT_NUM_LINES , 4, 32}

    ,{STR(AAS_SBM_CLIENT_TYPE__LAST__E          )  , GT_NA,  GT_FALSE,  GT_FALSE,     0     , 0                 , 0, 0 }
};

/**
 * @struct AAS_SBM_CLIENT_INFO_STC
 *
 * @brief struct to hold info about the client that occupy the SBM (single line in UBM)
 *  (APPLICABLE DEVICES: AAS.)
 *
 */
typedef struct
{
    /**@brief the sbm Id (0..7)
     */
    GT_U32              sbmId;
    /**@brief the id of the client (FDB/EM/LPM/...)
     */
    AAS_SBM_CLIENT_TYPE_ENT client;
    /**@brief the id with in the client for 'port 0' connection (like : 5 for LPM client is 'LPM bank 5')
     */
    GT_U32              clientSubId_port0;
    /**@brief the id with in the client for 'port 1' connection (like : 4 for FDB client is 'FDB bank 4')
     */
    GT_U32              clientSubId_port1;
}AAS_SBM_CLIENT_INFO_STC;

#define CLIENT_TYPE_AND_NAME_MAC(_client)   _client,#_client
/* the SBM is connected to 2 'ports' (port 0 , port 1) */
#define SHM_PORTS_NUM     2

/* number of UBMs : 16 full (*8SBMs) + 1 small(*2SBMs)*/
#define NUM_OF_UBMS 17

/* AAS hold in each UBMs : 8 SBMs */
#define AAS_NUM_SBM_IN_UBM     8

/* no extra info for the SUB ID */
#define NO_INFO    0

#define SBM_ID(_id) (_id)

/* AAS UBM 0 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm0[] =
{
    /* UBM0-SBM 0 */
    /*   clienInfo    */                                     /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    0/*LPM0*/         , 1/*LPM1*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }

    /* UBM0-SBM 1 */
   /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    2/*LPM2*/         , 3/*LPM3*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }

    /* UBM0-SBM 2 */
   /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    4/*LPM4*/         , 5/*LPM5*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }

    /* UBM0-SBM 3 */
   /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    6/*LPM6*/         , 7/*LPM7*/}
    /* UBM0-SBM 4 */
   /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    24/*LPM24*/         , 25/*LPM25*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }

    /* UBM0-SBM 5 */
   /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    26/*LPM26*/       ,27/*LPM27*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }

    /* UBM0-SBM 6 */
       /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    28/*LPM28*/       ,29/*LPM29*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }

    /* UBM0-SBM 7 */
   /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    30/*LPM30*/       ,31/*LPM31*/}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 1 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm1[] =
{
    /* UBM1-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    8/*LPM8*/         ,9/*LPM9*/ }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    /* UBM1-SBM 1 */
        /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    10/*LPM10*/       ,11/*LPM11*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }

    /* UBM1-SBM 2 */
        /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    12/*LPM12*/       ,13/*LPM13*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }

    /* UBM1-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    14/*LPM14*/       ,15/*LPM15*/}

    /* UBM1-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    32/*LPM32*/       ,33/*LPM33*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }

    /* UBM1-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    34/*LPM34*/       ,35/*LPM35*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }

    /* UBM1-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    36/*LPM36*/       ,37/*LPM37*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }

    /* UBM1-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    38/*LPM38*/       ,39/*LPM39*/}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* use same value for port 1 as for port 0 */
#define GT_SAME_AS_0    (GT_NA-1)

/* build clientSubId_port0 or clientSubId_port1 for CNC blocks */
/* the result it is 'global' to block Id */
#define CNC_MAC(_unit/*0..3*/,_block/*0..15*/)  ((_unit)*16 + (_block))
/* build clientSubId_port0 or clientSubId_port1 for EMs MHTs */
/* the result it is 'global' to MHT index */
#define EM_MAC(_id/*0..7*/,_mht/*0..15*/)  ((_id)*16 + (_mht))
/* build clientSubId_port0 or clientSubId_port1 for FDBs MHTs */
#define FDB_MAC(_mht/*0..15*/)  (_mht)

/* AAS UBM 2 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm2[] =
{
    /* UBM2-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    16/*LPM16*/       , 17/*LPM17*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,4)      , CNC_MAC(0,5)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,0)      , CNC_MAC(2,1)}

    /* UBM2-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    18/*LPM18*/       , 19/*LPM19*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,6)      , CNC_MAC(0,7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,2)      , CNC_MAC(2,3)}

    /* UBM2-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    20/*LPM20*/       , 21/*LPM21*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,8)      , CNC_MAC(0,9)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,4)      , CNC_MAC(2,5)}

    /* UBM2-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    22/*LPM22*/       , 23/*LPM23*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,10)     , CNC_MAC(0,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,6)      , CNC_MAC(2,7)}

    /* UBM2-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    40/*LPM40*/       , 41/*LPM41*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,0)      , CNC_MAC(3,1)}

    /* UBM2-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    42/*LPM42*/       ,43/*LPM43*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,2)      , CNC_MAC(3,3)}

    /* UBM2-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    44/*LPM44*/       ,45/*LPM45*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,4)      , CNC_MAC(3,5)}

    /* UBM2-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    46/*LPM46*/       ,47/*LPM47*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,6)      , CNC_MAC(3,7)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 3 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm3[] =
{
    /* UBM3-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    6/*LPM6*/         , 7/*LPM7*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,12)     , CNC_MAC(0,13)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,8)      , CNC_MAC(2,9)}

    /* UBM3-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    2/*LPM2*/         , 3/*LPM3*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,14)     , CNC_MAC(0,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,10)     , CNC_MAC(2,11)}

    /* UBM3-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    4/*LPM4*/         , 5/*LPM5*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,4)      , CNC_MAC(0,5)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,12)     , CNC_MAC(2,13)}

    /* UBM3-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    0/*LPM0*/         , 1/*LPM1*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,6)      , CNC_MAC(0,7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,14)     , CNC_MAC(2,15)}

    /* UBM3-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    30/*LPM30*/       , 31/*LPM31*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,8)      , CNC_MAC(3,9)}

    /* UBM3-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    26/*LPM26*/       ,27/*LPM27*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,10)     , CNC_MAC(3,11)}

    /* UBM3-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    28/*LPM28*/       , 29/*LPM29*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,12)     , CNC_MAC(3,13)}

    /* UBM3-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    24/*LPM24*/       , 25/*LPM25*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,14)     , CNC_MAC(3,15)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 4 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm4[] =
{
    /* UBM4-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    14/*LPM14*/       , 15/*LPM15*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,8)      , CNC_MAC(0,9)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,0)      , CNC_MAC(2,1)}

    /* UBM4-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    10/*LPM10*/       , 11/*LPM11*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,10)     , CNC_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,2)      , CNC_MAC(2,3)}

    /* UBM4-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    12/*LPM12*/       , 13/*LPM13*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,12)     , CNC_MAC(0,13)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,6)      , CNC_MAC(2,7)}

    /* UBM4-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    8/*LPM8*/         , 9/*LPM9*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,14)     , CNC_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,4)      , CNC_MAC(2,5)}

    /* UBM4-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    38/*LPM38*/       ,39/*LPM39*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,0)      , CNC_MAC(3,1)}

    /* UBM4-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    34/*LPM34*/       ,35/*LPM35*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,2)      , CNC_MAC(3,3)}

    /* UBM4-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    37/*LPM37*/       ,36/*LPM36*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,6)      , CNC_MAC(3,7)}

    /* UBM4-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    32/*LPM32*/       ,33/*LPM33*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,4)      , CNC_MAC(3,5)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 5 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm5[] =
{
    /* UBM5-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    22/*LPM22*/       ,23/*LPM23*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,4)      , CNC_MAC(0,5)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,8)      , CNC_MAC(2,9)}

    /* UBM5-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    18/*LPM18*/       ,19/*LPM19*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,6)      , CNC_MAC(0,7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,10)     , CNC_MAC(2,11)}

    /* UBM5-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    20/*LPM20*/       ,21/*LPM21*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,10)     , CNC_MAC(0,11)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,14)     , CNC_MAC(2,15)}

    /* UBM5-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    16/*LPM16*/       ,17/*LPM17*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,8)      , CNC_MAC(0,9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,12)     , CNC_MAC(2,13)}

    /* UBM5-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    46/*LPM46*/       ,47/*LPM47*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,8)      , CNC_MAC(3,9)}

    /* UBM5-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    42/*LPM42*/        ,43/*LPM43*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,10)     , CNC_MAC(3,11)}

    /* UBM5-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    44/*LPM44*/       ,45/*LPM45*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,14)     , CNC_MAC(3,15)}

    /* UBM5-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    40/*LPM40*/       ,41/*LPM41*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,12)     , CNC_MAC(3,13)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 6 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm6[] =
{
    /* UBM6-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    0/*LPM0*/         , 1/*LPM1*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,12)     , CNC_MAC(0,13)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,0)      , CNC_MAC(2,1)}

    /* UBM6-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    6/*LPM6*/         , 7/*LPM7*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,14)     , CNC_MAC(0,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,6)      , CNC_MAC(2,7)}

    /* UBM6-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    4/*LPM4*/         , 5/*LPM5*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,6)      , CNC_MAC(0,7)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,4)      , CNC_MAC(2,5)}

    /* UBM6-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    2/*LPM2*/         , 3/*LPM3*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,4)      , CNC_MAC(0,5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,2)      , CNC_MAC(2,3)}

    /* UBM6-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    24/*LPM24*/       ,25/*LPM25*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,0)      , CNC_MAC(3,1)}

    /* UBM6-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    30/*LPM30*/       ,31/*LPM31*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,6)      , CNC_MAC(3,7)}

    /* UBM6-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    28/*LPM28*/       ,29/*LPM29*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,4)      , CNC_MAC(3,5)}

    /* UBM6-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    26/*LPM26*/       ,27/*LPM27*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,2)      , CNC_MAC(3,3)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 7 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm7[] =
{
    /* UBM7-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(8)  ,GT_SAME_AS_0}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    8/*LPM8*/         , 9/*LPM9*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,8)      , CNC_MAC(0,9)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,8)      , CNC_MAC(2,9)}

    /* UBM7-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(9)  ,GT_SAME_AS_0}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    14/*LPM14*/       ,15/*LPM15*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,10)     , CNC_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,14)     , CNC_MAC(2,15)}

    /* UBM7-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(10) ,GT_SAME_AS_0}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    12/*LPM12*/       ,13/*LPM13*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,14)     , CNC_MAC(0,15)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,12)     , CNC_MAC(2,13)}

    /* UBM7-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(11) ,GT_SAME_AS_0}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    10/*LPM10*/       ,11/*LPM11*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,12)     , CNC_MAC(0,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,10)     , CNC_MAC(2,11)}

    /* UBM7-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(12) ,GT_SAME_AS_0}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    32/*LPM32*/       ,33/*LPM33*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,8)      , CNC_MAC(3,9)}

    /* UBM7-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(13) ,GT_SAME_AS_0}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    38/*LPM38*/       ,39/*LPM39*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,14)     , CNC_MAC(3,15)}

    /* UBM7-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(14) ,GT_SAME_AS_0}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    36/*LPM36*/       ,37/*LPM37*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,12)     , CNC_MAC(3,13)}

    /* UBM7-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(15) ,GT_SAME_AS_0}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    34/*LPM34*/       ,35/*LPM35*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,10)     , CNC_MAC(3,11)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 8 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm8[] =
{
    /* UBM8-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(0)  ,GT_SAME_AS_0}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    16/*LPM16*/       ,17/*LPM17*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,4)      , CNC_MAC(0,5)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,6)      , CNC_MAC(2,7)}

    /* UBM8-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(1)  ,GT_SAME_AS_0}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    22/*LPM22*/       ,23/*LPM23*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,6)      , CNC_MAC(0,7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,2)      , CNC_MAC(2,3)}

    /* UBM8-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(2)  ,GT_SAME_AS_0}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    20/*LPM20*/       ,21/*LPM21*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,8)      , CNC_MAC(0,9)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,4)      , CNC_MAC(2,5)}

    /* UBM8-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(3)  ,GT_SAME_AS_0}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    18/*LPM18*/       ,19/*LPM19*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,10)     , CNC_MAC(0,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,0)      , CNC_MAC(2,1)}

    /* UBM8-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(4)  ,GT_SAME_AS_0}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    40/*LPM40*/       ,41/*LPM41*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,6)      , CNC_MAC(3,7)}

    /* UBM8-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(5)  ,GT_SAME_AS_0}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    46/*LPM46*/       ,47/*LPM47*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,2)      , CNC_MAC(3,3)}

    /* UBM8-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(6)  ,GT_SAME_AS_0}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    44/*LPM44*/       ,45/*LPM45*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,4)      , CNC_MAC(3,5)}

    /* UBM8-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(7)  ,GT_SAME_AS_0}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    42/*LPM42*/       ,43/*LPM43*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,0)      , CNC_MAC(3,1)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 9 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm9[] =
{
    /* UBM9-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(8)  ,GT_SAME_AS_0}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    0/*LPM0*/         , 1/*LPM1*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,12)     , CNC_MAC(0,13)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,14)     , CNC_MAC(2,15)}

    /* UBM9-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(9)  ,GT_SAME_AS_0}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    2/*LPM2*/         , 3/*LPM3*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,14)     , CNC_MAC(0,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,10)     , CNC_MAC(2,11)}

    /* UBM9-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(10) ,GT_SAME_AS_0}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    6/*LPM6*/         , 7/*LPM7*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,4)      , CNC_MAC(0,5)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,12)     , CNC_MAC(2,13)}

    /* UBM9-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(11) ,GT_SAME_AS_0}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    4/*LPM4*/         , 5/*LPM5*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,6)      , CNC_MAC(0,7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,8)      , CNC_MAC(2,9)}

    /* UBM9-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(12) ,GT_SAME_AS_0}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    24/*LPM24*/       ,25/*LPM25*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,14)     , CNC_MAC(3,15)}

    /* UBM9-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(13) ,GT_SAME_AS_0}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    26/*LPM26*/       ,27/*LPM27*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,10)     , CNC_MAC(3,11)}

    /* UBM9-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(14) ,GT_SAME_AS_0}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    30/*LPM30*/       , 31/*LPM31*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,12)     , CNC_MAC(3,13)}

    /* UBM9-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(15) ,GT_SAME_AS_0}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    28/*LPM28*/       ,29/*LPM29*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,8)      , CNC_MAC(3,9)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 10 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm10[] =
{
    /* UBM10-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(0)  ,GT_SAME_AS_0}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    8/*LPM8*/         , 9/*LPM9*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,8)      , CNC_MAC(0,9)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,0)      , CNC_MAC(2,1)}

    /* UBM10-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(1)  ,GT_SAME_AS_0}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    10/*LPM10*/       ,11/*LPM11*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,10)     , CNC_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,6)      , CNC_MAC(2,7)}

    /* UBM10-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(2) ,GT_SAME_AS_0}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    14/*LPM14*/       ,15/*LPM15*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,12)     , CNC_MAC(0,13)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,4)      , CNC_MAC(2,5)}

    /* UBM10-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(3) ,GT_SAME_AS_0}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    12/*LPM12*/       ,13/*LPM13*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,14)     , CNC_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,2)      , CNC_MAC(2,3)}

    /* UBM10-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(4) ,GT_SAME_AS_0}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    32/*LPM32*/       ,33/*LPM33*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,0)      , CNC_MAC(3,1)}

    /* UBM10-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(5) ,GT_SAME_AS_0}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    34/*LPM34*/       ,35/*LPM35*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,6)      , CNC_MAC(3,7)}

    /* UBM10-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(6) ,GT_SAME_AS_0}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    38/*LPM38*/       , 39/*LPM39*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,4)      , CNC_MAC(3,5)}

    /* UBM10-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(7) ,GT_SAME_AS_0}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    36/*LPM36*/       ,37/*LPM37*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,2)      , CNC_MAC(3,3)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 11 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm11[] =
{
    /* UBM11-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(8)  ,GT_SAME_AS_0}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_LPM_E   ,    16/*LPM16*/       ,17/*LPM17*/}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,4)      , CNC_MAC(0,5)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,8)      , CNC_MAC(2,9)}

    /* UBM11-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(9)  ,GT_SAME_AS_0}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_LPM_E   ,    18/*LPM18*/       ,19/*LPM19*/}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,6)      , CNC_MAC(0,7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,14)     , CNC_MAC(2,15)}

    /* UBM11-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(10) ,GT_SAME_AS_0}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_LPM_E   ,    22/*LPM22*/       , 23/*LPM23*/}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,10)     , CNC_MAC(0,11)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,12)     , CNC_MAC(2,13)}

    /* UBM11-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(11) ,GT_SAME_AS_0}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    20/*LPM20*/       ,21/*LPM21*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,8)      , CNC_MAC(0,9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,10)     , CNC_MAC(2,11)}

    /* UBM11-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(12) ,GT_SAME_AS_0}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_LPM_E   ,    40/*LPM40*/       ,41/*LPM41*/}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,8)      , CNC_MAC(3,9)}

    /* UBM11-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(13) ,GT_SAME_AS_0}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_LPM_E   ,    42/*LPM42*/       ,43/*LPM43*/}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,14)     , CNC_MAC(3,15)}

    /* UBM11-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(14) ,GT_SAME_AS_0}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_LPM_E   ,    46/*LPM46*/       , 47/*LPM47*/}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,12)     , CNC_MAC(3,13)}

    /* UBM11-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(15) ,GT_SAME_AS_0}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    44/*LPM44*/       ,45/*LPM45*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,10)     , CNC_MAC(3,11)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 12 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm12[] =
{
    /* UBM12-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(0)  ,GT_SAME_AS_0}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,12)      , CNC_MAC(0,13)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,6)      , CNC_MAC(2,7)}

    /* UBM12-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(1)  ,GT_SAME_AS_0}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,14)     , CNC_MAC(0,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,2)      , CNC_MAC(2,3)}

    /* UBM12-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(2) ,GT_SAME_AS_0}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,6)      , CNC_MAC(0,7)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,4)      , CNC_MAC(2,5)}

    /* UBM12-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(3) ,GT_SAME_AS_0}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    16/*LPM12*/       ,17/*LPM13*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,4)      , CNC_MAC(0,5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,0)      , CNC_MAC(2,1)}

    /* UBM12-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(4) ,GT_SAME_AS_0}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,6)      , CNC_MAC(3,7)}

    /* UBM12-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(5) ,GT_SAME_AS_0}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,2)      , CNC_MAC(3,3)}

    /* UBM12-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(6) ,GT_SAME_AS_0}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,4)      , CNC_MAC(3,5)}

    /* UBM12-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(7) ,GT_SAME_AS_0}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    40/*LPM40*/       ,41/*LPM41*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,0)      , CNC_MAC(3,1)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 13 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm13[] =
{
    /* UBM13-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(8)  ,GT_SAME_AS_0}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 0),EM_MAC(0, 2)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 8),EM_MAC(0,10)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 2),EM_MAC(1, 0)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,10),EM_MAC(1, 8)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 4),EM_MAC(2, 6)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,12),EM_MAC(2,14)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 6),EM_MAC(3, 4)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,14),EM_MAC(3,12)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,8)      , CNC_MAC(0,9)}
    ,{SBM_ID(0),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,14)     , CNC_MAC(2,15)}

    /* UBM13-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(9)  ,GT_SAME_AS_0}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 1),EM_MAC(0, 3)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0, 9),EM_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 3),EM_MAC(1, 1)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,11),EM_MAC(1, 9)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 5),EM_MAC(2, 7)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2,13),EM_MAC(2,15)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 7),EM_MAC(3, 5)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,15),EM_MAC(3,13)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,10)     , CNC_MAC(0,11)}
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,10)     , CNC_MAC(2,11)}

    /* UBM13-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(10) ,GT_SAME_AS_0}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 4),EM_MAC(0, 6)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,12),EM_MAC(0,14)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 6),EM_MAC(1, 4)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,14),EM_MAC(1,12)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 0),EM_MAC(2, 2)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 8),EM_MAC(2,10)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 2),EM_MAC(3, 0)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,10),EM_MAC(3, 8)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,14)     , CNC_MAC(0,15)}
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,12)     , CNC_MAC(2,13)}

    /* UBM13-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(11) ,GT_SAME_AS_0}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(0, 5),EM_MAC(0, 7)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(0,13),EM_MAC(0,15)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(1, 7),EM_MAC(1, 5)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(1,15),EM_MAC(1,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(2, 1),EM_MAC(2, 3)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(2, 9),EM_MAC(2,11)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(3, 3),EM_MAC(3, 1)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(3,11),EM_MAC(3, 9)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_LPM_E   ,    18/*LPM18*/       ,19/*LPM19*/}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,12)     , CNC_MAC(0,13)}
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(2,8)      , CNC_MAC(2,9)}

    /* UBM13-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(12) ,GT_SAME_AS_0}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 0),EM_MAC(4, 2)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 8),EM_MAC(4,10)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 2),EM_MAC(5, 0)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,10),EM_MAC(5, 8)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 4),EM_MAC(6, 6)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,12),EM_MAC(6,14)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 6),EM_MAC(7, 4)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,14),EM_MAC(7,12)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,14)     , CNC_MAC(3,15)}

    /* UBM13-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(13) ,GT_SAME_AS_0}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 1),EM_MAC(4, 3)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4, 9),EM_MAC(4,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 3),EM_MAC(5, 1)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,11),EM_MAC(5, 9)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 5),EM_MAC(6, 7)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6,13),EM_MAC(6,15)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 7),EM_MAC(7, 5)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,15),EM_MAC(7,13)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,10)     , CNC_MAC(3,11)}

    /* UBM13-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(14) ,GT_SAME_AS_0}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 4),EM_MAC(4, 6)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,12),EM_MAC(4,14)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 6),EM_MAC(5, 4)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,14),EM_MAC(5,12)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 0),EM_MAC(6, 2)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 8),EM_MAC(6,10)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 2),EM_MAC(7, 0)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,10),EM_MAC(7, 8)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_ARP_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_TS_E    ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF0_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_HF1_E   ,    NO_INFO           , NO_INFO  }
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,12)     , CNC_MAC(3,13)}

    /* UBM13-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(15) ,GT_SAME_AS_0}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E   ,EM_MAC(4, 5),EM_MAC(4, 7)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E  ,EM_MAC(4,13),EM_MAC(4,15)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E   ,EM_MAC(5, 7),EM_MAC(5, 5)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E  ,EM_MAC(5,15),EM_MAC(5,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E   ,EM_MAC(6, 1),EM_MAC(6, 3)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E  ,EM_MAC(6, 9),EM_MAC(6,11)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E   ,EM_MAC(7, 3),EM_MAC(7, 1)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ,EM_MAC(7,11),EM_MAC(7, 9)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_LPM_E   ,    42/*LPM42*/       ,43/*LPM43*/}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_2_3_E,   CNC_MAC(3,8)      , CNC_MAC(3,9)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 14 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm14[] =
{
    /* UBM14-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(0)  ,GT_SAME_AS_0}

    /* UBM14-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(1)  ,GT_SAME_AS_0}

    /* UBM14-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(2)  ,GT_SAME_AS_0}

    /* UBM14-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(3)  ,GT_SAME_AS_0}

    /* UBM14-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(4)  ,GT_SAME_AS_0}
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,8)      , CNC_MAC(1,9)}

    /* UBM14-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(5)  ,GT_SAME_AS_0}
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,10)     , CNC_MAC(1,11)}

    /* UBM14-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(6)  ,GT_SAME_AS_0}
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,12)     , CNC_MAC(1,13)}

    /* UBM14-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_FDB_E              ,FDB_MAC(7)  ,GT_SAME_AS_0}
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,14)     , CNC_MAC(1,15)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 15 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm15[] =
{
    /* UBM15-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/

    /* UBM15-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_CNT_E,   NO_INFO           , GT_NA    }/*port1 not valid*/

    /* UBM15-SBM 2 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(2),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,0)      , CNC_MAC(0,1)}

    /* UBM15-SBM 3 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(3),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(0,2)      , CNC_MAC(0,3)}

    /* UBM15-SBM 4 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(4),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,0)     , CNC_MAC(1,1)}

    /* UBM15-SBM 5 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(5),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,2)      , CNC_MAC(1,3)}

    /* UBM15-SBM 6 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(6),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,4)      , CNC_MAC(1,5)}

    /* UBM15-SBM 7 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(7),AAS_SBM_CLIENT_TYPE_CNC_0_1_E,   CNC_MAC(1,6)      , CNC_MAC(1,7)}

    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS UBM 16 - list of all optional clients and their info (per SBM) */
static const AAS_SBM_CLIENT_INFO_STC aas_ubm16[] =
{
    /* UBM16-SBM 0 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
     {SBM_ID(0),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/

    /* UBM16-SBM 1 */
    /* SBM-id , client-id */                   /*clientSubId_port0*//*clientSubId_port1*/
    ,{SBM_ID(1),AAS_SBM_CLIENT_TYPE_PLR_MET_E,   NO_INFO           , GT_NA    }/*port1 not valid*/


    ,{GT_NA,AAS_SBM_CLIENT_TYPE__LAST__E,GT_NA,GT_NA}
};

/* AAS ALL UBMs */
static const AAS_SBM_CLIENT_INFO_STC* const aas_ubm_arr[NUM_OF_UBMS] =
{
      &aas_ubm0[0]
     ,&aas_ubm1[0]
     ,&aas_ubm2[0]
     ,&aas_ubm3[0]
     ,&aas_ubm4[0]
     ,&aas_ubm5[0]
     ,&aas_ubm6[0]
     ,&aas_ubm7[0]
     ,&aas_ubm8[0]
     ,&aas_ubm9[0]
     ,&aas_ubm10[0]
     ,&aas_ubm11[0]
     ,&aas_ubm12[0]
     ,&aas_ubm13[0]
     ,&aas_ubm14[0]
     ,&aas_ubm15[0]
     ,&aas_ubm16[0]
};


#define ROUND_UP_DIVIDE_MAC(subject,divider) (GT_U32)((subject)+(divider)-1)/(divider)

/**
* @internal calcNumSbmsNeeded function
* @endinternal
*
* @brief   function to calculate the number of SBMs that needed to support the
*       number of entries that the client need .
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       numEntriesNeeded  - the number of entries that the client needed
*       aasShmClientsInfoPtr  - (pointer to) the client info
*       namePtr - the name of the client - for error log purposes
*       SORTED__limitedNumSbmValuesPtr - (pointer to) sorted list of number of
*           SBMs that the client may use.
*           ignored if NULL
*
* @param[out]
*       numSbmsNeededPtr - (pointer to) the number of SBMs that the client will need to use.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_BAD_PARAM   - when the number of needed SBMs is more than the
*                           SHM unit supports for this client
*
*/
static GT_STATUS calcNumSbmsNeeded(
    IN GT_U32   numEntriesNeeded,
    IN const AAS_CLIENT_INFO_STC *aasShmClientsInfoPtr,
    IN GT_CHAR  *namePtr,
    IN const GT_U32 *SORTED__limitedNumSbmValuesPtr,/* must hold SORTED list */
    OUT GT_U32  *numSbmsNeededPtr
)
{
    GT_U32 ii;
    GT_U32 numSbmsNeeded;

    namePtr = namePtr;/* avoid compilation error when CPSS_LOG_ENABLE not defined */

    numSbmsNeeded = ROUND_UP_DIVIDE_MAC(numEntriesNeeded,
                    aasShmClientsInfoPtr->numEntriesPerSbm);

    if(aasShmClientsInfoPtr->numOfConsecutiveSbmsNeeded)
    {
        numSbmsNeeded *= aasShmClientsInfoPtr->numOfConsecutiveSbmsNeeded;
    }

    if(numSbmsNeeded < aasShmClientsInfoPtr->minNumSbms)
    {
        numSbmsNeeded = aasShmClientsInfoPtr->minNumSbms;
    }

    if(numSbmsNeeded > aasShmClientsInfoPtr->maxNumSbms)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "[%s] not support more than [%d] entries (but requested for[%d])",
            namePtr,
            aasShmClientsInfoPtr->maxNumSbms *
            aasShmClientsInfoPtr->numEntriesPerSbm,
            numEntriesNeeded);
    }

    if(SORTED__limitedNumSbmValuesPtr)
    {
        /* assuming SORTED__limitedNumSbmValuesPtr[] is sorted */
        for(ii = 0 ; SORTED__limitedNumSbmValuesPtr[ii] != GT_NA ; ii++)
        {
            if(numSbmsNeeded == SORTED__limitedNumSbmValuesPtr[ii])
            {
                /* good value that not need update */
                break;
            }

            if(numSbmsNeeded > SORTED__limitedNumSbmValuesPtr[ii])
            {
                continue;
            }

            /* we got numSbmsNeeded is less than SORTED__limitedNumSbmValuesPtr[ii] */
            /* so need to use SORTED__limitedNumSbmValuesPtr[ii] */
            numSbmsNeeded = SORTED__limitedNumSbmValuesPtr[ii];
            break;
        }
    }


    *numSbmsNeededPtr = numSbmsNeeded;

    return GT_OK;
}
/* hold info as needed by register :
/Cider/EBU-IP/SIP/SHM_IP/SIP 7.0 (Seahawk1)/SHM_IP {Current}/SHM/SHM/Block <%n> Port Select
sbm_port1_lpm_taken_from_port0
sbm_port0_lpm_taken_from_port1
sbm_port1_lpm_address_segment
sbm_port0_lpm_address_segment
sbm_port_mode
sbm_port1_select
sbm_port0_select
*/
typedef struct {

    AAS_SBM_CLIENT_TYPE_ENT sbm_port0_select;
    AAS_SBM_CLIENT_TYPE_ENT sbm_port1_select;
    GT_BOOL                 sbm_port_mode;

    struct {
        GT_U32              sbm_port0_lpm_address_segment;
        GT_U32              sbm_port1_lpm_address_segment;
        GT_U32              sbm_port0_lpm_taken_from_port1;
        GT_U32              sbm_port1_lpm_taken_from_port0;
    }lpmSubInfo;
}PORT_SELECT_INFO_STC;

/* max number of SHM SBMs : single tile */
#define MAX_SHM_SBMS_SINGLE_TILE   130

/* max number of SHM SBMs */
#define MAX_SHM_SBMS   (MAX_SHM_SBMS_SINGLE_TILE * 2/*tiles*/)

static GT_STATUS shmBlockPortSelectWriteToHw
(
    IN  GT_U8       devNum,
    OUT PORT_SELECT_INFO_STC portSelectArr[/*MAX_SHM_SBMS*/]
);
static GT_STATUS fdbWriteToHwAndUpdateFineTune
(
    IN  GT_U8       devNum ,
    IN  GT_U32      fdbNumSbms
);
static GT_STATUS emWriteToHwAndUpdateFineTune
(
    IN  GT_U8       devNum ,
    IN  GT_U32      emId,/*0..7*/
    IN  GT_U32      emNumSbms,
    IN  GT_BOOL     autoLearnEn
);
static GT_STATUS shmCncIpeMuxWriteToHw
(
    IN  GT_U8       devNum,
    IN  GT_U32      ipe0_usedCncBlocksBmp,
    IN  GT_U32      ipe1_usedCncBlocksBmp
);
static GT_STATUS  wmOnly_sharedTablesSizeSet(
    IN  GT_U8                   devNum,
    IN  AAS_SBM_CLIENT_TYPE_ENT client,
    IN  GT_U32                  tableSize,
    IN  GT_U32                  numOfSbms
);
static GT_STATUS  wmOnly_sharedTablesPairsInfo(
    IN  GT_U8                   devNum,
    IN  GT_U32                  orig_tileId,
    IN  AAS_SBM_CLIENT_TYPE_ENT orig_client,
    IN  GT_U32                  sharing_tileId,
    IN  AAS_SBM_CLIENT_TYPE_ENT sharing_client
);


/**
 * @struct WM__SHARING_PAIRS_INFO_STC
 *
 * @brief This struct hold info about pairs of tables that share the memory between them
 *     meaning using the same memory pointer.
 *     example :
 *     tile 0 , EM 6 with tile 0 EM 7 (the 2 EM clients : TTI-ILM2,TTI-ILM3 --> NOT shared between tiles)
 *
 *     other example :
 *     tile 0 , EM 0 with tile 1 EM 0 (allow this EM to have twice the size of single EM , and share it between tiles !)
 *  (APPLICABLE DEVICES: AAS.)
 *
 */
typedef struct{
    /*
    */
    GT_BOOL                 isUsed;
    GT_U32                  orig_tileId;
    AAS_SBM_CLIENT_TYPE_ENT orig_client;
    GT_U32                  sharing_tileId;
    AAS_SBM_CLIENT_TYPE_ENT sharing_client;
}WM__SHARING_PAIRS_INFO_STC;

static GT_STATUS emxWriteEmClientsToHw
(
    IN  GT_U8       devNum,
    IN  GT_U32      numEmUnitsNeeded,
    IN  CPSS_DXCH_PP_CONFIG_SHARED_EM_UNIT_STC  emUnitsArr[/*CPSS_DXCH_PP_CONFIG_SHARED_MAX_NUM_EM_UNITS_E*/]
);

/**
* @internal applyShmClientFdb function
* @endinternal
*
* @brief   the function need to fill info into : portSelectArr[] for the FDB client
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       fdbNumSbms  - the number of SBMs that the FDB need
*
* @param[out]
*   portSelectArr[] -   (array of) all the 130/260 SBM of the tile/2 tiles ,
*                       that need to hold this client info (per SBM)
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR    - on hardware error.
* @retval GT_BAD_PARAM   - on wrong devNum or other table parameter.
*
*/
static GT_STATUS applyShmClientFdb
(
    IN  GT_U8       devNum,
    IN  GT_U32      fdbNumSbms,
    OUT PORT_SELECT_INFO_STC portSelectArr[/*MAX_SHM_SBMS*/]
)
{
    GT_U32  ii;
    GT_U32  globalSmbId;
    GT_U32  usedSbms;
    const AAS_SBM_CLIENT_INFO_STC*  currUbmPtr;
    const AAS_SBM_CLIENT_INFO_STC*  currSbmPtr;
    AAS_SBM_CLIENT_TYPE_ENT sbmClientType = AAS_SBM_CLIENT_TYPE_FDB_E;

    usedSbms = 0;

    /* loop on UBMs (reverse order) */
    for(ii = NUM_OF_UBMS-1 ; ii > 0 ; ii--)
    {
        currUbmPtr = aas_ubm_arr[ii];

        /* loop on potential clients of SBMs in the UBM */
        for(currSbmPtr = &currUbmPtr[0] ;
            currSbmPtr->sbmId != GT_NA ; currSbmPtr++)
        {
            if(currSbmPtr->client != sbmClientType)
            {
                continue;
            }

            globalSmbId = (ii * 8) + currSbmPtr->sbmId;

            /*
                working on FDB - share the SBM between MAC DA and MAC SA compare -
                So both port0 and port1 will point to FDB
            */

            portSelectArr[globalSmbId].sbm_port0_select = sbmClientType;
            portSelectArr[globalSmbId].sbm_port1_select = sbmClientType;

            usedSbms++;

            if(usedSbms == fdbNumSbms)
            {
                /* we filled all the needed indexes in portSelectArr[] */

                /*  write the FDB unit with the FDB size and MHT
                    and will update the DB info:
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb
                    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size
                    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank
                */
                return fdbWriteToHwAndUpdateFineTune(devNum,fdbNumSbms);
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
        "The FDB client found place for [%d] SBMs out of [%d] needed SBMs",
        usedSbms,fdbNumSbms);
}


/**
* @internal applyShmClientEM function
* @endinternal
*
* @brief   the function need to fill info into : portSelectArr[] for the EM client
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       emId    - the em client ID (0..7)
*       emNumSbms  - the number of SBMs that the EM need
*       singleTableUsed - indication that this table shared with the next table.
*       autoLearnEn - when GT_TRUE indication that this client can't be shared due to auto learn by this client.
*
* @param[out]
*   portSelectArr[] -   (array of) all the 130/260 SBM of the tile/2 tiles ,
*                       that need to hold this client info (per SBM)
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR    - on hardware error.
* @retval GT_BAD_PARAM   - on wrong devNum or other table parameter.
*
*/
static GT_STATUS applyShmClientEM
(
    IN  GT_U8       devNum,
    IN  GT_U32      emId,/*0..7*/
    IN  GT_U32      emNumSbms,
    OUT PORT_SELECT_INFO_STC portSelectArr[/*MAX_SHM_SBMS*/],
    IN  GT_BOOL     singleTableUsed,
    IN  GT_BOOL     autoLearnEn,
    INOUT WM__SHARING_PAIRS_INFO_STC    wm__sharing_pairs[/*WM__SHARING_PAIRS_MAX_NUM_CNS*/],
    INOUT GT_U32   *wm__num_sharing_pairsPtr
)
{
    GT_U32  ii;
    GT_U32  globalSmbId;
    GT_U32  usedSbms;
    const AAS_SBM_CLIENT_INFO_STC*  currUbmPtr;
    const AAS_SBM_CLIENT_INFO_STC*  currSbmPtr;
    GT_U32  mhtIndex;
    AAS_SBM_CLIENT_TYPE_ENT sbmClientType_low,sbmClientType_high,sbmClientType;
    GT_U32  bmpNotUsedMht;

    usedSbms = 0;

    if(emNumSbms == 0)
    {
        return GT_OK;
    }

    sbmClientType_low  = AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E + (emId % 4) * 2;
    sbmClientType_high = sbmClientType_low + 1;

    if(autoLearnEn == GT_TRUE && singleTableUsed == GT_TRUE)
    {
        /* we already checked this condition , so we should not get here
           but just for the sake of preventing mistake , do check here too
        */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "The EM table[%d] cant use autoLearnEn == GT_TRUE and singleTableUsed == GT_TRUE",
            emId);
    }

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 1 &&
        (singleTableUsed == GT_TRUE))
    {
        /* we add the client at this point */
        wm__sharing_pairs[*wm__num_sharing_pairsPtr].isUsed = GT_TRUE;
        wm__sharing_pairs[*wm__num_sharing_pairsPtr].orig_client = (AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E + emId);
        (*wm__num_sharing_pairsPtr)++;
    }

    for(ii = 0 ; ii < (*wm__num_sharing_pairsPtr) ; ii++)
    {
        if(wm__sharing_pairs[ii].isUsed == GT_FALSE)
        {
            continue;
        }

        if(wm__sharing_pairs[ii].orig_client == (AAS_SBM_CLIENT_TYPE_ENT)(AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E + emId))
        {
            if(autoLearnEn == GT_TRUE)
            {
                /* this table can't be shared so 'invalidate' it */
                wm__sharing_pairs[ii].isUsed = GT_FALSE;
                break;
            }

            if(singleTableUsed == GT_TRUE)
            {
                /* already in single tile (and in dual tiles)
                   the table is shared with in it's own tile
                */
                wm__sharing_pairs[ii].orig_tileId    = 0;
                wm__sharing_pairs[ii].sharing_tileId = 0;
                wm__sharing_pairs[ii].orig_client    = (AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E + emId);
                wm__sharing_pairs[ii].sharing_client = (AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E + emId)+1;

                if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles > 1)
                {
                    /* this entry ignored for single tile device */
                    wm__sharing_pairs[ii+1].orig_tileId    = 1;
                    wm__sharing_pairs[ii+1].sharing_tileId = 1;
                    wm__sharing_pairs[ii+1].orig_client    = (AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E + emId);
                    wm__sharing_pairs[ii+1].sharing_client = (AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E + emId)+1;
                }
            }
            else
            {
                /* shared between tiles , we already set the wm__sharing_pairs[ii] properly */
            }

            break;
        }
    }

    if(emNumSbms < 16)
    {
        bmpNotUsedMht = 0xFFFF >> (16 - emNumSbms);
    }
    else
    {
        bmpNotUsedMht = 0xFFFF;
    }

    /* loop on UBMs */
    for(ii = 2 ; ii < NUM_OF_UBMS ; ii++)
    {
        currUbmPtr = aas_ubm_arr[ii];

        /* loop on potential clients of SBMs in the UBM */
        for(currSbmPtr = &currUbmPtr[0] ;
            currSbmPtr->sbmId != GT_NA ; currSbmPtr++)
        {
            if(currSbmPtr->client != sbmClientType_low   &&
               currSbmPtr->client != sbmClientType_high)
            {
                /* not relevant to this EM  */
                continue;
            }

            if(emId < 4 && currSbmPtr->sbmId >= 4)
            {
                /* SMBs 4..7 are for EMs 4..7 */
                continue;
            }
            if(emId >= 4 && currSbmPtr->sbmId < 4)
            {
                /* SMBs 0..1 are for EMs 0..3 */
                continue;
            }

            globalSmbId = (ii * 8) + currSbmPtr->sbmId;

            if(portSelectArr[globalSmbId].sbm_port0_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId].sbm_port1_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }

            mhtIndex = currSbmPtr->clientSubId_port0 & 0xF;
            sbmClientType = mhtIndex >= 8 ? sbmClientType_high : sbmClientType_low;
            if(currSbmPtr->clientSubId_port0 == EM_MAC(emId ,mhtIndex) &&
               ((1 << mhtIndex) & bmpNotUsedMht))
            {
                /* this MHT not used yet , we can use it */
                /*
                    from SHM AAS MAS doc :
                    Must hold for same EM at least 2 SBMs on same port - for same EM channel
                    Example: connect EM2 Low MHTs on SBM2,3 port0
                */
                portSelectArr[globalSmbId].sbm_port0_select =
                    sbmClientType;
                if(singleTableUsed == GT_TRUE)
                {
                    portSelectArr[globalSmbId].sbm_port1_select =
                        sbmClientType;
                }
                /* remove the MHT from the BMP of non used MHT */
                bmpNotUsedMht &= ~(1 << (currSbmPtr->clientSubId_port0 & 0xF));
            }
            else
            {
                mhtIndex = currSbmPtr->clientSubId_port1 & 0xF;
                if(currSbmPtr->clientSubId_port1 == EM_MAC(emId ,mhtIndex) &&
                   ((1 << mhtIndex) & bmpNotUsedMht))
                {
                    /* this MHT not used yet , we can use it */
                    /*
                        from SHM AAS MAS doc :
                        Must hold for same EM at least 2 SBMs on same port - for same EM channel
                        Example: connect EM2 Low MHTs on SBM2,3 port0
                    */
                    portSelectArr[globalSmbId].sbm_port1_select =
                        sbmClientType;
                    /* remove the MHT from the BMP of non used MHT */
                    bmpNotUsedMht &= ~(1 << (currSbmPtr->clientSubId_port1 & 0xF));
                    if(singleTableUsed == GT_TRUE)
                    {
                        portSelectArr[globalSmbId].sbm_port0_select =
                            sbmClientType;
                    }
                }
                else
                {
                    /* this {EM,MHT} not match the entry , or already used */
                    continue;
                }
            }

            usedSbms ++;
            /* jump over the next one too , as it uses the same SBM as current */
            currSbmPtr++;

            if(usedSbms == emNumSbms)
            {
                /* we filled all the needed indexes in portSelectArr[] */

                /*  write the EM unit with the EM size and MHT
                    and will update the DB info:
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum
                    PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks
                */
                return emWriteToHwAndUpdateFineTune(devNum,emId,emNumSbms,autoLearnEn);
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
        "The EM table[%d] found place for [%d] SBMs out of [%d] needed SBMs",
        emId,usedSbms,emNumSbms);
}


/**
* @internal applyShmClientArpTsHf function
* @endinternal
*
* @brief   the function need to fill info into : portSelectArr[] for the ARP/TS/HF0,1 client
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       sbmClientType - the client
*       numSbms  - the number of SBMs that the ARP/TS/HF0,1 need
*
* @param[out]
*   portSelectArr[] -   (array of) all the 130/260 SBM of the tile/2 tiles ,
*                       that need to hold this client info (per SBM)
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR    - on hardware error.
* @retval GT_BAD_PARAM   - on wrong devNum or other table parameter.
*
*/
static GT_STATUS applyShmClientArpTsHf
(
    IN  GT_U8       devNum,
    IN GT_CHAR*     clientName,
    IN  AAS_SBM_CLIENT_TYPE_ENT sbmClientType,/*ARP/TS/HF0,1*/
    IN  GT_U32      numSbms,
    OUT PORT_SELECT_INFO_STC portSelectArr[/*MAX_SHM_SBMS*/],
    OUT GT_U32      *numLinesPtr
)
{
    GT_U32  ii;
    GT_U32  globalSmbId;
    GT_U32  usedSbms;
    const AAS_SBM_CLIENT_INFO_STC*  currUbmPtr;
    const AAS_SBM_CLIENT_INFO_STC*  currSbmPtr;

    usedSbms = 0;
    clientName = clientName;

    if(numSbms == 0)
    {
        return GT_OK;
    }

    /* loop on UBMs */
    for(ii = 0 ; ii < NUM_OF_UBMS ; ii++)
    {
        currUbmPtr = aas_ubm_arr[ii];

        /* loop on potential clients of SBMs in the UBM */
        for(currSbmPtr = &currUbmPtr[0] ;
            currSbmPtr->sbmId != GT_NA ; currSbmPtr++)
        {
            if(currSbmPtr->client != sbmClientType)
            {
                /* not relevant to this client  */
                continue;
            }

            globalSmbId = (ii * 8) + currSbmPtr->sbmId;

            /*
                Must hold the same connection at each UBM SBMs 0,1,2 and/or 4,5,6
            */
            if((currSbmPtr->sbmId %4) != 0)/* 0 or 4 */
            {
                continue;
            }

            if(portSelectArr[globalSmbId].sbm_port0_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId].sbm_port1_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId+1].sbm_port0_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId+1].sbm_port1_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId+2].sbm_port0_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId+2].sbm_port1_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }

            portSelectArr[globalSmbId + 0].sbm_port0_select =
            portSelectArr[globalSmbId + 1].sbm_port0_select =
            portSelectArr[globalSmbId + 2].sbm_port0_select =
                sbmClientType;

            usedSbms += 3;

            currSbmPtr+=2;/* the 'third' currSbmPtr++ will come from the 'for loop' */

            if(usedSbms == numSbms)
            {
                GT_U32  numEntries = (numSbms/3) * SBM_NUM_LINES;
                GT_U32  haveEntriesWithPartialLine = 0;
                /* we filled all the needed indexes in portSelectArr[] */

                switch(sbmClientType)
                {
                    case AAS_SBM_CLIENT_TYPE_ARP_E:
                        if(numEntries < (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp / 8))
                        {
                            /* shrink the limit (do not enlarge it) */
                            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp =
                                numEntries * 8;/* 8 ARPs in single line */
                        }
                        break;
                    case AAS_SBM_CLIENT_TYPE_TS_E:
                        haveEntriesWithPartialLine = (numSbms / 3) & 3;
                        numEntries /= 4;/* per 12 SBMs not 3 */
                        if(haveEntriesWithPartialLine)
                        {
                            /* the last 3/6/9 SBMs instead of 12 SBMs are used as 'partial entry' (short entry) */
                            /* the memory should return ZEROs on the high bits of the line */
                            /* and write to the the high bits of the line will be ignored  */
                            numEntries += SBM_NUM_LINES;
                        }

                        /* the numEntries here is according to 'regular TS' (not ipv6 TS) ,
                           as we calculated according to 6 SBMs and not 12 */
                        if(numEntries < PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart)
                        {
                            /* shrink the limit (do not enlarge it) */
                            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart =
                                numEntries;/* 2 regular TS in single line */
                        }
                        break;
                    case AAS_SBM_CLIENT_TYPE_HF0_E:
                    case AAS_SBM_CLIENT_TYPE_HF1_E:

                        haveEntriesWithPartialLine = (numSbms / 3) & 1;
                        numEntries /= 2;/* per 6 SBMs not 3 */
                        if(haveEntriesWithPartialLine)
                        {
                            /* the last 3 SBMs instead of 6 SBMs are used as 'half entry' (short entry) */
                            /* the memory should return ZEROs on the high bits of the line */
                            /* and write to the the high bits of the line will be ignored  */
                            numEntries += SBM_NUM_LINES;
                        }

                        if(sbmClientType == AAS_SBM_CLIENT_TYPE_HF0_E &&
                           numEntries < PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_hf0)
                        {
                            /* shrink the limit (do not enlarge it) */
                            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_hf0 =
                                numEntries;
                        }
                        else
                        if(sbmClientType == AAS_SBM_CLIENT_TYPE_HF1_E &&
                           numEntries < PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_hf1)
                        {
                            /* shrink the limit (do not enlarge it) */
                            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_hf1 =
                                numEntries;
                        }
                        break;
                    default:
                        break;/*we should not get here*/
                }

                *numLinesPtr =  numEntries;

                return GT_OK;
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
        "The client[%s] found place for [%d] SBMs out of [%d] needed SBMs",
        clientName,usedSbms,numSbms);
}


/**
* @internal applyShmClientPlr function
* @endinternal
*
* @brief   the function need to fill info into : portSelectArr[] for the PLR counters/meters client
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       sbmClientType - the client
*       numSbms  - the number of SBMs that the PLR counters/meters need
*
* @param[out]
*   portSelectArr[] -   (array of) all the 130/260 SBM of the tile/2 tiles ,
*                       that need to hold this client info (per SBM)
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR    - on hardware error.
* @retval GT_BAD_PARAM   - on wrong devNum or other table parameter.
*
*/
static GT_STATUS applyShmClientPlr
(
    IN GT_CHAR*     clientName,
    IN  AAS_SBM_CLIENT_TYPE_ENT sbmClientType,/*PLR counters/meters*/
    IN  GT_U32      numSbms,
    OUT PORT_SELECT_INFO_STC portSelectArr[/*MAX_SHM_SBMS*/]
)
{
    GT_U32  ii;
    GT_U32  globalSmbId;
    GT_U32  usedSbms;
    const AAS_SBM_CLIENT_INFO_STC*  currUbmPtr;
    const AAS_SBM_CLIENT_INFO_STC*  currSbmPtr;

    usedSbms = 0;
    clientName = clientName;

    if(numSbms == 0)
    {
        return GT_OK;
    }

    /* loop on UBMs (reverse order) , because in UBM 16,15 there SBMs that can
       be used only by the PLRs , so need to use them before the other SBMs in other UBMs */
    for(ii = NUM_OF_UBMS-1 ; ii > 0 ; ii--)
    {
        currUbmPtr = aas_ubm_arr[ii];

        /* loop on potential clients of SBMs in the UBM */
        for(currSbmPtr = &currUbmPtr[0] ;
            currSbmPtr->sbmId != GT_NA ; currSbmPtr++)
        {
            if(currSbmPtr->client != sbmClientType)
            {
                /* not relevant to this client  */
                continue;
            }

            globalSmbId = (ii * 8) + currSbmPtr->sbmId;

            /*
                Must hold the same connection at each UBM SBMs 0,1,2 and/or 4,5,6
            */
            if((currSbmPtr->sbmId %2) != 0)/* 0/2/4/6 */
            {
                continue;
            }

            if(portSelectArr[globalSmbId].sbm_port0_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId].sbm_port1_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }

            if(portSelectArr[globalSmbId+1].sbm_port0_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId+1].sbm_port1_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }

            portSelectArr[globalSmbId + 0].sbm_port0_select =
            portSelectArr[globalSmbId + 1].sbm_port0_select =
                sbmClientType;

            usedSbms += 2;

            currSbmPtr++;/* the 'second' currSbmPtr++ will come from the 'for loop' */

            if(usedSbms == numSbms)
            {
                /* we filled all the needed indexes in portSelectArr[] */
                return GT_OK;
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
        "The client[%s] found place for [%d] SBMs out of [%d] needed SBMs",
        clientName,usedSbms,numSbms);
}

/**
* @internal applyShmClientCnc function
* @endinternal
*
* @brief   the function need to fill info into : portSelectArr[] for the CNC_0_1 client (for CNC/IPE0/IPE1)
*           to set up to 6 SBMs dedicated only for CNC_0_1 (for 12 CNC blocks)
*           and can't be used for other clients
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       sbmClientType - the client
*       numSbms  - the number of SBMs that the PLR counters/meters need
*
* @param[out]
*   portSelectArr[] -   (array of) all the 130/260 SBM of the tile/2 tiles ,
*                       that need to hold this client info (per SBM)
*   cnc_0_1_UsedBlocksBmpPtr - (pointer to) the blocks that are selected to be
*       used from the CNC_0_1
*   cnc_2_3_UsedBlocksBmpPtr - (pointer to) the blocks that are selected to be
*       used from the CNC_2_3
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR    - on hardware error.
* @retval GT_BAD_PARAM   - on wrong devNum or other table parameter.
*
*/
static GT_STATUS applyShmClientCnc
(
    IN  GT_U8       devNum,
    IN GT_CHAR*     clientName,
    IN  AAS_SBM_CLIENT_TYPE_ENT sbmClientType,/*CNC_0_1 or CNC_2_3*/
    IN  GT_U32      numSbms,
    OUT PORT_SELECT_INFO_STC portSelectArr[/*MAX_SHM_SBMS*/],
    OUT GT_U32      *cnc_0_1_UsedBlocksBmpPtr,
    OUT GT_U32      *cnc_2_3_UsedBlocksBmpPtr
)
{
    GT_U32  ii;
    GT_U32  globalSmbId;
    GT_U32  usedSbms;
    const AAS_SBM_CLIENT_INFO_STC*  currUbmPtr;
    const AAS_SBM_CLIENT_INFO_STC*  currSbmPtr;
    GT_U32  bmpOfUsedBlocks;
    GT_U32  index_2_cncs;
    GT_U32  *usedBlocksBmpPtr;

    usedSbms = 0;
    clientName = clientName;

    if(numSbms == 0)
    {
        return GT_OK;
    }

    usedBlocksBmpPtr = sbmClientType == AAS_SBM_CLIENT_TYPE_CNC_2_3_E ?
        cnc_2_3_UsedBlocksBmpPtr :
        cnc_0_1_UsedBlocksBmpPtr;

    bmpOfUsedBlocks = *usedBlocksBmpPtr;/* must have been set to 0 by the caller */

    /* loop on UBMs (reverse order) , because in UBM 16,15 there SBMs that can
       be used only by the PLRs , so need to use them before the other SBMs in other UBMs */
    for(ii = NUM_OF_UBMS-1 ; ii > 0 ; ii--)
    {
        currUbmPtr = aas_ubm_arr[ii];

        /* loop on potential clients of SBMs in the UBM */
        for(currSbmPtr = &currUbmPtr[0] ;
            currSbmPtr->sbmId != GT_NA ; currSbmPtr++)
        {
            if(currSbmPtr->client != sbmClientType)
            {
                /* not relevant to this client  */
                continue;
            }

            globalSmbId = (ii * 8) + currSbmPtr->sbmId;

            if(portSelectArr[globalSmbId].sbm_port0_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId].sbm_port1_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }

            index_2_cncs = currSbmPtr->clientSubId_port0 & 0xFFFFFFFE ;/* ignore the parity bit*/

            if(0 == (bmpOfUsedBlocks & (1<<index_2_cncs)))
            {
                /* not used , can use it */
                bmpOfUsedBlocks |= 3 << index_2_cncs;
                portSelectArr[globalSmbId].sbm_port0_select = sbmClientType;
                portSelectArr[globalSmbId].sbm_port1_select = sbmClientType;
            }
            else
            {
                /* this CNC SBM can't be used as we already use the blocks that it gives */
                continue;
            }

            usedSbms ++;

            if(usedSbms == numSbms)
            {
                *usedBlocksBmpPtr = bmpOfUsedBlocks; /* update the used blocks before exit the function */

                /* we filled all the needed indexes in portSelectArr[] */
                return GT_OK;
            }
        }
    }

    *usedBlocksBmpPtr = bmpOfUsedBlocks; /* update the used blocks before exit the function */

    if(sbmClientType == AAS_SBM_CLIENT_TYPE_CNC_2_3_E)
    {
        /* we not filled all the needs from CNC_2_3 , so check if CNC_0_1 can
           help out with the extra needed blocks */
        /* apply the CNC_0_1 client on the leftovers */
        return applyShmClientCnc(devNum,"CNC_0_1 leftovers",AAS_SBM_CLIENT_TYPE_CNC_0_1_E,
            numSbms - usedSbms,&portSelectArr[0],
            cnc_0_1_UsedBlocksBmpPtr,cnc_2_3_UsedBlocksBmpPtr);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
        "The client[%s] found place for [%d] SBMs out of [%d] needed SBMs",
        clientName,usedSbms,numSbms);
}

/**
* @internal calcIpeUsedCncBlocksBmp function
* @endinternal
*
* @brief   the function  calc :
*    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe0_usedCncBlocksBmp,
*    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe1_usedCncBlocksBmp,
*
*   and update :
*    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp
*    (remove from it the CNCs that used by IPE0,1)
*
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       ipe0_numOf_cncBlocks - number of CNC blocks needed for the IPE 0
*       ipe1_numOf_cncBlocks - number of CNC blocks needed for the IPE 1
*       cnc_0_1_UsedBlocksBmpPtr - (pointer to) the blocks that are selected to be
*           used from the CNC_0_1
*
* @param[out]
*       cnc_0_1_UsedBlocksBmpPtr - (pointer to) the blocks that are selected to be
*           used from the CNC_0_1 (after removing those that used for IPE0,1)
*       ipe0_usedCncBlocksBmpPtr - (pointer to) the blocks that are selected to be
*           used from the CNC_0_1 , for the IPE0
*       ipe1_usedCncBlocksBmpPtr - (pointer to) the blocks that are selected to be
*           used from the CNC_0_1 , for the IPE1
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_BAD_STATE   - on case that there are not enough blocks in CNC_0_1 for the IPE0/1.
*
*/
static GT_STATUS calcIpeUsedCncBlocksBmp
(
    IN  GT_U32      ipe0_numOf_cncBlocks,
    IN  GT_U32      ipe1_numOf_cncBlocks,
    INOUT GT_U32    *cnc_0_1_UsedBlocksBmpPtr,
    OUT GT_U32      *ipe0_usedCncBlocksBmpPtr,
    OUT GT_U32      *ipe1_usedCncBlocksBmpPtr
)
{
    GT_U32  ii;
    GT_U32  counter_ipe0 = 0;
    GT_U32  counter_ipe1 = 0;

    if(ipe0_numOf_cncBlocks == 0 && ipe1_numOf_cncBlocks == 0)
    {
        /* no more to do */
        return GT_OK;
    }


    for(ii = 31 ; /*no condition here for end*/ ; ii--)
    {
        if(0 == ((*cnc_0_1_UsedBlocksBmpPtr) & (1 << ii)))
        {
            if(ii == 0)
            {
                /* no more possible iterations */
                /* we should not get here unless 'GT_BAD_STATE' that ipe0/1 not get
                   enough blocks */
                break;
            }

            continue;
        }

        /* this block can be given to the IPE0/1 (if needed) */
        if(counter_ipe1 < ipe1_numOf_cncBlocks)
        {
            /* IPE1 is first (before IPE0)  to get blocks from 31 --> 0 */
            counter_ipe1 ++;

            (*ipe1_usedCncBlocksBmpPtr) |= (1 << ii);
        }
        else
        if(counter_ipe0 < ipe0_numOf_cncBlocks)
        {
            /* IPE0 is last (after IPE1) to get blocks from 31 --> 0 */
            counter_ipe0 ++;

            (*ipe0_usedCncBlocksBmpPtr) |= (1 << ii);
        }

        if(counter_ipe0 == ipe0_numOf_cncBlocks &&
           counter_ipe1 == ipe1_numOf_cncBlocks)
        {
            /* no more needed --> this is 'GOOD' and GT_OK */
            /* meaning we got all the needed IPE0,1 needed blocks */
            break;
        }

        if(ii == 0)
        {
            /* no more possible iterations */
            /* we should not get here unless 'GT_BAD_STATE' that ipe0/1 not get
               enough blocks */
            break;
        }
    }

    if(counter_ipe1 != ipe1_numOf_cncBlocks ||
       counter_ipe0 != ipe0_numOf_cncBlocks)
    {
#ifdef CPSS_LOG_ENABLE
        GT_U32  numCncBlocks = prvCpssPpConfigBitmapNumBitsGet(*cnc_0_1_UsedBlocksBmpPtr);

        if(counter_ipe1 != ipe1_numOf_cncBlocks)
        {
            CPSS_LOG_ERROR_MAC(
                "CNC_0_1 not hold enough (have %d ,gave %d) blocks for IPE1 (need %d) " ,
                    numCncBlocks ,counter_ipe1 ,ipe1_numOf_cncBlocks);
        }
        if(counter_ipe0 != ipe0_numOf_cncBlocks)
        {
            CPSS_LOG_ERROR_MAC(
                "CNC_0_1 not hold enough (have %d ,gave %d) blocks for IPE1 (need %d) " ,
                    numCncBlocks ,counter_ipe0 ,ipe0_numOf_cncBlocks);
        }
#endif /*CPSS_LOG_ENABLE*/
        return /* already did error log info */GT_BAD_STATE;
    }

    /* remove from CNC_0_1 bmp the bits that used for IPE0,1 */
    (*cnc_0_1_UsedBlocksBmpPtr) &= ~ ((*ipe0_usedCncBlocksBmpPtr) | (*ipe1_usedCncBlocksBmpPtr));

    return GT_OK;
}


/**
* @internal calcCncBlocksMapping function
* @endinternal
*
* @brief   calculate PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).sip7_cnc_mapToActualBlocksUsed[...] based on
*       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp,
*       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_2_3_usedBlocksBmp
*
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       sip7_cnc_0_1_usedBlocksBmp - the blocks that are selected to be
*           used from the CNC_0_1
*       sip7_cnc_2_3_usedBlocksBmp - the blocks that are selected to be
*           used from the CNC_2_3
*
* @param[out]
*       sip7_cnc_mapToActualBlocksUsed[] - (array of) the HW blocks that CNC_0_1 and CNC_2_3
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_BAD_STATE   - unexpected error.
*
*/
static GT_STATUS calcCncBlocksMapping
(
    IN GT_U32      sip7_cnc_0_1_usedBlocksBmp,
    IN GT_U32      sip7_cnc_2_3_usedBlocksBmp,
    OUT GT_U32     sip7_cnc_mapToActualBlocksUsed[]
)
{
    GT_U32  ii;/* bit index in sip7_cnc_0_1_usedBlocksBmp , and sip7_cnc_2_3_usedBlocksBmp */
    GT_U32  index = 0;/* index to sip7_cnc_mapToActualBlocksUsed*/

    /* map blocks from CNC0,1 */
    for(ii = 0; ii < 32; ii++)
    {
        if(0 == ((1 << ii) & sip7_cnc_0_1_usedBlocksBmp))
        {
            continue;
        }
        sip7_cnc_mapToActualBlocksUsed[index++] = ii;
    }

    /* map blocks from CNC2,3 */
    for(ii = 0; ii < 32; ii++)
    {
        if(0 == ((1 << ii) & sip7_cnc_2_3_usedBlocksBmp))
        {
            continue;
        }
        sip7_cnc_mapToActualBlocksUsed[index++] = 32 + ii;
    }

    return GT_OK;
}

/**
* @internal applyShmClientLpm function
* @endinternal
*
* @brief   the function need to fill info into : portSelectArr[] for the LPM client
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       lpmNumSbms  - the number of SBMs that the LPM need
*       lpm_numSbmsPerBlock_arraySize - the number of elements in lpm_numSbmsPerBlock[]
* @param[out]
*   portSelectArr[] -   (array of) all the 130/260 SBM of the tile/2 tiles ,
*                       that need to hold this client info (per SBM)
*   lpm_numSbmsPerBlock - (array of) the number of SBMs that are selected to be
*                       used for the LPM , for the LPM banks
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR    - on hardware error.
* @retval GT_BAD_PARAM   - on wrong devNum or other table parameter.
*
*/
static GT_STATUS applyShmClientLpm
(
    IN  GT_U8       devNum,
    IN  GT_U32      lpmNumSbms,
    IN  GT_U32      numPrefixesForLPM,
    OUT PORT_SELECT_INFO_STC portSelectArr[/*MAX_SHM_SBMS*/],
    OUT GT_U32      lpm_numSbmsPerBlock[/*48*/],
    IN  GT_U32      lpm_numSbmsPerBlock_arraySize
)
{
    GT_U32  ii;
    GT_U32  globalSmbId;
    GT_U32  usedSbms;
    const AAS_SBM_CLIENT_INFO_STC*  currUbmPtr;
    const AAS_SBM_CLIENT_INFO_STC*  currSbmPtr;
    AAS_SBM_CLIENT_TYPE_ENT sbmClientType = AAS_SBM_CLIENT_TYPE_LPM_E;
    GT_U32  index_2_lpms;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC * moduleCfgPtr =
        &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg); /* pointer to the module configure of the PP's database*/
                                                    CPSS_TBD_BOOKMARK_AAS
    moduleCfgPtr->ip.lpmSharedMemoryBankNumber = 24;/* 24 like before enlarge to 32K per block
                                            allow cpssInitSystem to PASS. lpmNumSbms;*/
    moduleCfgPtr->ip.lpmBankSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;

    moduleCfgPtr->ip.lastBankIdConfigured = 0xFFF; /* default value mean: parameter was not configured yet */

    numPrefixesForLPM = numPrefixesForLPM;
/*
    ERROR GT_NO_RESOURCE in function: prvCpssDxChLpmRamDbDevListAddSip7, file mainPpDrv/src/
    cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip7LpmRam.c, line[2036].

    * update the <lpmRam> *
    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam    = numPrefixesForLPM;
*/

    usedSbms = 0;

    /* loop on UBMs  */
    for(ii = 0 ; ii < NUM_OF_UBMS ; ii++)
    {
        currUbmPtr = aas_ubm_arr[ii];

        /* loop on potential clients of SBMs in the UBM */
        for(currSbmPtr = &currUbmPtr[0] ;
            currSbmPtr->sbmId != GT_NA ; currSbmPtr++)
        {
            if(currSbmPtr->client != sbmClientType)
            {
                continue;
            }

            globalSmbId = (ii * 8) + currSbmPtr->sbmId;

            if (globalSmbId >= MAX_SHM_SBMS)
            {
                /* should not happen */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            if(portSelectArr[globalSmbId].sbm_port0_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }
            if(portSelectArr[globalSmbId].sbm_port1_select != AAS_UNUSED_HW_VALUE)
            {
                /* already used by other client */
                continue;
            }

            index_2_lpms = currSbmPtr->clientSubId_port0 & 0xFFFFFFFE ;/* ignore the parity bit*/

            if(index_2_lpms >= lpm_numSbmsPerBlock_arraySize)
            {
                /* should not happen , protect access violation */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                    "protect access violation into array lpm_numSbmsPerBlock[0..%d] with index[%d] \n",
                    lpm_numSbmsPerBlock_arraySize-1,
                    index_2_lpms);
            }

            lpm_numSbmsPerBlock[index_2_lpms]++;CPSS_TBD_BOOKMARK_AAS/* how to split SBMs with the other bank */

            portSelectArr[globalSmbId].sbm_port0_select = sbmClientType;
            if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2)
            {
                portSelectArr[globalSmbId].sbm_port1_select = sbmClientType;
            }

            usedSbms++;

            if(usedSbms == lpmNumSbms)
            {
                /* we filled all the needed indexes in portSelectArr[] */
                return GT_OK;
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
        "The LPM client found place for [%d] SBMs out of [%d] needed SBMs",
        usedSbms,lpmNumSbms);

}
#ifdef CPSS_LOG_ENABLE
static const GT_CHAR*   emClientNames[] = {
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_FIRST_E       ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_SECOND_E      ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL0_FIRST_E     ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL0_SECOND_E    ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL1_FIRST_E     ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL1_SECOND_E    ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_MPCL_FIRST_E      ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_MPCL_SECOND_E     ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_EPCL_FIRST_E      ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_EPCL_SECOND_E     ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCE_FIRST_E      ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCE_SECOND_E     ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_ILM2_E        ),
    STR(CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_ILM3_E        )
};
#endif /*CPSS_LOG_ENABLE*/

/**
* @internal enNumEntriesCheck function
* @endinternal
*
* @brief   function to check if the client of the EM unit can hold the needed
*       number of entries .
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       emClientType  - the client of the EM unit
*       numEntriesForClient - the number of entries needed for the client
*
* @param[out]
*   None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_BAD_PARAM   - on table size that is not supported.
*
*/
static GT_STATUS    enNumEntriesCheck
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_ENT  emClientType,
    IN  GT_U32                                  numEntriesForClient
)
{
    GT_U32  maxAllowedValue_1;/* when value is GT_NA , need to use maxAllowedValue_2 */
    GT_U32  maxAllowedValue_2;
    GT_U32  maxAllowedValue;

    maxAllowedValue_2 = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum;

    switch(emClientType)
    {
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_FIRST_E:
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_SECOND_E:
            maxAllowedValue_1 = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_em_clients.ttiNum;
            break;
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL0_FIRST_E:
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL0_SECOND_E:
            maxAllowedValue_1 = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_em_clients.ipcl0Num;
            break;
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL1_FIRST_E:
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL1_SECOND_E:
            maxAllowedValue_1 = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_em_clients.ipcl1Num;
            break;
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_MPCL_FIRST_E:
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_MPCL_SECOND_E:
            maxAllowedValue_1 = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_em_clients.mpclNum;
            break;
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_EPCL_FIRST_E:
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_EPCL_SECOND_E:
            maxAllowedValue_1 = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_em_clients.epclNum;
            break;
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPE_FIRST_E:
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPE_SECOND_E:
            maxAllowedValue_1 = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_em_clients.ipeNum;
            break;
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_ILM2_E:
        case CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_ILM3_E:
            maxAllowedValue_1 = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_em_clients.ttiIlmNum;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(emClientType);
    }

    maxAllowedValue = maxAllowedValue_1;
    if(maxAllowedValue == GT_NA)
    {
        maxAllowedValue = maxAllowedValue_2;
    }

    if(maxAllowedValue < numEntriesForClient)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "The EM client [%s] ask for [%d] entries , but supports only [%d] entries",
            emClientNames[emClientType-1],
            numEntriesForClient , maxAllowedValue);
    }

    return GT_OK;
}

/* info needed for WM */

#define WM__SHARING_PAIRS_MAX_NUM_CNS   32

/* !!! SORTED !!! list of the FDB num of SBMs supported */
static const GT_U32 SORTED__fdbNumSbmsValidOptions[] = {4,8,16,24,32,48,64,GT_NA};

/* !!! SORTED !!! list of the EM num of SBMs supported (per single EM client) */
static const GT_U32 SORTED__emNumSbmsValidOptions[] = {/*0,*/4,8,16,32,GT_NA};

/* !!! SORTED !!! list of the PLR CNT/METER num of SBMs supported */
static const GT_U32 SORTED__plrNumSbmsValidOptions[] = {2,4,6,8,10,12,14,16,32,GT_NA};

/* the CNC block with width of 128 bits hold 4K lines */
/* there are 2 CNC blocks in SBM (_8K lines of 128 bits)*/
#define CNC_NUM_BLOCKS_PER_SBM 2 /* _8K / _4K */

/* the CNC block with width of 128 bits hold 4K lines */
#define CNC_NUM_LINES_PER_BLOCK _4K

/**
* @internal prvCpssDxChSharedMemoryAasConnectClientsToMemory function
* @endinternal
*
* @brief   function to fill into the info of SHM :
*           set shared tables HW with the needed configurations,
*           according to info about the sizes of memories that the clients need.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       sharedTablesSizeInfoPtr  - (pointer to) the shared tables sizes info
*
* @param[out]
*   None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR    - on hardware error.
* @retval GT_BAD_PARAM   - on wrong devNum or other table parameter.
*
*/
GT_STATUS prvCpssDxChSharedMemoryAasConnectClientsToMemory(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_PP_CONFIG_SHARED_TABLES_SIZE_STC  *sharedTablesSizeInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj,index;
    GT_BOOL autoLearnEn;
    GT_U32  numLpmLinesNeeded;
    GT_U32  lpmNumSbms=0,fdbNumSbms=0,arpNumSbms=0,tsNumSbms=0,hf0NumSbms=0,hf1NumSbms=0,plrCountersNumSbms=0,plrMetersNumSbms=0;
    GT_U32  arpNumLines,tsNumLines,hf0NumLines,hf1NumLines;
    GT_U32  ipe0_numOf_cncBlocks,ipe1_numOf_cncBlocks;
    GT_U32  mandatory_numOfCncBlocks_0_1;/*number of cnc blocks that must be on CNC_0_1*/
    GT_U32  more_numOfCncBlocks;/*number of cnc blocks that can be on CNC_0_1/CNC_2_3 , after we set the mandatory on CNC_0_1 */
    GT_U32  mandatory_numOfSbmCncBlocks_0_1;/*number of cnc SBMs that must be on CNC_0_1*/
    GT_U32  more_cncNumSbms;/*number of SBMs needed from CNC_0_1 or CNC_2_3 ,after we set the mandatory on CNC_0_1 */
    GT_U32  emNumSbmsArr[8] = {0,0,0,0,0,0,0,0};
    GT_U32  total_emNumSbms = 0;/* summary of emNumSbmsArr[0..7]*/
    GT_U32  sbmClientType;
    const AAS_CLIENT_INFO_STC *aasShmClientsInfoPtr;
    GT_CHAR emName[6] = "EM[x]";/*x replaced by 0/1/2/3/4/5/6/7 in run time as needed*/
    GT_U32  numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    PORT_SELECT_INFO_STC portSelectArr[MAX_SHM_SBMS];
    CPSS_DXCH_PP_CONFIG_EM_CLIENT_INFO_STC  *currEmClientPtr;
    CPSS_DXCH_PP_CONFIG_SHARED_TABLES_SIZE_STC  *actualSizesPtr;
    WM__SHARING_PAIRS_INFO_STC wm__sharing_pairs[WM__SHARING_PAIRS_MAX_NUM_CNS];
    GT_U32 wm__num_sharing_pairs = 0;/* how many pairs valid in wm__sharing_pairs[] */
    WM__SHARING_PAIRS_INFO_STC  *wm__currPairsInfoPtr;
    GT_BOOL DID_GT_BAD_PARAM_ERROR = GT_FALSE;
    GT_U32  lpm_numSbmsPerBlock_arraySize; /*the number of elements in sip7_lpm_numSbmsPerBlock[]*/

    if(numOfTiles == 2)
    {
        static const AAS_SBM_CLIENT_TYPE_ENT dualTilePairsArr[] = {
            AAS_SBM_CLIENT_TYPE_HF0_E,
            AAS_SBM_CLIENT_TYPE_HF1_E,
            AAS_SBM_CLIENT_TYPE_LPM_E,
            AAS_SBM_CLIENT_TYPE_ARP_E,
            AAS_SBM_CLIENT_TYPE_TS_E ,

            /* also the EM tables shared between tiles , but not all of them */
            /* the EM clients of TTI-ILM2,3 are shared within the tile and not between tiles */
            AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E ,
            AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_HIGH_E,
            AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_LOW_E ,
            AAS_SBM_CLIENT_TYPE_EM1_OR_EM5_HIGH_E,
            AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_LOW_E ,
            AAS_SBM_CLIENT_TYPE_EM2_OR_EM6_HIGH_E,
            AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_LOW_E ,
            AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E,

            AAS_SBM_CLIENT_TYPE__LAST__E
        };

        wm__currPairsInfoPtr = &wm__sharing_pairs[0];
        for(ii = 0 ; dualTilePairsArr[ii] !=  AAS_SBM_CLIENT_TYPE__LAST__E; ii++)
        {
            wm__currPairsInfoPtr->isUsed = GT_TRUE;
            wm__currPairsInfoPtr->orig_tileId    = 0;
            wm__currPairsInfoPtr->sharing_tileId = 1;
            wm__currPairsInfoPtr->orig_client    =
            wm__currPairsInfoPtr->sharing_client = dualTilePairsArr[ii];
        }
        wm__num_sharing_pairs = ii;
    }


    if(numOfTiles != 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,
            "The logic was not implemented for 2 tiles");
    }

    actualSizesPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_sharedMemoryClientsActualSize.actualSizes;

    /* convert 'prefixes' to number of lines in HW */
    numLpmLinesNeeded = PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_LINES_MAC(sharedTablesSizeInfoPtr->numPrefixesForLPM);
    /*
        check for LPM needs
    */
    sbmClientType = AAS_SBM_CLIENT_TYPE_LPM_E;
    aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];
    rc = calcNumSbmsNeeded(numLpmLinesNeeded,
        aasShmClientsInfoPtr,
        "LPM",
        NULL,/* no extra limitations */
        &lpmNumSbms);
    if(rc != GT_OK)
    {
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    actualSizesPtr->numPrefixesForLPM = (GT_U32)(PRV_CPSS_DXCH_SIP6_LPM_LINES_TO_PREFIX_RATIO_MAC *
                                        lpmNumSbms * aasShmClientsInfoPtr->numEntriesPerSbm);

    /*
        check for FDB needs
    */
    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb <
        sharedTablesSizeInfoPtr->numEntriesForFDB)
    {
        CPSS_LOG_ERROR_MAC(
            "The FDB ask for [%d] entries , but supports only [%d] entries",
            sharedTablesSizeInfoPtr->numEntriesForFDB ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    sbmClientType = AAS_SBM_CLIENT_TYPE_FDB_E;
    aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];
    rc = calcNumSbmsNeeded(sharedTablesSizeInfoPtr->numEntriesForFDB,
        aasShmClientsInfoPtr,
        "FDB",
        SORTED__fdbNumSbmsValidOptions,
        &fdbNumSbms);
    if(rc != GT_OK)
    {
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    actualSizesPtr->numEntriesForFDB = fdbNumSbms * aasShmClientsInfoPtr->numEntriesPerSbm;

    /*
        check for EM needs
    */
    if(sharedTablesSizeInfoPtr->numEmUnitsNeeded > 4)
    {
        CPSS_LOG_ERROR_MAC(
            "There are [%d] EM units (so not support request for [%d] EM units)",
            4 , sharedTablesSizeInfoPtr->numEmUnitsNeeded);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;

        /* allow to query the first 4 units */
        sharedTablesSizeInfoPtr->numEmUnitsNeeded = 4;
    }

    for(ii = 0 ; ii < sharedTablesSizeInfoPtr->numEmUnitsNeeded; ii++)
    {
        for(jj = 0 ; jj < 2 ; jj++)
        {
            currEmClientPtr = &sharedTablesSizeInfoPtr->emUnits[ii].clientInfoArr[jj];

            if(sharedTablesSizeInfoPtr->emUnits[ii].singleTableUsedForThe2Clients == GT_TRUE &&
               jj == 1)
            {
                /* we not care about the number of entries that the second client defines , as we use the numbers from the first client */

                /* but the second client must not be 'not connected' */

                if(currEmClientPtr->clientType == CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE__NOT_CONNECTED__E)
                {
                    CPSS_LOG_ERROR_MAC(
                        "in EM units , emUnits[%d].clientInfoArr[%d] client 1 must be connected (since singleTableUsedForThe2Clients == GT_TRUE)",
                        ii,jj);
                    DID_GT_BAD_PARAM_ERROR = GT_TRUE;
                }

                continue;
            }

            if(currEmClientPtr->clientType == CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE__NOT_CONNECTED__E)
            {
                if(jj == 0)
                {
                    CPSS_LOG_ERROR_MAC(
                        "in EM units , emUnits[%d].clientInfoArr[%d] client 0 must be connected (client 1 can be not connected)",
                        ii,jj);
                    DID_GT_BAD_PARAM_ERROR = GT_TRUE;
                }

                /* client 1 allowed to not be connected */
                continue;
            }


            if(0 == currEmClientPtr->numEntriesForClient)
            {
                CPSS_LOG_ERROR_MAC(
                    "in EM units , emUnits[%d].clientInfoArr[%d] with numEntriesForClient = 0 , but not stated as 'no-connected' ",
                    ii,jj);
                DID_GT_BAD_PARAM_ERROR = GT_TRUE;
            }

            /* check that the device allowed to hold the needed table size for this client */
            rc = enNumEntriesCheck(devNum,
                currEmClientPtr->clientType,
                currEmClientPtr->numEntriesForClient);
            if(rc != GT_OK)
            {
                DID_GT_BAD_PARAM_ERROR = GT_TRUE;
            }

            /*
                Handle the client of the EM
            */

            index = 2*ii + jj;/*0/1/2/3/4/5/6/7*/

            emName[3] = (GT_CHAR)('0' + index); /* "EM[x]" --> x replaced by 0/2/4/6 in run time as needed*/
            sbmClientType = AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E + (index*2)%8;
            aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];
            rc = calcNumSbmsNeeded(currEmClientPtr->numEntriesForClient ,
                aasShmClientsInfoPtr,
                emName,
                SORTED__emNumSbmsValidOptions,
                &emNumSbmsArr[index]);
            if(rc != GT_OK)
            {
                DID_GT_BAD_PARAM_ERROR = GT_TRUE;
            }

            actualSizesPtr->emUnits[ii].clientInfoArr[jj].clientType = sharedTablesSizeInfoPtr->emUnits[ii].clientInfoArr[jj].clientType;
            actualSizesPtr->emUnits[ii].clientInfoArr[jj].numEntriesForClient =
                emNumSbmsArr[index] * aasShmClientsInfoPtr->numEntriesPerSbm;

            total_emNumSbms += emNumSbmsArr[index];
        }/*jj*/
        actualSizesPtr->emUnits[ii].singleTableUsedForThe2Clients = sharedTablesSizeInfoPtr->emUnits[ii].singleTableUsedForThe2Clients;
        actualSizesPtr->emUnits[ii].autoLearnMode                 = sharedTablesSizeInfoPtr->emUnits[ii].autoLearnMode;

        if(actualSizesPtr->emUnits[ii].singleTableUsedForThe2Clients == GT_TRUE &&
           actualSizesPtr->emUnits[ii].autoLearnMode != CPSS_DXCH_PP_CONFIG_EM_AUTO_LEARN_MODE__NOT_USED__E)
        {
            CPSS_LOG_ERROR_MAC(
                "The EM unit[%d] cant use autoLearnEn != 'non used' with singleTableUsed == GT_TRUE",
                ii);
            DID_GT_BAD_PARAM_ERROR = GT_TRUE;
        }

    }/*ii*/




    /*
        check for ARP needs
    */
    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp <
        sharedTablesSizeInfoPtr->numEntriesForARP)
    {
        CPSS_LOG_ERROR_MAC(
            "The ARP ask for [%d] entries , but supports only [%d] entries",
            sharedTablesSizeInfoPtr->numEntriesForARP ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    sbmClientType = AAS_SBM_CLIENT_TYPE_ARP_E;
    aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];
    rc = calcNumSbmsNeeded(sharedTablesSizeInfoPtr->numEntriesForARP,
        aasShmClientsInfoPtr,
        "ARP",
        NULL,/* no extra limitations (the steps of 3 comes from : numOfConsecutiveSbmsNeeded)*/
        &arpNumSbms);
    if(rc != GT_OK)
    {
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }
    /* 8 ARPs in entry (th e8 is already in aasShmClientsInfoPtr->numEntriesPerSbm)*/
    actualSizesPtr->numEntriesForARP = arpNumSbms * aasShmClientsInfoPtr->numEntriesPerSbm / aasShmClientsInfoPtr->numOfConsecutiveSbmsNeeded;

    /*
        check for TS needs
    */
    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart <
        sharedTablesSizeInfoPtr->numEntriesForTS)
    {
        CPSS_LOG_ERROR_MAC(
            "The TS ask for [%d] entries , but supports only [%d] entries",
            sharedTablesSizeInfoPtr->numEntriesForTS ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    sbmClientType = AAS_SBM_CLIENT_TYPE_TS_E;
    aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];
    rc = calcNumSbmsNeeded(sharedTablesSizeInfoPtr->numEntriesForTS,
        aasShmClientsInfoPtr,
        "TunnelStart",
        NULL,/* no extra limitations (the steps of 3 comes from : numOfConsecutiveSbmsNeeded)*/
        &tsNumSbms);
    if(rc != GT_OK)
    {
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    actualSizesPtr->numEntriesForTS = tsNumSbms * aasShmClientsInfoPtr->numEntriesPerSbm / aasShmClientsInfoPtr->numOfConsecutiveSbmsNeeded;

    /*
        check for HF0 needs
    */
    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_hf0 <
        sharedTablesSizeInfoPtr->numEntriesForHF0)
    {
        CPSS_LOG_ERROR_MAC(
            "The HF0 ask for [%d] entries , but supports only [%d] entries",
            sharedTablesSizeInfoPtr->numEntriesForHF0 ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_hf0);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    sbmClientType = AAS_SBM_CLIENT_TYPE_HF0_E;
    aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];
    rc = calcNumSbmsNeeded(sharedTablesSizeInfoPtr->numEntriesForHF0,
        aasShmClientsInfoPtr,
        "HF0",
        NULL,/* no extra limitations (the steps of 3 comes from : numOfConsecutiveSbmsNeeded)*/
        &hf0NumSbms);
    if(rc != GT_OK)
    {
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    actualSizesPtr->numEntriesForHF0 = hf0NumSbms * aasShmClientsInfoPtr->numEntriesPerSbm / aasShmClientsInfoPtr->numOfConsecutiveSbmsNeeded;
    /*
        check for HF1 needs
    */
    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_hf1 <
        sharedTablesSizeInfoPtr->numEntriesForHF1)
    {
        CPSS_LOG_ERROR_MAC(
            "The HF1 ask for [%d] entries , but supports only [%d] entries",
            sharedTablesSizeInfoPtr->numEntriesForHF1 ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_hf1);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    sbmClientType = AAS_SBM_CLIENT_TYPE_HF1_E;
    aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];
    rc = calcNumSbmsNeeded(sharedTablesSizeInfoPtr->numEntriesForHF1,
        aasShmClientsInfoPtr,
        "HF1",
        NULL,/* no extra limitations (the steps of 3 comes from : numOfConsecutiveSbmsNeeded)*/
        &hf1NumSbms);
    if(rc != GT_OK)
    {
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    actualSizesPtr->numEntriesForHF1 = hf1NumSbms * aasShmClientsInfoPtr->numEntriesPerSbm / aasShmClientsInfoPtr->numOfConsecutiveSbmsNeeded;

    /*
        check for PLR ipfix or billing needs
    */
    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_plr_counters <
        sharedTablesSizeInfoPtr->numEntriesForIpfixOrBilling)
    {
        CPSS_LOG_ERROR_MAC(
            "The PLR:IpfixOrBilling ask for [%d] entries , but supports only [%d] entries",
            sharedTablesSizeInfoPtr->numEntriesForIpfixOrBilling ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_plr_counters);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    sbmClientType = AAS_SBM_CLIENT_TYPE_PLR_CNT_E;
    aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];
    rc = calcNumSbmsNeeded(sharedTablesSizeInfoPtr->numEntriesForIpfixOrBilling,
        aasShmClientsInfoPtr,
        "PLR:IpfixOrBilling",
        SORTED__plrNumSbmsValidOptions,
        &plrCountersNumSbms);
    if(rc != GT_OK)
    {
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    actualSizesPtr->numEntriesForIpfixOrBilling = plrCountersNumSbms * aasShmClientsInfoPtr->numEntriesPerSbm / aasShmClientsInfoPtr->numOfConsecutiveSbmsNeeded;
    /*
        check for PLR meter needs
    */
    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum <
        sharedTablesSizeInfoPtr->numEntriesForIpfixOrBilling)
    {
        CPSS_LOG_ERROR_MAC(
            "The PLR:Metering ask for [%d] entries , but supports only [%d] entries",
            sharedTablesSizeInfoPtr->numEntriesForIpfixOrBilling ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    sbmClientType = AAS_SBM_CLIENT_TYPE_PLR_MET_E;
    aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];
    rc = calcNumSbmsNeeded(sharedTablesSizeInfoPtr->numEntriesForMetering,
        aasShmClientsInfoPtr,
        "PLR:Metering",
        SORTED__plrNumSbmsValidOptions,
        &plrMetersNumSbms);
    if(rc != GT_OK)
    {
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    actualSizesPtr->numEntriesForMetering = plrMetersNumSbms * aasShmClientsInfoPtr->numEntriesPerSbm / aasShmClientsInfoPtr->numOfConsecutiveSbmsNeeded;

    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks <
        sharedTablesSizeInfoPtr->numOfCncBlocks)
    {
        CPSS_LOG_ERROR_MAC(
            "The CNC ask for [%d] blocks , but supports only [%d] blocks",
            sharedTablesSizeInfoPtr->numOfCncBlocks ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_ipe0 <
        sharedTablesSizeInfoPtr->numEntriesForIpeState0)
    {
        CPSS_LOG_ERROR_MAC(
            "The IPE0 ask for [%d] entries , but supports only [%d] entries",
            sharedTablesSizeInfoPtr->numEntriesForIpeState0 ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_ipe0);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    /* check that the device allowed to hold the needed table size for this client */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_ipe1 <
        sharedTablesSizeInfoPtr->numEntriesForIpeState1)
    {
        CPSS_LOG_ERROR_MAC(
            "The IPE1 ask for [%d] entries , but supports only [%d] entries",
            sharedTablesSizeInfoPtr->numEntriesForIpeState1 ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.sip7_ipe1);
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    /*
        check for IPE0,1 needs.
        those are NOT 'direct' clients of the SHM , but are muxed with the CNC_0_1 max of 16 SBMs (==32 blocks)
    */
    sbmClientType = AAS_SBM_CLIENT_TYPE_CNC_0_1_E;
    aasShmClientsInfoPtr = &aasShmClientsInfoArr[sbmClientType];

    /* calc the number of CNC blocks that needed to cover the IPE0 */
    ipe0_numOf_cncBlocks = ROUND_UP_DIVIDE_MAC(sharedTablesSizeInfoPtr->numEntriesForIpeState0 ,
        CNC_NUM_LINES_PER_BLOCK);

    /* calc the number of CNC blocks that needed to cover the IPE1 */
    ipe1_numOf_cncBlocks = ROUND_UP_DIVIDE_MAC(sharedTablesSizeInfoPtr->numEntriesForIpeState1 ,
        CNC_NUM_LINES_PER_BLOCK);

    mandatory_numOfCncBlocks_0_1 = aasShmClientsInfoPtr->minNumSbms/*6*/ * CNC_NUM_BLOCKS_PER_SBM/*2*/;
    /* we need to satisfy the IPE0,1 and the CNC needs */
    /* but CNC_0_1 use minimal of 6 SBMs               */
    /* and we not want to 'waist' the minimal SBMs     */
    /* so we check if the number of CNC block that will cover IPE0,1 is under 12 */
    if((ipe0_numOf_cncBlocks + ipe1_numOf_cncBlocks) < mandatory_numOfCncBlocks_0_1)
    {
        GT_U32  cnc_0_1_mandatory_cnc_blocks_leftover =
            mandatory_numOfCncBlocks_0_1 - (ipe0_numOf_cncBlocks + ipe1_numOf_cncBlocks);

        if(sharedTablesSizeInfoPtr->numOfCncBlocks > cnc_0_1_mandatory_cnc_blocks_leftover)
        {
            more_numOfCncBlocks = sharedTablesSizeInfoPtr->numOfCncBlocks - cnc_0_1_mandatory_cnc_blocks_leftover;
        }
        else
        {
            more_numOfCncBlocks = 0;
        }
    }
    else
    {
        /* IPE0,1 are muxed only with CNC_0_1*/
        mandatory_numOfCncBlocks_0_1 = ipe0_numOf_cncBlocks + ipe1_numOf_cncBlocks;
        if(sharedTablesSizeInfoPtr->numOfCncBlocks > mandatory_numOfCncBlocks_0_1)
        {
            more_numOfCncBlocks = sharedTablesSizeInfoPtr->numOfCncBlocks - mandatory_numOfCncBlocks_0_1;
        }
        else
        {
            more_numOfCncBlocks = 0;
        }
    }

    if((ipe0_numOf_cncBlocks + ipe1_numOf_cncBlocks + sharedTablesSizeInfoPtr->numOfCncBlocks) > 64)
    {
        CPSS_LOG_ERROR_MAC(
            "(ipe0 require [%d] cnc blocks + ipe1 require [%d] cnc blocks + cnc require [%d] cnc blocks) = total [%d] cnc blocks that is more than 'max' 64",
            ipe0_numOf_cncBlocks , ipe1_numOf_cncBlocks , sharedTablesSizeInfoPtr->numOfCncBlocks ,
            ipe0_numOf_cncBlocks + ipe1_numOf_cncBlocks + sharedTablesSizeInfoPtr->numOfCncBlocks
            );
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    if((mandatory_numOfCncBlocks_0_1 + more_numOfCncBlocks) > 64)
    {
        /* 'BAD_STATE' --> something wrong with the calculations */
        CPSS_LOG_ERROR_MAC(
            "(mandatory_numOfCncBlocks_0_1 require [%d] cnc blocks + more_numOfCncBlocks require cnc blocks) = total [%d] cnc blocks that is more than 'max' 64",
            mandatory_numOfCncBlocks_0_1 , more_numOfCncBlocks ,
            mandatory_numOfCncBlocks_0_1 + more_numOfCncBlocks
            );
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }


    if((mandatory_numOfCncBlocks_0_1 % CNC_NUM_BLOCKS_PER_SBM) && more_numOfCncBlocks)
    {
        /* we can force one of the 'more' into the 'mandatory' of CNC_0_1 */
        more_numOfCncBlocks--;
        mandatory_numOfCncBlocks_0_1++;
    }

    mandatory_numOfSbmCncBlocks_0_1 = ROUND_UP_DIVIDE_MAC(mandatory_numOfCncBlocks_0_1,CNC_NUM_BLOCKS_PER_SBM);

    if(more_numOfCncBlocks % CNC_NUM_BLOCKS_PER_SBM)
    {
        /* we can align the CNC to even number (as SBM uses 2 blocks)*/
        more_numOfCncBlocks++;
    }
    more_cncNumSbms = ROUND_UP_DIVIDE_MAC(more_numOfCncBlocks,CNC_NUM_BLOCKS_PER_SBM);

    actualSizesPtr->numOfCncBlocks = (mandatory_numOfCncBlocks_0_1 + more_numOfCncBlocks) - (ipe0_numOf_cncBlocks + ipe1_numOf_cncBlocks);
    actualSizesPtr->numEntriesForIpeState0 = CNC_NUM_LINES_PER_BLOCK * ipe0_numOf_cncBlocks;
    actualSizesPtr->numEntriesForIpeState1 = CNC_NUM_LINES_PER_BLOCK * ipe1_numOf_cncBlocks;

    /*
        check now that the total number of SBMs can fit into the device as total
        before checking other limitations
    */

    if((lpmNumSbms + fdbNumSbms + total_emNumSbms + arpNumSbms + tsNumSbms + hf0NumSbms + hf1NumSbms + plrCountersNumSbms + plrMetersNumSbms +
        mandatory_numOfSbmCncBlocks_0_1 + more_cncNumSbms) >
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm)
    {
        CPSS_LOG_ERROR_MAC(
            "(lpmNumSbms[%d] + fdbNumSbms[%d] + total_emNumSbms[%d] + arpNumSbms[%d] + tsNumSbms[%d] + hf0NumSbms[%d] + hf1NumSbms[%d] + plrCountersNumSbms[%d] + plrMetersNumSbms[%d] + \n"
            "mandatory_numOfSbmCncBlocks_0_1[%d] + more_cncNumSbms[%d]) = total [%d] SMBs that is more than 'max' [%d]",
            lpmNumSbms , fdbNumSbms , total_emNumSbms , arpNumSbms , tsNumSbms , hf0NumSbms , hf1NumSbms , plrCountersNumSbms , plrMetersNumSbms ,
                mandatory_numOfSbmCncBlocks_0_1 , more_cncNumSbms ,

            lpmNumSbms + fdbNumSbms + total_emNumSbms + arpNumSbms + tsNumSbms + hf0NumSbms + hf1NumSbms + plrCountersNumSbms + plrMetersNumSbms +
                mandatory_numOfSbmCncBlocks_0_1 + more_cncNumSbms,

            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm
            );
        DID_GT_BAD_PARAM_ERROR = GT_TRUE;
    }

    if(DID_GT_BAD_PARAM_ERROR == GT_TRUE)
    {
        /* we finished to print all the 'bad' params that the info hold ,
           so with single iteration we can get all of them and no need to call
           the function many times and every time to fix single param and still
           fail on another*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    for(ii = 0 ; ii < MAX_SHM_SBMS ; ii++)
    {
        /* set 'unused' value to so after first client set value the next clients will know it is used. */
        portSelectArr[ii].sbm_port0_select = AAS_UNUSED_HW_VALUE;
        portSelectArr[ii].sbm_port1_select = AAS_UNUSED_HW_VALUE;
        portSelectArr[ii].sbm_port_mode    = 0;
        portSelectArr[ii].lpmSubInfo.sbm_port0_lpm_address_segment    = 0;
        portSelectArr[ii].lpmSubInfo.sbm_port1_lpm_address_segment    = 0;
        portSelectArr[ii].lpmSubInfo.sbm_port0_lpm_taken_from_port1   = 0;
        portSelectArr[ii].lpmSubInfo.sbm_port1_lpm_taken_from_port0   = 0;
    }

    /* apply clients according to their needed order :
        FDB,
        EM,
        TS and ARP,
        HF,
        PLRs,
        CNC,
        LPM
    */

    /* apply the FDB client */
    rc = applyShmClientFdb(devNum,fdbNumSbms,&portSelectArr[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0 ; ii < sharedTablesSizeInfoPtr->numEmUnitsNeeded ; ii++)
    {
        for(jj = 0 ; jj < 2 ; jj++)
        {
            index = 2*ii + jj;/*0/1/2/3/4/5/6/7*/

            autoLearnEn =
                (jj == 0 && sharedTablesSizeInfoPtr->emUnits[ii].autoLearnMode ==
                    CPSS_DXCH_PP_CONFIG_EM_AUTO_LEARN_MODE_CLIENT0_E) ? GT_TRUE :
                (jj == 1 && sharedTablesSizeInfoPtr->emUnits[ii].autoLearnMode ==
                    CPSS_DXCH_PP_CONFIG_EM_AUTO_LEARN_MODE_CLIENT1_E) ? GT_TRUE :
                    GT_FALSE;

            /* apply the EM client tables */
            rc = applyShmClientEM(devNum,index,emNumSbmsArr[index],&portSelectArr[0],
                sharedTablesSizeInfoPtr->emUnits[ii].singleTableUsedForThe2Clients,
                autoLearnEn,
                &wm__sharing_pairs[0],
                &wm__num_sharing_pairs);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* apply the ARP client */
    rc = applyShmClientArpTsHf(devNum,"ARP",AAS_SBM_CLIENT_TYPE_ARP_E,arpNumSbms,&portSelectArr[0],&arpNumLines);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* apply the TS client */
    rc = applyShmClientArpTsHf(devNum,"TS",AAS_SBM_CLIENT_TYPE_TS_E,tsNumSbms,&portSelectArr[0],&tsNumLines);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* apply the HF0 client */
    rc = applyShmClientArpTsHf(devNum,"HF0",AAS_SBM_CLIENT_TYPE_HF0_E,hf0NumSbms,&portSelectArr[0],&hf0NumLines);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* apply the HF1 client */
    rc = applyShmClientArpTsHf(devNum,"HF1",AAS_SBM_CLIENT_TYPE_HF1_E,hf1NumSbms,&portSelectArr[0],&hf1NumLines);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* apply the PLR counters client */
    rc = applyShmClientPlr("plr-counters",AAS_SBM_CLIENT_TYPE_PLR_CNT_E,plrCountersNumSbms,&portSelectArr[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* apply the PLR meters client */
    rc = applyShmClientPlr("plr-meter",AAS_SBM_CLIENT_TYPE_PLR_MET_E,plrMetersNumSbms,&portSelectArr[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* apply the mandatory CNC_0_1 client */
    rc = applyShmClientCnc(devNum,"mandatory CNC_0_1",AAS_SBM_CLIENT_TYPE_CNC_0_1_E,
        mandatory_numOfSbmCncBlocks_0_1,&portSelectArr[0],
        &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp,
        &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_2_3_usedBlocksBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* apply the others CNC_0_1,CNC_2_3 client :

       call the function with the CNC_2_3 and it will call internally the function
       to CNC_0_1 if cant get enough from CNC_2_3
    */
    rc = applyShmClientCnc(devNum,"others from CNC_0_1 or CNC_2_3",AAS_SBM_CLIENT_TYPE_CNC_2_3_E,
        more_cncNumSbms,&portSelectArr[0],
        &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp,
        &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_2_3_usedBlocksBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* calc
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe0_usedCncBlocksBmp,
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe1_usedCncBlocksBmp,

       and update :
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp
        (remove from it the CNCs that used by IPE0,1)
    */
    rc = calcIpeUsedCncBlocksBmp(ipe0_numOf_cncBlocks,ipe1_numOf_cncBlocks,
        &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp,
        &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe0_usedCncBlocksBmp,
        &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe1_usedCncBlocksBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* calculate PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).sip7_cnc_mapToActualBlocksUsed[...] based on
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp,
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_2_3_usedBlocksBmp
    */
    rc = calcCncBlocksMapping(
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp,
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_2_3_usedBlocksBmp,
        PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).sip7_cnc_mapToActualBlocksUsed);
    if(rc != GT_OK)
    {
        return rc;
    }

    lpm_numSbmsPerBlock_arraySize = NUM_ELEMENTS_IN_ARR_MAC(
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip7_lpm_numSbmsPerBlock);

    /* apply the LPM client (the last client) */
    rc = applyShmClientLpm(devNum,lpmNumSbms,actualSizesPtr->numPrefixesForLPM,
        &portSelectArr[0],
        &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip7_lpm_numSbmsPerBlock[0],
        lpm_numSbmsPerBlock_arraySize);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* split the LPM banks between the pairs */
    for(ii = 0 ; ii < lpm_numSbmsPerBlock_arraySize ; ii+=2)
    {
        /* give half to the upper part of the pair */
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip7_lpm_numSbmsPerBlock[ii+1] =
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip7_lpm_numSbmsPerBlock[ii] / 2;

        /* keep the rest for the lower part of the pair */
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip7_lpm_numSbmsPerBlock[ii] -=
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip7_lpm_numSbmsPerBlock[ii+1];
    }




    /*  finished filling the info into portSelectArr[0] , so we can write it into :
        SHM_IP {Current}/SHM/SHM/Block <%n> Port Select registers
    */
    rc = shmBlockPortSelectWriteToHw(devNum,&portSelectArr[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*  write to HW the CNC/IPE0,1 mux , according to :
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe0_usedCncBlocksBmp,
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe1_usedCncBlocksBmp,
    */
    rc = shmCncIpeMuxWriteToHw(devNum,
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe0_usedCncBlocksBmp,
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe1_usedCncBlocksBmp);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* write to EMX unit the EM clients*/
    rc = emxWriteEmClientsToHw(devNum,
        sharedTablesSizeInfoPtr->numEmUnitsNeeded,
        &sharedTablesSizeInfoPtr->emUnits[0]);
    if(rc != GT_OK)
    {
        return rc;
    }



    /* we need to tell the WM that we are done allocating SBMs for the clients ,
        so the WM can allocate the memories to those clients , as it not hold the
        SHM connectivity logic

       NOTE: this must be done only after we finished to write the SHM registers
        as WM may need to read some of them to get 'full picture'
    */
    if(CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
    {
        GT_U32 numCnc_0_1_used_blocks;/* for CNC only (without IPE) */
        GT_U32 numCnc_2_3_used_blocks;

        cpssOsPrintf("sip7_cnc_0_1_usedBlocksBmp = [0x%8.8x]\n",PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp);
        cpssOsPrintf("sip7_cnc_2_3_usedBlocksBmp = [0x%8.8x]\n",PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_2_3_usedBlocksBmp);
        cpssOsPrintf("sip7_ipe0_usedCncBlocksBmp = [0x%8.8x]\n",PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe0_usedCncBlocksBmp);
        cpssOsPrintf("sip7_ipe1_usedCncBlocksBmp = [0x%8.8x]\n",PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip7_ipe.sip7_ipe1_usedCncBlocksBmp);

        numCnc_0_1_used_blocks = prvCpssPpConfigBitmapNumBitsGet(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp);
        numCnc_2_3_used_blocks = prvCpssPpConfigBitmapNumBitsGet(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_2_3_usedBlocksBmp);

        /* allow WM/GM to optimize the memory allocations */
        /* the GM not support it (yet)                    */
        wmOnly_sharedTablesSizeSet(devNum,GT_NA/*start session*/             ,0/*dont care*/,0);

        /* CNC_0_1 : give blocks BMP */
        wmOnly_sharedTablesSizeSet(devNum,WM__SPECIAL_MESSAGE_TYPE_CNC_0_1_BLOCKS_BMP_E,
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_0_1_usedBlocksBmp,0/*not care*/);
        /* CNC_2_3 : give blocks BMP */
        wmOnly_sharedTablesSizeSet(devNum,WM__SPECIAL_MESSAGE_TYPE_CNC_2_3_BLOCKS_BMP_E,
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cnc.sip7_cnc_2_3_usedBlocksBmp,0/*not care*/);

        /* LPM_0_31  : give blocks BMP */
        for(ii = 0 ; ii < lpm_numSbmsPerBlock_arraySize ; ii++)
        {
            wmOnly_sharedTablesSizeSet(devNum,WM__SPECIAL_MESSAGE_TYPE_LPM_BLOCKS_BMP_E,
                ii,/* block Id*/
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip7_lpm_numSbmsPerBlock[ii]);/* num SBMs*/
        }

        /*WM__SPECIAL_MESSAGE_TYPE_SHARING_PAIRS_INFO_E*/
        for(ii = 0 ; ii < wm__num_sharing_pairs ; ii++)
        {
            if(wm__sharing_pairs[ii].isUsed == GT_FALSE)
            {
                continue;
            }

            wmOnly_sharedTablesPairsInfo(devNum,
                wm__sharing_pairs[ii].orig_tileId,
                wm__sharing_pairs[ii].orig_client,
                wm__sharing_pairs[ii].sharing_tileId,
                wm__sharing_pairs[ii].sharing_client);
        }

        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_FDB_E             ,actualSizesPtr->numEntriesForFDB  ,fdbNumSbms);
        for(ii = 0 ; ii < sharedTablesSizeInfoPtr->numEmUnitsNeeded ; ii++)
        {
            for(jj = 0 ; jj < 2 ; jj++)
            {
                index = 2*ii + jj;/*0/1/2/3/4/5/6/7*/

                wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E +  index ,
                    actualSizesPtr->emUnits[ii].clientInfoArr[jj].numEntriesForClient   ,
                    emNumSbmsArr[index]);
            }
        }
        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_LPM_E     ,actualSizesPtr->numPrefixesForLPM  ,lpmNumSbms);
        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_ARP_E     ,arpNumLines , arpNumSbms);
        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_TS_E      ,tsNumLines  , tsNumSbms);
        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_PLR_MET_E ,actualSizesPtr->numEntriesForMetering,plrMetersNumSbms);
        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_PLR_CNT_E ,actualSizesPtr->numEntriesForIpfixOrBilling,plrCountersNumSbms);
        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_HF0_E     ,hf0NumLines  ,hf0NumSbms);
        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_HF1_E     ,hf1NumLines  ,hf1NumSbms);

        /* CNC_0_1 : give number of blocks (not number of entries)*/
        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_CNC_0_1_E ,numCnc_0_1_used_blocks,(numCnc_0_1_used_blocks+1)/2);
        /* CNC_2_3 : give number of blocks (not number of entries)*/
        wmOnly_sharedTablesSizeSet(devNum,AAS_SBM_CLIENT_TYPE_CNC_2_3_E ,numCnc_2_3_used_blocks,(numCnc_2_3_used_blocks+1)/2);

        wmOnly_sharedTablesSizeSet(devNum,GT_NA-1/*end session*/             ,0/*dont care*/,0);
    }


    return GT_OK;
}

/* for WM purpose only : see smemAasActiveWriteShmSizeForWm */
#define SHM_ADDR_REG_FOR_WM_TABLE_SIZE 0x1000
enum{/* this enum will also be in WM code */
    WM_TABLE_STATRTED_SESSION_E = 1,
    WM_TABLE_ENDED_SESSION_E,
    WM_TABLE_FDB_E  ,
    WM_TABLE_LPM_E  ,
    WM_TABLE_TS_E   ,
    /* new in AAS */
    WM_TABLE_ARP_E  ,
    WM_TABLE_CNC_0_1_E  ,
    WM_TABLE_CNC_2_3_E  ,
    WM_TABLE_EM_0_E  ,
    WM_TABLE_EM_1_E  ,
    WM_TABLE_EM_2_E  ,
    WM_TABLE_EM_3_E  ,
    WM_TABLE_EM_4_E  ,
    WM_TABLE_EM_5_E  ,
    WM_TABLE_EM_6_E  ,
    WM_TABLE_EM_7_E  ,
    WM_TABLE_HF0_E  ,
    WM_TABLE_HF1_E  ,
    WM_TABLE_PLR_METERS_E    ,
    WM_TABLE_PLR_COUNTERS_E  ,
};

/**
* @internal wmOnly_sharedTablesConvetClientToWmTable function
* @endinternal
*
* @brief   function for WM/GM - convert 'cpss client' to 'WM table type' .
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       client  - the client
*
* @param[out]
*       wmTableTypePtr - (pointer to) the WM table type
*
* @retval GT_OK   - always
*
*/
static  GT_STATUS wmOnly_sharedTablesConvetClientToWmTable
(
    IN AAS_SBM_CLIENT_TYPE_ENT client,
    OUT GT_U32  *wmTableTypePtr
)
{
    GT_U32 wmTableType =
            client == GT_NA                                  ? WM_TABLE_STATRTED_SESSION_E :
            client == GT_NA-1                                ? WM_TABLE_ENDED_SESSION_E    :

            client == AAS_SBM_CLIENT_TYPE_FDB_E              ? WM_TABLE_FDB_E :
            client >= AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E &&
            client <= AAS_SBM_CLIENT_TYPE_EM3_OR_EM7_HIGH_E  ? WM_TABLE_EM_0_E + (client-AAS_SBM_CLIENT_TYPE_EM0_OR_EM4_LOW_E) :
            client == AAS_SBM_CLIENT_TYPE_LPM_E              ? WM_TABLE_LPM_E :
            client == AAS_SBM_CLIENT_TYPE_TS_E               ? WM_TABLE_TS_E :
            client == AAS_SBM_CLIENT_TYPE_ARP_E              ? WM_TABLE_ARP_E :
            client == AAS_SBM_CLIENT_TYPE_HF0_E              ? WM_TABLE_HF0_E :
            client == AAS_SBM_CLIENT_TYPE_HF1_E              ? WM_TABLE_HF1_E :
            client == AAS_SBM_CLIENT_TYPE_PLR_CNT_E          ? WM_TABLE_PLR_COUNTERS_E :
            client == AAS_SBM_CLIENT_TYPE_PLR_MET_E          ? WM_TABLE_PLR_METERS_E :
            client == AAS_SBM_CLIENT_TYPE_CNC_0_1_E          ? WM_TABLE_CNC_0_1_E :
            client == AAS_SBM_CLIENT_TYPE_CNC_2_3_E          ? WM_TABLE_CNC_2_3_E :

            0;/* unknown */
    *wmTableTypePtr = wmTableType;

    return GT_OK;
}

/**
* @internal wmOnly_sharedTablesPairsInfo function
* @endinternal
*
* @brief   function for WM/GM - to write to the device info about the pairs of clients
*           that share the same memory.
*           the WM have special active memories that collect this info and use it
*           instead of the complex logic that HW for SHM is doing
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum          - the device number
*       orig_tileId     - the tile id   of the first client
*       orig_client     - the client id of the first client
*       sharing_tileId  - the tile id   of the second client
*       sharing_client  - the client id of the second client
*
* @param[out]
*       None
*
* @retval GT_OK   - always
*
*/
static GT_STATUS  wmOnly_sharedTablesPairsInfo(
    IN  GT_U8                   devNum,
    IN  GT_U32                  orig_tileId,
    IN  AAS_SBM_CLIENT_TYPE_ENT orig_client,
    IN  GT_U32                  sharing_tileId,
    IN  AAS_SBM_CLIENT_TYPE_ENT sharing_client
)
{
    GT_U32  regAddrBase,regAddr,value;
    GT_U32  offset = 0xc;
    GT_U32  orig_wmTableType,sharing_wmTableType;

    wmOnly_sharedTablesConvetClientToWmTable(orig_client   ,&orig_wmTableType);
    wmOnly_sharedTablesConvetClientToWmTable(sharing_client,&sharing_wmTableType);

    regAddrBase = SHM_ADDR_REG_FOR_WM_TABLE_SIZE +
        (PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.fdb_illegal_address & ~ 0xFFFF);

    regAddr = regAddrBase + offset;

    value = 0;

    U32_SET_FIELD(value,0,1,sharing_tileId);
    U32_SET_FIELD(value,1,8,sharing_wmTableType);
    U32_SET_FIELD(value,9,1,1);/*isSharedWithOrig*/
    U32_SET_FIELD(value,10,1,orig_tileId);
    U32_SET_FIELD(value,11,8,orig_wmTableType);

    return prvCpssHwPpWriteRegister(devNum,regAddr,value);
}


/**
* @internal wmOnly_sharedTablesSizeSet function
* @endinternal
*
* @brief   function for WM/GM - to write to the device info about the table size and num SBMs.
*           the WM have special active memories that collect this info and use it
*           instead of the complex logic that HW for SHM is doing
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum      - the device number
*       client      - the client id
*       tableSize   - the number of entries for the client
*                   if client == WM__SPECIAL_MESSAGE_TYPE_CNC_0_1_BLOCKS_BMP_E
*                       then used as 'bmp of CNC blocks'
*                   if client == WM__SPECIAL_MESSAGE_TYPE_CNC_2_3_BLOCKS_BMP_E
*                       then used as 'bmp of CNC blocks'
*                   if client == WM__SPECIAL_MESSAGE_TYPE_LPM_BLOCKS_BMP_E
*                       then used as 'block index'
*                   for all others used as 'number of entries for the client'
*       numOfSbms   - the number of SMBs for the client
*                   if client == WM__SPECIAL_MESSAGE_TYPE_CNC_0_1_BLOCKS_BMP_E
*                       then ignored
*                   if client == WM__SPECIAL_MESSAGE_TYPE_CNC_2_3_BLOCKS_BMP_E
*                       then ignored
*                   for all others used as 'number of SMBs for the client'
*
* @param[out]
*       None
*
* @retval GT_OK   - always
*
*/
static GT_STATUS  wmOnly_sharedTablesSizeSet(
    IN  GT_U8                   devNum,
    IN  AAS_SBM_CLIENT_TYPE_ENT client,
    IN  GT_U32                  tableSize,
    IN  GT_U32                  numOfSbms
)
{
    GT_U32  wmTableType;
    GT_U32  wmSize;
    GT_U32  regAddrBase,regAddr,value;

    regAddrBase = SHM_ADDR_REG_FOR_WM_TABLE_SIZE +
        (PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.fdb_illegal_address & ~ 0xFFFF);

    if(client >= WM__SPECIAL_MESSAGE_TYPE__FIRST__E)
    {
        GT_U32  offset = 0;

        switch(client)
        {
            case WM__SPECIAL_MESSAGE_TYPE_CNC_0_1_BLOCKS_BMP_E:   offset = 0x4  ; break;
            case WM__SPECIAL_MESSAGE_TYPE_CNC_2_3_BLOCKS_BMP_E:   offset = 0x8  ; break;
            case WM__SPECIAL_MESSAGE_TYPE_LPM_BLOCKS_BMP_E:       offset = 0x10 ; break;
            default :
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"unknown table type [%d]",client);
        }

        regAddr = regAddrBase + offset;

        if(client == WM__SPECIAL_MESSAGE_TYPE_LPM_BLOCKS_BMP_E)
        {
            GT_U32  numLines = numOfSbms /*num of sbms for this LPM block*/ * SBM_NUM_LINES / _1K;
            value = tableSize/*block index*/ << 16 | numLines /*num of entries in LPM block (in 1K values) */;
        }
        else
        {
            value = tableSize;/* data */
        }

        return prvCpssHwPpWriteRegister(devNum,regAddr,value);
    }

    wmOnly_sharedTablesConvetClientToWmTable(client,&wmTableType);

    if(wmTableType == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"unknown table type [%d]",client);
    }

    if(client == AAS_SBM_CLIENT_TYPE_CNC_0_1_E ||
       client == AAS_SBM_CLIENT_TYPE_CNC_2_3_E )
    {
        wmSize = tableSize;/* num of CNC blocks */
    }
    else
    {
        wmSize = tableSize / _1K;
    }

    value = wmTableType     | /* 7 bits  : up to 128 different table types  */
            wmSize    <<  7 | /* 18 bits : for table size (times 1K)        */
            numOfSbms << 25;  /* 7 bits  : for up to 128 SBMs               */

    regAddr = regAddrBase + 0;

    return prvCpssHwPpWriteRegister(devNum,regAddr,value);
}


/**
* @internal shmBlockPortSelectWriteToHw function
* @endinternal
*
* @brief    the function will write portSelectArr[] into :
*       SHM_IP {Current}/SHM/SHM/Block <%n> Port Select registers
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       portSelectArr[] - (array of) all the 130/260 SBM of the tile/2 tiles ,
*                       that need to hold this client info (per SBM)
*
* @param[out]
*       None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_HW_ERROR    - on hardware error.
*
*/
static GT_STATUS shmBlockPortSelectWriteToHw
(
    IN  GT_U8       devNum,
    OUT PORT_SELECT_INFO_STC portSelectArr[/*MAX_SHM_SBMS*/]
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numSbms = MAX_SHM_SBMS_SINGLE_TILE * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    PORT_SELECT_INFO_STC    *currInfoPtr;
    GT_U32  regValue;

    currInfoPtr = &portSelectArr[0];

    for (ii = 0 ; ii < numSbms; ii++ , currInfoPtr++)
    {
        regValue = 0;

        U32_SET_FIELD_MAC(regValue, 0,5,currInfoPtr->sbm_port0_select);
        U32_SET_FIELD_MAC(regValue, 8,5,currInfoPtr->sbm_port1_select);
        U32_SET_FIELD_MAC(regValue,16,1,BOOL2BIT_MAC(currInfoPtr->sbm_port_mode));
        U32_SET_FIELD_MAC(regValue,20,3,currInfoPtr->lpmSubInfo.sbm_port0_lpm_address_segment );
        U32_SET_FIELD_MAC(regValue,24,3,currInfoPtr->lpmSubInfo.sbm_port1_lpm_address_segment );
        U32_SET_FIELD_MAC(regValue,28,1,currInfoPtr->lpmSubInfo.sbm_port0_lpm_taken_from_port1);
        U32_SET_FIELD_MAC(regValue,29,1,currInfoPtr->lpmSubInfo.sbm_port1_lpm_taken_from_port0);

        rc = prvCpssHwPpWriteRegister(devNum,
            PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.block_port_select[ii],
            regValue);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

#define AAS_GM_TABLE(t) AAS_GM_##t
/* the FDB size was limited to 64K , so can't use more than that .
due to HUGE allocations on many tables */
const unsigned AAS_GM_TABLE(MAC_TABLE_SIZE) = 64*1024;/*AmitK - 262144 kill the 'malloc' */

/**
* @internal fdbWriteToHwAndUpdateFineTune function
* @endinternal
*
* @brief    the function will write the FDB unit with the FDB size and MHT
*           and will update the DB info:
*           PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb
*           PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size
*           PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*
* @param[out]
*       None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_HW_ERROR    - on hardware error.
*
*/
static GT_STATUS fdbWriteToHwAndUpdateFineTune
(
    IN  GT_U8       devNum ,
    IN  GT_U32      fdbNumSbms
)
{
    GT_STATUS   rc;
    GT_U32  value;
    GT_U32  fdbSize = SBM_NUM_LINES * fdbNumSbms;

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice &&
        fdbSize >= AAS_GM_TABLE(MAC_TABLE_SIZE))
    {
        fdbSize = AAS_GM_TABLE(MAC_TABLE_SIZE);
    }

    /* update the <fdb> (involve updating other SW parameters and update the HW)*/
    /* will update : PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb     */
    /* will update : PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size    */
    rc = prvCpssDxChBrgFdbSizeSet(devNum,fdbSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* value is the MHT : 4/8/16 */
    value = fdbNumSbms <=  4 ?  4 :
            fdbNumSbms >= 16 ? 16 :
            8;
    /* will update HW and DB : PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank      */
    rc = prvCpssDxChBrgFdbMhtSet(devNum,value);

    return rc;
}

/**
* @internal emWriteToHwAndUpdateFineTune function
* @endinternal
*
* @brief    the function will write the FDB unit with the FDB size and MHT
*           and will update the DB info:
*           PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum
*           PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*
* @param[out]
*       None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_HW_ERROR    - on hardware error.
*
*/
static GT_STATUS emWriteToHwAndUpdateFineTune
(
    IN  GT_U8       devNum ,
    IN  GT_U32      emId,/*0..7*/
    IN  GT_U32      emNumSbms,
    IN  GT_BOOL     autoLearnEn
)
{
    GT_STATUS   rc;
    GT_U32  value;
    GT_U32  emSize = SBM_NUM_LINES * emNumSbms;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    exactMatchAutoLearnLookupNum;

    if(0 == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm)
    {
        /* NUM_EM was defined 0 , so the next API will fail if we call it */
        return GT_OK;
    }

    /* TBD: Disabling autolearn lookup for AAS
     * Will enable this once the Autolearn support is provided */
    if(((emId & 1) == 0) && 0)
    {
        /* config per dual clients */
        exactMatchAutoLearnLookupNum = (autoLearnEn == GT_TRUE) ?
            CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E :
            CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        rc = cpssDxChExactMatchAutoLearnLookupSet(devNum,CPSS_DXCH_EXACT_MATCH_UNIT_0_E,
                                                  exactMatchAutoLearnLookupNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* update the <emSize> (involve updating other SW parameters and update the HW)*/
    /* will update : PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum      */
    rc = prvCpssDxChCfgEmSizeSet(devNum, emId, emSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* value is the MHT : 4/8/16 */
    value = emNumSbms <=  4 ?  4 :
            emNumSbms >= 16 ? 16 :
            8;
    /* will update HW and DB : PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks  */
    rc = prvCpssDxChCfgEmMhtSet(devNum, emId, value);

    return rc;
}

/**
* @internal shmCncIpeMuxWriteToHw function
* @endinternal
*
* @brief    the function will write CNC/IPE0,1 mux into :
*        SHM_IP {Current}/SHM/SHM/IPE State0 Select register
*        SHM_IP {Current}/SHM/SHM/IPE State1 Select register
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       ipe0_usedCncBlocksBmp - the blocks that are selected to be
*           used from the CNC_0_1 , for the IPE0
*       ipe1_usedCncBlocksBmp - the blocks that are selected to be
*           used from the CNC_0_1 , for the IPE1
*
* @param[out]
*       None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_HW_ERROR    - on hardware error.
*
*/
static GT_STATUS shmCncIpeMuxWriteToHw
(
    IN  GT_U8       devNum,
    IN  GT_U32      ipe0_usedCncBlocksBmp,
    IN  GT_U32      ipe1_usedCncBlocksBmp
)
{
    GT_STATUS   rc;

    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.ipe0_state_select,
        ipe0_usedCncBlocksBmp);

    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SHM.ipe1_state_select,
        ipe1_usedCncBlocksBmp);

    return rc;
}

/**
* @internal emxWriteEmClientsToHw function
* @endinternal
*
* @brief    the function will write to EMX unit the EM clients
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       numEmUnitsNeeded - number of EM units in emUnitsArr[]
*       emUnitsArr[] - array of EM units, with the clients info (2 clients per unit)
*
* @param[out]
*       None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_HW_ERROR    - on hardware error.
*
*/
static GT_STATUS emxWriteEmClientsToHw
(
    IN  GT_U8       devNum,
    IN  GT_U32      numEmUnitsNeeded,
    IN  CPSS_DXCH_PP_CONFIG_SHARED_EM_UNIT_STC  emUnitsArr[/*CPSS_DXCH_PP_CONFIG_SHARED_MAX_NUM_EM_UNITS_E*/]
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj,index;
    static const GT_U32 converSwToHw[] = {
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE__NOT_CONNECTED__E  */  0xF
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_FIRST_E        */, 0
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_SECOND_E       */, 1
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL0_FIRST_E      */, 2
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL0_SECOND_E     */, 3
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL1_FIRST_E      */, 4
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL1_SECOND_E     */, 5
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_MPCL_FIRST_E       */, 6
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_MPCL_SECOND_E      */, 7
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_EPCL_FIRST_E       */, 8
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_EPCL_SECOND_E      */, 9
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPE_FIRST_E        */,10
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPE_SECOND_E       */,11
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_ILM2_E         */,12
        /*CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_ILM3_E         */,13
    };
    GT_U32  hwValue;


    for(ii = 0 ; ii < numEmUnitsNeeded ; ii++)
    {
        for(jj = 0 ; jj < 2 ; jj++)
        {
            index = 2*ii + jj;/*0/1/2/3/4/5/6/7*/

            hwValue = converSwToHw[emUnitsArr[ii].clientInfoArr[jj].clientType];

            /* apply the EM client tables */
            rc = prvCpssHwPpSetRegField(devNum,
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EMX.emChannel[index],
                0,4,hwValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

