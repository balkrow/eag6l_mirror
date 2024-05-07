/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatfomRunTimeConfig.c
*
* @brief CPSS Application Platform - run time configuration
*
* @version   1
********************************************************************************
*/

#include <profiles/cpssAppPlatformProfile.h>
#include <ezbringup/cpssAppPlatformEzBringupTools.h>
#include <ezbringup/cpssAppPlatformMpdTool.h>
#include <ezbringup/cpssAppPlatformMpdAndPp.h>
#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformRunTimeConfig.h>
#include <cpssAppPlatformPortInit.h>
#include <cpssAppPlatformPpConfig.h>
#include <cpssAppPlatformPpUtils.h>

#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsTimer.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpssAppUtilsCommon.h>

#if (defined CHX_FAMILY)
    #define DXCH_CODE
#endif /* (defined CHX_FAMILY) */
#if (defined PX_FAMILY)
    #define PX_CODE
#endif /* (defined PX_FAMILY) */

#include <cpssAppPlatformPortInit.h>
#include <cpssAppPlatformSysConfig.h>

#include <cpss/common/port/cpssPortManager.h>

#ifdef DXCH_CODE
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#endif

#include <extUtils/common/cpssEnablerUtils.h>
#include <cpssAppUtilsEvents.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>





extern GT_STATUS cpssAppPlatformPpAc5xPortInitListGet
(
   IN  GT_U8                              devNum,
   OUT CPSS_APP_PLATFORM_PORT_CONFIG_STC **portInitListPtr
);

/* default HWS squelch for some of the 10G port modes on Aldrin2 DB boards can be problematic,
   so there is a need to set a custom value */


#define FREE_TASK_CNS   0xFFFFFFFF

static const GT_CHAR  * const uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};



static GT_STATUS xcat3PortPhy1680Init
(
     IN GT_U8 devNum
)
{
    GT_STATUS   rc;             /* return code */
    GT_U8       portNum;            /* port number */

    /* port 0-23*/
    for(portNum = 0; portNum < 24; portNum++)
    {
        /* matrix mode */
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 4);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 27, 0x3FA0);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);



        /* MACSec and PTP disable */
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0x12);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 27, 0);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);



        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0xFD);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 8, 0xB53);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 7, 0x200D);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0xFF);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 17, 0xB030);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 16, 0x215C);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);


        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 3);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 16, 0x1117);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 16, 0x3360);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 0, 0x9140);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);

    }
    return GT_OK;
}


GT_CHAR * CPSS_FEC_2_STR
(
    CPSS_PORT_FEC_MODE_ENT fecMode
)
{
    typedef struct
    {
        CPSS_PORT_FEC_MODE_ENT fecModeEnm;
        GT_CHAR            *fecStr;
    }APPDEMO_FEC_TO_STR_STC;

    static APPDEMO_FEC_TO_STR_STC const prv_fec2str[]  =
    {
         {  CPSS_PORT_FEC_MODE_ENABLED_E,              "FC    "}
        ,{  CPSS_PORT_FEC_MODE_DISABLED_E,             "NONE  "}
        ,{  CPSS_PORT_RS_FEC_MODE_ENABLED_E,           "RS    "}
        ,{  CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E,   "RS544 "}
    };

    GT_U8 i;
    for (i = 0 ; prv_fec2str[i].fecModeEnm != CPSS_PORT_FEC_MODE_LAST_E; i++)
    {
        if (prv_fec2str[i].fecModeEnm == fecMode)
        {
            return prv_fec2str[i].fecStr;
        }
    }
    return "-----";
}

/**
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
* @internal cpssAppPlatfromPpEvTreat function
* @endinternal
*
* @brief   This routine handles PP events.
*
* @param[in] callbackFuncPtr - event callback function given by runTime profile.
* @param[in] devNum          - the device number.
* @param[in] uniEv           - Unified event number
* @param[in] evExtData       - Unified event additional information
*
* @retval GT_OK          - on success,
* @retval GT_FAIL        - otherwise.
*/
static GT_STATUS cpssAppPlatfromPpEvTreat
(
    CPSS_APP_PLATFORM_USER_EVENT_CB_FUNC callbackFuncPtr,
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
)
{
    GT_STATUS            rc = GT_OK;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PORT_SPEED_ENT apSpeed;
    GT_BOOL     hcdFound;
    GT_U32    physicalPortNum;
    CPSS_DXCH_PORT_AP_STATUS_STC apStatusDx;
    CPSS_PORT_INTERFACE_MODE_ENT apIfMode;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /* device was removed */
        return GT_OK;
    }
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    switch (uniEv)
    {
        case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
            portNum = (GT_U32)evExtData;
            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                rc = cpssAppPlatformPmPortLinkStatusChangeSignal(devNum, portNum);
                if (rc != GT_OK)
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortLinkStatusChangeSignal rc=%d\n", rc);
            }
            break;

        case CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E:
             if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
             {
                 /* this event is not handled for SIP6 devices */
                 break;
             }
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_PORT_SYNC_STATUS_CHANGED_E:
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E:
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E:
             portNum = (GT_U32)evExtData;
             if (cpssAppPlatformIsPortMgrPort(devNum))
             {
                 rc = cpssAppPlatformPmPortStatusChangeSignal(devNum, portNum, uniEv);
                 if (rc != GT_OK)
                     CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortStatusChangeSignal rc=%d\n", rc);
             }
             break;
        case CPSS_SRVCPU_PORT_802_3_AP_E:
             portNum = (GT_U32)evExtData;

             rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,
                     portNum, &physicalPortNum);
             CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortMapReverseMappingGet);

             if (cpssAppPlatformIsPortMgrPort(devNum))
             {
                 cpssAppPlatformPmPortStatusChangeSignal(devNum, physicalPortNum, CPSS_SRVCPU_PORT_802_3_AP_E);
                 /* pizza allocation done inside port manager, so no need to continue */
                 break;
             }

             /* query resolution results */
             rc = cpssDxChPortApPortStatusGet(devNum, physicalPortNum, &apStatusDx);
             CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortApPortStatusGet);

             hcdFound = apStatusDx.hcdFound;
             apSpeed = apStatusDx.portMode.speed;
             apIfMode = apStatusDx.portMode.ifMode;

             /* resolution found - allocate pizza resources*/
             if(hcdFound)
             {
                 CPSS_PORT_SPEED_ENT speed;
                 CPSS_PORTS_BMP_STC portsBmp;
                 CPSS_PORT_INTERFACE_MODE_ENT ifMode;
                 PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                 CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,physicalPortNum);

                 /* Check current spped */
                 rc = cpssDxChPortSpeedGet(devNum, physicalPortNum, &speed);
                 CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSpeedGet);

                 /*if speed <= CPSS_PORT_SPEED_1000_E we get the value from HW; at this stage speed is not always update correctly;
                   assume pizza allocation cannot be less than 1G*/
                 if(speed <= CPSS_PORT_SPEED_1000_E)
                 {
                     speed = CPSS_PORT_SPEED_1000_E;
                 }

                 /*if pizza already configured, Release it if not the same speed */
                 if((speed != CPSS_PORT_SPEED_NA_HCD_E) && ((speed != apStatusDx.portMode.speed) && (CPSS_PORT_SPEED_20000_E != speed)))
                 {
                     /* Release pizza resources */
                     rc = cpssDxChPortInterfaceModeGet(devNum, physicalPortNum, &ifMode);
                     if(rc != GT_OK)
                     {
                         CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortInterfaceModeGet:rc=%d,portNum=%d\n",rc, physicalPortNum);
                         return rc;
                     }

                     rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_FALSE, ifMode + CPSS_PORT_INTERFACE_MODE_NA_E, speed + CPSS_PORT_SPEED_NA_E);
                     if(rc != GT_OK)
                     {
                         CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n", rc, physicalPortNum);
                         return rc;
                     }
                 }
                 /* allocate pizza resources only if not configured earlier or speed changed */
                 if(speed == CPSS_PORT_SPEED_NA_HCD_E)
                 {
                     rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, apIfMode + CPSS_PORT_INTERFACE_MODE_NA_E, apSpeed + CPSS_PORT_SPEED_NA_E);
                     if(rc != GT_OK)
                     {
                         CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n", rc, physicalPortNum);
                         return rc;
                     }
                 }
             }
             else
             {
                 CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_802_3_AP_E - portNum=%d, no resolution\n",physicalPortNum);
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
             }

             break;
        case CPSS_SRVCPU_PORT_AP_DISABLE_E:
            portNum = (GT_U8)evExtData;
            if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
            {
                rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,portNum, &physicalPortNum);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_AP_DISABLE_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                    return rc;
                }
            }
            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                rc = cpssAppPlatformPmPortStatusChangeSignal(devNum, physicalPortNum, uniEv);
                if (rc != GT_OK)
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortStatusChangeSignal rc=%d\n", rc);
            }
            break;
         case CPSS_PP_PORT_LANE_SQ_EVENT_E:
            rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_SERDES_E,evExtData, &physicalPortNum);
            if(rc != GT_OK && rc != GT_NO_SUCH)
            {
                CPSS_APP_PLATFORM_LOG_INFO_MAC("CPSS_PP_PORT_LANE_SQ_EVENT_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, evExtData);
                return rc;
            }
            else if (rc == GT_NO_SUCH)
            {
                /* GT_NO_SUCH is for AP ports, in that case we don't need to handle them for */
                return GT_OK;
            }

            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                rc = cpssAppPlatformPmPortStatusChangeSignal(devNum, physicalPortNum, uniEv);
                if (rc != GT_OK)
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortStatusChangeSignal rc=%d\n", rc);
                break;
            }
           break;

        default:
            break;
    }

    if(callbackFuncPtr != NULL)
    {
        rc = callbackFuncPtr(devNum, uniEv, evExtData);
    }

    return rc;
}

/*
* @internal cpssAppPlatformEventHandler function
* @endinternal
*
* @brief   This function is the event handler for CPSS Event-Request-Driven mode
*          (polling mode).
*
* @param[in] param               - pointer to event handler parameters.
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - otherwise.
*/
static unsigned __TASKCONV cpssAppPlatformEventHandler
(
    GT_VOID_PTR param
)
{
    GT_STATUS           rc;                                         /* return code         */
    GT_U32              i;                                          /* iterator            */
    GT_UINTPTR          evHndl;                                     /* event handler       */
    GT_U32              evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS];   /* event bitmap array  */
    GT_U32              evBitmap;                                   /* event bitmap 32 bit */
    GT_U32              evExtData;                                  /* event extended data */
    GT_U8               devNum;                                     /* device number       */
    GT_U32              uniEv;                                      /* unified event cause */
    GT_U32              appDemoDefaultTO;                           /* appDemo default T.O */
    GT_U32              evCauseIdx;                                 /* event index         */
    EVENT_HANDLER_PARAM_STC      *hndlrParamPtr;                             /* bind event array    */
    GT_U32                  self_tid = 0;

    if (osTaskGetSelf(&self_tid) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    hndlrParamPtr = (EVENT_HANDLER_PARAM_STC*)param;
    evHndl        = hndlrParamPtr->evHndl;
    appDemoDefaultTO = MV_DEFAULT_TIMEOUT;

    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssAppPlatformEventHandler[%d]: created \n", hndlrParamPtr->index);

    /* indicate that task start running */
    PRV_APP_REF_RUN_TIME_CONFIG_VAR(taskCreated) = 1;
    cpssOsTimerWkAfter(1);

    while (1)
    {
        rc = cpssEventSelect(evHndl, &appDemoDefaultTO, evBitmapArr, (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);

        if((PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerReset) || osTaskGracefulGetTerminationReq(self_tid)))
        {
            if (osTaskGracefulGetTerminationReq(self_tid))
                osTaskGracefulSetTerminationAck(self_tid);
            break;
        }
        if (rc == GT_TIMEOUT)
            continue;

        if(rc != GT_OK)
        {
            if (rc != GT_EMPTY)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssEventSelect rc=%d\n", rc);
            }

            continue;
        }
        for (evCauseIdx = 0; evCauseIdx < CPSS_UNI_EV_BITMAP_SIZE_CNS; evCauseIdx++)
        {
            if (evBitmapArr[evCauseIdx] == 0)
            {
                continue;
            }

            evBitmap = evBitmapArr[evCauseIdx];

            for (i = 0; evBitmap; evBitmap >>= 1, i++)
            {
                if ((evBitmap & 1) == 0)
                {
                    continue;
                }

                uniEv = (evCauseIdx << 5) + i;

                if (cpssEventRecv(evHndl, uniEv, &evExtData, &devNum) == GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssEventRecv: %d <dev %d, %s, extData %d>\n",
                             hndlrParamPtr->index, devNum, uniEvName[uniEv], evExtData);

                    if (uniEv <= CPSS_SRVCPU_MAX_E)
                    {
                        rc = cpssAppPlatfromPpEvTreat(hndlrParamPtr->callbackFuncPtr, devNum, uniEv, evExtData);
                        if (rc != GT_OK)
                        {
                            /* Frequent printing changes running timing which caused failure of FDB statistics test
                               CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatfromPpEvTreat rc=%d portNum=%d\n", rc, evExtData);
                            */
                        }
                    }

                    if(NULL != cpssAppUtilsGenEventCounterIncrementCb)
                    {
                     rc= cpssAppUtilsGenEventCounterIncrementCb(devNum, uniEv, evExtData);
                     if(rc != GT_OK)
                     {
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("prvAppPlatformGenEventCounterIncrement rc=%d\n", rc);
                     }
                    }

                }
            }
        }
    }

    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssAppPlatformEventHandler[%d]: thread exits\n",hndlrParamPtr->index);

    hndlrParamPtr->index = FREE_TASK_CNS;

    return 0;
}

/*
* @internal prvEventMaskSet function
* @endinternal
*
* @brief   Mask/Unmask user events for given device.
*
* @param [in] devNum       - CPSS Device Number.
* @param [in] operation    - Mask / Unmask option.
*
* @retval GT_OK            - on success.
* @retval GT_FAIL          - otherwise.
*/
GT_STATUS prvEventMaskSet
(
    IN GT_U8 devNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i  = 0;
    GT_U32    j  = 0;

    for (i = 0; i < PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum); i++)
    {
        for (j = 0; j < PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].numOfEvents; j++)
        {
            if (PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].eventListPtr[j] > CPSS_UNI_EVENT_COUNT_E) /* last event */
            {
                continue;
            }

            rc = cpssEventDeviceMaskSet(devNum, PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].eventListPtr[j], operation);
            switch(rc)
            {
                case GT_NOT_INITIALIZED:
                case GT_BAD_PARAM:
                case GT_NOT_FOUND:
                    /* this event not relevant to this device */
                    rc = GT_OK;
                    break;

                case GT_OK:
                    break;

                default:
                    /* other real error */
                    break;
            }

            if (rc != GT_OK)
            {
                /* Mask/unMask fails. Continue to other events in the list */
                CPSS_APP_PLATFORM_LOG_ERR_MAC("%s failed for Event: %s\n",
                             (operation == CPSS_EVENT_MASK_E) ? "Masking":"UnMasking",
                             uniEvName[PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].eventListPtr[j]]);
                continue;
            }
        }
    }

    return rc;
}

/*
* @internal cpssAppPlatformUserEventHandlerInit function
* @endinternal
*
* @brief   create user event handlers.
*
* @param [in] eventHandlePtr   - Event Handle Ptr for RX,AUQ,Link change events.
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformUserEventHandlerInit
(
    IN CPSS_APP_PLATFORM_EVENT_HANDLE_STC *eventHandlePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_CHAR   taskName[30];

    if(PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum) >= CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Number of User Event handlers exceeds supported limit: %d\n", CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if((eventHandlePtr->numOfEvents == 0) ||
       (eventHandlePtr->eventListPtr[0] >= CPSS_UNI_EVENT_COUNT_E)) /* last event */
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("eventListPtr is Empty");
        return GT_OK;
    }

    /* call CPSS to bind the events under single handler */
    rc = cpssEventBind(eventHandlePtr->eventListPtr,
                       eventHandlePtr->numOfEvents,
                       &PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)].evHndl);
    if (rc == GT_ALREADY_EXIST)
        CPSS_APP_PLATFORM_LOG_ERR_MAC("One of the events is already bound to another handler\n");
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventBind);

    PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)].index = PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum);
    PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)].callbackFuncPtr = eventHandlePtr->callbackFuncPtr;
    PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)].eventListPtr = eventHandlePtr->eventListPtr;
    PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)].numOfEvents = eventHandlePtr->numOfEvents;

    /* spawn all the event handler processes */
    cpssOsSprintf(taskName, "evHndl_%d", PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum));

    PRV_APP_REF_RUN_TIME_CONFIG_VAR(taskCreated) = 0;
    rc = cpssOsTaskCreate(taskName,
                      eventHandlePtr->taskPriority,
                      _32KB,
                      cpssAppPlatformEventHandler,
                      &PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)],
                      &PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)].taskId);

    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("failed spawning evHndl_%d\n", PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum));
    }

    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTaskCreate);
    osTaskGracefulCallerRegister(PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)].taskId, 1, NULL, NULL);
    while (PRV_APP_REF_RUN_TIME_CONFIG_VAR(taskCreated) == 0)
    {
        /* wait for indication that task created */
        cpssOsTimerWkAfter(1);
    }

    PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)++;
    return rc;
}

/**
* @internal cpssAppPlatformUserEventHandlerReset function
* @endinternal
*
* @brief   This routine deletes event handlers when last PP
*          device is removed. It also deletes event counters.
*
* @param[in] devNum           - device number
*
* @retval GT_OK               - on success,
* @retval GT_FAIL             - otherwise.
*/
GT_STATUS cpssAppPlatformUserEventHandlerReset
(
    IN CPSS_APP_PLATFORM_EVENT_HANDLE_STC *eventHandlePtr
)
{
    GT_STATUS rc           = GT_OK;
    GT_U32    hdlrNum      = 0;
    GT_U32    evHdlrNum    = 0;
    GT_U32    eventNum     = 0;
    GT_CHAR   taskName[30];

    if(PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum) == 0)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("No User Event handlers available\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(eventHandlePtr->numOfEvents == 0)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("eventListPtr is Empty");
        return GT_OK;
    }

    for (eventNum = 0; eventNum < eventHandlePtr->numOfEvents; eventNum++)
    {
        if (eventHandlePtr->eventListPtr[eventNum] > CPSS_UNI_EVENT_COUNT_E)
        {
            continue;
        }

        /* mask events of the devices */
        rc = cpssEventDeviceMaskSet(0, eventHandlePtr->eventListPtr[eventNum], CPSS_EVENT_MASK_E);
        switch(rc)
        {
            case GT_NOT_INITIALIZED:
            case GT_BAD_PARAM:
            case GT_NOT_FOUND:
                /* this event not relevant to this device */
                rc = GT_OK;
                break;

            case GT_OK:
                break;

            default:
                /* other real error */
                break;
        }

        if (rc != GT_OK)
        {
            /* Mask/unMask fails. Continue to other events in the list */
            CPSS_APP_PLATFORM_LOG_ERR_MAC("%s failed for Event: %s\n", "UnMasking",
                    uniEvName[eventHandlePtr->eventListPtr[eventNum]]);
            continue;
        }
    }

    /* changing the mask interrupts registers may invoke 'last minute' interrupts */
    /* let the task handle it before we continue */
    cpssOsTimerWkAfter(100);

    evHdlrNum = PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum);
    for(hdlrNum = 0; hdlrNum < evHdlrNum; hdlrNum++)
    {
        if(PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[hdlrNum].callbackFuncPtr == NULL)
        {
            evHdlrNum++;
            if(evHdlrNum == CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS)
            {
                rc = GT_BAD_STATE;
                CPSS_APP_PLATFORM_LOG_ERR_MAC("%s failed for Event: %s\n", "Task deletion and unbinding",
                        uniEvName[eventHandlePtr->eventListPtr[hdlrNum]]);
                /* end of loop */
                break;
            }
            continue;
        }
        if(PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[hdlrNum].callbackFuncPtr == eventHandlePtr->callbackFuncPtr)
        {
            /* spawn all the event handler processes */
            cpssOsSprintf(taskName, "evHndl_%d", hdlrNum);

            rc = cpssOsTaskDelete(PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[hdlrNum].taskId);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTaskDestroy);

            /* call CPSS to destroy the events under single handler */
            rc = cpssEventUnBind(eventHandlePtr->eventListPtr, eventHandlePtr->numOfEvents);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventUnBind);

            /* ("One of the events is not bound to any handler\n");*/
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[hdlrNum].index = 0;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[hdlrNum].evHndl= 0;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[hdlrNum].taskId= 0;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[hdlrNum].callbackFuncPtr = NULL;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[hdlrNum].eventListPtr = NULL;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[hdlrNum].numOfEvents = 0;

            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum)--;
            break;
        }
    }
    return rc;
}

/**
* @internal cpssAppPlatformEventHandlerReset function
* @endinternal
*
* @brief   This routine deletes event handlers when last PP
*          device is removed. It also deletes event counters.
*
* @param[in] devNum           - device number
*
* @retval GT_OK               - on success,
* @retval GT_FAIL             - otherwise.
*/
GT_STATUS cpssAppPlatformEventHandlerReset
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc     = GT_OK;
    GT_U32    i      = 0;
    GT_U32    evNum  = 0;
    GT_BOOL   isLast = GT_FALSE;

    /* mask events of the devices */
    rc = prvEventMaskSet(devNum, CPSS_EVENT_MASK_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvEventMaskSet);

    /* delete event handlers when last device is removed */
    PRV_IS_LAST_DEVICE_MAC(devNum, isLast);
    if(isLast == GT_TRUE)
    {
        PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerReset) = 1;

        /* changing the mask interrupts registers may invoke 'last minute' interrupts */
        /* let the task handle it before we continue */
        cpssOsTimerWkAfter(100);

        CPSS_APP_PLATFORM_LOG_INFO_MAC("cpssAppPlatformEventHandlerReset: destroy task handlers \n");

        evNum = PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum);
        for (i = 0; i < evNum; i++)
        {
            /* call CPSS to destroy the events */
            if(PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].evHndl == 0)
            {
                evNum++;
                if(evNum == CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS)
                {
                    rc = GT_BAD_STATE;
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("%s failed \n", "Event Destroy");
                    /* end of loop */
                    break;
                }
                continue;
            }
            rc = cpssEventDestroy(PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].evHndl);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventDestroy);

            while(PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].index != FREE_TASK_CNS)
            {
                /* this is indication that the thread is exited properly ... no need to kill it */
                cpssOsTimerWkAfter(1);
                CPSS_APP_PLATFORM_LOG_INFO_MAC("*");
            }

            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].index = 0;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].evHndl= 0;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].taskId= 0;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].callbackFuncPtr = NULL;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].eventListPtr = NULL;
            PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerParams)[i].numOfEvents = 0;
        }

        /* state that reset of appDemo events finished */
        PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerNum) = 0;
        PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerReset) = 0;
        PRV_APP_REF_RUN_TIME_CONFIG_VAR(eventHandlerInitDone) = GT_FALSE;
    }

    if(NULL!= cpssAppUtilsUniEventsFreeCb)
    {
        cpssAppUtilsUniEventsFreeCb(devNum);
    }

    return GT_OK;
}

/*
* @internal prvPerProfileEventMaskSet function
* @endinternal
*
* @brief   Mask/Unmask user events for given device.
*
* @param [in] devNum            - CPSS Device Number.
* @param [in] eventHandlePtr    - Event Handler Ptr
* @param [in] operation         - Mask / Unmask option.
*
* @retval GT_OK            - on success.
* @retval GT_FAIL          - otherwise.
*/
GT_STATUS prvPerProfileEventMaskSet
(
    IN GT_U8                                dev,
    IN CPSS_APP_PLATFORM_EVENT_HANDLE_STC   *eventHandlePtr,
    IN CPSS_EVENT_MASK_SET_ENT              operation
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          arrLength = 0;

    /* unmask the interrupt */
    for (arrLength = 0; arrLength < eventHandlePtr->numOfEvents; arrLength++)
    {
        rc = cpssEventDeviceMaskSet(dev, eventHandlePtr->eventListPtr[arrLength],
                                operation);

        switch(rc)
        {
            case GT_NOT_INITIALIZED:
            case GT_BAD_PARAM:
            case GT_NOT_FOUND:
                /* this event not relevant to this device */
                rc = GT_OK;
                break;

            case GT_OK:
                break;

            default:
                /* other real error */
                break;
        }

        if (GT_OK != rc)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("prvPerProfileEventMaskSet rc=%d\n", rc);
            return rc;
        }
    }

    return GT_OK;
}

/*
* @internal cpssAppPlatformUserEventsInit function
* @endinternal
*
* @brief   Initialize user event handlers.
*
* @param[in] inputProfileListPtr - Input profile list containing run time profile.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not run time profile.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformUserEventsInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC  *inputProfileListPtr
)
{
    GT_STATUS rc  = GT_OK;
    GT_U8     dev = 0;
    CPSS_APP_PLATFORM_PROFILE_STC *profileListPtr = inputProfileListPtr;
    CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC *runTimeProfilePtr = NULL;
    CPSS_APP_PLATFORM_EVENT_HANDLE_STC    *eventHandlePtr = NULL;

    if(profileListPtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Profile list is NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Iterate through all run-time profiles and Create Event Handlers */
    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)
    {
        if(profileListPtr->profileValue.runTimeInfoPtr == NULL)
        {
            /* RunTime profile is Empty. proceed to next profile */
            continue;
        }

        runTimeProfilePtr = profileListPtr->profileValue.runTimeInfoPtr;
        PRV_CPSS_APP_START_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E)
        {
            eventHandlePtr = runTimeProfilePtr->runtimeInfoValue.eventHandlePtr;
            if(eventHandlePtr == NULL)
            {
                /* event handle is NULL. proceed to next profile */
                continue;
            }
            rc = cpssAppPlatformUserEventHandlerInit(eventHandlePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformUserEventHandlerInit);

            /* TODO : Need to check to set on all devices or not */
            /* unmask user events for all devices */
            for (dev = 0; dev < CPSS_APP_PLATFORM_MAX_PP_CNS; dev++)
            {
                if(!PRV_CPSS_IS_DEV_EXISTS_MAC(dev))
                    continue;

                rc = prvPerProfileEventMaskSet(dev, eventHandlePtr, CPSS_EVENT_UNMASK_E);

                if (GT_OK != rc)
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("prvEventMaskSet rc=%d\n", rc);
            }
            CPSS_APP_PLATFORM_LOG_INFO_MAC("Event Handler Init Done ...\n");

            if (GT_OK != rc)
               CPSS_APP_PLATFORM_LOG_ERR_MAC("prvEventMaskSet rc=%d\n", rc);
        }
        PRV_CPSS_APP_END_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E)
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)

    return GT_OK;
}

/*
* @internal cpssAppPlatformUserEventsDeinit function
* @endinternal
*
* @brief   Initialize user event handlers.
*
* @param[in] inputProfileListPtr - Input profile list containing run time profile.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not run time profile.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformUserEventsDeinit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC  *inputProfileListPtr
)
{
    GT_STATUS rc  = GT_OK;
    GT_U8     dev = 0;
    CPSS_APP_PLATFORM_PROFILE_STC *profileListPtr = inputProfileListPtr;
    CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC *runTimeProfilePtr = NULL;
    CPSS_APP_PLATFORM_EVENT_HANDLE_STC    *eventHandlePtr = NULL;

    if(profileListPtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Profile list is NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Iterate through all run-time profiles and Create Event Handlers */
    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)
    {
        if(profileListPtr->profileValue.runTimeInfoPtr == NULL)
        {
            /* RunTime profile is Empty. proceed to next profile */
            continue;
        }

        runTimeProfilePtr = profileListPtr->profileValue.runTimeInfoPtr;
        PRV_CPSS_APP_START_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E)
        {
            eventHandlePtr = runTimeProfilePtr->runtimeInfoValue.eventHandlePtr;
            if(eventHandlePtr == NULL)
            {
                /* event handle is NULL. proceed to next profile */
                continue;
            }
            rc = cpssAppPlatformUserEventHandlerReset(eventHandlePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformUserEventHandlerInit);

            /* TODO : Need to check to set on all devices or not */
            /* unmask user events for all devices */
            for (dev = 0; dev < CPSS_APP_PLATFORM_MAX_PP_CNS; dev++)
            {
                if(!PRV_CPSS_IS_DEV_EXISTS_MAC(dev))
                    continue;

                rc = prvPerProfileEventMaskSet(dev, eventHandlePtr, CPSS_EVENT_UNMASK_E);

                if (GT_OK != rc)
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("prvEventMaskSet rc=%d\n", rc);
            }
            CPSS_APP_PLATFORM_LOG_INFO_MAC("Event Handler Init Done ...\n");
         }

        PRV_CPSS_APP_END_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E)
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)

    return GT_OK;
}

/*
* @internal cpssAppPlatformPortManagerInit function
* @endinternal
*
* @brief   initialize port manager.
*
* @param[in] inputProfileListPtr - Input profile list containing run time profile.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not run time profile.
* @retval GT_FAIL                - otherwise.
*
*/
GT_STATUS cpssAppPlatformPortManagerInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC *inputProfileListPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_APP_PLATFORM_PROFILE_STC     *profileListPtr = inputProfileListPtr;
    CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC     *runTimeProfilePtr = NULL;
    CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC *portManagerHandlePtr = NULL;

    if(profileListPtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Profile list is NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)
    {
        if(profileListPtr->profileValue.runTimeInfoPtr == NULL)
        {
            /* RunTime profile is Empty. proceed to next profile */
            continue;
        }

        runTimeProfilePtr = profileListPtr->profileValue.runTimeInfoPtr;
        PRV_CPSS_APP_START_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_PORT_MANAGER_HANDLE_E)
        {
            portManagerHandlePtr = runTimeProfilePtr->runtimeInfoValue.portManagerHandlePtr;
            if(portManagerHandlePtr == NULL)
            {
                /* port manager handle is NULL. proceed to next profile */
                continue;
            }

            /* create Port Manager task to initialize ports based on user events */
            rc = cpssAppPlatformPmTaskCreate(portManagerHandlePtr->taskPriority);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPmTaskCreate);

            PRV_APP_REF_PORT_MGR_MODE = GT_TRUE;
            appPlatformDbEntryAdd("portMgr",1);
        }
        PRV_CPSS_APP_END_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_PORT_MANAGER_HANDLE_E)
        if(PRV_APP_REF_PORT_MGR_MODE == GT_TRUE)
        {
            CPSS_APP_PLATFORM_LOG_INFO_MAC("Port manager Init Done ...\n");
            break;
        }
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)

    return rc;
}

#define SIP6_ROW_DELEMITER \
                     "\n|----+------+-------+--------------+------------+----+-----+-----+-----+----|"

#define SIP6_15_ROW_DELEMITER \
                     "\n|----+------+-------+--------------+------------+----+-----+----|"




static GT_STATUS prvCpssAppPlatformTrafficEnable
(
    IN CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *trafficEnablePtr,
    IN PortInitInternal_STC        portList[CPSS_MAX_PORTS_NUM_CNS],
    IN GT_U32                      listSize
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i;
    GT_U32    portIdx;
    GT_U32    maxPortIdx = 0;
    GT_U32    pipeId, tileId, dpIndex;
    GT_U8     devNum;
    GT_U32    portNum;
    GT_U32    mcFifo_0_1 = 0,
              mcFifo_2_3 = 0,
              mcFifo_0_1_2_3 = 0;
#ifndef INCLUDE_MPD
    GT_U32    smi0      = 0,
              smi1      = 0,
              xsmi0     = 0,
              xsmi1     = 0;
    GT_U32    ii = 0;
#endif

    CPSS_APP_PLATFORM_PORT_CONFIG_STC *portTypeList = NULL;
    CPSS_APP_PLATFORM_PORT_CONFIG_STC *portInitPtr  = NULL;
    CPSS_PORTS_BMP_STC                 portsBmp;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    portMap;
    CPSS_SYSTEM_RECOVERY_INFO_STC      system_recovery; /* holds system recovery information */
    GT_U32  doJustPrintPortMapping = 0;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssSystemRecoveryStateGet);

    devNum = trafficEnablePtr->devNum;

    if(trafficEnablePtr->portTypeListPtr == NULL)
    {
        rc = cpssAppPlatformPpAc5xPortInitListGet(devNum, &trafficEnablePtr->portTypeListPtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpAc5xPortInitListGet);
    }
    portTypeList = trafficEnablePtr->portTypeListPtr;

    for (portIdx = 0 ; portIdx < listSize; portIdx++)
    {
        portList[portIdx].portNum       = APP_INV_PORT_CNS;
        portList[portIdx].speed         = CPSS_PORT_SPEED_NA_E;
        portList[portIdx].interfaceMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        portList[portIdx].fecMode       = CPSS_PORT_FEC_MODE_DISABLED_E;
    }

    maxPortIdx = 0;
    portInitPtr = portTypeList;
    for (i = 0 ; portInitPtr->entryType != CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E; i++, portInitPtr++)
    {
        if (maxPortIdx >= CPSS_MAX_PORTS_NUM_CNS)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("port list has more ports than allowed - %d\n", maxPortIdx);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        switch (portInitPtr->entryType)
        {
            case CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E:
                for (portNum = portInitPtr->portList[0] ; portNum <= portInitPtr->portList[1]; portNum += portInitPtr->portList[2])
                {
                    CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->ifMode;
                    portList[maxPortIdx].fecMode       = portInitPtr->fecMode;
                    portList[maxPortIdx].portEnabledByDefault = (CPSS_APP_PLATFORM_L1_PORT_DEFAULT_TYPE_ENT) portInitPtr->portEnabledByDefault;
                    portList[maxPortIdx].laneParams.validLaneParamsBitMask = CPSS_PM_LANE_PARAM_TX_E;
                    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
                    {
                        portList[maxPortIdx].laneParams.txParams.type = CPSS_PORT_SERDES_COMPHY_C28G_E;
                        portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.pre  = portInitPtr->txParams.pre;
                        portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.peak = portInitPtr->txParams.peak;
                        portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.post = portInitPtr->txParams.post;
                    }
                    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
                    {
                        if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType == PRV_CPSS_PORT_MTI_CPU_E)
                        {
                            portList[maxPortIdx].laneParams.txParams.type = CPSS_PORT_SERDES_COMPHY_C28G_E;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.pre  = portInitPtr->txParams.pre;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.peak = portInitPtr->txParams.peak;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.post = portInitPtr->txParams.post;
                        }
                        else
                        {
                            portList[maxPortIdx].laneParams.txParams.type = CPSS_PORT_SERDES_COMPHY_C112G_E;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C112G.pre  = portInitPtr->txParams.pre;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C112G.pre2 = portInitPtr->txParams.pre2;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C112G.main = portInitPtr->txParams.atten;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C112G.post = portInitPtr->txParams.post;
                        }
                    }
                    else
                    {
                        portList[maxPortIdx].laneParams.txParams.type = CPSS_PORT_SERDES_AVAGO_E;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.atten = portInitPtr->txParams.atten;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.post = portInitPtr->txParams.post;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.pre = portInitPtr->txParams.pre;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.pre2 = portInitPtr->txParams.pre2;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.pre3 = portInitPtr->txParams.pre3;
                    }

                    maxPortIdx++;
                }
            break;
            case CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E:
                for (portIdx = 0 ; portInitPtr->portList[portIdx] != APP_INV_PORT_CNS; portIdx++)
                {
                    portNum = portInitPtr->portList[portIdx];
                    CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->ifMode;
                    portList[maxPortIdx].fecMode       = portInitPtr->fecMode;
                    portList[maxPortIdx].portEnabledByDefault = (CPSS_APP_PLATFORM_L1_PORT_DEFAULT_TYPE_ENT) portInitPtr->portEnabledByDefault;
                    portList[maxPortIdx].laneParams.validLaneParamsBitMask = CPSS_PM_LANE_PARAM_TX_E;
                    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
                    {
                        portList[maxPortIdx].laneParams.txParams.type = CPSS_PORT_SERDES_COMPHY_C28G_E;
                        portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.pre  = portInitPtr->txParams.pre;
                        portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.peak = portInitPtr->txParams.peak;
                        portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.post = portInitPtr->txParams.post;
                    }
                    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
                    {
                        if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType == PRV_CPSS_PORT_MTI_CPU_E)
                        {
                            portList[maxPortIdx].laneParams.txParams.type = CPSS_PORT_SERDES_COMPHY_C28G_E;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.pre  = portInitPtr->txParams.pre;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.peak = portInitPtr->txParams.peak;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C28G.post = portInitPtr->txParams.post;
                        }
                        else
                        {
                            portList[maxPortIdx].laneParams.txParams.type = CPSS_PORT_SERDES_COMPHY_C112G_E;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C112G.pre  = portInitPtr->txParams.pre;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C112G.pre2 = portInitPtr->txParams.pre2;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C112G.main = portInitPtr->txParams.atten;
                            portList[maxPortIdx].laneParams.txParams.txTune.comphy_C112G.post = portInitPtr->txParams.post;
                        }
                    }
                    else
                    {
                        portList[maxPortIdx].laneParams.txParams.type = CPSS_PORT_SERDES_AVAGO_E;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.atten = portInitPtr->txParams.atten;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.post = portInitPtr->txParams.post;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.pre = portInitPtr->txParams.pre;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.pre2 = portInitPtr->txParams.pre2;
                        portList[maxPortIdx].laneParams.txParams.txTune.avago.pre3 = portInitPtr->txParams.pre3;
                    }
                    maxPortIdx++;
                }
            break;
            default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
        }
    }

    if(PRV_APP_REF_PORT_MGR_MODE == GT_TRUE)
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("Configuring ports in Port Manager mode\n");
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("Configuring ports in Legacy mode\n");
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC (SIP6_ROW_DELEMITER);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| #  | Port | Speed | Fec  | MAC IF MODE  | map  Type  | mac| txq | pipe| tile| dp |");
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_ROW_DELEMITER);
        }
        else
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC (SIP6_15_ROW_DELEMITER);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| #  | Port | Speed | MAC IF MODE  | map  Type  | mac| txq | dp |");
            CPSS_APP_PLATFORM_LOG_PRINT_MAC (SIP6_15_ROW_DELEMITER);
        }
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+----+------+-------+--------------+-----------------+------------------------------+");
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| #  | Port | Speed |    IF        |   mapping Type  | rxdma txdma mac txq ilkn  tm |");
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+----+------+-------+--------------+-----------------+------------------------------+");
    }

    for (portIdx = 0 ; portIdx < maxPortIdx; portIdx++)
    {
        portNum = portList[portIdx].portNum;

        if ( portList[portIdx].speed == CPSS_PORT_SPEED_NA_E || portList[portIdx].interfaceMode == CPSS_PORT_INTERFACE_MODE_NA_E )
        {
            doJustPrintPortMapping = 1; /* do print mapping only in case mode/speed specified as NA in profile */
        }

        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMap);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortDetailedMapGet);

        if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | %s | %s | %-15s | %5d %5d %3d %3d %4d %3d |",
                    portIdx, portNum,
                    doJustPrintPortMapping?"--NA-":CPSS_SPEED_2_STR(portList[portIdx].speed),
                    doJustPrintPortMapping?"  ---NA---  ":CPSS_IF_2_STR(portList[portIdx].interfaceMode),
                    CPSS_MAPPING_2_STR(portMap.portMap.mappingType),
                    portMap.portMap.rxDmaNum, portMap.portMap.txDmaNum,
                    portMap.portMap.macNum, portMap.portMap.txqNum,
                    portMap.portMap.ilknChannel, portMap.portMap.tmPortIdx);
        }
        else
        {
            if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
            {
                /* global pipeId (not relative to the tileId) */
                rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
                        portMap.portMap.macNum/*global MAC port*/,
                        &pipeId, NULL);/*local MAC*/

                rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
                        portMap.portMap.rxDmaNum/*global DMA port*/,
                        &dpIndex, NULL);/*local DMA*/

                tileId = pipeId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

                CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | %s | %s | %s | %-10s |%3d |%4d | %3d | %3d | %2d |",
                        portIdx, portNum,
                        doJustPrintPortMapping?"--NA-":CPSS_SPEED_2_STR(portList[portIdx].speed),
                        doJustPrintPortMapping?" -NA-  ":CPSS_FEC_2_STR(portList[portIdx].fecMode),
                        doJustPrintPortMapping?"  ---NA---  ":CPSS_IF_2_STR(portList[portIdx].interfaceMode),
                        CPSS_MAPPING_2_STR(portMap.portMap.mappingType),
                        portMap.portMap.macNum,
                        portMap.portMap.txqNum,
                        pipeId,
                        tileId,
                        dpIndex);
            }
            else
            {
                rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
                    portMap.portMap.rxDmaNum/*global DMA port*/,
                    &dpIndex, NULL);/*local DMA*/
                CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | %s | %s | %-10s |%3d |%4d | %2d |",
                                                portIdx, portNum
                                                ,doJustPrintPortMapping?"--NA-":CPSS_SPEED_2_STR(portList[portIdx].speed)
                                                ,doJustPrintPortMapping?"  ---NA---  ":CPSS_IF_2_STR(portList[portIdx].interfaceMode)
                                                ,CPSS_MAPPING_2_STR(portMap.portMap.mappingType)
                                                ,portMap.portMap.macNum
                                                ,portMap.portMap.txqNum
                                                ,dpIndex);
            }
        }

        if(doJustPrintPortMapping)
        {
            doJustPrintPortMapping = 0;
            continue;
        }

        PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);

        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
            if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
            {
                if ((portNum >= 24) && (portNum < 28))
                {
                    CPSS_DXCH_IMPLEMENT_WA_ENT   waArr[1];
                    GT_U32                       waInfoArr[1];
                    waArr[0] = CPSS_DXCH_IMPLEMENT_WA_XCAT3_XLG_PORT_BUFFER_STUCK_UNIDIRECTIONAL_E;
                    waInfoArr[0] = portNum;
                    rc = cpssDxChHwPpImplementWaInit(devNum, 1, &(waArr[0]), &(waInfoArr[0]));
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpImplementWaInit);
                }
            }
            if(PRV_APP_REF_PORT_MGR_MODE == GT_TRUE)
            {
                rc = cpssAppPlatformPortInitSeqStart(devNum, portNum, portList[portIdx].interfaceMode, portList[portIdx].speed,
					                                 portList[portIdx].fecMode, portList[portIdx].laneParams,
					                                 (GT_BOOL) portList[portIdx].portEnabledByDefault, trafficEnablePtr->overrideTxParams,
					                                 GT_TRUE);
                if(GT_OK != rc)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPortInitSeqStart(portNum=%d, ifMode=%s, speed=%s) :rc=%d\n",
                            portNum, CPSS_IF_2_STR(portList[portIdx].interfaceMode), CPSS_SPEED_2_STR(portList[portIdx].speed), rc);
                }
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPortInitSeqStart);
                if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
                {
                    rc = cpssDxChPortForceLinkDownEnableSet(devNum, portNum, GT_FALSE);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortForceLinkDownEnableSet);
                }

            }
            else /* configure ports in legacy mode */
            {
                rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE,
                        portList[portIdx].interfaceMode, portList[portIdx].speed);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortModeSpeedSet(portNum=%d, ifMode=%s, speed=%s) :rc=%d\n",
                            portNum, CPSS_IF_2_STR(portList[portIdx].interfaceMode), CPSS_SPEED_2_STR(portList[portIdx].speed), rc);
                }
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortModeSpeedSet);

#if 0
                /* MC FIFO mapping for SIP devices */
                rc = cpssDxChPortTxMcFifoSet(devNum, portNum, portlist_aldrin2[portIdx].mcFifo);
#endif
                /* Enable/Disable ports based on profile */
                rc = cpssDxChPortEnableSet(devNum, portList[portIdx].portNum, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortEnableSet);
            }

            /* WA for Aldrin2 DB board */
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /*if(port_serdes_squelch_WA_enable == GT_TRUE)*/
                {
                    if ((portList[portIdx].speed == CPSS_PORT_SPEED_10000_E) &&
                            (!prvCpssDxChPortRemotePortCheck(devNum, portNum)) &&
                             (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5_E))
                    {
                        /* value 2 is the normalized value of signal_ok level (squelch) of value 100mV, to Avago API units */
                        rc = cpssDxChPortSerdesSquelchSet(devNum, portNum, 2);
                        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSerdesSquelchSet);
                    }
                }
            }

            if (PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum))
            {
                if (portList[portIdx].speed <= CPSS_PORT_SPEED_10000_E ||
                        portList[portIdx].speed == CPSS_PORT_SPEED_2500_E ||
                        portList[portIdx].speed == CPSS_PORT_SPEED_5000_E)
                {
                    rc = cpssDxChPortTxMcFifoSet(devNum, portNum, mcFifo_0_1%2);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
                    mcFifo_0_1++;
                }
                else
                {
                    rc = cpssDxChPortTxMcFifoSet(devNum, portNum, ((mcFifo_2_3%2) + 2));
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
                    mcFifo_2_3++;
                }
            }
            else if ( PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
            {
                rc = cpssDxChPortTxMcFifoSet(devNum, portNum, mcFifo_0_1_2_3%4);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
                mcFifo_0_1_2_3++;
            }
            else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* Sip6 have FIFO 0-1 for all port speeds.
                 */
                rc = cpssDxChPortTxMcFifoSet(devNum, portNum, mcFifo_0_1%2);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
                mcFifo_0_1++;
            }
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        GT_U32    numOfNetIfs,mgUnitId;
        GT_U8     sdmaQueueIndex = 0;

        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_ROW_DELEMITER);
        }
        else
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_15_ROW_DELEMITER);
        }
        rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum, &numOfNetIfs);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDxChNetIfMultiNetIfNumberGet);

        /***************************/
        /* list the SDMA CPU ports */
        /***************************/
        for(/*portIdx*/;portIdx < (maxPortIdx+numOfNetIfs); portIdx++ , sdmaQueueIndex+=8)
        {
            rc = cpssDxChNetIfSdmaQueueToPhysicalPortGet(devNum,sdmaQueueIndex, &portNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNetIfSdmaQueueToPhysicalPortGet);

            rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum,/*OUT*/&portMap);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortDetailedMapGet);

            rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
                         portMap.portMap.rxDmaNum/*global DMA port*/, &dpIndex, NULL);/*local DMA*/
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert);

            PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,(sdmaQueueIndex>>3),mgUnitId);

            if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
            {
                pipeId = dpIndex / 4;
                tileId = pipeId  / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

                CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | CPU-SDMA queue [%3.1d..%3.1d]  MG[%2.1d]",portIdx,
                                                      portNum,
                                                      sdmaQueueIndex,
                                                      sdmaQueueIndex+7,
                                                      mgUnitId);

                CPSS_APP_PLATFORM_LOG_PRINT_MAC(" |%3d |%4d | %3d | %3d | %2d |"
                                                ,portMap.portMap.rxDmaNum
                                                ,portMap.portMap.txqNum
                                                ,pipeId
                                                ,tileId
                                                ,dpIndex);
            }
            else
            {
                CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | CPU-SDMA queue [%3.1d..%3.1d]  MG[%2.1d]",portIdx,
                                                      portNum,
                                                      sdmaQueueIndex,
                                                      sdmaQueueIndex+7,
                                                      mgUnitId);

                CPSS_APP_PLATFORM_LOG_PRINT_MAC(" |%3d |%4d | %2d |"
                                                ,portMap.portMap.rxDmaNum
                                                ,portMap.portMap.txqNum
                                                ,dpIndex);
            }
        }

        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_ROW_DELEMITER);
        }
        else
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_15_ROW_DELEMITER);
        }
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+----+------+-------+--------------+-----------------+------------------------------+------+");
    }

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\nTraffic Enable Done...\n");
    if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        for (portNum = 0 ; portNum < 28;)
        {
            if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType == PRV_CPSS_PORT_NOT_EXISTS_E)
            {
                if(portNum < 24)
                {
                    portNum += 4;
                }
                else
                {
                    portNum++;
                }

                continue;
            }

            if (portNum >= 24)
            {
                /*bypass setting of port 24..27*/
            }
            else
            {
                if (portList[portIdx].speed != CPSS_PORT_SPEED_NA_E)
                {
                    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5_E)
                    {
                        rc = cpssDxChPortSerdesPpmSet(devNum, portNum, 60);
                        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, "cpssDxChPortSerdesPpmSet");
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }

            if(portNum < 24)
            {
                portNum += 4;
            }
            else
            {
                portNum++;
            }
        }
        rc = xcat3PortPhy1680Init(devNum);
        if(GT_OK != rc)
        {
            return rc;
        }

    }


#ifndef INCLUDE_MPD

        if (trafficEnablePtr->phyMapListPtr == NULL)
        {
            CPSS_APP_PLATFORM_LOG_INFO_MAC("\nPhy Init - Skipping: External PHY Config not present\n");
            return rc;
        }

        CPSS_APP_PLATFORM_LOG_INFO_MAC("\nExternal Phy Init\n");
        for (portIdx = 0; portIdx < trafficEnablePtr->phyMapListPtrSize; portIdx++)
        {
            switch (trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItfType)
            {
            case CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E:
                rc = cpssDxChPhyPortSmiInterfaceSet(devNum, trafficEnablePtr->phyMapListPtr[portIdx].portNum,
                        trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItf.smiItf);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiInterfaceSet);
                if (trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItf.smiItf == CPSS_PHY_SMI_INTERFACE_0_E)
                {
                    smi0 = 1;
                }
                if ( smi0 && trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItf.smiItf == CPSS_PHY_SMI_INTERFACE_1_E)
                {
                    smi1 = 1;
                }
                break;
            case CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E:
                rc = cpssDxChPhyPortXSmiInterfaceSet(devNum, trafficEnablePtr->phyMapListPtr[portIdx].portNum,
                        trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItf.xSmiItf);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiInterfaceSet);
                if (trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItf.xSmiItf == CPSS_PHY_XSMI_INTERFACE_0_E)
                {
                    xsmi0 = 1;
                }
                if ( trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItf.xSmiItf == CPSS_PHY_XSMI_INTERFACE_1_E)
                {
                    xsmi1 = 1;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if((trafficEnablePtr->phyMapListPtr[portIdx].phyConfigData.configDataArray) != NULL)
            {
                rc = cpssDxChPhyPortAddrSet(devNum, trafficEnablePtr->phyMapListPtr[portIdx].portNum, trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyAddr);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortAddrSet);

                rc = cpssAppPlatformExtPhyConfig(devNum, trafficEnablePtr->phyMapListPtr[portIdx].portNum,
                        &(trafficEnablePtr->phyMapListPtr[portIdx].phyConfigData));
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformExtPhyConfig);
            }
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            /* 4 valid cases of SMI/XSMI setting for AC5x
                - smi0  & smi1
                - smi0  & xsmi1
                - xsmi0 & smi1
                - xsmi0 & xsmi1
            */

            if ( smi0 && smi1 )/*smi0  & smi1*/
            {
                /* Set MPP 28,29 (SMI interface 0) 30,31 (SMI interface 1) */
                GT_U32 const mppListArr[][2] =
                {
                    {28,2}, {29,2}, {30,2}, {31,2}
                };
                GT_U32 mppListSize = (sizeof(mppListArr) / sizeof(mppListArr[0]));
                for (ii = 0; ii < mppListSize; ii++)
                {
                    rc = cpssDxChHwMppSelectSet(devNum, mppListArr[ii][0], mppListArr[ii][1]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
            else if ( smi0 )/*smi0  & xsmi1*/
            {
                GT_U32 const mppListArr[][2] =
                {
                        {28,1}, {29,1}, {30,2}, {31,2}
                };
                GT_U32 mppListSize = (sizeof(mppListArr) / sizeof(mppListArr[0]));
                for (ii = 0; ii < mppListSize; ii++)
                {
                    rc = cpssDxChHwMppSelectSet(devNum, mppListArr[ii][0], mppListArr[ii][1]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
            else
            {
                if ( xsmi0 && xsmi1 )/*xsmi0  & xsmi1*/
                {
                    GT_U32 const mppListArr[][2] =
                    {
                        {28,1}, {29,1}, {30,1}, {31,1}
                    };
                    GT_U32 mppListSize = (sizeof(mppListArr) / sizeof(mppListArr[0]));
                    for (ii = 0; ii < mppListSize; ii++)
                    {
                        rc = cpssDxChHwMppSelectSet(devNum, mppListArr[ii][0], mppListArr[ii][1]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
                else/*xsmi0  & smi1*/
                {
                    GT_U32 const mppListArr[][2] =
                    {
                        {28,2}, {29,2}, {30,1}, {31,1}
                    };
                    GT_U32 mppListSize = (sizeof(mppListArr) / sizeof(mppListArr[0]));
                    for (ii = 0; ii < mppListSize; ii++)
                    {
                        rc = cpssDxChHwMppSelectSet(devNum, mppListArr[ii][0], mppListArr[ii][1]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
        }
        else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            if ( smi0 && smi1 )
            {
                GT_U32 const mppListArr[][2] =
                {
                    {28,2}, {29,2}, {30,2}, {31,2}
                };
                GT_U32 mppListSize = (sizeof(mppListArr) / sizeof(mppListArr[0]));
                for (ii = 0; ii < mppListSize; ii++)
                {
                    rc = cpssDxChHwMppSelectSet(devNum, mppListArr[ii][0], mppListArr[ii][1]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
            else
            {
                GT_U32 const mppListArr[][2] =
                {
                    {28,1}, {29,1}, {30,1}, {31,1}
                };
                    GT_U32 mppListSize = (sizeof(mppListArr) / sizeof(mppListArr[0]));
                for (ii = 0; ii < mppListSize; ii++)
                {
                    rc = cpssDxChHwMppSelectSet(devNum, mppListArr[ii][0], mppListArr[ii][1]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
#endif
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\nExternal PHY Config End...\n");
    return rc;
}

/*
* @internal cpssAppPlatformTrafficEnable function
* @endinternal
*
* @brief   Enable traffic for given ports.
*
* @param [in] *trafficEnablePtr - traffic enable handle ptr
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformTrafficEnable
(
    IN CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *trafficEnablePtr
)
{
    GT_STATUS rc = GT_OK;
    PortInitInternal_STC        * portListPtr = NULL;

    portListPtr = (PortInitInternal_STC *) cpssOsMalloc(sizeof(PortInitInternal_STC)*CPSS_MAX_PORTS_NUM_CNS);

    if(NULL==portListPtr)
    {
        rc = GT_OUT_OF_CPU_MEM;
    }
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc ,"Error allocating portListPtr");

    rc = prvCpssAppPlatformTrafficEnable(trafficEnablePtr,portListPtr,CPSS_MAX_PORTS_NUM_CNS);
    cpssOsFree(portListPtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, "prvCpssAppPlatformTrafficEnable");


    return rc;
}


/**
* @internal appRefEventsToTestsHandlerBind function
* @endinternal
*
* @brief   This routine creates new event handler for the unified event list.
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
GT_STATUS appRefEventsToTestsHandlerBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS  rc = GT_OK;            /* return code */
    char    name[30] = "eventGenerationTask"; /* task name */
    GT_U32 i;

    if (operation == CPSS_EVENT_UNMASK_E && PRV_APP_REF_RUN_TIME_CONFIG_VAR(appRefTestsEventHandlerTid) == 0)
    {
        /* call CPSS to bind the events under single handler */
        rc = cpssEventBind(cpssUniEventArr, arrLength, &PRV_APP_REF_RUN_TIME_CONFIG_VAR(testEventHandlerParam).evHndl);
        if (rc == GT_ALREADY_EXIST)
            CPSS_APP_PLATFORM_LOG_ERR_MAC("One of the events is already bound to another handler\n");
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventBind);

        /* Create event handler task */
        PRV_APP_REF_RUN_TIME_CONFIG_VAR(testEventHandlerParam).callbackFuncPtr = NULL;
        PRV_APP_REF_RUN_TIME_CONFIG_VAR(testEventHandlerParam).index = 25;
        PRV_APP_REF_RUN_TIME_CONFIG_VAR(testEventHandlerParam).numOfEvents = arrLength;
        PRV_APP_REF_RUN_TIME_CONFIG_VAR(testEventHandlerParam).eventListPtr = cpssUniEventArr;
        /* spawn all the event handler processes */
        cpssOsSprintf(name, "evHndl_test_%d", PRV_APP_REF_RUN_TIME_CONFIG_VAR(testEventHandlerParam).index);

        PRV_APP_REF_RUN_TIME_CONFIG_VAR(taskCreated) = 0;
        rc = cpssOsTaskCreate(name,
                      200,
                      _32KB,
                      cpssAppPlatformEventHandler,
                      &PRV_APP_REF_RUN_TIME_CONFIG_VAR(testEventHandlerParam),
                      &PRV_APP_REF_RUN_TIME_CONFIG_VAR(appRefTestsEventHandlerTid));
        if (rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("failed spawning evHndl_test_%d", PRV_APP_REF_RUN_TIME_CONFIG_VAR(testEventHandlerParam).index);
        }
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTaskCreate);
        osTaskGracefulCallerRegister(PRV_APP_REF_RUN_TIME_CONFIG_VAR(appRefTestsEventHandlerTid), 1, NULL, NULL);

        PRV_APP_REF_RUN_TIME_CONFIG_VAR(testEventHandlerParam).taskId = PRV_APP_REF_RUN_TIME_CONFIG_VAR(appRefTestsEventHandlerTid);

        while(PRV_APP_REF_RUN_TIME_CONFIG_VAR(taskCreated) == 0)
        {
            /* wait for indication that task created */
            cpssOsTimerWkAfter(1);
        }
    }

    for(i=0;i<arrLength;i++)
    {
        if (cpssUniEventArr[i] > CPSS_UNI_EVENT_COUNT_E) /* last event */
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if(cpssUniEventArr[i] == CPSS_PP_PEX_HIT_DEFAULT_WIN_ERR_E)
        {
            continue;
        }

        /* call the CPSS to enable those interrupts in the HW of the device */
        rc = cpssEventDeviceMaskSet(0, cpssUniEventArr[i], operation);
        switch(rc)
        {
            case GT_NOT_INITIALIZED:
            case GT_BAD_PARAM:
            case GT_NOT_FOUND:
                /* this event not relevant to this device */
                rc = GT_OK;
                break;

            case GT_OK:
                break;

            default:
                /* other real error */
                break;
        }
    }

    return rc;
}

/**
* @internal appRefEventsToTestsHandlerUnbind function
* @endinternal
*
* @brief   This routine unbind the unified event list and remove
*          the event handler made in appRefEventsToTestsHandlerBind .
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
*/
GT_STATUS appRefEventsToTestsHandlerUnbind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength
)
{
    GT_STATUS  rc = GT_OK;            /* return code */

    if (PRV_APP_REF_RUN_TIME_CONFIG_VAR(appRefTestsEventHandlerTid) != 0)
    {
        rc  = cpssEventUnBind(cpssUniEventArr, arrLength);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssEventUnBind failed rc[%d] \n",rc);
            return rc ;
        }

         rc = osTaskDelete(PRV_APP_REF_RUN_TIME_CONFIG_VAR(appRefTestsEventHandlerTid));
        if (rc != GT_OK)
        {
            cpssOsPrintf("osTaskDelete : osTaskDelete: failed on [i=%d] \n",PRV_APP_REF_RUN_TIME_CONFIG_VAR(appRefTestsEventHandlerTid));
            return rc;
        }

        PRV_APP_REF_RUN_TIME_CONFIG_VAR(appRefTestsEventHandlerTid) = 0;
    }
    return GT_OK;
}

