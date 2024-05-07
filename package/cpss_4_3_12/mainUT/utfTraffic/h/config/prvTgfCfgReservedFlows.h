/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfCfgReservedFlows.h
*
* @brief     Test PHA threads for Reserved flows
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCfgReservedFlows
#define __prvTgfCfgReservedFlows

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* PHA fw thread IDs of */
#define PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_CNS    77
#define PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_CNS    78
#define PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_CNS    79

/* Generic TS medium type Data1 size */
#define PRV_TGF_TS_MEDIUM_TYPE_DATA1_SIZE_CNS     12

/* Generic TS Medium type structure */
typedef struct
{
    TGF_PACKET_L2_STC    tsL2part;                                        /* TS MAC DA + TA MAC SA */
    GT_U8                data1[PRV_TGF_TS_MEDIUM_TYPE_DATA1_SIZE_CNS];    /* TS 12 bytes DAta1     */
} TGF_PACKET_TS_MEDIUM_STC;

/* struct for RX0 packet */
typedef struct
{
    TGF_ETHER_TYPE      etherType;      /* 16 Bits */
    GT_U8               M;              /* 1  Bits */
    GT_U8               pType;          /* 5  Bits */
    GT_U8               X;              /* 2  Bits */
    GT_U8               Y;              /* 3  Bits */
    GT_U16              lookupKey;      /* 13 Bits */
    GT_U16              W;              /* 16 Bits */
} TGF_PACKET_RX0_STC;

/* struct for RX1 packet */
typedef struct
{
    TGF_ETHER_TYPE      etherType;      /* 16 Bits */
    GT_U8               M;              /* 1  Bits */
    GT_U8               pType;          /* 5  Bits */
    GT_U8               X;              /* 2  Bits */
    GT_U8               hopLimit;       /* 8  Bits */
    GT_U16              frwdLabel;      /* 16 Bits */
    GT_U16              K;              /* 16 Bits */
    GT_U8               Y;              /* 3  Bits */
    GT_U16              lookupKey;      /* 13 Bits */
    GT_U16              W;              /* 16 Bits */
} TGF_PACKET_RX1_STC;

/* struct for RX2 packet */
typedef struct
{
    TGF_ETHER_TYPE      etherType;      /* 16 Bits */
    GT_U8               M;              /* 1  Bits */
    GT_U8               pType;          /* 5  Bits */
    GT_U8               X;              /* 2  Bits */
    GT_U16              K;              /* 16 Bits */
    GT_U8               Y;              /* 3  Bits */
    GT_U32              Z;              /* 21 Bits */
    GT_U16              W;              /* 16 Bits */
} TGF_PACKET_RX2_STC;

/* struct for RX3 packet */
typedef struct
{
    TGF_ETHER_TYPE      etherType;      /* 16 Bits */
    GT_U8               M;              /* 1  Bits */
    GT_U8               pType;          /* 5  Bits */
    GT_U8               X;              /* 2  Bits */
    GT_U8               hopLimit;       /* 8  Bits */
    GT_U16              frwdLabel;      /* 16 Bits */
    GT_U16              K;              /* 16 Bits */
    GT_U8               Y;              /* 3  Bits */
    GT_U32              Z;              /* 21 Bits */
    GT_U16              W;              /* 16 Bits */
} TGF_PACKET_RX3_STC;

/* Thread 79 STRCs */
/* struct for RXP_6 packet */
typedef struct
{
        TGF_ETHER_TYPE etherType; /* 16 Bits */
        uint8_t M;                /* 1 Bits */
        uint8_t PType;            /* 5 Bits */
        uint8_t X;                /* 2 Bits */
        uint16_t K;               /* 16 Bits */
        uint8_t Y;                /* 3 Bits */
        uint32_t Z;               /* 21 Bits */
        uint16_t W;               /* 16 Bits */
} TGF_PACKET_RXP_Type_6_STC; /* 8B */

typedef struct
{
        TGF_ETHER_TYPE etherType;  /* 16 Bits */
        uint8_t M;                 /* 1 Bits */
        uint8_t PType;             /* 5 Bits */
        uint8_t X;                 /* 2 Bits */
        uint8_t Hop_Limit;         /* 8 Bits */
        uint16_t Forwarding_Label; /* 16 Bits */
        uint16_t K;                /* 16 Bits */
        uint8_t Y;                 /* 3 Bits */
        uint32_t Z;                /* 21 Bits */
        uint16_t W;                /* 16 Bits */
}  TGF_PACKET_JTM_Type_7_STC; /* 11B */

typedef struct
{
        TGF_ETHER_TYPE etherType;   /* 16 Bits */
        uint8_t M;                  /* 1 Bits */
        uint8_t PType;              /* 5 Bits */
        uint8_t X;                  /* 2 Bits */
        uint8_t Hop_Limit;          /* 8 Bits */
        uint16_t Forwarding_Label;  /* 16 Bits */
        uint16_t K;                 /* 16 Bits */
        uint8_t Y;                  /* 3 Bits */
        uint16_t Lookup_Key;        /* 13 Bits */
        uint16_t W;                 /* 16 Bits */
} TGF_PACKET_JTM_Type_2_STC; /* 10B */

/**
* @internal prvTgfCfgReservedFlowsConfigurationSet function
* @endinternal
*
* @brief  Test PHA thread with given phaThreadId for corresponding Reserved flows.
*    configure VLAN
*    Configure E2phy and Generic TS for eport
*    configure EPCL rule to set flowId and copy_reserved
*    send UC traffic with ethertype 0x1701 and verify expected traffic on target port
*/
void prvTgfCfgReservedFlowsConfigurationSet(GT_U32 phaThreadId);

/**
* @internal prvTgfCfgReservedFlowsTrafficSend function
* @endinternal
*
* @brief  Traffic test for PHA thread Reserved flows with generic TS encapsulation
*/
void prvTgfCfgReservedFlowsTrafficSend
(
    IN GT_U32    phaThreadId
);

/**
* @internal prvTgfCfgReservedFlowsConfigurationRestore function
* @endinternal
*
* @brief  Restore PHA threads for Reserved flows for given phaThreadId.
*/
void prvTgfCfgReservedFlowsConfigurationRestore
(
    IN GT_U32    phaThreadId
);

/**
* @internal prvTgfCfgRouterSaModificationConfigurationSet function
* @endinternal
*
* @brief    Target ePort Attribute assignments config
*            Enable tunnel start
*            Set tunnel start pointer = 10
*            Set tunnel generic entry genericType = medium, ethertype = 0x1701
*            and MAC DA = 00:00:00:00:44:44
*            PHA config set entry for Router Sa Modification
*
*/
GT_VOID prvTgfCfgRouterSaModificationConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCfgRouterSaModificationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and set EPCL rule for Router Sa Modification
*
*/
GT_VOID prvTgfCfgRouterSaModificationTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCfgRouterSaModificationConfigurationRestore function
* @endinternal
*
* @brief    Restore Router SA Modification configuration
*
*/
GT_VOID prvTgfCfgRouterSaModificationConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfInqaEcnConfigurationSet function
* @endinternal
*
* @brief  Test PHA thread with given phaThreadId = PRV_TGF_PHA_THREAD_ID_INQA_ECN_CNS.
*    configure VLAN
*    configure PHA
*/
void prvTgfCfgInqaEcnConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCfgInqaEcnTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and set EPCL rule for INQA ECN
*          send IP UC traffic with verify expected traffic on target port
*
*/
GT_VOID prvTgfCfgInqaEcnTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCfgInqaEcnConfigurationRestore function
* @endinternal
*
* @brief    Restore INQA ECN configuration
*
*/
GT_VOID prvTgfCfgInqaEcnConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfCfgVxlanDciVniRemapConfigurationSet function
* @endinternal
*
* @brief  Test PHA thread with given phaThreadId = PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS.
*    configure VLAN
*    configure PHA
*/
void prvTgfCfgVxlanDciVniRemapConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCfgVxlanDciVniRemapTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and set EPCL rule for VXLAN DCI VNI REMAP
*          send IP UDP UC traffic with verify expected traffic on target port
*
*/
GT_VOID prvTgfCfgVxlanDciVniRemapTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCfgVxlanDciVniRemapConfigurationRestore function
* @endinternal
*
* @brief    Restore VXLAN DCI VNI REMAP configuration
*
*/
GT_VOID prvTgfCfgVxlanDciVniRemapConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfCfgMplsLsrInStackingSystemConfigSet function
* @endinternal
*
* @brief  Test PHA thread with given phaThreadId = PRV_TGF_PHA_THREAD_ID_MPLS_LSR_IN_STACKING_SYSTEM_CNS.
*         configure Bridge
*/
void prvTgfCfgMplsLsrInStackingSystemConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfCfgMplsLsrInStackingSystemTrafficGenerate function
* @endinternal
*
* @brief   PHA config
*          Generate traffic
*          send DSA tagged traffic verify expected traffic on target port
*
*/
GT_VOID prvTgfCfgMplsLsrInStackingSystemTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCfgMplsLsrInStackingSystemConfigRestore function
* @endinternal
*
* @brief    Restore MPLS LSR In Stacking System configuration
*
*/
GT_VOID prvTgfCfgMplsLsrInStackingSystemConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCfgReservedFlows */
