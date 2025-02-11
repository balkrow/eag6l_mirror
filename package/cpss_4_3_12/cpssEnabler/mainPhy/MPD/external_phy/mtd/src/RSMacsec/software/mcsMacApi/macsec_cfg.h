/*! 
 * @file macsec_cfg.h
 */

// *******************************************************************************
// *
// * MARVELL CONFIDENTIAL
// * ___________________
// *
// * Copyright (C) 2022 Marvell.
// * All Rights Reserved.
// *
// * NOTICE: All information contained herein is, and remains the property of
// * Marvell Asia Pte Ltd and its suppliers, if any. The intellectual and
// * technical concepts contained herein are proprietary to Marvell Asia Pte Ltd
// * and its suppliers and may be covered by U.S. and Foreign Patents, patents
// * in process, and are protected by trade secret or copyright law.
// * Dissemination of this information or reproduction of this material is strictly
// * forbidden unless prior written permission is obtained from Marvell Asia Pte Ltd.
// *
// ******************************************************************************
// * $Rev: 1.1.0
// * $Date: 2022-11-17-16:28:56
// ******************************************************************************

#ifndef __MACSEC_CFG_H__
#define __MACSEC_CFG_H__

#ifdef MACSEC_API_IEEE
#include "mcs_internals_ieee.h"
#else
#include "mcs_internals_CT.h"
#endif
/*! @brief The configuration of the MACsec.
 *
 * This defines all the parameters that are configurable in the software API. It is probably
 * desireable to have a configuration tool or to read these from hardware at some point.
 */

/*! @brief Known variants
 */
#define     VARIANT_mcs100          100         //!< MCS100 Variant
#define     VARIANT_mcs1ash         1           //!< MCS1ash Variant
#define     VARIANT_mcs3510         3510        //!< MCS3510 Variant
#define     VARIANT_mcs400          400         //!< MCS400 Variant
#define     VARIANT_mcst10x         10          //!< MCST10X Variant

/*! @brief major version number
 */
#define MACSEC_SWAPI_VERSION_MAJOR         0
/*! @brief minor version number
 */
#define MACSEC_SWAPI_VERSION_MINOR         0
/*! @brief release version number
 */
#define MACSEC_SWAPI_VERSION_RELEASE       0
/*! @brief build version number
 *
 * This comes directly from the SVN revision number.
 *
 * @remark This is only used for Linux, Windows imports it using a different
 *         mechanism
 */
#define MACSEC_SWAPI_VERSION_BUILD         REPO_REV
/*! @brief The configuration of the MACsec.
 *
 * This defines all the parameters that are configurable in the software API. The majority
 * of the parameters are read from the autogenerated files.
 */


/*! @brief number of ingress SecY policy table entries
 */
#define INGRESS_SECY_POLICY_TABLE_SIZE      (RA01_RS_MCS_CPM_RX_SECY_PLCY_MEM_SIZE)

/*! @brief Number of Supported ingress TCAM entries
 *
 * There are a number of tables that share one index:
 * - Flow-ID Enable
 * - Flow-ID TCAM Data
 * - Flow-ID TCAM mask
 * - Flow-ID TCAM index to Secy Map
 */
#define INGRESS_FLOW_ID_TCAM_TABLE_SIZE     (RA01_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_SIZE)
/*! @brief Number of SC CAM lookup table entries
 *
 * The SC CAM enable table shares this index
 */
#define INGRESS_SC_CAM_LOOKUP_TABLE_SIZE    (RA01_RS_MCS_CPM_RX_SC_CAM_SIZE)
/*! @brief number of ANs per SC CAM lookp key
 *
 * The index of the Ingress SC CAM lookup key table and the AN are combined to
 * get the index into the Ingress SC=AN to MA map table.
 */
#define INGRESS_AN_PER_SC                   ((RA01_RS_MCS_CPM_RX_SA_MAP_MEM_SIZE)/(RA01_RS_MCS_CPM_RX_SC_CAM_SIZE))
/*! @brief the size of the Ingress SC=AN to SA map table
 */
#define INGRESS_SA_INDEX_TABLE_SIZE         (RA01_RS_MCS_CPM_RX_SA_MAP_MEM_SIZE)
/*! @brief The size of the Ingress SA Policy table
 *
 * The ingress PN table shares this index.
 */
#define INGRESS_SA_POLICY_TABLE_SIZE        (RA01_RS_MCS_CPM_RX_SA_PLCY_MEM_SIZE)

/*! @brief The size of the Ingress SA PN table
 *
 * The ingress Policy table shares this index.
 */
#define INGRESS_SA_PN_TABLE_SIZE            (RA01_RS_MCS_CPM_RX_SA_PN_TABLE_MEM_SIZE)

/*! @brief Egress Flow-ID TCAM table size.
 *
 * The Flow-ID TCAM Index to SecY map table shares this index.
 */
#define EGRESS_FLOW_ID_TCAM_TABLE_SIZE      (RA01_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_SIZE)
/*! @brief Size of the Egress SecY Policy Table.
 */
#define EGRESS_SECY_POLICY_TABLE_SIZE       (RA01_RS_MCS_CPM_TX_SECY_PLCY_MEM_SIZE)
/*! @brief Size of the Egress SecY to SA Map table.
 */
#define EGRESS_SECY_TO_SA_MAP_TABLE_SIZE    (RA01_RS_MCS_CPM_TX_SA_MAP_MEM_SIZE)
/*! @brief Number of indices per SA Map table entry.
 */
#define EGRESS_INDICES_PER_SC               ((RA01_RS_MCS_CPM_TX_SA_PLCY_MEM_SIZE)/(RA01_RS_MCS_CPM_TX_SA_MAP_MEM_SIZE))
/*! @brief Size of the Egress SA Policy Table
 *
 * The Egress PN table shares this index.
 */
#define EGRESS_SA_POLICY_TABLE_SIZE         (RA01_RS_MCS_CPM_TX_SA_PLCY_MEM_SIZE)
/*! @brief Size of the egress PN table.
 *
 * The Egress Policy Table shares this size
 */
#define EGRESS_SA_PN_TABLE_SIZE             (RA01_RS_MCS_CPM_TX_SA_PN_TABLE_MEM_SIZE)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // __MACSEC_CFG_H__

