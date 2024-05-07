/*******************************************************************************
*           Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsAasPortAnp.c
*
* @brief This file contains API for ANPp port configuartion
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsAasPortAnp.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>


/**************************** Globals ****************************************************/
extern  GT_BOOL hwsPpHwTraceFlag;

/**************************** Definition *************************************************/

extern GT_STATUS mvHwsExtIfLastInPortGroupCheck
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT   *isLastPtr
);

extern GT_STATUS mvHwsExtIfFirstInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isFisrtPtr
);

extern GT_STATUS mvHwsExtIfLastInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isLastPtr
);

extern GT_U32 mvHwsAasUnitFieldAddrFind
(
    IN GT_U8           devNum,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr
);

/**************************** Declaration ********************************************/

/**
* @internal mvHwsAasAnpSerdesMuxEnableSet function
* @endinternal
*
* @brief  ANP unit SD ingress/egress MUX enable
*
* @param[in] devNum    - system device number
* @param[in] pcsNum    - system PCS channel
* @param[in] portMode  - port mode
* @param[in] serdesNum - SerDes number
* @param[in] enable    - true  - enable PCS-to-SD connection
*                      - false - disable PCS-to-SD connection
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAasAnpSerdesMuxEnableSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   pcsNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_U32                   serdesNum,
    IN GT_BOOL                  enable
)
{
    MV_HWS_UNITS_ID                 unitId;
    MV_HWS_AAS_ANP_UNITS_FIELDS_ENT fieldName;
    GT_U32                          regAddr;
    GT_U32                          regData;
    MV_HWS_REG_ADDR_FIELD_STC       fieldRegOut;

    regData = (enable) ? 1 : 0;

    if(mvHwsMtipIsReducedPort(devNum, pcsNum))
    {
           hwsOsPrintf("\n\n\n");
           hwsOsPrintf("mvHwsAasAnpSerdesMuxEnableSet: IMPLEMENTATION NEEDED for CPU ports");
           hwsOsPrintf("\n\n\n");
           unitId = ANP_CPU_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
        regData = (enable == GT_TRUE) ? 1 : 0;

        /****************************/
        /* INGRESS DIRECTION        */
        /****************************/
        fieldName = AAS_ANP_UNIT_INGRESS_SD_MUX_CONTROL_SD_MUX_INGRESS_CONTROL_0_ENABLE_E;
        regAddr = mvHwsAasUnitFieldAddrFind(devNum, pcsNum, unitId, portMode, fieldName, &fieldRegOut);
        if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
        {
            return GT_BAD_PARAM;
        }
        /* Ingress SD Mux Control <1> register */
        if(((pcsNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) %8) >= 4)
        {
            regAddr += 4;
        }
        fieldRegOut.fieldStart = 7 + (pcsNum % 4) * 8;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldRegOut.fieldStart, fieldRegOut.fieldLen, regData));


        /****************************/
        /* EGRESS DIRECTION        */
        /****************************/
        fieldName = AAS_ANP_UNIT_EGRESS_SD_MUX_CONTROL_SD_MUX_EGRESS_CONTROL_0_ENABLE_E;
        regAddr = mvHwsAasUnitFieldAddrFind(devNum, serdesNum, unitId, portMode, fieldName, &fieldRegOut);
        if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
        {
            return GT_BAD_PARAM;
        }
        /* Egress SD Mux Control <1> register */
        if(((serdesNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) %8) >= 4)
        {
            regAddr += 4;
        }
        fieldRegOut.fieldStart = 7 + (serdesNum % 4) * 8;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldRegOut.fieldStart, fieldRegOut.fieldLen, regData));
    }

    return GT_OK;
}


/**
* @internal mvHwsAasAnpSerdesMuxConfigSet function
* @endinternal
*
* @brief  ANP unit SD ingress/egress MUX
*
* @param[in] devNum    - system device number
* @param[in] portNum   - system MAC number
* @param[in] anpEnable - true  - SerDes is controlled by ANP
*                      - false - SerDes is controlled by regs
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAasAnpSerdesMuxConfigSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   pcsNum,/*pcs channel*/
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_U32                   serdesNum
)
{
    MV_HWS_UNITS_ID                 unitId;
    MV_HWS_AAS_ANP_UNITS_FIELDS_ENT fieldName;
    GT_U32                          regAddr;
    GT_U32                          regData;
    MV_HWS_REG_ADDR_FIELD_STC       fieldRegOut;

    if(mvHwsMtipIsReducedPort(devNum, pcsNum))
    {
           hwsOsPrintf("\n\n\n");
           hwsOsPrintf("mvHwsAasAnpSerdesMuxConfigSet: IMPLEMENTATION NEEDED for CPU ports");
           hwsOsPrintf("\n\n\n");
           unitId = ANP_CPU_UNIT;
    }
    else
    {
        /*
            In AAS GOP claster there are 2 ANP units with COMMON ingress and egress SD MUX
            Example:
                PCS_0 need to be connected to SD_13

            What should be configured:
                Ingress direction -
                    ANP_0 Ingress SD Mux Control <0> register
                        field [0..6]
                        value = 13

                Egress direction -
                    ANP_1 Egress SD Mux Control <1> register
                        field [8..14]
                        value = 0

        */

        unitId = ANP_400_UNIT;

        /****************************/
        /* INGRESS DIRECTION        */
        /****************************/
        fieldName = AAS_ANP_UNIT_INGRESS_SD_MUX_CONTROL_SD_MUX_INGRESS_CONTROL_0_E;
        regData = serdesNum % MV_HWS_AAS_GOP_PORT_NUM_CNS;

        regAddr = mvHwsAasUnitFieldAddrFind(devNum, pcsNum, unitId, portMode, fieldName, &fieldRegOut);
        if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
        {
            return GT_BAD_PARAM;
        }
        /* Ingress SD Mux Control <1> register */
        if(((pcsNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) %8) >= 4)
        {
            regAddr += 4;
        }
        fieldRegOut.fieldStart = (pcsNum % 4) * 8;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldRegOut.fieldStart, fieldRegOut.fieldLen, regData));


        /****************************/
        /* EGRESS DIRECTION        */
        /****************************/
        fieldName = AAS_ANP_UNIT_EGRESS_SD_MUX_CONTROL_SD_MUX_EGRESS_CONTROL_0_E;
        regData = pcsNum % MV_HWS_AAS_GOP_PORT_NUM_CNS;

        regAddr = mvHwsAasUnitFieldAddrFind(devNum, serdesNum, unitId, portMode, fieldName, &fieldRegOut);
        if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
        {
            return GT_BAD_PARAM;
        }
        /* Egress SD Mux Control <1> register */
        if(((serdesNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) %8) >= 4)
        {
            regAddr += 4;
        }
        fieldRegOut.fieldStart = (serdesNum % 4) * 8;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldRegOut.fieldStart, fieldRegOut.fieldLen, regData));

        /* Enable/disable SD MUX mask */
        CHECK_STATUS(mvHwsAasAnpSerdesMuxEnableSet(devNum, pcsNum, portMode, serdesNum, GT_TRUE));
    }

    return GT_OK;
}

/**
* @internal mvHwsAasAnpInit function
* @endinternal
*
* @brief  ANP unit init
*
* @param[in] devNum                - system device number
* @param[in] skipWritingToHW       - skip writing to HW
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAasAnpInit
(
    IN GT_U8   devNum,
    IN GT_BOOL skipWritingToHW
)
{
    GT_U32                          portNum;
    MV_HWS_UNITS_ID                 unitId;
    MV_HWS_AAS_ANP_UNITS_FIELDS_ENT regField;


    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Start of mvHwsAasAnpInit ******\n");
    }

    if (skipWritingToHW == GT_TRUE)
    {
        return GT_OK;
    }

    for(portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum - 1; portNum++)
    {
        if(mvHwsMtipIsReducedPort(devNum, portNum))
        {
            unitId = ANP_CPU_UNIT;
/*
            hwsOsPrintf("\n\n\n");
            hwsOsPrintf("mvHwsAasAnpSerdesMuxConfigSet: IMPLEMENTATION NEEDED for CPU ports");
            hwsOsPrintf("\n\n\n");
*/
            continue;
        }
        else
        {
            unitId = ANP_400_UNIT;
        }
        regField = AAS_ANP_UNIT_LANE_CONTROL_1_P0_FAST_SPEED_CHANGE_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, regField, 0x0, NULL));
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** End of mvHwsAasAnpInit ******\n");
    }

     return GT_OK;
}

/**
* @internal mvHwsAasAnpPortEnable function
* @endinternal
*
* @brief  ANP unit port enable
*
* @param[in] devNum                - system device number
* @param[in] portNum               - system port number
* @param[in] portMode              - configured port mode
* @param[in] apEnable              - apEnable flag
* @param[in] enable                - True = enable/False =
*       disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAasAnpPortEnable
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              apEnable,
    GT_BOOL              enable
)
{
    /*GT_STATUS                       rc;
    GT_U32                          regValue;
    MV_HWS_HAWK_CONVERT_STC         convertIdx;
    MV_HWS_AAS_ANP_UNITS_FIELDS_ENT anpField;*/
    MV_HWS_UNITS_ID                 unitId;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32                          i;
    GT_U32                          owValue = 0;

    if(((apEnable == GT_FALSE) && (enable == GT_TRUE))  || cpssDeviceRunCheck_onEmulator())
    {
        owValue = 1;
    }

#ifdef GM_USED
    return GT_OK;
#endif

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasAnpPortEnable ******\n");
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;

        for(i = 0; i < curPortParams.numOfActLanes; i++)
        {
            if(enable == GT_TRUE)
            {
                /* SD MUX configuration + mask enable */
                CHECK_STATUS(mvHwsAasAnpSerdesMuxConfigSet(devNum, portNum + i, portMode, curPortParams.activeLanesList[i]));
            }
            else
            {
                /* SD MUX mask disable */
                CHECK_STATUS(mvHwsAasAnpSerdesMuxEnableSet(devNum, portNum + i, portMode, curPortParams.activeLanesList[i], GT_FALSE));
            }

            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum + i, unitId, AAS_ANP_UNIT_LANE_CONTROL_7_P0_PM_PCS_SD_RX_RESETN_OW_E, owValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum + i, unitId, AAS_ANP_UNIT_LANE_CONTROL_7_P0_PM_PCS_SD_RX_RESETN_OW_VAL_E, owValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum + i, unitId, AAS_ANP_UNIT_LANE_CONTROL_7_P0_PM_PCS_SD_TX_RESETN_OW_E, owValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum + i, unitId, AAS_ANP_UNIT_LANE_CONTROL_7_P0_PM_PCS_SD_TX_RESETN_OW_VAL_E, owValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum + i, unitId, AAS_ANP_UNIT_LANE_CONTROL_7_P0_PM_PCS_RX_CLK_ENA_OW_E, owValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum + i, unitId, AAS_ANP_UNIT_LANE_CONTROL_7_P0_PM_PCS_RX_CLK_ENA_OW_VAL_E, owValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum + i, unitId, AAS_ANP_UNIT_LANE_CONTROL_7_P0_PM_PCS_TX_CLK_ENA_OW_E, owValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum + i, unitId, AAS_ANP_UNIT_LANE_CONTROL_7_P0_PM_PCS_TX_CLK_ENA_OW_VAL_E, owValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum + i, unitId, AAS_ANP_UNIT_LANE_CONTROL_7_P0_REG_SIGDET_MODE_E, (1-owValue), NULL));
        }
    }


    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasAnpPortEnable ******\n");
    }

    return GT_OK;
}




