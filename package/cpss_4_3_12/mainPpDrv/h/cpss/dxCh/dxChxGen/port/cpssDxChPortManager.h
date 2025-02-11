/********************************************************************************
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
* @file cpssDxChPortManager.h
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPortManager
#define __cpssDxChPortManager

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/port/cpssPortManager.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal cpssDxChPortManagerEventSet function
* @endinternal
*
* @brief   Set the port according to the given event. This API performs actions based on
*         the given event and will transfer the port to a new state. For example, when port
*         is in PORT_MANAGER_STATE_RESET_E state and event PORT_MANAGER_EVENT_CREATE_E is called,
*         this API will perform port creation operations and upon success, port state will be
*         changed to PORT_MANAGER_STATE_LINK_DOWN_E state.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portEventStc             - (pointer to) structure containing the event data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_RESOURCE           - no resource
*/
GT_STATUS cpssDxChPortManagerEventSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_MANAGER_STC   *portEventStc
);

/**
* @internal cpssDxChPortManagerStatusGet function
* @endinternal
*
* @brief   Get the status of the port. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerStatusGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC    *portStagePtr
);

/**
* @internal cpssDxChPortManagerPortParamsSet function
* @endinternal
*
* @brief   Set the given attributes to a port. Those configurations will take place
*         during the port management. This API is applicable only when port is in
*         reset (PORT_MANAGER_STATE_RESET_E state).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerPortParamsSet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    IN  CPSS_PM_PORT_PARAMS_STC               *portParamsStcPtr
);

/**
* @internal cpssDxChPortManagerPortParamsGet function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssDxChPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently, On devices below SIP_5_15, if RX parameters not set and
*       port is in reset, the RX values are not availble to be fetched, and
*       filled with zeros.
*
*/
GT_STATUS cpssDxChPortManagerPortParamsGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT CPSS_PM_PORT_PARAMS_STC               *portParamsStcPtr
);

/**
* @internal cpssDxChPortManagerPortParamsStructInit function
* @endinternal
*
* @brief  This API resets the Port Manager Parameter structure
*         so once the application update the structure with
*         relevant data - all other feilds are clean for sure.
*         Application must call this API before calling
*         cpssDxChPortManagerPortParamsSet.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in,out] portParamsStcPtr   - (pointer to) a structure
*       contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChPortManagerPortParamsStructInit
(
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    INOUT  CPSS_PM_PORT_PARAMS_STC *portParamsStcPtr
);

/**
* prvCpssDxChPortManagerParametersSetExt function
*
* @brief   extended parmaeter set port manager command
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                    - interface mode
* @param[in] speed                    - speed
* @param[in] adaptiveMode             - wheater to run adaptive
*       or not
* @param[in] trainMode                - training mode
* @param[in] overrideElectricalParams - override TX parameters
* @param[in] overridePolarityParams   - override polarity
*       parameters
* @param[in] -                        minLF
* @param[in] -                        maxLF
* @param[in] fecMode                  = fecMode
*
* @param[out] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssDxChPortManagerParametersSetExt
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         adaptiveMode,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT  trainMode,
    IN  GT_BOOL                              overrideElectricalParams,
    IN  GT_BOOL                              overridePolarityParams,
    IN  GT_U8                            minLF,
    IN  GT_U8                            maxLF,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
);


/**
* prvCpssDxChPortManagerUnMaskModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortManagerUnMaskModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT            ifMode,
    IN  CPSS_PORT_SPEED_ENT                     speed,
    IN  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskMode
);

/**
* @internal cpssDxChPortManagerGlobalParamsOverride function
* @endinternal
*
* @brief  To use this function for overide, need to give it a
*         struct that include parameters to override, and to
*         enable override flag to 1, else it's will not override
*         and use defualts values.
*         Don't forget to fill the globalParamstType enum with
*         the match enum of what you want to override.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] globalParamsStc          - stc with the parameters
*                                       to override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To add other globals parameter to the struct, make sure
*       to add enum and enable flag for the set of values.
*       *See CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC as a
*        good format to add other parameters.
*
*/
GT_STATUS cpssDxChPortManagerGlobalParamsOverride
(
    IN  GT_U8                                  devNum,
    IN  CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC    *globalParamsStcPtr
);

/** @internal cpssDxChPortManagerPortParamsUpdate function
* @endinternal
*
* @brief  To use this function for update the PA mode also when port is active. In order to configure the new mode,
* application should call the port manager update API (called cpssDxChPortManagerPortParamsUpdate ) with the new
* specified mode
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] updateParamsStc          - stc with the parameters to update
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerPortParamsUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_MANAGER_UPDATE_PARAMS_STC *updateParamsStcPtr
);

/**
* @internal cpssDxChSamplePortManagerMandatoryParamsSet function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - port mode
* @param[in] speed                    - port speed
* @param[in] speed                    - port fec mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerMandatoryParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
);

/**
* @internal cpssDxChSamplePortManagerLoopbackSet function
* @endinternal
*
* @brief   set the Loopback params in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] loopbackType             - loopback type
* @param[in] macLoopbackEnable        - MAC loopback
*       enable/disable
* @param[in] serdesLoopbackMode       - SerDes loopback type
* @param[in] enableRegularTraffic     - enable trafic
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerLoopbackSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT     loopbackType,
    IN  GT_BOOL                                 macLoopbackEnable,
    IN  CPSS_PORT_SERDES_LOOPBACK_MODE_ENT      serdesLoopbackMode,
    IN  GT_BOOL                                 enableRegularTraffic
);

/**
* @internal cpssDxChPortManagerInit function
* @endinternal
*
* @brief  Port Manager Init system - this function is
*         responsible on all configurations that till now was
*         application responsiblity.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/

GT_STATUS cpssDxChPortManagerInit
(
    IN  GT_U8   devNum
);

/**
* @internal cpssDxChPortManagerEnableGet function
* @endinternal
*
* @brief  This function is returns if port manager enabled/disabled
*
* @note   APPLICABLE DEVICES:      AC3X; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3
*
* @param[in] devNum                - physical device number
* @param[out] enablePtr            - (pointer to)port manager enable/disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/

GT_STATUS cpssDxChPortManagerEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortManagerStatGet function
* @endinternal
*
* @brief  This function is returns the port manager statistics
*
* @note   APPLICABLE DEVICES:      AC3X;Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3
*
* @param[in] devNum             - physical device number
* @param[in] portNum            - port number
* @param[out] portStatStcPtr    - statistics structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChPortManagerStatGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PORT_MANAGER_STATISTICS_STC    *portStatStcPtr
);

/**
* @internal cpssDxChPortManagerStatClear function
* @endinternal
*
* @brief  This function clear the port manager statistics.
*
* @note   APPLICABLE DEVICES:      AC3X;Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3
*
* @param[in] devNum             - physical device number
* @param[in] portNum            - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChPortManagerStatClear
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum
);

/*
* @internal prvCpssPortManagerLinkChangeWA
* @endinternal
*
* @brief  function for handling link change WA in UX modes.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] tmpPortManagerDbPtr   - port manager DB
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPortManagerLinkChangeWA
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortManager */

