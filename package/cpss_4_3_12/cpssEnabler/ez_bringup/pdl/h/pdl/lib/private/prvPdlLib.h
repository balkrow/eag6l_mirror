/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\lib\private\prvpdllib.h.
 *
 * @brief   Declares the prvpdllib class
 */

#ifndef __prvPdlLibh

#define __prvPdlLibh
/**
********************************************************************************
 * @file prvPdlLib.h
 * @copyright
 *    (c), Copyright (C) 2023, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 *
 * @brief Platform driver layer - Library private declarations and APIs
 *
 * @version   1
********************************************************************************
*/

#include <pdlib/lib/private/prvPdlLib.h>
#include <pdl/init/pdlInit.h>
#include <pdl/parser/pdlParser.h>

/**
 * Return in case status != PDL_OK
 * If debug flag enabled, prints debug info
 */
#define PDL_DEBUG_STATUS(__status, __flag) if (__status==PDL_OK && __flag) prvPdlDebugRawLog
#define PDL_DEBUG_ERROR(__status, __flag) if (__status!=PDL_OK && __flag) prvPdlDebugRawLog
#define PDL_DEBUG_ERROR_PRINT prvPdlDebugRawPrint
#define PDL_LIB_DEBUG_MAC(__id) if (__id) prvPdlDebugRawLog

/**
 * Return in case status != XML_PARSER_RET_CODE_OK
 * If debug flag enabled, prints debug info
 */
#define XML_DEBUG_STATUS(__status, __flag) if (__status==XML_PARSER_RET_CODE_OK  && __flag) prvPdlDebugRawLog
#define PDL_XML_BOARD_DESC_LEN 200

/**
 * @fn  void prvPdlDebugRawLog ( IN const char * format, IN ... )
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR   The function name pointer.
 * @param   format          Describes the format to use.
 * @param   ...             The ...
 */

void prvPdlDebugRawLog(
    IN const char * func_name_PTR,
    IN UINT_32      line,
    IN const char * format,
    IN ...
);

/**
 * @fn  void prvPdlDebugRawPrint ( IN const char * format, IN ... )
 *
 * @brief   Prv pdl debug print
 *
 * @param   func_name_PTR   The function name pointer.
 * @param   format          Describes the format to use.
 * @param   ...             The ...
 */

void prvPdlDebugRawPrint(
    IN const char * func_name_PTR,
    IN UINT_32      line,
    IN const char * format,
    IN ...
);

/**
 * @fn  void prvPdlStatusDebugLogHdr ( IN const char * func_name_PTR, IN const char * line_num, IN PDL_STATUS   status, IN UINT_32  pdlIdentifier)
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR          The function name pointer.
 * @param   line_num               Describes the format to use.
 * @param   status                 error code.
 * @param   pdlIdentifier          identifier for the status type.
 */

extern BOOLEAN prvPdlStatusDebugLogHdr(
    IN   const char * func_name_PTR,
    IN   UINT_32      line_num,
    IN   PDL_STATUS   status,
    IN   UINT_32      pdlIdentifier
);

/**
 * @fn  void prvPdlLock ( IN PDL_OS_LOCK_TYPE_ENT lockType )
 *
 * @brief   uses lock mechanism given by higher layer to provide mutual exclusion
 *
 * @param [in]      lockType        what are we protecting
 */

void prvPdlLock(
    IN PDL_OS_LOCK_TYPE_ENT         lockType
);

/**
 * @fn  void prvPdlUnlock ( IN PDL_OS_LOCK_TYPE_ENT lockType )
 *
 * @brief   uses lock mechanism given by higher layer to provide mutual exclusion
 *
 * @param [in]      lockType        what are we protecting
 */

void prvPdlUnlock(
    IN PDL_OS_LOCK_TYPE_ENT         lockType
);

/**
 * @fn  PDL_STATUS prvPdlCmRegRead
 *
 * @brief   Reads PP C&M register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlCmRegRead(
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
);

/**
 * @fn  PDL_STATUS prvPdlCmRegWrite
 *
 * @brief   Writes PP C&M register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [in]      data          data to write
 */

PDL_STATUS prvPdlCmRegWrite(
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
);

/**
 * @fn  PDL_STATUS prvPdlRegRead
 *
 * @brief   Reads PP register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlRegRead(
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
);

/**
 * @fn  PDL_STATUS prvPdlRegWrite
 *
 * @brief   Writes PP register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [in]      data          data to write
 */

PDL_STATUS prvPdlRegWrite(
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
);

/**
 * @fn  PDL_STATUS prvPdlSmiRegRead
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      phyPageSelReg phy page selected register
 * @param [in]      phyPage       phy page number
 * @param [in]      regAddr       register address
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlSmiRegRead(
    IN  GT_U8      devIdx,
    IN  GT_U32     portGroupsBmp,
    IN  GT_U32     smiInterface,
    IN  GT_U32     smiAddr,
    IN  GT_U8      phyPageSelReg,
    IN  GT_U8      phyPage,
    IN  GT_U32     regAddr,
    OUT GT_U16    *dataPtr
);

/**
 * @fn  PDL_STATUS prvPdlSmiRegWrite
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      phyPageSelReg phy page selected register
 * @param [in]      phyPage       phy page number
 * @param [in]      regAddr       register address
 * @param [out]     dataPtr       data to write
 */

PDL_STATUS prvPdlSmiRegWrite(
    IN  GT_U8      devIdx,
    IN  GT_U32     portGroupsBmp,
    IN  GT_U32     smiInterface,
    IN  GT_U32     smiAddr,
    IN  GT_U8      phyPageSelReg,
    IN  GT_U8      phyPage,
    IN  GT_U32     regAddr,
    IN  GT_U16     mask,
    IN  GT_U16     data
);

/**
 * @fn  PDL_STATUS prvPdlSmiPpuLock
 *
 * @brief   Control the SMI MAC polling unit using given callback
 *
 * @param [in]      devIdx        device index
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      lock          stop/start smi auto polling unit
 * @param [out]     prevLockStatePtr previous state of the lock
 */

PDL_STATUS prvPdlSmiPpuLock(
    IN  GT_U8       devIdx,
    IN  GT_U32      smiInterface,
    IN  BOOLEAN     lock,
    OUT BOOLEAN    *prevLockStatePtr
);

/**
 * @fn  PDL_STATUS prvPdlXsmiRegRead
 *
 * @brief   Reads XSMI register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      xsmiInterface Xsmi interface (0..3)
 * @param [in]      xsmiAddr      Xsmi adresss
 * @param [in]      regAddr       register address
 * @param [in]      phyDev        the PHY device to read from (APPLICABLE RANGES: 0..31).
 * @param [out]     dataPtr       pointer to where data read will be stored
 */
PDL_STATUS prvPdlXsmiRegRead(
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev,
    OUT GT_U16     *dataPtr
);

/**
 * @fn  PDL_STATUS prvPdlXsmiRegWrite
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      regAddr       register address
 * @param [out]     data          data to write
 */
PDL_STATUS prvPdlXsmiRegWrite(
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev,
    IN  GT_U16     mask,
    IN  GT_U16     data
);
/**
 * @fn  void prvPdlI2cResultHandler
 *
 * @brief   Call to given call-back which handle I2C operation result
 *
 * @param [in]      i2c_ret_status  return status from I2C operation
 * @param [in]      slave_address   I2C device (slave) address
 * @param [in]      bus_id          I2C bus id
 * @param [in]      offset          I2C offset
 * @param [in]      i2c_write       whether the operation was I2C write or read
 */
void prvPdlI2cResultHandler(
    IN  PDL_STATUS i2c_ret_status,
    IN  UINT_8     slave_address,
    IN  UINT_8     bus_id,
    IN  UINT_8     offset,
    IN  BOOLEAN    i2c_write
);

/**
 * @fn  PDL_STATUS prvPdlI2cChannelToDevIdConvert
 *
 * @brief   Convert i2cbase address & channel id (retrieved from XML) to i2c device id
 *
 * @param [in]      i2cBaseAddress      i2c base address
 * @param [in]      i2cChannelId        i2c channel id
 * @param [out]     i2cDevIdPtr         i2c device id (as found in /dev/)
 */

PDL_STATUS prvPdlI2cChannelToDevIdConvert(
    IN  UINT_32      i2cBaseAddress,
    IN  UINT_32      i2cChannelId,
    OUT UINT_32    * i2cDevIdPtr
);
/**
 * @fn  PDL_STATUS prvPdlLedStreamClassManipulationSet
 *
 * @brief   ledstream manipulation set value
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      portType                port type
 * @param [in]      classNum                class number
 * @param [in]      classParamsPtr          value
 */

PDL_STATUS prvPdlLedStreamClassManipulationSet(
    IN  GT_U8                           devIdx,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    IN  PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

/**
 * @fn  PDL_STATUS prvPdlLibStrToEnumConvert( IN PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr, IN char * namePtr, OUT UINT_32 * valuePtr )
 *
 * @brief   Prv pdl library string to enum convert
 *
 * @param [in,out]  convertDbPtr    If non-null, the convert database pointer.
 * @param [in,out]  namePtr         If non-null, the name pointer.
 * @param [in,out]  valuePtr        If non-null, the value pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLibStrToEnumConvert(
    IN  PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr,
    IN  char                                * namePtr,
    OUT UINT_32                             * valuePtr
);

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibPortModeSupported
*
* DESCRIPTION:   verify port interface mode & speed are supported by HW
*
*****************************************************************************/

BOOLEAN prvPdlLibPortModeSupported(
    UINT_8                      devIdx,
    UINT_32                     mac_port,
    PDL_PORT_SPEED_ENT          speed,
    PDL_INTERFACE_MODE_ENT      interface_mode
);

/**
 * @fn  PDL_STATUS prvPdlLedStreamClassManipulationGet
 *
 * @brief   ledstream manipulation get value
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      portType                port type
 * @param [in]      classNum                class number
 * @param [out]     classParamsPtr          value
 */

PDL_STATUS prvPdlLedStreamClassManipulationGet(
    IN  GT_U8                           devIdx,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    OUT PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

/**
 * @fn  PDL_STATUS prvPdlLedStreamPortPositionSet
 *
 * @brief   set ledstream port position
 *
 * @param [in]      devIdx        device index
 * @param [in]      portNum       port number
 * @param [in]      position      led stream port position
 */

PDL_STATUS prvPdlLedStreamPortPositionSet(
    IN  GT_U8                   devIdx,
    IN  GT_U32                  portNum,
    IN  GT_U32                  position
);
/**
 * @fn  PDL_STATUS prvPdlLedStreamPortClassPolarityInvertEnableSet
 *
 * @brief   set ledstream port polarity
 *
 * @param [in]      devIdx        device index
 * @param [in]      portNum       port number
 * @param [in]      classNum      class number
 * @param [in]      invertEnable  polarity invert enable/disable
 */

PDL_STATUS prvPdlLedStreamPortClassPolarityInvertEnableSet(
    IN  GT_U8                           devIdx,
    IN  GT_U32                          portNum,
    IN  GT_U32                          classNum,
    IN  BOOLEAN                         invertEnable
);

/**
 * @fn  PDL_STATUS prvPdlLedStreamConfigSet
 *
 * @brief   set ledstream configuration
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      ledConfPtr              led stream configuration paramters
 */

PDL_STATUS prvPdlLedStreamConfigSet(
    IN  GT_U8                               devIdx,
    IN  GT_U32                              ledInterfaceNum,
    IN  PDL_LED_STEAM_INTERFACE_CONF_STC   *ledConfPtr
);

extern BOOLEAN prvPdlInitDone;
extern BOOLEAN prvPdlBtnDebugFlag;
extern BOOLEAN prvPdlInitDebugFlag;
extern BOOLEAN prvPdlPpDebugFlag;
extern BOOLEAN prvPdlSfpDebugFlag;
extern BOOLEAN prvPdlSerdesDebugFlag;
extern BOOLEAN prvPdlOobPortDebugFlag;
extern BOOLEAN prvPdlFanDebugFlag;
extern BOOLEAN prvPdlSensorDebugFlag;
extern BOOLEAN prvPdlPowerDebugFlag;
extern BOOLEAN prvPdlPhyDebugFlag;
extern BOOLEAN prvPdlLedDebugFlag;
extern BOOLEAN prvPdlCpuDebugFlag;
extern BOOLEAN prvPdlLibDebugFlag;
extern BOOLEAN prvPdlInterfaceDebugFlag;
extern BOOLEAN prvPdlFanControllerDebugFlag;
extern BOOLEAN prvPdlInitDebugFlag;
extern BOOLEAN prvPdlI2CInterfaceMuxDebugFlag;
extern BOOLEAN prvPdlAutoParserDebugFlag;
/**
 * @fn  PDL_STATUS pdlLibDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Lib debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLibDebugSet(
    IN  BOOLEAN             state
);
/**
 * @fn  PDL_STATUS pdlLibInit ( IN PDL_OS_CALLBACK_API_STC * callbacksPTR )
 *
 * @brief   Pdl library initialize
 *
 * @param [in,out]  callbacksPTR    If non-null, the callbacks pointer.
 *
 * @return  A PDL_STATUS.
 */

/**
 * @fn  PDL_STATUS pdlLibConvertEnumValueToString ( IN PDL_ENUM_IDS_ENT enumId, IN UINT_32 intValue, OUT char ** strValuePtr )
 *
 * @brief   Convert enum int to string value.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValue        enum int value to convert.
 * @param [out] strValuePtr     pointer to string value to return.
 *
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
PDL_STATUS pdlLibConvertEnumValueToString(
    IN PDL_ENUM_IDS_ENT         enumId,
    IN UINT_32                  intValue,
    OUT char                 ** strValuePtr
);

/*$ END OF pdlLibConvertEnumValueToString */

/**
 * @fn  PDL_STATUS pdlLibConvertStringToEnumValue ( IN PDL_ENUM_IDS_ENT enumId, IN char * strValuePtr, OUT UINT_32 * intValuePtr )
 *
 * @brief   Convert string to enum int.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  strValuePtr     string value to convert
 * @param [out] intValuePtr     enum int value to return.
 *
 * @return  PDL_OK              success
 * @return  PDL_BAD_PTR         if invalid id or pointer was supplied.
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
PDL_STATUS pdlLibConvertStringToEnumValue(
    IN PDL_ENUM_IDS_ENT         enumId,
    IN char                   * strValuePtr,
    OUT UINT_32               * intValuePtr
);

/*$ END OF pdlLibConvertEnumValueToString */

/**
 * @fn  PDL_STATUS prvPdlLibSleep
 *
 * @brief   sleep for a milliseconds amount of time
 *
 * @param [in]      msec                duration of sleep in milliseconds
 */

PDL_STATUS prvPdlLibSleep(
    UINT_32                     msec
);

/*$ END OF prvPdlLibSleep */

/**
 * @fn  PDL_STATUS prvPdlLibMpdLogicalPortValueSet
 *
 * @brief   set MPD logical port phy value
 *
 * @param [in]      mpdLogicalPort      MPD logical port number
 * @param [in]      phyDeviceOrPage     phy device or page
 * @param [in]      regAddress          phy reg address
 * @param [in]      mask                operation mask value
 * @param [in]      data                data to write
 */

PDL_STATUS prvPdlLibMpdLogicalPortValueSet(
    IN  UINT_32                         mpdLogicalPort,
    IN  UINT_8                          phyDeviceOrPage,
    IN  UINT_16                         regAddress,
    IN  UINT_16                         mask,
    IN  UINT_16                         data
);

/*$ END OF prvPdlLibMpdLogicalPortValueSet */

PDL_STATUS pdlLibInit(
    IN PDL_OS_CALLBACK_API_STC    * callbacksPTR,
    IN PDL_OS_INIT_TYPE_ENT         initType
);
#endif
