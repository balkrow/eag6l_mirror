/* *****************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
/**
 * @file mpdUtils.c
 *  @brief MPD utility functions that are shared among all MPD modules
 *
 */
#include    <stdarg.h>
#include 	<mpdPrefix.h>
#include    <mpdPrv.h>
#include    <mpdDebug.h>

#ifdef MPD_XML_INIT
#include    <pdlib/lib/pdlLib.h>
#include    <pdlib/lib/private/prvPdlLib.h>
#include    <pdlib/xml/private/prvXmlParser.h>
#include    <pdlib/xml/private/prvXmlParserBuilder.h>
#include    <pdlib/init/pdlInit.h>
#include    <parser/mpdParser.h>
#endif

#undef __FUNCTION__

/* this array is used in order to count num of members in PRV_MPD_PORT_LIST_TYP */

static const UINT_8 prvMpdByteBitmapPopulation [256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

static const PRV_MPD_PORT_LIST_TYP prvMpdEmptyPortList_CNS = {{0, 0, 0, 0}};

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListIsMember
*
* DESCRIPTION: TRUE when rel_ifIndex is member of portList_PTR
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListIsMember(
    /*     INPUTS:             */
    UINT_32                 rel_ifIndex,
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_32 word, idx;

    if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        return FALSE;
    }
    word = ((rel_ifIndex - 1)/32);
    idx = ((rel_ifIndex - 1)%32);

    return (portList_PTR->portsList[word] & (1<<idx))?TRUE:FALSE;

}
/* END OF prvMpdPortListIsMember */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListIsEmpty
*
* DESCRIPTION: checks if no port is selected in set
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListIsEmpty(
    /*!     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    BOOLEAN is_empty = TRUE;
    UINT_32 word;

    for (word = 0; word < PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS; word++) {
        if (portList_PTR->portsList[word] != 0) {
            is_empty = FALSE;
            break;
        }
    }
    return is_empty;
}
/*$ END OF <prvMpdPortListIsEmpty> */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListGetNext
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListGetNext(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portsList_PTR,
    /*     INPUTS / OUTPUTS:   */
    UINT_32    * relative_ifIndex_PTR
    /*     OUTPUTS:            */
)
{
    UINT_32 i_in_word = 0, word = 0;

    (* relative_ifIndex_PTR)++;
    if (PRV_MPD_PORT_LIST_REL_IFINDEX_IS_LEGAL_MAC((* relative_ifIndex_PTR)) == TRUE) {
        PRV_MPD_PORT_LIST_GET_SHIFT_AND_WORD_MAC((* relative_ifIndex_PTR), i_in_word, word);

        for (; word < PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS; word++, i_in_word = 0) {
            for (; i_in_word < 32; i_in_word++) {
                if (portsList_PTR->portsList[word] & (1 << i_in_word)) {
                    (* relative_ifIndex_PTR) = PRV_MPD_PORT_LIST_WORD_AND_BIT_TO_REL_IFINDEX_MAC(word, i_in_word);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

/*$ END OF  prvMpdPortListGetNext */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListNumOfMembers
*
* DESCRIPTION: get num of ports in pset
*
*
*****************************************************************************/
extern UINT_32 prvMpdPortListNumOfMembers(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portsList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_32      tmp_byte;
    UINT_32      tmp_word = 0, mask;
    UINT_32      numOfMembers = 0, i, j;

    if (portsList_PTR == NULL) {
        return 0;
    }

    for (i = 0; i < PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS; i++) {
        tmp_word = portsList_PTR->portsList[i];
        mask = 0xFF;
        for (j = 0; j < 4; j++) {
            tmp_byte = ((tmp_word & mask) >> (8 * j));
            numOfMembers += prvMpdByteBitmapPopulation[tmp_byte];
            mask = mask << 8;
        }
    }
    return numOfMembers;

}

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListRemove
*
* DESCRIPTION:
*
*
*****************************************************************************/
BOOLEAN prvMpdPortListRemove(
    /*     INPUTS:             */
    UINT_32                         rel_ifIndex,
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_32 word, idx;

    if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        return FALSE;
    }
    word = ((rel_ifIndex - 1)/32);
    idx = ((rel_ifIndex - 1)%32);
    portList_PTR->portsList[word] &= ~(1<<idx);
    return TRUE;
}
/* END OF prvMpdPortListRemove */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListClear
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern void prvMpdPortListClear(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    if (portList_PTR) {
        memset(portList_PTR, 0, sizeof(PRV_MPD_PORT_LIST_TYP));
    }
}

/*$ END OF  prvMpdPortListClear */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListAdd
*
* DESCRIPTION:
*
*
*****************************************************************************/
BOOLEAN prvMpdPortListAdd(
    /*     INPUTS:             */
    UINT_32                 rel_ifIndex,
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_32 word, idx;

    if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        return FALSE;
    }
    word = ((rel_ifIndex - 1)/32);
    idx = ((rel_ifIndex - 1)%32);
    portList_PTR->portsList[word] |= (1<<idx);
    return TRUE;
}
/* END OF prvMpdPortListAdd */

/* ***************************************************************************
* FUNCTION NAME: prvMpdCheckIsPortValid
*
* DESCRIPTION: check that the port exist and valid in the DB, and return the entry
*
*
*****************************************************************************/
static PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdCheckIsPortValid(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC * portEntry_PTR = NULL;

    if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC() == NULL) {
        return NULL;
    }

    if (rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        return NULL;
    }

    if (prvMpdPortListIsMember(rel_ifIndex, &(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->validPorts)) == FALSE) {
        return NULL;
    }
    portEntry_PTR = &(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex]);

    return portEntry_PTR;

}
/* END OF prvMpdCheckIsPortValid */



/* ***************************************************************************
* FUNCTION NAME: prvMpdPortHashCreate
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern MPD_RESULT_ENT prvMpdPortHashCreate(
    /*     INPUTS:             */
    void
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC() != NULL) {
        return MPD_NOT_SUPPORTED_E;
    }

    PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC() = PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_GLOBAL_DB_STC));
    if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()) {
        memset(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries, 0, sizeof(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries));
        PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->numOfMembers = 0;
        PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->validPorts = prvMpdEmptyPortList_CNS;
        return MPD_OK_E;
    }
    return MPD_OP_FAILED_E;
}
/* END OF prvMpdPortHashCreate */


/* ***************************************************************************
* FUNCTION NAME: mpdPortInitDbValidation
*
* DESCRIPTION: validate port init db function.
*
*****************************************************************************/
static MPD_RESULT_ENT mpdPortInitDbValidation(
    /*     INPUTS:             */
    UINT_32                     rel_ifIndex,
    MPD_PORT_INIT_DB_STC      * data_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    char error_msg [256];
    UINT_32 i = 0;

    if (data_PTR->phyType >= MPD_TYPE_NUM_OF_TYPES_E) {
        prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - received %d which is not valid PHY type\n", rel_ifIndex, data_PTR->phyType);
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                   MPD_ERROR_SEVERITY_FATAL_E,
                                   error_msg);
        return MPD_OP_FAILED_E;
    }

    if (data_PTR->phyNumber > PRV_MPD_MAX_NUM_OF_PHY_CNS) {
        sprintf(error_msg, "rel_ifIndex %d - received %d which is not valid PHY number\n", rel_ifIndex, data_PTR->phyType);
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                   MPD_ERROR_SEVERITY_FATAL_E,
                                   error_msg);
        return MPD_OP_FAILED_E;
    }

    if (data_PTR->transceiverType >= MPD_TRANSCEIVER_MAX_TYPES_E) {
        prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - received %d which is not valid transceiver type\n", rel_ifIndex, data_PTR->transceiverType);
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                   MPD_ERROR_SEVERITY_FATAL_E,
                                   error_msg);
        return MPD_OP_FAILED_E;
    }
    if (data_PTR->usxInfo.usxType != MPD_PHY_USX_TYPE_NONE) {
        if (data_PTR->usxInfo.usxType >= MPD_PHY_USX_MAX_TYPES) {
            prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - received %d which is not valid USX type\n", rel_ifIndex, data_PTR->usxInfo.usxType);
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_FATAL_E,
                                       error_msg);
            return MPD_OP_FAILED_E;
        }
        if ((data_PTR->phyType == MPD_TYPE_88E2540_E) || (data_PTR->phyType == MPD_TYPE_88E2580_E)) {
            if (data_PTR->usxInfo.usxType == MPD_PHY_USX_TYPE_20G_DXGMII) {
                prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - USX type 20G DXGMII is not supported on this PHY\n", rel_ifIndex);
                PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                           MPD_ERROR_SEVERITY_FATAL_E,
                                           error_msg);
                return MPD_OP_FAILED_E;
            }
        }
    }
    if (data_PTR->mdioInfo.mdioAddress  > PRV_MPD_MAX_MDIO_ADRESS_CNS) {
        prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - received %d which is not valid MDIO address\n", rel_ifIndex, data_PTR->mdioInfo.mdioAddress);
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                   MPD_ERROR_SEVERITY_FATAL_E,
                                   error_msg);
        return MPD_OP_FAILED_E;
    }
    if ((data_PTR->phyType == MPD_TYPE_88E1543_E) && (data_PTR->mdioInfo.mdioAddress == 0)) {
        prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - MDIO address 0 is not valid for this PHY\n", rel_ifIndex);
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                   MPD_ERROR_SEVERITY_FATAL_E,
                                   error_msg);
        return MPD_OP_FAILED_E;
    }

    if (data_PTR->serdesParams_ARR != NULL){
        while ((data_PTR->serdesParams_ARR[i].serdesTuneParams.hostOrLineSide != MPD_PHY_SIDE_LAST_E) &&
               (i < PRV_MPD_MAX_NUM_OF_SERDES_PARAMS)){
            i++;
        }
        if (i >= PRV_MPD_MAX_NUM_OF_SERDES_PARAMS){
            prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - max serdes list permitted is PRV_MPD_MAX_NUM_OF_SERDES_PARAMS: %d\n", rel_ifIndex, PRV_MPD_MAX_NUM_OF_SERDES_PARAMS);
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_FATAL_E,
                                       error_msg);
            return MPD_OP_FAILED_E;
        }
    }

    if (!PRV_MPD_IS_INFO_VALID_MAC(data_PTR)){
        prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - application validity failed.\n", rel_ifIndex);
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                   MPD_ERROR_SEVERITY_FATAL_E,
                                   error_msg);
        return MPD_OP_FAILED_E;
    }

    return MPD_OK_E;
}

/* END OF mpdPortInitDbValidation */

/* ***************************************************************************
* FUNCTION NAME: mpdPortRunningDbInit
*
* DESCRIPTION: init port running db function.
*
*****************************************************************************/
static MPD_RESULT_ENT mpdPortRunningDbInit(
    /*     INPUTS:             */
    UINT_32                       rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    memset(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR, 0, sizeof(PRV_MPD_RUNNING_DB_STC));
    PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR->adminMode = MPD_PORT_ADMIN_UNKNOWN_E;
    PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR->macOnPhyState = PRV_MPD_MAC_ON_PHY_NOT_INITIALIZED_E;
    PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR->opMode = MPD_OP_MODE_UNKNOWN_E;
    PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR->sfpPresent = FALSE;
    PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR->speed = MPD_SPEED_LAST_E;
    PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR->comboMediaTypeChanged = FALSE;
    PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR->duplex = MPD_DUPLEX_ADMIN_MODE_FULL_E;

    return MPD_OK_E;
}

/* END OF mpdPortRunningDbInit */



/* ***************************************************************************
* FUNCTION NAME: mpdPortDbUpdate
*
* DESCRIPTION: update entry for <rel_ifIndex>
*           allocates and adds entry if not allocated
*
*****************************************************************************/
extern MPD_RESULT_ENT mpdPortDbUpdate(
    /*     INPUTS:             */
    UINT_32                     rel_ifIndex,
    MPD_PORT_INIT_DB_STC      * data_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    BOOLEAN                 exists = FALSE;
    MPD_RESULT_ENT          status = MPD_OK_E;

    if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC() == NULL) {
        return MPD_OP_FAILED_E;
    }

    if (rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        return MPD_OP_FAILED_E;
    }

    if (prvMpdPortListIsMember(rel_ifIndex, &(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->validPorts))) {
        exists = TRUE;
    }
    /* remove entry */
    if (data_PTR == NULL) {
        if (exists) {
            prvMpdPortListRemove(rel_ifIndex, &(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->validPorts));
            PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->numOfMembers--;
            return MPD_OK_E;
        }
        return MPD_OP_FAILED_E;
    }

    /* allocation is required */
    if (exists == FALSE) {
        status = mpdPortInitDbValidation(rel_ifIndex, data_PTR);
        if (status != MPD_OK_E) {
            return MPD_OP_FAILED_E;
        }
        PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].initData_PTR = PRV_MPD_ALLOC_MAC(sizeof(MPD_PORT_INIT_DB_STC));
        /* allocation failed */
        if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].initData_PTR == NULL) {
            return MPD_OP_FAILED_E;
        }
        PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR = PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_RUNNING_DB_STC));
        /* allocation failed */
        if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR == NULL) {
            return MPD_OP_FAILED_E;
        }

        mpdPortRunningDbInit(rel_ifIndex);
        PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->numOfMembers++;
        prvMpdPortListAdd(rel_ifIndex, &(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->validPorts));
        PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].rel_ifIndex = rel_ifIndex;
    }
    /* add the entry */
    memcpy(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].initData_PTR, data_PTR, sizeof(MPD_PORT_INIT_DB_STC));

    /* To access the TAI registers port 0 must be set to page 26 */
    if (PRV_MPD_SHARED_GLBVAR_PHY_DB_ACCESS_MAC()[data_PTR->phyNumber].representativePortEntry_PTR == NULL){
        if ((PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].initData_PTR->mdioInfo.mdioAddress % 8) == 0) {
            PRV_MPD_SHARED_GLBVAR_PHY_DB_ACCESS_MAC()[data_PTR->phyNumber].representativePortEntry_PTR = &(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex]);
        }
    }

    return MPD_OK_E;
}
/* END OF mpdPortDbUpdate */

#ifdef MPD_XML_INIT


/* ***************************************************************************
* FUNCTION NAME: prvMpdConvertXmlPhyType
*
* DESCRIPTION: convert phy type of xml to mpd
*
*****************************************************************************/
static  MPD_RESULT_ENT prvMpdConvertXmlPhyType(
    IN MPD_PHY_ENUM_TYPE_ENT xmlPhyType,
    OUT MPD_TYPE_ENT        *phyTypePtr
)
{
    MPD_TYPE_ENT        phyType;

    if (phyTypePtr == NULL) {
        return MPD_OP_FAILED_E;
    }

    switch (xmlPhyType) {
    case MPD_PHY_ENUM_TYPE_NO_PHY_DIRECT_ATTACHED_FIBER_E:
        phyType = MPD_TYPE_DIRECT_ATTACHED_FIBER_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E1543_E:
        phyType = MPD_TYPE_88E1543_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E1545_E:
        phyType = MPD_TYPE_88E1545_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E1548_E:
        phyType = MPD_TYPE_88E1548_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E1680_E:
        phyType = MPD_TYPE_88E1680_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E1680L_E:
        phyType = MPD_TYPE_88E1680L_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E151X_E:
        phyType = MPD_TYPE_88E151x_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E3680_E:
        phyType = MPD_TYPE_88E3680_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E32X0_E:
        phyType = MPD_TYPE_88X32x0_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E33X0_E:
        phyType = MPD_TYPE_88X33x0_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E20X0_E:
        phyType = MPD_TYPE_88X20x0_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E2180_E:
        phyType = MPD_TYPE_88X2180_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E2540_E:
        phyType = MPD_TYPE_88E2540_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88X3540_E:
        phyType = MPD_TYPE_88X3540_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E1780_E:
        phyType = MPD_TYPE_88E1780_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E1781_E:
        phyType = MPD_TYPE_88E1781_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E1781_INTERNAL_E:
        phyType = MPD_TYPE_88E1781_internal_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E2580_E:
        phyType = MPD_TYPE_88E2580_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88X3580_E:
        phyType = MPD_TYPE_88X3580_E;
        break;
    case MPD_PHY_ENUM_TYPE_ALASKA_88E1112_E:
        phyType = MPD_TYPE_88E1112_E;
        break;
    case MPD_PHY_ENUM_TYPE_LAST_E:
    default:
        return MPD_OP_FAILED_E;
    }

    * phyTypePtr = phyType;

    return MPD_OK_E;

}

/**
 * @fn  MPD_RESULT_ENT prvMpdXmlSignature ( IN char * xmlFilePathPtr, IN UINT_32 signatureSize, OUT UINT_8  *signaturePtr)
 *
 * @brief   calculate xml file md5 signature
 * @param [in]  xmlFilePathPtr  XML location.
 * @param [in]  signatureSize  required signature length
 * @param [out] signaturePtr   md5 digest result
 * @return  TRUE if success.
 */
static BOOLEAN prvMpdXmlSignature(
/*!     INPUTS:             */
    IN  char       *xmlFileNamePtr,
    IN UINT_32      signatureSize,
    OUT UINT_8     *signaturePtr
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    FILE                              * xml_fd;
    UINT_32                             file_size;
    PDL_LIB_MD5_DIGEST_STC              md5_digest;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/

    xml_fd = fopen(xmlFileNamePtr, "rb");
    if (!xml_fd) {
        return FALSE;
    }

    /* get md5 checksum for file & convert to string format */
    fseek(xml_fd,0L,SEEK_END);
    file_size=ftell(xml_fd);
    fseek(xml_fd, 0L, SEEK_SET);
    pdlibMd5DigestCompute(&xml_fd, NULL, file_size, &md5_digest);
    fclose (xml_fd);

    memcpy(signaturePtr, &md5_digest, signatureSize);
    return TRUE;
}
/*$ END OF prvMpdXmlSignature */

/**
 * @fn  MPD_RESULT_ENT prvMpdConvertXMLOpModeToMpd ( IN MPD_OP_CONNECTED_TYPE_ENT xmlOpMode, IN MPD_SPEED_TYPE_ENT xmlSpeed, OUT MPD_OP_MODE_ENT *mpdOpModePTR)
 *
 * @brief   convert XML opMode to MPD opMode
 * @param [in]  xmlOpMode
 * @param [in]  xmlSpeed
 * @param [out] mpdOpModePTR
 * @return  MPD_OK if success.
 */
static MPD_RESULT_ENT prvMpdConvertXMLOpModeToMpd(
/*!     INPUTS:             */
    IN MPD_OP_CONNECTED_TYPE_ENT xmlOpMode,
	IN MPD_SPEED_TYPE_ENT xmlSpeed,
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
	OUT  MPD_OP_MODE_ENT *mpdOpModePTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
	MPD_OP_MODE_ENT mpdOpMode;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/
	if (mpdOpModePTR == NULL) {
		return MPD_OP_FAILED_E;
	}
	switch (xmlOpMode) {
	case MPD_OP_CONNECTED_TYPE_FIBER_E:
		mpdOpMode = (xmlSpeed == MPD_SPEED_TYPE_10G_E) ? MPD_OP_MODE_FIBER_SFP_PLUSE_E : MPD_OP_MODE_FIBER_E;
		break;
	case MPD_OP_CONNECTED_TYPE_DAC_E:
		mpdOpMode = MPD_OP_MODE_DIRECT_ATTACH_E;
		break;
	case MPD_OP_CONNECTED_TYPE_RJ45_SFP_E:
		mpdOpMode = MPD_OP_MODE_COPPER_SFP_1000BASE_X_MODE_E;
		break;
	case MPD_OP_CONNECTED_TYPE_UNKNOWN_E:
		mpdOpMode = MPD_OP_MODE_UNKNOWN_E;
		break;
	case MPD_OP_CONNECTED_TYPE_ALL_E:
	case MPD_OP_CONNECTED_TYPE_LAST_E:
		mpdOpMode = MPD_OP_MODE_LAST_E;
		break;
	default:
		PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
				"failed to convert xmlOpMode %d and xmlSpeed %d"
				, xmlOpMode, xmlSpeed);
		return MPD_OP_FAILED_E;
	}
	*mpdOpModePTR = mpdOpMode;
	return MPD_OK_E;
}

/**
 * @fn  MPD_RESULT_ENT mpdPortDbUpdateFromXml ( IN char * xmlFilePathPtr, IN char * xmlTagPrefixPtr)
 *
 * @brief   initialize mpd ports info from xml
 * @param [in]  xmlFilePathPtr  XML location.
 * @param [in]  signatureFilePathPtr  md5 file location - for authentication of the xml file, if not required pass NULL.
 * @return  MPD_RESULT_ENT.
 */

extern MPD_RESULT_ENT mpdPortDbUpdateFromXml(
    IN char                       * xmlFilePathPtr,
    IN char                       * signatureFilePathPtr
)
{
    MPD_RESULT_ENT                                   status;
    UINT_32                                          rel_ifIndex;
    UINT_32                                          numOfTxParams, i;
    XML_PARSER_RET_CODE_TYP                          xmlStatus;
    MPD_MPD_PARAMS_STC                               mpdParams;
    MPD_PHY_LIST_PARAMS_STC                         *phyList_PTR;
    MPD_PORT_LIST_PARAMS_STC                        *portList_PTR;
    MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC            *comphyTxParamsList_PTR;
    MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC    *comphyVoltronTxParamsList_PTR;
    MPD_PORT_INIT_DB_STC                             phyData;
    PDLIB_OS_CALLBACK_API_STC                        pdLibCallbacks;
    PDL_STATUS                                       pdlStatus, pdlStatus2, pdlStatus3;
    MPD_OP_MODE_ENT									 opMode;
     char error_msg [256];

    memset(&pdLibCallbacks, 0, sizeof(pdLibCallbacks));
    pdLibCallbacks.printStringPtr       = printf;
    pdLibCallbacks.mallocPtr            = malloc;
    pdLibCallbacks.freePtr              = free;
    pdLibCallbacks.debugLogPtr          = PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().debug_log_PTR;
    pdLibCallbacks.getXmlSignatureClbk  = prvMpdXmlSignature;

    pdlStatus = pdlibLibInit(&pdLibCallbacks);

    /* builds database from XML file and returns descriptor to that database */
    xmlStatus = xmlParserUncompressedFileBuild (xmlFilePathPtr, signatureFilePathPtr, PRV_MPD_XML_TAG_PREFIX_CNS, &(PRV_MPD_NONSHARED_GLBVAR_MPDUTILS_XMLROOT_VOID_ACCESS_MAC()));
    XML_CHECK_STATUS(xmlStatus);
    /* initialize parser component */
    status = mpdProjectParamsInit(PRV_MPD_NONSHARED_GLBVAR_MPDUTILS_XMLROOT_ACCESS_MAC());
    if (status != MPD_OK_E){
        return status;
    }

    /* getting parsed data */
    status = mpdProjectParamsGet(MPD_FEATURE_ID_MPD_E, (MPD_FEATURE_DATA_PARAMS_UNT *) &mpdParams);
    if (status != MPD_OK_E){
        return status;
    }
    /* update DB that configuration done from XML */
    PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->infoReceivedFromXml = TRUE;

    /* going over parsed data and update PRV_MPD_SHARED_GLBVAR_PHY_DB_ACCESS_MAC()  */
    /* getting the first phy db from parser */
    pdlStatus = prvPdlibDbGetFirst(mpdParams.phyList_PTR, (void **)& phyList_PTR);
    while (pdlStatus == PDL_OK) {
        memset(&phyData, 0, sizeof(phyData));
        phyData.phyNumber = phyList_PTR->list_keys.phyNumber;
        status = prvMpdConvertXmlPhyType(phyList_PTR->phyType, &phyData.phyType);
        if (status != MPD_OK_E){
            return status;
        }
        phyData.disableOnInit = mpdParams.disableOnInit;
        if (mpdIsFieldHasValue(phyList_PTR->isOob_mask)) {
            phyData.isOob = phyList_PTR->isOob;
        }
        if (mpdIsFieldHasValue(phyList_PTR->oobDevice_mask)) {
            phyData.mdioInfo.mdioDev = phyList_PTR->oobDevice;
        }
        if (mpdIsFieldHasValue(phyList_PTR->usxMode_mask)) {
            phyData.usxInfo.usxType = (MPD_PHY_USX_TYPE_ENT)phyList_PTR->usxMode;
        }
        if (mpdIsFieldHasValue(phyList_PTR->ledMode_mask)) {
            phyData.ledMode = (MPD_LED_MODE_TYPE_ENT)phyList_PTR->ledMode;
        }
        if (mpdIsFieldHasValue(phyList_PTR->phyFirmwareDownloadMethod_mask)){
            PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->fwDownloadType_ARR[phyData.phyType] = (MPD_FW_DOWNLOAD_TYPE_ENT) phyList_PTR->phyFirmwareDownloadMethod;
        }
        /* getting the first port db on the phy */
        pdlStatus2 = prvPdlibDbGetFirst(phyList_PTR->portList_PTR,
                                        (void **)& portList_PTR);
        while (pdlStatus2 == PDL_OK) {
            rel_ifIndex = portList_PTR->list_keys.logicalPort;
            if (mpdIsFieldHasValue(portList_PTR->mdioAddress_mask)) {
                phyData.mdioInfo.mdioAddress = portList_PTR->mdioAddress;
            }
            else if (mpdIsFieldHasValue(portList_PTR->oobAddress_mask)) {
                phyData.mdioInfo.mdioAddress = portList_PTR->oobAddress;
            }
            else if (phyData.phyType != MPD_TYPE_DIRECT_ATTACHED_FIBER_E){
                prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - no mdio address \n", rel_ifIndex);
                PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                           MPD_ERROR_SEVERITY_FATAL_E,
                                           error_msg);
                return MPD_OP_FAILED_E;
            }
            if (mpdIsFieldHasValue(portList_PTR->mdioDevice_mask)) {
                phyData.mdioInfo.mdioDev = portList_PTR->mdioDevice;
            }
            if (mpdIsFieldHasValue(portList_PTR->mdioBus_mask)) {
                phyData.mdioInfo.mdioBus = portList_PTR->mdioBus;
            }
            if (mpdIsFieldHasValue(portList_PTR->mtdSpecificInformationType_mask)) {
                phyData.doSwapAbcd = portList_PTR->mtdSpecificInformationType.doSwapAbcd;
                phyData.shortReachSerdes =
                portList_PTR->mtdSpecificInformationType.shortReachSerdes;
            }
            else {
                phyData.doSwapAbcd = FALSE;
                phyData.shortReachSerdes = FALSE;
            }
            if (mpdIsFieldHasValue(portList_PTR->rj45PortInformationType_mask)) {
                phyData.transceiverType = (MPD_TRANSCEIVER_TYPE_ENT)portList_PTR->rj45PortInformationType.transceiverType;
            }
            else if (mpdIsFieldHasValue(portList_PTR->fiberPortInformationType_mask)){
                phyData.transceiverType = (MPD_TRANSCEIVER_TYPE_ENT)portList_PTR->fiberPortInformationType.transceiverType;
            }
            else if (mpdIsFieldHasValue(portList_PTR->comboPortInformationType_mask)){
                phyData.transceiverType = (MPD_TRANSCEIVER_TYPE_ENT) portList_PTR->comboPortInformationType.transceiverType;
            }
            else {
                prvMpdSnprintf(error_msg, sizeof(error_msg),"rel_ifIndex %d - no TRANSCEIVER TYPE info\n", rel_ifIndex);
                PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                           MPD_ERROR_SEVERITY_FATAL_E,
                                           error_msg);
                return MPD_OP_FAILED_E;
            }
            numOfTxParams = 0;
            phyData.serdesParams_ARR = NULL;
            if (mpdIsFieldHasValue(portList_PTR->comphyTxParamsType_mask)){
                pdlStatus3 = prvPdlibDbGetFirst(portList_PTR->comphyTxParamsType.comphyTxParamsList_PTR, (void **)& comphyTxParamsList_PTR);
                while (pdlStatus3 == PDL_OK){
                	numOfTxParams++;
                    pdlStatus3 = prvPdlibDbGetNext(portList_PTR->comphyTxParamsType.comphyTxParamsList_PTR, (void *)&comphyTxParamsList_PTR->list_keys, (void **)&comphyTxParamsList_PTR);
                }

            }
            else if (mpdIsFieldHasValue(portList_PTR->comphyVoltronTxParamsType_mask)){
                pdlStatus3 = prvPdlibDbGetFirst(portList_PTR->comphyVoltronTxParamsType.comphyVoltronTxParamsList_PTR, (void **)& comphyVoltronTxParamsList_PTR);
                while (pdlStatus3 == PDL_OK){
                	numOfTxParams++;
                    pdlStatus3 = prvPdlibDbGetNext(portList_PTR->comphyVoltronTxParamsType.comphyVoltronTxParamsList_PTR, (void *)&comphyVoltronTxParamsList_PTR->list_keys, (void **)&comphyVoltronTxParamsList_PTR);
                }
            }
            if (numOfTxParams > 0){
                /* numOfTxParams + 1 -> for last element */
                phyData.serdesParams_ARR = (MPD_SERDES_CONFIG_PARAMS_STC*) PRV_MPD_CALLOC_MAC(numOfTxParams + 1, sizeof(MPD_SERDES_CONFIG_PARAMS_STC));
                if (phyData.serdesParams_ARR == NULL) {
                	return MPD_OUT_OF_CPU_MEM_E;
                }
                i = 0;
                if (mpdIsFieldHasValue(portList_PTR->comphyTxParamsType_mask)){
                    pdlStatus3 = prvPdlibDbGetFirst(portList_PTR->comphyTxParamsType.comphyTxParamsList_PTR, (void **)& comphyTxParamsList_PTR);
                    while (pdlStatus3 == PDL_OK){
                    	prvMpdConvertXMLOpModeToMpd(comphyTxParamsList_PTR->list_keys.opConnected, comphyTxParamsList_PTR->list_keys.speed, &opMode);
                        phyData.serdesParams_ARR[i].opMode = opMode;
                        phyData.serdesParams_ARR[i].speed = (MPD_SPEED_ENT)comphyTxParamsList_PTR->list_keys.speed;
                        phyData.serdesParams_ARR[i].serdesTuneParams.hostOrLineSide = (MPD_PHY_HOST_LINE_ENT)comphyTxParamsList_PTR->list_keys.side;
                        phyData.serdesParams_ARR[i].serdesTuneParams.tuneParams.paramsType = MPD_SERDES_PARAMS_TX_E;
                        phyData.serdesParams_ARR[i].serdesTuneParams.tuneParams.txTune.comphy.preTap = comphyTxParamsList_PTR->comphyTxParams.pretap;
                        phyData.serdesParams_ARR[i].serdesTuneParams.tuneParams.txTune.comphy.mainTap = comphyTxParamsList_PTR->comphyTxParams.maintap;
                        phyData.serdesParams_ARR[i].serdesTuneParams.tuneParams.txTune.comphy.postTap = comphyTxParamsList_PTR->comphyTxParams.posttap;
                        i++;
                        pdlStatus3 = prvPdlibDbGetNext(portList_PTR->comphyTxParamsType.comphyTxParamsList_PTR, (void *)&comphyTxParamsList_PTR->list_keys, (void **)&comphyTxParamsList_PTR);
                    }
                    phyData.serdesParams_ARR[numOfTxParams].serdesTuneParams.hostOrLineSide = MPD_PHY_SIDE_LAST_E;
                }
                else if (mpdIsFieldHasValue(portList_PTR->comphyVoltronTxParamsType_mask)){
                    pdlStatus3 = prvPdlibDbGetFirst(portList_PTR->comphyVoltronTxParamsType.comphyVoltronTxParamsList_PTR, (void **)& comphyVoltronTxParamsList_PTR);
                    while (pdlStatus3 == PDL_OK){
                    	prvMpdConvertXMLOpModeToMpd(comphyVoltronTxParamsList_PTR->list_keys.opConnected, comphyVoltronTxParamsList_PTR->list_keys.speed, &opMode);
                        phyData.serdesParams_ARR[i].opMode = opMode;
                        phyData.serdesParams_ARR[i].speed = (MPD_SPEED_ENT)comphyVoltronTxParamsList_PTR->list_keys.speed;
                        phyData.serdesParams_ARR[i].serdesTuneParams.hostOrLineSide = (MPD_PHY_HOST_LINE_ENT)comphyVoltronTxParamsList_PTR->list_keys.side;
                        phyData.serdesParams_ARR[i].serdesTuneParams.tuneParams.paramsType = MPD_SERDES_PARAMS_TX_E;
                        phyData.serdesParams_ARR[i].serdesTuneParams.tuneParams.txTune.comphy_28G.pre = comphyVoltronTxParamsList_PTR->voltronTxParams.pretap;
                        phyData.serdesParams_ARR[i].serdesTuneParams.tuneParams.txTune.comphy_28G.peak = comphyVoltronTxParamsList_PTR->voltronTxParams.peaktap;
                        phyData.serdesParams_ARR[i].serdesTuneParams.tuneParams.txTune.comphy_28G.post = comphyVoltronTxParamsList_PTR->voltronTxParams.posttap;
                        phyData.serdesParams_ARR[i].serdesTuneParams.tuneParams.txTune.comphy_28G.margin = comphyVoltronTxParamsList_PTR->voltronTxParams.margintap;
                        i++;
                        pdlStatus3 = prvPdlibDbGetNext(portList_PTR->comphyVoltronTxParamsType.comphyVoltronTxParamsList_PTR, (void *)&comphyVoltronTxParamsList_PTR->list_keys, (void **)&comphyVoltronTxParamsList_PTR);
                    }
                    phyData.serdesParams_ARR[numOfTxParams].serdesTuneParams.hostOrLineSide = MPD_PHY_SIDE_LAST_E;
                }
            }

            if (mpdIsFieldHasValue(portList_PTR->invertMdi_mask)){
                phyData.invertMdi = portList_PTR->invertMdi;
            }
            else {
                phyData.invertMdi = FALSE;
            }
            if (mpdIsFieldHasValue(portList_PTR->vctOffset_mask)){
                phyData.vctOffset = portList_PTR->vctOffset;
            }

            if (mpdPortDbUpdate(rel_ifIndex, &phyData) != MPD_OK_E) {
                return MPD_OP_FAILED_E;
            }
            /* getting the next port db on the phy*/
            pdlStatus2 = prvPdlibDbGetNext(phyList_PTR->portList_PTR, (void *)&portList_PTR->list_keys, (void **)&portList_PTR);
        }
        /* getting the next phy db*/
        pdlStatus = prvPdlibDbGetNext(mpdParams.phyList_PTR, (void *)&phyList_PTR->list_keys, (void **)&phyList_PTR);
    }

    xmlParserDestroy(&(PRV_MPD_NONSHARED_GLBVAR_MPDUTILS_XMLROOT_VOID_ACCESS_MAC()));

    return MPD_OK_E;
}

/*$ END OF mpdPortDbUpdateFromXml */
#else
extern MPD_RESULT_ENT mpdPortDbUpdateFromXml(
    IN char * xmlFilePathPtr,
    IN char * signatureFilePathPtr
)
{
    MPD_UNUSED_PARAM(xmlFilePathPtr);
    MPD_UNUSED_PARAM(signatureFilePathPtr);
    return MPD_NOT_SUPPORTED_E;
}
#endif

/* ***************************************************************************
* FUNCTION NAME: prvMpdGetPortEntry
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdGetPortEntry(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    return  prvMpdCheckIsPortValid(rel_ifIndex);
}
/* END OF prvMpdGetPortEntry */

/* ***************************************************************************
* FUNCTION NAME: prvMpdFindPortEntry
*
* DESCRIPTION:
*
*
*****************************************************************************/

extern PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdFindPortEntry(
    /*     INPUTS:             */
    PRV_MPD_APP_DATA_STC    * app_data_PTR,
    UINT_16                 mdio_address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_32                         rel_ifIndex, devIdx = 0;
    PRV_MPD_PORT_HASH_ENTRY_STC   * port_entry_PTR;

    if (mdio_address > PRV_MPD_MAX_MDIO_ADRESS_CNS) {
        return NULL;
    }

    rel_ifIndex = PRV_MPD_SHARED_GLBVAR_PORTENTRY_ASSIST_ACCESS_MAC()[app_data_PTR->mdioDev][app_data_PTR->mdioBus][mdio_address];
    if (rel_ifIndex) {
        return prvMpdGetPortEntry(rel_ifIndex);
    }
    for (rel_ifIndex = 0 ; prvMpdPortListGetNext(&(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->validPorts), &rel_ifIndex);) {
        port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
        if (port_entry_PTR != NULL) {
            devIdx = port_entry_PTR->initData_PTR->mdioInfo.mdioDev % PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS;
            if (devIdx == app_data_PTR->mdioDev &&
                port_entry_PTR->initData_PTR->mdioInfo.mdioBus == app_data_PTR->mdioBus &&
                port_entry_PTR->initData_PTR->phyType == app_data_PTR->phyType &&
                port_entry_PTR->initData_PTR->mdioInfo.mdioAddress == mdio_address) {

                PRV_MPD_SHARED_GLBVAR_PORTENTRY_ASSIST_ACCESS_MAC()[app_data_PTR->mdioDev][app_data_PTR->mdioBus][mdio_address] = rel_ifIndex;
                return port_entry_PTR;
            }
        }
    }

    return NULL;
}


/* ***************************************************************************
* FUNCTION NAME: prvMpdCalloc
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern void* prvMpdCalloc(
    /*     INPUTS:             */
    UINT_32                 numOfObjects,
    UINT_32                 len
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    void        * data_PTR;
    UINT_32     size = numOfObjects*len;

    data_PTR = PRV_MPD_ALLOC_MAC(size);
    if (data_PTR) {
        memset(data_PTR, 0, size);
    }
    return data_PTR;
}
/* END OF prvMpdCalloc */


/* ***************************************************************************
 * FUNCTION NAME: prvMpdMdioReadRegisterNoPage
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMdioReadRegisterNoPage(
    /*     INPUTS:             */
    const char *calling_func_PTR,
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_16 *value_PTR
)
{
    return prvMpdMdioReadRegister(calling_func_PTR,
                                  portEntry_PTR,
                                  PRV_MPD_IGNORE_PAGE_CNS,
                                  address,
                                  value_PTR);
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdXsmiReadRegister
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMdioReadRegister(
    /*     INPUTS:             */
    const char *calling_func_PTR,
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 deviceOrPage,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_16 *value_PTR
)
{
    UINT_16 read_value = 0;
    UINT_32 retry_counter = 10;
    MPD_RESULT_ENT status = MPD_OP_FAILED_E;
    MPD_OPERATIONS_PARAMS_UNT phy_params;

    while (status != MPD_OK_E && (retry_counter--) > 0) {
        *value_PTR = 0;

        /* check if phy type support page select register and we are called with valid page number */
        if ((deviceOrPage != PRV_MPD_IGNORE_PAGE_CNS) && (portEntry_PTR->initData_PTR->isOob == FALSE) && (prvMpdGetPhyOperationName(portEntry_PTR, PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E) != NULL)) {
            /* moving to new page */
            phy_params.internal.phyPageSelect.page = deviceOrPage;
            phy_params.internal.phyPageSelect.readPrevPage = TRUE;
            status = prvMpdPerformPhyOperation(portEntry_PTR,
                                               PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                               &phy_params);
            if (status == MPD_OP_FAILED_E) {
#ifndef PHY_SIMULATION
                PRV_MPD_SLEEP_MAC((10));
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                              "%s. rel_ifIndex %d. new page %d. failed with status 0x%x",
                                                              calling_func_PTR,
                                                              portEntry_PTR->rel_ifIndex,
                                                              phy_params.internal.phyPageSelect.page,
                                                              status);
                continue;
#endif
            }
            /* read register */
            status = PRV_MPD_MDIO_READ_MAC(portEntry_PTR->rel_ifIndex,
                                           PRV_MPD_IGNORE_PAGE_CNS,
                                           address,
                                           &read_value);
            if (status == MPD_OK_E) {
                /* moving to old page */
                phy_params.internal.phyPageSelect.page = phy_params.internal.phyPageSelect.prevPage;
                phy_params.internal.phyPageSelect.readPrevPage = FALSE;
                status = prvMpdPerformPhyOperation(portEntry_PTR,
                                                   PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                                   &phy_params);
                if (status == MPD_OP_FAILED_E) {
#ifndef PHY_SIMULATION
                    PRV_MPD_SLEEP_MAC((10));
                    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                  "%s. rel_ifIndex %d. old page %d. failed with status 0x%x",
                                                                  calling_func_PTR,
                                                                  portEntry_PTR->rel_ifIndex,
                                                                  phy_params.internal.phyPageSelect.page,
                                                                  status);
                    continue;
#endif
                }
            } else {
#ifndef PHY_SIMULATION
				PRV_MPD_SLEEP_MAC((10));
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															  "%s. rel_ifIndex %d. Read Failed with status 0x%x",
															  calling_func_PTR,
															  portEntry_PTR->rel_ifIndex,
															  status);
				continue;
#endif
            }
        }
        else {
            status = PRV_MPD_MDIO_READ_MAC(portEntry_PTR->rel_ifIndex,
                                           deviceOrPage,
                                           address,
                                           &read_value);
        }

        *value_PTR = read_value;

        if (status != MPD_OK_E) {
#ifndef PHY_SIMULATION
            PRV_MPD_SLEEP_MAC((10));
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "%s. reading from portEntry_PTR->rel_ifIndex %d failed with status  0x%x",
                                                          calling_func_PTR,
                                                          portEntry_PTR->rel_ifIndex,
                                                          status);
#endif
        }
        else {
        	if (deviceOrPage != PRV_MPD_IGNORE_PAGE_CNS) {
				PRV_MPD_DEBUG_LOG_PORT_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_READ_ACCESS_MAC(), portEntry_PTR->rel_ifIndex)(calling_func_PTR,
																							  "[R] rel_ifIndex [%0d] deviceOrPage [%02d] address [0x%04x] value [0x%04x]",
																							  portEntry_PTR->rel_ifIndex,
																							  deviceOrPage,
																							  address,
																							  read_value);
        	}
        	else {
				PRV_MPD_DEBUG_LOG_PORT_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_READ_ACCESS_MAC(), portEntry_PTR->rel_ifIndex)(calling_func_PTR,
																							  "[R] rel_ifIndex [%0d] address [0x%04x] value [0x%04x]",
																							  portEntry_PTR->rel_ifIndex,
																							  address,
																							  read_value);
        	}
        }
    }
#ifdef PHY_SIMULATION
    return MPD_OK_E;
#else
    return status;
#endif
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMdioWriteRegisterNoPage
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMdioWriteRegisterNoPage(
    /*     INPUTS:             */
    const char *calling_func_PTR,
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    return prvMpdMdioWriteRegister(calling_func_PTR,
                                   portEntry_PTR,
                                   PRV_MPD_IGNORE_PAGE_CNS,
                                   address,
                                   PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                   value);
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMdioWriteRegister
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMdioWriteRegister(
    /*     INPUTS:             */
    const char *calling_func_PTR,
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 device,
    UINT_16 address,
    UINT_16 mask,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_16 read_value = 0, write_value;
    MPD_RESULT_ENT status = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT phy_params;

    if (mask != PRV_MPD_MDIO_WRITE_ALL_MASK_CNS) {
		PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_WRITE_ACCESS_MAC())(calling_func_PTR,
													  "[R&W] rel_ifIndex [%0d] deviceOrPage [%02d] address [0x%04x] mask [0x%04x] value [0x%04x]",
													  portEntry_PTR->rel_ifIndex,
													  device,
													  address,
													  mask,
													  value);

        status = prvMpdMdioReadRegister(calling_func_PTR,
                                        portEntry_PTR,
                                        device,
                                        address,
                                        &read_value);
        if (status != MPD_OK_E) {
#ifndef PHY_SIMULATION
            PRV_MPD_HANDLE_FAILURE_MAC(portEntry_PTR->rel_ifIndex,
                                       MPD_ERROR_SEVERITY_FATAL_E,
                                       PRV_MPD_DEBUG_FUNC_NAME_MAC());
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "read failed with status 0x%x",
                                                          status);
            return status;
#endif
        }
        write_value = (~mask & read_value) | (mask & value);
        /* no need to configure*/
        if (write_value == read_value) {
        	PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(calling_func_PTR,
        														  "[SKIP] rel_ifIndex [%0d] deviceOrPage [%02d] address [0x%04x] mask [0x%04x] value [0x%04x]",
        														  portEntry_PTR->rel_ifIndex,
        														  device,
        														  address,
        														  mask,
        														  value);
            return MPD_OK_E;
        }
    }
    else {
        write_value = value;
		PRV_MPD_DEBUG_LOG_PORT_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_WRITE_ACCESS_MAC(), portEntry_PTR->rel_ifIndex)(calling_func_PTR,
				"[W] rel_ifIndex [%0d] deviceOrPage [%02d] address [0x%04x] value [0x%04x]",
				portEntry_PTR->rel_ifIndex,
				device,
				address,
				write_value);
    }
    /* write value */
    /* check if phy type support page select register and we are called with valid page number */
    if ((device != PRV_MPD_IGNORE_PAGE_CNS) && (portEntry_PTR->initData_PTR->isOob == FALSE) && (prvMpdGetPhyOperationName(portEntry_PTR, PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E) != NULL)) {
        /* moving to new page */
        phy_params.internal.phyPageSelect.page = device;
        phy_params.internal.phyPageSelect.readPrevPage = TRUE;
        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                           &phy_params);
        if (status == MPD_OP_FAILED_E) {
#ifndef PHY_SIMULATION
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "%s. rel_ifIndex %d. new page %d. failed with status 0x%x",
                                                          calling_func_PTR,
                                                          portEntry_PTR->rel_ifIndex,
                                                          phy_params.internal.phyPageSelect.page,
                                                          status);
            return status;
#endif
        }
        /* write register */
        status = PRV_MPD_MDIO_WRITE_MAC(portEntry_PTR->rel_ifIndex,
                                        device,
                                        address,
                                        write_value);
        if (status == MPD_OK_E) {
            /* moving to old page */
            phy_params.internal.phyPageSelect.page = phy_params.internal.phyPageSelect.prevPage;
            phy_params.internal.phyPageSelect.readPrevPage = FALSE;
            status = prvMpdPerformPhyOperation(portEntry_PTR,
                                               PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                               &phy_params);
            if (status == MPD_OP_FAILED_E) {
#ifndef PHY_SIMULATION
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                              "%s. rel_ifIndex %d. old page %d. failed with status 0x%x",
                                                              calling_func_PTR,
                                                              portEntry_PTR->rel_ifIndex,
                                                              phy_params.internal.phyPageSelect.page,
                                                              status);
                return status;
#endif
            }
        }
    }
    else {
        status = PRV_MPD_MDIO_WRITE_MAC(portEntry_PTR->rel_ifIndex,
                                        device,
                                        address,
                                        write_value);
    }
    if (status != MPD_OK_E) {
#ifndef PHY_SIMULATION
    	PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							  "%s. rel_ifIndex %d. Write Failed with status 0x%x",
							  calling_func_PTR,
							  portEntry_PTR->rel_ifIndex,
							  status);
        PRV_MPD_HANDLE_FAILURE_MAC(portEntry_PTR->rel_ifIndex,
                                   MPD_ERROR_SEVERITY_FATAL_E,
                                   PRV_MPD_DEBUG_FUNC_NAME_MAC());
#endif
    }
#ifdef PHY_SIMULATION
    return MPD_OK_E;
#else
    return status;
#endif
}
static UINT_16 prvMpdSliceNumToOffset_ARR[PRV_MPD_MAX_NUM_OF_SLICES_CNS] = {
    PRV_MPD_1540_SLICE_0_OFFSET_CNS,
    PRV_MPD_1540_SLICE_1_OFFSET_CNS,
    PRV_MPD_1540_SLICE_2_OFFSET_CNS,
    PRV_MPD_1540_SLICE_3_OFFSET_CNS
};

/* ***************************************************************************
 * FUNCTION NAME: prvMpdLinkCryptGetActualAddress
 *
 * DESCRIPTION: indirect access of linkcrypt register is accessing a shared (all ports in quad)
 *  RAM area, each port in quad has it's addressing offset which is defined at prvMpdSliceNumToOffset_ARR
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdLinkCryptGetActualAddress(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    UINT_16 *address_PTR
    /*     OUTPUTS:            */
)
{
    UINT_32 slice = 0;
    UINT_16 offset;

    slice = portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 4;
    offset = prvMpdSliceNumToOffset_ARR[slice];
    *address_PTR = *address_PTR + offset;
    return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdReadLinkCryptReg
 *
 * DESCRIPTION: indirect access to linkCrypt registers, 1540M
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdReadLinkCryptReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_32 *data_PTR
)
{
    MPD_RESULT_ENT status;
    UINT_16 data_high = 0, data_low = 0;
    prvMpdLinkCryptGetActualAddress(portEntry_PTR,
                                    &address);

    /* switch to page 16*/
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                           PRV_MPD_LINKCRYPT_INDIRECT_ACCESS_PAGE_CNS);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* select address */
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_LINKCRYPT_READ_ADDRESS_REG_CNS,
                                           address);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* read data low */
    status = prvMpdMdioReadRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                          portEntry_PTR,
                                          PRV_MPD_LINKCRYPT_DATA_LOW_REG_CNS,
                                          &data_low);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* read data high */
    status = prvMpdMdioReadRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                          portEntry_PTR,
                                          PRV_MPD_LINKCRYPT_DATA_HIGH_REG_CNS,
                                          &data_high);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    *data_PTR = (data_high << 16) | (data_low & 0xFFFF);
    /* write back old page */
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                           0);
    return status;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdWriteLinkCryptRegMask
 *
 * DESCRIPTION: write linkcrypt register using given mask, 1540M
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdWriteLinkCryptRegMask(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 address,
    UINT_32 data,
    UINT_32 mask
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT status;
    UINT_32 write_value, old_data;
    UINT_16 data_high = 0, data_low = 0;
    prvMpdLinkCryptGetActualAddress(portEntry_PTR,
                                    &address);
    /* switch to page 16*/
    write_value = data;
    if (mask != 0XFFFFFFFF) {
        prvMpdReadLinkCryptReg(portEntry_PTR,
                               address,
                               &old_data);
        write_value = (~mask & old_data) | (mask & data);
        if (write_value == old_data) {
            return MPD_OK_E;
        }
    }
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                           PRV_MPD_LINKCRYPT_INDIRECT_ACCESS_PAGE_CNS);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* select address */
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_LINKCRYPT_WRITE_ADDRESS_REG_CNS,
                                           address);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    data_high = (write_value >> 16);
    data_low = write_value & 0xFFFF;
    /* write data low */
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_LINKCRYPT_DATA_LOW_REG_CNS,
                                           data_low);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* write data high */
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_LINKCRYPT_DATA_HIGH_REG_CNS,
                                           data_high);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* write back old page */
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                           0);
    return status;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdWriteLinkCryptRegMask
 *
 * DESCRIPTION: indirect access to linkCrypt registers
 *
 *
 *****************************************************************************/

GT_STATUS prvMpdWriteLinkCryptReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 address,
    UINT_32 data
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    return prvMpdWriteLinkCryptRegMask(portEntry_PTR,
                                       address,
                                       data,
                                       0xFFFFFFFF);
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetTxSerdesParams
 *
 * DESCRIPTION: get tx serdes params to be configure
 *
 *
 *****************************************************************************/

BOOLEAN prvMpdGetTxSerdesParams(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_PHY_HOST_LINE_ENT hostOrLineSide,
    MPD_SPEED_ENT  phySpeed,
    MPD_SERDES_TUNE_PARAMS_STC serdesTuneParams,
	/*     INPUTS / OUTPUTS:   */
	MPD_SERDES_TX_CONFIG_UNT * phyTxParams_PTR
    /*     OUTPUTS:            */
)
{
    MPD_SERDES_CONFIG_PARAMS_STC      *txSerdesParams_ARR;
    UINT_32 i = 0;
    BOOLEAN matchFound = FALSE;

    if (portEntry_PTR->runningData_PTR->isNotImmidiateTxConfigure_ARR[hostOrLineSide]){
        /* Not immidiate config only once during init sequence */
        return FALSE;
    }

    if (serdesTuneParams.tuneParams.paramsType == MPD_SERDES_PARAMS_SET_FROM_XML_E){
        if (portEntry_PTR->initData_PTR->serdesParams_ARR != NULL){
            txSerdesParams_ARR = portEntry_PTR->initData_PTR->serdesParams_ARR;
            while ((txSerdesParams_ARR) && (txSerdesParams_ARR[i].serdesTuneParams.hostOrLineSide != MPD_PHY_SIDE_LAST_E)) {
                if ((txSerdesParams_ARR[i].serdesTuneParams.tuneParams.paramsType == MPD_SERDES_PARAMS_TX_E) &&
                   (txSerdesParams_ARR[i].serdesTuneParams.hostOrLineSide == hostOrLineSide)){
                    if (txSerdesParams_ARR[i].serdesTuneParams.hostOrLineSide == hostOrLineSide){
                        if ((txSerdesParams_ARR[i].speed == MPD_SPEED_LAST_E) && (txSerdesParams_ARR[i].opMode == MPD_OP_MODE_LAST_E)) {
                            portEntry_PTR->runningData_PTR->isNotImmidiateTxConfigure_ARR[hostOrLineSide] = TRUE;
                            matchFound = TRUE;
                        }
                        else if (txSerdesParams_ARR[i].speed == phySpeed) {
                        	if (txSerdesParams_ARR[i].opMode == portEntry_PTR->runningData_PTR->opMode){
                        		matchFound = TRUE;
                        	/* patch to identify both fiber to copper modes - SGMII & 1000Base-X */
                        	} else if ((txSerdesParams_ARR[i].opMode == MPD_OP_MODE_COPPER_SFP_1000BASE_X_MODE_E) && (portEntry_PTR->runningData_PTR->opMode == MPD_OP_MODE_COPPER_SFP_SGMII_MODE_E)) {
                        		matchFound = TRUE;
                        	}
                        }
                        if (matchFound == TRUE) {
                        	memcpy (phyTxParams_PTR, &txSerdesParams_ARR[i].serdesTuneParams.tuneParams.txTune, sizeof(MPD_SERDES_TX_CONFIG_UNT));
							return TRUE;
                        }
                    }
                }
                i++;
            }
        }
    }
    else {
        phyTxParams_PTR->comphy.preTap = serdesTuneParams.tuneParams.txTune.comphy.preTap;
        phyTxParams_PTR->comphy.mainTap = serdesTuneParams.tuneParams.txTune.comphy.mainTap;
        phyTxParams_PTR->comphy.postTap = serdesTuneParams.tuneParams.txTune.comphy.postTap;
        if (serdesTuneParams.txConfig == MPD_SERDES_CONFIG_PERMANENT_E){
            portEntry_PTR->runningData_PTR->isNotImmidiateTxConfigure_ARR[hostOrLineSide] = TRUE;
        }
        return TRUE;
    }

    return FALSE;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdXmdioWriteReg
 *
 * DESCRIPTION: indirect access to XMDIO registers
 *
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdXmdioWriteReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_8 device,
    UINT_16 address,
    UINT_16 data
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT status;
    UINT_16 value;

    /* switch to page 0*/
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                           PRV_MPD_MMD_INDIRECT_ACCESS_PAGE_CNS);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* select address */
    value = (PRV_MPD_MMD_FUNCTION_ADDRESS_CNS | (PRV_MPD_MMD_DEVICE_MASK_CNS & device));
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS,
                                           value);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* write address  */
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_MMD_ADDRESS_DATA_REG_CNS,
                                           address);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* select data */
    value = (PRV_MPD_MMD_FUNCTION_DATA_CNS | (PRV_MPD_MMD_DEVICE_MASK_CNS & device));
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS,
                                           value);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* write data  */
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_MMD_ADDRESS_DATA_REG_CNS,
                                           data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdXmdioReadReg
 *
 * DESCRIPTION: indirect access to XMDIO registers
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdXmdioReadReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_8 device,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_16 *data_PTR
)
{
    MPD_RESULT_ENT status;
    UINT_16 value;

    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                           PRV_MPD_MMD_INDIRECT_ACCESS_PAGE_CNS);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* select address */
    value = (PRV_MPD_MMD_FUNCTION_ADDRESS_CNS | (PRV_MPD_MMD_DEVICE_MASK_CNS & device));
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS,
                                           value);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* write address  */
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_MMD_ADDRESS_DATA_REG_CNS,
                                           address);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* select data */
    value = (PRV_MPD_MMD_FUNCTION_DATA_CNS | (PRV_MPD_MMD_DEVICE_MASK_CNS & device));
    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS,
                                           value);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    /* read data  */
    status = prvMpdMdioReadRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                          portEntry_PTR,
                                          PRV_MPD_MMD_ADDRESS_DATA_REG_CNS,
                                          data_PTR);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    return MPD_OK_E;
}

extern MPD_RESULT_ENT prvMpdMdioWrapRead(
    IN  UINT_32      					rel_ifIndex,
	IN  PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    IN  UINT_16      					deviceOrPage,
    IN  UINT_16      					address,
    OUT UINT_16    					  * value_PTR
)
{
    UINT_8	mdioAddress;
    
    if (portEntry_PTR == NULL) {
        return MPD_OP_FAILED_E;
    }

    if (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == FALSE) {
        return MPD_OP_FAILED_E;
    }
    if (portEntry_PTR == NULL) {
        return MPD_OP_FAILED_E;
    }
    if (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().mdioRead_PTR == NULL && portEntry_PTR->initData_PTR->mdioInfo.readFunc_PTR == NULL) {
        return MPD_OP_FAILED_E;
    }
    mdioAddress = portEntry_PTR->initData_PTR->mdioInfo.mdioAddress;
    if (portEntry_PTR->initData_PTR->mdioInfo.readFunc_PTR) {
        return portEntry_PTR->initData_PTR->mdioInfo.readFunc_PTR(rel_ifIndex, mdioAddress, deviceOrPage, address, value_PTR)?MPD_OK_E:MPD_OP_FAILED_E;
    }
    return PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().mdioRead_PTR(rel_ifIndex, mdioAddress, deviceOrPage, address, value_PTR)?MPD_OK_E:MPD_OP_FAILED_E;

}

extern MPD_RESULT_ENT prvMpdMdioWrapWrite(
    IN  UINT_32      					rel_ifIndex,
	IN  PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    IN  UINT_16      					deviceOrPage,
    IN  UINT_16      					address,
    IN  UINT_16      					value
)
{
    UINT_8       mdioAddress;

    if (portEntry_PTR == NULL) {
        return MPD_OP_FAILED_E;
    }
    if (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == FALSE) {
        return MPD_OP_FAILED_E;
    }
    if (portEntry_PTR == NULL) {
        return MPD_OP_FAILED_E;
    }
    if (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().mdioWrite_PTR == NULL && portEntry_PTR->initData_PTR->mdioInfo.writeFunc_PTR == NULL) {
        return MPD_OP_FAILED_E;
    }

    mdioAddress = portEntry_PTR->initData_PTR->mdioInfo.mdioAddress;

    if (portEntry_PTR->initData_PTR->mdioInfo.writeFunc_PTR) {
        return portEntry_PTR->initData_PTR->mdioInfo.writeFunc_PTR(rel_ifIndex, mdioAddress, deviceOrPage, address, value)?MPD_OK_E:MPD_OP_FAILED_E;
    }
    return PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().mdioWrite_PTR(rel_ifIndex, mdioAddress, deviceOrPage, address, value)?MPD_OK_E:MPD_OP_FAILED_E;

}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdSnprintf
 *
 * DESCRIPTION: Implement the native snprintf
 *
 *
 *****************************************************************************/

int prvMpdSnprintf (
	/*!     INPUTS:             */
	char                *str,
	UINT_32              size,
	const char          * format,
	...
	/*!     OUTPUTS:            */
)
{
	UINT_32       needed_buffer_len = 0;
	char      buffer[MPD_MAXSTR_CNS];
	va_list   argptr;

	va_start( argptr, format);
    needed_buffer_len = vsprintf(buffer, format, argptr);
	va_end( argptr);
	if (needed_buffer_len > MPD_MAXSTR_CNS) {
		PRV_MPD_HANDLE_FAILURE_MAC(0,
			MPD_ERROR_SEVERITY_ERROR_E,
			"prvMpdSnprintf - output length too long");
		return 0;
	}
	/* also copy the '\0' at end of buffer to str */
	strncpy (str, buffer, MIN(size, needed_buffer_len+1));
	if (needed_buffer_len + 1 > size) {
		str[size-1] = '\0';
	}
	return needed_buffer_len;
}

/*$ END OF OSIOG_snprintf */

/* ***************************************************************************
 * FUNCTION NAME: mpdRenumberDevNum
 *
 * DESCRIPTION: renumber DB device number. should be used in auto number when cpssDxChCfgReNumberDevNum is used
 *
 *
 *****************************************************************************/

extern MPD_RESULT_ENT mpdRenumberDevNum (
	/*!     INPUTS:             */
    UINT_8     oldDevNum,
    UINT_8     newDevNum
	/*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32                     rel_ifIndex;
    BOOLEAN                     found = FALSE;
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/
    for (rel_ifIndex = 0 ; prvMpdPortListGetNext(&(PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->validPorts), &rel_ifIndex);) {
        portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
        if (portEntry_PTR != NULL) {
            if (portEntry_PTR->initData_PTR->mdioInfo.mdioDev == oldDevNum) {
                portEntry_PTR->initData_PTR->mdioInfo.mdioDev = newDevNum;
                found = TRUE;
            }
        }
    }
    if  (found == FALSE) {
        return MPD_NOT_FOUND_E;
    }
    return MPD_OK_E;
}

/*$ END OF mpdRenumberDevNum */
