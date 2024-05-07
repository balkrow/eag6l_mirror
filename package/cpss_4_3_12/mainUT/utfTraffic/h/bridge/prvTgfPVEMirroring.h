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
* @file prvTgfPVEMirroring.h
*
* @brief
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPVEMirroring
#define __prvTgfPVEMirroring

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_TGF_PVE_INGRESS_PORT_IDX_CNS        3
#define PRV_TGF_PVE_EGR_PORT_IDX_CNS            1

#define PRV_TGF_PVE_PACKET_SIZE_CNS (127)

GT_VOID prvTgfBrgPVEMirroringConfigSet(GT_VOID);
GT_VOID prvTgfBrgPVEMirroringTrafficGenerate(GT_VOID);
GT_VOID prvTgfBrgPVEMirroringConfigRestore(GT_VOID);
/*GT_VOID tgfBridgePVEMirroringVerification(GT_U32  packetSize);   */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPVEMirroring */



