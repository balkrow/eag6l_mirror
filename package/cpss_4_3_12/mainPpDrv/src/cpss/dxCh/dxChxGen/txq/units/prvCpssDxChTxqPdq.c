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
 * @file prvCpssDxChTxqPdq.c
 *
 * @brief CPSS SIP6 TXQ PDQ low level configurations.
 *
 * @version   1
 ********************************************************************************
 */
/*Synced to \Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.5} \TXQ_PSI \TXQ_PDQ */
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedCtl.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCtlInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegs.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSched.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedShaping.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesTree.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesCreate.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <errno.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesRead.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesUpdate.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrors.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedGetGenParamInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedEligPrioFunc.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssEligFuncDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedNodesCtlInternal.h>



#define PRV_CPSS_PSI_FALCON_SCHED_OFFSET 0x200
#define PRV_CPSS_PSI_HAWK_SCHED_OFFSET   0x300
#define PRV_CPSS_PSI_AAS_SCHED_OFFSET    0x00100000

#define PRV_CPSS_MAX_TB_SIZE 0xFFF


#define PRECISION_FACTOR 1000
#define PRV_CPSS_LEVEL_CONVERT_TO_TM_FORMAT_MAC(_level) (( PRV_CPSS_PDQ_LEVEL_Q_E==_level) ? SCHED_Q_LEVEL : \
    (( PRV_CPSS_PDQ_LEVEL_A_E==_level)?SCHED_A_LEVEL:\
    (( PRV_CPSS_PDQ_LEVEL_B_E==_level)?SCHED_B_LEVEL:\
    (( PRV_CPSS_PDQ_LEVEL_C_E==_level)?SCHED_C_LEVEL:SCHED_P_LEVEL))\
    ))

#define PRV_CPSS_ELIG_FUNC_GET_MAC(_tc,_shaping)  (GT_TRUE == _shaping)?\
        PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(_tc):\
        PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(_tc)

#define PRV_CPSS_PRIORITY_GROUP_INDEX_GET_MAC(_priorityGroupIndex,_elig_prio_func_ptr) \
 do\
 {\
     switch(_elig_prio_func_ptr)\
     {\
         case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(0):\
         case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(0):\
          _priorityGroupIndex = 0;\
          break;\
         case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(1):\
         case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(1):\
          _priorityGroupIndex = 1;\
          break;\
         case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(2):\
         case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(2):\
          _priorityGroupIndex = 2;\
          break;\
         case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(3):\
         case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(3):\
          _priorityGroupIndex = 3;\
          break;\
         case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(4):\
          case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(4):\
           _priorityGroupIndex = 4;\
           break;\
          case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(5):\
          case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(5):\
           _priorityGroupIndex = 5;\
           break;\
          case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(6):\
          case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(6):\
           _priorityGroupIndex = 6;\
           break;\
          case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(7):\
          case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(7):\
           _priorityGroupIndex = 7;\
           break;\
         default:\
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " elig_prio_func_ptr %d not valid \n",_elig_prio_func_ptr);\
         break;\
     }\
 }while(0);\


#define PRV_CPSS_PRIORITY_FUNC_CONFIGURE_GET_MAC(_priorityGroupIndex,_shapingEnabled,_elig_prio_func_ptr) \
      do\
      {\
          switch(_priorityGroupIndex)\
          {\
              case 0:\
               _elig_prio_func_ptr = PRV_CPSS_ELIG_FUNC_GET_MAC(0,_shapingEnabled);\
               break;\
              case 1:\
               _elig_prio_func_ptr = PRV_CPSS_ELIG_FUNC_GET_MAC(1,_shapingEnabled);\
               break;\
              case 2:\
               _elig_prio_func_ptr = PRV_CPSS_ELIG_FUNC_GET_MAC(2,_shapingEnabled);\
               break;\
              case 3:\
               _elig_prio_func_ptr = PRV_CPSS_ELIG_FUNC_GET_MAC(3,_shapingEnabled);\
               break;\
              case 4:\
               _elig_prio_func_ptr = PRV_CPSS_ELIG_FUNC_GET_MAC(4,_shapingEnabled);\
               break;\
              case 5:\
               _elig_prio_func_ptr = PRV_CPSS_ELIG_FUNC_GET_MAC(5,_shapingEnabled);\
               break;\
              case 6:\
               _elig_prio_func_ptr = PRV_CPSS_ELIG_FUNC_GET_MAC(6,_shapingEnabled);\
               break;\
              case 7:\
               _elig_prio_func_ptr = PRV_CPSS_ELIG_FUNC_GET_MAC(7,_shapingEnabled);\
               break;\
              default:\
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " priorityGroupIndex %d not valid \n",_priorityGroupIndex);\
              break;\
          }\
      }while(0);\

#define PRV_CPSS_ELIG_FUNC_TO_STR(_eligFunc)   case _eligFunc: \
                                                namePtr = #_eligFunc; \
                                               break;\


#define PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(_eligFunc,_description)  case _eligFunc: \
                                                                    descPtr =_description; \
                                                                   break;\

#define PRV_CPSS_ELIG_FUNC_GROUP_GET(_eligFunc) (((GT_U32)_eligFunc)>>3)

/*See   prvCpssTxqSchedulerGetUnit for unit defenitions*/
/*See   prvSchedShapingUtilsCalculateNodeShapingNoBurstSizeAdaption for shaping calculations*/
/*See   prvSchedInitDefaultQueueEligPrioFuncTable for  eligeble function defenition*/


extern GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
);

extern GT_U32 prvCpssTxqUtilsCreditSizeGet
(
    IN GT_U8 devNum
);

extern  GT_STATUS prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN GT_U32 dpNum,
    IN GT_U32 localPortNum,
    OUT GT_U32 * indexPtr,
    IN GT_BOOL isErrorForLog
);


static GT_CHAR_PTR prvCpssSip6TxqEligFuncNameGet
(
    unsigned int eligFunc
);

static GT_CHAR_PTR prvCpssSip6TxqQueueEligFuncNameGet
(
    unsigned int eligFunc
);

static GT_CHAR_PTR prvCpssSip6TxqQueueEligFuncDescriptionGet
(
    unsigned int eligFunc
);

static GT_STATUS prvCpssSip6TxqPdqInitAdressSpace
(
    IN GT_U32 tileNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr
);


static GT_U32 prvSchedQueueEligFuncInfoGet
(
    IN prvShedEligFuncQueue  eligFunc,
    OUT GT_BOOL             *shapingPtr,
    OUT GT_BOOL             *minBwPtr,
    OUT GT_U32              *minBwTypePtr
)
{
    GT_U32 priorityGroup =((GT_U32)eligFunc)&0x7;

    if(PRV_SCHED_ELIG_Q_DEQ_DIS_E==eligFunc)
    {
        if(shapingPtr)
        {
            *shapingPtr = GT_FALSE;
        }
        if(minBwPtr)
        {
         *minBwPtr = GT_FALSE;
        }
        if(minBwTypePtr)
        {
            *minBwTypePtr = GT_FALSE;
        }
        /*we are not suppose to get here*/
        return 0;
    }

    if((PRV_CPSS_ELIG_FUNC_GROUP_GET(eligFunc)==PRV_SCHED_ELIG_FUNC_GROUP_NORMAL_E)||
        (PRV_CPSS_ELIG_FUNC_GROUP_GET(eligFunc)==PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE0_E)||
        (PRV_CPSS_ELIG_FUNC_GROUP_GET(eligFunc)==PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE1_E))
    {
        *shapingPtr = GT_FALSE;
    }
    else
    {
         *shapingPtr = GT_TRUE;
    }

    if(minBwPtr!=NULL)
    {
        if(PRV_CPSS_ELIG_FUNC_GROUP_GET(eligFunc)<=PRV_SCHED_ELIG_FUNC_GROUP_SHAPING_E)
        {
           *minBwPtr = GT_FALSE;
        }
        else
        {
           *minBwPtr = GT_TRUE;
           if(minBwTypePtr!=NULL)
           {
               if((PRV_CPSS_ELIG_FUNC_GROUP_GET(eligFunc)==PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE0_WITH_SHAPING_E)||
                (PRV_CPSS_ELIG_FUNC_GROUP_GET(eligFunc)==PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE0_E))
               {
                 *minBwTypePtr = 0;
               }
               else
               {
                 *minBwTypePtr = 1;
               }
          }

        }
     }

    return priorityGroup;
}


static GT_STATUS prvSchedToCpssErrCodeConvert
(
    IN GT_U32 errorCode
)
{
    GT_STATUS rc;


    if(0==errorCode)
    {
        rc= GT_OK;
    }
    else if(errorCode>=1&&errorCode<=TM_HW_MAX_ERROR)
    {
         rc = GT_FAIL;
    }
    else if (errorCode==TM_BW_UNDERFLOW)
    {
        rc = GT_SHAPER_GRANULARITY_OUT_OF_RANGE;
    }
    else if((errorCode>=TM_CONF_INVALID_PROD_NAME&&errorCode<=TM_CONF_PER_RATE_L_K_N_NOT_FOUND)||
        (errorCode==TM_CONF_SHAPING_PROF_REF_OOR)||(errorCode==TM_WRONG_SHP_PROFILE_LEVEL)||
        (errorCode==TM_CONF_WRONG_LOGICAL_NAME)||(errorCode==TM_CONF_NULL_LOGICAL_NAME))
    {
         rc = GT_BAD_VALUE;
    }
    else if((errorCode>=TM_CONF_PORT_IND_OOR&&errorCode<=TM_CONF_TM2TM_PORT_FOR_CTRL_PKT_OOR)||
        (errorCode==TM_BW_OUT_OF_RANGE))
    {
         rc = GT_OUT_OF_RANGE;
    }
    else if(errorCode>=TM_CONF_PORT_BW_OUT_OF_SPEED&&errorCode<=TM_CONF_REORDER_NODES_NOT_ADJECENT)
    {
         rc = GT_BAD_PARAM;
    }
    else if(errorCode>=TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE&&errorCode<=TM_CONF_REORDER_CHILDREN_NOT_AVAIL)
    {
         rc = GT_BAD_SIZE;
    }
    else if(errorCode>=TM_CONF_PORT_IND_NOT_EXIST&&errorCode<=TM_CONF_C_NODE_IND_NOT_EXIST)
    {
         rc = GT_BAD_STATE;
    }
    else if(errorCode>=TM_CONF_UPD_RATE_NOT_CONF_FOR_LEVEL&&errorCode<=TM_CONF_TM2TM_CHANNEL_NOT_CONFIGURED)
    {
         rc = GT_NOT_INITIALIZED;
    }
    else
    {
        switch(errorCode)
        {
            case TM_CONF_PORT_IND_USED:
              rc = GT_ALREADY_EXIST;
              break;
            case TM_CONF_CANNT_GET_LAD_FREQUENCY:
              rc = GT_GET_ERROR;
              break;
            default:
              rc = GT_FAIL;
              break;
        }
     }

    return rc;

}


static GT_STATUS prvSchedToCpssNegErrCodeConvert
(
    IN GT_U32 errorCode
)
{
    GT_STATUS rc;

    switch(errorCode)
    {

        case EBADF:
        case EACCES:
        case EADDRNOTAVAIL:
        case EDOM:
        case EFAULT:
        case ENODATA:
        case ENODEV:
        case ERANGE:
        case EPERM:
          rc = GT_BAD_PARAM;
          break;
        case EBUSY:
          rc = GT_BAD_STATE;
          break;
        case EBADMSG:
          rc = GT_NOT_INITIALIZED;
          break;
        case ENOBUFS:
        case ENOMEM:
          rc = GT_NO_RESOURCE;
          break;
        case EINVAL:
          rc = GT_BAD_PTR;
          break;
        case ENOSPC:
          rc = GT_FULL;
          break;
        default:
          rc= GT_FAIL;
          break;
    }

    return rc;

}


/* convert errors from xel code to CPSS error code */
/* and support for the CPSS ERROR LOG */
GT_STATUS falcon_xel_to_cpss_err_code
(
    IN GT_U8 devNum,
    IN int retCode_xel,
    IN GT_U32 pdqNum,
    IN const char * funcName,
    IN GT_U32 lineNum
)
{
    GT_STATUS rc ;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedLibIsInitilized[pdqNum] == GT_FALSE)
    {
        rc = GT_NOT_INITIALIZED;
    }
    else
    {
        if( (retCode_xel) < 0)
        {
            retCode_xel*=(-1);
            rc = prvSchedToCpssNegErrCodeConvert(retCode_xel);
        }
        else
        {
            rc = prvSchedToCpssErrCodeConvert(retCode_xel);
        }
    }

    if (rc != GT_OK)
    {
#ifndef CPSS_LOG_ENABLE
        funcName    = funcName;/*avoid warning of unused parameter */
        lineNum     = lineNum;
#endif/*!CPSS_LOG_ENABLE*/
        CPSS_TXQ_DEBUG_EXT_LOG("[device %d pdq %d ] Convert scheduler error %d to CPSS status %d.\n",funcName,devNum,pdqNum,retCode_xel,rc);

        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "[%s , line[%d]] failed : in pdqNum[%d] (retCode_xel[%d])",
                          funcName, lineNum, pdqNum, retCode_xel);
    }
    return rc;
}
static GT_VOID prvCpssSchedPortParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC     *paramsPtr,
    OUT struct sched_port_params       *prmsPtr
);
static GT_VOID prvCpssSchedCNodeParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC      *paramsPtr,
    OUT struct sched_c_node_params        *prmsPtr
);
static GT_VOID prvCpssSchedANodeParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC      *paramsPtr,
    OUT struct sched_a_node_params        *prmsPtr
);
static GT_VOID prvCpssSchedBNodeParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC      *paramsPtr,
    OUT struct sched_b_node_params        *prmsPtr
);
static GT_VOID prvCpssSchedQueueParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC      *paramsPtr,
    OUT struct sched_queue_params        *prmsPtr
);



GT_STATUS prvCpssSip6TxqPdqSchedulerHwParamsGet
(
    IN GT_U32 devNum,
    OUT PRV_CPSS_SCHED_HW_PARAMS_STC * pdqSchedHwParamsPtr
)
{
    const PRV_CPSS_SCHED_HW_PARAMS_STC pdqSchedHwParamsFalcon =
    {
        2048,   /* maxQueues */
        512,    /* maxAnodes */
        128,    /* maxBnodes */
        128,    /* maxCnodes */
        128     /* maxPorts  */
    };
    const PRV_CPSS_SCHED_HW_PARAMS_STC pdqSchedHwParamsPhoenix =
    {
        512,    /* maxQueues */
        64,     /* maxAnodes */
        64,     /* maxBnodes */
        64,     /* maxCnodes */
        64      /* maxPorts  */
    };

    const PRV_CPSS_SCHED_HW_PARAMS_STC pdqSchedHwParamsSeahawk =
    {
        4096,   /* maxQueues */
        512,    /* maxAnodes */
        128,    /* maxBnodes */
        128,    /* maxCnodes */
        128 /*cider -73*/      /* maxPorts  */
    };

    CPSS_NULL_PTR_CHECK_MAC(pdqSchedHwParamsPtr);
    if (GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
        *pdqSchedHwParamsPtr = pdqSchedHwParamsSeahawk;
    }
    else if (GT_TRUE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        *pdqSchedHwParamsPtr = pdqSchedHwParamsPhoenix;
    }
    else
    {
        *pdqSchedHwParamsPtr= pdqSchedHwParamsFalcon;
    }

    return GT_OK;

}

GT_STATUS  prvCpssSip6TxqPdqSchedulerHwParamsByLevelGet
(
    IN GT_U32 devNum,
    OUT  GT_U32 *maxPnodePtr,
    OUT  GT_U32 *maxCnodePtr,
    OUT  GT_U32 *maxBnodePtr,
    OUT  GT_U32 *maxAnodePtr,
    OUT  GT_U32 *maxQnodePtr
)
{
    PRV_CPSS_SCHED_HW_PARAMS_STC hwMaxSize,*hwMaxSizePtr;
    GT_STATUS rc;

    hwMaxSizePtr= &hwMaxSize;

    rc = prvCpssSip6TxqPdqSchedulerHwParamsGet(devNum, hwMaxSizePtr);
    if(rc!=GT_OK)
    {
        return rc;
    }

    if(maxPnodePtr)
    {
        *maxPnodePtr = hwMaxSizePtr->maxPorts;
    }
    if(maxCnodePtr)
    {
        *maxCnodePtr = hwMaxSizePtr->maxCnodes;
    }
    if(maxBnodePtr)
    {
        *maxBnodePtr = hwMaxSizePtr->maxBnodes;
    }
    if(maxAnodePtr)
    {
        *maxAnodePtr = hwMaxSizePtr->maxAnodes;
    }
    if(maxQnodePtr)
    {
       *maxQnodePtr = hwMaxSizePtr->maxQueues;
    }

    return GT_OK;

}

/**
 * @internal prvCpssFalconTxqPdqInit function
 * @endinternal
 *
 * @brief   Initialize the PDQ scheduler configuration library.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 */
GT_STATUS prvCpssFalconTxqPdqInit
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 mtu
)
{
    GT_STATUS                   rc          = GT_OK;
    struct PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC   tm_lib_init_params  = { 0, 0 };
    struct PRV_CPSS_SCHED_TREE_MAX_SIZES_STC    pdqSchedTreeParams;
    PRV_CPSS_SCHED_HW_PARAMS_STC *          pdqSchedHwParamsPtr;
    PRV_CPSS_SCHED_HW_PARAMS_STC            pdqSchedHwParams;
    int                     ret = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION *tilePtr;
    PRV_CPSS_DEV_CHECK_MAC(devNum);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdqNum);
    pdqSchedHwParamsPtr= &pdqSchedHwParams;

    if(0==pdqNum)
    {
        rc = prvSchedShapingBurstOptimizationSet(devNum,CPSS_DXCH_TXQ_SHAPER_TYPE_CIR_E,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvSchedShapingBurstOptimizationSet(devNum,CPSS_DXCH_TXQ_SHAPER_TYPE_EIR_E,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvSchedShapingLogResultSet(devNum,GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    tilePtr = PRV_CPSS_TILE_HANDLE_GET_MAC(devNum,pdqNum);

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedLibIsInitilized[pdqNum] == GT_TRUE)
    {
        /*TM_DBG_INFO(("---- cpssTmInit:tm hndl already exist and tm_lib was invoked, ignoring ...\n"));*/
        return GT_OK;
    }
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.regDataBaseInitialized == GT_FALSE&&
        0==pdqNum)
    {
        rc = prvCpssSip6TxqPdqInitAdressSpace(pdqNum,tilePtr);
        if(rc != GT_OK)
        {
          return rc;
        }

        /* init scheduler  registers */
        if (PRV_CPSS_SIP_7_CHECK_MAC(devNum))
        {
            prvSchedInitAdressStruct(devNum,pdqNum, PRV_CPSS_PSI_AAS_SCHED_OFFSET,( struct prvCpssDxChTxqSchedAddressSpace *)(tilePtr->general.addressSpacePtr));
        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            prvSchedInitAdressStruct(devNum,pdqNum, PRV_CPSS_PSI_HAWK_SCHED_OFFSET,( struct prvCpssDxChTxqSchedAddressSpace *)(tilePtr->general.addressSpacePtr));
        }
        else
        {
            prvSchedInitAdressStruct(devNum,pdqNum, PRV_CPSS_PSI_FALCON_SCHED_OFFSET,( struct prvCpssDxChTxqSchedAddressSpace *)(tilePtr->general.addressSpacePtr));
        }
        PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.regDataBaseInitialized = GT_TRUE;
    }
    tm_lib_init_params.schedLibMtu      = mtu;
    tm_lib_init_params.schedLibPdqNum   = pdqNum;

    rc = prvCpssSip6TxqPdqSchedulerHwParamsGet(devNum, pdqSchedHwParamsPtr);
    if(rc != GT_OK)
    {
      return rc;
    }


    if (PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E != PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(devNum))
    {

        pdqSchedTreeParams.numOfQueues  = tilePtr->mapping.searchTable.qNodeIndexToAnodeIndexSize;
        pdqSchedTreeParams.numOfAnodes  = tilePtr->mapping.searchTable.aNodeIndexToPnodeIndexSize;
        pdqSchedTreeParams.numOfPorts   = tilePtr->mapping.size;

        pdqSchedTreeParams.numOfBnodes  = pdqSchedTreeParams.numOfPorts ;
        pdqSchedTreeParams.numOfCnodes  = pdqSchedTreeParams.numOfPorts ;
    }
    else
    {
        pdqSchedTreeParams.numOfQueues  = pdqSchedHwParamsPtr->maxQueues;
        pdqSchedTreeParams.numOfAnodes  = pdqSchedHwParamsPtr->maxAnodes;
        pdqSchedTreeParams.numOfPorts   = tilePtr->mapping.size;
        pdqSchedTreeParams.numOfBnodes  = pdqSchedHwParamsPtr->maxBnodes;
        pdqSchedTreeParams.numOfCnodes  = pdqSchedHwParamsPtr->maxCnodes ;
    }

    if(pdqSchedTreeParams.numOfQueues>pdqSchedHwParamsPtr->maxQueues)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Requested number of queues [%d] is bigger then supported [%d]\n",
            pdqSchedTreeParams.numOfQueues,pdqSchedHwParamsPtr->maxQueues);
    }

    if(pdqSchedTreeParams.numOfAnodes>pdqSchedHwParamsPtr->maxAnodes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Requested number of Anodes  [%d] is bigger then supported [%d]\n",
            pdqSchedTreeParams.numOfAnodes,pdqSchedHwParamsPtr->maxAnodes);
    }

    if(pdqSchedTreeParams.numOfPorts>pdqSchedHwParamsPtr->maxPorts)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Requested number of P nodes [%d] is bigger then supported [%d]\n",
            pdqSchedTreeParams.numOfPorts,pdqSchedHwParamsPtr->maxPorts);
    }

    if(pdqSchedTreeParams.numOfBnodes>pdqSchedHwParamsPtr->maxBnodes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Requested number of B nodes [%d] is bigger then supported [%d]\n",
            pdqSchedTreeParams.numOfBnodes,pdqSchedHwParamsPtr->maxBnodes);
    }

    if(pdqSchedTreeParams.numOfCnodes>pdqSchedHwParamsPtr->maxCnodes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Requested number of C nodes [%d] is bigger then supported [%d]\n",
            pdqSchedTreeParams.numOfCnodes,pdqSchedHwParamsPtr->maxCnodes);
    }

    /* Scheduler Initialization */
    ret = prvSchedLibOpenExt(devNum, &pdqSchedTreeParams,  &tm_lib_init_params, &PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum]);
    if (ret == 0)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedLibIsInitilized[pdqNum] = GT_TRUE;
    }
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.bucketUsedForShaping = BUCKET_INDX_USED_FOR_SHAPING;

    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqPeriodicSchemeConfig function
 * @endinternal
 *
 * @brief   Configure Periodic Scheme.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
 * @param[in] paramsPtr                - (pointer to) scheduling parameters structure.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 *
 * @note 1. The API may be invoked once in a system lifetime.
 *       2. To indicate the shaping is disabled for a level the
 *       periodicState field must be set to GT_FALSE.
 *       3 paramsPtr is handled as 5 elements array including also a
 *       port level scheduling configuration.
 *
 */
GT_STATUS prvCpssFalconTxqPdqPeriodicSchemeConfig
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN PRV_CPSS_PDQ_SCHED_LEVEL_PERIODIC_PARAMS_STC        *paramsPtr
)
{
    int                 i;
    int                 ret = 0;
    GT_STATUS               rc  = GT_OK;
    struct schedPerLevelPeriodicParams  prms[SCHED_P_LEVEL + 1];
    for (i = SCHED_Q_LEVEL; i <= SCHED_P_LEVEL; i++)
    {
        prms[i].per_state   = (uint8_t)(paramsPtr[i].periodicState);
        prms[i].shaper_dec  = (uint8_t)(paramsPtr[i].shaperDecoupling);
    }
    ret = prvCpssTxqSchedulerConfigurePeriodicScheme(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], prms);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}

GT_STATUS prvCpssFalconTxqPdqTreePlevelDwrrEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_BOOL enable
)
{
    int     ret = 0;
    GT_STATUS   rc  = GT_OK;
    uint8_t     prio[8];
    int     i;
    PRV_CPSS_DEV_CHECK_MAC(devNum);
    /*Enable/disable for all levels*/
    for (i = 0; i < 8; i++) {
        prio[i] = enable;
    }
    ret = prvCpssSchedNodesTreeSetDwrrPrio(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], prio);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqAsymPortCreate function
 * @endinternal
 *
 * @brief   Create Port with assymetric sub-tree and download its parameters to HW.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
 * @param[in] portInd                  - Port index.
 * @param[in] paramsPtr                - Port parameters structure pointer.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
 * @retval GT_OUT_OF_RANGE          - on parameter value out of range.
 * @retval GT_NO_RESOURCE           - on memory allocation fail.
 * @retval GT_BAD_STATE             - on portInd not in use.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 *
 * @note To indicate that no shaping is needed for port, set
 *       cirBw to CPSS_SIP6_TXQ_INVAL_DATA_CNS, in this case other shaping
 *       parameters will not be considered.
 *
 * @param[in]  fixedMappingToCNode - if equal GT_TRUE then P[i] is mapped to C[i]
 */
GT_STATUS prvCpssFalconTxqPdqAsymPortCreate
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 portInd,
    IN GT_U32 firstChildInRange,
    IN PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC      *paramsPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_port_params    prms;
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);
    if (portInd >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts)
    { /* check that the port index is valid */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "portInd[%d] >= [%d] 'max' ",
                          portInd,
                          PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts);
    }
    prvCpssSchedPortParamsCpy(paramsPtr, &prms);
    ret = prvCpssSchedNodesCreateAsymPort(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                          (uint8_t)portInd,
                          &prms, firstChildInRange);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqAnodeToPortCreate function
 * @endinternal
 *
 * @brief   Configure Periodic Scheme.
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
 * @param[in] portInd                  - Port index.
 * @param[in] aParamsPtr               - A-Node parameters structure pointer.
 * @param[in] bParamsPtr               - B-Node parameters structure pointer.
 * @param[in] cParamsPtr               - C-Node parameters structure pointer.
 * @param[in] optPtr            -              (pointer to)tree build requirments
 *
 * @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
 * @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
 * @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
 * @retval GT_OUT_OF_RANGE          - on parameter value out of range.
 * @retval GT_NO_RESOURCE           - on memory allocation fail.
 * @retval GT_BAD_STATE             - on portInd not in use.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 *
 * @note To indicate that no shaping is needed to the queue/node, set
 *       shapingProfilePtr to CPSS_TM_INVAL_CNS.
 *
 */
GT_STATUS prvCpssFalconTxqPdqAnodeToPortCreate
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 portInd,
    IN PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC          *aParamsPtr,
    IN PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC          *bParamsPtr,
    IN PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC          *cParamsPtr,
    IN PRV_CPSS_SCHED_PORT_TO_Q_CREATE_OPTIONS_STC   *optPtr,
    OUT GT_U32                                       *aNodeIndPtr,
    OUT GT_U32                                       *bNodeIndPtr,
    OUT GT_U32                                       *cNodeIndPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_a_node_params  a_prms;
    struct sched_b_node_params  b_prms;
    struct sched_c_node_params  c_prms;

    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(bParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(cParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(aNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(bNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(cNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(optPtr);
    prvCpssSchedANodeParamsCpy(aParamsPtr, &a_prms);
    prvCpssSchedBNodeParamsCpy(bParamsPtr, &b_prms);

    prvCpssSchedCNodeParamsCpy(cParamsPtr, &c_prms);

    ret = prvCpssSchedNodesCreateANodeToPort(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                         (uint8_t)portInd,
                         &a_prms,
                         &b_prms,
                         &c_prms,
                         aNodeIndPtr,
                         bNodeIndPtr,
                         cNodeIndPtr, optPtr);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    if (rc)
    {
        return rc;
    }

    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqQueueToAnodeCreate function
 * @endinternal
 *
 * @brief   Create path from Queue to A-node.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] aNodeInd                 - A-Node index.
 * @param[in] qParamsPtr               - Queue parameters structure pointer.
 *
 * @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
 * @retval GT_OUT_OF_RANGE          - on parameter value out of range.
 * @retval GT_NO_RESOURCE           - on memory allocation fail.
 * @retval GT_BAD_STATE             - on portInd not in use.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 */
GT_STATUS prvCpssFalconTxqPdqQueueToAnodeCreate
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 aNodeInd,
    IN PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC        *qParamsPtr,
    IN GT_BOOL explicitNumber,
    OUT GT_U32                          *queueIndPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_queue_params   q_prms;
    uint32_t            q_index;
    CPSS_NULL_PTR_CHECK_MAC(qParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(queueIndPtr);
    prvCpssSchedQueueParamsCpy(qParamsPtr, &q_prms);
    if (GT_FALSE == explicitNumber)
    {
        q_index = (uint32_t)-1;
    }
    else
    {
        q_index = *queueIndPtr;
    }
    ret = prvCpssSchedNodesCreateQueueToANode(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                          aNodeInd,
                          &q_prms,
                          &q_index);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    if (rc)
    {
        return rc;
    }
    *queueIndPtr = q_index;
    return rc;
}
/**
 * @internal prvCpssSchedPortParamsCpy function
 * @endinternal
 *
 * @brief   Copy CPSS Port structure to TM Port structure.
 *
 * @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
 *
 * @param[in] paramsPtr                - CPSS Port parameters structure pointer.
 *                                       None.
 */
GT_VOID prvCpssSchedPortParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC     *paramsPtr,
    OUT struct sched_port_params       *prmsPtr
)
{
    int i;
    prmsPtr->cir_bw = paramsPtr->cirBw;
    prmsPtr->eir_bw = paramsPtr->eirBw;
    prmsPtr->cbs    = paramsPtr->cbs;
    prmsPtr->ebs    = paramsPtr->ebs;
    for (i = 0; i < 8; i++) {
        prmsPtr->quantum[i]     = (uint16_t)paramsPtr->quantumArr[i];
        prmsPtr->dwrr_priority[i]   = (uint8_t)paramsPtr->schdModeArr[i];
    }
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children    = (uint16_t)paramsPtr->numOfChildren;
}
/**
 * @internal prvCpssSchedCNodeParamsCpy function
 * @endinternal
 *
 * @brief   Copy CPSS C-Node structure to TM C-Node structure.
 *
 * @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
 *
 * @param[in] paramsPtr                - CPSS C-Node parameters structure pointer.
 *                                       None.
 */
GT_VOID prvCpssSchedCNodeParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC      *paramsPtr,
    OUT struct sched_c_node_params        *prmsPtr
)
{
    int i;
    prmsPtr->shaping_profile_ref    = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum        = (uint16_t)paramsPtr->quantum;
    for (i = 0; i < 8; i++) {
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    }
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children    = (uint16_t)paramsPtr->numOfChildren;
}
/**
 * @internal prvCpssSchedBNodeParamsCpy function
 * @endinternal
 *
 * @brief   Copy CPSS B-Node structure to TM B-Node structure.
 *
 * @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
 *
 * @param[in] paramsPtr                - CPSS B-Node parameters structure pointer.
 *                                       None.
 */
GT_VOID prvCpssSchedBNodeParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC      *paramsPtr,
    OUT struct sched_b_node_params        *prmsPtr
)
{
    int i;
    prmsPtr->shaping_profile_ref    = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum        = (uint16_t)paramsPtr->quantum;
    for (i = 0; i < 8; i++) {
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    }
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children    = (uint16_t)paramsPtr->numOfChildren;
}
/**
 * @internal prvCpssSchedANodeParamsCpy function
 * @endinternal
 *
 * @brief   Copy CPSS A-Node structure to TM A-Node structure.
 *
 * @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
 *
 * @param[in] paramsPtr                - CPSS A-Node parameters structure pointer.
 *                                       None.
 */
GT_VOID prvCpssSchedANodeParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC      *paramsPtr,
    OUT struct sched_a_node_params        *prmsPtr
)
{
    int i;
    prmsPtr->shaping_profile_ref    = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum        = (uint16_t)paramsPtr->quantum;
    for (i = 0; i < 8; i++) {
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    }
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children    = (uint16_t)paramsPtr->numOfChildren;
}
/**
 * @internal prvCpssSchedQueueParamsCpy function
 * @endinternal
 *
 * @brief   Copy CPSS Queue structure to TM Queue structure.
 *
 * @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
 *
 * @param[in] paramsPtr                - CPSS Queue parameters structure pointer.
 *                                       None.
 */
GT_VOID prvCpssSchedQueueParamsCpy
(
    IN PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC      *paramsPtr,
    OUT struct sched_queue_params        *prmsPtr
)
{
    prmsPtr->shaping_profile_ref    = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum        = (uint16_t)paramsPtr->quantum;
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
}
/**
 * @internal prvCpssFalconTxqPdqPrintNodeFunc function
 * @endinternal
 *
 * @brief   This function print node
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
 * @retval GT_OUT_OF_RANGE          - on parameter value out of range.
 * @retval GT_NO_RESOURCE           - on memory allocation fail.
 * @retval GT_BAD_STATE             - on portInd not in use.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 */
static int prvCpssFalconTxqPdqPrintNodeFunc(int bBeginNodeEval,
                        void * vpEnv,
                        unsigned int level,
                        unsigned int node_index,
                        unsigned int quantum,
                        unsigned int node_parent_index,
                        unsigned int elig_fun,
                        unsigned int dwrrBmp,
                        void * vpUserData)
{
    GT_U8 devNum;
    devNum = *((GT_U8*)(vpEnv));
    (void)vpUserData;
    (void)node_parent_index;
    (void)devNum;
    (void)quantum;

    switch (level)
    {
    case PRV_CPSS_PDQ_LEVEL_P_E:
    {
        if (bBeginNodeEval)
        {
            cpssOsPrintf("port :  index = %d    quantum[0] %d elig_fun = %d     func_name =  %s dwrrBmp 0x%02x\n", node_index, quantum, elig_fun, prvCpssSip6TxqEligFuncNameGet(elig_fun),dwrrBmp);
        }
        break;
    }
    case PRV_CPSS_PDQ_LEVEL_C_E:
    {
        if (bBeginNodeEval)
        {
            cpssOsPrintf("      C_node : index = %d   quantum %d  elig_fun = %d     func_name =  %s dwrrBmp 0x%02x\n", node_index,quantum, elig_fun,  prvCpssSip6TxqEligFuncNameGet(elig_fun),dwrrBmp);
        }
        break;
    }
    case PRV_CPSS_PDQ_LEVEL_B_E:
    {
        if (bBeginNodeEval)
        {
            cpssOsPrintf("            B_node :  index = %d    quantum %d  elig_fun = %d     func_name =  %s dwrrBmp 0x%02x\n", node_index,quantum, elig_fun,  prvCpssSip6TxqEligFuncNameGet(elig_fun),dwrrBmp);
        }
        break;
    }
    case PRV_CPSS_PDQ_LEVEL_A_E:
    {
        if (bBeginNodeEval)
        {
            cpssOsPrintf("                  A_node :  index = %d   quantum %d  elig_fun=%d     func_name =  %s dwrrBmp 0x%02x\n", node_index, quantum, elig_fun,  prvCpssSip6TxqEligFuncNameGet(elig_fun),dwrrBmp);
        }
        break;
    }
    case PRV_CPSS_PDQ_LEVEL_Q_E:
    {
        /* queue callback is called once */
        cpssOsPrintf("                        Queue :index = %d  quantum %d  elig_fun = %d func_name =  %s (%s)\n", node_index, quantum, elig_fun,
        prvCpssSip6TxqQueueEligFuncNameGet(elig_fun),prvCpssSip6TxqQueueEligFuncDescriptionGet(elig_fun));
        break;
    }
    default:               cpssOsPrintf(" unknown level=%d index=%d\n", level, node_index); return 1;
    }
    return 0;
}
/**
 * @internal prvCpssFalconTxqPdqDumpPortSw function
 * @endinternal
 *
 * @brief   print the configure tree under a specific port (SW data base ,no read from HW)
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] portIndex                - Port index.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqPdqDumpPortSw
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 portIndex
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;
    ret = prvCpssSchedNodesReadTraversePortTreeSw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              portIndex,
                              prvCpssFalconTxqPdqPrintNodeFunc,
                              &devNum,
                              NULL);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqDumpPortHw function
 * @endinternal
 *
 * @brief   print the configure tree under a specific port ( read from HW)
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] portIndex                - Port index.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqPdqDumpPortHw
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 portIndex
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;
    ret = prvCpssSchedNodesReadTraversePortTreeHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              portIndex,
                              prvCpssFalconTxqPdqPrintNodeFunc,
                              &devNum,
                              NULL);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }
    return rc;
}

GT_STATUS prvCpssFalconTxqPdqPortMappingDumpHw
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 portIndex
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;
    ret = prvCpssSchedNodesReadPortMappingHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              portIndex);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }
    return rc;
}

GT_STATUS prvCpssFalconTxqPdqCnodeMappingDumpHw
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 index
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;
    ret = prvCpssSchedNodesReadCnodeMappingHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              index);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }
    return rc;
}


GT_STATUS prvCpssFalconTxqPdqBnodeMappingDumpHw
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 index
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;
    GT_U32 bnodeParent=0,bnodeChildLo=0, bnodeChildHi=0,bnodeEf=0;

    ret = prvCpssSchedNodesReadBnodeMappingHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              index,&bnodeParent,&bnodeChildLo,&bnodeChildHi,&bnodeEf);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }


    cpssOsPrintf("C level  %d \n",bnodeParent);
    cpssOsPrintf("A level low %d high %d EF %d \n",bnodeChildLo,bnodeChildHi,bnodeEf);
    return rc;
}


GT_STATUS prvCpssFalconTxqPdqAnodeMappingDumpHw
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 index
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;
    GT_U32 anodeParent,anodeChildLo,anodeChildHi,anodeEf;
    ret = prvCpssSchedNodesReadAnodeMappingHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              index,
                              &anodeParent,&anodeChildLo,&anodeChildHi,&anodeEf);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }


    cpssOsPrintf("B level  %d \n",anodeParent);
    cpssOsPrintf("Q level low %d high %d EF %d \n",anodeChildLo,anodeChildHi,anodeEf);

    return rc;
}


GT_STATUS prvCpssFalconTxqPdqQnodeMappingDump
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 index
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;
    GT_U32 qnodeParent,qnodeEf;
    ret = prvCpssSchedNodesReadQnodeMappingHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              index,
                              &qnodeParent,&qnodeEf);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }


    cpssOsPrintf("HW A level  %d \n",qnodeParent);
    cpssOsPrintf("HW Q EF %d \n",qnodeEf);

     ret = prvCpssSchedNodesReadQnodeMappingSw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              index,
                              &qnodeParent,&qnodeEf);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }

    cpssOsPrintf("SW A level  %d \n",qnodeParent);
    cpssOsPrintf("SW Q EF %d \n",qnodeEf);

    return rc;
}


GT_STATUS prvCpssFalconTxqPdqAnodeParentGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 aNodeIndex,
    OUT GT_U32 *bNodeIndexPtr
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;

    ret = prvCpssSchedNodesReadAnodeMappingHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              aNodeIndex,
                              bNodeIndexPtr,NULL,NULL,NULL);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }

    return rc;
}

GT_STATUS prvCpssFalconTxqPdqBnodeParentGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 bNodeIndex,
    OUT GT_U32 *cNodeIndexPtr
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;

    ret = prvCpssSchedNodesReadBnodeMappingHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              bNodeIndex,
                              cNodeIndexPtr,NULL,NULL,NULL);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }

    return rc;
}

GT_STATUS prvCpssFalconTxqPdqBnodeChildrenGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 bNodeIndex,
    OUT GT_U32 *firstAnode,
    OUT GT_U32 *lastAnode
)
{
    GT_STATUS   rc = GT_OK;
    int     ret;

    ret = prvCpssSchedNodesReadBnodeMappingHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              bNodeIndex,
                              NULL,firstAnode,lastAnode,NULL);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }

    return rc;
}




/**
 * @internal prvCpssSip6TxqPdqShapingRateFromHwGet function
 * @endinternal
 *
 * @brief   Read shaping parameters of specific node( read from HW)
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] nodeInd                  - Node index.
 * @param[in] level                    - Node level
 *
 * @param[in] burstSize                - burst size in units of 4K bytes
  *                                      max value is 1K-1 which results in 4M-1 burst size
 * @param[out] maxRatePtr               - Requested Rate in Kbps
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssSip6TxqPdqShapingRateFromHwGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeInd,
    IN PRV_CPSS_PDQ_LEVEL_ENT level,
    IN PRV_QUEUE_SHAPING_ACTION_ENT rateType,
    OUT GT_U16                               *burstSizePtr,
    OUT GT_U32                               *maxRatePtr
)
{
    int                     ret;
    GT_STATUS                   rc = GT_OK;
    GT_32                       token, res, divExp;
    struct prvCpssDxChTxqSchedShapingProfile    params;
    ret = prvSchedLowLevelGetNodeShapingParametersFromHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                 PRV_CPSS_LEVEL_CONVERT_TO_TM_FORMAT_MAC(level), nodeInd, &params);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }
    if(0==PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.bucketUsedForShaping)
    {
        if (rateType == PRV_QUEUE_SHAPING_ACTION_STOP_TRANSMITION_ENT)
        {
            *burstSizePtr   = (GT_U16)(params.min_burst_size / 4) /*In 4K bytes units*/;
            token       = params.min_token;
            res     = params.min_token_res;
            divExp      = params.min_div_exp;
        }
        else
        {
            *burstSizePtr   = (GT_U16)(params.max_burst_size) /*In 1K bytes units*/;
            token       = params.max_token;
            res     = params.max_token_res;
            divExp      = params.max_div_exp;
        }
    }
    else
    {
        if (rateType == PRV_QUEUE_SHAPING_ACTION_PRIORITY_DOWNGRADE_ENT)
        {
            *burstSizePtr   = (GT_U16)(params.min_burst_size ) /*In 1K bytes units*/;
            token       = params.min_token;
            res     = params.min_token_res;
            divExp      = params.min_div_exp;
        }
        else
        {
            *burstSizePtr   = (GT_U16)(params.min_burst_size / 4) /*In 4K bytes units*/;
            token       = params.max_token;
            res     = params.max_token_res;
            divExp      = params.max_div_exp;
        }
    }

    rc = prvCpssSip6TxqPdqCalculateActualBwGet(devNum, token, res, divExp, level, maxRatePtr);
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqShapingOnNodeSet function
 * @endinternal
 *
 * @brief   Write shaping parameters to specific node
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] nodeInd                  - Node index.
 * @param[in] level                    - node level
 * @param[in] burstSize                - burst size in units of 4K bytes
 *                                       max value is 1K-1 which results in 4M-1 burst size
 * @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
 * @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqPdqShapingOnNodeSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeInd,
    IN PRV_CPSS_PDQ_LEVEL_ENT level,
    IN GT_U16 burstSize,
    INOUT GT_U32                              *maxRatePtr
)
{
    int                 ret;
    GT_STATUS               rc = GT_OK;
    struct sched_shaping_profile_params params;
    uint16_t                profile = SCHED_DIRECT_NODE_SHAPING;
    GT_U16                  burstSizeFromHw;

    if (level != PRV_CPSS_PDQ_LEVEL_Q_E)
    {
        if(0==PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.bucketUsedForShaping)
        {
            params.cir_bw = *maxRatePtr;
            /* burst size in units of 4K bytes*/
            params.cbs  = burstSize * 4;
            params.eir_bw   = 0;
            params.ebs  = 0;
        }
        else
        {
            params.cir_bw =0;
            params.cbs  =0;
            params.eir_bw   = *maxRatePtr;
            /* burst size in units of 4K bytes*/
            params.ebs  = burstSize * 4;
        }
    }
    else
    {
        /*read from shadow */
        ret = prvCpssSchedReadNodeShapingConfigFromShadow(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                  SCHED_Q_LEVEL, nodeInd, &profile, &params);
        /*check that  CIR is greater then EIR*/
        if (params.eir_bw > *maxRatePtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "EIR %d should be smaller then CIR %d ", params.eir_bw, *maxRatePtr);
        }
        /*update only CIR*/
        params.cir_bw = *maxRatePtr;
        /* burst size in units of 4K bytes*/
        params.cbs = burstSize * 4;
    }
    ret = prvCpssSchedShapingUpdateNodeShaping(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                          PRV_CPSS_LEVEL_CONVERT_TO_TM_FORMAT_MAC(level),
                           nodeInd, &params, maxRatePtr, NULL);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }
    rc = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, pdqNum, nodeInd, level,
                           PRV_QUEUE_SHAPING_ACTION_STOP_TRANSMITION_ENT, &burstSizeFromHw, maxRatePtr);
    return rc;
}
static GT_U8 prvCpssFalconTxqPdqCalculateEligPrioFuncForNodeLevelShaping(GT_BOOL enable,GT_U8 oldEligPrioFunc)
{
    GT_U8 result;
    if (enable == GT_TRUE)
    {
        switch (oldEligPrioFunc)
        {
        case  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(0):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(0);
            break;
        case  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(1):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(1);
            break;
        case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(2):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(2);
            break;
        case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(3):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(3);
            break;
        case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(4):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(4);
            break;
        case PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(5):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(5);
            break;
        case  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(6):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(6);
            break;
        case  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(7):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(7);
            break;
        default:
            result = oldEligPrioFunc;
            break;
        }
    }
    else
    {
        switch (oldEligPrioFunc)
        {
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(0):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(0);
            break;
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(1):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(1);
            break;
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(2):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(2);
            break;
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(3):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(3);
            break;
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(4):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(4);
            break;
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(5):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(5);
            break;
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(6):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(6);
            break;
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(7):
            result = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE(7);
            break;
        default:
            result = oldEligPrioFunc;
            break;
        }
    }
    return result;
}


static GT_U8 prvCpssFalconTxqPdqCalculateEligPrioFuncForBlevelShaping(GT_BOOL enable)
{
    if (enable == GT_TRUE)
    {
        return PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_B_NODE_WITH_SHAPING;
    }
    else
    {
        return PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_B_NODE;
    }
}

static GT_BOOL prvCpssFalconTxqPdqShapingStatusForNodeGet(GT_U32 eligPrioFunc)
{
        switch (eligPrioFunc)
        {
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(0):
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(1):
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(2):
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(3):
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(4):
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(5):
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(6):
        case     PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_NODE_WITH_SHAPING(7):
            return GT_TRUE;
            break;
        default:
            return GT_FALSE;
            break;
        }
    }


static GT_BOOL prvCpssFalconTxqPdqShapingStatusForQlevelGet(GT_U32 eligPrioFunc)
{

    GT_BOOL shaping;

   (GT_VOID)prvSchedQueueEligFuncInfoGet(eligPrioFunc,&shaping,NULL,NULL);

   return shaping;


}
static GT_U8 prvCpssFalconTxqPdqCalculateEligPrioFuncForQlevelShaping(GT_BOOL enable, GT_U8 oldEligPrioFunc)
{
    GT_U8 result;
    GT_U32 priorityGroup,minBwType;
    GT_BOOL shaping,minBw;


    priorityGroup = prvSchedQueueEligFuncInfoGet(oldEligPrioFunc,&shaping,&minBw,&minBwType);

    if(enable == GT_TRUE)
    {
       if(minBw== GT_FALSE)
       {
         result = PRV_SCHED_ELIG_Q_MIN_SHP_PRIO0+priorityGroup;
       }
       else
       {
         result = (minBwType ==0)?(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0_WITH_SHAPING+priorityGroup):(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1_WITH_SHAPING+priorityGroup);
       }
    }
    else
    {
       if(minBw== GT_FALSE)
       {
        result = PRV_SCHED_ELIG_Q_PRIO0+priorityGroup;
       }
       else
       {
         result = (minBwType ==0)?(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0+priorityGroup):(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1+priorityGroup);
       }
    }

    return result;
}


static GT_U8 prvCpssFalconTxqPdqCalculateEligPrioFuncForQlevelMinimalBw
(
    GT_BOOL enable,
    GT_BOOL keepOriginalPriority,
    GT_U8   oldEligPrioFunc
)
{
    GT_U8 result;
    GT_U32 priorityGroup,minBwType;
    GT_BOOL shaping,minBw;


    priorityGroup = prvSchedQueueEligFuncInfoGet(oldEligPrioFunc,&shaping,&minBw,&minBwType);

    if(enable == GT_TRUE)
    {
       if(shaping== GT_FALSE)
       {
        result = (keepOriginalPriority ==GT_FALSE)?(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0+priorityGroup):(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1+priorityGroup);
       }
       else
       {
        result = (keepOriginalPriority ==GT_FALSE)?(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0_WITH_SHAPING+priorityGroup):(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1_WITH_SHAPING+priorityGroup);
       }
    }
    else
    {
       if(shaping== GT_FALSE)
       {
        result = PRV_SCHED_ELIG_Q_PRIO0+priorityGroup;
       }
       else
       {
        result =PRV_SCHED_ELIG_Q_MIN_SHP_PRIO0+priorityGroup;
       }
    }

    return result;
}
/**
* @internal prvCpssSip6TxqPdqCalculateEligPrioFuncForQlevelShaping function
* @endinternal
*
* @brief This function calculate eligeble function for queue.
*  The function chek if shaping or/and guranteed BW feature should be considered for this queue and
*  update eligeble function accordingly.
*
*
* @note APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier;Ironman.
* @note NOT APPLICABLE DEVICES:    xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -physical device number
* @param[in] enable                -if equals GT_TRUE SP queues always are scheduled first then WRR groups,
*                                   otherwise queue offset is treated as queue priority.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssSip6TxqPdqCalculateEligPrioFuncForQlevelShaping
(
  IN    GT_U8  devNum,
  IN    GT_U32 pdqNum,
  IN    GT_U32 queueNum,
  INOUT GT_U8  *eligPrioFuncPtr
)
{
    GT_BOOL shapingEnabled=GT_FALSE,minBwEnabled=GT_FALSE;
    GT_STATUS rc;
    GT_U32 minBwType;

    /*Check if shaping was configured*/
    rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, pdqNum, queueNum, PRV_CPSS_PDQ_LEVEL_Q_E, &shapingEnabled);
    if (rc)
    {
        return rc;
    }
    /*Check if minimal bw was configured*/
    rc = prvCpssFalconTxqPdqMinBwEnableGet(devNum, pdqNum, queueNum, &minBwEnabled,&minBwType);
    if (rc)
    {
        return rc;
    }
    /*If shaping was configured then change elig prio func to contain shaping*/
    if (GT_TRUE == shapingEnabled)
    {
        *eligPrioFuncPtr =  prvCpssFalconTxqPdqCalculateEligPrioFuncForQlevelShaping(GT_TRUE, *eligPrioFuncPtr);
    }
    /*If minimal bw was configured then change elig prio func to contain minimal bw*/
    if (GT_TRUE == minBwEnabled)
    {
        *eligPrioFuncPtr =  prvCpssFalconTxqPdqCalculateEligPrioFuncForQlevelMinimalBw(GT_TRUE,(minBwType==0)?GT_FALSE:GT_TRUE, *eligPrioFuncPtr);
    }

    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqPdqMinBwEnableSet function
 * @endinternal
 *
 * @brief
 *             Enable/Disable Token Bucket rate shaping on specified tile/ queue of  specified device.
 *             Once token bucket is empty the priority is lowered to lowest.
 *
 * @note   APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:    xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] nodeInd                  - Node index.
 * @param[in] enable                   - GT_TRUE,  Set lowest  priority once token bucket is empty
 *                                      GT_FALSE, otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqPdqMinBwEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeInd,
    IN GT_BOOL enable,
    IN  GT_BOOL keepPriorityGroup
)
{
    int             ret;
    GT_STATUS           rc = GT_OK;
    GT_U8               eligFunc;
    struct sched_queue_params   params;
    /*Read old elig prio func in order to get old priority*/
    ret = prvCpssSchedNodesReadQueueConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              nodeInd, &params);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }
    eligFunc    =  prvCpssFalconTxqPdqCalculateEligPrioFuncForQlevelMinimalBw(enable, keepPriorityGroup,params.elig_prio_func_ptr);
    ret     = prvCpssSchedUpdateEligFunc(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                             SCHED_Q_LEVEL, nodeInd, eligFunc);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqShapingEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] nodeInd                  - Node index.
 * @param[in] level              - node level
 * @param[in] enable                   - GT_TRUE,  Shaping
 *                                      GT_FALSE, disable Shaping
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqPdqShapingEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeInd,
    IN PRV_CPSS_PDQ_LEVEL_ENT level,
    IN GT_BOOL enable
)
{
    int             ret;
    GT_STATUS           rc = GT_OK;
    GT_U8               eligFunc =CPSS_PDQ_SCHED_ELIG_N_DEQ_DIS_E;
    struct sched_queue_params   qParams;
    struct sched_a_node_params  aNodeParams;
    struct sched_b_node_params  bNodeParams;
    struct sched_c_node_params  cNodeParams;
    struct sched_port_params    portParams;

    switch(level)
    {
        case PRV_CPSS_PDQ_LEVEL_A_E:
         /*Read old elig prio func in order to get old priority*/
         ret = prvCpssSchedNodesReadANodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                   nodeInd, &aNodeParams);
         if (ret)
         {
             rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
             return rc;
         }

         eligFunc = prvCpssFalconTxqPdqCalculateEligPrioFuncForNodeLevelShaping(enable,aNodeParams.elig_prio_func_ptr);
         break;
        case PRV_CPSS_PDQ_LEVEL_Q_E:
         /*Read old elig prio func in order to get old priority*/
         ret = prvCpssSchedNodesReadQueueConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                nodeInd, &qParams);
         if (ret)
         {
             rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
             return rc;
         }
        eligFunc =  prvCpssFalconTxqPdqCalculateEligPrioFuncForQlevelShaping(enable, qParams.elig_prio_func_ptr);
        break;
       case PRV_CPSS_PDQ_LEVEL_B_E:

        if (PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E == PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(devNum))
        {
            ret = prvCpssSchedNodesReadBNodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                   nodeInd, &bNodeParams);
            if (ret)
            {
                rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
                return rc;
            }

            eligFunc = prvCpssFalconTxqPdqCalculateEligPrioFuncForNodeLevelShaping(enable,bNodeParams.elig_prio_func_ptr);
        }
        else
        {
            eligFunc = prvCpssFalconTxqPdqCalculateEligPrioFuncForBlevelShaping(enable);
        }
        break;

       case PRV_CPSS_PDQ_LEVEL_C_E:

        if (PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E == PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(devNum))
        {
            ret = prvCpssSchedNodesReadCNodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                   nodeInd, &cNodeParams);
            if (ret)
            {
                rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
                return rc;
            }

            eligFunc = prvCpssFalconTxqPdqCalculateEligPrioFuncForNodeLevelShaping(enable,cNodeParams.elig_prio_func_ptr);
        }

        break;
       case PRV_CPSS_PDQ_LEVEL_P_E:
        if (PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E == PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(devNum))
        {
            ret = prvCpssSchedNodesReadPortConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                   nodeInd, &portParams);
            if (ret)
            {
                rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
                return rc;
            }

            eligFunc = prvCpssFalconTxqPdqCalculateEligPrioFuncForNodeLevelShaping(enable,portParams.elig_prio_func_ptr);
        }
       default:
        break;

    }

    if(CPSS_PDQ_SCHED_ELIG_N_DEQ_DIS_E==eligFunc)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "eligFunc not initialized correctly for level %d\n",level);
    }
    ret = prvCpssSchedUpdateEligFunc(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                     level, nodeInd, eligFunc);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqShapingEnableGet function
 * @endinternal
 *
 * @brief   Get Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] nodeInd                  - Node index.
 * @param[in] level                         - Node level.
 *
 * @param[out] enablePtr                - GT_TRUE, enable Shaping
 *                                      GT_FALSE, disable Shaping
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqPdqShapingEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeInd,
    IN PRV_CPSS_PDQ_LEVEL_ENT level,
    OUT GT_BOOL                   *enablePtr
)
{
    int             ret;
    GT_STATUS           rc = GT_OK;
    struct sched_a_node_params  aNodeParams;
    struct sched_b_node_params  bNodeParams;
    struct sched_queue_params   queueParams;
    struct sched_c_node_params  cNodeParams;
    struct sched_port_params    portParams;

    switch(level)
    {
        case PRV_CPSS_PDQ_LEVEL_A_E:
         ret = prvCpssSchedNodesReadANodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                  nodeInd, &aNodeParams);
         if (ret)
         {
             rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
             return rc;
         }
        *enablePtr =  prvCpssFalconTxqPdqShapingStatusForNodeGet(aNodeParams.elig_prio_func_ptr);
         break;
        case PRV_CPSS_PDQ_LEVEL_Q_E:
         ret = prvCpssSchedNodesReadQueueConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                  nodeInd, &queueParams);
         if (ret)
         {
             rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
             return rc;
         }
         *enablePtr = prvCpssFalconTxqPdqShapingStatusForQlevelGet(queueParams.elig_prio_func_ptr);
        break;
       case PRV_CPSS_PDQ_LEVEL_B_E:

        ret = prvCpssSchedNodesReadBNodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                  nodeInd, &bNodeParams);
        if (ret)
        {
            rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
            return rc;
        }
        *enablePtr =  prvCpssFalconTxqPdqShapingStatusForNodeGet(bNodeParams.elig_prio_func_ptr);
        break;

       case PRV_CPSS_PDQ_LEVEL_C_E:

        ret = prvCpssSchedNodesReadCNodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                  nodeInd, &cNodeParams);
        if (ret)
        {
            rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
            return rc;
        }
        *enablePtr =  prvCpssFalconTxqPdqShapingStatusForNodeGet(cNodeParams.elig_prio_func_ptr);
        break;

        break;
       case PRV_CPSS_PDQ_LEVEL_P_E:
        ret = prvCpssSchedNodesReadPortConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                  nodeInd, &portParams);
        if (ret)
        {
            rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
            return rc;
        }
        *enablePtr =  prvCpssFalconTxqPdqShapingStatusForNodeGet(portParams.elig_prio_func_ptr);
        break;
       default:
        break;

    }

    return GT_OK;
}

/**
* @internal prvCpssSip6TxqPdqANodeSchedulingAttributesSpPrecedeConvert function
* @endinternal
*
* @brief This function convert scheduling profile into scheduler format and update HW.
*  The algorithm prioritise SP queues over WRR queues.
*  Traffic in queues marked as strict_priority always takes precedens before wrr queues
*  Example :SP+DWRR mixed mode : Q0 is SP, Q1-Q7 is DWRR.Q0 is scheduled first, then Q1-Q7
*
*
*
* @note APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier;Ironman.
* @note NOT APPLICABLE DEVICES:    xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -physical device number
* @param[in] enable                -if equals GT_TRUE SP queues always are scheduled first then WRR groups,
*                                   otherwise queue offset is treated as queue priority.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssSip6TxqPdqANodeSchedulingAttributesSpPrecedeConvert
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE          *aNodePtr,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_PROFILE   *schedAtrributesPtr
)
{

   struct sched_queue_params   params;
   int             ret;
   GT_STATUS           rc = GT_OK;
   GT_U32              numberOfQueues;
   GT_U32              *wrrEnableBmpPtr, *wrrGroupBmpPtr;
   GT_U32              i, localIndex = 0,maxIterator;
   uint8_t             eligFuncArray [] = {
       PRV_SCHED_ELIG_Q_PRIO0,
       PRV_SCHED_ELIG_Q_PRIO1,
       PRV_SCHED_ELIG_Q_PRIO2,
       PRV_SCHED_ELIG_Q_PRIO3,
       PRV_SCHED_ELIG_Q_PRIO4,
       PRV_SCHED_ELIG_Q_PRIO5,
       PRV_SCHED_ELIG_Q_PRIO6,
       PRV_SCHED_ELIG_Q_PRIO7
   };
   GT_U32              currentSpFunction   = 0;
   GT_U32              wrrGroup0Function   = 0;
   GT_U32              wrrGroup1Function   = 0;
   GT_BOOL             wrrPresent = GT_FALSE;
   GT_BOOL             wrrGroup0Present = GT_FALSE,wrrGroup1Present = GT_FALSE;
   GT_U32              mask;

   numberOfQueues          = aNodePtr->queuesData.pdqQueueLast - aNodePtr->queuesData.pdqQueueFirst + 1;
   maxIterator = numberOfQueues>>5;/*32 queues per register*/

   if(numberOfQueues&0x1F)
   {
       maxIterator++;
   }

   params.shaping_profile_ref  = TM_INVAL;
   /*check if WRR exist*/
   wrrEnableBmpPtr = schedAtrributesPtr->wrrEnableBmp;
   wrrGroupBmpPtr = schedAtrributesPtr->wrrGroupBmp;


   for(i=0;i<maxIterator;i++)
   {
       /*last segment not alligned to 32*/
       if((i==maxIterator-1)&&(numberOfQueues&0x1F))
       {
         mask = (GT_U32)((1 << (numberOfQueues&0x1F)) - 1);
       }
       else
       {
         mask= 0xFFFFFFFF;
       }
       if((*wrrEnableBmpPtr))
       {
           if(i!=maxIterator-1)
           {
              wrrPresent = GT_TRUE;
           }
           else
           {
               if((*wrrEnableBmpPtr)&mask)
               {
                   wrrPresent =  GT_TRUE;
               }
           }
       }

       if(*wrrGroupBmpPtr)
       {
           if(i!=maxIterator-1)
           {
              if(*wrrGroupBmpPtr&(*wrrEnableBmpPtr))
              {
                wrrGroup1Present = GT_TRUE;
              }
              if(*wrrGroupBmpPtr!=0xFFFFFFFF)
              {
                   wrrGroup0Present = GT_TRUE;
              }
           }
           else
           {
               if(((*wrrGroupBmpPtr)&mask)&(*wrrEnableBmpPtr))
               {
                   wrrGroup1Present = GT_TRUE;
               }

               if((*wrrGroupBmpPtr)&mask&(*wrrEnableBmpPtr))
               {
                   wrrGroup0Present = GT_TRUE;
               }

           }
       }
       else
       {
           wrrGroup0Present = GT_TRUE;
       }

       wrrEnableBmpPtr++;
       wrrGroupBmpPtr++;
   }


   /*First check the profile*/
      if (wrrPresent)
      {
          if (wrrGroup1Present == GT_FALSE||wrrGroup0Present == GT_FALSE)
          {
              /*all are in the same wrr group */
              currentSpFunction = 1;
          }
          else
          {
              /*both  groups are present */
              wrrGroup0Function   = 0;
              wrrGroup1Function   = 1;
              currentSpFunction   = 2;
          }
      }
      else
      {
          /*all sp*/
          currentSpFunction = 0;
      }
      for (i = aNodePtr->queuesData.pdqQueueFirst; i <= aNodePtr->queuesData.pdqQueueLast; i++, localIndex++)
      {
          params.quantum = (uint16_t)schedAtrributesPtr->weights[localIndex];
          /*DWRR group*/
          if (schedAtrributesPtr->wrrEnableBmp[localIndex>>5] & (1 << (localIndex&0x1F)))
          {
              /*dwrrGroupSize++;*/
              /*DWRR group 1*/
              if (schedAtrributesPtr->wrrGroupBmp[localIndex>>5] & (1<<(localIndex&0x1F)))
              {
                  params.elig_prio_func_ptr = eligFuncArray[wrrGroup1Function];
              }
              else
              /*DWRR group 0*/
              {
                  params.elig_prio_func_ptr = eligFuncArray[wrrGroup0Function];
              }
          }
          else
          /*SP group*/
          {
              params.elig_prio_func_ptr = eligFuncArray[currentSpFunction++];
              if (currentSpFunction == 8)
              {
                  currentSpFunction = 7;/*In case there are more than 8 queues mapped*/
              }
          }
           /*Check  if minBw and/or  shaping is applied to this queue*/
        rc = prvCpssSip6TxqPdqCalculateEligPrioFuncForQlevelShaping(devNum, pdqNum, i, &params.elig_prio_func_ptr);
        if (rc)
        {
            return rc;
        }
        /*update Hw*/
        ret = prvCpssSchedUpdateQ(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                      i, &params);
        if (ret)
        {
            rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
            return rc;
        }
    }
    return GT_OK;

}

/**
* @internal prvCpssSip6TxqPdqANodeSchedulingAttributesSpNonPrecedeConvert function
* @endinternal
*
* @brief This function convert scheduling profile into scheduler format and update HW.
*  The algorithm DOES NOT prioritise SP queues over WRR queues.
 *
*  Scheduling priority of WRR group will be equal to lowest queue offset  that participate
*  in the group.In case this index is bigger then 7  the scheduling priority will be clipped to 7.
*  Example :SP+DWRR mixed mode : Q0 is SP, Q1-Q7 is DWRR.Q1-Q7 is scheduled first, then Q0.
*
*
*
* @note APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier;Ironman.
* @note NOT APPLICABLE DEVICES:    xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -physical device number
* @param[in] enable                -if equals GT_TRUE SP queues always are scheduled first then WRR groups,
*                                   otherwise queue offset is treated as queue priority.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static GT_STATUS prvCpssSip6TxqPdqANodeSchedulingAttributesSpNonPrecedeConvert
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE          *aNodePtr,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_PROFILE   *schedAtrributesPtr
)
{
    struct sched_queue_params   params;
    int             ret;
    GT_STATUS           rc = GT_OK;
    GT_U32              numberOfQueues;
    GT_U32              *wrrEnableBmpPtr, *wrrGroupBmpPtr;
    GT_U32              i,j,localIndex = 0,maxIterator;
    uint8_t             eligFuncArray [] = {
        PRV_SCHED_ELIG_Q_PRIO0,
        PRV_SCHED_ELIG_Q_PRIO1,
        PRV_SCHED_ELIG_Q_PRIO2,
        PRV_SCHED_ELIG_Q_PRIO3,
        PRV_SCHED_ELIG_Q_PRIO4,
        PRV_SCHED_ELIG_Q_PRIO5,
        PRV_SCHED_ELIG_Q_PRIO6,
        PRV_SCHED_ELIG_Q_PRIO7
    };
    GT_U32              mask;
    GT_U32              wrrGroupStart[2] = {TM_INVAL,TM_INVAL};
    GT_BOOL             wrrGroupPresent[2] = {GT_FALSE,GT_FALSE};
    GT_U32              groupIterator;
    GT_U32              tmp;

    numberOfQueues          = aNodePtr->queuesData.pdqQueueLast - aNodePtr->queuesData.pdqQueueFirst + 1;
    maxIterator = numberOfQueues>>5;/*32 queues per register*/

    if(numberOfQueues&0x1F)
    {
        maxIterator++;
    }

    params.shaping_profile_ref  = TM_INVAL;
    /*check if WRR exist*/
    wrrEnableBmpPtr = schedAtrributesPtr->wrrEnableBmp;
    wrrGroupBmpPtr = schedAtrributesPtr->wrrGroupBmp;


    for(i=0;i<maxIterator;i++)
    {
        /*last segment not alligned to 32*/
        if((i==maxIterator-1)&&(numberOfQueues&0x1F))
        {
          mask = (GT_U32)((1 << (numberOfQueues&0x1F)) - 1);
        }
        else
        {
          mask= 0xFFFFFFFF;
        }

        if((*wrrEnableBmpPtr)&mask)
        {
            if((*wrrGroupBmpPtr)&mask)
            {
               wrrGroupPresent[1] = GT_TRUE;
               if(((*wrrGroupBmpPtr)&mask)!=(0xFFFFFFFF&mask))
               {
                    wrrGroupPresent[0] = GT_TRUE;
               }
            }
            else
            {
                wrrGroupPresent[0] = GT_TRUE;
            }


            /*Get scheduling priority of WRR group.

                            NOTE :Scheduling priority of WRR group will be equal to lowest queue index that participate
                            in the group.
                            In case this index is bigger then 7  the scheduling priority will be clipped to 7

                        */

            for(groupIterator=0;groupIterator<2;groupIterator++)
            {
                if((TM_INVAL == wrrGroupStart[groupIterator])&&(wrrGroupPresent[groupIterator]))
                {
                    if(0==i)
                    {
                        tmp= *wrrGroupBmpPtr;
                        for(j=0;j<((maxIterator>1)?32:numberOfQueues);j++)
                        {
                            if(groupIterator==(tmp&0x1)&&((*wrrEnableBmpPtr)&(1<<j)))
                            {
                                wrrGroupStart[groupIterator]= (j<7)?j:7;
                                break;
                            }
                            tmp = tmp>>1;
                        }
                    }
                    else /*if the queue is bigger then 32*/
                    {
                        wrrGroupStart[groupIterator]= 7;
                    }
                 }
            }

        }

        wrrEnableBmpPtr++;
        wrrGroupBmpPtr++;
    }
   for(groupIterator=0;groupIterator<2;groupIterator++)
   {
     if(wrrGroupStart[groupIterator]!=TM_INVAL)
     {   /*may happend if queue >7 requested WRR */
        if(((schedAtrributesPtr->wrrEnableBmp[0])&((GT_U32)0x1<<(wrrGroupStart[groupIterator])))==0)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Scheduler %d  used for SP but requested for WRR \n",wrrGroupStart[groupIterator]);
        }
     }
   }

   if((TM_INVAL !=wrrGroupStart[0])&&(wrrGroupStart[0]==wrrGroupStart[1]))
   {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(wrrGroupStart[0]=wrrGroupStart[1]) %d = %d \n",wrrGroupStart[0],wrrGroupStart[1]);
   }
   for (i = aNodePtr->queuesData.pdqQueueFirst; i <= aNodePtr->queuesData.pdqQueueLast; i++, localIndex++)
    {
        params.quantum = (uint16_t)schedAtrributesPtr->weights[localIndex];

        /*DWRR group*/
        if (schedAtrributesPtr->wrrEnableBmp[localIndex>>5] & (1 << (localIndex&0x1F)))
        {
            /*dwrrGroupSize++;*/
            /*DWRR group 1*/
            if (schedAtrributesPtr->wrrGroupBmp[localIndex>>5] & (1<<(localIndex&0x1F)))
            {
                if(TM_INVAL==wrrGroupStart[1])
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrrGroupStart[1] invalid\n");
                }
                params.elig_prio_func_ptr = eligFuncArray[wrrGroupStart[1]];
            }
            else
            /*DWRR group 0*/
            {
                if(TM_INVAL==wrrGroupStart[0])
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrrGroupStart[0] invalid\n");
                }
                params.elig_prio_func_ptr = eligFuncArray[wrrGroupStart[0]];
            }
        }
        else
        /*SP group*/
        {
            /*In case there are more than 8 queues mapped, higher queues getting priority 7*/
            params.elig_prio_func_ptr = eligFuncArray[localIndex<7?localIndex:7];

        }
        /*Check  if minBw and/or  shaping is applied to this queue*/
        rc = prvCpssSip6TxqPdqCalculateEligPrioFuncForQlevelShaping(devNum, pdqNum, i, &params.elig_prio_func_ptr);
        if (rc)
        {
            return rc;
        }
        /*update Hw*/
        ret = prvCpssSchedUpdateQ(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                      i, &params);
        if (ret)
        {
            rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
            return rc;
        }
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqPdqUpdateTypeANodeSchedulingAttributes function
 * @endinternal
 *
 * @brief   Update scheduler HW with shadow scheduling profile attributes
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -                   Device number.
 * @param[in] pdqNum                                     - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] schedAtrributesPtr                  - (pointer to)Scheduling attributes
 * @param[in] lowLatencyQueueIsPresent     if equal GT_TRUE then it mean that there is at least one queue in SP group
 * @param[in] firstPnodeQueue                -      First queue in corresponding Pnode
 * @param[in] lastPnodeQueue                -      First queue in corresponding Pnode
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqPdqUpdateTypeANodeSchedulingAttributes
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE          *aNodePtr,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_PROFILE   *schedAtrributesPtr
)
{
    if(GT_FALSE==PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).spPrecedence)
    {
        return prvCpssSip6TxqPdqANodeSchedulingAttributesSpNonPrecedeConvert(devNum,pdqNum,aNodePtr,schedAtrributesPtr);
    }

    return prvCpssSip6TxqPdqANodeSchedulingAttributesSpPrecedeConvert(devNum,pdqNum,aNodePtr,schedAtrributesPtr);
}


/**
 * @internal prvCpssFalconTxqGlobalTxEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable transmission of specified tile on specific device
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] enable           -
 *                                      GT_TRUE, enable transmission
 *                                      GT_FALSE, disable transmission
 */
GT_STATUS   prvCpssFalconTxqPdqGlobalTxEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_BOOL enable
)
{
    int     ret = 0;
    GT_STATUS   rc  = GT_OK;
    ret = prvCpssSchedNodesTreeChangeStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], enable);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}
/**
 * @internal prvCpssFalconTxqGlobalTxEnableGet function
 * @endinternal
 *
 * @brief   Get the status of transmission of specified tile on specific device (Enable/Disable).
 *                Note this function check the status in all availeble tiles (The status should be the SAME)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *
 * @param[in] devNum                   - physical device number
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 *
 * @param[out] enablePtr                - (pointer to)
 *                                      GT_TRUE, enable transmission
 *                                      GT_FALSE, disable transmission
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS   prvCpssFalconTxqPdqGlobalTxEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_BOOL  * enablePtr
)
{
    int     ret = 0;
    GT_STATUS   rc  = GT_OK;
    uint8_t     status;
    ret = prvCpssSchedNodesTreeGetDeqStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], &status);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    if (rc == GT_OK)
    {
        *enablePtr = (status == 0) ? GT_FALSE : GT_TRUE;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqGetErrorStatus function
 * @endinternal
 *
 * @brief   Get the status of scheduler errors
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *
 * @param[in] devNum                   - physical device number
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 *
 * @param[out] errorCounterPtr                - (pointer to) error counter
 * @param[out] exceptionCounterPtr                - (pointer to) exception counter
 * @param[out] errorStatusPtr                - (pointer to) error status
 *
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS   prvCpssFalconTxqPdqGetErrorStatus
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32   * errorCounterPtr,
    IN GT_U32   * exceptionCounterPtr,
    IN GT_U32   * errorStatusPtr
)
{
    int         ret = 0;
    GT_STATUS       rc  = GT_OK;
    struct schedErrorInfo   info;
    ret = prvSchedGetErrors(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], &info);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    if (rc == GT_OK)
    {
        *errorCounterPtr    = info.error_counter;
        *exceptionCounterPtr    = info.exception_counter;
        ret         = prvSchedGetErrorStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], errorStatusPtr);
        rc          = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqGetQueueStatus function
 * @endinternal
 *
 * @brief   Read queue token bucket level and deficit
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *
 * @param[in] devNum                   - physical device number
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] queueNum                   - queue number .(APPLICABLE RANGES:0..2K-1)

 * @param[out] levelPtr                - (pointer to)  token bucket level
 * @param[out] deficitPtr                - (pointer to) deficit
 *
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS   prvCpssFalconTxqPdqGetQueueStatus
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 queueNum,
    OUT GT_32   *  levelPtr,
    OUT GT_U32   * dificitPtr
)
{
    int         ret = 0;
    GT_STATUS       rc  = GT_OK;
    struct schedNodeStatus  status;
    ret = prvSchedLowLevelGetQueueStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], queueNum, &status);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    if (rc == GT_OK)
    {
        *levelPtr = status.min_bucket_level;
        if (status.min_bucket_sign == 1)
        {
            (*levelPtr ) *= (-1);
        }
        *dificitPtr = status.deficit;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqGetAlevelStatus function
 * @endinternal
 *
 * @brief   Read A level token bucket level and deficit
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *
 * @param[in] devNum                   - physical device number
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] alevelNum                   - A level number .(APPLICABLE RANGES:0..511)

 * @param[out] levelPtr                - (pointer to)  token bucket level
 * @param[out] deficitPtr                - (pointer to) deficit
 *
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS   prvCpssFalconTxqPdqGetAlevelStatus
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 alevelNum,
    OUT GT_32   *  levelPtr,
    OUT GT_U32   * dificitPtr
)
{
    int         ret = 0;
    GT_STATUS       rc  = GT_OK;
    struct schedNodeStatus  status;
    ret = prvSchedLowLevelGetANodeStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], alevelNum, &status);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    if (rc == GT_OK)
    {
        *levelPtr = status.min_bucket_level;
        if (status.min_bucket_sign == 1)
        {
            (*levelPtr ) *= (-1);
        }
        *dificitPtr = status.deficit;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdqGetQueueShapingParameters function
 * @endinternal
 *
 * @brief   Read Q level shaping configurations
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *
 * @param[in] devNum                   - physical device number
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] queueNum                   - Q level number .(APPLICABLE RANGES:0..2047)

 * @param[out] divExpPtr                - (pointer to)  divider exponent
 * @param[out] tokenPtr                - (pointer to) number of tokens
 * @param[out] resPtr                - (pointer to)  resolution exponent
 * @param[out] burstPtr                - (pointer to) burst size
 *
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS   prvCpssFalconTxqPdqGetQueueShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 queueNum,
    IN PRV_QUEUE_SHAPING_ACTION_ENT rateType,
    OUT GT_U32                              *divExpPtr,
    OUT GT_U32                              *tokenPtr,
    OUT GT_U32                              *resPtr,
    OUT GT_U32                              *burstPtr
)
{
    int                     ret = 0;
    GT_STATUS                   rc  = GT_OK;
    struct prvCpssDxChTxqSchedShapingProfile    profile;
    ret = prvSchedLowLevelGetNodeShapingParametersFromHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                 SCHED_Q_LEVEL, queueNum, &profile);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    if (rc == GT_OK)
    {
        if (rateType == PRV_QUEUE_SHAPING_ACTION_PRIORITY_DOWNGRADE_ENT)
        {
            *divExpPtr  = profile.max_div_exp;
            *tokenPtr   = profile.max_token;
            *resPtr     = profile.max_token_res;
            *burstPtr   = profile.max_burst_size;
        }
        else
        {
            *divExpPtr  = profile.min_div_exp;
            *tokenPtr   = profile.min_token;
            *resPtr     = profile.min_token_res;
            *burstPtr   = profile.min_burst_size;
        }
    }
    return rc;
}
/**
 * @internal prvCpssSip6TxqPdqPerLevelShapingParametersGet function
 * @endinternal
 *
 * @brief   Read per  level shaping configurations
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *
 * @param[in] devNum                   - physical device number
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] level                   -          tree level
*
 * @param[out] shaperDecPtr                - (pointer to) shaper decopling
 * @param[out] perInterPtr                - (pointer to) periodic interval
 * @param[out] perEnPtr                - (pointer to)  per level shaping enabled
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS   prvCpssSip6TxqPdqPerLevelShapingParametersGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN PRV_CPSS_PDQ_LEVEL_ENT level,
    OUT GT_U32   *  shaperDecPtr,
    OUT GT_U32   * perInterPtr,
    OUT GT_U32   * perEnPtr
)
{
    int                 ret = 0;
    GT_STATUS               rc  = GT_OK;
    struct schedPerLevelPeriodicParams  periodicParams;
    ret = prvSchedLowLevelGetPerLevelShapingStatusFromHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                 PRV_CPSS_LEVEL_CONVERT_TO_TM_FORMAT_MAC(level), &periodicParams);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    if (rc == GT_OK)
    {
        *shaperDecPtr   = periodicParams.shaper_dec;
        *perInterPtr    = periodicParams.per_interval;
        *perEnPtr   = periodicParams.per_state;
    }
    return rc;
}
/**
 * @internal prvCpssSip6TxqPdqNodeShapingParametersGet function
 * @endinternal
 *
 * @brief   Read A level shaping configurations
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *
 * @param[in] devNum                   - physical device number
 * @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
 * @param[in] nodeNum                   - node index.(APPLICABLE RANGES:0..511)
 * @param[in] aLevel                   -if equal GT_TRUE  this is A level node ,else B level
 * @param[out] divExpPtr                - (pointer to)  divider exponent
 * @param[out] tokenPtr                - (pointer to) number of tokens
 * @param[out] resPtr                - (pointer to)  resolution exponent
 * @param[out] burstPtr                - (pointer to) burst size
 *
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS   prvCpssSip6TxqPdqNodeShapingParametersGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeNum,
    IN PRV_CPSS_PDQ_LEVEL_ENT level,
    OUT GT_U32   *  divExpPtr,
    OUT GT_U32   * tokenPtr,
    OUT GT_U32   * resPtr,
    OUT GT_U32   * burstPtr
)
{
    int                     ret = 0;
    GT_STATUS                   rc  = GT_OK;
    struct prvCpssDxChTxqSchedShapingProfile    profile;

    ret = prvSchedLowLevelGetNodeShapingParametersFromHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                 PRV_CPSS_LEVEL_CONVERT_TO_TM_FORMAT_MAC(level),nodeNum, &profile);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    if (rc == GT_OK)
    {
        *divExpPtr  = profile.min_div_exp;
        *tokenPtr   = profile.min_token;
        *resPtr     = profile.min_token_res;
        *burstPtr   = profile.min_burst_size;
    }
    return rc;
}


GT_STATUS   prvCpssSip6TxqPdqAlevelQuantumSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 aNodeIndex,
    IN GT_U32 quantum
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_a_node_params  params;
    params.quantum  = (uint16_t)quantum;
    ret     = prvCpssSchedUpdateNodeTypeA(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], aNodeIndex, &params, PRV_CPSS_NODE_UPDATE_TYPE_QUANTUM_E);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}


GT_STATUS   prvCpssSip6TxqPdqAlevelQuantumGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 aNodeIndex,
    IN GT_U32 *quantumPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_a_node_params  params;

    ret     = prvCpssSchedNodesReadANodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], aNodeIndex, &params);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    *quantumPtr = params.quantum;

    return rc;
}


GT_STATUS   prvCpssSip6TxqPdqAlevelPrioritySet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 aNodeIndex,
    IN GT_U32 priorityGroupIndex
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_a_node_params  params;
    GT_BOOL shapingEnabled = GT_FALSE;
    rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, pdqNum, aNodeIndex, PRV_CPSS_PDQ_LEVEL_A_E, &shapingEnabled);
    if (rc)
    {
        return rc;
    }

    PRV_CPSS_PRIORITY_FUNC_CONFIGURE_GET_MAC(priorityGroupIndex,shapingEnabled,params.elig_prio_func_ptr);


    ret     = prvCpssSchedUpdateNodeTypeA(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
        aNodeIndex, &params, PRV_CPSS_NODE_UPDATE_TYPE_ELIG_FUNC_E);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}

GT_STATUS   prvCpssSip6TxqPdqAlevelPriorityGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 aNodeIndex,
    IN GT_U32 * priorityGroupIndexPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_a_node_params  params;

    ret     = prvCpssSchedNodesReadANodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], aNodeIndex, &params);
    if(ret==0)
    {
        PRV_CPSS_PRIORITY_GROUP_INDEX_GET_MAC(* priorityGroupIndexPtr,params.elig_prio_func_ptr);
    }
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    return rc;
}



GT_STATUS   prvCpssSip6TxqPdqBlevelPrioritySet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 bNodeIndex,
    IN GT_U32 priorityGroupIndex
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_b_node_params  params;
    GT_BOOL shapingEnabled = GT_FALSE;
    rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, pdqNum, bNodeIndex, PRV_CPSS_PDQ_LEVEL_B_E, &shapingEnabled);
    if (rc)
    {
        return rc;
    }

    PRV_CPSS_PRIORITY_FUNC_CONFIGURE_GET_MAC(priorityGroupIndex,shapingEnabled,params.elig_prio_func_ptr);


    ret     = prvCpssSchedUpdateNodeTypeB(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
        bNodeIndex, &params, PRV_CPSS_NODE_UPDATE_TYPE_ELIG_FUNC_E);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}


GT_STATUS   prvCpssSip6TxqPdqBlevelPriorityGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 bNodeIndex,
    IN GT_U32 * priorityGroupIndexPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_b_node_params  params;

    ret     = prvCpssSchedNodesReadBNodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], bNodeIndex, &params);
    if(ret==0)
    {
        PRV_CPSS_PRIORITY_GROUP_INDEX_GET_MAC(* priorityGroupIndexPtr,params.elig_prio_func_ptr);
    }
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    return rc;
}


GT_STATUS   prvCpssSip6TxqPdqBlevelQuantumSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 bNodeIndex,
    IN GT_U32 quantum
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_b_node_params  params;
    params.quantum  = (uint16_t)quantum;
    ret     = prvCpssSchedUpdateNodeTypeB(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], bNodeIndex, &params, PRV_CPSS_NODE_UPDATE_TYPE_QUANTUM_E);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}


GT_STATUS   prvCpssSip6TxqPdqBlevelQuantumGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 bNodeIndex,
    IN GT_U32 *quantumPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_b_node_params  params;

    ret     = prvCpssSchedNodesReadBNodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], bNodeIndex, &params);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    *quantumPtr = params.quantum;

    return rc;
}

GT_STATUS   prvCpssSip6TxqPdqClevelQuantumSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 cNodeIndex,
    IN GT_U32 quantum
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_c_node_params  params;
    params.quantum  = (uint16_t)quantum;
    ret     = prvCpssSchedUpdateNodeTypeC(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], cNodeIndex, &params, PRV_CPSS_NODE_UPDATE_TYPE_QUANTUM_E);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}


GT_STATUS   prvCpssSip6TxqPdqClevelQuantumGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 cNodeIndex,
    IN GT_U32 *quantumPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_c_node_params  params;

    ret     = prvCpssSchedNodesReadCNodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], cNodeIndex, &params);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    *quantumPtr = params.quantum;

    return rc;
}



GT_STATUS   prvCpssSip6TxqPdqClevelPrioritySet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 cNodeIndex,
    IN GT_U32 priorityGroupIndex
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_c_node_params  params;
    GT_BOOL shapingEnabled = GT_FALSE;
    rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, pdqNum, cNodeIndex, PRV_CPSS_PDQ_LEVEL_C_E, &shapingEnabled);
    if (rc)
    {
        return rc;
    }

    PRV_CPSS_PRIORITY_FUNC_CONFIGURE_GET_MAC(priorityGroupIndex,shapingEnabled,params.elig_prio_func_ptr);


    ret     = prvCpssSchedUpdateNodeTypeC(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
        cNodeIndex, &params, PRV_CPSS_NODE_UPDATE_TYPE_ELIG_FUNC_E);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
    return rc;
}


GT_STATUS   prvCpssSip6TxqPdqClevelPriorityGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 cNodeIndex,
    IN GT_U32 * priorityGroupIndexPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    struct sched_c_node_params  params;

    ret     = prvCpssSchedNodesReadCNodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], cNodeIndex, &params);
    if(ret==0)
    {
        PRV_CPSS_PRIORITY_GROUP_INDEX_GET_MAC(*priorityGroupIndexPtr,params.elig_prio_func_ptr);
    }
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    return rc;
}


/**
 * @internal prvCpssFalconTxqPdqClose function
 * @endinternal
 *
 * @brief  Release dynamic memory allocation for tile specific TxQ scheduler
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum             -physical device number
 * @param[in] pdqNum             -number of PDQ[0..3]
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqPdqClose
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum
)
{
    int     ret = 0;
    GT_STATUS   rc;

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum] == NULL)
    {
        /* nothing to free */
        return GT_OK;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[pdqNum].general.addressSpacePtr != NULL)
    {
        cpssOsFree(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[pdqNum].general.addressSpacePtr);
    }

    /* Scheduler destructor*/
    ret = prvSchedLibCloseSw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum]);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    if (PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E == PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(devNum))
    {
        (GT_VOID)prvDxChTxqSip7AllocManagerFreeAllRanges(devNum,pdqNum);
    }

    return rc;
}
static GT_BOOL prvCpssFalconTxqPdqMinBwStatusForQlevelGet
(
    GT_U32 eligPrioFunc,
    GT_U32 *typePtr
)
{

    GT_U32 minBwType=0;
    GT_BOOL shaping,minBw = GT_FALSE;

    (GT_VOID) prvSchedQueueEligFuncInfoGet(eligPrioFunc,&shaping,&minBw,&minBwType);
    if(typePtr)
    {
        *typePtr = minBwType;
    }

    return minBw;
}

GT_STATUS prvCpssSip6TxqPdqQSchedParamsGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeInd,
    OUT GT_U32 *eligFuncPtr,
    OUT GT_U32 *quantumPtr
)
{
    int             ret;
    GT_STATUS           rc = GT_OK;
    struct sched_queue_params   queueParams;
    ret = prvCpssSchedNodesReadQueueConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              nodeInd, &queueParams);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }

    *eligFuncPtr = queueParams.elig_prio_func_ptr;
    *quantumPtr = queueParams.quantum;

    return GT_OK;
}

GT_STATUS prvCpssSip6TxqPdqQSchedParamsSet
(
    IN GT_U8  devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeInd,
    IN GT_U32 eligFunc,
    IN GT_U32 quantum
)
{
    int             ret;
    GT_STATUS           rc = GT_OK;
    struct sched_queue_params   queueParams;

    queueParams.elig_prio_func_ptr = (uint8_t)eligFunc;
    queueParams.quantum = (uint16_t)quantum;
    queueParams.shaping_profile_ref = TM_INVAL;

    ret = prvCpssSchedUpdateQ(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                      nodeInd, &queueParams);

    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }

    return GT_OK;
}


GT_STATUS prvCpssFalconTxqPdqMinBwEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeInd,
    OUT GT_BOOL                     *enablePtr,
    OUT GT_U32                      *typePtr
)
{
    int             ret;
    GT_STATUS           rc = GT_OK;
    struct sched_queue_params   queueParams;
    ret = prvCpssSchedNodesReadQueueConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              nodeInd, &queueParams);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }
    *enablePtr = prvCpssFalconTxqPdqMinBwStatusForQlevelGet(queueParams.elig_prio_func_ptr,typePtr);
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqPdqMinBwOnNodeSet function
 * @endinternal
 *
 * @brief   Write shaping parameters to specific node
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] pdqNum                   - PDQ index[0..3]
 * @param[in] nodeInd                  -      Q-LEVEL node index
 * @param[in] burstSize                - burst size in units of 4K bytes
 *                                       max value is 1K-1 which results in 4M-1 burst size
 * @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
 * @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqPdqMinBwOnNodeSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 nodeInd,
    IN GT_U16 burstSize,
    INOUT GT_U32                       *maxRatePtr
)
{
    int                 ret;
    GT_STATUS               rc = GT_OK;
    struct sched_shaping_profile_params params;
    uint16_t                profile = SCHED_DIRECT_NODE_SHAPING;
    GT_U16                  burstSizeFromHw;
    GT_BOOL                 shapingEnable;
    ret = prvCpssSchedReadNodeShapingConfigFromShadow(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              SCHED_Q_LEVEL, nodeInd, &profile, &params);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }

    /*check that shaping is enabled on node*/

    rc = prvCpssFalconTxqPdqShapingEnableGet(devNum,pdqNum,nodeInd,PRV_CPSS_PDQ_LEVEL_Q_E,&shapingEnable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqShapingEnableGet  failed  ");
    }

    /*check that  CIR is greater then EIR*/
    if (GT_TRUE==shapingEnable&&params.cir_bw < *maxRatePtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Shaping rate  %d should be greater then minimal BW rate %d ", params.cir_bw, *maxRatePtr);
    }

    if(0==PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.bucketUsedForShaping)
    {
        /*update only EIR*/
        params.eir_bw = *maxRatePtr;
        /* burst size in units of 1K bytes*/
        params.ebs  = burstSize;
    }
    else
    {
        /*update only CIR*/
        params.cir_bw = *maxRatePtr;
        /* burst size in units of 1K bytes*/
        params.cbs  = burstSize;
    }
    ret     = prvCpssSchedShapingUpdateNodeShaping(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                   SCHED_Q_LEVEL,
                                   nodeInd, &params, NULL, maxRatePtr);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }
    rc = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, pdqNum, nodeInd, PRV_CPSS_PDQ_LEVEL_Q_E,
                           PRV_QUEUE_SHAPING_ACTION_PRIORITY_DOWNGRADE_ENT, &burstSizeFromHw, maxRatePtr);
    return rc;
}
GT_STATUS prvCpssSip6TxqPdqCalculateActualBwGet
(
    IN GT_U32 devNum,
    IN GT_32 token,
    IN GT_U32 res,
    IN GT_U32 divExp,
    IN PRV_CPSS_PDQ_LEVEL_ENT level,
    OUT GT_U32                     *maxRatePtr
)
{
    double      tmp;
    GT_U32      freqInKHz, shaperDec, perInter, perEn;
    int     ret;
    GT_STATUS   rc;
    rc = prvCpssSip6TxqPdqPerLevelShapingParametersGet(devNum, 0, level, &shaperDec, &perInter, &perEn);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqPdqPerLevelShapingParametersGet  failed  ");
    }
    ret = prvCpssTxqSchedulerGetOptimalShaperUpdateFrequency(devNum, &freqInKHz, 0);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);
        return rc;
    }
    freqInKHz   /= 1000;
    tmp     = (double)(/*multiply by 100000  in order to avoid loss of precision */ PRECISION_FACTOR * token * (1 << res)) / (perInter * (1 << divExp));
    *maxRatePtr = (GT_U32)(0.5 + ((freqInKHz * 8) * tmp) / PRECISION_FACTOR );
    if(GT_TRUE==PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).logShapingResults )
    {
        cpssOsPrintf("\nNode shaping parameters from HW\n");
        cpssOsPrintf("freqInKHz = %d \n",freqInKHz);
        cpssOsPrintf("res = %d \n",res);
        cpssOsPrintf("perInter = %d \n",perInter);
        cpssOsPrintf("divider_exp = %d \n",divExp);
        cpssOsPrintf("token = %d \n",token);

    }
    return GT_OK;
}
GT_STATUS prvCpssFalconTxqPdqShaperInputFreqGet
(
    IN GT_U32 devNum,
    IN GT_U32 tileNum,
    IN GT_U32                     *freqPtr
)
{
    int     ret;
    GT_STATUS   rc;
    ret = prvSchedInputFreqGet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum], freqPtr);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, tileNum);
        return rc;
    }
    return GT_OK;
}


GT_STATUS prvCpssSip6TxqPdqShaperPeriodSet
(
    IN GT_U32 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 period,
    IN GT_U32 resolution
)
{
    int     ret;
    GT_STATUS   rc;
    ret = prvCpssTxqSchedulerQLevelPeriodSet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum], period,resolution);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, tileNum);
        return rc;
    }
    return GT_OK;
}

GT_U32 prvCpssSip6TxqPdqMinNodeQuantumGet
(
    IN GT_U32 devNum
)
{
    return (prvCpssTxqUtilsCreditSizeGet(devNum)/prvCpssSchedGetNodeQuantumChunkSize());
}



GT_VOID prvSchedToCpssErrCodeConvertDump
(
    IN GT_U32 errorCode
)
{
    cpssOsPrintf("Error code %d is mapped to CPSS status %d\n",errorCode,prvSchedToCpssErrCodeConvert(errorCode));
}

GT_STATUS prvCpssSip6TxqPdqInitAdressSpace
(
    IN GT_U32 tileNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(tileConfigsPtr);

    /*init only for 0*/
    if(tileNum == 0)
    {
        tileConfigsPtr->general.addressSpacePtr = (struct prvCpssDxChTxqSchedAddressSpace *)cpssOsMalloc(sizeof(struct prvCpssDxChTxqSchedAddressSpace));
        if(tileConfigsPtr->general.addressSpacePtr== NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

GT_CHAR_PTR prvCpssSip6TxqEligFuncNameGet
(
    unsigned int eligFunc
)
{
    GT_CHAR_PTR namePtr;

    switch(eligFunc)
    {
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PRIO1_E)/*0*/
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PRIO5_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_SHP_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PPA_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PPA_SP_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PPA_SHP_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PPA_SP_MIN_SHP_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PPA_SHP_IGN_E)/*8*/
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PPA_MIN_SHP_SP_IGN_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_FP0_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_FP1_E )
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_FP2_E )
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_FP3_E) /*13*/
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_FP4_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_FP5_E )
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_FP6_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_FP7_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP0_E)/*18*/
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP1_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP2_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP3_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP4_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP5_E)/*23*/
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP6_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP7_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MAX_INC_MIN_SHP_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PP_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_PP_E)/*28*/
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_PP_SHP_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_SHP_4P_MIN_4P_MAX_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_SHP_PP_TB_E)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_SHP_PP_MAX_TB_0)
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_MAX_LIM_SHP_FP)/*33*/
        PRV_CPSS_ELIG_FUNC_TO_STR(CPSS_PDQ_SCHED_ELIG_N_DEQ_DIS_E)/*63*/

        default:
            namePtr = "Unknown";
            break;
    }

    return namePtr;
}

GT_CHAR_PTR prvCpssSip6TxqQueueEligFuncNameGet
(
    unsigned int eligFunc
)
{
    GT_CHAR_PTR namePtr;

    switch(eligFunc)
    {
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_PRIO0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_PRIO1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_PRIO2)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_PRIO3)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_PRIO4)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_PRIO5)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_PRIO6)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_PRIO7)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO2)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO3)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO4)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO5)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO6)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO7)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q1_MIN_BW_TYPE0_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q2_MIN_BW_TYPE0_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q3_MIN_BW_TYPE0_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q4_MIN_BW_TYPE0_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q5_MIN_BW_TYPE0_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q6_MIN_BW_TYPE0_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q7_MIN_BW_TYPE0_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q1_MIN_BW_TYPE0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q2_MIN_BW_TYPE0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q3_MIN_BW_TYPE0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q4_MIN_BW_TYPE0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q5_MIN_BW_TYPE0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q6_MIN_BW_TYPE0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q7_MIN_BW_TYPE0)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q1_MIN_BW_TYPE1_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q2_MIN_BW_TYPE1_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q3_MIN_BW_TYPE1_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q4_MIN_BW_TYPE1_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q5_MIN_BW_TYPE1_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q6_MIN_BW_TYPE1_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q7_MIN_BW_TYPE1_WITH_SHAPING)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q1_MIN_BW_TYPE1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q2_MIN_BW_TYPE1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q3_MIN_BW_TYPE1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q4_MIN_BW_TYPE1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q5_MIN_BW_TYPE1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q6_MIN_BW_TYPE1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q7_MIN_BW_TYPE1)
      PRV_CPSS_ELIG_FUNC_TO_STR(PRV_SCHED_ELIG_Q_DEQ_DIS_E)
      default:
            namePtr = "Unknown";
            break;

    }

    return namePtr;
}



GT_CHAR_PTR prvCpssSip6TxqQueueEligFuncDescriptionGet
(
    unsigned int eligFunc
)
{
    GT_CHAR_PTR descPtr;

    switch(eligFunc)
    {
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_PRIO0,"Prio 0 no minimal BW /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_PRIO1,"Prio 1 no minimal BW /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_PRIO2,"Prio 2 no minimal BW /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_PRIO3,"Prio 3 no minimal BW /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_PRIO4,"Prio 4 no minimal BW /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_PRIO5,"Prio 5 no minimal BW /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_PRIO6,"Prio 6 no minimal BW /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_PRIO7,"Prio 7 no minimal BW /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO0,"Prio 0 no minimal BW /shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO1,"Prio 1 no minimal BW /shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO2,"Prio 2 no minimal BW /shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO3,"Prio 3 no minimal BW /shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO4,"Prio 4 no minimal BW /shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO5,"Prio 5 no minimal BW /shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO6,"Prio 6 no minimal BW /shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q_MIN_SHP_PRIO7,"Prio 7 no minimal BW /shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0_WITH_SHAPING,"Prio 0 with minimal BW (downgrade to 0)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q1_MIN_BW_TYPE0_WITH_SHAPING,"Prio 1 with minimal BW (downgrade to 0)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q2_MIN_BW_TYPE0_WITH_SHAPING,"Prio 2 with minimal BW (downgrade to 0)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q3_MIN_BW_TYPE0_WITH_SHAPING,"Prio 3 with minimal BW (downgrade to 0)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q4_MIN_BW_TYPE0_WITH_SHAPING,"Prio 4 with minimal BW (downgrade to 0)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q5_MIN_BW_TYPE0_WITH_SHAPING,"Prio 5 with minimal BW (downgrade to 0)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q6_MIN_BW_TYPE0_WITH_SHAPING,"Prio 6 with minimal BW (downgrade to 0)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q7_MIN_BW_TYPE0_WITH_SHAPING,"Prio 7 with minimal BW (downgrade to 0)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0,"Prio 0 with minimal BW (downgrade to 0) /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q1_MIN_BW_TYPE0,"Prio 1 with minimal BW (downgrade to 0)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q2_MIN_BW_TYPE0,"Prio 2 with minimal BW (downgrade to 0)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q3_MIN_BW_TYPE0,"Prio 3 with minimal BW (downgrade to 0)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q4_MIN_BW_TYPE0,"Prio 4 with minimal BW (downgrade to 0)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q5_MIN_BW_TYPE0,"Prio 5 with minimal BW (downgrade to 0)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q6_MIN_BW_TYPE0,"Prio 6 with minimal BW (downgrade to 0)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q7_MIN_BW_TYPE0,"Prio 7 with minimal BW (downgrade to 0)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1_WITH_SHAPING,"Prio 0 with minimal BW (downgrade to original priority)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q1_MIN_BW_TYPE1_WITH_SHAPING,"Prio 1 with minimal BW (downgrade to original priority)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q2_MIN_BW_TYPE1_WITH_SHAPING,"Prio 2 with minimal BW (downgrade to original priority)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q3_MIN_BW_TYPE1_WITH_SHAPING,"Prio 3 with minimal BW (downgrade to original priority)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q4_MIN_BW_TYPE1_WITH_SHAPING,"Prio 4 with minimal BW (downgrade to original priority)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q5_MIN_BW_TYPE1_WITH_SHAPING,"Prio 5 with minimal BW (downgrade to original priority)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q6_MIN_BW_TYPE1_WITH_SHAPING,"Prio 6 with minimal BW (downgrade to original priority)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q7_MIN_BW_TYPE1_WITH_SHAPING,"Prio 7 with minimal BW (downgrade to original priority)/shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1,"Prio 0 with minimal BW (downgrade to original priority) /no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q1_MIN_BW_TYPE1,"Prio 1 with minimal BW (downgrade to original priority)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q2_MIN_BW_TYPE1,"Prio 2 with minimal BW (downgrade to original priority)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q3_MIN_BW_TYPE1,"Prio 3 with minimal BW (downgrade to original priority)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q4_MIN_BW_TYPE1,"Prio 4 with minimal BW (downgrade to original priority)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q5_MIN_BW_TYPE1,"Prio 5 with minimal BW (downgrade to original priority)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q6_MIN_BW_TYPE1,"Prio 6 with minimal BW (downgrade to original priority)/no shaping")
      PRV_CPSS_ELIG_FUNC_TO_DESCRIPTION(PRV_SCHED_ELIG_Q7_MIN_BW_TYPE1,"Prio 7 with minimal BW (downgrade to original priority)/no shaping")

      default:
            descPtr = "no description";
            break;

    }

    return descPtr;
}
/**
* @internal prvCpssSip6TxqPdqNodeShapingCreditAccumulationEnableSet function
* @endinternal
*
* @brief       Set enable/Disable credit accumulation. Required for 802.1.Q 8.6.8.2 (previously 802.1Qav)
*              A frame is only allowed to transmit when the corresponding credit is not negative.
*              If credit accumulation is disabled ,then positive credit is set to zero if there is no frame of the corresponding class.
*              Note : Default behavior is enabling credit accumulation
*
* @note   APPLICABLE DEVICES:          Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; .
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                  - tile number
* @param[in] queueNum              -  offset of the queue in PDQ
* @param[in] enable                   -  GT_TRUE,  Credit is accumulated until max burst size.
*                                                                  GT_FALSE, Credit is not accumulated .Positive credit is set to zero if there is no frame of the corresponding class.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssSip6TxqPdqNodeShapingCreditAccumulationEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 queueNum,
    IN GT_BOOL enable
)
{
    int                     ret = 0;
    GT_STATUS                   rc  = GT_OK;
    struct prvCpssDxChTxqSchedShapingProfile    profile;
    profile.level = SCHED_Q_LEVEL;
    ret = prvSchedLowLevelGetNodeShapingParametersFromHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                 SCHED_Q_LEVEL, queueNum, &profile);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }

    if(profile.credit_accumulation_disable== BOOL2BIT_MAC(enable))
    {
        profile.credit_accumulation_disable = (GT_TRUE==enable)?0:1;

        /*

                Token resolution  will be mutiplied by correct resolution exponent at set function.
                 So we need to normalize here .

                 */

        if(profile.min_token_res>0)
        {
            profile.min_token_res =1;
        }

        if(profile.max_token_res>0)
        {
            profile.max_token_res =1;
        }

        ret = prvSchedLowLevelSetNodeShapingParametersToHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                 SCHED_Q_LEVEL, queueNum, &profile);
        if (ret)
        {
            rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvCpssSip6TxqPdqNodeShapingCreditAccumulationEnableGet function
* @endinternal
*
* @brief       Get enable/Disable credit accumulation. Required for 802.1.Q 8.6.8.2 (previously 802.1Qav)
*              A frame is only allowed to transmit when the corresponding credit is not negative.
*              If credit accumulation is disabled ,then positive credit is set to zero if there is no frame of the corresponding class.
*              Note : Default behavior is enabling credit accumulation
*
* @note   APPLICABLE DEVICES:          Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; .
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                  - tile number
* @param[in] queueNum              -  offset of the queue in PDQ
* @param[out] enablePtr                   - (pointer to)  GT_TRUE,  Credit is accumulated until max burst size.
*                                                                  GT_FALSE, Credit is not accumulated .Positive credit is set to zero if there is no frame of the corresponding class.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssSip6TxqPdqNodeShapingCreditAccumulationEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 queueNum,
    IN GT_BOOL *enablePtr
)
{
    int                     ret = 0;
    GT_STATUS               rc  = GT_OK;
    struct prvCpssDxChTxqSchedShapingProfile    profile;
    ret = prvSchedLowLevelGetNodeShapingParametersFromHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                 SCHED_Q_LEVEL, queueNum, &profile);
    if (ret)
    {
        rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
        return rc;
    }

    if(0x1 == profile.credit_accumulation_disable)
    {
        *enablePtr =GT_FALSE;
    }
    else
    {
        *enablePtr =GT_TRUE;
    }

    return rc;
}

GT_STATUS   prvCpssSip6_30TxqPdqSpeedToSlicesGet
(
    IN GT_U8 devNum,
    IN CPSS_PORT_SPEED_ENT speed,
    IN GT_U32 *sliceNumPtr
)
{
     PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);

     switch(speed)
     {
        case CPSS_PORT_SPEED_10000_E:
        case CPSS_PORT_SPEED_12000_E:
            *sliceNumPtr = 8;
            break;
        case CPSS_PORT_SPEED_5000_E:
            *sliceNumPtr = 4;
            break;
        case CPSS_PORT_SPEED_2500_E:
            *sliceNumPtr = 2;
            break;
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_1000_E:
        default:
            *sliceNumPtr = 1;
            break;
     }

     return GT_OK;

}


GT_STATUS   prvCpssSip7TxqPdqSpeedToSlicesGet
(
    IN GT_U8 devNum,
    IN CPSS_PORT_SPEED_ENT speed,
    IN GT_U32 *sliceNumPtr
)
{
     PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AAS_E);

     switch(speed)
     {
        case CPSS_PORT_SPEED_400G_E:
        case CPSS_PORT_SPEED_424G_E:
            *sliceNumPtr = 8;
            break;
        case CPSS_PORT_SPEED_200G_E:
        case CPSS_PORT_SPEED_212G_E:
            *sliceNumPtr = 4;
            break;
        case CPSS_PORT_SPEED_140G_E:
            *sliceNumPtr = 3;
            break;
        case CPSS_PORT_SPEED_100G_E:
        case CPSS_PORT_SPEED_107G_E:
        case CPSS_PORT_SPEED_102G_E:
        case CPSS_PORT_SPEED_106G_E:
            *sliceNumPtr = 2;
            break;
        default:
            *sliceNumPtr = 1;
            break;
     }

     return GT_OK;

}


GT_STATUS prvCpssSip6_30TxqPdqPortPizzaLastSliceSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 lastSlice
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if(lastSlice>=PRV_TXQ_SIP6_30_PIZZA_MAX_SLICE_NUM)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Last slice is too big %d [should be less then %d]\n",lastSlice,PRV_TXQ_SIP6_30_PIZZA_MAX_SLICE_NUM);
    }

    ret     = prvSchedLowLevelPortPizzaLastSliceSet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],lastSlice);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);
    return rc;
}

GT_STATUS prvCpssSip6_30TxqPdqPortPizzaLastSliceGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 *lastSlicePtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(lastSlicePtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    ret     = prvSchedLowLevelPortPizzaLastSliceGet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],lastSlicePtr);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);
    return rc;
}


static  GT_STATUS prvCpssSip6_30TxqPdqPizzaChunkAdd
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum
)
{
    GT_STATUS           rc  = GT_OK;
    GT_U32              lastSlice =0 ;
    int                 ret = 0;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    rc      = prvCpssSip6_30TxqPdqPortPizzaLastSliceGet(devNum,tileNum,&lastSlice);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqPortPizzaLastSliceGet failed \n");
    }

    rc      = prvCpssSip6_30TxqPdqPortPizzaLastSliceSet(devNum,tileNum,lastSlice+PRV_TXQ_SIP6_30_PIZZA_SLICE_CHUNK_SIZE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqPortPizzaLastSliceGet failed \n");
    }

    ret     = prvSchedLowLevelPizzaSliceEmptyLineSet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],(lastSlice>>2)+1);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);
    return rc;
}


static GT_STATUS prvCpssSip6_30TxqPdqPizzaSliceDelete
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sliceInd
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if(sliceInd>=PRV_TXQ_SIP6_30_PIZZA_MAX_SLICE_NUM)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Slice index is too big %d [should be less then %d]\n",sliceInd,PRV_TXQ_SIP6_30_PIZZA_MAX_SLICE_NUM);
    }

    ret     = prvSchedLowLevelPizzaSliceSet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],sliceInd,PRV_TXQ_PIZZA_EMPTY_SLICE_VALUE_CNS(devNum));
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);
    return rc;
}


GT_STATUS prvCpssSip6_30TxqPdqPizzaSliceSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sliceInd,
    IN GT_U32 portInd
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if(sliceInd>=PRV_TXQ_SIP6_30_PIZZA_MAX_SLICE_NUM)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Slice index is too big %d [should be less then %d]\n",sliceInd,PRV_TXQ_SIP6_30_PIZZA_MAX_SLICE_NUM);
    }

    ret     = prvSchedLowLevelPizzaSliceSet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],sliceInd,portInd);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);
    return rc;
}


GT_STATUS prvCpssSip6_30TxqPdqPizzaSliceLineGet
(
    IN GT_U8     devNum,
    IN GT_U32    tileNum,
    IN GT_U32    lineInd,
    OUT GT_U32   *dataPtr
)
{
    int             ret = 0;
    GT_STATUS           rc  = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    ret     = prvSchedLowLevelPizzaSliceLineGet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],lineInd,dataPtr);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);


    return rc;
}


GT_STATUS prvCpssSip6_30TxqPdqPnodeSpeedSet
(
    IN GT_U8     devNum,
    IN GT_U32    tileNum,
    IN GT_U32    pNodeIndSet,
    IN CPSS_PORT_SPEED_ENT speed
)
{
    GT_STATUS           rc  = GT_OK;
    GT_U32              sliceNum =0 ;
    GT_U32              lastSlice =0 ;
    GT_U32              i,j;
    GT_U32              numOfLines;
    GT_U32              data;
    GT_U32              mask;
    GT_U32              lastIndex=0xFFFFFFFE;
    GT_U32              currentIndex;
    GT_U32              pNodeInd;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        return GT_OK;
    }

    if(CPSS_PORT_SPEED_NA_E!=speed)
    {

        if(GT_TRUE==PRV_CPSS_SIP_7_CHECK_MAC(devNum))
        {
            rc = prvCpssSip7TxqPdqSpeedToSlicesGet(devNum, speed,&sliceNum);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip7TxqPdqSpeedToSlicesGet failed\n");
            }
        }
        else
        {
            rc  = prvCpssSip6_30TxqPdqSpeedToSlicesGet(devNum, speed,&sliceNum);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqSpeedToSlicesGet failed\n");
            }
        }
    }

    /*Get the size of pizza*/
    rc = prvCpssSip6_30TxqPdqPortPizzaLastSliceGet(devNum,tileNum,&lastSlice);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqPortPizzaLastSliceGet failed\n");
    }



    numOfLines = ((lastSlice+1)>>2)+(((lastSlice+1)&0x3)?1:0);
    mask = (1<<PRV_TXQ_PIZZA_SLICE_WIDTH_IN_BITS(devNum))-1;

    if(0!=sliceNum)
    {
        for(i=0;i<numOfLines&&sliceNum>0;i++)
        {
            rc = prvCpssSip6_30TxqPdqPizzaSliceLineGet(devNum,tileNum,i,&data);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqPizzaSliceLineGet failed\n");
            }
            for(j=0;j<PRV_TXQ_SIP6_30_PIZZA_SLICE_PER_LINE&&sliceNum>0;j++)
            {
                pNodeInd = data&mask;
                data>>=PRV_TXQ_PIZZA_SLICE_WIDTH_IN_BITS(devNum);
                currentIndex = PRV_TXQ_SIP6_30_PIZZA_SLICE_PER_LINE*i+j;

                if(PRV_TXQ_PIZZA_EMPTY_SLICE_VALUE_CNS(devNum)==pNodeInd)
                {
                    if((0xFFFFFFFE==lastIndex)||(currentIndex-lastIndex>PRV_TXQ_SIP6_30_PIZZA_SLICE_MIN_GAP_CNS))
                    {
                        /*found one that fit*/
                        rc = prvCpssSip6_30TxqPdqPizzaSliceSet(devNum,tileNum,currentIndex,pNodeIndSet);
                        if(rc!=GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqPizzaSliceSet failed\n");
                        }
                        lastIndex = currentIndex;
                        sliceNum--;
                    }
                }
            }

            if((i==numOfLines-1)&&(sliceNum>0))
            {
                /*increase the size by one line*/
                rc = prvCpssSip6_30TxqPdqPizzaChunkAdd(devNum,tileNum);
                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqPizzaChunkAdd failed\n");
                }

                numOfLines++;
            }
        }
    }
    else
    {
        for(i=0;i<numOfLines;i++)
        {
            rc = prvCpssSip6_30TxqPdqPizzaSliceLineGet(devNum,tileNum,i,&data);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqPizzaSliceLineGet failed\n");
            }
            for(j=0;j<PRV_TXQ_SIP6_30_PIZZA_SLICE_PER_LINE;j++)
            {
                pNodeInd = data&mask;
                data>>=PRV_TXQ_PIZZA_SLICE_WIDTH_IN_BITS(devNum);
                currentIndex = PRV_TXQ_SIP6_30_PIZZA_SLICE_PER_LINE*i+j;

                if(pNodeIndSet==pNodeInd)
                {
                    rc = prvCpssSip6_30TxqPdqPizzaSliceDelete(devNum,tileNum,currentIndex);
                    if(rc!=GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqPizzaSliceDelete failed\n");
                    }
                }
            }

        }
    }

    return rc;
}




GT_STATUS prvCpssSip6_30TxqPdqPizzaInit
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum
)
{
    GT_STATUS           rc  = GT_OK;
    int                 ret = 0;
    GT_U32              i,index;

    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);

    rc      = prvCpssSip6_30TxqPdqPortPizzaLastSliceSet(devNum,tileNum,PRV_TXQ_SIP6_30_PIZZA_SLICE_CHUNK_SIZE-1);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_30TxqPdqPortPizzaLastSliceGet failed \n");
    }

    ret     = prvSchedLowLevelPizzaSliceEmptyLineSet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],0);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvSchedLowLevelPizzaSliceEmptyLineSet failed\n");
    }

    /*check if there is SDMA CPU P node*/
    for (i = 0;i < MAX_DP_IN_TILE(devNum); i++)
    {
        if (GT_OK == prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
                (devNum, tileNum, i, CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum) - 1, &index, GT_FALSE))
        {
            if((tileConfigsPtr->mapping.pNodeMappingConfiguration)&&(tileConfigsPtr->mapping.pNodeMappingConfiguration[index]))
            {
                 if(GT_TRUE==tileConfigsPtr->mapping.pNodeMappingConfiguration[index]->isCpuSdma)
                 {
                    rc = prvCpssSip6_30TxqPdqPnodeSpeedSet(devNum,tileNum,index,CPSS_PORT_SPEED_1000_E);/*assure at least one slice to CPU SDMA*/
                 }
            }
        }
    }

    return rc;
}

GT_STATUS prvCpssSip6LKNGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN PRV_CPSS_PDQ_LEVEL_ENT level,
    OUT GT_U32 *Lptr,
    OUT GT_U32 *Kptr,
    OUT GT_U32 *Nptr
)
{
    GT_STATUS           rc  = GT_OK;
    int                 ret = 0;

    CPSS_NULL_PTR_CHECK_MAC(Lptr);
    CPSS_NULL_PTR_CHECK_MAC(Kptr);
    CPSS_NULL_PTR_CHECK_MAC(Nptr);

    ret     = prvSchedLowLevelLKNGet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],
        PRV_CPSS_LEVEL_CONVERT_TO_TM_FORMAT_MAC(level),Lptr,Kptr,Nptr);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

    return rc;
}


GT_STATUS prvCpssSip7PdqAnodeMappingCopy
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 aNodeSource,
    IN GT_U32 aNodeTarget,
    IN GT_U32 nodesToCopy
)
{
    GT_STATUS           rc  = GT_OK;
    int                 ret = 0;
    GT_U32              i;
    GT_U32 anodeParent,anodeChildLo,anodeChildHi,anodeEf;

    for (i=0; i<nodesToCopy; i++)
    {
     ret = prvCpssSchedNodesReadAnodeMappingHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              aNodeSource+i,
                              &anodeParent,&anodeChildLo,&anodeChildHi,&anodeEf);
     if (ret)
     {
         rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);
     }
     rc     = prvCpssSip7PdqAnodeMappingUpdateSet(devNum,pdqNum,aNodeTarget,anodeChildLo,anodeChildHi-anodeChildLo+1);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip7PdqAnodeMappingUpdateSet failed\n");
     }
    }



    return rc;
}


GT_STATUS prvCpssSip7PdqAnodeMappingUpdateSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 aNodeIndex,
    IN GT_U32 qNodeIndex,
    IN GT_U32 qNum
)
{
    GT_STATUS           rc  = GT_OK;
    int                 ret = 0;

    ret     = prvSchedANodeUpdateChildren(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],aNodeIndex,qNodeIndex,
        qNodeIndex+qNum-1);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip7PdqAnodeMappingUpdateSet failed\n");
    }

    ret     = prvSchedQNodeUpdateParent(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],qNodeIndex,
        qNodeIndex+qNum-1,aNodeIndex);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

    return rc;
}


GT_STATUS prvCpssSip7PdqQueueRangeDelete
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 qNodeIndexFirst,
    IN GT_U32 qNodeIndexLast
)
{
    GT_STATUS           rc  = GT_OK;
    int                 ret = 0;
    GT_U32              i;


    for(i=qNodeIndexFirst;i<=qNodeIndexLast;i++)
    {
        ret     = prvSchedNodesCtlFreeQueue(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],i,TM_NODE_FREE,GT_TRUE);
        rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvSchedNodesCtlFreeQueue failed\n");
        }

        ret     = prvCpssSchedUpdateEligFunc(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                     SCHED_Q_LEVEL, i, PRV_SCHED_ELIG_DEQ_DISABLE);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSchedUpdateEligFunc failed\n");
        }

        ret=prvSchedLowLevelSetDefaultQnodeMapping(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],i);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvSchedLowLevelSetDefaultQnodeMapping failed\n");
        }

    }

    return rc;
}


GT_STATUS prvCpssSip7PdqAlevelNodeRangeDelete
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 aNodeIndexFirst,
    IN GT_U32 aNodeIndexLast
)
{
    GT_STATUS           rc  = GT_OK;
    int                 ret = 0;
    GT_U32              i;
    GT_U32 rangeSize = aNodeIndexLast-aNodeIndexFirst+1;

    /*handle SW DB*/
    for(i=aNodeIndexFirst;i<=aNodeIndexLast;i++)
    {
        ret     = prvSchedDbDeleteANodeProc(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],i);
        rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvSchedNodesCtlFreeQueue failed\n");
        }

    }

    ret = prvSchedLowLevelDisableAlevelEligPrioFunc(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],aNodeIndexFirst, rangeSize);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvSchedLowLevelDisableAlevelEligPrioFunc failed\n");
    }

    ret = prvSchedLowLevelSetDefaultAnodeMapping(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],aNodeIndexFirst, rangeSize);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvSchedLowLevelSetDefaultQnodeMapping failed\n");
    }


    return rc;
}


GT_STATUS prvCpssSip7PdqBnodeMappingUpdateSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdqNum,
    IN GT_U32 bNodeIndex,
    IN GT_U32 aNodeIndex,
    IN GT_U32 aNodeNum
)
{
    GT_STATUS           rc  = GT_OK;
    int                 ret = 0;

    ret     = prvSchedBNodeUpdateChildren(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],bNodeIndex,aNodeIndex,
        aNodeIndex+aNodeNum-1);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip7PdqAnodeMappingUpdateSet failed\n");
    }

    ret     = prvSchedANodeUpdateParent(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],aNodeIndex,
        aNodeIndex+aNodeNum-1,bNodeIndex);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

    return rc;
}

GT_STATUS prvCpssSip6PdqRenumber
(
    IN GT_U8 devNum,
    IN GT_U8 newDevNum
)
{
    GT_STATUS           rc  = GT_OK;
    int                 ret = 0;
    GT_U32              i,numberOfTiles;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum, numberOfTiles);

        for(i=0;i<numberOfTiles;i++)
        {
            ret     = prvSchedDevNumSet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[i],newDevNum);
            rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, i);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6PdqRenumber failed\n");
            }
        }
    }

    return rc;
}

GT_STATUS prvCpssSip6TxqPdqShapingParametersCalculate
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_TXQ_SHAPER_RATE_TYPE_ENT type,
    IN CPSS_DXCH_TXQ_SHAPER_LEVEL_ENT     level,
    IN GT_U32                             maxRate,
    OUT CPSS_DXCH_TXQ_SHAPER_OUTPUT_STC   *outputPtr
)
{
    int                 ret;
    GT_STATUS           rc = GT_OK;

    GT_U32              unit;
    GT_U8               tokenResExp,dividerResExp;
    PRV_CPSS_PDQ_LEVEL_ENT     treeLevel;
    PRV_DXCH_TXQ_SIP_6_SHAPER_OUTPUT_STC prvOutParams;
    PRV_DXCH_TXQ_SIP_6_SHAPER_INPUT_STC inParams;

    CPSS_NULL_PTR_CHECK_MAC(outputPtr);

    switch(level)
    {
       case CPSS_DXCH_TXQ_SHAPER_LEVEL_PHYS_PORT_E:
           treeLevel = PRV_CPSS_PDQ_LEVEL_A_E;
           break;
        case CPSS_DXCH_TXQ_SHAPER_LEVEL_CSCD_PORT_E:
           treeLevel = PRV_CPSS_PDQ_LEVEL_B_E;
           break;
        case CPSS_DXCH_TXQ_SHAPER_LEVEL_QUEUE_E:
        default:
           treeLevel = PRV_CPSS_PDQ_LEVEL_Q_E;
           break;

    }

    ret = prvCpssTxqSchedulerGetUnit(devNum,(enum schedLevel)treeLevel,&unit);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssTxqSchedulerGetUnit failed\n");
    }

    ret = prvCpssTxqSchedulerGetOptimalResolutionExponent((enum schedLevel)treeLevel,&tokenResExp);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssTxqSchedulerGetOptimalResolutionExponent failed\n");
    }


    ret = prvCpssTxqSchedulerGetDividerExp(devNum,(enum schedLevel)treeLevel,&dividerResExp);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssTxqSchedulerGetOptimalResolutionExponent failed\n");
    }

    inParams.burst_size_bytes = PRV_CPSS_MAX_TB_SIZE*1024;/*set burst to max(in HW busrst is in Kb)*/
    inParams.bw_in_tokens = 1000.0 * maxRate/unit;
    inParams.token_resolution_multiplyer = 1<<tokenResExp;
    inParams.hndl = PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[0];
    inParams.divider_res_exp = dividerResExp;
    inParams.tbNum = prvCpssSip6RateTypeToTokenBucket(devNum,type);

    ret = prvSchedShapingUtilsCalculateNodeShapingNoBurstSizeAdaption(devNum,
                                                &inParams,
                                                &prvOutParams);
    rc      = FALCON_XEL_TO_CPSS_ERR_CODE(ret, 0);

    if(GT_OK==rc)
    {

        rc = prvCpssSip6TxqPdqCalculateActualBwGet(devNum,prvOutParams.token,(prvOutParams.resolution==0)?0:dividerResExp,
            prvOutParams.divider_exp,treeLevel,&outputPtr->outRate);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPdqCalculateActualBwGet failed\n");
        }

        outputPtr->inAccuracyPercent= (GT_U32)(prvOutParams.out_accuracy>=0?prvOutParams.out_accuracy*100000:prvOutParams.out_accuracy*(-100000));
        outputPtr->inAccuracyFracPercent=outputPtr->inAccuracyPercent%1000;
        outputPtr->inAccuracyPercent/=1000;
        outputPtr->roundDown= prvOutParams.out_accuracy<0?GT_TRUE:GT_FALSE;
        outputPtr->effectiveBurst= prvOutParams.effectiveBurst;
        outputPtr->dividerExp = prvOutParams.divider_exp;
        outputPtr->tokenResolution = (prvOutParams.resolution==0)?1:prvOutParams.token_resolution_multiplyer;
        outputPtr->token = prvOutParams.token;
    }

    return rc;
}


