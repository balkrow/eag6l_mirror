/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\packet_processor\private\prvpdlpacketprocessor.h.
 *
 * @brief   Declares the prvpdlpacketprocessor class
 */

#ifndef __prvPdlPacketProcessorh

#define __prvPdlPacketProcessorh
/**
********************************************************************************
 * @file prvPdlPacketProcessor.h
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
 * @brief Platform driver layer - Packet Processor private declarations and APIs
 *
 * @version   1
********************************************************************************
*/
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>

/**
* @addtogroup Packet_Processor
* @{
*/
/**
* @defgroup Packet_processor_private Packet Processor Private
* @{Button private definitions and declarations including:
*/

/*! Max number of Back to Back packet processors */
#define PRV_PDL_PP_B2B_MAX_NUM                          10

#define PRV_PDL_FP_FIRST_PORT_NUMBER_DEFAULT_SHIFT_CNS  48

/**
 * @struct  PRV_PDL_PORT_DB_KEY_STC
 *
 * @brief   defines structure for key of port db
 */

typedef struct {
    /** @brief   The port */
    UINT_32                                 port;
} PRV_PDL_PORT_DB_KEY_STC;

/**
 * @struct  PRV_PDL_PORT_DB_ENTRY_STC
 *
 * @brief   defines structure for entry of port db
 */

typedef struct {
    /** @brief   key */
    PRV_PDL_PORT_DB_KEY_STC                 key;
    /** @brief   Corresponding front panel entry */
    void                                   *frontPanelEntryPtr;
    /** @brief   Corresponding front panel port entry */
    void                                   *frontPanelPortEntryPtr;
    /** @brief   In case of phy - phy smi/xsmi interface */
    PDL_INTERFACE_TYP                       phyInterfaceId;
} PRV_PDL_PORT_DB_ENTRY_STC;

/**
 * @struct  PRV_PDL_DEVICE_DB_KEY_STC
 *
 * @brief   defines structure for key of device db
 */

typedef struct {
    /** @brief   The device */
    UINT_32                                 dev;
} PRV_PDL_DEVICE_DB_KEY_STC;

/**
 * @struct  PRV_PDL_DEVICE_DB_ENTRY_STC
 *
 * @brief   defines structure for entry of device db
 */

typedef struct {
    /** @brief   key */
    PRV_PDL_DEVICE_DB_KEY_STC               key;
    /** @brief   The logical ports db */
    PRV_PDLIB_DB_TYP                          logicalPortsList;
    /** @brief   The mac ports db */
    PRV_PDLIB_DB_TYP                          macPortsList;
} PRV_PDL_DEVICE_DB_ENTRY_STC;

/**
 * @struct  PRV_PDL_PORT_ENTRY_KEY_STC
 *
 * @brief   defines structure for port key in db
 */

typedef struct {
    /** @brief   The device */
    UINT_32                                 dev;
    /** @brief   The mac port */
    UINT_32                                 logicalPort;
} PRV_PDL_PORT_ENTRY_KEY_STC;


/**
 * @struct  PRV_PDL_PORT_ENTRY_STC
 *
 * @brief   defines structure for port entry in db
 */

typedef struct {
    /** @brief   The key */
    PRV_PDL_PORT_ENTRY_KEY_STC              key;
    /** @brief   The data */
    PDL_PP_NETWORK_PORT_ATTRIBUTES_STC      data;
    PRV_PDLIB_DB_STC                          fiberPhyInitValueDb[PDL_PORT_SPEED_LAST_E];
    PRV_PDLIB_DB_STC                          copperPhyInitValueDb[PDL_PORT_SPEED_LAST_E];
    PDL_INTERFACE_EXTENDED_TYPE_ENT         phyInterfaceType;
    PDL_INTERFACE_TYP                       phyInterfaceId;
    PRV_PDLIB_DB_STC                          phyPostInitValues;
} PRV_PDL_PORT_ENTRY_STC;

/**
 * @struct  PRV_PDL_FRONT_PANEL_GROUP_KEY_STC
 *
 * @brief   defines structure for front panel key in db
 */

typedef struct {
    /** @brief   The front panel number */
    UINT_32                                 frontPanelNumber;
} PRV_PDL_FRONT_PANEL_GROUP_KEY_STC;

/**
 * @struct  PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC
 *
 * @brief   defines structure for front panel entry in db
 */

typedef struct {
    /** @brief   The key */
    PRV_PDL_FRONT_PANEL_GROUP_KEY_STC       key;

    /** @brief   The data */
    PDL_PP_FRONT_PANEL_ATTRIBUTES_STC       data;
} PRV_PDL_FRONT_PANEL_GROUP_ENTRY_STC;

/**
 * @fn  PDL_STATUS pdlPacketProcessorInit ( void );
 *
 * @brief   Init Packet Processor module Create DB and initialize
 *
 * @return  PDL_STATUS
 */

PDL_STATUS pdlPacketProcessorInit(
    void
);

/**
 * @fn  PDL_STATUS pdlPpDbPortAttributesSet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_PP_NETWORK_PORT_ATTRIBUTES_STC * portAttributesPtr );
 *
 * @brief   Update PP port attributes
 *
 * @param [in]  dev                 dev number.
 * @param [in]  port                port number.
 * @param [out] portAttributesPtr   port attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  port's entry wasn't found.
 */

PDL_STATUS pdlPpDbPortAttributesSet(
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,
    OUT  PDL_PP_NETWORK_PORT_ATTRIBUTES_STC       * portAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlPpDbB2bAttributesSet ( IN UINT_32 b2bLinkId, OUT PDL_PP_B2B_ATTRIBUTES_STC * b2bAttributesPtr );
 *
 * @brief   Update PP back-to-back link attributes
 *
 * @param [in]  b2bLinkId           b2b link number.
 * @param [out] b2bAttributesPtr    b2b link attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  b2b's entry wasn't found.
 */

PDL_STATUS pdlPpDbB2bAttributesSet(
    IN   UINT_32                                 b2bLinkId,
    OUT  PDL_PP_B2B_ATTRIBUTES_STC             * b2bAttributesPtr
);

/**
 * @fn  PDL_STATUS pdlPpDbFrontPanelAttributesSet ( IN UINT_32 frontPanelNumber, OUT PDL_PP_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr );
 *
 * @brief   Update attributes of front panel group
 *
 * @param [in]  frontPanelNumber        front panel group id.
 * @param [out] frontPanelAttributesPtr front panel attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpDbFrontPanelAttributesSet(
    IN   UINT_32                                 frontPanelNumber,
    OUT  PDL_PP_FRONT_PANEL_ATTRIBUTES_STC     * frontPanelAttributesPtr
);

/**
 * @fn  PDL_STATUS prvPdlPpPortGetPhyInterfceId (IN UINT_32 dev, IN UINT_32 logicalPort, OUT UINT_32 * phyInterfaceIdPtr );
 *
 * @brief   Get the phy interface id that is used inside PDL private interface database
 *
 * @param [in]      dev                     dev number.
 * @param [in]      logiaclPort             logicalPort number.
 * @param [out]     phyInterfaceIdPtr       phyInterfaceId.
  *
 * @return  PDL_OK         success.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS prvPdlPpPortGetPhyInterfceId(
    IN  UINT_32                                  dev,
    IN  UINT_32                                  logicalPort,
    OUT UINT_32                                * phyInterfaceIdPtr
);

/**
 * @fn  PDL_STATUS prvPdlPacketProcessorDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPacketProcessorDestroy(
    void
);

/* @}*/
/* @}*/

#endif
