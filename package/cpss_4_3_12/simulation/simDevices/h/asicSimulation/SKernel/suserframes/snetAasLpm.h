/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* snetAasLpm.h
*
* DESCRIPTION:
*       This is a external API definition for SIP7 LPM
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __snetAasLpmh
#define __snetAasLpmh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2Routing.h>
#include <asicSimulation/SKernel/suserframes/snetFalconLpm.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
*   snetAasLpm
*
* DESCRIPTION:
*        Aas LPM lookup function
*
* INPUTS:
*        devObjPtr - pointer to device object.
*        descrPtr  - pointer to the frame's descriptor.
*
* OUTPUTS:
*        matchIndexPtr         - array to indicate dip sip match
*        priorityPtr           - routing priority between FDB and LPM lookup
*        isPbrPtr              - pointer to isPbr with values
*                                  GT_TRUE  - Policy Based Routing
*                                  GT_FALSE - LPM lookup
*        ipSecurChecksInfoPtr  - routing security checks information
*
* RETURNS:
*       whether sip lookup was performed
*
*******************************************************************************/
GT_U32 snetAasLpm
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    OUT GT_U32                              *matchIndexPtr,
    OUT GT_BOOL                             *isPbrPtr,
    OUT LPM_LEAF_PRIORITY_ENT               *priorityPtr,
    OUT SNET_ROUTE_SECURITY_CHECKS_INFO_STC *ipSecurChecksInfoPtr
);

/*******************************************************************************
*   snetSip7FetchRouteEntry
*
* DESCRIPTION:
*       process sip/dip ecmp or qos leafs (if any)
*       and update matchIndex array (if nessecary)
*
* INPUTS:
*        devObjPtr     - pointer to device object.
*        descrPtr      - pointer to the frame's descriptor.
*        dipLpmDataPtr - pointer to dip leaf entry structure
*        sipLpmDataPtr - pointer to sip leaf entry structure
*        matchIndexPtr - array to indicate dip sip match
*
* OUTPUTS:
*        matchIndexPtr         - array to indicate dip sip match
*
*******************************************************************************/
GT_VOID snetSip7FetchRouteEntry
(
    IN    SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN    SNET_SIP6_LPM_MEM_LEAF_STC          *dipLpmDataPtr,
    IN    SNET_SIP6_LPM_MEM_LEAF_STC          *sipLpmDataPtr,
    INOUT GT_U32                              *matchIndexPtr
);


/*******************************************************************************
*   snetSip7LpmUcSipLookup
*
* DESCRIPTION:
*        sip7 LPM UC SIP lookup function
*
* INPUTS:
*        devObjPtr - pointer to device object.
*        descrPtr  - pointer to the frame's descriptor.
*
* OUTPUTS:
        matchIndexPtr - array to indicate sip match
*       sipLpmLeafPtr - pointer to sip leaf entry structure
*       ipSecurChecksInfoPtr  - routing security checks information
*
* RETURNS:
*       whether sip lookup was performed
*
*******************************************************************************/
GT_U32 snetSip7LpmUcSipLookup
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    OUT GT_U32                              *matchIndexPtr,
    OUT SNET_SIP6_LPM_MEM_LEAF_STC          *sipLpmLeafPtr,
    OUT SNET_ROUTE_SECURITY_CHECKS_INFO_STC *ipSecurChecksInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetAasLpmh */









