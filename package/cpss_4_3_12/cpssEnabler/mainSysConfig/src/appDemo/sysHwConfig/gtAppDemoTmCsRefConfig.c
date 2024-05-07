/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtAppDemoTmCsRefConfig.c
*
* @brief Initialization functions for the TM.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/sysHwConfig/gtAppDemoTmCsRefConfig.h>

#include <cpss/generic/tm/cpssTmPublicDefs.h>
#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesCtl.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesUpdate.h>
#include <cpss/generic/tm/cpssTmNodesStatus.h>
#include <cpss/generic/tm/cpssTmNodesRead.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDrop.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManagerTypes.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManager.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/generic/tm/prvCpssTmCtl.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>

/*
    Global variables for the example
    Virtual TCAM manager and PCL global variables
*/
#define MaxNumOfFlowPerQueue      5 /* how many S-Tag and C-Tag are mapped to chunk of queeus */
#define CS_REF_TTI_KEY_SIZE 30
#define IPCL_60_UDB 20
#define NUM_OF_UDB 60
#define VTCAM_NUM_OF_UDB 10


#define MaxNumOfFlowPerQueue     5 /* how many S-Tag and C-Tag are mapped to chunk of queeus */
#define MaxNumOfFlowPerAnode     5 /* how many S-Tag and C-Tag are mapped to chunk of queeus */
#define MaxNumOfTmPorts          17 /* how many S-Tag and C-Tag are mapped to chunk of queeus */
#define CAELUM_EPCL_ID           100
#define VTCAM_PCL_MNG_ID         1
#define VTCAM_ID                 1
#define maxNumOf8Qchunks         512 /* 4K : 8 */
#define maxNumOfvTCamRuleId      2304 /* 18K-1 / 8 for BobK */

/***************************** TM tree structures  *************************/

/* A flow is defined by TAG0 TAG1 and 8 UPs [0..7] */
/* Each flow is associated with 8 vTACMrules       */
typedef struct
{
    GT_U16              tag0sTag        ;
    GT_U16              tag1cTag        ;
    GT_U32              up[8]           ;
    GT_U32              priority        ; /* the priority in the rule in the vTCAM block Deafult flow has priority = 16*/
    GT_U32              queueNum[8]     ;
    GT_U32              vTcamRuleId[8]  ;
}appDemoFlowStruct  ;

/* the 8 flows IDs are assocated with A-Node and 8 queues */
/* Since there might be more than a flow per A-Node need  */
/* to keep the number of flows associated with this A-Node*/
typedef struct
{
    GT_U32               aNode                              ; /* 8 A-Nodes */
     GT_U32              numOfFlowsperAnode                 ;
    appDemoFlowStruct     aNodeAttr[MaxNumOfFlowPerAnode]    ;
}appDemoANodePath  ;


typedef struct
{
    GT_U32              cNode        ;
    GT_U32              bNode        ;
    appDemoANodePath     flow2aNode[8]; /* data structure holding the flow attributes per every aNode in the Port-C-B path */
}appDemoFullPathAttr  ;

typedef struct
{
    GT_PORT_NUM           egressPort      ;
    appDemoFullPathAttr    cNodeTree[8]    ;
}appDemoEpcl2qmapStruct ;

typedef struct
{
    GT_U32                            udbIndex;        /* UDB index at PCL key */
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT     pclOffsetType; /* UDB offset type (anchor) */
    GT_U8                             pclOffset;     /* UDB offset from anchor */
    GT_U8                             udbMask;        /* Mask for EM */
    GT_CHAR                           UDBstr[100]    ;
} appDemoPclUdbStruct;

static appDemoEpcl2qmapStruct appDemoEgressPortDb[MaxNumOfTmPorts];



/***************************** TCAM and EPCL structures  *************************/

static CPSS_PCL_DIRECTION_ENT               direction          = CPSS_PCL_DIRECTION_INGRESS_E                    ;
static CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum          = CPSS_PCL_LOOKUP_0_E                            ;

static appDemoPclUdbStruct  appDemoIpclUdb[60]=
{
/* CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E */
/* Must check the UDB offset per device */
/*    0 */ { 30, CPSS_DXCH_PCL_OFFSET_METADATA_E   ,  10,0xff ,"Local Dev Src Port LSB                    10 [7:0]"},
/*    1 */ { 31,CPSS_DXCH_PCL_OFFSET_METADATA_E    , 108,0xff ,"Local Dev Src Port Ext MSB               108 [9:8]"},
/*    2 */ { 32,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  28,0xff ,"Orig VID MSB                               28 [7:0]"},
/*    3 */ { 33,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  27,0xff ,"Orig VID LSB                               27 [3:0]"},
/*    4 */ { 34,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  21,0xff ,"Orig Src Phy Port/Trunk ID LSB            21 [7:2]"},
/*    5 */ { 35,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  22,0xff ,"Orig Src Phy Port/Trunk ID MSB            22 [5:0]"},
/*    6 */ { 36,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  90,0xff ,"Packet Type [0-Other 4-MPLS 6-IPv6 7-IPv4 90 [5:3]"},
/*    7 */ { 37,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  41,0xff ,"IsIp[1:1 isIpv4 2:2 isIpv6 3:3 isArp 5:5  41 [   ]"},
/*    8 */ { 38,CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E  ,   0,0xff ,"EtherType MSB                                0 [7:0]"},
/*    9 */ { 39,CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E  ,   1,0xff ,"EtherType LSB                                1 [7:0]"},
/* 10 */ { 40,CPSS_DXCH_PCL_OFFSET_METADATA_E      ,  23,0xff ,"Orig VID LSB                               27 [3:0]"},
/* 11 */ { 41,CPSS_DXCH_PCL_OFFSET_METADATA_E      ,  21,0xff ,"PCLID[4:0]                                21 [4:0]"},
/* 12 */ { 42,CPSS_DXCH_PCL_OFFSET_METADATA_E      ,  22,0xff ,"PCLID[4:0]                                22 [4:0]"},
};

/* extructed from BobK FC Only UDBs 30-49 may be configured with metadata anchor type. */
static appDemoPclUdbStruct  appDemoEpclUdb[]=
{
/*    */
/* Must check the UDB offset per device for Caelum*/
/*    0 */ { 30,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  12,0x00 ,"Local Dev Trg PHY Port                   12 [7:0]"},
/*    1 */ { 31,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  26,0xFF ,"eVlan LSB                                26 [7:0]"},
/*    2 */ { 32,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  27,0x1F ,"eVlan MSB                                27 [4:0]"},
/*    3 */ { 33,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  29,0xFF ,"Tag1 VID LSB                             29 [7:0]"},
/*    4 */ { 34,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  30,0x0F ,"Tag1 VID MSB                             30 [3:0]"},
/*    5 */ { 35,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  28,0x07 ,"UP0                                      28 [2:0]"},
/*    6 */ { 36,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  25,0xFF ,"Orig VID LSB                             25 [7:0]"},
/*    7 */ { 37,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  20,0x00 ,"TBD"},
/*    8 */ { 38,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  20,0x00 ,"TBD"},
/*    9 */ { 39,CPSS_DXCH_PCL_OFFSET_METADATA_E    ,  20,0x00 ,"TBD"}
};


/***************************** Port List DB *************************/
/* total number of ports that might be enable for TM */
static GT_U32      numOfTmPorts = MaxNumOfTmPorts;
static GT_PORT_NUM tmEnablePortList[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
static GT_PORT_NUM tmEnablePhysicalPortList[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

/* a pool of 512 blocks of 8 queeus each */
static GT_BOOL queuePool[512]; /* 4*1024/8 */
static GT_BOOL vTcamRuleIdPool[maxNumOfvTCamRuleId]; /* 18*1024/8 */

static GT_BOOL printOut = GT_FALSE;

/**
 * @internal appDemoTmPrintOutEnable function
 * @endinternal
 *
 * @brief  Eanble/Disable debug prints
 *
 * @note   APPLICABLE DEVICES:      Caelum.
 *
 * @param[in] enable - GT_TRUE/GT_FALSE to enable/disable
 *                     debug prints
 *
 * @retval GT_OK   -   on success
 */
GT_STATUS appDemoTmPrintOutEnable
(
    IN GT_BOOL enable
)
{
    printOut = enable;

    return GT_OK;
}

/**
* @internal appDemoTmNumOfTmPortSet function
* @endinternal
*
* @brief   This API set the total number of ports enabled for TM
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] numOfPorts - Number of ports used by TM
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoTmNumOfTmPortSet
(
    IN GT_U32      numOfPorts
)
{
    numOfTmPorts = numOfPorts;
    if (printOut)
    {
        cpssOsPrintf("enter appDemoTmNumOfTmPortSet \n");
        cpssOsPrintf("numOfTmPorts is set to = %d \n",numOfTmPorts);
    }
    return GT_OK;
}

/**
* @internal appDemoTmNumOfTmPortGet function
* @endinternal
*
* @brief   get the total number of ports enabled for TM
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @retval numOfPorts - number of ports used by TM
*/

/* get the total number of ports enabled for TM */
GT_U32 appDemoTmNumOfTmPortGet
(
    GT_VOID
)
{
    if (printOut)
    {
        cpssOsPrintf("enter appDemoTmNumOfTmPortGet \n");
        cpssOsPrintf("numOfTmPorts = %d \n",numOfTmPorts);
    }

    return numOfTmPorts;
}

/**
* @internal appDemoTmPortIndexSet function
* @endinternal
*
* @brief   This API set a port to be enable for TM in the port list array
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] portIndex    - port index
* @param[in] portNum      - port number
*
* @retval GT_OK                    - on success
*/

GT_STATUS appDemoTmPortIndexSet
(
    IN  GT_U32      portIndex   ,
    IN  GT_PORT_NUM portNum
)
{
    if (printOut)
        cpssOsPrintf("enter appDemoTmPortIndexSet \n");

    tmEnablePhysicalPortList[portIndex] = portNum;

    return GT_OK;
}

/**
* @internal appDemoTmPortIndexGet function
* @endinternal
*
* @brief   This API gets the index of the portNum from the array of ports
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] portNum      - port number
*
* @retval portIndex                - index of the port List
* @retval GT_NO_SUCH               - port number not found
*/

GT_U32 appDemoTmPortIndexGet
(
    IN  GT_U32      portNum
)
{
    GT_U32      ii;

    for(ii = 0; ii < numOfTmPorts; ii++)
    {
        if (tmEnablePhysicalPortList[ii] == portNum)
        {
            if (printOut)
                cpssOsPrintf("portNum %d is associated with index %d\n",portNum,ii);
            return ii;
        }
    }
    if (printOut)
        cpssOsPrintf("portNum %d was not found \n",portNum);

    return GT_NO_SUCH;
}


/***************************** Queues index Chunk DB *************************/

/**
* @internal appDemoTM8queuesPoolClear function
* @endinternal
*
* @brief   This API clear the queues DB (free all of them )
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @retval GT_OK                 - on success
*
*
*/
GT_STATUS appDemoTM8queuesPoolClear
(
    GT_VOID
)
{
    cpssOsMemSet(queuePool,0x0,sizeof(queuePool));
    return GT_OK;
}

/**
* @internal appDemoTM8queuesFree function
* @endinternal
*
* @brief   This API free a group of 8 queues
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] qNodeInd - the 1st queeu num in the chunk
*
* @retval    GT_OK    - on success
*
*
*/
GT_STATUS appDemoTM8queuesFree
(
    IN  GT_U32  qNodeInd /* the 1st queeu num in the chunk */
)
{
    queuePool[qNodeInd] = GT_FALSE  ;
    if(printOut)
        cpssOsPrintf("8 queues chunk is freed start with Q=%d\n",qNodeInd);
    return GT_OK;
}

/**
* @internal appDemoTM8queuesAllocate function
* @endinternal
*
* @brief   This API allocate a group of 8 queues
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @retval queueNum                 - the 1st queue number in a chunk of 8
*
*
*/
GT_U32 appDemoTM8queuesAllocate
(
    GT_VOID
)
{
    GT_U32 ii;

    if(printOut)
        cpssOsPrintf("\t\tenter appDemoTM8queuesAllocate\n");
    for(ii=0 ; ii < maxNumOf8Qchunks;ii++)
    {
        if(queuePool[ii] == GT_FALSE)
        {
            queuePool[ii] = GT_TRUE;
            if(printOut)
                cpssOsPrintf("8 queues chunk is allocated start with Q=%d\n",ii*8+1);
            return ii;
        }
    }
    return 0xffffffff;
}
/**
* @internal appDemoTM8queuesPoolDump function
* @endinternal
*
* @brief   This API dump the map of the queues DB
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @retval  GT_OK   -  on success
*
*
*/
GT_STATUS appDemoTM8queuesPoolDump
(
    GT_VOID
)
{
    GT_U32 ii;

    cpssOsPrintf("Queue \n");
    for(ii=0;ii<maxNumOf8Qchunks;ii++)
    {
        if((ii%24) == 0)
            cpssOsPrintf(" \n");
        if (queuePool[ii])
            cpssOsPrintf("  1");
        else
            cpssOsPrintf("  0");

    }
    cpssOsPrintf(" \n");
    return GT_OK;
}

/***************************** vTcam Rules DB *************************/
/**
* @internal appDemoTMePcl8RulesClear function
* @endinternal
*
* @brief   This API clear the vTCAM rules DB (free all of them )
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] devNum         - device number
*
* @retval   GT_OK - on success
*
*/

GT_STATUS appDemoTMePcl8RulesClear
(
    GT_VOID
)
{
    cpssOsMemSet(vTcamRuleIdPool,0x0,sizeof(vTcamRuleIdPool));
    return GT_OK;
}

/**
* @internal appDemoTMePcl8RulesFree function
* @endinternal
*
* @brief   This API free 8 vTCAM rules (single entry in the array)
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] vTcamRuleId    - index of 8 rules ID.
*                             The 1st queue num in the chunk of 8
*
* @retval GT_OK    - on success
*/
GT_STATUS appDemoTMePcl8RulesFree
(
    IN  GT_U32  vTcamRuleId
)
{
    if(vTcamRuleId >= maxNumOfvTCamRuleId)
        return GT_BAD_PARAM;

    vTcamRuleIdPool[vTcamRuleId] = GT_FALSE ;
    if(printOut)
        cpssOsPrintf("8 queues chunk is freed start with Q=%d\n",vTcamRuleId+1);
    return GT_OK;
}

/**
* @internal appDemoTMePcl8RulesAllocate function
* @endinternal
*
* @brief   This API allocate 8 vTCAM rules (single entry in the array)
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @retval  GT_OK    - on success
*
*/
GT_U32 appDemoTMePcl8RulesAllocate
(
    GT_VOID
)
{
    GT_U32 ii;

    for(ii=0; ii < maxNumOfvTCamRuleId;ii++)
    {
        if(vTcamRuleIdPool[ii] == GT_FALSE)
        {
            vTcamRuleIdPool[ii] = GT_TRUE;
            if(printOut)
                cpssOsPrintf("vTcamRuleId chunk is allocated start with ruleId=%d\n",ii+1);
            return ii;
        }
    }
    return 0xffffffff;
}

/**
* @internal appDemoTMePcl8RulesDump function
* @endinternal
*
* @brief   This API dump vTcam rules DB
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] startIndex         - start index
* @param[in] numOfEntries       - number of entries
*
* @retval   GT_OK  - on success
*/
GT_STATUS appDemoTMePcl8RulesDump
(
    IN  GT_U32  startIndex,
    IN GT_U32   numOfEntries
)
{
    GT_U32 ii;

    cpssOsPrintf("vTcamRuleId \n");
    for(ii = startIndex; ii < (startIndex+numOfEntries);ii++)
    {
        if((ii%24) == 0)
            cpssOsPrintf(" \n");
        if (vTcamRuleIdPool[ii])
            cpssOsPrintf("  1");
        else
            cpssOsPrintf("  0");

    }
    cpssOsPrintf(" \n");
    return GT_OK;
}

/***************************** TM port related APIs *************************/
/**
* @internal appDemoEgressPortsDbClearAll function
* @endinternal
*
* @brief   This API clear the ports DB
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @retval   GT_OK  - on success
*
*/
/* Clear the DB for all ports */
GT_STATUS appDemoEgressPortsDbClearAll
(
    GT_VOID
)
{
    if(printOut)
        cpssOsPrintf("enter appDemoEgressPortsDbClearAll\n");

    cpssOsMemSet(&appDemoEgressPortDb,0x0,sizeof(appDemoEgressPortDb));
    /*for(portNumIndex = 0; portNumIndex < numOfTmPorts;portNumIndex++)
    {
        cpssOsMemSet(&appDemoEgressPortDb[portNumIndex],0x0,sizeof(appDemoEgressPortDb[portNumIndex]));
    }*/
    return GT_OK;
}

/**
* @internal appDemoEgressPortDbDump function
* @endinternal
*
* @brief   This API clear the ports DB
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] portNum        - a port Num (not the index )
*
* @retval    GT_OK      - on success
*/
GT_STATUS appDemoEgressPortDbDump
(
    GT_PORT_NUM portNum
)
{
    GT_U32      ii,jj,kk,mm=0   ;
    GT_U32      portNumIndex    ;

    if(printOut)
        cpssOsPrintf("enter appDemoEgressPortDbDump\n");

    portNumIndex = appDemoTmPortIndexGet(portNum);
    if( portNumIndex == 0xffffffff)
    {
        cpssOsPrintf("appDemoTmPortIndexGet failed portNUm = %d doesn't exist\n",portNum);
        return GT_BAD_PARAM;
    }

    cpssOsPrintf("Port Num %d\n",appDemoEgressPortDb[portNumIndex].egressPort);
    for (ii = 0 ; ii < 8 ; ii++) /* loop over C and  B Nodes */
    {
        cpssOsPrintf("\tC-Node ID %d\n",appDemoEgressPortDb[portNumIndex].cNodeTree[ii].cNode);
        cpssOsPrintf("\t\tB-Node ID %d\n",appDemoEgressPortDb[portNumIndex].cNodeTree[ii].bNode);
        for (jj = 0 ; jj < 8 ; jj++)/* loop over A Nodes */
        {
            cpssOsPrintf("\t\t\tA-Node ID %d\n",appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNode);
            mm = 0;
            while  (mm < appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].numOfFlowsperAnode)
            {
                cpssOsPrintf("\t\t\t\tflow %d / %d\n",(mm+1),appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].numOfFlowsperAnode);
                for (kk = 0 ; kk < 8 ; kk++)/* loop over queues */

                {
                    cpssOsPrintf("\t\t\t\tqueue ID %d vTcamRuleId %d Tag0 0x%X Tag1 0x%X UP %d\n",
                            appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[mm].queueNum[kk],
                            appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[mm].vTcamRuleId[kk],
                            appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[mm].tag0sTag,
                            appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[mm].tag1cTag,
                            appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[mm].up[kk]);

                }
                mm++;
            }
        }
    }
    return GT_OK;
}

/***************************** CNC Counters blocks *************************/
/**
* @internal appDemoappDemoTcam2CncBlockSet function
* @endinternal
*
* @brief   This API enables and bind 18 blocks of CNC to EPCL
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] devNum       - device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoTcam2CncBlockSet
(
    IN GT_U8 devNum
)
{
    GT_STATUS   rc=GT_OK      ;
    GT_U64      indexRangesBmp;
    GT_U32      ii            ;
    GT_U32      cncBlockNum;

    /* no need to call this API for EPCL */
    rc = cpssDxChCncCountingEnableSet(devNum, CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E, GT_TRUE);
    if( GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChCncCountingEnableSet failed for PCL rc = %d\n",rc);
        return rc;
    }

  /*  for(cncCientNum = CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E;cncCientNum <CPSS_DXCH_CNC_CLIENT_LAST_E;cncCientNum++)*/
    for(cncBlockNum = 0; cncBlockNum < 18; cncBlockNum++)
    {
        rc = cpssDxChCncCounterFormatSet(devNum,cncBlockNum, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E);
        if( GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChCncCounterFormatSet failed to setblock %d rc = %d\n",cncBlockNum,rc);
            return rc;
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* each counter block contains 1024 counters; there are 32 blocks */
        for (ii=0; ii < 18 ; ii++) /* bobK has 32 blocks but only 18K TCAM rules of 10B */
        {
            rc = cpssDxChCncBlockClientEnableSet(devNum, ii,CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E/*  CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E*/, GT_TRUE);
            if( GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChCncBlockClientEnableSet[1] failed i=%d rc = %d\n",ii,rc);
                return rc;
            }
            indexRangesBmp.l[0] = 1 << ii;
            indexRangesBmp.l[1] = 0;
            rc = cpssDxChCncBlockClientRangesSet(devNum,ii,
                                                 CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E/*CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E*/,
                                                 &indexRangesBmp);
            if( GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChCncBlockClientRangesSet[1] failed i=%d rc = %d\n",ii,rc);
                return rc;
            }
        }
    }
    return rc;
}

/**
* @internal appDemoTcam2CncBlockGet function
* @endinternal
*
* @brief   This API gets the CNC counters
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] devNum       - device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoTcam2CncBlockGet
(
    IN GT_U8 devNum
)
{
    CPSS_DXCH_CNC_COUNTER_STC counter;
    GT_U32 totalEPCLCounter = 0;
    GT_STATUS rc;
    GT_U16  ii;
    GT_U16  block;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(0) == GT_TRUE)
    {
        for (block=0; block < 18 ; block++)
        {
            for(ii=0; ii< 1024;ii++)
            {
                rc = cpssDxChCncCounterGet(devNum, block, ii, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
                if( GT_OK != rc)
                   return rc;

                if(counter.packetCount.l[0] != 0)
                {
                    totalEPCLCounter = counter.packetCount.l[0] + totalEPCLCounter;
                    cpssOsPrintf("EPCL rule %d hit %d times\n", ii + block*1024,counter.packetCount.l[0] );
                }

            }
        }

    }

    cpssOsPrintf("%d packets passed through EPCL\n", totalEPCLCounter);
    return GT_OK;
}

/**
* @internal  appDemoTMtreeManipulation function
* @endinternal
*
* @brief   This API map Tag0 and Tag1 and UP0 to TM queues
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] devNum       - device number
* @param[in] startPortInd - start port index in the list
* @param[in] numOfPorts   - number of ports
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoTMtreeManipulation
(
    IN  GT_U8  devNum       ,
    IN  GT_U32 startPortInd ,
    IN  GT_U32 numOfPorts
)
{
    GT_STATUS rc = GT_OK;
    CPSS_TM_PORT_PARAMS_STC      portParams;
    CPSS_TM_C_NODE_PARAMS_STC    cParams;
    CPSS_TM_B_NODE_PARAMS_STC    bParams;
    CPSS_TM_A_NODE_PARAMS_STC    aParams;
    CPSS_TM_QUEUE_PARAMS_STC     qParams;

    GT_U32  portInd,cNodeId,bNodeId,aNodeId,qNodeId;
    GT_U32  ii,queueInd;

    if (printOut)
        cpssOsPrintf("enter appDemoTMtreeManipulation \n");

    cpssOsMemSet(&portParams,0x0,sizeof(portParams));
    cpssOsMemSet(&cParams,0x0,sizeof(cParams));
    cpssOsMemSet(&bParams,0x0,sizeof(bParams));
    cpssOsMemSet(&aParams,0x0,sizeof(aParams));


    portParams.eligiblePrioFuncId = CPSS_TM_ELIG_N_PRIO1_E;
    portParams.numOfChildren = 8;

    cParams.dropCosMap = 0;
    cParams.eligiblePrioFuncId = CPSS_TM_ELIG_N_PRIO1_E;
    cParams.numOfChildren       = 8;
    cParams.quantum             = 64;
    cParams.shapingProfilePtr   = 0;

    bParams.numOfChildren       = 8 ;
    bParams.quantum             = 64;
    bParams.shapingProfilePtr   = 0;
    bParams.eligiblePrioFuncId  = CPSS_TM_ELIG_N_PRIO1_E ;

    aParams.dropProfileInd      = 0;
    aParams.eligiblePrioFuncId  = CPSS_TM_ELIG_N_PRIO1_E;
    aParams.numOfChildren       = 8;
    aParams.quantum             = 64;
    aParams.shapingProfilePtr   = 0;

    qParams.dropProfileInd      = 0;
    qParams.eligiblePrioFuncId  = CPSS_TM_ELIG_Q_PRIO1_E;
    qParams.quantum             = 64;
    qParams.shapingProfilePtr   = 0;

    for(ii=0 ; ii<8;ii++)
    {
        /* port */
        portParams.quantumArr[ii]       = 64;
        portParams.schdModeArr[ii]      = CPSS_TM_SCHD_MODE_RR_E;
        /* c-node */
        cParams.dropProfileIndArr[ii]   = 0;
        cParams.schdModeArr[ii]         = CPSS_TM_SCHD_MODE_RR_E;
        /* b-node */
        bParams.schdModeArr[ii]         = CPSS_TM_SCHD_MODE_RR_E;
        /* a-node */
        aParams.schdModeArr[ii]         = CPSS_TM_SCHD_MODE_RR_E;
        /* queue */
    }

    for(portInd = startPortInd; portInd < numOfPorts; portInd++)
    {
        rc = cpssTmAsymPortCreate(devNum,portInd,&portParams);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssTmAsymPortCreate FAILED for portInd = %d , rc = [%d]\n",portInd, rc);
            return rc;
        }
        rc = cpssTmCnodeToPortCreate(devNum,portInd,&cParams,&cNodeId);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssTmCnodeToPortCreate FAILED for portInd = %d , rc = [%d]\n",portInd, rc);
            return rc;
        }
        else if (printOut)
        {
            cpssOsPrintf("C-Node %d created and hooked to port %d\n",cNodeId,portInd);
        }
        rc = cpssTmBnodeToCnodeCreate (devNum,portInd*8,&bParams,&bNodeId);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssTmBnodeToCnodeCreate FAILED for portInd = %d , rc = [%d]\n",portInd, rc);
            return rc;
        }
        else if (printOut)
        {
            cpssOsPrintf("B-Node %d created and hooked to C-Node %d\n",bNodeId,portInd);
        }
        rc = cpssTmAnodeToBnodeCreate(devNum,portInd*8,&aParams,&aNodeId);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssTmAnodeToBnodeCreate FAILED for portInd = %d , rc = [%d]\n",portInd, rc);
            return rc;
        }
        else if (printOut)
        {
            cpssOsPrintf("A-Node %d created and hooked to B-Node %d\n",aNodeId,portInd);
        }


        for(queueInd = 0; queueInd<8;queueInd++)
        {
            rc = cpssTmQueueToAnodeCreate(devNum,portInd*8,&qParams,&qNodeId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssTmQueueToAnodeCreate FAILED for portInd,Q = [%d,%d] , rc = [%d]\n",portInd, queueInd, rc);
                return rc;
            }
            else if (printOut)
            {
                cpssOsPrintf("queue %d created and hooked to A-Node %d\n",queueInd,portInd);
            }
        }
    }
    return rc;
}

/**
* @internal appDemoTMtreeManipulationSingleAnodePerPort function
* @endinternal
*
* @brief   This API connects  8 C-Nodes an 8 B-Nodes (1:1) and 1 A-Node with 8 queues
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in]  devNum         - device number
* @param[in]  startPortInd   - the port Num to start with
* @param[in]  numOfPorts     - Num of ports to be configured
* @param[out] queueIdPtr     - (pointer to) queue index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoTMtreeManipulationSingleAnodePerPort
(
    IN  GT_U8   devNum      ,
    IN  GT_U32  startPortInd,
    IN  GT_U32  numOfPorts  ,
    OUT GT_U32  *queueIdPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_TM_PORT_PARAMS_STC      portParams;
    CPSS_TM_C_NODE_PARAMS_STC    cParams;
    CPSS_TM_B_NODE_PARAMS_STC    bParams;
    CPSS_TM_A_NODE_PARAMS_STC    aParams;
    CPSS_TM_QUEUE_PARAMS_STC     qParams;
    CPSS_TM_LEVEL_ENT            tmLevel;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC  profileShaping;

    GT_U32   portInd,cNodeId,bNodeId,aNodeId,qNodeId, profileShapingInd;
    GT_U32  i, ii,queueInd,cNodeInd;
    GT_U32  cNodeBaseIndex;

    if (printOut)
        cpssOsPrintf("enter appDemoTMtreeManipulationSingleAnodePerPort \n");


    cpssOsMemSet(&portParams,0x0,sizeof(portParams  ));
    cpssOsMemSet(&cParams   ,0x0,sizeof(cParams     ));
    cpssOsMemSet(&bParams   ,0x0,sizeof(bParams     ));
    cpssOsMemSet(&aParams   ,0x0,sizeof(aParams     ));
    cpssOsMemSet(&qParams   ,0x0,sizeof(qParams     ));


    portParams.cirBw = 1000000; /* 1G */
    portParams.eirBw = 0;
    portParams.cbs = 8;
    portParams.ebs = 0;
    portParams.eligiblePrioFuncId = CPSS_TM_ELIG_N_PRIO1_E;
    portParams.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
    portParams.numOfChildren    = 8;

    cParams.dropCosMap          = CPSS_TM_NO_DROP_PROFILE_CNS;
    cParams.quantum             = CPSS_TM_NODE_QUANTUM_UNIT_CNS;
    cParams.shapingProfilePtr   = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    cParams.eligiblePrioFuncId  = CPSS_TM_ELIG_N_PRIO1_E;
    cParams.numOfChildren       = 1;

    bParams.dropProfileInd      = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    bParams.quantum             = CPSS_TM_NODE_QUANTUM_UNIT_CNS;
    bParams.shapingProfilePtr   = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    bParams.eligiblePrioFuncId  = CPSS_TM_ELIG_N_PRIO1_E;
    bParams.numOfChildren       = 8;

    aParams.dropProfileInd      = CPSS_TM_NO_DROP_PROFILE_CNS;
    aParams.quantum             = CPSS_TM_NODE_QUANTUM_UNIT_CNS;
    aParams.shapingProfilePtr   = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    aParams.eligiblePrioFuncId  = CPSS_TM_ELIG_N_PRIO1_E;
    aParams.numOfChildren       = 8;

    qParams.dropProfileInd      = CPSS_TM_NO_DROP_PROFILE_CNS;
    qParams.quantum             = CPSS_TM_NODE_QUANTUM_UNIT_CNS;
    qParams.shapingProfilePtr   = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    qParams.eligiblePrioFuncId  = CPSS_TM_ELIG_Q_PRIO1_E;

    for(ii=0 ; ii<8;ii++)
    {
        /* port */
        portParams.quantumArr[ii]       = CPSS_TM_PORT_QUANTUM_UNIT_CNS;
        portParams.schdModeArr[ii]      = CPSS_TM_SCHD_MODE_RR_E;
        /* c-node */
        cParams.dropProfileIndArr[ii]   = CPSS_TM_NO_DROP_PROFILE_CNS;
        cParams.schdModeArr[ii]         = CPSS_TM_SCHD_MODE_RR_E;
        /* b-node */
        bParams.schdModeArr[ii]         = CPSS_TM_SCHD_MODE_RR_E;
        /* a-node */
        aParams.schdModeArr[ii]         = CPSS_TM_SCHD_MODE_RR_E;
        /* queue */
    }

    /* create 10 "dummy" shaping profiles per level, in actual scenario they will be changed with updateShaper*/
    for (tmLevel = CPSS_TM_LEVEL_Q_E; tmLevel < CPSS_TM_LEVEL_P_E; tmLevel++)
    {
        profileShaping.cirBw = 200000; /* 200M */
        profileShaping.cbs = 80;  /* Kbytes */
        profileShaping.eirBw = 0;
        profileShaping.ebs = 0;

        for (i = 0; i < 10; i++)
        {
            rc = cpssTmShapingProfileCreate(devNum, tmLevel, &profileShaping, &profileShapingInd);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssTmShapingProfileCreate", rc);
            if (rc != GT_OK)
                return rc;
        }
    }

    /* loops over the ports and create and bind 8 C-Node and B-Nodes, 64 A-Nodes and 8 queues */
    for(portInd = startPortInd; portInd < (startPortInd+numOfPorts); portInd++)
    {
        /* each port has 8 C-Nodes as num of children */
        rc = cpssTmAsymPortCreate(devNum,tmEnablePortList[portInd],&portParams);
        if (GT_OK != rc)
        {
            cpssOsPrintf("\ncpssTmAsymPortCreate FAILED for PhysicalPort = %d, TM Port = %d, rc = [%d]\n",
                         tmEnablePhysicalPortList[portInd], tmEnablePortList[portInd], rc);
            return rc;
        }

        if (printOut)
        {
            cpssOsPrintf("\ncpssTmAsymPortCreate PhysicalPort = %d, TM Port = %d\n",
                         tmEnablePhysicalPortList[portInd], tmEnablePortList[portInd]);
        }

        cNodeBaseIndex = portInd*8;
        for(cNodeInd = 0; cNodeInd < 8; cNodeInd++) /* call it 8 times to creates 8 C-Nodes */
        {
            rc = cpssTmCnodeToPortCreate(devNum,tmEnablePortList[portInd],&cParams,&cNodeId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssTmCnodeToPortCreate FAILED for portInd = %d , rc = [%d]\n",cNodeBaseIndex, rc);
                return rc;
            }

            if (printOut)
            {
                cpssOsPrintf("        cNode %d\n",cNodeId);
            }

            /*rc = cpssTmBnodeToCnodeCreate(devNum,cNodeBaseIndex+cNodeInd,&bParams,&bNodeId);*/
            rc = cpssTmBnodeToCnodeCreate(devNum,cNodeId,&bParams,&bNodeId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssTmBnodeToCnodeCreate FAILED for portInd = %d , rc = [%d]\n",tmEnablePortList[portInd], rc);
                return rc;
            }

            if (printOut)
            {
                cpssOsPrintf("                bNode %d\n",bNodeId);
            }

            if(cNodeInd == 0)
            {
                /*rc = cpssTmAnodeToBnodeCreate(devNum,bNodeBaseIndex+cNodeInd,&aParams,&aNodeId);*/
                /*rc = cpssTmAnodeToBnodeWithQueuePoolCreate(devNum,bNodeInd,&aParams,qNodeId,8,&aNodeId);*/
                rc = cpssTmAnodeToBnodeCreate(devNum, bNodeId, &aParams, &aNodeId);
                if (GT_OK != rc)
                {
                    cpssOsPrintf("cpssTmAnodeToBnodeCreate FAILED for portInd = %d , rc = [%d]\n",tmEnablePortList[portInd], rc);
                    return rc;
                }
                else if (printOut)
                {
                    cpssOsPrintf("                        aNode %d\n",aNodeId);
                }
                /* run over A-Node and add queues */
                /*aNodeBaseIndex = cNodeBaseIndex*8;*/ /* just for the formality */

                qNodeId = appDemoTM8queuesAllocate();
                if(qNodeId >= 512)
                    return GT_NO_RESOURCE;
                *queueIdPtr = qNodeId;

                for(queueInd = 0; queueInd < 8; queueInd++)
                {
                    rc = cpssTmQueueToAnodeByIndexCreate(devNum,aNodeId,&qParams,(qNodeId*8+queueInd+1));
                    if (GT_OK != rc)
                    {
                        cpssOsPrintf("cpssTmQueueToAnodeByIndexCreate FAILED for portInd,Q [%d,%d] , rc = [%d] A-Node=%d qNodeId=%d\n",portInd, queueInd, rc,aNodeId,(qNodeId*8+queueInd+1));
                        return rc;
                    }

                    /* install queue */
                    rc = cpssTmNodesCtlQueueInstall(devNum,(qNodeId*8+queueInd+1));
                    if (GT_OK != rc)
                    {
                        cpssOsPrintf("cpssTmNodesCtlQueueInstall FAILED for Q %d rc %d\n",(qNodeId*8+queueInd+1),rc);
                        return rc;
                    }

                    if (printOut)
                    {
                        cpssOsPrintf("                                queueInd %d installed\n",(qNodeId*8+queueInd+1));
                    }
                }
            }/*queues*/
        }/* cNode */
    }/*port*/
    return rc;
}


/**
* @internal appDemoTMtreeManipulationSingleAnodePerPortDelete
*           function
* @endinternal
*
* @brief   This API Deletes  8 C-Nodes an 8 B-Nodes (1:1) and 1
*          A-Node with 8 queues
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in]  devNum         - device number
* @param[in]  startPortInd   - the port Num to start with
* @param[in]  numOfPorts     - Num of ports to be configured
* @param[out] queueIdPtr     - (pointer to) queue index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoTMtreeManipulationSingleAnodePerPortDelete
(
    IN  GT_U8   devNum      ,
    IN  GT_U32  startPortInd,
    IN  GT_U32  numOfPorts
)
{
    GT_STATUS           rc = GT_OK;
    GT_U32              aNodeId,qNodeId;
    GT_U32              queueInd, cNodeInd, portInd;

    for(portInd = startPortInd; portInd < (startPortInd+numOfPorts); portInd++)
    {
        for(cNodeInd = 0; cNodeInd < 8; cNodeInd++) /* call it 8 times to creates 8 C-Nodes */
        {
            aNodeId = appDemoEgressPortDb[portInd].cNodeTree[cNodeInd].flow2aNode[cNodeInd].aNode;
            for(queueInd = 0; queueInd < 8; queueInd++)
            {
                /* uninstall queue */
                qNodeId = appDemoEgressPortDb[portInd].cNodeTree[cNodeInd].flow2aNode[cNodeInd].aNodeAttr[0].queueNum[queueInd];
                rc = cpssTmNodesCtlQueueUninstall(devNum,qNodeId);
                if (GT_OK != rc)
                {
                    cpssOsPrintf("cpssTmNodesCtlQueueUninstall FAILED for Q %d rc %d\n",qNodeId,rc);
                    return rc;
                }

                rc = cpssTmNodesCtlNodeDelete(devNum, CPSS_TM_LEVEL_Q_E,qNodeId);
                if (GT_OK != rc)
                {
                    cpssOsPrintf("cpssTmNodesCtlNodeDelete FAILED for portInd,Q [%d,%d] , rc = [%d] A-Node=%d qNodeId=%d\n",portInd, queueInd, rc,aNodeId,(qNodeId*8+queueInd+1));
                    return rc;
                }
            }

            rc = cpssTmNodesCtlNodeDelete(devNum, CPSS_TM_LEVEL_A_E, aNodeId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssTmNodesCtlNodeDelete FAILED for aNode=%d rc = %d\n", aNodeId, rc);
                return rc;
            }
            break;
        }/* cNode */
    }/*port*/
    return rc;
}

/**
* @internal appDemoTmPortDefaultRulesSet function
* @endinternal
*
* @brief   This API set 8 queeus per port
* @brief   classify the flows per target Port and UP regardless TAG0/TAG1
* @brief   appDemoTmPortDefaultRulesSet(devNum,tmEnablePortList[portNumIndex],aNode
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] portNum      - port number
* @param[in] pclDirection - PCL direction
* @param[in] aNode        - A node index
* @param[in] ruleFormat   - PCL rule format
* @param[in] vTcamPclMngId - virtual TCAM manager PCL ID
* @param[in] vTcamId       - Virtual TCAM ID
* @param[in] queueId       - queue index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS appDemoTmPortDefaultRulesSet
(
    GT_PORT_NUM                             portNum         ,
    CPSS_PCL_DIRECTION_ENT                  pclDirection    ,
    GT_U32                                  aNode           ,
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT      ruleFormat      ,
    GT_U32                                  vTcamPclMngId   ,
    GT_U32                                  vTcamId         ,
    GT_U32                                  pclRuleId       ,
    GT_U32                                  queueId
)
{
    /*GT_STATUS rc;*/
    GT_U32                                  ii              ;

    /* For IPCL rules */
    CPSS_DXCH_PCL_RULE_FORMAT_UNT           mask            ;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT           pattern         ;
    CPSS_DXCH_PCL_ACTION_STC                action          ;
    GT_STATUS                               rc = GT_OK      ;
    GT_U32                                  priority        ;
    GT_U32                                  up              ;

    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  pclRuleAttributes   ;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        pclRuleType         ;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        pclRuleData         ;

    cpssOsMemSet(&pattern       , 0x0, sizeof(pattern       ));
    cpssOsMemSet(&mask          , 0x0, sizeof(mask          ));
    cpssOsMemSet(&action        , 0x0, sizeof(action        ));
    cpssOsMemSet(&pclRuleType   , 0x0, sizeof(pclRuleType   ));
    cpssOsMemSet(&pclRuleData   , 0x0, sizeof(pclRuleData   ));

    priority                            = 16    ;/* Lowest priority because these are default rules*/
    if (printOut)
        cpssOsPrintf("\tenter appDemoTmPortDefaultRulesSet portNum %d aNode %d Queue %d pclRuleId %d\n",portNum,aNode,queueId,pclRuleId);
/*  numOfTmPorts = (sizeof(tmEnablePortList)/sizeof(portNum));
    for(ii = 0; ii < numOfTmPorts;ii++)*/
    {
        /*portNum = tmEnablePortList[ii];*/
        /* the default rules are UP 0-7 per Port */
        pattern.ruleEgrUdbOnly.udb[0]   = (portNum & 0xFF)  ; /* Classifiy all ports */
        mask.ruleEgrUdbOnly.udb[0]      = appDemoEpclUdb[0].udbMask;

        for (up = 0 ; up < 8; up++)
        {
            pattern.ruleEgrUdbOnly.udb[5]           = up                            ; /* UP */
            mask.ruleEgrUdbOnly.udb[5]              = appDemoEpclUdb[5].udbMask     ;

            action.pktCmd                           = CPSS_PACKET_CMD_FORWARD_E     ;

            action.egressPolicy = pclDirection;

            action.matchCounter.enableMatchCount    = GT_TRUE                   ;
            action.matchCounter.matchCounterIndex   = pclRuleId                 ;
            action.exactMatchOverPclEn              = GT_FALSE                  ;
            action.mirror.cpuCode                   = CPSS_NET_USER_DEFINED_0_E ;
            action.tmQueueId                        = queueId*8 + up + 1        ;

            pclRuleType.ruleType                = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
            pclRuleAttributes.priority          = priority                              ;
            pclRuleType.rule.pcl.ruleFormat     = ruleFormat                            ;
            pclRuleData.valid                   = GT_TRUE                               ;
            pclRuleData.rule.pcl.maskPtr        = &mask                                 ;
            pclRuleData.rule.pcl.patternPtr     = &pattern                              ;
            pclRuleData.rule.pcl.actionPtr      = &action                               ;

            rc = cpssDxChVirtualTcamRuleWrite(vTcamPclMngId,vTcamId,pclRuleId,&pclRuleAttributes,&pclRuleType,&pclRuleData);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChVirtualTcamRuleWrite FAILED, rc = [%d]\n", rc);
                return rc;
            }

            if(printOut)
            {
                cpssOsPrintf("cpssDxChVirtualTcamRuleWrite passed rule %d with priority %d queueId %d\n", pclRuleId,priority,action.tmQueueId);
                for(ii=0;ii<6;ii++)
                    cpssOsPrintf("UDB[%d]=[%x]",ii,pattern.ruleIngrUdbOnly.udb[ii]);
                cpssOsPrintf("\n");
            }

            rc = cpssDxChVirtualTcamRuleRead(vTcamPclMngId,vTcamId,pclRuleId,&pclRuleType,&pclRuleData);
            if(printOut)
            {
                cpssOsPrintf("ruleType %d ruleFormat %d\n",pclRuleType.ruleType,pclRuleType.rule.pcl.ruleFormat);
                cpssOsPrintf("valdi %d udb[0] %d\n",pclRuleData.valid,pclRuleData.rule.pcl.patternPtr->ruleEgrUdbOnly.udb[0]);
            }

            pclRuleId++;
        }
    }
    return GT_OK;

}

/**
* @internal appDemoTmRulesPerFlowRemove function
* @endinternal
*
* @brief   This API removes default rules for 8 queeus per port
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] vTcamPclMngId - virtual TCAM manager PCL ID
* @param[in] vTcamId       - Virtual TCAM ID
* @param[in] pclRuleId     - PCL Rule ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS appDemoTmRulesPerFlowRemove
(
    GT_U32                                  vTcamPclMngId   ,
    GT_U32                                  vTcamId         ,
    GT_U32                                  pclRuleId
)
{
    GT_STATUS                               rc = GT_OK      ;
    GT_U32                                  up              ;

    if (printOut)
        cpssOsPrintf("\tenter appDemoTmPortDefaultRulesDelete pclRuleId %d\n",pclRuleId);

    for (up = 0 ; up < 8; up++)
    {
        rc = cpssDxChVirtualTcamRuleDelete(vTcamPclMngId,vTcamId,pclRuleId);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamRuleDelete FAILED, rc = [%d], rule %d\n", rc, pclRuleId);
            return rc;
        }

        if(printOut)
        {
            cpssOsPrintf("cpssDxChVirtualTcamRuleDelete failed rule %d\n", pclRuleId);
        }

        pclRuleId++;
    }
    return GT_OK;
}

/**
* @internal appDemoTmRulesPerFlowSet function
* @endinternal
*
* @brief   This API map Tag0 and Tag1 and UP0 to TM queues
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] portNum       - port number
* @param[in] sTag          - service Vlan tag vlan ID
* @param[in] cTag          - customer Vlan tag vlan ID
* @param[in] pclDirection  - PCL direction
* @param[in] aNode         - A node index
* @param[in] ruleFormal    - PCL rule format
* @param[in] vTcamPclMngId - Virtual TCAM Manager PCL ID
* @param[in] vTcamId       - Virtual TCAM ID
* @param[in] pclRuleId     - PCL Rule ID.
* @param[in] queuId        - queue index
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoTmRulesPerFlowSet
(
    GT_PORT_NUM                             portNum         ,
    GT_U16                                  sTag            ,
    GT_U16                                  cTag            ,
    CPSS_PCL_DIRECTION_ENT                  pclDirection    ,
    GT_U32                                  aNode           ,
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT      ruleFormat      ,
    GT_U32                                  vTcamPclMngId   ,
    GT_U32                                  vTcamId         ,
    GT_U32                                  pclRuleId       ,
    GT_U32                                  queueId
)
{
    /* For IPCL rules */
    CPSS_DXCH_PCL_RULE_FORMAT_UNT           mask            ;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT           pattern         ;
    CPSS_DXCH_PCL_ACTION_STC                action          ;
    GT_STATUS                               rc = GT_OK      ;
    GT_U32                                  priority        ;
    GT_U32                                  up              ;

    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  pclRuleAttributes   ;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        pclRuleType         ;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        pclRuleData         ;

    cpssOsMemSet(&pattern       , 0x0, sizeof(pattern       ));
    cpssOsMemSet(&mask          , 0x0, sizeof(mask          ));
    cpssOsMemSet(&action        , 0x0, sizeof(action        ));
    cpssOsMemSet(&pclRuleType   , 0x0, sizeof(pclRuleType   ));
    cpssOsMemSet(&pclRuleData   , 0x0, sizeof(pclRuleData   ));

    priority                            = 4 ;/* Higher priority than the default rules*/
    if (printOut)
    {
        cpssOsPrintf("\tenter appDemoTmRulesPerFlowSet portNum %d aNode %d Queue %d pclRuleId %d\n",portNum,aNode,queueId,pclRuleId);
    }

    /* the default rules are UP 0-7 per Port */
    pattern.ruleEgrUdbOnly.udb[0]   = (portNum & 0xFF)  ; /* Local Dev Trg PHY Port */
    mask.ruleEgrUdbOnly.udb[0]      = appDemoEpclUdb[0].udbMask;

    for (up = 0 ; up < 8; up++)
    {
        pattern.ruleEgrUdbOnly.udb[5]           = up                        ; /* UP0 */
        mask.ruleEgrUdbOnly.udb[5]              = appDemoEpclUdb[5].udbMask ;


        pattern.ruleEgrUdbOnly.udb[1]           = sTag & 0xff               ; /* eVlan LSB */
        mask.ruleEgrUdbOnly.udb[1]              = appDemoEpclUdb[1].udbMask ;

        pattern.ruleEgrUdbOnly.udb[2]           = ( sTag >> 8 ) & 0xff      ; /* eVlan MSB */
        mask.ruleEgrUdbOnly.udb[2]              = appDemoEpclUdb[2].udbMask ;

        pattern.ruleEgrUdbOnly.udb[3]           = cTag & 0xff               ; /* Tag1 VID LSB */
        mask.ruleEgrUdbOnly.udb[3]              = appDemoEpclUdb[1].udbMask ;

        pattern.ruleEgrUdbOnly.udb[4]           = ( cTag >> 8 ) & 0xff      ; /* Tag1 VID MSB */
        mask.ruleEgrUdbOnly.udb[4]              = appDemoEpclUdb[2].udbMask ;

        action.pktCmd                           = CPSS_PACKET_CMD_FORWARD_E ;

        action.egressPolicy = pclDirection;

        action.matchCounter.enableMatchCount    = GT_TRUE                   ;/*GT_FALSE didn't help to fix the problem that EM counter should be inceremented */
        action.matchCounter.matchCounterIndex   = pclRuleId                 ;
        action.exactMatchOverPclEn              = GT_FALSE                  ;
        action.mirror.cpuCode                   = CPSS_NET_USER_DEFINED_0_E ;
        action.tmQueueId                        = queueId + up              ;

        pclRuleType.ruleType                = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        pclRuleAttributes.priority          = priority                              ;
        pclRuleType.rule.pcl.ruleFormat     = ruleFormat                            ;
        pclRuleData.valid                   = GT_TRUE                               ;
        pclRuleData.rule.pcl.maskPtr        = &mask                                 ;
        pclRuleData.rule.pcl.patternPtr     = &pattern                              ;
        pclRuleData.rule.pcl.actionPtr      = &action                               ;

        rc = cpssDxChVirtualTcamRuleWrite(vTcamPclMngId,vTcamId,pclRuleId,&pclRuleAttributes,&pclRuleType,&pclRuleData);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamRuleWrite FAILED, rc = [%d]\n", rc);
            return rc;
        }

        if(printOut)
        {
            cpssOsPrintf("cpssDxChVirtualTcamRuleWrite passed rule %d with priority %d queueId %d\n", pclRuleId,priority,action.tmQueueId);
            cpssOsPrintf("UDB[0,1,2,3,4,5]=[%-3x%-3x%-3x%-3x%-3x]\n",pattern.ruleIngrUdbOnly.udb[0],
                pattern.ruleIngrUdbOnly.udb[1],
                pattern.ruleIngrUdbOnly.udb[2],
                pattern.ruleIngrUdbOnly.udb[3],
                pattern.ruleIngrUdbOnly.udb[4],
                pattern.ruleIngrUdbOnly.udb[5]);
        }
        pclRuleId++;
    }
    return GT_OK;

}

GT_STATUS appDemoTmAnodeToBnodeCreateAtIndex
(
    IN    GT_U8   devNum,
    IN    GT_U32  bNodeInd,
    IN    CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    INOUT GT_U32 *aNodeIndPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 ii, jj = 0;
    GT_U32 aNodeInd;
    GT_U32 aNodeIndArr[8];
    GT_BOOL aNodeCreated = GT_FALSE;

    cpssOsMemSet(aNodeIndArr, 0, sizeof(aNodeIndArr));

    jj = 0;
    for (ii = (bNodeInd*8); ii < (bNodeInd*8 + 8); ii++)
    {
        rc = cpssTmAnodeToBnodeCreate(devNum,bNodeInd,aParamsPtr,&aNodeInd);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssTmAnodeToBnodeCreate FAILED for bNode-aNode = %d-%d , rc = [%d]\n",bNodeInd,aNodeInd, rc);
            return rc;
        }

        if (aNodeInd == *aNodeIndPtr)
        {
            aNodeCreated = GT_TRUE;
            break;
        }

        aNodeIndArr[jj] = aNodeInd;
        jj++;

        if (aNodeInd == (bNodeInd*8 + 7))
        {
            break;
        }
    }

    for (ii = 0; ii < jj; ii++)
    {
        rc = cpssTmNodesCtlNodeDelete(devNum, CPSS_TM_LEVEL_A_E, aNodeIndArr[ii]);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssTmNodesCtlNodeDelete FAILED for aNode=%d rc = %d\n", aNodeInd, rc);
        }
    }

    if (aNodeCreated == GT_FALSE)
    {
        rc = cpssTmAnodeToBnodeCreate(devNum,bNodeInd,aParamsPtr,&aNodeInd);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssTmAnodeToBnodeCreate FAILED for bNode-aNode = %d-%d , rc = [%d]\n",bNodeInd,aNodeInd, rc);
            return rc;
        }

        *aNodeIndPtr = aNodeInd;
    }

    return GT_OK;
}

/**
 * @internal appDemoTmFlowAdd function
 * @endinternal
 *
 * @brief   This API map Tag0 and Tag1 and UP0 to TM queues
 *
 * @note   APPLICABLE DEVICES     -  Caelum
 *
 * @param[in]    devNum        - device number
 * @param[in]    sTag          - service Vlan tag vlan ID
 * @param[in]    cTag          - customer Vlan tag vlan ID
 * @param[in]    bNodeInd      - B node index
 * @param[inout] aNodeIndPtr   - (pointer to) A node index
 * @param[in]    egressPortNum - egress port number
 *
 * @retval  GT_OK - on success
 */
GT_STATUS appDemoTmFlowAdd
(
    IN      GT_U8       devNum          ,
    IN      GT_U16      sTag            ,
    IN      GT_U16      cTag            ,
    IN      GT_U32      bNodeInd        ,
    INOUT   GT_U32      *aNodeIndPtr    , /* This is the global aNode index in the TM */
    IN      GT_PORT_NUM egressPortNum /* jus to avoid confusion */
)
{
    GT_STATUS   rc, rc2;
    GT_U32      portNumIndex    ;
    GT_U32      ii;
    GT_U32      aNodeInd        ;
    GT_U32      aNodeDbInd      ; /* the aNode index in the DB 0..7 */
    GT_U32      bNodeDbInd      ; /* the bNode index in the DB 0..7 */
    GT_U32      vTcamRuleId     ;
    GT_U32      qNodeId         ; /* an index for the 8 queues chunk in DB */
    GT_U32      queueInd        ; /* used for loop over 8 queues */
    GT_U32      numOfFlows      ;
    GT_U32      vTcamPclMngId   = VTCAM_PCL_MNG_ID      ;
    GT_U32      vTcamId         = VTCAM_ID              ;
    GT_BOOL     nodeAlreadyExist = GT_FALSE;
    GT_BOOL     flowExists = GT_FALSE;
    GT_U32      parentIndex = 0;
    GT_U32      firstChildIndex = 0;
    GT_U32      lastChildIndex = 0;

    CPSS_TM_QUEUE_PARAMS_STC     qParams;
    CPSS_TM_A_NODE_PARAMS_STC   aParams         ;
    appDemoANodePath            *aNodePathPtr   ;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E;

    if (printOut)
        cpssOsPrintf("enter appDemoTmFlowAdd \n");

    /* get the prot index in the array of ports */
    portNumIndex = appDemoTmPortIndexGet(egressPortNum);
    bNodeDbInd = bNodeInd - portNumIndex * 8        ; /* each port has 8 bNodes and 8 cNodes */
    aNodeDbInd = (*aNodeIndPtr - portNumIndex * 64) % 8   ; /* each port has 64 aNodes             */

    /* need to validate aNode */
    if (aNodeDbInd > 7)
    {
        cpssOsPrintf("error aNodeDbInd = %d\n",aNodeDbInd);
        return GT_BAD_PARAM;
    }

    aNodePathPtr = &appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd];
    numOfFlows = aNodePathPtr->numOfFlowsperAnode;

    if (numOfFlows >= MaxNumOfFlowPerAnode)
    {
        if (printOut)
        {
            cpssOsPrintf("\t Cannot add flow, Max flows per A-Node is %d \n", MaxNumOfFlowPerAnode);
        }
        return GT_NO_RESOURCE;
    }

    for (ii = 0; ii < numOfFlows; ii++)
    {
        if((aNodePathPtr->aNodeAttr[ii].tag0sTag == sTag) &&
           (aNodePathPtr->aNodeAttr[ii].tag1cTag == cTag))
        {
            flowExists = GT_TRUE;
            break;
        }
    }

    /* check if there are queeus already allocated to this aNode */
    /* it might also be that this aNode was not set yet          */
    if(numOfFlows > 0)
    {
        if (flowExists == GT_TRUE)
        {
            if (printOut)
            {
                cpssOsPrintf("\t Flow already exists\n");
            }
            return GT_ALREADY_EXIST;
        }

        /* aNode already exist and host at least 1 flow */
        nodeAlreadyExist    = GT_TRUE       ;
        /* The real aNode is */
        aNodeInd = appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].aNode;
        qNodeId  = appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].aNodeAttr[0].queueNum[0];
        if(printOut)
        {
            cpssOsPrintf("\tthere are %d flows already attached to this aNode\n",numOfFlows);
            cpssOsPrintf("\ta flow will be attached to portNum %d cNode %d aNode %d queue%d\n",egressPortNum,bNodeInd,aNodeInd,qNodeId);
        }
    }
    else
    {
        /* Feb 4th */
        /* might need to create a new API to bind 8 queues to aNode */
        /* and also get the queue attributes as parameters */
        cpssOsMemSet(&qParams   ,0x0,sizeof(qParams     ));
        cpssOsMemSet(&aParams   ,0x0,sizeof(aParams     ));
        aParams.dropProfileInd      = CPSS_TM_NO_DROP_PROFILE_CNS ;
        aParams.quantum             = CPSS_TM_NODE_QUANTUM_UNIT_CNS;
        aParams.shapingProfilePtr   = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
        aParams.eligiblePrioFuncId  = CPSS_TM_ELIG_N_PRIO1_E;
        aParams.numOfChildren       = 8                     ;
        qParams.dropProfileInd      = CPSS_TM_NO_DROP_PROFILE_CNS;
        qParams.quantum             = CPSS_TM_NODE_QUANTUM_UNIT_CNS;
        qParams.shapingProfilePtr   = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
        qParams.eligiblePrioFuncId  = CPSS_TM_ELIG_Q_PRIO1_E;

        aNodeInd = *aNodeIndPtr;
        /* need to allocate queues */
        /* rc = appDemoTmAnodeToBnodeCreateAtIndex(devNum,bNodeInd,&aParams,&aNodeInd);*/
        rc = cpssTmAnodeToBnodeCreate(devNum,bNodeInd,&aParams,&aNodeInd);
        if (GT_OK != rc)
        {
            cpssOsPrintf("appDemoTmAnodeToBnodeCreate FAILED for bNode-aNode = %d-%d , rc = [%d]\n",bNodeInd,aNodeInd, rc);
            return rc;
        }
        else if (printOut)
        {
            cpssOsPrintf("                        aNode %d\n",aNodeInd);
        }

        *aNodeIndPtr = aNodeInd;
        aNodeDbInd = (*aNodeIndPtr - portNumIndex * 64) % 8;
        aNodePathPtr = &appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd];
        numOfFlows = aNodePathPtr->numOfFlowsperAnode;

        rc = cpssTmNodeMappingInfoRead(devNum, CPSS_TM_LEVEL_A_E, aNodeInd, &parentIndex, &firstChildIndex, &lastChildIndex);
        qNodeId = firstChildIndex/8;
        if (qNodeId < 512)
        {
            queuePool[qNodeId] = GT_TRUE;
        }

        if(qNodeId>=512)
            return GT_NO_RESOURCE;

        qNodeId = qNodeId*8 + 1;
        /* set the TM tree */
        for(queueInd = 0; queueInd<8;queueInd++)
        {
            rc = cpssTmQueueToAnodeByIndexCreate(devNum,aNodeInd,&qParams,(qNodeId + queueInd));
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssTmQueueToAnodeByIndexCreate FAILED for aNode=%d queueInd=%d rc %d\n",aNodeInd, (qNodeId + queueInd), rc);

                appDemoTM8queuesFree(qNodeId/8);

                rc2 = cpssTmNodesCtlNodeDelete(devNum, CPSS_TM_LEVEL_A_E, aNodeInd);
                if (GT_OK != rc2)
                {
                    cpssOsPrintf("cpssTmNodesCtlNodeDelete FAILED for aNode=%d rc = %d\n", aNodeInd, rc2);
                }

                return rc;
            }

            rc = cpssTmNodesCtlQueueInstall(devNum,(qNodeId + queueInd));
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssTmNodesCtlQueueInstall FAILED for Q %d rc %d\n",(qNodeId + queueInd),rc);

                appDemoTM8queuesFree(qNodeId/8);

                rc2 = cpssTmNodesCtlNodeDelete(devNum, CPSS_TM_LEVEL_A_E, aNodeInd);
                if (GT_OK != rc2)
                {
                    cpssOsPrintf("cpssTmNodesCtlNodeDelete FAILED for aNode=%d rc = %d\n", aNodeInd, rc2);
                }

                return rc;
            }

            if (printOut)
            {
                cpssOsPrintf("                                queueInd %d\n",(qNodeId + queueInd));
            }

        }
        /* need to update the DB */

    }

    vTcamRuleId = appDemoTMePcl8RulesAllocate();
    if (vTcamRuleId == 0xFFFFFFFF)
        return GT_OUT_OF_PP_MEM;
    else
        vTcamRuleId = vTcamRuleId*8;


    /* search the aNode location in the srray                               */
    /* assume that the aNode are set in the beginning and each port has 64 aNodes */
    /* aNode % (portIndex*64) =  */
    /* if found no need to allocat queues if not need to allocated queues   */
    /* in both cases need to allocated vTcam rules                          */
    /* increase the number of flows associated with that aNode              */
    if (nodeAlreadyExist)
    {
        /*qNodeId = appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeInd].flow2aNode[aNodeInd].aNodeAttr[numOfFlows].queueNum[0];*/
    }
    rc = appDemoTmRulesPerFlowSet(egressPortNum ,
            sTag                            ,
            cTag                            ,
            CPSS_PCL_DIRECTION_EGRESS_E     ,
            aNodeInd                        ,
            ruleFormat                      ,
            vTcamPclMngId                   ,
            vTcamId                         ,
            vTcamRuleId                     ,
            qNodeId                         );
    if(rc !=GT_OK)
    {
        cpssOsPrintf("appDemoTmRulesPerFlowSet fails rc = %d for port %d aNode %d vTcamRuleId %d\n",rc,
                appDemoEgressPortDb[portNumIndex].egressPort,
                aNodePathPtr->aNode,vTcamRuleId);
        return rc;
    }
    else if (printOut)
    {
        cpssOsPrintf("appDemoTmRulesPerFlowSet passed for port %d aNode %d vTcamRuleId %d\n",
                appDemoEgressPortDb[portNumIndex].egressPort,
                aNodePathPtr->aNode,vTcamRuleId );
    }

    appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].numOfFlowsperAnode  += 1;
    appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].aNode                = aNodeInd;
    appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].aNodeAttr[numOfFlows].tag0sTag = sTag      ;
    appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].aNodeAttr[numOfFlows].tag1cTag = cTag      ;
    appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].aNodeAttr[numOfFlows].priority = 4         ;
    for(ii = 0; ii < 8; ii++)
    {
        appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].aNodeAttr[numOfFlows].up[ii]           = ii    ;
        appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].aNodeAttr[numOfFlows].vTcamRuleId[ii] = vTcamRuleId+ii; /* already multiple by 8 above */
        appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd].aNodeAttr[numOfFlows].queueNum[ii] = qNodeId +ii;
    }
    return GT_OK;
}

/**
* @internal appDemoTmFlowAddWrapper function
* @endinternal
*
* @brief   This API map Tag0 and Tag1 and UP0 to TM queues
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
 * @param[in]    devNum        - device number
 * @param[in]    sTag          - service Vlan tag vlan ID
 * @param[in]    cTag          - customer Vlan tag vlan ID
 * @param[in]    bNodeInd      - B node index
 * @param[inout] aNodeIndPtr   - (pointer to) A node index
 * @param[in]    egressPortNum - egress port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoTmFlowAddWrapper
(
    IN      GT_U8       devNum          ,
    IN      GT_U16      sTag            ,
    IN      GT_U16      cTag            ,
    IN      GT_U32      bNodeInd        ,
    INOUT   GT_U32      aNodeInd        ,
    IN      GT_PORT_NUM egressPortNum /* jus to avoid confusion */
)
{
    GT_STATUS rc;
    rc = appDemoTmFlowAdd(devNum,sTag,cTag,bNodeInd,&aNodeInd,egressPortNum);
    return rc;
}


/**
* @internal appDemoTmRulesPerFlowInvalidate function
* @endinternal
*
* @brief   This API unmap Tag0 and Tag1 and UP0 to TM queues
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] vTcamPclMngId       - virtual TCAM Manager PCL ID
* @param[in] vTcamId             - virtual TCAM ID
* @param[in] pclRuleId           - PCL rule ID
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoTmRulesPerFlowInvalidate
(
    GT_U32                                  vTcamPclMngId   ,
    GT_U32                                  vTcamId         ,
    GT_U32                                  pclRuleId
)
{
    /* For IPCL rules */
    GT_STATUS                               rc = GT_OK      ;
    GT_U32                                  up              ;

    if (printOut)
        cpssOsPrintf("\tenter appDemoTmRulesPerFlowInvalidate pclRuleId %d\n",pclRuleId);

    for (up = 0 ; up < 8; up++)
    {
        rc = cpssDxChVirtualTcamRuleValidStatusSet(vTcamPclMngId,vTcamId,pclRuleId,GT_FALSE);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamRuleValidStatusSet FAILED, rc = [%d]\n", rc);
            return rc;
        }
        pclRuleId++;
    }
    return GT_OK;

}

/**
* @internal appDemoTmFlowRemove function
* @endinternal
*
* @brief   This API unmap Tag0 and Tag1 and UP0 to TM queues
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
 * @param[in]    devNum        - device number
 * @param[in]    sTag          - service Vlan tag vlan ID
 * @param[in]    cTag          - customer Vlan tag vlan ID
 * @param[in]    bNodeInd      - B node index
 * @param[inout] aNodeIndPtr   - (pointer to) A node index
 * @param[in]    egressPortNum - egress port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoTmFlowRemove
(
    IN      GT_U8       devNum          ,
    IN      GT_U16      sTag            ,
    IN      GT_U16      cTag            ,
    IN      GT_U32      bNodeInd        ,
    IN      GT_U32      aNodeInd       , /* This is the global aNode index in the TM */
    IN      GT_PORT_NUM egressPortNum /* jus to avoid confusion */
)
{
    GT_STATUS   rc              ;
    GT_U32      portNumIndex    ;
    GT_U32      ii;
    GT_U32      aNodeDbInd      ; /* the aNode index in the DB 0..7 */
    GT_U32      bNodeDbInd      ; /* the bNode index in the DB 0..7 */
    GT_U32      vTcamRuleId     ;
    GT_U32      qNodeId         ; /* an index for the 8 queues chunk in DB */
    GT_U32      queueInd        ; /* used for loop over 8 queues */
    GT_U32      numOfFlows      ;
    GT_U32      vTcamPclMngId   = VTCAM_PCL_MNG_ID      ;
    GT_U32      vTcamId         = VTCAM_ID              ;
    GT_U32      flowInd = 0;
    appDemoANodePath            *aNodePathPtr   ;

    if (printOut)
        cpssOsPrintf("enter appDemoTmFlowRemove \n");

    /* get the prot index in the array of ports */
    portNumIndex = appDemoTmPortIndexGet(egressPortNum);
    bNodeDbInd = bNodeInd - portNumIndex * 8        ; /* each port has 8 bNodes and 8 cNodes */
    aNodeDbInd = (aNodeInd - portNumIndex * 64) % 8   ; /* each port has 64 aNodes             */

    if (aNodeDbInd > 7)
    {
        cpssOsPrintf("error aNodeDbInd = %d\n",aNodeDbInd);
        return GT_BAD_PARAM;
    }
    if(printOut)
        cpssOsPrintf("\tportNum %d has index %d cNodeIndex %d aNodeIndex %d \n",egressPortNum,portNumIndex,bNodeInd,aNodeInd);


    aNodePathPtr = &appDemoEgressPortDb[portNumIndex].cNodeTree[bNodeDbInd].flow2aNode[aNodeDbInd];
    numOfFlows = aNodePathPtr->numOfFlowsperAnode;

    flowInd = numOfFlows;
    for (ii = 0; ii < numOfFlows; ii++)
    {
        if((aNodePathPtr->aNodeAttr[ii].tag0sTag == sTag) &&
           (aNodePathPtr->aNodeAttr[ii].tag1cTag == cTag))
        {
            flowInd = ii;
            break;
        }
    }

    if (flowInd == numOfFlows)
    {
        return GT_NOT_FOUND;
    }

    qNodeId = aNodePathPtr->aNodeAttr[flowInd].queueNum[0];
    if(qNodeId >= 512)
    {
        return GT_BAD_PARAM;
    }

    vTcamRuleId = aNodePathPtr->aNodeAttr[flowInd].vTcamRuleId[0];
    if (vTcamRuleId == 0xFFFFFFFF)
    {
        return GT_BAD_VALUE;
    }

    rc = appDemoTMePcl8RulesFree(vTcamRuleId/8);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoTmRulesPerFlowRemove(vTcamPclMngId, vTcamId, vTcamRuleId);
    if(rc !=GT_OK)
    {
        cpssOsPrintf("appDemoTmRulesPerFlowInvalidate fails rc = %d for port %d vTcamRuleId %d\n",rc,
                appDemoEgressPortDb[portNumIndex].egressPort, vTcamRuleId);
        return rc;
    }
    else if (printOut)
    {
        cpssOsPrintf("appDemoTmRulesPerFlowInvalidate passed for port %d aNode %d vTcamRuleId %d\n",
                appDemoEgressPortDb[portNumIndex].egressPort,
                aNodePathPtr->aNode,vTcamRuleId );
    }

    /* Delete flowInd and shift all data after flowInd in the array by one index */
    for (ii = flowInd; ii < numOfFlows - 1; ii++)
    {
        cpssOsMemCpy(&(aNodePathPtr->aNodeAttr[ii]), &(aNodePathPtr->aNodeAttr[ii+1]),
                     sizeof(appDemoFlowStruct));
    }
    aNodePathPtr->numOfFlowsperAnode  -= 1;
    cpssOsMemSet(&(aNodePathPtr->aNodeAttr[ii]), 0, sizeof(appDemoFlowStruct));


    if (((aNodeDbInd != 0) || (bNodeDbInd != 0)) && (numOfFlows == 1) && (flowInd == 0))
    {
        /* set the TM tree */
        for(queueInd = qNodeId; queueInd < qNodeId + 8; queueInd++)
        {
            /* un-install queue */
            rc = cpssTmNodesCtlQueueUninstall(devNum,queueInd);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssTmNodesCtlQueueUninstall FAILED for Q %d rc %d\n",queueInd,rc);
                return rc;
            }
            else if (printOut)
            {
                cpssOsPrintf("                                queueInd %d un installed\n",queueInd);
            }

            rc = cpssTmNodesCtlNodeDelete(devNum, CPSS_TM_LEVEL_Q_E,queueInd);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssTmNodesCtlNodeDelete FAILED for aNode=%d queueInd=%d rc %d\n",aNodeInd, queueInd, rc);
                return rc;
            }
        }

        rc = cpssTmNodesCtlNodeDelete(devNum, CPSS_TM_LEVEL_A_E, aNodePathPtr->aNode);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssTmNodesCtlNodeDelete FAILED for aNode=%d rc = %d\n", aNodeInd, rc);
            return rc;
        }

        rc = appDemoTM8queuesFree(qNodeId/8);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoPclConfigurationTableSet function
* @endinternal
*
* @brief   This API set PCL configuration table
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] devNum       - device number
* @param[in] portNumber   - the port Num to start with
* @param[in] ruleFormat   - rule format for nonIp,IPv4 and IPv6
* @param[in] pclDirection - IPCL or EPCL
* @param[in] pclLookupNum - LU number
* @param[in] pclId        - PCL-ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS appDemoPclConfigurationTableSet
(
    GT_U8                               devNum          ,
    GT_U32                              portNumber      ,
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat      , /* for nonIp,IPv4 and IPv6 */
    CPSS_PCL_DIRECTION_ENT              pclDirection    ,
    CPSS_PCL_LOOKUP_NUMBER_ENT          pclLookupNum    ,
    GT_U32                              pclId
)
{
    GT_STATUS                       rc              ;
    /* PCL Configuration variables */
    CPSS_INTERFACE_INFO_STC         interfaceInfo   ;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg       ;
    /* PCL UDB definition */
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC   accMode;

    if (printOut)
        cpssOsPrintf("enter appDemoPclConfigurationTableSet \n");

    cpssOsMemSet(&accMode,0x0,sizeof(accMode));

    if(pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNumber, GT_TRUE);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChPclPortIngressPolicyEnable FAILED, rc = [%d]\n", rc);
            return rc;
        }
        else if (printOut)
            cpssOsPrintf("cpssDxChPclPortIngressPolicyEnable is OK\n");

        accMode.ipclAccMode     = CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E ;
        accMode.ipclDevPortBase = CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E    ;
        accMode.ipclMaxDevPorts = CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_64_E  ;

        rc = cpssDxChPclCfgTblAccessModeSet(devNum,&accMode);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChPclCfgTblAccessModeSet FAILED, rc = [%d]\n", rc);
            return rc;
        }
        else if (printOut)
            cpssOsPrintf("cpssDxChPclCfgTblAccessModeSet is OK\n");


        rc = cpssDxChPclPortLookupCfgTabAccessModeSet(devNum, portNumber,pclDirection,  pclLookupNum, 0,CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChPclPortLookupCfgTabAccessModeSet FAILED, rc = [%d]\n", rc);
            return rc;
        }
        else if (printOut)
            cpssOsPrintf("cpssDxChPclPortLookupCfgTabAccessModeSet is OK\n");
    }
    else if (pclDirection == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        accMode.epclAccMode     = CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E ;
        accMode.epclDevPortBase = CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E    ;
        accMode.epclMaxDevPorts = CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_64_E  ;

        rc = cpssDxChPclCfgTblAccessModeSet(devNum,&accMode);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChPclCfgTblAccessModeSet FAILED, rc = [%d]\n", rc);
            return rc;
        }
        else if (printOut)
            cpssOsPrintf("cpssDxChPclCfgTblAccessModeSet is OK\n");
    }
    else
    {
        rc = GT_BAD_PARAM ;
    }
    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo , 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg     , 0, sizeof(lookupCfg));

    interfaceInfo.type                  = CPSS_INTERFACE_PORT_E             ;
    interfaceInfo.devPort.hwDevNum      = PRV_CPSS_HW_DEV_NUM_MAC(devNum)   ;
    interfaceInfo.devPort.portNum       = portNumber                    ;

    lookupCfg.pclId                     = pclId         ;
    lookupCfg.enableLookup              = GT_TRUE       ;

    lookupCfg.groupKeyTypes.nonIpKey    = ruleFormat    ;
    lookupCfg.groupKeyTypes.ipv4Key     = ruleFormat    ;
    lookupCfg.groupKeyTypes.ipv6Key     = ruleFormat    ;

    rc = cpssDxChPclCfgTblSet(devNum,&interfaceInfo,pclDirection,pclLookupNum, &lookupCfg);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChPclCfgTblSet FAILED, rc = [%d]\n", rc);
        return rc;
    }
    else if (printOut)
        cpssOsPrintf("cpssDxChPclCfgTblSet is OK\n");

    return rc;
}

/**
* @internal appDemoTmFlatQoSAllPortsSet function
* @endinternal
*
* @brief   1. allocate 8 C-Nodes,8 B-Nodes,64 A-Nodes and 8 queues to the port
*          2. set EPCL rules classifu traffic to that port with UP and map it to relevant queue
*          3. port & UP[0..7] ==> queue 1..8
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] devNum       - device number
* @param[in] pclDirection - IPCL or EPCL
* @param[in] ruleFormat   - rule format for nonIp,IPv4 and IPv6
* @param[in] vTcamPclMngId - virtual TCAM Manager PCL ID
* @param[in] vTcamId       - virtual TCAM ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS appDemoTmFlatQoSAllPortsSet
(
    GT_U8                                   devNum          ,
    CPSS_PCL_DIRECTION_ENT                  pclDirection    ,
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT      ruleFormat      ,
    GT_U32                                  vTcamPclMngId   ,
    GT_U32                                  vTcamId
)
{
    GT_U32      ii, jj, kk   ;
    GT_STATUS   rc           ;
    GT_U32      portNumIndex ;
    GT_U32      vTcamRuleId  ;
    GT_U32      queueId      ;
    CPSS_DXCH_PORT_MAP_STC portMapArray[1];

    if (printOut)
        cpssOsPrintf("enter appDemoTmFlatQoSAllPortsSet \n");

    for (ii=0; ii < numOfTmPorts; ii++)
    {
        rc = cpssDxChPortPhysicalPortMapGet(devNum, tmEnablePhysicalPortList[ii], 1, portMapArray);
        if (rc != GT_OK)
        {
            if (printOut)
            {
                cpssOsPrintf("cpssDxChPortPhysicalPortMapGet failed\n");
            }
            return rc;
        }

        tmEnablePortList[ii] = portMapArray[0].interfaceNum;
    }

    /* init all nodes w/o vTCAM rules and queeus ID */
    /* could be done by cpssOsMemset :-)            */
    for(portNumIndex = 0; portNumIndex < numOfTmPorts ; portNumIndex++)
    {
        appDemoEgressPortDb[portNumIndex].egressPort = tmEnablePhysicalPortList[portNumIndex];
        for (ii = 0 ; ii < 8 ; ii++)/* loop over cNode */
        {
            appDemoEgressPortDb[portNumIndex].cNodeTree[ii].cNode = portNumIndex*8 + ii;
            appDemoEgressPortDb[portNumIndex].cNodeTree[ii].bNode = appDemoEgressPortDb[portNumIndex].cNodeTree[ii].cNode;
            for (jj = 0 ; jj < 8 ; jj++)/* loop over aNode */
            {
                appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNode = portNumIndex*64+ii*8+jj /*ii*8+jj*/; /* assuming that all ports are initiated before */
                appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].numOfFlowsperAnode = 0;
                for(kk = 0; kk < 8; kk++)/* loop over queue */
                {
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].queueNum[kk]     = 0x9999; /* Queue starts with 1 and not 0 as other nodes */
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].vTcamRuleId[kk]  = 0x9999;
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].tag0sTag         = 0x9999;
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].tag1cTag         = 0x9999;
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].up[kk]           = 0x99;
                }
            } /* if jj == 0*/
        } /* for jj */
    }/* for portIndex */


    /**************************************************************************************/
    /**************************************************************************************/
    /**************************************************************************************/
    /* set the 1st aNode in each port */
    for(portNumIndex = 0; portNumIndex < numOfTmPorts;portNumIndex++)
    {
        /* Set the relevant TM tree */
        rc = appDemoTMtreeManipulationSingleAnodePerPort (devNum,portNumIndex,1,&queueId);
        if(rc !=GT_OK)
            return rc;
        else if (printOut)
        {
            cpssOsPrintf("appDemoTMtreeManipulationSingleAnodePerPort set port %d with Flat QoS \n",tmEnablePhysicalPortList[portNumIndex]);
        }
        /* fill the DB of the egress ports */
        appDemoEgressPortDb[portNumIndex].egressPort = tmEnablePhysicalPortList[portNumIndex];
        for (ii = 0 ; ii < 8 ; ii++)/* loop over cNode */
        {
            appDemoEgressPortDb[portNumIndex].cNodeTree[ii].cNode = portNumIndex*8 + ii;
            appDemoEgressPortDb[portNumIndex].cNodeTree[ii].bNode = appDemoEgressPortDb[portNumIndex].cNodeTree[ii].cNode;
            /* there are 8 flows bound to the 1st aNode of each port  */
            for (jj = 0 ; jj < 8 ; jj++)/* loop over aNode */
            {
                /* set the number flows assuming no failure and therefore no need for roolback */
                appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].numOfFlowsperAnode = 1; /* per 8 UPs */
                vTcamRuleId = appDemoTMePcl8RulesAllocate();
                if (vTcamRuleId == 0xFFFFFFFF)
                    return GT_OUT_OF_PP_MEM;
                else
                    vTcamRuleId = vTcamRuleId*8;


                appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNode = portNumIndex*64+ii*8+jj /*ii*8+jj*/; /* assuming that all ports are initiated before */
                for(kk = 0; kk < 8; kk++)/* loop over queue */
                {
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].queueNum[kk]    = queueId*8 + kk + 1;/*portNumIndex*8+kk+1;*/ /* Queue starts with 1 and not 0 as other nodes */
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].vTcamRuleId[kk] = vTcamRuleId + kk  ;
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].tag0sTag        = 0xffff            ;
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].tag1cTag        = 0xffff            ;
                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].up[kk]          = kk                ;
                    /* set the 8 related EPCL rule to classify the egress poort and UP */
                    /* This API set the rule index in the DB */
                    if (kk==0)
                    {
                        rc = appDemoTmPortDefaultRulesSet(
                                appDemoEgressPortDb[portNumIndex].egressPort,
                                pclDirection                    ,
                                appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNode,
                                ruleFormat                      ,
                                vTcamPclMngId                   ,
                                vTcamId                         ,
                                vTcamRuleId                     ,
                                queueId                         );
                        if(rc !=GT_OK)
                        {
                            cpssOsPrintf("appDemoTmPortDefaultRulesSet fails rc = %d for port %d aNode %d vTcamRuleId %d\n",rc,
                                    appDemoEgressPortDb[portNumIndex].egressPort,
                                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNode,vTcamRuleId);
                            return rc;
                        }
                        else if (printOut)
                        {
                            cpssOsPrintf("appDemoTmPortDefaultRulesSet passed for port %d aNode %d vTcamRuleId %d\n",
                                    appDemoEgressPortDb[portNumIndex].egressPort,
                                    appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNode,vTcamRuleId );
                        }
                    }
                }
                break; /* Set only for first index*/
            } /* for jj */
            break; /* Set only for 1 A-node */
        }/* for ii */
    }/* for portIndex */
    return GT_OK;
}

/**
 * @internal appDemoVTcamUdbSet function
 * @endinternal
 *
 * @brief This API sets UDB for PCL key
 *
 * @note  APPLICABLE DEVICES    - Caelum
 *
 * @param[in] devNum       - device number
 * @param[in] numOfUDB     - number of UDBs
 * @param[in] pclDirection - PCL direction
 * @param[in] pclLookupNum - PCL lookup number
 * @param[in] ruleFormat   - PCL rule format
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoVTcamUdbSet
(
    IN GT_U8                                devNum      ,
    IN GT_U32                               numOfUDB    ,
    IN CPSS_PCL_DIRECTION_ENT               pclDirection,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT           pclLookupNum,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat
)
{

    /* For PCL */
    CPSS_DXCH_PCL_PACKET_TYPE_ENT           packetType      ;
    /* For IPCL rules */
    GT_U32                                  ii              ;
    CPSS_DXCH_PCL_UDB_SELECT_STC            udbSelectStruct ;
    GT_STATUS                               rc              ;

    if (printOut)
        cpssOsPrintf("enter appDemoVTcamUdbSet \n");

    /* set UDB's */
    osMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    /*
       it's a must to "turn on" UDB's
       since most of the UDB are based on metadata and they must strt in index > 30
       not all teh UDBs are in used and therefor enot in the list
    */
    for(ii = 0; ii < numOfUDB ; ii++)
    {
        udbSelectStruct.udbSelectArr[ii] = appDemoEpclUdb[ii].udbIndex;
        udbSelectStruct.egrUdb01Replace    = GT_FALSE                   ;
        udbSelectStruct.egrUdbBit15Replace = GT_FALSE                   ;
    }


    /* loop over different packet types so different kind of packets will be hitted */
    for(packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E ; packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E;packetType++)
    {
        switch (packetType)
        {
            case CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E :
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E       :
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E       :
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E     :
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E       :
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E       :
                rc = cpssDxChPclUserDefinedBytesSelectSet(devNum,ruleFormat,packetType,pclLookupNum,&udbSelectStruct);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssDxChPclUserDefinedBytesSelectSet failed rc = %d\n",rc);
                    return rc;
                }
                else if(printOut)
                {
                    cpssOsPrintf("cpssDxChPclUserDefinedBytesSelectSet sets the ruleFormat %d for packetType %d\n",ruleFormat,packetType);
                }

                if(pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
                {
                    for(ii = 0; ii < numOfUDB; ii++)
                    {


                        rc = cpssDxChPclUserDefinedByteSet(devNum               ,
                                                ruleFormat                      , /* irrelevant for BC2 */
                                                packetType                      ,
                                                pclDirection                    ,
                                                appDemoIpclUdb[ii].udbIndex     ,
                                                appDemoIpclUdb[ii].pclOffsetType,
                                                appDemoIpclUdb[ii].pclOffset   );
                        if( GT_OK != rc)
                        {
                            cpssOsPrintf("\n\ncpssDxChPclUserDefinedByteSet failed to set UDB i=%d rc = %d\n",ii,rc);
                            return rc;
                        }
                    }
                }
                else if(pclDirection == CPSS_PCL_DIRECTION_EGRESS_E)
                {
                    for(ii = 0; ii < numOfUDB; ii++)
                    {
                        rc = cpssDxChPclUserDefinedByteSet(devNum               ,
                                                ruleFormat                      , /* irrelevant for BC2 */
                                                packetType                      ,
                                                pclDirection                    ,
                                                appDemoEpclUdb[ii].udbIndex     ,
                                                appDemoEpclUdb[ii].pclOffsetType,
                                                appDemoEpclUdb[ii].pclOffset   );
                        if( GT_OK != rc)
                        {
                            cpssOsPrintf("\n\ncpssDxChPclUserDefinedByteSet failed to set UDB i=%d rc = %d\n",ii,rc);
                            return rc;
                        }
                    }
                }
                else
                    return GT_BAD_PARAM;

            break;
            default:
                continue;
                break;
        }
    }

    return GT_OK;
}

/**
 * @internal appDemoVTcamCreation  function
 * @endinteral
 *
 * @brief  This function creates a vTCAM per givven rule size
 *         The methid that is used is priority and auto resize
 *
 * @note   APPLICABLE DEVICES  - Caelum
 *
 * @param[in]   devNum      - device number
 * @param[in]   vTcamMngId  - virtual TCAM Manager ID
 * @param[in]   vTcamId     - virtual TCAM ID
 * @param[in]   ruleSize    - rule size
 *
 * @retval     GT_OK   - on success
 */
GT_STATUS appDemoVTcamCreation
(
    IN GT_U8                                    devNum      ,
    IN GT_U32                                   vTcamMngId  ,
    IN GT_U32                                   vTcamId     ,
    IN CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT     ruleSize
)
{
    GT_STATUS                                       rc;
    /* Virtual TCAM manager configuration */
    GT_U8                                           devListArr[1]   ;
    GT_U32                                          numOfDevs = 1   ;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC                 vTcamInfo       ;
    CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC     vTcamMngCfg     ;

    cpssOsMemSet(&vTcamMngCfg,0x0,sizeof(vTcamMngCfg));

    vTcamMngCfg.haFeaturesEnabledBmp    = 0x0       ;
    vTcamMngCfg.haSupportEnabled        = GT_FALSE  ;

    /* Create VTcam manager instance, add to device */
    devListArr[0] = devNum;

    rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId, &vTcamMngCfg);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChVirtualTcamManagerCreate FAILED, rc = [%d]\n", rc);
        return rc;
    }
    else if (printOut)
        cpssOsPrintf("cpssDxChVirtualTcamManagerCreate passed \n");


    rc = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChVirtualTcamManagerDevListAdd FAILED, rc = [%d]\n", rc);
        return rc;
    }
    else if (printOut)
        cpssOsPrintf("cpssDxChVirtualTcamManagerDevListAdd passed \n");

    /* Create vTcam on the newly created vTcam manager */
    /* vTcam attributes:
        - Assigned to PCL clientGroup, parallel lookup 0.
        - Created in Priority mode, with automatic resize.
        - Created with 0 guaranteed rules, so that it grows on demand.
    */
    cpssOsMemSet (&vTcamInfo, 0, sizeof(vTcamInfo));
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
        {
            switch (lookupNum)
            {
                case CPSS_PCL_LOOKUP_0_E:
                case CPSS_PCL_LOOKUP_NUMBER_0_E:
                    vTcamInfo.clientGroup = 1;
                    break;
                case CPSS_PCL_LOOKUP_NUMBER_1_E:
                    vTcamInfo.clientGroup = 2;
                    break;
                case CPSS_PCL_LOOKUP_NUMBER_2_E:
                    vTcamInfo.clientGroup = 3;
                    break;
            }
        }
        else
        {
            vTcamInfo.clientGroup = 3;
        }
    }
    else
    {
        /* Default appDemo assignment is clientGroup 0 for all PCL stages */
        vTcamInfo.clientGroup = 0;
    }
    vTcamInfo.hitNumber             = 0                                                     ;
    vTcamInfo.guaranteedNumOfRules  = 0                                                     ;
    vTcamInfo.autoResize            = GT_TRUE                                               ;
    vTcamInfo.ruleSize              = ruleSize;
    vTcamInfo.ruleAdditionMethod    = CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

    rc = cpssDxChVirtualTcamCreate(vTcamMngId,vTcamId,&vTcamInfo);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChVirtualTcamCreate FAILED, rc = [%d]\n", rc);
        return rc;
    }
    else if (printOut)
    {
        cpssOsPrintf("cpssDxChVirtualTcamCreate passed\n");
        cpssOsPrintf("Result: %d \n", rc);
        cpssOsPrintf("vTcam[MngId/Id] %d /%d created successfully! \n",vTcamMngId,  vTcamId);
    }

    return rc;
}

/**
 * @internal appDemoTmVTcamDelete  function
 * @endinteral
 *
 * @brief  This function delete a vTCAM Manager
 *
 * @note   APPLICABLE DEVICES  - Caelum
 *
 * @param[in]   devNum      - device number
 * @param[in]   vTcamMngId  - virtual TCAM Manager ID
 * @param[in]   vTcamId     - virtual TCAM ID
 * @param[in]   ruleSize    - rule size
 *
 * @retval     GT_OK   - on success
 */
GT_STATUS appDemoTmVTcamDelete
(
    IN GT_U8                                    devNum      ,
    IN GT_U32                                   vTcamMngId  ,
    IN GT_U32                                   vTcamId
)
{
    GT_STATUS                                       rc;
    /* Virtual TCAM manager configuration */
    GT_U8                                           devListArr[1]   ;
    GT_U32                                          numOfDevs = 1   ;
    GT_U32      ii, jj, kk   ;
    GT_U32      portNumIndex ;
    GT_U32      vTcamRuleId  ;

    /* Create VTcam manager instance, add to device */
    devListArr[0] = devNum;

    /* Delete all default vTCAM rules */
    for(portNumIndex = 0; portNumIndex < numOfTmPorts ; portNumIndex++)
    {
        for (ii = 0 ; ii < 8 ; ii++)/* loop over cNode */
        {
            for (jj = 0 ; jj < 8 ; jj++)/* loop over aNode */
            {
                for(kk = 0; kk < 8; kk++)/* loop over queue */
                {
                    vTcamRuleId = appDemoEgressPortDb[portNumIndex].cNodeTree[ii].flow2aNode[jj].aNodeAttr[0].vTcamRuleId[kk];
                    rc = appDemoTmRulesPerFlowRemove(vTcamMngId, vTcamId, vTcamRuleId);
                    if (rc != GT_OK)
                    {
                        cpssOsPrintf("appDemoTmRulesPerFlowRemove failed: portNumidx=%d, vTcamRuleId = %d\n", portNumIndex, vTcamRuleId);
                        return rc;
                    }
                    break;
                }
                break;
            }
            break;
        }
    }

    rc = cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChVirtualTcamRemove FAILED, rc = [%d]\n", rc);
        return rc;
    }
    else if (printOut)
    {
        cpssOsPrintf("cpssDxChVirtualTcamRemove passed\n");
        cpssOsPrintf("vTcam[MngId/Id] %d /%d removed successfully! \n",vTcamMngId,  vTcamId);
    }

    rc = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChVirtualTcamManagerDevListRemove FAILED, rc = [%d]\n", rc);
        return rc;
    }
    else if (printOut)
        cpssOsPrintf("cpssDxChVirtualTcamManagerDevListRemove passed \n");


    rc = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChVirtualTcamManagerDelete FAILED, rc = [%d]\n", rc);
        return rc;
    }
    else if (printOut)
        cpssOsPrintf("cpssDxChVirtualTcamManagerDelete passed \n");

    return rc;
}

/**
 * @internal  appDemoPclInit function
 * @endinternal
 *
 * @brief  This API initializes PCL and configure
 *
 * @note  APPLICABLE DEVICES  - Caelum
 *
 * @param[in] devNum         - device number
 * @param[in] vTcamPclMngId  - virtual TCAM Manager PCL ID
 * @param[in] vTcamId        - virtual TCAM ID
 * @param[in] pclId          - PCL Id
 * @param[in] pclDirection   - PCL direction
 * @param[in] ruleFormat     - rule format
 * @param[in] ruleSize       - rule size
 * @param[in] pclLookupNum   - PCL Lookup number
 *
 * @retval GT_OK  -  on success
 */
GT_STATUS appDemoPclInit
(
    IN GT_U8                                devNum          ,
    IN GT_U32                               vTcamPclMngId   ,
    IN GT_U32                               vTcamId         ,
    IN GT_U32                               pclId           ,
    IN CPSS_PCL_DIRECTION_ENT               pclDirection    ,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat      ,
    IN CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT ruleSize        ,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT           pclLookupNum
)
{
    GT_U32                                  ii;
    GT_STATUS                               rc                  ;
    GT_U32                                  numOfUDB =6         ;
    CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT       egressPacketType;

    if (printOut)
        cpssOsPrintf("enter appDemoPclInit \n");

    if (pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        rc = cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChPclIngressPolicyEnable FAILED, rc = [%d]\n", rc);
            return rc;
        }
        else if (printOut)
            cpssOsPrintf("cpssDxChPclIngressPolicyEnable is OK \n");
    }
    else if (pclDirection == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        rc = cpssDxCh2PclEgressPolicyEnable (devNum,GT_TRUE);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxCh2PclEgressPolicyEnable FAILED, rc = [%d]\n", rc);
            return rc;
        }
        else if (printOut)
            cpssOsPrintf("cpssDxCh2PclEgressPolicyEnable is OK \n");
    }

    /* Create vTCAM */
    rc = appDemoVTcamCreation (devNum,vTcamPclMngId  ,vTcamId ,ruleSize);
    if (GT_OK != rc)
    {
        cpssOsPrintf("appDemoVTcamCreation FAILED on rc = [%d]\n", rc);
        return rc;
    }
    else if (printOut)
        cpssOsPrintf("appDemoVTcamCreation is set with vTcamMngId %d vTcamId %d and ruleSize %d \n",vTcamPclMngId,vTcamId ,ruleSize);


    /* Create vTCAM */
    rc = appDemoVTcamUdbSet  (devNum ,numOfUDB,pclDirection,pclLookupNum,ruleFormat);
    if (GT_OK != rc)
    {
        cpssOsPrintf("appDemoVTcamUdbSet FAILED on rc = %d\n", rc);
        return rc;
    }
    else if (printOut)
        cpssOsPrintf("appDemoVTcamUdbSet is set \n");
    /* set the UDB for all packet types */
    for (ii = 0; ii < numOfTmPorts ;ii++)
    {
        /* enable all ports for I/EPCL and set PCL configuration table */
        rc = appDemoPclConfigurationTableSet (devNum,tmEnablePhysicalPortList[ii],ruleFormat,pclDirection,
                                                pclLookupNum,pclId);

        if (GT_OK != rc)
        {
            cpssOsPrintf("appDemoPclConfigurationTableSet FAILED on rc = [%d]\n", rc);
            return rc;
        }
        else if (printOut)
        {
            cpssOsPrintf("appDemoPclConfigurationTableSet is set for port %d\n",tmEnablePhysicalPortList[ii]);
        }

        for (egressPacketType = CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E ; egressPacketType < CPSS_DXCH_PCL_EGRESS_PKT_DROP_E; egressPacketType++)
        {
            rc = cpssDxCh2EgressPclPacketTypesSet(devNum, tmEnablePhysicalPortList[ii], egressPacketType,GT_TRUE);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChPclPortIngressPolicyEnable FAILED for port %d, rc = [%d]\n",tmEnablePhysicalPortList[ii], rc);
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
 * @internal  appDemoTmScenario17PortInit function
 * @endinternal
 *
 * @brief Initialize Traffic Manager for 17 ports scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK   -  on success
 */
GT_STATUS appDemoTmScenario17PortInitTmAndPcl
(
    IN GT_U8   devNum,
    IN GT_BOOL onlyTmInit
)
{
    GT_STATUS                   rc              = GT_OK                 ;
    GT_U32                      caelumEpclId    = CAELUM_EPCL_ID        ;
    GT_U32                      vTcamPclMngId   = VTCAM_PCL_MNG_ID      ;
    GT_U32                      vTcamId         = VTCAM_ID              ;
    CPSS_TM_LEVEL_PERIODIC_PARAMS_STC       params[5];
    CPSS_TM_LEVEL_ENT           tmLevel;

    if (printOut)
        cpssOsPrintf("enter appDemoTmScenario17PortInit \n");

    appDemoTmDbReset(devNum);

    rc = cpssTmAgingChangeStatus(devNum, 0);
    if (rc != GT_OK)
        return rc;

    /*****************************************************/
    /* Configure Periodic Scheme                         */
    /*****************************************************/

    for (tmLevel = CPSS_TM_LEVEL_Q_E; tmLevel <= CPSS_TM_LEVEL_P_E; tmLevel++)
    {
        params[tmLevel].periodicState = GT_TRUE;
        params[tmLevel].shaperDecoupling = GT_FALSE; /* should be parametrized ?*/
    }

    rc = cpssTmSchedPeriodicSchemeConfig(devNum, params);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssTmSchedPeriodicSchemeConfig", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (!onlyTmInit) {
        rc = appDemoPclInit(devNum,
                            vTcamPclMngId                               ,
                            vTcamId                                     ,
                            caelumEpclId                                ,
                            CPSS_PCL_DIRECTION_EGRESS_E                 ,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E   ,
                            CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E     ,
                            CPSS_PCL_LOOKUP_0_E);

        if (GT_OK != rc)
        {
            cpssOsPrintf("appDemoPclInit FAILED, rc = [%d]\n", rc);
            return rc;
        }
        else if (printOut)
            cpssOsPrintf("appDemoPclInit is done \n");
    }

    rc = appDemoTmFlatQoSAllPortsSet (   devNum                                      ,
                                        CPSS_PCL_DIRECTION_EGRESS_E                 ,
                                        CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E   ,
                                        vTcamPclMngId                               ,
                                        vTcamId);
    if (GT_OK != rc)
    {
        cpssOsPrintf("appDemoTmFlatQoSAllPortsSet FAILED, rc = [%d]\n", rc);
        return rc;
    }
    else if (printOut)
        cpssOsPrintf("appDemoTmFlatQoSAllPortsSet is done \n");

    return GT_OK;
}

/**
 * @internal  appDemoTmScenario17PortInit function
 * @endinternal
 *
 * @brief Initialize Traffic Manager for 17 ports scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK   -  on success
 */
GT_STATUS appDemoTmScenario17PortInit
(
    IN GT_U8   devNum
)
{
    return appDemoTmScenario17PortInitTmAndPcl(devNum, GT_FALSE);
}
/**
 * @internal  appDemoTmScenario17PortConfigSet function
 * @endinternal
 *
 * @brief  Configure Traffic Manager for 17 port scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum  - device number
 *
 * @retval   GT_OK  - on success
 */
GT_STATUS appDemoTmScenario17PortConfigSet
(
    GT_U8   devNum
)
{
    GT_STATUS  rc = GT_OK;
    GT_U32     floorIndex = 0;
    GT_U32     maxFloorIndex = 6;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];

    if (printOut)
        cpssOsPrintf("enter appDemoTmScenario17PortConfigSet \n");

    cpssOsMemSet(floorInfoArr, 0, sizeof(floorInfoArr));

    for (floorIndex = 0; floorIndex < maxFloorIndex; floorIndex++)
    {
        rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, floorIndex, floorInfoArr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = appDemoTcam2CncBlockSet(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
 * @internal  appDemoTmScenario17PortSchedulerDelete function
 * @endinternal
 *
 * @brief Delete Scheduler nodes for 17 ports scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK   -  on success
 */
GT_STATUS appDemoTmScenario17PortSchedulerDelete
(
    IN GT_U8   devNum
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portNum;

    /* Delete all default vTCAM rules */
    for(portNum = 0; portNum < numOfTmPorts ; portNum++)
    {
        rc = appDemoTMtreeManipulationSingleAnodePerPortDelete(devNum, portNum, 1);
        if (rc != GT_OK)
        {
            cpssOsPrintf("appDemoTMtreeManipulationSingleAnodePerPortDelete Failed, portNum = %d, rc = %d", portNum, rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
 * @internal  appDemoTmScenario17PortVTcamDelete function
 * @endinternal
 *
 * @brief Delete VTCAM Manager for 17 ports scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK   -  on success
 */
GT_STATUS appDemoTmScenario17PortVTcamDelete
(
    IN GT_U8   devNum
)
{
    return appDemoTmVTcamDelete(devNum, VTCAM_PCL_MNG_ID, VTCAM_ID);
}

/**
 * @internal appDemoTmDbReset function
 * @endinternal
 *
 * @brief Reset appDemo Database
 *
 * @param[in] devNum  - device Number
 *
 * @retval GT_OK    - on success
 */
GT_STATUS appDemoTmDbReset
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    (void) devNum;

    cpssOsMemSet(appDemoEgressPortDb, 0, sizeof(appDemoEgressPortDb));
    cpssOsMemSet(queuePool, 0, sizeof(queuePool));
    cpssOsMemSet(vTcamRuleIdPool, 0, sizeof(vTcamRuleIdPool));

    return rc;
}
