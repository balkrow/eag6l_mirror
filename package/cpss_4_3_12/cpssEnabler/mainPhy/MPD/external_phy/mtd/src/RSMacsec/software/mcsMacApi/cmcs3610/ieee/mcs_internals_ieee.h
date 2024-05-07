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

#include <stdio.h>

#ifndef __MCS_INTERNALS_NOCT_H
#define __MCS_INTERNALS_NOCT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rianta_arch.h"

#include "rianta_platform.h"

/********************************************************************************
 *     T Y P E D E F S
 ********************************************************************************/

/* Tag API functions with the DLL_PUBLIC macro: */
#ifndef DLL_PUBLIC
 #define DLL_PUBLIC extern
#endif

/* Set the basic register size for this hardware */
typedef uint64_t   ra01_register_t;

/* @brief Set the internal data bus width
 * 
 * The data bus width determines the minimum spacing of the registers. If the registers is
 * less than or equal to this value it is one 32-bit write, otherwise it is a 
 * 64-bit sized write or writes. Note that memories are always 64-bit aligned.
 */
#define APD_DATA_WIDTH 32

#define RA01_MAKE_MASK64(__RA01_MAKE_MASK_LSB) \
   (((((ra01_register_t)1U << __RA01_MAKE_MASK_MSB) - 1) | ((ra01_register_t)1U << __RA01_MAKE_MASK_MSB)) & (~(((ra01_register_t)1U << __RA01_MAKE_MASK_LSB) - 1)))

/* @brief Size descriminator for memory access
 * 
 * Used to indicate the size of a field access when reading from or writing to 
 * a memory field on the device.
 *
 */
typedef enum {
    RA01_TYPE_BOOL,         // Element is a boolean
    RA01_TYPE_REGISTER,     // Element is a register
    RA01_TYPE_MEMORY,       // Element is a memory field
} ra01_mem_access_type_t;

/* @brief Access operation code
 * 
 * Used to indicate the type of operation to be performed on a register
 */
typedef enum {
    RA01_WRITE_OP,    // write operation
    RA01_READ_OP,     // read operation
} ra01_reg_access_op_t;

/* @defgroup RA01 memory sizes
 *
 * @brief Extraction of the memory sizes.
 *
 * @{
 */

/*
 * From rs_mcs_cse_rx_mem_slave
 */
#define RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLOCTETS_SIZE                         32    // Number of elements in IFINUNCTLOCTETS
#define RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLOCTETS_WIDTH_IN_BITS                64    // Bits in each element of IFINUNCTLOCTETS

#define RA01_RS_MCS_CSE_RX_MEM_IFINCTLOCTETS_SIZE                           32    // Number of elements in IFINCTLOCTETS
#define RA01_RS_MCS_CSE_RX_MEM_IFINCTLOCTETS_WIDTH_IN_BITS                  64    // Bits in each element of IFINCTLOCTETS

#define RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLUCPKTS_SIZE                         32    // Number of elements in IFINUNCTLUCPKTS
#define RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLUCPKTS_WIDTH_IN_BITS                64    // Bits in each element of IFINUNCTLUCPKTS

#define RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLMCPKTS_SIZE                         32    // Number of elements in IFINUNCTLMCPKTS
#define RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLMCPKTS_WIDTH_IN_BITS                64    // Bits in each element of IFINUNCTLMCPKTS

#define RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLBCPKTS_SIZE                         32    // Number of elements in IFINUNCTLBCPKTS
#define RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLBCPKTS_WIDTH_IN_BITS                64    // Bits in each element of IFINUNCTLBCPKTS

#define RA01_RS_MCS_CSE_RX_MEM_IFINCTLUCPKTS_SIZE                           32    // Number of elements in IFINCTLUCPKTS
#define RA01_RS_MCS_CSE_RX_MEM_IFINCTLUCPKTS_WIDTH_IN_BITS                  64    // Bits in each element of IFINCTLUCPKTS

#define RA01_RS_MCS_CSE_RX_MEM_IFINCTLMCPKTS_SIZE                           32    // Number of elements in IFINCTLMCPKTS
#define RA01_RS_MCS_CSE_RX_MEM_IFINCTLMCPKTS_WIDTH_IN_BITS                  64    // Bits in each element of IFINCTLMCPKTS

#define RA01_RS_MCS_CSE_RX_MEM_IFINCTLBCPKTS_SIZE                           32    // Number of elements in IFINCTLBCPKTS
#define RA01_RS_MCS_CSE_RX_MEM_IFINCTLBCPKTS_WIDTH_IN_BITS                  64    // Bits in each element of IFINCTLBCPKTS

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYUNTAGGEDORNOTAG_SIZE               32    // Number of elements in INPKTSSECYUNTAGGEDORNOTAG
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYUNTAGGEDORNOTAG_WIDTH_IN_BITS      64    // Bits in each element of INPKTSSECYUNTAGGEDORNOTAG

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYBADTAG_SIZE                        32    // Number of elements in INPKTSSECYBADTAG
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYBADTAG_WIDTH_IN_BITS               64    // Bits in each element of INPKTSSECYBADTAG

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYCTL_SIZE                           32    // Number of elements in INPKTSSECYCTL
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYCTL_WIDTH_IN_BITS                  64    // Bits in each element of INPKTSSECYCTL

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYTAGGEDCTL_SIZE                     32    // Number of elements in INPKTSSECYTAGGEDCTL
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYTAGGEDCTL_WIDTH_IN_BITS            64    // Bits in each element of INPKTSSECYTAGGEDCTL

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYUNKNOWNSCI_SIZE                    32    // Number of elements in INPKTSSECYUNKNOWNSCI
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYUNKNOWNSCI_WIDTH_IN_BITS           64    // Bits in each element of INPKTSSECYUNKNOWNSCI

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYNOSCI_SIZE                         32    // Number of elements in INPKTSSECYNOSCI
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYNOSCI_WIDTH_IN_BITS                64    // Bits in each element of INPKTSSECYNOSCI

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSCTRLPORTDISABLED_SIZE                  32    // Number of elements in INPKTSCTRLPORTDISABLED
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSCTRLPORTDISABLED_WIDTH_IN_BITS         64    // Bits in each element of INPKTSCTRLPORTDISABLED

#define RA01_RS_MCS_CSE_RX_MEM_INOCTETSSCVALIDATE_SIZE                      32    // Number of elements in INOCTETSSCVALIDATE
#define RA01_RS_MCS_CSE_RX_MEM_INOCTETSSCVALIDATE_WIDTH_IN_BITS             64    // Bits in each element of INOCTETSSCVALIDATE

#define RA01_RS_MCS_CSE_RX_MEM_INOCTETSSCDECRYPTED_SIZE                     32    // Number of elements in INOCTETSSCDECRYPTED
#define RA01_RS_MCS_CSE_RX_MEM_INOCTETSSCDECRYPTED_WIDTH_IN_BITS            64    // Bits in each element of INOCTETSSCDECRYPTED

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSCUNCHECKED_SIZE                       32    // Number of elements in INPKTSSCUNCHECKED
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSCUNCHECKED_WIDTH_IN_BITS              64    // Bits in each element of INPKTSSCUNCHECKED

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSCLATEORDELAYED_SIZE                   32    // Number of elements in INPKTSSCLATEORDELAYED
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSCLATEORDELAYED_WIDTH_IN_BITS          64    // Bits in each element of INPKTSSCLATEORDELAYED

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSCCAMHIT_SIZE                          32    // Number of elements in INPKTSSCCAMHIT
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSCCAMHIT_WIDTH_IN_BITS                 64    // Bits in each element of INPKTSSCCAMHIT

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSAOK_SIZE                              64    // Number of elements in INPKTSSAOK
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSAOK_WIDTH_IN_BITS                     64    // Bits in each element of INPKTSSAOK

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSAINVALID_SIZE                         64    // Number of elements in INPKTSSAINVALID
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSAINVALID_WIDTH_IN_BITS                64    // Bits in each element of INPKTSSAINVALID

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSANOTVALID_SIZE                        64    // Number of elements in INPKTSSANOTVALID
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSANOTVALID_WIDTH_IN_BITS               64    // Bits in each element of INPKTSSANOTVALID

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSAUNUSEDSA_SIZE                        64    // Number of elements in INPKTSSAUNUSEDSA
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSAUNUSEDSA_WIDTH_IN_BITS               64    // Bits in each element of INPKTSSAUNUSEDSA

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSANOTUSINGSAERROR_SIZE                 64    // Number of elements in INPKTSSANOTUSINGSAERROR
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSSANOTUSINGSAERROR_WIDTH_IN_BITS        64    // Bits in each element of INPKTSSANOTUSINGSAERROR

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSFLOWIDTCAMMISS_SIZE                    1     // Number of elements in INPKTSFLOWIDTCAMMISS
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSFLOWIDTCAMMISS_WIDTH_IN_BITS           64    // Bits in each element of INPKTSFLOWIDTCAMMISS

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSPARSEERR_SIZE                          1     // Number of elements in INPKTSPARSEERR
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSPARSEERR_WIDTH_IN_BITS                 64    // Bits in each element of INPKTSPARSEERR

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSEARLYPREEMPTERR_SIZE                   1     // Number of elements in INPKTSEARLYPREEMPTERR
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSEARLYPREEMPTERR_WIDTH_IN_BITS          64    // Bits in each element of INPKTSEARLYPREEMPTERR

#define RA01_RS_MCS_CSE_RX_MEM_INPKTSFLOWIDTCAMHIT_SIZE                     32    // Number of elements in INPKTSFLOWIDTCAMHIT
#define RA01_RS_MCS_CSE_RX_MEM_INPKTSFLOWIDTCAMHIT_WIDTH_IN_BITS            64    // Bits in each element of INPKTSFLOWIDTCAMHIT

/*
 * From rs_mcs_cse_tx_mem_slave
 */
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCOMMONOCTETS_SIZE                       32    // Number of elements in IFOUTCOMMONOCTETS
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCOMMONOCTETS_WIDTH_IN_BITS              64    // Bits in each element of IFOUTCOMMONOCTETS

#define RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLOCTETS_SIZE                        32    // Number of elements in IFOUTUNCTLOCTETS
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLOCTETS_WIDTH_IN_BITS               64    // Bits in each element of IFOUTUNCTLOCTETS

#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLOCTETS_SIZE                          32    // Number of elements in IFOUTCTLOCTETS
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLOCTETS_WIDTH_IN_BITS                 64    // Bits in each element of IFOUTCTLOCTETS

#define RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLUCPKTS_SIZE                        32    // Number of elements in IFOUTUNCTLUCPKTS
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLUCPKTS_WIDTH_IN_BITS               64    // Bits in each element of IFOUTUNCTLUCPKTS

#define RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLMCPKTS_SIZE                        32    // Number of elements in IFOUTUNCTLMCPKTS
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLMCPKTS_WIDTH_IN_BITS               64    // Bits in each element of IFOUTUNCTLMCPKTS

#define RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLBCPKTS_SIZE                        32    // Number of elements in IFOUTUNCTLBCPKTS
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLBCPKTS_WIDTH_IN_BITS               64    // Bits in each element of IFOUTUNCTLBCPKTS

#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLUCPKTS_SIZE                          32    // Number of elements in IFOUTCTLUCPKTS
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLUCPKTS_WIDTH_IN_BITS                 64    // Bits in each element of IFOUTCTLUCPKTS

#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLMCPKTS_SIZE                          32    // Number of elements in IFOUTCTLMCPKTS
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLMCPKTS_WIDTH_IN_BITS                 64    // Bits in each element of IFOUTCTLMCPKTS

#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLBCPKTS_SIZE                          32    // Number of elements in IFOUTCTLBCPKTS
#define RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLBCPKTS_WIDTH_IN_BITS                 64    // Bits in each element of IFOUTCTLBCPKTS

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECYUNTAGGED_SIZE                     32    // Number of elements in OUTPKTSSECYUNTAGGED
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECYUNTAGGED_WIDTH_IN_BITS            64    // Bits in each element of OUTPKTSSECYUNTAGGED

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECYTOOLONG_SIZE                      32    // Number of elements in OUTPKTSSECYTOOLONG
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECYTOOLONG_WIDTH_IN_BITS             64    // Bits in each element of OUTPKTSSECYTOOLONG

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECYNOACTIVESA_SIZE                   32    // Number of elements in OUTPKTSSECYNOACTIVESA
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECYNOACTIVESA_WIDTH_IN_BITS          64    // Bits in each element of OUTPKTSSECYNOACTIVESA

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSCTRLPORTDISABLED_SIZE                 32    // Number of elements in OUTPKTSCTRLPORTDISABLED
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSCTRLPORTDISABLED_WIDTH_IN_BITS        64    // Bits in each element of OUTPKTSCTRLPORTDISABLED

#define RA01_RS_MCS_CSE_TX_MEM_OUTOCTETSSCPROTECTED_SIZE                    32    // Number of elements in OUTOCTETSSCPROTECTED
#define RA01_RS_MCS_CSE_TX_MEM_OUTOCTETSSCPROTECTED_WIDTH_IN_BITS           64    // Bits in each element of OUTOCTETSSCPROTECTED

#define RA01_RS_MCS_CSE_TX_MEM_OUTOCTETSSCENCRYPTED_SIZE                    32    // Number of elements in OUTOCTETSSCENCRYPTED
#define RA01_RS_MCS_CSE_TX_MEM_OUTOCTETSSCENCRYPTED_WIDTH_IN_BITS           64    // Bits in each element of OUTOCTETSSCENCRYPTED

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSAPROTECTED_SIZE                      64    // Number of elements in OUTPKTSSAPROTECTED
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSAPROTECTED_WIDTH_IN_BITS             64    // Bits in each element of OUTPKTSSAPROTECTED

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSAENCRYPTED_SIZE                      64    // Number of elements in OUTPKTSSAENCRYPTED
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSAENCRYPTED_WIDTH_IN_BITS             64    // Bits in each element of OUTPKTSSAENCRYPTED

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSFLOWIDTCAMMISS_SIZE                   1     // Number of elements in OUTPKTSFLOWIDTCAMMISS
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSFLOWIDTCAMMISS_WIDTH_IN_BITS          64    // Bits in each element of OUTPKTSFLOWIDTCAMMISS

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSPARSEERR_SIZE                         1     // Number of elements in OUTPKTSPARSEERR
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSPARSEERR_WIDTH_IN_BITS                64    // Bits in each element of OUTPKTSPARSEERR

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECTAGINSERTIONERR_SIZE               1     // Number of elements in OUTPKTSSECTAGINSERTIONERR
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECTAGINSERTIONERR_WIDTH_IN_BITS      64    // Bits in each element of OUTPKTSSECTAGINSERTIONERR

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSEARLYPREEMPTERR_SIZE                  1     // Number of elements in OUTPKTSEARLYPREEMPTERR
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSEARLYPREEMPTERR_WIDTH_IN_BITS         64    // Bits in each element of OUTPKTSEARLYPREEMPTERR

#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSFLOWIDTCAMHIT_SIZE                    32    // Number of elements in OUTPKTSFLOWIDTCAMHIT
#define RA01_RS_MCS_CSE_TX_MEM_OUTPKTSFLOWIDTCAMHIT_WIDTH_IN_BITS           64    // Bits in each element of OUTPKTSFLOWIDTCAMHIT

/*
 * From rs_mcs_cpm_rx_slave
 */
#define RA01_RS_MCS_CPM_RX_SECY_MAP_MEM_SIZE                                32    // Number of elements in secy_map_mem
#define RA01_RS_MCS_CPM_RX_SECY_MAP_MEM_WIDTH_IN_BITS                       6     // Bits in each element of secy_map_mem

#define RA01_RS_MCS_CPM_RX_SECY_PLCY_MEM_SIZE                               32    // Number of elements in secy_plcy_mem
#define RA01_RS_MCS_CPM_RX_SECY_PLCY_MEM_WIDTH_IN_BITS                      73    // Bits in each element of secy_plcy_mem

#define RA01_RS_MCS_CPM_RX_SA_MAP_MEM_SIZE                                  128   // Number of elements in sa_map_mem
#define RA01_RS_MCS_CPM_RX_SA_MAP_MEM_WIDTH_IN_BITS                         7     // Bits in each element of sa_map_mem

#define RA01_RS_MCS_CPM_RX_SA_PLCY_MEM_SIZE                                 64    // Number of elements in sa_plcy_mem
#define RA01_RS_MCS_CPM_RX_SA_PLCY_MEM_WIDTH_IN_BITS                        512   // Bits in each element of sa_plcy_mem

#define RA01_RS_MCS_CPM_RX_SA_PN_TABLE_MEM_SIZE                             64    // Number of elements in sa_pn_table_mem
#define RA01_RS_MCS_CPM_RX_SA_PN_TABLE_MEM_WIDTH_IN_BITS                    64    // Bits in each element of sa_pn_table_mem

#define RA01_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_SIZE                            32    // Number of elements in flowid_tcam_data
#define RA01_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_WIDTH_IN_BITS                   203   // Bits in each element of flowid_tcam_data

#define RA01_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_SIZE                            32    // Number of elements in flowid_tcam_mask
#define RA01_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_WIDTH_IN_BITS                   203   // Bits in each element of flowid_tcam_mask

#define RA01_RS_MCS_CPM_RX_SC_CAM_SIZE                                      32    // Number of elements in sc_cam
#define RA01_RS_MCS_CPM_RX_SC_CAM_WIDTH_IN_BITS                             69    // Bits in each element of sc_cam

#define RA01_RS_MCS_CPM_RX_SC_TIMER_MEM_SIZE                                32    // Number of elements in sc_timer_mem
#define RA01_RS_MCS_CPM_RX_SC_TIMER_MEM_WIDTH_IN_BITS                       37    // Bits in each element of sc_timer_mem

/*
 * From rs_mcs_cpm_tx_slave
 */
#define RA01_RS_MCS_CPM_TX_SECY_MAP_MEM_SIZE                                32    // Number of elements in secy_map_mem
#define RA01_RS_MCS_CPM_TX_SECY_MAP_MEM_WIDTH_IN_BITS                       12    // Bits in each element of secy_map_mem

#define RA01_RS_MCS_CPM_TX_SECY_PLCY_MEM_SIZE                               32    // Number of elements in secy_plcy_mem
#define RA01_RS_MCS_CPM_TX_SECY_PLCY_MEM_WIDTH_IN_BITS                      60    // Bits in each element of secy_plcy_mem

#define RA01_RS_MCS_CPM_TX_SA_MAP_MEM_SIZE                                  32    // Number of elements in sa_map_mem
#define RA01_RS_MCS_CPM_TX_SA_MAP_MEM_WIDTH_IN_BITS                         80    // Bits in each element of sa_map_mem

#define RA01_RS_MCS_CPM_TX_SA_PLCY_MEM_SIZE                                 64    // Number of elements in sa_plcy_mem
#define RA01_RS_MCS_CPM_TX_SA_PLCY_MEM_WIDTH_IN_BITS                        514   // Bits in each element of sa_plcy_mem

#define RA01_RS_MCS_CPM_TX_SA_PN_TABLE_MEM_SIZE                             64    // Number of elements in sa_pn_table_mem
#define RA01_RS_MCS_CPM_TX_SA_PN_TABLE_MEM_WIDTH_IN_BITS                    64    // Bits in each element of sa_pn_table_mem

#define RA01_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_SIZE                            32    // Number of elements in flowid_tcam_data
#define RA01_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_WIDTH_IN_BITS                   203   // Bits in each element of flowid_tcam_data

#define RA01_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_SIZE                            32    // Number of elements in flowid_tcam_mask
#define RA01_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_WIDTH_IN_BITS                   203   // Bits in each element of flowid_tcam_mask

#define RA01_RS_MCS_CPM_TX_SC_TIMER_MEM_SIZE                                32    // Number of elements in sc_timer_mem
#define RA01_RS_MCS_CPM_TX_SC_TIMER_MEM_WIDTH_IN_BITS                       39    // Bits in each element of sc_timer_mem

/* @} */

//-------------------------------------------------------------------------------
// MEM field Access
//-------------------------------------------------------------------------------

/* @brief Memory access for IFINUNCTLOCTETS
 *
 * This block is the same size as IFINUNCTLOCTETS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_ifinunctloctets_t[8];
/* @brief Memory access for IFINCTLOCTETS
 *
 * This block is the same size as IFINCTLOCTETS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_ifinctloctets_t[8];
/* @brief Memory access for IFINUNCTLUCPKTS
 *
 * This block is the same size as IFINUNCTLUCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_ifinunctlucpkts_t[8];
/* @brief Memory access for IFINUNCTLMCPKTS
 *
 * This block is the same size as IFINUNCTLMCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_ifinunctlmcpkts_t[8];
/* @brief Memory access for IFINUNCTLBCPKTS
 *
 * This block is the same size as IFINUNCTLBCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_ifinunctlbcpkts_t[8];
/* @brief Memory access for IFINCTLUCPKTS
 *
 * This block is the same size as IFINCTLUCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_ifinctlucpkts_t[8];
/* @brief Memory access for IFINCTLMCPKTS
 *
 * This block is the same size as IFINCTLMCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_ifinctlmcpkts_t[8];
/* @brief Memory access for IFINCTLBCPKTS
 *
 * This block is the same size as IFINCTLBCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_ifinctlbcpkts_t[8];
/* @brief Memory access for INPKTSSECYUNTAGGEDORNOTAG
 *
 * This block is the same size as INPKTSSECYUNTAGGEDORNOTAG and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssecyuntaggedornotag_t[8];
/* @brief Memory access for INPKTSSECYBADTAG
 *
 * This block is the same size as INPKTSSECYBADTAG and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssecybadtag_t[8];
/* @brief Memory access for INPKTSSECYCTL
 *
 * This block is the same size as INPKTSSECYCTL and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssecyctl_t[8];
/* @brief Memory access for INPKTSSECYTAGGEDCTL
 *
 * This block is the same size as INPKTSSECYTAGGEDCTL and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssecytaggedctl_t[8];
/* @brief Memory access for INPKTSSECYUNKNOWNSCI
 *
 * This block is the same size as INPKTSSECYUNKNOWNSCI and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssecyunknownsci_t[8];
/* @brief Memory access for INPKTSSECYNOSCI
 *
 * This block is the same size as INPKTSSECYNOSCI and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssecynosci_t[8];
/* @brief Memory access for INPKTSCTRLPORTDISABLED
 *
 * This block is the same size as INPKTSCTRLPORTDISABLED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktsctrlportdisabled_t[8];
/* @brief Memory access for INOCTETSSCVALIDATE
 *
 * This block is the same size as INOCTETSSCVALIDATE and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inoctetsscvalidate_t[8];
/* @brief Memory access for INOCTETSSCDECRYPTED
 *
 * This block is the same size as INOCTETSSCDECRYPTED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inoctetsscdecrypted_t[8];
/* @brief Memory access for INPKTSSCUNCHECKED
 *
 * This block is the same size as INPKTSSCUNCHECKED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktsscunchecked_t[8];
/* @brief Memory access for INPKTSSCLATEORDELAYED
 *
 * This block is the same size as INPKTSSCLATEORDELAYED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssclateordelayed_t[8];
/* @brief Memory access for INPKTSSCCAMHIT
 *
 * This block is the same size as INPKTSSCCAMHIT and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssccamhit_t[8];
/* @brief Memory access for INPKTSSAOK
 *
 * This block is the same size as INPKTSSAOK and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssaok_t[8];
/* @brief Memory access for INPKTSSAINVALID
 *
 * This block is the same size as INPKTSSAINVALID and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssainvalid_t[8];
/* @brief Memory access for INPKTSSANOTVALID
 *
 * This block is the same size as INPKTSSANOTVALID and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssanotvalid_t[8];
/* @brief Memory access for INPKTSSAUNUSEDSA
 *
 * This block is the same size as INPKTSSAUNUSEDSA and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssaunusedsa_t[8];
/* @brief Memory access for INPKTSSANOTUSINGSAERROR
 *
 * This block is the same size as INPKTSSANOTUSINGSAERROR and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktssanotusingsaerror_t[8];
/* @brief Memory access for INPKTSFLOWIDTCAMMISS
 *
 * This block is the same size as INPKTSFLOWIDTCAMMISS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktsflowidtcammiss_t[8];
/* @brief Memory access for INPKTSPARSEERR
 *
 * This block is the same size as INPKTSPARSEERR and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktsparseerr_t[8];
/* @brief Memory access for INPKTSEARLYPREEMPTERR
 *
 * This block is the same size as INPKTSEARLYPREEMPTERR and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktsearlypreempterr_t[8];
/* @brief Memory access for INPKTSFLOWIDTCAMHIT
 *
 * This block is the same size as INPKTSFLOWIDTCAMHIT and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_rx_mem_inpktsflowidtcamhit_t[8];
/* @brief Memory access for IFOUTCOMMONOCTETS
 *
 * This block is the same size as IFOUTCOMMONOCTETS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_ifoutcommonoctets_t[8];
/* @brief Memory access for IFOUTUNCTLOCTETS
 *
 * This block is the same size as IFOUTUNCTLOCTETS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_ifoutunctloctets_t[8];
/* @brief Memory access for IFOUTCTLOCTETS
 *
 * This block is the same size as IFOUTCTLOCTETS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_ifoutctloctets_t[8];
/* @brief Memory access for IFOUTUNCTLUCPKTS
 *
 * This block is the same size as IFOUTUNCTLUCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_ifoutunctlucpkts_t[8];
/* @brief Memory access for IFOUTUNCTLMCPKTS
 *
 * This block is the same size as IFOUTUNCTLMCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_ifoutunctlmcpkts_t[8];
/* @brief Memory access for IFOUTUNCTLBCPKTS
 *
 * This block is the same size as IFOUTUNCTLBCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_ifoutunctlbcpkts_t[8];
/* @brief Memory access for IFOUTCTLUCPKTS
 *
 * This block is the same size as IFOUTCTLUCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_ifoutctlucpkts_t[8];
/* @brief Memory access for IFOUTCTLMCPKTS
 *
 * This block is the same size as IFOUTCTLMCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_ifoutctlmcpkts_t[8];
/* @brief Memory access for IFOUTCTLBCPKTS
 *
 * This block is the same size as IFOUTCTLBCPKTS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_ifoutctlbcpkts_t[8];
/* @brief Memory access for OUTPKTSSECYUNTAGGED
 *
 * This block is the same size as OUTPKTSSECYUNTAGGED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktssecyuntagged_t[8];
/* @brief Memory access for OUTPKTSSECYTOOLONG
 *
 * This block is the same size as OUTPKTSSECYTOOLONG and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktssecytoolong_t[8];
/* @brief Memory access for OUTPKTSSECYNOACTIVESA
 *
 * This block is the same size as OUTPKTSSECYNOACTIVESA and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktssecynoactivesa_t[8];
/* @brief Memory access for OUTPKTSCTRLPORTDISABLED
 *
 * This block is the same size as OUTPKTSCTRLPORTDISABLED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktsctrlportdisabled_t[8];
/* @brief Memory access for OUTOCTETSSCPROTECTED
 *
 * This block is the same size as OUTOCTETSSCPROTECTED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outoctetsscprotected_t[8];
/* @brief Memory access for OUTOCTETSSCENCRYPTED
 *
 * This block is the same size as OUTOCTETSSCENCRYPTED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outoctetsscencrypted_t[8];
/* @brief Memory access for OUTPKTSSAPROTECTED
 *
 * This block is the same size as OUTPKTSSAPROTECTED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktssaprotected_t[8];
/* @brief Memory access for OUTPKTSSAENCRYPTED
 *
 * This block is the same size as OUTPKTSSAENCRYPTED and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktssaencrypted_t[8];
/* @brief Memory access for OUTPKTSFLOWIDTCAMMISS
 *
 * This block is the same size as OUTPKTSFLOWIDTCAMMISS and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktsflowidtcammiss_t[8];
/* @brief Memory access for OUTPKTSPARSEERR
 *
 * This block is the same size as OUTPKTSPARSEERR and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktsparseerr_t[8];
/* @brief Memory access for OUTPKTSSECTAGINSERTIONERR
 *
 * This block is the same size as OUTPKTSSECTAGINSERTIONERR and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktssectaginsertionerr_t[8];
/* @brief Memory access for OUTPKTSEARLYPREEMPTERR
 *
 * This block is the same size as OUTPKTSEARLYPREEMPTERR and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktsearlypreempterr_t[8];
/* @brief Memory access for OUTPKTSFLOWIDTCAMHIT
 *
 * This block is the same size as OUTPKTSFLOWIDTCAMHIT and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cse_tx_mem_outpktsflowidtcamhit_t[8];
/* @brief Memory access for secy_map_mem
 *
 * This block is the same size as secy_map_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_rx_secy_map_mem_t[8];
/* @brief Memory access for secy_plcy_mem
 *
 * This block is the same size as secy_plcy_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_rx_secy_plcy_mem_t[16];
/* @brief Memory access for sa_map_mem
 *
 * This block is the same size as sa_map_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_rx_sa_map_mem_t[8];
/* @brief Memory access for sa_plcy_mem
 *
 * This block is the same size as sa_plcy_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_rx_sa_plcy_mem_t[64];
/* @brief Memory access for sa_pn_table_mem
 *
 * This block is the same size as sa_pn_table_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_rx_sa_pn_table_mem_t[8];
/* @brief Memory access for flowid_tcam_data
 *
 * This block is the same size as flowid_tcam_data and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_rx_flowid_tcam_data_t[32];
/* @brief Memory access for flowid_tcam_mask
 *
 * This block is the same size as flowid_tcam_mask and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_rx_flowid_tcam_mask_t[32];
/* @brief Memory access for sc_cam
 *
 * This block is the same size as sc_cam and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_rx_sc_cam_t[16];
/* @brief Memory access for sc_timer_mem
 *
 * This block is the same size as sc_timer_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_rx_sc_timer_mem_t[8];
/* @brief Memory access for secy_map_mem
 *
 * This block is the same size as secy_map_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_tx_secy_map_mem_t[8];
/* @brief Memory access for secy_plcy_mem
 *
 * This block is the same size as secy_plcy_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_tx_secy_plcy_mem_t[8];
/* @brief Memory access for sa_map_mem
 *
 * This block is the same size as sa_map_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_tx_sa_map_mem_t[16];
/* @brief Memory access for sa_plcy_mem
 *
 * This block is the same size as sa_plcy_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_tx_sa_plcy_mem_t[128];
/* @brief Memory access for sa_pn_table_mem
 *
 * This block is the same size as sa_pn_table_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_tx_sa_pn_table_mem_t[8];
/* @brief Memory access for flowid_tcam_data
 *
 * This block is the same size as flowid_tcam_data and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_tx_flowid_tcam_data_t[32];
/* @brief Memory access for flowid_tcam_mask
 *
 * This block is the same size as flowid_tcam_mask and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_tx_flowid_tcam_mask_t[32];
/* @brief Memory access for sc_timer_mem
 *
 * This block is the same size as sc_timer_mem and is used to facilitate access.
 *
 */
typedef uint8_t ra01_rs_mcs_cpm_tx_sc_timer_mem_t[8];



/* @brief Enumerate the rs_mcs_top_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_TOP_SLAVE = 0,  // Instance 0 of rs_mcs_top_slave

    RA01_RS_MCS_TOP_INSTANCE_MAX
} ra01_rs_mcs_top_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_top_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_top_slave_instance_t Ra01RsMcsTopInstance_t;




/* @brief version bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave version 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint8_t    major;                                   // @sa RA01_BF_RS_MCS_TOP_VERSION_MAJOR
    uint8_t    minor;                                   // @sa RA01_BF_RS_MCS_TOP_VERSION_MINOR
    uint8_t    patch;                                   // @sa RA01_BF_RS_MCS_TOP_VERSION_PATCH
} Ra01RsMcsTopVersion_t;

/* @brief Set the value of rs_mcs_top_slave version
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * version are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01GetRsMcsTopVersion( RmsDev_t * rmsDev_p, 
  Ra01RsMcsTopVersion_t * value);







/* @brief port_reset bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave port_reset 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    bool    portReset;                               // @sa RA01_BF_RS_MCS_TOP_PORT_RESET_PORT_RESET
} Ra01RsMcsTopPortReset_t;

/* @brief Set the value of rs_mcs_top_slave port_reset
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * port_reset are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01AccRsMcsTopPortReset( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsTopPortReset_t * value,
  ra01_reg_access_op_t op);







/* @brief port_config bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave port_config 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint8_t    parseDepth;                              // @sa RA01_BF_RS_MCS_TOP_PORT_CONFIG_PARSE_DEPTH
} Ra01RsMcsTopPortConfig_t;

/* @brief Set the value of rs_mcs_top_slave port_config
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * port_config are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01AccRsMcsTopPortConfig( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsTopPortConfig_t * value,
  ra01_reg_access_op_t op);







/* @brief channel_config bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave channel_config 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    bool    chBypass;                                // @sa RA01_BF_RS_MCS_TOP_CHANNEL_CONFIG_CH_BYPASS
} Ra01RsMcsTopChannelConfig_t;

/* @brief Set the value of rs_mcs_top_slave channel_config
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * channel_config are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01AccRsMcsTopChannelConfig( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsTopChannelConfig_t * value,
  ra01_reg_access_op_t op);











/* @brief mcs_bp_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave mcs_bp_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint8_t    rxPolicyFifoCredits;                     // @sa RA01_BF_RS_MCS_TOP_MCS_BP_CFG_RX_POLICY_FIFO_CREDITS
    uint8_t    rxDataFifoCredits;                       // @sa RA01_BF_RS_MCS_TOP_MCS_BP_CFG_RX_DATA_FIFO_CREDITS
    uint8_t    txPolicyFifoCredits;                     // @sa RA01_BF_RS_MCS_TOP_MCS_BP_CFG_TX_POLICY_FIFO_CREDITS
    uint8_t    txDataFifoCredits;                       // @sa RA01_BF_RS_MCS_TOP_MCS_BP_CFG_TX_DATA_FIFO_CREDITS
} Ra01RsMcsTopMcsBpCfg_t;

/* @brief Set the value of rs_mcs_top_slave mcs_bp_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * mcs_bp_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01AccRsMcsTopMcsBpCfg( RmsDev_t * rmsDev_p, 
  Ra01RsMcsTopMcsBpCfg_t * value,
  ra01_reg_access_op_t op);







/* @brief output_adapter_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave output_adapter_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint8_t    rxAfullThreshold;                        // @sa RA01_BF_RS_MCS_TOP_OUTPUT_ADAPTER_CFG_RX_AFULL_THRESHOLD
    uint8_t    txAfullThreshold;                        // @sa RA01_BF_RS_MCS_TOP_OUTPUT_ADAPTER_CFG_TX_AFULL_THRESHOLD
} Ra01RsMcsTopOutputAdapterCfg_t;

/* @brief Set the value of rs_mcs_top_slave output_adapter_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * output_adapter_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01AccRsMcsTopOutputAdapterCfg( RmsDev_t * rmsDev_p, 
  Ra01RsMcsTopOutputAdapterCfg_t * value,
  ra01_reg_access_op_t op);







/* @brief scratch bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave scratch 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint32_t    scratch;                                 // @sa RA01_BF_RS_MCS_TOP_SCRATCH_SCRATCH
} Ra01RsMcsTopScratch_t;

/* @brief Set the value of rs_mcs_top_slave scratch
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * scratch are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01AccRsMcsTopScratch( RmsDev_t * rmsDev_p, 
  Ra01RsMcsTopScratch_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief dbg_mux_sel bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave dbg_mux_sel 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint8_t    seg0;                                    // @sa RA01_BF_RS_MCS_TOP_DBG_MUX_SEL_SEG0
    uint8_t    seg1;                                    // @sa RA01_BF_RS_MCS_TOP_DBG_MUX_SEL_SEG1
    uint8_t    seg2;                                    // @sa RA01_BF_RS_MCS_TOP_DBG_MUX_SEL_SEG2
    uint8_t    seg3;                                    // @sa RA01_BF_RS_MCS_TOP_DBG_MUX_SEL_SEG3
    bool    en;                                      // @sa RA01_BF_RS_MCS_TOP_DBG_MUX_SEL_EN
} Ra01RsMcsTopDbgMuxSel_t;

/* @brief Set the value of rs_mcs_top_slave dbg_mux_sel
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * dbg_mux_sel are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01AccRsMcsTopDbgMuxSel( RmsDev_t * rmsDev_p, 
  Ra01RsMcsTopDbgMuxSel_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief rs_mcs_top_slave_int_sum bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave rs_mcs_top_slave_int_sum 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint8_t    rsMcsBbeSlaveInterrupt;                  // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RS_MCS_BBE_SLAVE_INTERRUPT
    bool    rsMcsCpmRxSlaveInterrupt;                // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RS_MCS_CPM_RX_SLAVE_INTERRUPT
    bool    rsMcsCpmTxSlaveInterrupt;                // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RS_MCS_CPM_TX_SLAVE_INTERRUPT
    uint8_t    rsMcsPabSlaveInterrupt;                  // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RS_MCS_PAB_SLAVE_INTERRUPT
    uint8_t    rsMcsPexSlaveInterrupt;                  // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RS_MCS_PEX_SLAVE_INTERRUPT
} Ra01RsMcsTopRsMcsTopSlaveIntSum_t;

/* @brief Set the value of rs_mcs_top_slave rs_mcs_top_slave_int_sum
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * rs_mcs_top_slave_int_sum are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01GetRsMcsTopRsMcsTopSlaveIntSum( RmsDev_t * rmsDev_p, 
  Ra01RsMcsTopRsMcsTopSlaveIntSum_t * value);







/* @brief rs_mcs_top_slave_int_sum_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave rs_mcs_top_slave_int_sum_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint8_t    rsMcsBbeSlaveInterruptEnb;               // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_ENB_RS_MCS_BBE_SLAVE_INTERRUPT_ENB
    bool    rsMcsCpmRxSlaveInterruptEnb;             // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_ENB_RS_MCS_CPM_RX_SLAVE_INTERRUPT_ENB
    bool    rsMcsCpmTxSlaveInterruptEnb;             // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_ENB_RS_MCS_CPM_TX_SLAVE_INTERRUPT_ENB
    uint8_t    rsMcsPabSlaveInterruptEnb;               // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_ENB_RS_MCS_PAB_SLAVE_INTERRUPT_ENB
    uint8_t    rsMcsPexSlaveInterruptEnb;               // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_ENB_RS_MCS_PEX_SLAVE_INTERRUPT_ENB
} Ra01RsMcsTopRsMcsTopSlaveIntSumEnb_t;

/* @brief Set the value of rs_mcs_top_slave rs_mcs_top_slave_int_sum_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * rs_mcs_top_slave_int_sum_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01AccRsMcsTopRsMcsTopSlaveIntSumEnb( RmsDev_t * rmsDev_p, 
  Ra01RsMcsTopRsMcsTopSlaveIntSumEnb_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief rs_mcs_top_slave_int_sum_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave rs_mcs_top_slave_int_sum_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint8_t    rsMcsBbeSlaveInterrupt;                  // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RAW_RS_MCS_BBE_SLAVE_INTERRUPT
    bool    rsMcsCpmRxSlaveInterrupt;                // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RAW_RS_MCS_CPM_RX_SLAVE_INTERRUPT
    bool    rsMcsCpmTxSlaveInterrupt;                // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RAW_RS_MCS_CPM_TX_SLAVE_INTERRUPT
    uint8_t    rsMcsPabSlaveInterrupt;                  // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RAW_RS_MCS_PAB_SLAVE_INTERRUPT
    uint8_t    rsMcsPexSlaveInterrupt;                  // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_RAW_RS_MCS_PEX_SLAVE_INTERRUPT
} Ra01RsMcsTopRsMcsTopSlaveIntSumRaw_t;

/* @brief Set the value of rs_mcs_top_slave rs_mcs_top_slave_int_sum_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * rs_mcs_top_slave_int_sum_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01GetRsMcsTopRsMcsTopSlaveIntSumRaw( RmsDev_t * rmsDev_p, 
  Ra01RsMcsTopRsMcsTopSlaveIntSumRaw_t * value);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief rs_mcs_top_slave_int_sum_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_top_slave rs_mcs_top_slave_int_sum_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 *
 */
typedef struct {
    uint8_t    rsMcsBbeSlaveInterrupt;                  // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_INTR_RW_RS_MCS_BBE_SLAVE_INTERRUPT
    bool    rsMcsCpmRxSlaveInterrupt;                // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_INTR_RW_RS_MCS_CPM_RX_SLAVE_INTERRUPT
    bool    rsMcsCpmTxSlaveInterrupt;                // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_INTR_RW_RS_MCS_CPM_TX_SLAVE_INTERRUPT
    uint8_t    rsMcsPabSlaveInterrupt;                  // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_INTR_RW_RS_MCS_PAB_SLAVE_INTERRUPT
    uint8_t    rsMcsPexSlaveInterrupt;                  // @sa RA01_BF_RS_MCS_TOP_RS_MCS_TOP_SLAVE_INT_SUM_INTR_RW_RS_MCS_PEX_SLAVE_INTERRUPT
} Ra01RsMcsTopRsMcsTopSlaveIntSumIntrRw_t;

/* @brief Set the value of rs_mcs_top_slave rs_mcs_top_slave_int_sum_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * rs_mcs_top_slave_int_sum_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_top_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_top_slave
 * 
*/
int Ra01AccRsMcsTopRsMcsTopSlaveIntSumIntrRw( RmsDev_t * rmsDev_p, 
  Ra01RsMcsTopRsMcsTopSlaveIntSumIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


/* @brief Enumerate the rs_mcs_gae_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_GAE_RX_SLAVE = 0,  // Instance 0 of rs_mcs_gae_slave
    RA01_RS_MCS_GAE_TX_SLAVE = 1,  // Instance 1 of rs_mcs_gae_slave

    RA01_RS_MCS_GAE_INSTANCE_MAX
} ra01_rs_mcs_gae_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_gae_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_gae_slave_instance_t Ra01RsMcsGaeInstance_t;




/* @brief fips_reset bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_reset 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    bool    fipsReset;                               // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESET_FIPS_RESET, RA01_BF_RS_MCS_GAE_TX_FIPS_RESET_FIPS_RESET
} Ra01RsMcsGaeFipsReset_t;

/* @brief Set the value of rs_mcs_gae_slave fips_reset
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_reset are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsReset( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsReset_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_mode bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_mode 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    bool    ctrMode;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_MODE_CTR_MODE, RA01_BF_RS_MCS_GAE_TX_FIPS_MODE_CTR_MODE
    bool    keylen;                                  // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_MODE_KEYLEN, RA01_BF_RS_MCS_GAE_TX_FIPS_MODE_KEYLEN
    uint8_t    channel;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_MODE_CHANNEL, RA01_BF_RS_MCS_GAE_TX_FIPS_MODE_CHANNEL
    bool    ecb;                                     // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_MODE_ECB, RA01_BF_RS_MCS_GAE_TX_FIPS_MODE_ECB
} Ra01RsMcsGaeFipsMode_t;

/* @brief Set the value of rs_mcs_gae_slave fips_mode
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_mode are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsMode( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsMode_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_ctl bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_ctl 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    bool    nextIcv;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_CTL_NEXT_ICV, RA01_BF_RS_MCS_GAE_TX_FIPS_CTL_NEXT_ICV
    bool    lenRound;                                // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_CTL_LEN_ROUND, RA01_BF_RS_MCS_GAE_TX_FIPS_CTL_LEN_ROUND
    bool    blockIsAad;                              // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_CTL_BLOCK_IS_AAD, RA01_BF_RS_MCS_GAE_TX_FIPS_CTL_BLOCK_IS_AAD
    bool    eop;                                     // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_CTL_EOP, RA01_BF_RS_MCS_GAE_TX_FIPS_CTL_EOP
    uint8_t    blockSize;                               // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_CTL_BLOCK_SIZE, RA01_BF_RS_MCS_GAE_TX_FIPS_CTL_BLOCK_SIZE
    bool    vld;                                     // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_CTL_VLD, RA01_BF_RS_MCS_GAE_TX_FIPS_CTL_VLD
} Ra01RsMcsGaeFipsCtl_t;

/* @brief Set the value of rs_mcs_gae_slave fips_ctl
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_ctl are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsCtl( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsCtl_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_iv_bits95_64 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_iv_bits95_64 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    data;                                    // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_IV_BITS95_64_DATA, RA01_BF_RS_MCS_GAE_TX_FIPS_IV_BITS95_64_DATA
} Ra01RsMcsGaeFipsIvBits9564_t;

/* @brief Set the value of rs_mcs_gae_slave fips_iv_bits95_64
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_iv_bits95_64 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsIvBits9564( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsIvBits9564_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_iv_bits63_0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_iv_bits63_0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_IV_BITS63_0_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_IV_BITS63_0_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_IV_BITS63_0_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_IV_BITS63_0_DATA_MSB
} Ra01RsMcsGaeFipsIvBits630_t;

/* @brief Set the value of rs_mcs_gae_slave fips_iv_bits63_0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_iv_bits63_0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsIvBits630( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsIvBits630_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_ctr bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_ctr 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    fipsCtr;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_CTR_FIPS_CTR, RA01_BF_RS_MCS_GAE_TX_FIPS_CTR_FIPS_CTR
} Ra01RsMcsGaeFipsCtr_t;

/* @brief Set the value of rs_mcs_gae_slave fips_ctr
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_ctr are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsCtr( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsCtr_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_sak_bits255_192 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_sak_bits255_192 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_SAK_BITS255_192_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_SAK_BITS255_192_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_SAK_BITS255_192_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_SAK_BITS255_192_DATA_MSB
} Ra01RsMcsGaeFipsSakBits255192_t;

/* @brief Set the value of rs_mcs_gae_slave fips_sak_bits255_192
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_sak_bits255_192 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsSakBits255192( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsSakBits255192_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_sak_bits191_128 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_sak_bits191_128 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_SAK_BITS191_128_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_SAK_BITS191_128_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_SAK_BITS191_128_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_SAK_BITS191_128_DATA_MSB
} Ra01RsMcsGaeFipsSakBits191128_t;

/* @brief Set the value of rs_mcs_gae_slave fips_sak_bits191_128
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_sak_bits191_128 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsSakBits191128( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsSakBits191128_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_sak_bits127_64 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_sak_bits127_64 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_SAK_BITS127_64_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_SAK_BITS127_64_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_SAK_BITS127_64_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_SAK_BITS127_64_DATA_MSB
} Ra01RsMcsGaeFipsSakBits12764_t;

/* @brief Set the value of rs_mcs_gae_slave fips_sak_bits127_64
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_sak_bits127_64 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsSakBits12764( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsSakBits12764_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_sak_bits63_0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_sak_bits63_0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_SAK_BITS63_0_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_SAK_BITS63_0_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_SAK_BITS63_0_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_SAK_BITS63_0_DATA_MSB
} Ra01RsMcsGaeFipsSakBits630_t;

/* @brief Set the value of rs_mcs_gae_slave fips_sak_bits63_0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_sak_bits63_0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsSakBits630( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsSakBits630_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_hashkey_bits127_64 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_hashkey_bits127_64 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_HASHKEY_BITS127_64_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_HASHKEY_BITS127_64_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_HASHKEY_BITS127_64_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_HASHKEY_BITS127_64_DATA_MSB
} Ra01RsMcsGaeFipsHashkeyBits12764_t;

/* @brief Set the value of rs_mcs_gae_slave fips_hashkey_bits127_64
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_hashkey_bits127_64 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsHashkeyBits12764( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsHashkeyBits12764_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_hashkey_bits63_0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_hashkey_bits63_0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_HASHKEY_BITS63_0_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_HASHKEY_BITS63_0_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_HASHKEY_BITS63_0_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_HASHKEY_BITS63_0_DATA_MSB
} Ra01RsMcsGaeFipsHashkeyBits630_t;

/* @brief Set the value of rs_mcs_gae_slave fips_hashkey_bits63_0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_hashkey_bits63_0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsHashkeyBits630( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsHashkeyBits630_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_block_bits127_64 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_block_bits127_64 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_BLOCK_BITS127_64_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_BLOCK_BITS127_64_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_BLOCK_BITS127_64_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_BLOCK_BITS127_64_DATA_MSB
} Ra01RsMcsGaeFipsBlockBits12764_t;

/* @brief Set the value of rs_mcs_gae_slave fips_block_bits127_64
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_block_bits127_64 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsBlockBits12764( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsBlockBits12764_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_block_bits63_0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_block_bits63_0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_BLOCK_BITS63_0_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_BLOCK_BITS63_0_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_BLOCK_BITS63_0_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_BLOCK_BITS63_0_DATA_MSB
} Ra01RsMcsGaeFipsBlockBits630_t;

/* @brief Set the value of rs_mcs_gae_slave fips_block_bits63_0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_block_bits63_0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsBlockBits630( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsBlockBits630_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_start bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_start 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    bool    fipsStart;                               // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_START_FIPS_START, RA01_BF_RS_MCS_GAE_TX_FIPS_START_FIPS_START
} Ra01RsMcsGaeFipsStart_t;

/* @brief Set the value of rs_mcs_gae_slave fips_start
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_start are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeFipsStart( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsStart_t * value,
  ra01_reg_access_op_t op);







/* @brief fips_result_block_bits127_64 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_result_block_bits127_64 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESULT_BLOCK_BITS127_64_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_RESULT_BLOCK_BITS127_64_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESULT_BLOCK_BITS127_64_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_RESULT_BLOCK_BITS127_64_DATA_MSB
} Ra01RsMcsGaeFipsResultBlockBits12764_t;

/* @brief Set the value of rs_mcs_gae_slave fips_result_block_bits127_64
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_result_block_bits127_64 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01GetRsMcsGaeFipsResultBlockBits12764( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsResultBlockBits12764_t * value);







/* @brief fips_result_block_bits63_0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_result_block_bits63_0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESULT_BLOCK_BITS63_0_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_RESULT_BLOCK_BITS63_0_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESULT_BLOCK_BITS63_0_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_RESULT_BLOCK_BITS63_0_DATA_MSB
} Ra01RsMcsGaeFipsResultBlockBits630_t;

/* @brief Set the value of rs_mcs_gae_slave fips_result_block_bits63_0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_result_block_bits63_0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01GetRsMcsGaeFipsResultBlockBits630( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsResultBlockBits630_t * value);







/* @brief fips_result_pass bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_result_pass 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    bool    fipsResultPass;                          // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESULT_PASS_FIPS_RESULT_PASS, RA01_BF_RS_MCS_GAE_TX_FIPS_RESULT_PASS_FIPS_RESULT_PASS
} Ra01RsMcsGaeFipsResultPass_t;

/* @brief Set the value of rs_mcs_gae_slave fips_result_pass
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_result_pass are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01GetRsMcsGaeFipsResultPass( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsResultPass_t * value);







/* @brief fips_result_icv_bits127_64 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_result_icv_bits127_64 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESULT_ICV_BITS127_64_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_RESULT_ICV_BITS127_64_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESULT_ICV_BITS127_64_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_RESULT_ICV_BITS127_64_DATA_MSB
} Ra01RsMcsGaeFipsResultIcvBits12764_t;

/* @brief Set the value of rs_mcs_gae_slave fips_result_icv_bits127_64
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_result_icv_bits127_64 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01GetRsMcsGaeFipsResultIcvBits12764( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsResultIcvBits12764_t * value);







/* @brief fips_result_icv_bits63_0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave fips_result_icv_bits63_0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint32_t    dataLsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESULT_ICV_BITS63_0_DATA_LSB, RA01_BF_RS_MCS_GAE_TX_FIPS_RESULT_ICV_BITS63_0_DATA_LSB
    uint32_t    dataMsb;                                 // @sa RA01_BF_RS_MCS_GAE_RX_FIPS_RESULT_ICV_BITS63_0_DATA_MSB, RA01_BF_RS_MCS_GAE_TX_FIPS_RESULT_ICV_BITS63_0_DATA_MSB
} Ra01RsMcsGaeFipsResultIcvBits630_t;

/* @brief Set the value of rs_mcs_gae_slave fips_result_icv_bits63_0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fips_result_icv_bits63_0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01GetRsMcsGaeFipsResultIcvBits630( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeFipsResultIcvBits630_t * value);




#ifdef MCS_API_DEBUG



/* @brief dbg_mux_sel bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave dbg_mux_sel 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint8_t    seg0;                                    // @sa RA01_BF_RS_MCS_GAE_RX_DBG_MUX_SEL_SEG0, RA01_BF_RS_MCS_GAE_TX_DBG_MUX_SEL_SEG0
    uint8_t    seg1;                                    // @sa RA01_BF_RS_MCS_GAE_RX_DBG_MUX_SEL_SEG1, RA01_BF_RS_MCS_GAE_TX_DBG_MUX_SEL_SEG1
    uint8_t    seg2;                                    // @sa RA01_BF_RS_MCS_GAE_RX_DBG_MUX_SEL_SEG2, RA01_BF_RS_MCS_GAE_TX_DBG_MUX_SEL_SEG2
    uint8_t    seg3;                                    // @sa RA01_BF_RS_MCS_GAE_RX_DBG_MUX_SEL_SEG3, RA01_BF_RS_MCS_GAE_TX_DBG_MUX_SEL_SEG3
} Ra01RsMcsGaeDbgMuxSel_t;

/* @brief Set the value of rs_mcs_gae_slave dbg_mux_sel
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * dbg_mux_sel are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01AccRsMcsGaeDbgMuxSel( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeDbgMuxSel_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief debug_status bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_gae_slave debug_status 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_GAE_RX_DEBUG_STATUS_SEG0, RA01_BF_RS_MCS_GAE_TX_DEBUG_STATUS_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_GAE_RX_DEBUG_STATUS_SEG1, RA01_BF_RS_MCS_GAE_TX_DEBUG_STATUS_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_GAE_RX_DEBUG_STATUS_SEG2, RA01_BF_RS_MCS_GAE_TX_DEBUG_STATUS_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_GAE_RX_DEBUG_STATUS_SEG3, RA01_BF_RS_MCS_GAE_TX_DEBUG_STATUS_SEG3
} Ra01RsMcsGaeDebugStatus_t;

/* @brief Set the value of rs_mcs_gae_slave debug_status
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_gae_rx_slave, rs_mcs_gae_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * debug_status are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_gae_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_gae_slave
 * 
*/
int Ra01GetRsMcsGaeDebugStatus( RmsDev_t * rmsDev_p, 
    Ra01RsMcsGaeInstance_t instance, 
  Ra01RsMcsGaeDebugStatus_t * value);


#endif // MCS_API_DEBUG


/* @brief Enumerate the rs_mcs_bbe_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_BBE_RX_SLAVE = 0,  // Instance 0 of rs_mcs_bbe_slave
    RA01_RS_MCS_BBE_TX_SLAVE = 1,  // Instance 1 of rs_mcs_bbe_slave

    RA01_RS_MCS_BBE_INSTANCE_MAX
} ra01_rs_mcs_bbe_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_bbe_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_bbe_slave_instance_t Ra01RsMcsBbeInstance_t;




/* @brief bbe_int bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_bbe_slave bbe_int 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 *
 */
typedef struct {
    bool    dfifoOverflow;                           // @sa RA01_BF_RS_MCS_BBE_RX_BBE_INT_DFIFO_OVERFLOW, RA01_BF_RS_MCS_BBE_TX_BBE_INT_DFIFO_OVERFLOW
    bool    plfifoOverflow;                          // @sa RA01_BF_RS_MCS_BBE_RX_BBE_INT_PLFIFO_OVERFLOW, RA01_BF_RS_MCS_BBE_TX_BBE_INT_PLFIFO_OVERFLOW
} Ra01RsMcsBbeBbeInt_t;

/* @brief Set the value of rs_mcs_bbe_slave bbe_int
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_bbe_rx_slave, rs_mcs_bbe_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * bbe_int are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_bbe_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 * 
*/
int Ra01AccRsMcsBbeBbeInt( RmsDev_t * rmsDev_p, 
    Ra01RsMcsBbeInstance_t instance, 
  Ra01RsMcsBbeBbeInt_t * value,
  ra01_reg_access_op_t op);







/* @brief bbe_int_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_bbe_slave bbe_int_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 *
 */
typedef struct {
    bool    dfifoOverflowEnb;                        // @sa RA01_BF_RS_MCS_BBE_RX_BBE_INT_ENB_DFIFO_OVERFLOW_ENB, RA01_BF_RS_MCS_BBE_TX_BBE_INT_ENB_DFIFO_OVERFLOW_ENB
    bool    plfifoOverflowEnb;                       // @sa RA01_BF_RS_MCS_BBE_RX_BBE_INT_ENB_PLFIFO_OVERFLOW_ENB, RA01_BF_RS_MCS_BBE_TX_BBE_INT_ENB_PLFIFO_OVERFLOW_ENB
} Ra01RsMcsBbeBbeIntEnb_t;

/* @brief Set the value of rs_mcs_bbe_slave bbe_int_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_bbe_rx_slave, rs_mcs_bbe_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * bbe_int_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_bbe_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 * 
*/
int Ra01AccRsMcsBbeBbeIntEnb( RmsDev_t * rmsDev_p, 
    Ra01RsMcsBbeInstance_t instance, 
  Ra01RsMcsBbeBbeIntEnb_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief bbe_int_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_bbe_slave bbe_int_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 *
 */
typedef struct {
    bool    dfifoOverflow;                           // @sa RA01_BF_RS_MCS_BBE_RX_BBE_INT_RAW_DFIFO_OVERFLOW, RA01_BF_RS_MCS_BBE_TX_BBE_INT_RAW_DFIFO_OVERFLOW
    bool    plfifoOverflow;                          // @sa RA01_BF_RS_MCS_BBE_RX_BBE_INT_RAW_PLFIFO_OVERFLOW, RA01_BF_RS_MCS_BBE_TX_BBE_INT_RAW_PLFIFO_OVERFLOW
} Ra01RsMcsBbeBbeIntRaw_t;

/* @brief Set the value of rs_mcs_bbe_slave bbe_int_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_bbe_rx_slave, rs_mcs_bbe_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * bbe_int_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_bbe_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 * 
*/
int Ra01GetRsMcsBbeBbeIntRaw( RmsDev_t * rmsDev_p, 
    Ra01RsMcsBbeInstance_t instance, 
  Ra01RsMcsBbeBbeIntRaw_t * value);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief bbe_int_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_bbe_slave bbe_int_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 *
 */
typedef struct {
    bool    dfifoOverflow;                           // @sa RA01_BF_RS_MCS_BBE_RX_BBE_INT_INTR_RW_DFIFO_OVERFLOW, RA01_BF_RS_MCS_BBE_TX_BBE_INT_INTR_RW_DFIFO_OVERFLOW
    bool    plfifoOverflow;                          // @sa RA01_BF_RS_MCS_BBE_RX_BBE_INT_INTR_RW_PLFIFO_OVERFLOW, RA01_BF_RS_MCS_BBE_TX_BBE_INT_INTR_RW_PLFIFO_OVERFLOW
} Ra01RsMcsBbeBbeIntIntrRw_t;

/* @brief Set the value of rs_mcs_bbe_slave bbe_int_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_bbe_rx_slave, rs_mcs_bbe_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * bbe_int_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_bbe_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 * 
*/
int Ra01AccRsMcsBbeBbeIntIntrRw( RmsDev_t * rmsDev_p, 
    Ra01RsMcsBbeInstance_t instance, 
  Ra01RsMcsBbeBbeIntIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief hw_init bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_bbe_slave hw_init 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 *
 */
typedef struct {
    bool    trigger;                                 // @sa RA01_BF_RS_MCS_BBE_RX_HW_INIT_TRIGGER, RA01_BF_RS_MCS_BBE_TX_HW_INIT_TRIGGER
} Ra01RsMcsBbeHwInit_t;

/* @brief Set the value of rs_mcs_bbe_slave hw_init
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_bbe_rx_slave, rs_mcs_bbe_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * hw_init are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_bbe_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 * 
*/
int Ra01AccRsMcsBbeHwInit( RmsDev_t * rmsDev_p, 
    Ra01RsMcsBbeInstance_t instance, 
  Ra01RsMcsBbeHwInit_t * value,
  ra01_reg_access_op_t op);







/* @brief padding_ctl bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_bbe_slave padding_ctl 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 *
 */
typedef struct {
    uint8_t    minPktSize;                              // @sa RA01_BF_RS_MCS_BBE_RX_PADDING_CTL_MIN_PKT_SIZE, RA01_BF_RS_MCS_BBE_TX_PADDING_CTL_MIN_PKT_SIZE
    bool    paddingEn;                               // @sa RA01_BF_RS_MCS_BBE_RX_PADDING_CTL_PADDING_EN, RA01_BF_RS_MCS_BBE_TX_PADDING_CTL_PADDING_EN
} Ra01RsMcsBbePaddingCtl_t;

/* @brief Set the value of rs_mcs_bbe_slave padding_ctl
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_bbe_rx_slave, rs_mcs_bbe_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * padding_ctl are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_bbe_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 * 
*/
int Ra01AccRsMcsBbePaddingCtl( RmsDev_t * rmsDev_p, 
    Ra01RsMcsBbeInstance_t instance, 
  Ra01RsMcsBbePaddingCtl_t * value,
  ra01_reg_access_op_t op);







/* @brief preempt_filter_credit_corr_count bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_bbe_slave preempt_filter_credit_corr_count 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 *
 */
typedef struct {
    uint8_t    creditCorrCount;                         // @sa RA01_BF_RS_MCS_BBE_RX_PREEMPT_FILTER_CREDIT_CORR_COUNT_CREDIT_CORR_COUNT, RA01_BF_RS_MCS_BBE_TX_PREEMPT_FILTER_CREDIT_CORR_COUNT_CREDIT_CORR_COUNT
} Ra01RsMcsBbePreemptFilterCreditCorrCount_t;

/* @brief Set the value of rs_mcs_bbe_slave preempt_filter_credit_corr_count
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_bbe_rx_slave, rs_mcs_bbe_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * preempt_filter_credit_corr_count are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_bbe_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 * 
*/
int Ra01GetRsMcsBbePreemptFilterCreditCorrCount( RmsDev_t * rmsDev_p, 
    Ra01RsMcsBbeInstance_t instance, 
  unsigned index,
  Ra01RsMcsBbePreemptFilterCreditCorrCount_t * value);




#ifdef MCS_API_DEBUG



/* @brief dbg_mux_sel bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_bbe_slave dbg_mux_sel 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 *
 */
typedef struct {
    uint8_t    seg0;                                    // @sa RA01_BF_RS_MCS_BBE_RX_DBG_MUX_SEL_SEG0, RA01_BF_RS_MCS_BBE_TX_DBG_MUX_SEL_SEG0
    uint8_t    seg1;                                    // @sa RA01_BF_RS_MCS_BBE_RX_DBG_MUX_SEL_SEG1, RA01_BF_RS_MCS_BBE_TX_DBG_MUX_SEL_SEG1
    uint8_t    seg2;                                    // @sa RA01_BF_RS_MCS_BBE_RX_DBG_MUX_SEL_SEG2, RA01_BF_RS_MCS_BBE_TX_DBG_MUX_SEL_SEG2
    uint8_t    seg3;                                    // @sa RA01_BF_RS_MCS_BBE_RX_DBG_MUX_SEL_SEG3, RA01_BF_RS_MCS_BBE_TX_DBG_MUX_SEL_SEG3
} Ra01RsMcsBbeDbgMuxSel_t;

/* @brief Set the value of rs_mcs_bbe_slave dbg_mux_sel
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_bbe_rx_slave, rs_mcs_bbe_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * dbg_mux_sel are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_bbe_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 * 
*/
int Ra01AccRsMcsBbeDbgMuxSel( RmsDev_t * rmsDev_p, 
    Ra01RsMcsBbeInstance_t instance, 
  Ra01RsMcsBbeDbgMuxSel_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief debug_status bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_bbe_slave debug_status 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_BBE_RX_DEBUG_STATUS_SEG0, RA01_BF_RS_MCS_BBE_TX_DEBUG_STATUS_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_BBE_RX_DEBUG_STATUS_SEG1, RA01_BF_RS_MCS_BBE_TX_DEBUG_STATUS_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_BBE_RX_DEBUG_STATUS_SEG2, RA01_BF_RS_MCS_BBE_TX_DEBUG_STATUS_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_BBE_RX_DEBUG_STATUS_SEG3, RA01_BF_RS_MCS_BBE_TX_DEBUG_STATUS_SEG3
} Ra01RsMcsBbeDebugStatus_t;

/* @brief Set the value of rs_mcs_bbe_slave debug_status
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_bbe_rx_slave, rs_mcs_bbe_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * debug_status are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_bbe_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_bbe_slave
 * 
*/
int Ra01GetRsMcsBbeDebugStatus( RmsDev_t * rmsDev_p, 
    Ra01RsMcsBbeInstance_t instance, 
  Ra01RsMcsBbeDebugStatus_t * value);


#endif // MCS_API_DEBUG


/* @brief Enumerate the rs_mcs_pab_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_PAB_RX_SLAVE = 0,  // Instance 0 of rs_mcs_pab_slave
    RA01_RS_MCS_PAB_TX_SLAVE = 1,  // Instance 1 of rs_mcs_pab_slave

    RA01_RS_MCS_PAB_INSTANCE_MAX
} ra01_rs_mcs_pab_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_pab_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_pab_slave_instance_t Ra01RsMcsPabInstance_t;




/* @brief pab_int bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave pab_int 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    uint8_t    overflow;                                // @sa RA01_BF_RS_MCS_PAB_RX_PAB_INT_OVERFLOW, RA01_BF_RS_MCS_PAB_TX_PAB_INT_OVERFLOW
} Ra01RsMcsPabPabInt_t;

/* @brief Set the value of rs_mcs_pab_slave pab_int
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pab_int are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabPabInt( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  Ra01RsMcsPabPabInt_t * value,
  ra01_reg_access_op_t op);







/* @brief pab_int_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave pab_int_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    uint8_t    overflowEnb;                             // @sa RA01_BF_RS_MCS_PAB_RX_PAB_INT_ENB_OVERFLOW_ENB, RA01_BF_RS_MCS_PAB_TX_PAB_INT_ENB_OVERFLOW_ENB
} Ra01RsMcsPabPabIntEnb_t;

/* @brief Set the value of rs_mcs_pab_slave pab_int_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pab_int_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabPabIntEnb( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  Ra01RsMcsPabPabIntEnb_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief pab_int_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave pab_int_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    uint8_t    overflow;                                // @sa RA01_BF_RS_MCS_PAB_RX_PAB_INT_RAW_OVERFLOW, RA01_BF_RS_MCS_PAB_TX_PAB_INT_RAW_OVERFLOW
} Ra01RsMcsPabPabIntRaw_t;

/* @brief Set the value of rs_mcs_pab_slave pab_int_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pab_int_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01GetRsMcsPabPabIntRaw( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  Ra01RsMcsPabPabIntRaw_t * value);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief pab_int_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave pab_int_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    uint8_t    overflow;                                // @sa RA01_BF_RS_MCS_PAB_RX_PAB_INT_INTR_RW_OVERFLOW, RA01_BF_RS_MCS_PAB_TX_PAB_INT_INTR_RW_OVERFLOW
} Ra01RsMcsPabPabIntIntrRw_t;

/* @brief Set the value of rs_mcs_pab_slave pab_int_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pab_int_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabPabIntIntrRw( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  Ra01RsMcsPabPabIntIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief hw_init bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave hw_init 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    bool    trigger;                                 // @sa RA01_BF_RS_MCS_PAB_RX_HW_INIT_TRIGGER, RA01_BF_RS_MCS_PAB_TX_HW_INIT_TRIGGER
} Ra01RsMcsPabHwInit_t;

/* @brief Set the value of rs_mcs_pab_slave hw_init
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * hw_init are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabHwInit( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  Ra01RsMcsPabHwInit_t * value,
  ra01_reg_access_op_t op);







/* @brief delay_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave delay_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    uint32_t    fixedLatency;                            // @sa RA01_BF_RS_MCS_PAB_RX_DELAY_CFG_FIXED_LATENCY, RA01_BF_RS_MCS_PAB_TX_DELAY_CFG_FIXED_LATENCY
    bool    enablePreemptFixedLatency;               // @sa RA01_BF_RS_MCS_PAB_RX_DELAY_CFG_ENABLE_PREEMPT_FIXED_LATENCY, RA01_BF_RS_MCS_PAB_TX_DELAY_CFG_ENABLE_PREEMPT_FIXED_LATENCY
} Ra01RsMcsPabDelayCfg_t;

/* @brief Set the value of rs_mcs_pab_slave delay_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * delay_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabDelayCfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  unsigned index,
  Ra01RsMcsPabDelayCfg_t * value,
  ra01_reg_access_op_t op);







/* @brief fifo_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave fifo_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    uint8_t    aeThExp;                                 // @sa RA01_BF_RS_MCS_PAB_RX_FIFO_CFG_AE_TH_EXP, RA01_BF_RS_MCS_PAB_TX_FIFO_CFG_AE_TH_EXP
    uint8_t    aeThPre;                                 // @sa RA01_BF_RS_MCS_PAB_RX_FIFO_CFG_AE_TH_PRE, RA01_BF_RS_MCS_PAB_TX_FIFO_CFG_AE_TH_PRE
} Ra01RsMcsPabFifoCfg_t;

/* @brief Set the value of rs_mcs_pab_slave fifo_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fifo_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabFifoCfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  unsigned index,
  Ra01RsMcsPabFifoCfg_t * value,
  ra01_reg_access_op_t op);







/* @brief include_tag_cnt bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave include_tag_cnt 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    bool    includeTagCnt;                           // @sa RA01_BF_RS_MCS_PAB_RX_INCLUDE_TAG_CNT_INCLUDE_TAG_CNT, RA01_BF_RS_MCS_PAB_TX_INCLUDE_TAG_CNT_INCLUDE_TAG_CNT
} Ra01RsMcsPabIncludeTagCnt_t;

/* @brief Set the value of rs_mcs_pab_slave include_tag_cnt
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * include_tag_cnt are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabIncludeTagCnt( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  Ra01RsMcsPabIncludeTagCnt_t * value,
  ra01_reg_access_op_t op);







/* @brief tx_trunc bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave tx_trunc 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    bool    enableTrunc;                             // @sa RA01_BF_RS_MCS_PAB_RX_TX_TRUNC_ENABLE_TRUNC, RA01_BF_RS_MCS_PAB_TX_TX_TRUNC_ENABLE_TRUNC
} Ra01RsMcsPabTxTrunc_t;

/* @brief Set the value of rs_mcs_pab_slave tx_trunc
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * tx_trunc are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabTxTrunc( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  Ra01RsMcsPabTxTrunc_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief dbg_mux_sel bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave dbg_mux_sel 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    uint8_t    seg0;                                    // @sa RA01_BF_RS_MCS_PAB_RX_DBG_MUX_SEL_SEG0, RA01_BF_RS_MCS_PAB_TX_DBG_MUX_SEL_SEG0
    uint8_t    seg1;                                    // @sa RA01_BF_RS_MCS_PAB_RX_DBG_MUX_SEL_SEG1, RA01_BF_RS_MCS_PAB_TX_DBG_MUX_SEL_SEG1
    uint8_t    seg2;                                    // @sa RA01_BF_RS_MCS_PAB_RX_DBG_MUX_SEL_SEG2, RA01_BF_RS_MCS_PAB_TX_DBG_MUX_SEL_SEG2
    uint8_t    seg3;                                    // @sa RA01_BF_RS_MCS_PAB_RX_DBG_MUX_SEL_SEG3, RA01_BF_RS_MCS_PAB_TX_DBG_MUX_SEL_SEG3
} Ra01RsMcsPabDbgMuxSel_t;

/* @brief Set the value of rs_mcs_pab_slave dbg_mux_sel
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * dbg_mux_sel are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabDbgMuxSel( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  Ra01RsMcsPabDbgMuxSel_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief debug_status bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave debug_status 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_PAB_RX_DEBUG_STATUS_SEG0, RA01_BF_RS_MCS_PAB_TX_DEBUG_STATUS_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_PAB_RX_DEBUG_STATUS_SEG1, RA01_BF_RS_MCS_PAB_TX_DEBUG_STATUS_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_PAB_RX_DEBUG_STATUS_SEG2, RA01_BF_RS_MCS_PAB_TX_DEBUG_STATUS_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_PAB_RX_DEBUG_STATUS_SEG3, RA01_BF_RS_MCS_PAB_TX_DEBUG_STATUS_SEG3
} Ra01RsMcsPabDebugStatus_t;

/* @brief Set the value of rs_mcs_pab_slave debug_status
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * debug_status are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01GetRsMcsPabDebugStatus( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  Ra01RsMcsPabDebugStatus_t * value);


#endif // MCS_API_DEBUG





/* @brief fifo_skid_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pab_slave fifo_skid_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 *
 */
typedef struct {
    uint8_t    bbePabSkidExp;                           // @sa RA01_BF_RS_MCS_PAB_RX_FIFO_SKID_CFG_BBE_PAB_SKID_EXP, RA01_BF_RS_MCS_PAB_TX_FIFO_SKID_CFG_BBE_PAB_SKID_EXP
    uint8_t    bbePabSkidPre;                           // @sa RA01_BF_RS_MCS_PAB_RX_FIFO_SKID_CFG_BBE_PAB_SKID_PRE, RA01_BF_RS_MCS_PAB_TX_FIFO_SKID_CFG_BBE_PAB_SKID_PRE
} Ra01RsMcsPabFifoSkidCfg_t;

/* @brief Set the value of rs_mcs_pab_slave fifo_skid_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pab_rx_slave, rs_mcs_pab_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fifo_skid_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pab_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pab_slave
 * 
*/
int Ra01AccRsMcsPabFifoSkidCfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPabInstance_t instance, 
  unsigned index,
  Ra01RsMcsPabFifoSkidCfg_t * value,
  ra01_reg_access_op_t op);




/* @brief Enumerate the rs_mcs_pex_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_PEX_RX_SLAVE = 0,  // Instance 0 of rs_mcs_pex_slave
    RA01_RS_MCS_PEX_TX_SLAVE = 1,  // Instance 1 of rs_mcs_pex_slave

    RA01_RS_MCS_PEX_INSTANCE_MAX
} ra01_rs_mcs_pex_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_pex_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_pex_slave_instance_t Ra01RsMcsPexInstance_t;




/* @brief pex_int bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave pex_int 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    earlyPreemptErr;                         // @sa RA01_BF_RS_MCS_PEX_RX_PEX_INT_EARLY_PREEMPT_ERR, RA01_BF_RS_MCS_PEX_TX_PEX_INT_EARLY_PREEMPT_ERR
} Ra01RsMcsPexPexInt_t;

/* @brief Set the value of rs_mcs_pex_slave pex_int
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pex_int are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexPexInt( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexPexInt_t * value,
  ra01_reg_access_op_t op);







/* @brief pex_int_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave pex_int_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    earlyPreemptErrEnb;                      // @sa RA01_BF_RS_MCS_PEX_RX_PEX_INT_ENB_EARLY_PREEMPT_ERR_ENB, RA01_BF_RS_MCS_PEX_TX_PEX_INT_ENB_EARLY_PREEMPT_ERR_ENB
} Ra01RsMcsPexPexIntEnb_t;

/* @brief Set the value of rs_mcs_pex_slave pex_int_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pex_int_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexPexIntEnb( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexPexIntEnb_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief pex_int_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave pex_int_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    earlyPreemptErr;                         // @sa RA01_BF_RS_MCS_PEX_RX_PEX_INT_RAW_EARLY_PREEMPT_ERR, RA01_BF_RS_MCS_PEX_TX_PEX_INT_RAW_EARLY_PREEMPT_ERR
} Ra01RsMcsPexPexIntRaw_t;

/* @brief Set the value of rs_mcs_pex_slave pex_int_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pex_int_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01GetRsMcsPexPexIntRaw( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexPexIntRaw_t * value);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief pex_int_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave pex_int_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    earlyPreemptErr;                         // @sa RA01_BF_RS_MCS_PEX_RX_PEX_INT_INTR_RW_EARLY_PREEMPT_ERR, RA01_BF_RS_MCS_PEX_TX_PEX_INT_INTR_RW_EARLY_PREEMPT_ERR
} Ra01RsMcsPexPexIntIntrRw_t;

/* @brief Set the value of rs_mcs_pex_slave pex_int_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pex_int_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexPexIntIntrRw( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexPexIntIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief pex_configuration bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave pex_configuration 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    vlanAfterCustom;                         // @sa RA01_BF_RS_MCS_PEX_RX_PEX_CONFIGURATION_VLAN_AFTER_CUSTOM, RA01_BF_RS_MCS_PEX_TX_PEX_CONFIGURATION_VLAN_AFTER_CUSTOM
    bool    nonDixErr;                               // @sa RA01_BF_RS_MCS_PEX_RX_PEX_CONFIGURATION_NON_DIX_ERR, RA01_BF_RS_MCS_PEX_TX_PEX_CONFIGURATION_NON_DIX_ERR
} Ra01RsMcsPexPexConfiguration_t;

/* @brief Set the value of rs_mcs_pex_slave pex_configuration
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pex_configuration are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexPexConfiguration( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexPexConfiguration_t * value,
  ra01_reg_access_op_t op);







/* @brief vlan_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave vlan_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    vlanEn;                                  // @sa RA01_BF_RS_MCS_PEX_RX_VLAN_CFG_VLAN_EN, RA01_BF_RS_MCS_PEX_TX_VLAN_CFG_VLAN_EN
    uint16_t    vlanEtype;                               // @sa RA01_BF_RS_MCS_PEX_RX_VLAN_CFG_VLAN_ETYPE, RA01_BF_RS_MCS_PEX_TX_VLAN_CFG_VLAN_ETYPE
    uint8_t    vlanIndx;                                // @sa RA01_BF_RS_MCS_PEX_RX_VLAN_CFG_VLAN_INDX, RA01_BF_RS_MCS_PEX_TX_VLAN_CFG_VLAN_INDX
    uint8_t    vlanSize;                                // @sa RA01_BF_RS_MCS_PEX_RX_VLAN_CFG_VLAN_SIZE, RA01_BF_RS_MCS_PEX_TX_VLAN_CFG_VLAN_SIZE
    bool    isVlan;                                  // @sa RA01_BF_RS_MCS_PEX_RX_VLAN_CFG_IS_VLAN, RA01_BF_RS_MCS_PEX_TX_VLAN_CFG_IS_VLAN
    bool    vlanBonus;                               // @sa RA01_BF_RS_MCS_PEX_RX_VLAN_CFG_VLAN_BONUS, RA01_BF_RS_MCS_PEX_TX_VLAN_CFG_VLAN_BONUS
} Ra01RsMcsPexVlanCfg_t;

/* @brief Set the value of rs_mcs_pex_slave vlan_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * vlan_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexVlanCfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexVlanCfg_t * value,
  ra01_reg_access_op_t op);



































/* @brief port_config bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave port_config 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint8_t    preamble;                                // @sa RA01_BF_RS_MCS_PEX_RX_PORT_CONFIG_PREAMBLE, RA01_BF_RS_MCS_PEX_TX_PORT_CONFIG_PREAMBLE
    uint8_t    vlanTagRelModeSel;                       // @sa RA01_BF_RS_MCS_PEX_RX_PORT_CONFIG_VLAN_TAG_REL_MODE_SEL, RA01_BF_RS_MCS_PEX_TX_PORT_CONFIG_VLAN_TAG_REL_MODE_SEL
} Ra01RsMcsPexPortConfig_t;

/* @brief Set the value of rs_mcs_pex_slave port_config
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * port_config are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexPortConfig( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexPortConfig_t * value,
  ra01_reg_access_op_t op);







/* @brief mpls_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave mpls_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    mplsEn;                                  // @sa RA01_BF_RS_MCS_PEX_RX_MPLS_CFG_MPLS_EN, RA01_BF_RS_MCS_PEX_TX_MPLS_CFG_MPLS_EN
    uint16_t    mplsEtype;                               // @sa RA01_BF_RS_MCS_PEX_RX_MPLS_CFG_MPLS_ETYPE, RA01_BF_RS_MCS_PEX_TX_MPLS_CFG_MPLS_ETYPE
} Ra01RsMcsPexMplsCfg_t;

/* @brief Set the value of rs_mcs_pex_slave mpls_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * mpls_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexMplsCfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexMplsCfg_t * value,
  ra01_reg_access_op_t op);



















/* @brief sectag_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave sectag_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    sectagEn;                                // @sa RA01_BF_RS_MCS_PEX_RX_SECTAG_CFG_SECTAG_EN, RA01_BF_RS_MCS_PEX_TX_SECTAG_CFG_SECTAG_EN
    uint16_t    sectagEtype;                             // @sa RA01_BF_RS_MCS_PEX_RX_SECTAG_CFG_SECTAG_ETYPE, RA01_BF_RS_MCS_PEX_TX_SECTAG_CFG_SECTAG_ETYPE
} Ra01RsMcsPexSectagCfg_t;

/* @brief Set the value of rs_mcs_pex_slave sectag_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sectag_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexSectagCfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexSectagCfg_t * value,
  ra01_reg_access_op_t op);







/* @brief mcs_header_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave mcs_header_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    rxMcsHdrEn;                              // @sa RA01_BF_RS_MCS_PEX_RX_MCS_HEADER_CFG_RX_MCS_HDR_EN, RA01_BF_RS_MCS_PEX_TX_MCS_HEADER_CFG_RX_MCS_HDR_EN
    uint16_t    rxMcsHeader;                             // @sa RA01_BF_RS_MCS_PEX_RX_MCS_HEADER_CFG_RX_MCS_HEADER, RA01_BF_RS_MCS_PEX_TX_MCS_HEADER_CFG_RX_MCS_HEADER
    bool    txMcsHdrEn;                              // @sa RA01_BF_RS_MCS_PEX_RX_MCS_HEADER_CFG_TX_MCS_HDR_EN, RA01_BF_RS_MCS_PEX_TX_MCS_HEADER_CFG_TX_MCS_HDR_EN
    uint16_t    txMcsHeader;                             // @sa RA01_BF_RS_MCS_PEX_RX_MCS_HEADER_CFG_TX_MCS_HEADER, RA01_BF_RS_MCS_PEX_TX_MCS_HEADER_CFG_TX_MCS_HEADER
} Ra01RsMcsPexMcsHeaderCfg_t;

/* @brief Set the value of rs_mcs_pex_slave mcs_header_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * mcs_header_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexMcsHeaderCfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexMcsHeaderCfg_t * value,
  ra01_reg_access_op_t op);







/* @brief ipv4_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ipv4_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    ipv4En;                                  // @sa RA01_BF_RS_MCS_PEX_RX_IPV4_CFG_IPV4_EN, RA01_BF_RS_MCS_PEX_TX_IPV4_CFG_IPV4_EN
    uint16_t    ipv4Etype;                               // @sa RA01_BF_RS_MCS_PEX_RX_IPV4_CFG_IPV4_ETYPE, RA01_BF_RS_MCS_PEX_TX_IPV4_CFG_IPV4_ETYPE
} Ra01RsMcsPexIpv4Cfg_t;

/* @brief Set the value of rs_mcs_pex_slave ipv4_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ipv4_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexIpv4Cfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexIpv4Cfg_t * value,
  ra01_reg_access_op_t op);







/* @brief ipv6_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ipv6_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    ipv6En;                                  // @sa RA01_BF_RS_MCS_PEX_RX_IPV6_CFG_IPV6_EN, RA01_BF_RS_MCS_PEX_TX_IPV6_CFG_IPV6_EN
    uint16_t    ipv6Etype;                               // @sa RA01_BF_RS_MCS_PEX_RX_IPV6_CFG_IPV6_ETYPE, RA01_BF_RS_MCS_PEX_TX_IPV6_CFG_IPV6_ETYPE
} Ra01RsMcsPexIpv6Cfg_t;

/* @brief Set the value of rs_mcs_pex_slave ipv6_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ipv6_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexIpv6Cfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexIpv6Cfg_t * value,
  ra01_reg_access_op_t op);







/* @brief udp_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave udp_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    udpEn;                                   // @sa RA01_BF_RS_MCS_PEX_RX_UDP_CFG_UDP_EN, RA01_BF_RS_MCS_PEX_TX_UDP_CFG_UDP_EN
    uint8_t    udpProtNum;                              // @sa RA01_BF_RS_MCS_PEX_RX_UDP_CFG_UDP_PROT_NUM, RA01_BF_RS_MCS_PEX_TX_UDP_CFG_UDP_PROT_NUM
} Ra01RsMcsPexUdpCfg_t;

/* @brief Set the value of rs_mcs_pex_slave udp_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * udp_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexUdpCfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexUdpCfg_t * value,
  ra01_reg_access_op_t op);







/* @brief ptp_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ptp_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    ptpEn;                                   // @sa RA01_BF_RS_MCS_PEX_RX_PTP_CFG_PTP_EN, RA01_BF_RS_MCS_PEX_TX_PTP_CFG_PTP_EN
    uint16_t    ptpPort;                                 // @sa RA01_BF_RS_MCS_PEX_RX_PTP_CFG_PTP_PORT, RA01_BF_RS_MCS_PEX_TX_PTP_CFG_PTP_PORT
} Ra01RsMcsPexPtpCfg_t;

/* @brief Set the value of rs_mcs_pex_slave ptp_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ptp_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexPtpCfg( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexPtpCfg_t * value,
  ra01_reg_access_op_t op);











/* @brief ctl_pkt_rule_etype bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ctl_pkt_rule_etype 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint16_t    ruleEtype;                               // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_ETYPE_RULE_ETYPE, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_ETYPE_RULE_ETYPE
} Ra01RsMcsPexCtlPktRuleEtype_t;

/* @brief Set the value of rs_mcs_pex_slave ctl_pkt_rule_etype
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ctl_pkt_rule_etype are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexCtlPktRuleEtype( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexCtlPktRuleEtype_t * value,
  ra01_reg_access_op_t op);



































/* @brief ctl_pkt_rule_da bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ctl_pkt_rule_da 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint32_t    ruleDaLsb;                               // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_DA_RULE_DA_LSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_DA_RULE_DA_LSB
    uint16_t    ruleDaMsb;                               // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_DA_RULE_DA_MSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_DA_RULE_DA_MSB
} Ra01RsMcsPexCtlPktRuleDa_t;

/* @brief Set the value of rs_mcs_pex_slave ctl_pkt_rule_da
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ctl_pkt_rule_da are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexCtlPktRuleDa( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexCtlPktRuleDa_t * value,
  ra01_reg_access_op_t op);



































/* @brief ctl_pkt_rule_da_range_min bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ctl_pkt_rule_da_range_min 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint32_t    ruleRangeMinLsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_DA_RANGE_MIN_RULE_RANGE_MIN_LSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_DA_RANGE_MIN_RULE_RANGE_MIN_LSB
    uint16_t    ruleRangeMinMsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_DA_RANGE_MIN_RULE_RANGE_MIN_MSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_DA_RANGE_MIN_RULE_RANGE_MIN_MSB
} Ra01RsMcsPexCtlPktRuleDaRangeMin_t;

/* @brief Set the value of rs_mcs_pex_slave ctl_pkt_rule_da_range_min
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ctl_pkt_rule_da_range_min are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexCtlPktRuleDaRangeMin( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexCtlPktRuleDaRangeMin_t * value,
  ra01_reg_access_op_t op);







/* @brief ctl_pkt_rule_da_range_max bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ctl_pkt_rule_da_range_max 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint32_t    ruleRangeMaxLsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_DA_RANGE_MAX_RULE_RANGE_MAX_LSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_DA_RANGE_MAX_RULE_RANGE_MAX_LSB
    uint16_t    ruleRangeMaxMsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_DA_RANGE_MAX_RULE_RANGE_MAX_MSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_DA_RANGE_MAX_RULE_RANGE_MAX_MSB
} Ra01RsMcsPexCtlPktRuleDaRangeMax_t;

/* @brief Set the value of rs_mcs_pex_slave ctl_pkt_rule_da_range_max
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ctl_pkt_rule_da_range_max are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexCtlPktRuleDaRangeMax( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexCtlPktRuleDaRangeMax_t * value,
  ra01_reg_access_op_t op);































/* @brief ctl_pkt_rule_combo_min bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ctl_pkt_rule_combo_min 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint32_t    ruleComboMinLsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_COMBO_MIN_RULE_COMBO_MIN_LSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_COMBO_MIN_RULE_COMBO_MIN_LSB
    uint16_t    ruleComboMinMsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_COMBO_MIN_RULE_COMBO_MIN_MSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_COMBO_MIN_RULE_COMBO_MIN_MSB
} Ra01RsMcsPexCtlPktRuleComboMin_t;

/* @brief Set the value of rs_mcs_pex_slave ctl_pkt_rule_combo_min
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ctl_pkt_rule_combo_min are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexCtlPktRuleComboMin( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexCtlPktRuleComboMin_t * value,
  ra01_reg_access_op_t op);







/* @brief ctl_pkt_rule_combo_max bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ctl_pkt_rule_combo_max 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint32_t    ruleComboMaxLsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_COMBO_MAX_RULE_COMBO_MAX_LSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_COMBO_MAX_RULE_COMBO_MAX_LSB
    uint16_t    ruleComboMaxMsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_COMBO_MAX_RULE_COMBO_MAX_MSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_COMBO_MAX_RULE_COMBO_MAX_MSB
} Ra01RsMcsPexCtlPktRuleComboMax_t;

/* @brief Set the value of rs_mcs_pex_slave ctl_pkt_rule_combo_max
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ctl_pkt_rule_combo_max are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexCtlPktRuleComboMax( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexCtlPktRuleComboMax_t * value,
  ra01_reg_access_op_t op);







/* @brief ctl_pkt_rule_combo_et bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ctl_pkt_rule_combo_et 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint16_t    ruleComboEt;                             // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_COMBO_ET_RULE_COMBO_ET, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_COMBO_ET_RULE_COMBO_ET
} Ra01RsMcsPexCtlPktRuleComboEt_t;

/* @brief Set the value of rs_mcs_pex_slave ctl_pkt_rule_combo_et
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ctl_pkt_rule_combo_et are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexCtlPktRuleComboEt( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexCtlPktRuleComboEt_t * value,
  ra01_reg_access_op_t op);











































/* @brief ctl_pkt_rule_da_prefix bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ctl_pkt_rule_da_prefix 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint32_t    ruleDaPrefixLsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_DA_PREFIX_RULE_DA_PREFIX_LSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_DA_PREFIX_RULE_DA_PREFIX_LSB
    uint16_t    ruleDaPrefixMsb;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_DA_PREFIX_RULE_DA_PREFIX_MSB, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_DA_PREFIX_RULE_DA_PREFIX_MSB
} Ra01RsMcsPexCtlPktRuleDaPrefix_t;

/* @brief Set the value of rs_mcs_pex_slave ctl_pkt_rule_da_prefix
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ctl_pkt_rule_da_prefix are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexCtlPktRuleDaPrefix( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexCtlPktRuleDaPrefix_t * value,
  ra01_reg_access_op_t op);







/* @brief ctl_pkt_rule_enable bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave ctl_pkt_rule_enable 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint8_t    ctlPktRuleEtypeEn;                       // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_ETYPE_EN, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_ETYPE_EN
    uint8_t    ctlPktRuleDaEn;                          // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_DA_EN, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_DA_EN
    uint8_t    ctlPktRuleDaRangeEn;                     // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_DA_RANGE_EN, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_DA_RANGE_EN
    uint8_t    ctlPktRuleComboEn;                       // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_COMBO_EN, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_COMBO_EN
    bool    ctlPktRuleMacEn;                         // @sa RA01_BF_RS_MCS_PEX_RX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_MAC_EN, RA01_BF_RS_MCS_PEX_TX_CTL_PKT_RULE_ENABLE_CTL_PKT_RULE_MAC_EN
} Ra01RsMcsPexCtlPktRuleEnable_t;

/* @brief Set the value of rs_mcs_pex_slave ctl_pkt_rule_enable
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * ctl_pkt_rule_enable are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexCtlPktRuleEnable( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexCtlPktRuleEnable_t * value,
  ra01_reg_access_op_t op);







/* @brief early_preempt_filter_ctrl bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave early_preempt_filter_ctrl 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    bool    earlyPreemptFilterEnable;                // @sa RA01_BF_RS_MCS_PEX_RX_EARLY_PREEMPT_FILTER_CTRL_EARLY_PREEMPT_FILTER_ENABLE, RA01_BF_RS_MCS_PEX_TX_EARLY_PREEMPT_FILTER_CTRL_EARLY_PREEMPT_FILTER_ENABLE
    uint8_t    earlyPreemptMinNumWords;                 // @sa RA01_BF_RS_MCS_PEX_RX_EARLY_PREEMPT_FILTER_CTRL_EARLY_PREEMPT_MIN_NUM_WORDS, RA01_BF_RS_MCS_PEX_TX_EARLY_PREEMPT_FILTER_CTRL_EARLY_PREEMPT_MIN_NUM_WORDS
} Ra01RsMcsPexEarlyPreemptFilterCtrl_t;

/* @brief Set the value of rs_mcs_pex_slave early_preempt_filter_ctrl
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * early_preempt_filter_ctrl are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexEarlyPreemptFilterCtrl( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  unsigned index,
  Ra01RsMcsPexEarlyPreemptFilterCtrl_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief dbg_mux_sel bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave dbg_mux_sel 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint8_t    seg0;                                    // @sa RA01_BF_RS_MCS_PEX_RX_DBG_MUX_SEL_SEG0, RA01_BF_RS_MCS_PEX_TX_DBG_MUX_SEL_SEG0
    uint8_t    seg1;                                    // @sa RA01_BF_RS_MCS_PEX_RX_DBG_MUX_SEL_SEG1, RA01_BF_RS_MCS_PEX_TX_DBG_MUX_SEL_SEG1
    uint8_t    seg2;                                    // @sa RA01_BF_RS_MCS_PEX_RX_DBG_MUX_SEL_SEG2, RA01_BF_RS_MCS_PEX_TX_DBG_MUX_SEL_SEG2
    uint8_t    seg3;                                    // @sa RA01_BF_RS_MCS_PEX_RX_DBG_MUX_SEL_SEG3, RA01_BF_RS_MCS_PEX_TX_DBG_MUX_SEL_SEG3
} Ra01RsMcsPexDbgMuxSel_t;

/* @brief Set the value of rs_mcs_pex_slave dbg_mux_sel
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * dbg_mux_sel are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01AccRsMcsPexDbgMuxSel( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexDbgMuxSel_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief debug_status bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_pex_slave debug_status 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_PEX_RX_DEBUG_STATUS_SEG0, RA01_BF_RS_MCS_PEX_TX_DEBUG_STATUS_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_PEX_RX_DEBUG_STATUS_SEG1, RA01_BF_RS_MCS_PEX_TX_DEBUG_STATUS_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_PEX_RX_DEBUG_STATUS_SEG2, RA01_BF_RS_MCS_PEX_TX_DEBUG_STATUS_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_PEX_RX_DEBUG_STATUS_SEG3, RA01_BF_RS_MCS_PEX_TX_DEBUG_STATUS_SEG3
} Ra01RsMcsPexDebugStatus_t;

/* @brief Set the value of rs_mcs_pex_slave debug_status
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    instance  Which instance to be written { rs_mcs_pex_rx_slave, rs_mcs_pex_tx_slave  }
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * debug_status are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_pex_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_pex_slave
 * 
*/
int Ra01GetRsMcsPexDebugStatus( RmsDev_t * rmsDev_p, 
    Ra01RsMcsPexInstance_t instance, 
  Ra01RsMcsPexDebugStatus_t * value);


#endif // MCS_API_DEBUG


/* @brief Enumerate the rs_mcs_cse_rx_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_CSE_RX_SLAVE = 0,  // Instance 0 of rs_mcs_cse_rx_slave

    RA01_RS_MCS_CSE_RX_INSTANCE_MAX
} ra01_rs_mcs_cse_rx_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_cse_rx_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_cse_rx_slave_instance_t Ra01RsMcsCseRxInstance_t;

#ifdef MCS_API_DEBUG



/* @brief reserved0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_rx_slave reserved0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 *
 */
typedef struct {
    bool    reserved;                                // @sa RA01_BF_RS_MCS_CSE_RX_RESERVED0_RESERVED
} Ra01RsMcsCseRxReserved0_t;

/* @brief Set the value of rs_mcs_cse_rx_slave reserved0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * reserved0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 * 
*/
int Ra01GetRsMcsCseRxReserved0( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCseRxReserved0_t * value);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG





/* @brief CTRL bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_rx_slave CTRL 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 *
 */
typedef struct {
    bool    cpuCseClrOnRd;                           // @sa RA01_BF_RS_MCS_CSE_RX_CTRL_CPU_CSE_CLR_ON_RD
} Ra01RsMcsCseRxCtrl_t;

/* @brief Set the value of rs_mcs_cse_rx_slave CTRL
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * CTRL are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 * 
*/
int Ra01AccRsMcsCseRxCtrl( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCseRxCtrl_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief dbg_mux_sel bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_rx_slave dbg_mux_sel 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_CSE_RX_DBG_MUX_SEL_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_CSE_RX_DBG_MUX_SEL_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_CSE_RX_DBG_MUX_SEL_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_CSE_RX_DBG_MUX_SEL_SEG3
} Ra01RsMcsCseRxDbgMuxSel_t;

/* @brief Set the value of rs_mcs_cse_rx_slave dbg_mux_sel
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * dbg_mux_sel are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 * 
*/
int Ra01AccRsMcsCseRxDbgMuxSel( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCseRxDbgMuxSel_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief debug_status bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_rx_slave debug_status 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_CSE_RX_DEBUG_STATUS_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_CSE_RX_DEBUG_STATUS_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_CSE_RX_DEBUG_STATUS_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_CSE_RX_DEBUG_STATUS_SEG3
} Ra01RsMcsCseRxDebugStatus_t;

/* @brief Set the value of rs_mcs_cse_rx_slave debug_status
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * debug_status are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 * 
*/
int Ra01GetRsMcsCseRxDebugStatus( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCseRxDebugStatus_t * value);


#endif // MCS_API_DEBUG





/* @brief stats_clear bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_rx_slave stats_clear 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 *
 */
typedef struct {
    bool    rxSecyGo;                                // @sa RA01_BF_RS_MCS_CSE_RX_STATS_CLEAR_RX_SECY_GO
    bool    rxScGo;                                  // @sa RA01_BF_RS_MCS_CSE_RX_STATS_CLEAR_RX_SC_GO
    bool    rxPortGo;                                // @sa RA01_BF_RS_MCS_CSE_RX_STATS_CLEAR_RX_PORT_GO
    bool    rxFlowidGo;                              // @sa RA01_BF_RS_MCS_CSE_RX_STATS_CLEAR_RX_FLOWID_GO
    bool    rxSaGo;                                  // @sa RA01_BF_RS_MCS_CSE_RX_STATS_CLEAR_RX_SA_GO
} Ra01RsMcsCseRxStatsClear_t;

/* @brief Set the value of rs_mcs_cse_rx_slave stats_clear
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * stats_clear are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_slave
 * 
*/
int Ra01AccRsMcsCseRxStatsClear( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCseRxStatsClear_t * value,
  ra01_reg_access_op_t op);




/* @brief Enumerate the rs_mcs_cse_tx_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_CSE_TX_SLAVE = 0,  // Instance 0 of rs_mcs_cse_tx_slave

    RA01_RS_MCS_CSE_TX_INSTANCE_MAX
} ra01_rs_mcs_cse_tx_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_cse_tx_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_cse_tx_slave_instance_t Ra01RsMcsCseTxInstance_t;

#ifdef MCS_API_DEBUG



/* @brief reserved0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_tx_slave reserved0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 *
 */
typedef struct {
    bool    reserved;                                // @sa RA01_BF_RS_MCS_CSE_TX_RESERVED0_RESERVED
} Ra01RsMcsCseTxReserved0_t;

/* @brief Set the value of rs_mcs_cse_tx_slave reserved0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * reserved0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 * 
*/
int Ra01GetRsMcsCseTxReserved0( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCseTxReserved0_t * value);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG





/* @brief CTRL bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_tx_slave CTRL 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 *
 */
typedef struct {
    bool    cpuCseClrOnRd;                           // @sa RA01_BF_RS_MCS_CSE_TX_CTRL_CPU_CSE_CLR_ON_RD
} Ra01RsMcsCseTxCtrl_t;

/* @brief Set the value of rs_mcs_cse_tx_slave CTRL
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * CTRL are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 * 
*/
int Ra01AccRsMcsCseTxCtrl( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCseTxCtrl_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief dbg_mux_sel bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_tx_slave dbg_mux_sel 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_CSE_TX_DBG_MUX_SEL_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_CSE_TX_DBG_MUX_SEL_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_CSE_TX_DBG_MUX_SEL_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_CSE_TX_DBG_MUX_SEL_SEG3
} Ra01RsMcsCseTxDbgMuxSel_t;

/* @brief Set the value of rs_mcs_cse_tx_slave dbg_mux_sel
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * dbg_mux_sel are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 * 
*/
int Ra01AccRsMcsCseTxDbgMuxSel( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCseTxDbgMuxSel_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief debug_status bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_tx_slave debug_status 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_CSE_TX_DEBUG_STATUS_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_CSE_TX_DEBUG_STATUS_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_CSE_TX_DEBUG_STATUS_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_CSE_TX_DEBUG_STATUS_SEG3
} Ra01RsMcsCseTxDebugStatus_t;

/* @brief Set the value of rs_mcs_cse_tx_slave debug_status
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * debug_status are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 * 
*/
int Ra01GetRsMcsCseTxDebugStatus( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCseTxDebugStatus_t * value);


#endif // MCS_API_DEBUG





/* @brief stats_clear bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cse_tx_slave stats_clear 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 *
 */
typedef struct {
    bool    txSecyGo;                                // @sa RA01_BF_RS_MCS_CSE_TX_STATS_CLEAR_TX_SECY_GO
    bool    txScGo;                                  // @sa RA01_BF_RS_MCS_CSE_TX_STATS_CLEAR_TX_SC_GO
    bool    txPortGo;                                // @sa RA01_BF_RS_MCS_CSE_TX_STATS_CLEAR_TX_PORT_GO
    bool    txFlowidGo;                              // @sa RA01_BF_RS_MCS_CSE_TX_STATS_CLEAR_TX_FLOWID_GO
    bool    txSaGo;                                  // @sa RA01_BF_RS_MCS_CSE_TX_STATS_CLEAR_TX_SA_GO
} Ra01RsMcsCseTxStatsClear_t;

/* @brief Set the value of rs_mcs_cse_tx_slave stats_clear
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * stats_clear are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_slave
 * 
*/
int Ra01AccRsMcsCseTxStatsClear( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCseTxStatsClear_t * value,
  ra01_reg_access_op_t op);




/* @brief Enumerate the rs_mcs_cse_rx_mem_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_CSE_RX_MEM_SLAVE = 0,  // Instance 0 of rs_mcs_cse_rx_mem_slave

    RA01_RS_MCS_CSE_RX_MEM_INSTANCE_MAX
} ra01_rs_mcs_cse_rx_mem_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_cse_rx_mem_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_cse_rx_mem_slave_instance_t Ra01RsMcsCseRxMemInstance_t;

/* @brief Enumerate the rs_mcs_cse_tx_mem_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_CSE_TX_MEM_SLAVE = 0,  // Instance 0 of rs_mcs_cse_tx_mem_slave

    RA01_RS_MCS_CSE_TX_MEM_INSTANCE_MAX
} ra01_rs_mcs_cse_tx_mem_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_cse_tx_mem_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_cse_tx_mem_slave_instance_t Ra01RsMcsCseTxMemInstance_t;

/* @brief Enumerate the rs_mcs_cpm_rx_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_CPM_RX_SLAVE = 0,  // Instance 0 of rs_mcs_cpm_rx_slave

    RA01_RS_MCS_CPM_RX_INSTANCE_MAX
} ra01_rs_mcs_cpm_rx_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_cpm_rx_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_cpm_rx_slave_instance_t Ra01RsMcsCpmRxInstance_t;




/* @brief cpm_rx_int bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave cpm_rx_int 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    bool    sectagVEq1;                              // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_SECTAG_V_EQ1
    bool    sectagEEq0CEq1;                          // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_SECTAG_E_EQ0_C_EQ1
    bool    sectagSlGte48;                           // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_SECTAG_SL_GTE48
    bool    sectagEsEq1ScEq1;                        // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_SECTAG_ES_EQ1_SC_EQ1
    bool    sectagScEq1ScbEq1;                       // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_SECTAG_SC_EQ1_SCB_EQ1
    bool    packetXpnEq0;                            // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_PACKET_XPN_EQ0
    bool    pnThreshReached;                         // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_PN_THRESH_REACHED
    bool    scExpiryPreTimeout;                      // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_SC_EXPIRY_PRE_TIMEOUT
    bool    scExpiryActualTimeout;                   // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_SC_EXPIRY_ACTUAL_TIMEOUT
} Ra01RsMcsCpmRxCpmRxInt_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave cpm_rx_int
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * cpm_rx_int are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxCpmRxInt( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxCpmRxInt_t * value,
  ra01_reg_access_op_t op);







/* @brief cpm_rx_int_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave cpm_rx_int_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    bool    sectagVEq1Enb;                           // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_ENB_SECTAG_V_EQ1_ENB
    bool    sectagEEq0CEq1Enb;                       // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_ENB_SECTAG_E_EQ0_C_EQ1_ENB
    bool    sectagSlGte48Enb;                        // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_ENB_SECTAG_SL_GTE48_ENB
    bool    sectagEsEq1ScEq1Enb;                     // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_ENB_SECTAG_ES_EQ1_SC_EQ1_ENB
    bool    sectagScEq1ScbEq1Enb;                    // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_ENB_SECTAG_SC_EQ1_SCB_EQ1_ENB
    bool    packetXpnEq0Enb;                         // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_ENB_PACKET_XPN_EQ0_ENB
    bool    pnThreshReachedEnb;                      // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_ENB_PN_THRESH_REACHED_ENB
    bool    scExpiryPreTimeoutEnb;                   // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_ENB_SC_EXPIRY_PRE_TIMEOUT_ENB
    bool    scExpiryActualTimeoutEnb;                // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_ENB_SC_EXPIRY_ACTUAL_TIMEOUT_ENB
} Ra01RsMcsCpmRxCpmRxIntEnb_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave cpm_rx_int_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * cpm_rx_int_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxCpmRxIntEnb( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxCpmRxIntEnb_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief cpm_rx_int_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave cpm_rx_int_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    bool    sectagVEq1;                              // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_RAW_SECTAG_V_EQ1
    bool    sectagEEq0CEq1;                          // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_RAW_SECTAG_E_EQ0_C_EQ1
    bool    sectagSlGte48;                           // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_RAW_SECTAG_SL_GTE48
    bool    sectagEsEq1ScEq1;                        // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_RAW_SECTAG_ES_EQ1_SC_EQ1
    bool    sectagScEq1ScbEq1;                       // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_RAW_SECTAG_SC_EQ1_SCB_EQ1
    bool    packetXpnEq0;                            // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_RAW_PACKET_XPN_EQ0
    bool    pnThreshReached;                         // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_RAW_PN_THRESH_REACHED
    bool    scExpiryPreTimeout;                      // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_RAW_SC_EXPIRY_PRE_TIMEOUT
    bool    scExpiryActualTimeout;                   // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_RAW_SC_EXPIRY_ACTUAL_TIMEOUT
} Ra01RsMcsCpmRxCpmRxIntRaw_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave cpm_rx_int_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * cpm_rx_int_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01GetRsMcsCpmRxCpmRxIntRaw( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxCpmRxIntRaw_t * value);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief cpm_rx_int_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave cpm_rx_int_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    bool    sectagVEq1;                              // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_INTR_RW_SECTAG_V_EQ1
    bool    sectagEEq0CEq1;                          // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_INTR_RW_SECTAG_E_EQ0_C_EQ1
    bool    sectagSlGte48;                           // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_INTR_RW_SECTAG_SL_GTE48
    bool    sectagEsEq1ScEq1;                        // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_INTR_RW_SECTAG_ES_EQ1_SC_EQ1
    bool    sectagScEq1ScbEq1;                       // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_INTR_RW_SECTAG_SC_EQ1_SCB_EQ1
    bool    packetXpnEq0;                            // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_INTR_RW_PACKET_XPN_EQ0
    bool    pnThreshReached;                         // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_INTR_RW_PN_THRESH_REACHED
    bool    scExpiryPreTimeout;                      // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_INTR_RW_SC_EXPIRY_PRE_TIMEOUT
    bool    scExpiryActualTimeout;                   // @sa RA01_BF_RS_MCS_CPM_RX_CPM_RX_INT_INTR_RW_SC_EXPIRY_ACTUAL_TIMEOUT
} Ra01RsMcsCpmRxCpmRxIntIntrRw_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave cpm_rx_int_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * cpm_rx_int_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxCpmRxIntIntrRw( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxCpmRxIntIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief sc_expiry_pre_timeout bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_expiry_pre_timeout 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryPreTimeoutInt;                   // @sa RA01_BF_RS_MCS_CPM_RX_SC_EXPIRY_PRE_TIMEOUT_SC_EXPIRY_PRE_TIMEOUT_INT
} Ra01RsMcsCpmRxScExpiryPreTimeout_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_expiry_pre_timeout
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_pre_timeout are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScExpiryPreTimeout( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScExpiryPreTimeout_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_expiry_pre_timeout_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_expiry_pre_timeout_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryPreTimeoutIntEnb;                // @sa RA01_BF_RS_MCS_CPM_RX_SC_EXPIRY_PRE_TIMEOUT_ENB_SC_EXPIRY_PRE_TIMEOUT_INT_ENB
} Ra01RsMcsCpmRxScExpiryPreTimeoutEnb_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_expiry_pre_timeout_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_pre_timeout_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScExpiryPreTimeoutEnb( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScExpiryPreTimeoutEnb_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_expiry_pre_timeout_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_expiry_pre_timeout_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryPreTimeoutInt;                   // @sa RA01_BF_RS_MCS_CPM_RX_SC_EXPIRY_PRE_TIMEOUT_RAW_SC_EXPIRY_PRE_TIMEOUT_INT
} Ra01RsMcsCpmRxScExpiryPreTimeoutRaw_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_expiry_pre_timeout_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_pre_timeout_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01GetRsMcsCpmRxScExpiryPreTimeoutRaw( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScExpiryPreTimeoutRaw_t * value);




#ifdef MCS_API_DEBUG



/* @brief sc_expiry_pre_timeout_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_expiry_pre_timeout_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryPreTimeoutInt;                   // @sa RA01_BF_RS_MCS_CPM_RX_SC_EXPIRY_PRE_TIMEOUT_INTR_RW_SC_EXPIRY_PRE_TIMEOUT_INT
} Ra01RsMcsCpmRxScExpiryPreTimeoutIntrRw_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_expiry_pre_timeout_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_pre_timeout_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScExpiryPreTimeoutIntrRw( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScExpiryPreTimeoutIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief sc_expiry_actual_timeout bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_expiry_actual_timeout 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryActualTimeoutInt;                // @sa RA01_BF_RS_MCS_CPM_RX_SC_EXPIRY_ACTUAL_TIMEOUT_SC_EXPIRY_ACTUAL_TIMEOUT_INT
} Ra01RsMcsCpmRxScExpiryActualTimeout_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_expiry_actual_timeout
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_actual_timeout are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScExpiryActualTimeout( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScExpiryActualTimeout_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_expiry_actual_timeout_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_expiry_actual_timeout_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryActualTimeoutIntEnb;             // @sa RA01_BF_RS_MCS_CPM_RX_SC_EXPIRY_ACTUAL_TIMEOUT_ENB_SC_EXPIRY_ACTUAL_TIMEOUT_INT_ENB
} Ra01RsMcsCpmRxScExpiryActualTimeoutEnb_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_expiry_actual_timeout_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_actual_timeout_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScExpiryActualTimeoutEnb( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScExpiryActualTimeoutEnb_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_expiry_actual_timeout_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_expiry_actual_timeout_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryActualTimeoutInt;                // @sa RA01_BF_RS_MCS_CPM_RX_SC_EXPIRY_ACTUAL_TIMEOUT_RAW_SC_EXPIRY_ACTUAL_TIMEOUT_INT
} Ra01RsMcsCpmRxScExpiryActualTimeoutRaw_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_expiry_actual_timeout_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_actual_timeout_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01GetRsMcsCpmRxScExpiryActualTimeoutRaw( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScExpiryActualTimeoutRaw_t * value);




#ifdef MCS_API_DEBUG



/* @brief sc_expiry_actual_timeout_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_expiry_actual_timeout_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryActualTimeoutInt;                // @sa RA01_BF_RS_MCS_CPM_RX_SC_EXPIRY_ACTUAL_TIMEOUT_INTR_RW_SC_EXPIRY_ACTUAL_TIMEOUT_INT
} Ra01RsMcsCpmRxScExpiryActualTimeoutIntrRw_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_expiry_actual_timeout_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_actual_timeout_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScExpiryActualTimeoutIntrRw( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScExpiryActualTimeoutIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief sectag_rule_chk_enable bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sectag_rule_chk_enable 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    bool    sectagVEq1Ena;                           // @sa RA01_BF_RS_MCS_CPM_RX_SECTAG_RULE_CHK_ENABLE_SECTAG_V_EQ1_ENA
    bool    sectagEsEq1ScEq1Ena;                     // @sa RA01_BF_RS_MCS_CPM_RX_SECTAG_RULE_CHK_ENABLE_SECTAG_ES_EQ1_SC_EQ1_ENA
    bool    sectagScEq1ScbEq1Ena;                    // @sa RA01_BF_RS_MCS_CPM_RX_SECTAG_RULE_CHK_ENABLE_SECTAG_SC_EQ1_SCB_EQ1_ENA
    bool    sectagPnEq0Ena;                          // @sa RA01_BF_RS_MCS_CPM_RX_SECTAG_RULE_CHK_ENABLE_SECTAG_PN_EQ0_ENA
} Ra01RsMcsCpmRxSectagRuleChkEnable_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sectag_rule_chk_enable
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sectag_rule_chk_enable are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxSectagRuleChkEnable( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxSectagRuleChkEnable_t * value,
  ra01_reg_access_op_t op);







/* @brief cpu_punt_enable bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave cpu_punt_enable 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    bool    puntParseError;                          // @sa RA01_BF_RS_MCS_CPM_RX_CPU_PUNT_ENABLE_PUNT_PARSE_ERROR
    bool    puntFlowidTcamMiss;                      // @sa RA01_BF_RS_MCS_CPM_RX_CPU_PUNT_ENABLE_PUNT_FLOWID_TCAM_MISS
    bool    puntCtrlPortDisabled;                    // @sa RA01_BF_RS_MCS_CPM_RX_CPU_PUNT_ENABLE_PUNT_CTRL_PORT_DISABLED
    bool    puntUntagged;                            // @sa RA01_BF_RS_MCS_CPM_RX_CPU_PUNT_ENABLE_PUNT_UNTAGGED
    bool    puntBadTag;                              // @sa RA01_BF_RS_MCS_CPM_RX_CPU_PUNT_ENABLE_PUNT_BAD_TAG
    bool    puntScCamMiss;                           // @sa RA01_BF_RS_MCS_CPM_RX_CPU_PUNT_ENABLE_PUNT_SC_CAM_MISS
    bool    puntSaNotInUse;                          // @sa RA01_BF_RS_MCS_CPM_RX_CPU_PUNT_ENABLE_PUNT_SA_NOT_IN_USE
} Ra01RsMcsCpmRxCpuPuntEnable_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave cpu_punt_enable
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * cpu_punt_enable are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxCpuPuntEnable( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxCpuPuntEnable_t * value,
  ra01_reg_access_op_t op);







/* @brief enable_rxmcs_insertion bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave enable_rxmcs_insertion 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    bool    insertOnErr;                             // @sa RA01_BF_RS_MCS_CPM_RX_ENABLE_RXMCS_INSERTION_INSERT_ON_ERR
    bool    insertOnKayCtrl;                         // @sa RA01_BF_RS_MCS_CPM_RX_ENABLE_RXMCS_INSERTION_INSERT_ON_KAY_CTRL
} Ra01RsMcsCpmRxEnableRxmcsInsertion_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave enable_rxmcs_insertion
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * enable_rxmcs_insertion are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxEnableRxmcsInsertion( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxEnableRxmcsInsertion_t * value,
  ra01_reg_access_op_t op);







/* @brief rx_etype bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave rx_etype 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint16_t    rxMcsHdrEtype;                           // @sa RA01_BF_RS_MCS_CPM_RX_RX_ETYPE_RX_MCS_HDR_ETYPE
} Ra01RsMcsCpmRxRxEtype_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave rx_etype
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * rx_etype are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxRxEtype( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxRxEtype_t * value,
  ra01_reg_access_op_t op);







/* @brief rx_default_sci bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave rx_default_sci 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    rxDefaultSciLsb;                         // @sa RA01_BF_RS_MCS_CPM_RX_RX_DEFAULT_SCI_RX_DEFAULT_SCI_LSB
    uint32_t    rxDefaultSciMsb;                         // @sa RA01_BF_RS_MCS_CPM_RX_RX_DEFAULT_SCI_RX_DEFAULT_SCI_MSB
} Ra01RsMcsCpmRxRxDefaultSci_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave rx_default_sci
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * rx_default_sci are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxRxDefaultSci( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxRxDefaultSci_t * value,
  ra01_reg_access_op_t op);







/* @brief xpn_threshold bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave xpn_threshold 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    xpnThresholdLsb;                         // @sa RA01_BF_RS_MCS_CPM_RX_XPN_THRESHOLD_XPN_THRESHOLD_LSB
    uint32_t    xpnThresholdMsb;                         // @sa RA01_BF_RS_MCS_CPM_RX_XPN_THRESHOLD_XPN_THRESHOLD_MSB
} Ra01RsMcsCpmRxXpnThreshold_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave xpn_threshold
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * xpn_threshold are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxXpnThreshold( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxXpnThreshold_t * value,
  ra01_reg_access_op_t op);







/* @brief pn_threshold bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave pn_threshold 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    pnThreshold;                             // @sa RA01_BF_RS_MCS_CPM_RX_PN_THRESHOLD_PN_THRESHOLD
} Ra01RsMcsCpmRxPnThreshold_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave pn_threshold
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pn_threshold are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxPnThreshold( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxPnThreshold_t * value,
  ra01_reg_access_op_t op);







/* @brief pn_thresh_reached bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave pn_thresh_reached 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint64_t    sa;                                      // @sa RA01_BF_RS_MCS_CPM_RX_PN_THRESH_REACHED_SA
} Ra01RsMcsCpmRxPnThreshReached_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave pn_thresh_reached
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pn_thresh_reached are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01GetRsMcsCpmRxPnThreshReached( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxPnThreshReached_t * value);







/* @brief sa_key_lockout bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sa_key_lockout 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint64_t    sa;                                      // @sa RA01_BF_RS_MCS_CPM_RX_SA_KEY_LOCKOUT_SA
} Ra01RsMcsCpmRxSaKeyLockout_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sa_key_lockout
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sa_key_lockout are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxSaKeyLockout( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxSaKeyLockout_t * value,
  ra01_reg_access_op_t op);







/* @brief rx_mcs_base_port bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave rx_mcs_base_port 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint8_t    rxMcsBasePort;                           // @sa RA01_BF_RS_MCS_CPM_RX_RX_MCS_BASE_PORT_RX_MCS_BASE_PORT
} Ra01RsMcsCpmRxRxMcsBasePort_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave rx_mcs_base_port
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * rx_mcs_base_port are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxRxMcsBasePort( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxRxMcsBasePort_t * value,
  ra01_reg_access_op_t op);







/* @brief sa_pn_table_update bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sa_pn_table_update 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    bool    maxUpdate;                               // @sa RA01_BF_RS_MCS_CPM_RX_SA_PN_TABLE_UPDATE_MAX_UPDATE
} Ra01RsMcsCpmRxSaPnTableUpdate_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sa_pn_table_update
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sa_pn_table_update are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxSaPnTableUpdate( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxSaPnTableUpdate_t * value,
  ra01_reg_access_op_t op);







/* @brief rx_flowid_tcam_enable bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave rx_flowid_tcam_enable 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    enable;                                  // @sa RA01_BF_RS_MCS_CPM_RX_RX_FLOWID_TCAM_ENABLE_ENABLE
} Ra01RsMcsCpmRxRxFlowidTcamEnable_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave rx_flowid_tcam_enable
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * rx_flowid_tcam_enable are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxRxFlowidTcamEnable( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxRxFlowidTcamEnable_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_cam_enable bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_cam_enable 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    enable;                                  // @sa RA01_BF_RS_MCS_CPM_RX_SC_CAM_ENABLE_ENABLE
} Ra01RsMcsCpmRxScCamEnable_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_cam_enable
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_cam_enable are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScCamEnable( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScCamEnable_t * value,
  ra01_reg_access_op_t op);







/* @brief time_unit_tick bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave time_unit_tick 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    numCycles;                               // @sa RA01_BF_RS_MCS_CPM_RX_TIME_UNIT_TICK_NUM_CYCLES
} Ra01RsMcsCpmRxTimeUnitTick_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave time_unit_tick
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * time_unit_tick are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxTimeUnitTick( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxTimeUnitTick_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_timer_reset_all_go bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_timer_reset_all_go 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    bool    reset;                                   // @sa RA01_BF_RS_MCS_CPM_RX_SC_TIMER_RESET_ALL_GO_RESET
} Ra01RsMcsCpmRxScTimerResetAllGo_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_timer_reset_all_go
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_timer_reset_all_go are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScTimerResetAllGo( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxScTimerResetAllGo_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief sc_timer_rsvd1 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_timer_rsvd1 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    rsvd;                                    // @sa RA01_BF_RS_MCS_CPM_RX_SC_TIMER_RSVD1_RSVD
} Ra01RsMcsCpmRxScTimerRsvd1_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_timer_rsvd1
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_timer_rsvd1 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScTimerRsvd1( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxScTimerRsvd1_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief sc_timer_rsvd2 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_timer_rsvd2 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    rsvd2;                                   // @sa RA01_BF_RS_MCS_CPM_RX_SC_TIMER_RSVD2_RSVD2
} Ra01RsMcsCpmRxScTimerRsvd2_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_timer_rsvd2
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_timer_rsvd2 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScTimerRsvd2( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxScTimerRsvd2_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief sc_timer_timeout_thresh bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave sc_timer_timeout_thresh 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    threshold;                               // @sa RA01_BF_RS_MCS_CPM_RX_SC_TIMER_TIMEOUT_THRESH_THRESHOLD
} Ra01RsMcsCpmRxScTimerTimeoutThresh_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave sc_timer_timeout_thresh
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_timer_timeout_thresh are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxScTimerTimeoutThresh( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxScTimerTimeoutThresh_t * value,
  ra01_reg_access_op_t op);



















/* @brief packet_seen_rw_an0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave packet_seen_rw_an0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    sc;                                      // @sa RA01_BF_RS_MCS_CPM_RX_PACKET_SEEN_RW_AN0_SC
} Ra01RsMcsCpmRxPacketSeenRwAn0_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave packet_seen_rw_an0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * packet_seen_rw_an0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxPacketSeenRwAn0( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxPacketSeenRwAn0_t * value,
  ra01_reg_access_op_t op);







/* @brief packet_seen_rw_an1 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave packet_seen_rw_an1 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    sc;                                      // @sa RA01_BF_RS_MCS_CPM_RX_PACKET_SEEN_RW_AN1_SC
} Ra01RsMcsCpmRxPacketSeenRwAn1_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave packet_seen_rw_an1
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * packet_seen_rw_an1 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxPacketSeenRwAn1( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxPacketSeenRwAn1_t * value,
  ra01_reg_access_op_t op);







/* @brief packet_seen_rw_an2 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave packet_seen_rw_an2 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    sc;                                      // @sa RA01_BF_RS_MCS_CPM_RX_PACKET_SEEN_RW_AN2_SC
} Ra01RsMcsCpmRxPacketSeenRwAn2_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave packet_seen_rw_an2
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * packet_seen_rw_an2 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxPacketSeenRwAn2( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxPacketSeenRwAn2_t * value,
  ra01_reg_access_op_t op);







/* @brief packet_seen_rw_an3 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave packet_seen_rw_an3 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    sc;                                      // @sa RA01_BF_RS_MCS_CPM_RX_PACKET_SEEN_RW_AN3_SC
} Ra01RsMcsCpmRxPacketSeenRwAn3_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave packet_seen_rw_an3
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * packet_seen_rw_an3 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxPacketSeenRwAn3( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxPacketSeenRwAn3_t * value,
  ra01_reg_access_op_t op);







/* @brief packet_seen_w1c_an0 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave packet_seen_w1c_an0 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    sc;                                      // @sa RA01_BF_RS_MCS_CPM_RX_PACKET_SEEN_W1C_AN0_SC
} Ra01RsMcsCpmRxPacketSeenW1cAn0_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave packet_seen_w1c_an0
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * packet_seen_w1c_an0 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxPacketSeenW1cAn0( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxPacketSeenW1cAn0_t * value,
  ra01_reg_access_op_t op);







/* @brief packet_seen_w1c_an1 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave packet_seen_w1c_an1 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    sc;                                      // @sa RA01_BF_RS_MCS_CPM_RX_PACKET_SEEN_W1C_AN1_SC
} Ra01RsMcsCpmRxPacketSeenW1cAn1_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave packet_seen_w1c_an1
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * packet_seen_w1c_an1 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxPacketSeenW1cAn1( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxPacketSeenW1cAn1_t * value,
  ra01_reg_access_op_t op);







/* @brief packet_seen_w1c_an2 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave packet_seen_w1c_an2 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    sc;                                      // @sa RA01_BF_RS_MCS_CPM_RX_PACKET_SEEN_W1C_AN2_SC
} Ra01RsMcsCpmRxPacketSeenW1cAn2_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave packet_seen_w1c_an2
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * packet_seen_w1c_an2 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxPacketSeenW1cAn2( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxPacketSeenW1cAn2_t * value,
  ra01_reg_access_op_t op);







/* @brief packet_seen_w1c_an3 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave packet_seen_w1c_an3 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint32_t    sc;                                      // @sa RA01_BF_RS_MCS_CPM_RX_PACKET_SEEN_W1C_AN3_SC
} Ra01RsMcsCpmRxPacketSeenW1cAn3_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave packet_seen_w1c_an3
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * packet_seen_w1c_an3 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxPacketSeenW1cAn3( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmRxPacketSeenW1cAn3_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief dbg_mux_sel bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave dbg_mux_sel 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint8_t    seg0;                                    // @sa RA01_BF_RS_MCS_CPM_RX_DBG_MUX_SEL_SEG0
    uint8_t    seg1;                                    // @sa RA01_BF_RS_MCS_CPM_RX_DBG_MUX_SEL_SEG1
    uint8_t    seg2;                                    // @sa RA01_BF_RS_MCS_CPM_RX_DBG_MUX_SEL_SEG2
    uint8_t    seg3;                                    // @sa RA01_BF_RS_MCS_CPM_RX_DBG_MUX_SEL_SEG3
} Ra01RsMcsCpmRxDbgMuxSel_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave dbg_mux_sel
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * dbg_mux_sel are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01AccRsMcsCpmRxDbgMuxSel( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxDbgMuxSel_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief debug_status bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_rx_slave debug_status 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_CPM_RX_DEBUG_STATUS_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_CPM_RX_DEBUG_STATUS_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_CPM_RX_DEBUG_STATUS_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_CPM_RX_DEBUG_STATUS_SEG3
} Ra01RsMcsCpmRxDebugStatus_t;

/* @brief Set the value of rs_mcs_cpm_rx_slave debug_status
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * debug_status are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 * 
*/
int Ra01GetRsMcsCpmRxDebugStatus( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmRxDebugStatus_t * value);


#endif // MCS_API_DEBUG


/* @brief Enumerate the rs_mcs_cpm_tx_slave instances.
 *
 * This enum is used as in index into the per-slave look arrays
 */
typedef enum {
    RA01_RS_MCS_CPM_TX_SLAVE = 0,  // Instance 0 of rs_mcs_cpm_tx_slave

    RA01_RS_MCS_CPM_TX_INSTANCE_MAX
} ra01_rs_mcs_cpm_tx_slave_instance_t;
/*-------------------------------------------------------------------------------
 * External APIs 
 *------------------------------------------------------------------------------*/
/* This is a little ugly, but until we can rewrite the software support both
 * camel and snake case for the register bitfield struct...
 */

/* @brief Enumerate the instances of rs_mcs_cpm_tx_slave in the system.
 *
 * The instance names are used to direct the read/write functions to the correct
 * register set. 
 */
typedef ra01_rs_mcs_cpm_tx_slave_instance_t Ra01RsMcsCpmTxInstance_t;




/* @brief cpm_tx_int bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave cpm_tx_int 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    bool    packetXpnEq0;                            // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_PACKET_XPN_EQ0
    bool    pnThreshReached;                         // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_PN_THRESH_REACHED
    bool    saNotValid;                              // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_SA_NOT_VALID
    bool    scExpiryPreTimeout;                      // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_SC_EXPIRY_PRE_TIMEOUT
    bool    scExpiryActualTimeout;                   // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_SC_EXPIRY_ACTUAL_TIMEOUT
} Ra01RsMcsCpmTxCpmTxInt_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave cpm_tx_int
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * cpm_tx_int are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxCpmTxInt( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxCpmTxInt_t * value,
  ra01_reg_access_op_t op);







/* @brief cpm_tx_int_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave cpm_tx_int_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    bool    packetXpnEq0Enb;                         // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_ENB_PACKET_XPN_EQ0_ENB
    bool    pnThreshReachedEnb;                      // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_ENB_PN_THRESH_REACHED_ENB
    bool    saNotValidEnb;                           // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_ENB_SA_NOT_VALID_ENB
    bool    scExpiryPreTimeoutEnb;                   // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_ENB_SC_EXPIRY_PRE_TIMEOUT_ENB
    bool    scExpiryActualTimeoutEnb;                // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_ENB_SC_EXPIRY_ACTUAL_TIMEOUT_ENB
} Ra01RsMcsCpmTxCpmTxIntEnb_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave cpm_tx_int_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * cpm_tx_int_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxCpmTxIntEnb( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxCpmTxIntEnb_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief cpm_tx_int_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave cpm_tx_int_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    bool    packetXpnEq0;                            // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_RAW_PACKET_XPN_EQ0
    bool    pnThreshReached;                         // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_RAW_PN_THRESH_REACHED
    bool    saNotValid;                              // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_RAW_SA_NOT_VALID
    bool    scExpiryPreTimeout;                      // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_RAW_SC_EXPIRY_PRE_TIMEOUT
    bool    scExpiryActualTimeout;                   // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_RAW_SC_EXPIRY_ACTUAL_TIMEOUT
} Ra01RsMcsCpmTxCpmTxIntRaw_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave cpm_tx_int_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * cpm_tx_int_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01GetRsMcsCpmTxCpmTxIntRaw( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxCpmTxIntRaw_t * value);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief cpm_tx_int_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave cpm_tx_int_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    bool    packetXpnEq0;                            // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_INTR_RW_PACKET_XPN_EQ0
    bool    pnThreshReached;                         // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_INTR_RW_PN_THRESH_REACHED
    bool    saNotValid;                              // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_INTR_RW_SA_NOT_VALID
    bool    scExpiryPreTimeout;                      // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_INTR_RW_SC_EXPIRY_PRE_TIMEOUT
    bool    scExpiryActualTimeout;                   // @sa RA01_BF_RS_MCS_CPM_TX_CPM_TX_INT_INTR_RW_SC_EXPIRY_ACTUAL_TIMEOUT
} Ra01RsMcsCpmTxCpmTxIntIntrRw_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave cpm_tx_int_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * cpm_tx_int_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxCpmTxIntIntrRw( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxCpmTxIntIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief sc_expiry_pre_timeout bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_expiry_pre_timeout 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryPreTimeoutInt;                   // @sa RA01_BF_RS_MCS_CPM_TX_SC_EXPIRY_PRE_TIMEOUT_SC_EXPIRY_PRE_TIMEOUT_INT
} Ra01RsMcsCpmTxScExpiryPreTimeout_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_expiry_pre_timeout
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_pre_timeout are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScExpiryPreTimeout( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxScExpiryPreTimeout_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_expiry_pre_timeout_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_expiry_pre_timeout_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryPreTimeoutIntEnb;                // @sa RA01_BF_RS_MCS_CPM_TX_SC_EXPIRY_PRE_TIMEOUT_ENB_SC_EXPIRY_PRE_TIMEOUT_INT_ENB
} Ra01RsMcsCpmTxScExpiryPreTimeoutEnb_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_expiry_pre_timeout_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_pre_timeout_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScExpiryPreTimeoutEnb( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxScExpiryPreTimeoutEnb_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_expiry_pre_timeout_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_expiry_pre_timeout_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryPreTimeoutInt;                   // @sa RA01_BF_RS_MCS_CPM_TX_SC_EXPIRY_PRE_TIMEOUT_RAW_SC_EXPIRY_PRE_TIMEOUT_INT
} Ra01RsMcsCpmTxScExpiryPreTimeoutRaw_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_expiry_pre_timeout_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_pre_timeout_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01GetRsMcsCpmTxScExpiryPreTimeoutRaw( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxScExpiryPreTimeoutRaw_t * value);




#ifdef MCS_API_DEBUG



/* @brief sc_expiry_pre_timeout_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_expiry_pre_timeout_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryPreTimeoutInt;                   // @sa RA01_BF_RS_MCS_CPM_TX_SC_EXPIRY_PRE_TIMEOUT_INTR_RW_SC_EXPIRY_PRE_TIMEOUT_INT
} Ra01RsMcsCpmTxScExpiryPreTimeoutIntrRw_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_expiry_pre_timeout_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_pre_timeout_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScExpiryPreTimeoutIntrRw( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxScExpiryPreTimeoutIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief sc_expiry_actual_timeout bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_expiry_actual_timeout 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryActualTimeoutInt;                // @sa RA01_BF_RS_MCS_CPM_TX_SC_EXPIRY_ACTUAL_TIMEOUT_SC_EXPIRY_ACTUAL_TIMEOUT_INT
} Ra01RsMcsCpmTxScExpiryActualTimeout_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_expiry_actual_timeout
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_actual_timeout are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScExpiryActualTimeout( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxScExpiryActualTimeout_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_expiry_actual_timeout_enb bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_expiry_actual_timeout_enb 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryActualTimeoutIntEnb;             // @sa RA01_BF_RS_MCS_CPM_TX_SC_EXPIRY_ACTUAL_TIMEOUT_ENB_SC_EXPIRY_ACTUAL_TIMEOUT_INT_ENB
} Ra01RsMcsCpmTxScExpiryActualTimeoutEnb_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_expiry_actual_timeout_enb
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_actual_timeout_enb are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScExpiryActualTimeoutEnb( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxScExpiryActualTimeoutEnb_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_expiry_actual_timeout_raw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_expiry_actual_timeout_raw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryActualTimeoutInt;                // @sa RA01_BF_RS_MCS_CPM_TX_SC_EXPIRY_ACTUAL_TIMEOUT_RAW_SC_EXPIRY_ACTUAL_TIMEOUT_INT
} Ra01RsMcsCpmTxScExpiryActualTimeoutRaw_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_expiry_actual_timeout_raw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_actual_timeout_raw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01GetRsMcsCpmTxScExpiryActualTimeoutRaw( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxScExpiryActualTimeoutRaw_t * value);




#ifdef MCS_API_DEBUG



/* @brief sc_expiry_actual_timeout_intr_rw bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_expiry_actual_timeout_intr_rw 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    scExpiryActualTimeoutInt;                // @sa RA01_BF_RS_MCS_CPM_TX_SC_EXPIRY_ACTUAL_TIMEOUT_INTR_RW_SC_EXPIRY_ACTUAL_TIMEOUT_INT
} Ra01RsMcsCpmTxScExpiryActualTimeoutIntrRw_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_expiry_actual_timeout_intr_rw
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_expiry_actual_timeout_intr_rw are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScExpiryActualTimeoutIntrRw( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxScExpiryActualTimeoutIntrRw_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief tx_port_cfg bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave tx_port_cfg 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint16_t    sectagEtype;                             // @sa RA01_BF_RS_MCS_CPM_TX_TX_PORT_CFG_SECTAG_ETYPE
} Ra01RsMcsCpmTxTxPortCfg_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave tx_port_cfg
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * tx_port_cfg are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxTxPortCfg( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxTxPortCfg_t * value,
  ra01_reg_access_op_t op);







/* @brief xpn_threshold bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave xpn_threshold 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    xpnThresholdLsb;                         // @sa RA01_BF_RS_MCS_CPM_TX_XPN_THRESHOLD_XPN_THRESHOLD_LSB
    uint32_t    xpnThresholdMsb;                         // @sa RA01_BF_RS_MCS_CPM_TX_XPN_THRESHOLD_XPN_THRESHOLD_MSB
} Ra01RsMcsCpmTxXpnThreshold_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave xpn_threshold
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * xpn_threshold are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxXpnThreshold( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxXpnThreshold_t * value,
  ra01_reg_access_op_t op);







/* @brief pn_threshold bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave pn_threshold 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    pnThreshold;                             // @sa RA01_BF_RS_MCS_CPM_TX_PN_THRESHOLD_PN_THRESHOLD
} Ra01RsMcsCpmTxPnThreshold_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave pn_threshold
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * pn_threshold are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxPnThreshold( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxPnThreshold_t * value,
  ra01_reg_access_op_t op);







/* @brief sa_key_lockout bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sa_key_lockout 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint64_t    sa;                                      // @sa RA01_BF_RS_MCS_CPM_TX_SA_KEY_LOCKOUT_SA
} Ra01RsMcsCpmTxSaKeyLockout_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sa_key_lockout
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sa_key_lockout are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxSaKeyLockout( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxSaKeyLockout_t * value,
  ra01_reg_access_op_t op);







/* @brief fixed_offset_adjust bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave fixed_offset_adjust 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint8_t    fixedOffsetAdjust;                       // @sa RA01_BF_RS_MCS_CPM_TX_FIXED_OFFSET_ADJUST_FIXED_OFFSET_ADJUST
} Ra01RsMcsCpmTxFixedOffsetAdjust_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave fixed_offset_adjust
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * fixed_offset_adjust are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxFixedOffsetAdjust( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxFixedOffsetAdjust_t * value,
  ra01_reg_access_op_t op);







/* @brief tx_flowid_tcam_enable bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave tx_flowid_tcam_enable 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    enable;                                  // @sa RA01_BF_RS_MCS_CPM_TX_TX_FLOWID_TCAM_ENABLE_ENABLE
} Ra01RsMcsCpmTxTxFlowidTcamEnable_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave tx_flowid_tcam_enable
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * tx_flowid_tcam_enable are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxTxFlowidTcamEnable( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxTxFlowidTcamEnable_t * value,
  ra01_reg_access_op_t op);







/* @brief time_unit_tick bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave time_unit_tick 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    numCycles;                               // @sa RA01_BF_RS_MCS_CPM_TX_TIME_UNIT_TICK_NUM_CYCLES
} Ra01RsMcsCpmTxTimeUnitTick_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave time_unit_tick
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * time_unit_tick are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxTimeUnitTick( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxTimeUnitTick_t * value,
  ra01_reg_access_op_t op);







/* @brief sc_timer_reset_all_go bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_timer_reset_all_go 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    bool    reset;                                   // @sa RA01_BF_RS_MCS_CPM_TX_SC_TIMER_RESET_ALL_GO_RESET
} Ra01RsMcsCpmTxScTimerResetAllGo_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_timer_reset_all_go
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_timer_reset_all_go are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScTimerResetAllGo( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxScTimerResetAllGo_t * value,
  ra01_reg_access_op_t op);




#ifdef MCS_API_DEBUG



/* @brief sc_timer_rsvd1 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_timer_rsvd1 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    rsvd1;                                   // @sa RA01_BF_RS_MCS_CPM_TX_SC_TIMER_RSVD1_RSVD1
} Ra01RsMcsCpmTxScTimerRsvd1_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_timer_rsvd1
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_timer_rsvd1 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScTimerRsvd1( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxScTimerRsvd1_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief sc_timer_rsvd2 bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_timer_rsvd2 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    rsvd2;                                   // @sa RA01_BF_RS_MCS_CPM_TX_SC_TIMER_RSVD2_RSVD2
} Ra01RsMcsCpmTxScTimerRsvd2_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_timer_rsvd2
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_timer_rsvd2 are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScTimerRsvd2( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxScTimerRsvd2_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG





/* @brief sc_timer_timeout_thresh bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave sc_timer_timeout_thresh 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint32_t    threshold;                               // @sa RA01_BF_RS_MCS_CPM_TX_SC_TIMER_TIMEOUT_THRESH_THRESHOLD
} Ra01RsMcsCpmTxScTimerTimeoutThresh_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave sc_timer_timeout_thresh
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[in]    index     Which index of the register to access
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * sc_timer_timeout_thresh are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxScTimerTimeoutThresh( RmsDev_t * rmsDev_p, 
  unsigned index,
  Ra01RsMcsCpmTxScTimerTimeoutThresh_t * value,
  ra01_reg_access_op_t op);
















#ifdef MCS_API_DEBUG



/* @brief dbg_mux_sel bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave dbg_mux_sel 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint8_t    seg0;                                    // @sa RA01_BF_RS_MCS_CPM_TX_DBG_MUX_SEL_SEG0
    uint8_t    seg1;                                    // @sa RA01_BF_RS_MCS_CPM_TX_DBG_MUX_SEL_SEG1
    uint8_t    seg2;                                    // @sa RA01_BF_RS_MCS_CPM_TX_DBG_MUX_SEL_SEG2
    uint8_t    seg3;                                    // @sa RA01_BF_RS_MCS_CPM_TX_DBG_MUX_SEL_SEG3
} Ra01RsMcsCpmTxDbgMuxSel_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave dbg_mux_sel
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * @param[in]    op        Operation to perform (read or write)
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * dbg_mux_sel are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01AccRsMcsCpmTxDbgMuxSel( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxDbgMuxSel_t * value,
  ra01_reg_access_op_t op);


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG



/* @brief debug_status bit fields. 
 *
 * Use this structure to pass the data to rs_mcs_cpm_tx_slave debug_status 
 * for wrting and a pointer for reading.
 * NOTE: this structure is not packed. Ths bits are packed in the software. C99 bit
 *       field packing is compiler dependent. 
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 *
 */
typedef struct {
    uint16_t    seg0;                                    // @sa RA01_BF_RS_MCS_CPM_TX_DEBUG_STATUS_SEG0
    uint16_t    seg1;                                    // @sa RA01_BF_RS_MCS_CPM_TX_DEBUG_STATUS_SEG1
    uint16_t    seg2;                                    // @sa RA01_BF_RS_MCS_CPM_TX_DEBUG_STATUS_SEG2
    uint16_t    seg3;                                    // @sa RA01_BF_RS_MCS_CPM_TX_DEBUG_STATUS_SEG3
} Ra01RsMcsCpmTxDebugStatus_t;

/* @brief Set the value of rs_mcs_cpm_tx_slave debug_status
 * 
 * @param[inout] rmsDev_p  Pointer for hardware write use
 * @param[out]   value     Pointer of value to write.
 * 
 * The structure pointed to by value will be repacked to the bitfield automatically. All bits of
 * debug_status are written.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 * 
*/
int Ra01GetRsMcsCpmTxDebugStatus( RmsDev_t * rmsDev_p, 
  Ra01RsMcsCpmTxDebugStatus_t * value);


#endif // MCS_API_DEBUG


/********************************************************************************
 * Convert memories from HW format (raw) to software format (cooked)
 ********************************************************************************/
 // software API structures:

/* @brief Struct mapping of IFINUNCTLOCTETS
 *
 * This struct can be used to set the values for IFINUNCTLOCTETS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxUnctlOctetCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_IFINUNCTLOCTETS_RXUNCTLOCTETCNT
} Ra01IfRsMcsCseRxMemIfinunctloctets_t;


/* @brief Read a table entry and convert it to a IFINUNCTLOCTETS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemIfinunctloctets_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemIfinunctloctets struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's IFINUNCTLOCTETS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemIfinunctloctets_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemIfinunctloctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinunctloctets_t * data );



/* @brief Struct mapping of IFINCTLOCTETS
 *
 * This struct can be used to set the values for IFINCTLOCTETS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxCtlOctetCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_IFINCTLOCTETS_RXCTLOCTETCNT
} Ra01IfRsMcsCseRxMemIfinctloctets_t;


/* @brief Read a table entry and convert it to a IFINCTLOCTETS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemIfinctloctets_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemIfinctloctets struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's IFINCTLOCTETS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemIfinctloctets_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemIfinctloctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinctloctets_t * data );



/* @brief Struct mapping of IFINUNCTLUCPKTS
 *
 * This struct can be used to set the values for IFINUNCTLUCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxUnctlPktUcastCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_IFINUNCTLUCPKTS_RXUNCTLPKTUCASTCNT
} Ra01IfRsMcsCseRxMemIfinunctlucpkts_t;


/* @brief Read a table entry and convert it to a IFINUNCTLUCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemIfinunctlucpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemIfinunctlucpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's IFINUNCTLUCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemIfinunctlucpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemIfinunctlucpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinunctlucpkts_t * data );



/* @brief Struct mapping of IFINUNCTLMCPKTS
 *
 * This struct can be used to set the values for IFINUNCTLMCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxUnctlPktMcastCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_IFINUNCTLMCPKTS_RXUNCTLPKTMCASTCNT
} Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t;


/* @brief Read a table entry and convert it to a IFINUNCTLMCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemIfinunctlmcpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's IFINUNCTLMCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemIfinunctlmcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t * data );



/* @brief Struct mapping of IFINUNCTLBCPKTS
 *
 * This struct can be used to set the values for IFINUNCTLBCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxUnctlPktBcastCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_IFINUNCTLBCPKTS_RXUNCTLPKTBCASTCNT
} Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t;


/* @brief Read a table entry and convert it to a IFINUNCTLBCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemIfinunctlbcpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's IFINUNCTLBCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemIfinunctlbcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t * data );



/* @brief Struct mapping of IFINCTLUCPKTS
 *
 * This struct can be used to set the values for IFINCTLUCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxCtlPktUcastCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_IFINCTLUCPKTS_RXCTLPKTUCASTCNT
} Ra01IfRsMcsCseRxMemIfinctlucpkts_t;


/* @brief Read a table entry and convert it to a IFINCTLUCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemIfinctlucpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemIfinctlucpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's IFINCTLUCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemIfinctlucpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemIfinctlucpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinctlucpkts_t * data );



/* @brief Struct mapping of IFINCTLMCPKTS
 *
 * This struct can be used to set the values for IFINCTLMCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxCtlPktMcastCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_IFINCTLMCPKTS_RXCTLPKTMCASTCNT
} Ra01IfRsMcsCseRxMemIfinctlmcpkts_t;


/* @brief Read a table entry and convert it to a IFINCTLMCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemIfinctlmcpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemIfinctlmcpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's IFINCTLMCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemIfinctlmcpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemIfinctlmcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinctlmcpkts_t * data );



/* @brief Struct mapping of IFINCTLBCPKTS
 *
 * This struct can be used to set the values for IFINCTLBCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxCtlPktBcastCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_IFINCTLBCPKTS_RXCTLPKTBCASTCNT
} Ra01IfRsMcsCseRxMemIfinctlbcpkts_t;


/* @brief Read a table entry and convert it to a IFINCTLBCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemIfinctlbcpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemIfinctlbcpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's IFINCTLBCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemIfinctlbcpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemIfinctlbcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinctlbcpkts_t * data );



/* @brief Struct mapping of INPKTSSECYUNTAGGEDORNOTAG
 *
 * This struct can be used to set the values for INPKTSSECYUNTAGGEDORNOTAG and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyPktUntaggedCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSECYUNTAGGEDORNOTAG_RXSECYPKTUNTAGGEDCNT
} Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t;


/* @brief Read a table entry and convert it to a INPKTSSECYUNTAGGEDORNOTAG struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSECYUNTAGGEDORNOTAG memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssecyuntaggedornotag( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t * data );



/* @brief Struct mapping of INPKTSSECYBADTAG
 *
 * This struct can be used to set the values for INPKTSSECYBADTAG and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyPktBadtagCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSECYBADTAG_RXSECYPKTBADTAGCNT
} Ra01IfRsMcsCseRxMemInpktssecybadtag_t;


/* @brief Read a table entry and convert it to a INPKTSSECYBADTAG struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssecybadtag_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssecybadtag struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSECYBADTAG memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssecybadtag_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssecybadtag( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecybadtag_t * data );



/* @brief Struct mapping of INPKTSSECYCTL
 *
 * This struct can be used to set the values for INPKTSSECYCTL and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyPktCtlCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSECYCTL_RXSECYPKTCTLCNT
} Ra01IfRsMcsCseRxMemInpktssecyctl_t;


/* @brief Read a table entry and convert it to a INPKTSSECYCTL struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssecyctl_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssecyctl struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSECYCTL memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssecyctl_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssecyctl( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecyctl_t * data );



/* @brief Struct mapping of INPKTSSECYTAGGEDCTL
 *
 * This struct can be used to set the values for INPKTSSECYTAGGEDCTL and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyPktTaggedCtlCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSECYTAGGEDCTL_RXSECYPKTTAGGEDCTLCNT
} Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t;


/* @brief Read a table entry and convert it to a INPKTSSECYTAGGEDCTL struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssecytaggedctl struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSECYTAGGEDCTL memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssecytaggedctl( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t * data );



/* @brief Struct mapping of INPKTSSECYUNKNOWNSCI
 *
 * This struct can be used to set the values for INPKTSSECYUNKNOWNSCI and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyPktNosaCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSECYUNKNOWNSCI_RXSECYPKTNOSACNT
} Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t;


/* @brief Read a table entry and convert it to a INPKTSSECYUNKNOWNSCI struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssecyunknownsci struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSECYUNKNOWNSCI memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssecyunknownsci( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t * data );



/* @brief Struct mapping of INPKTSSECYNOSCI
 *
 * This struct can be used to set the values for INPKTSSECYNOSCI and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyPktNosaerrorCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSECYNOSCI_RXSECYPKTNOSAERRORCNT
} Ra01IfRsMcsCseRxMemInpktssecynosci_t;


/* @brief Read a table entry and convert it to a INPKTSSECYNOSCI struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssecynosci_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssecynosci struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSECYNOSCI memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssecynosci_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssecynosci( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecynosci_t * data );



/* @brief Struct mapping of INPKTSCTRLPORTDISABLED
 *
 * This struct can be used to set the values for INPKTSCTRLPORTDISABLED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyPktCtrlPortDisabledCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSCTRLPORTDISABLED_RXSECYPKTCTRLPORTDISABLEDCNT
} Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t;


/* @brief Read a table entry and convert it to a INPKTSCTRLPORTDISABLED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktsctrlportdisabled struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSCTRLPORTDISABLED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktsctrlportdisabled( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t * data );



/* @brief Struct mapping of INOCTETSSCVALIDATE
 *
 * This struct can be used to set the values for INOCTETSSCVALIDATE and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyOctetValidatedCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INOCTETSSCVALIDATE_RXSECYOCTETVALIDATEDCNT
} Ra01IfRsMcsCseRxMemInoctetsscvalidate_t;


/* @brief Read a table entry and convert it to a INOCTETSSCVALIDATE struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInoctetsscvalidate_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInoctetsscvalidate struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INOCTETSSCVALIDATE memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInoctetsscvalidate_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInoctetsscvalidate( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInoctetsscvalidate_t * data );



/* @brief Struct mapping of INOCTETSSCDECRYPTED
 *
 * This struct can be used to set the values for INOCTETSSCDECRYPTED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyOctetDecryptedCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INOCTETSSCDECRYPTED_RXSECYOCTETDECRYPTEDCNT
} Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t;


/* @brief Read a table entry and convert it to a INOCTETSSCDECRYPTED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInoctetsscdecrypted struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INOCTETSSCDECRYPTED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInoctetsscdecrypted( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t * data );



/* @brief Struct mapping of INPKTSSCUNCHECKED
 *
 * This struct can be used to set the values for INPKTSSCUNCHECKED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxScPktUncheckedCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSCUNCHECKED_RXSCPKTUNCHECKEDCNT
} Ra01IfRsMcsCseRxMemInpktsscunchecked_t;


/* @brief Read a table entry and convert it to a INPKTSSCUNCHECKED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktsscunchecked_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktsscunchecked struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSCUNCHECKED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktsscunchecked_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktsscunchecked( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsscunchecked_t * data );



/* @brief Struct mapping of INPKTSSCLATEORDELAYED
 *
 * This struct can be used to set the values for INPKTSSCLATEORDELAYED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxScPktLateCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSCLATEORDELAYED_RXSCPKTLATECNT
} Ra01IfRsMcsCseRxMemInpktssclateordelayed_t;


/* @brief Read a table entry and convert it to a INPKTSSCLATEORDELAYED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssclateordelayed_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssclateordelayed struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSCLATEORDELAYED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssclateordelayed_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssclateordelayed( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssclateordelayed_t * data );



/* @brief Struct mapping of INPKTSSCCAMHIT
 *
 * This struct can be used to set the values for INPKTSSCCAMHIT and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxScPktScCamHitCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSCCAMHIT_RXSCPKTSCCAMHITCNT
} Ra01IfRsMcsCseRxMemInpktssccamhit_t;


/* @brief Read a table entry and convert it to a INPKTSSCCAMHIT struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssccamhit_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssccamhit struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSCCAMHIT memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssccamhit_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssccamhit( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssccamhit_t * data );



/* @brief Struct mapping of INPKTSSAOK
 *
 * This struct can be used to set the values for INPKTSSAOK and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxScPktOkCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSAOK_RXSCPKTOKCNT
} Ra01IfRsMcsCseRxMemInpktssaok_t;


/* @brief Read a table entry and convert it to a INPKTSSAOK struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssaok_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssaok struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSAOK memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssaok_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssaok( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssaok_t * data );



/* @brief Struct mapping of INPKTSSAINVALID
 *
 * This struct can be used to set the values for INPKTSSAINVALID and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxScPktInvalidCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSAINVALID_RXSCPKTINVALIDCNT
} Ra01IfRsMcsCseRxMemInpktssainvalid_t;


/* @brief Read a table entry and convert it to a INPKTSSAINVALID struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssainvalid_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssainvalid struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSAINVALID memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssainvalid_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssainvalid( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssainvalid_t * data );



/* @brief Struct mapping of INPKTSSANOTVALID
 *
 * This struct can be used to set the values for INPKTSSANOTVALID and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxScPktNotvalidCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSANOTVALID_RXSCPKTNOTVALIDCNT
} Ra01IfRsMcsCseRxMemInpktssanotvalid_t;


/* @brief Read a table entry and convert it to a INPKTSSANOTVALID struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssanotvalid_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssanotvalid struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSANOTVALID memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssanotvalid_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssanotvalid( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssanotvalid_t * data );



/* @brief Struct mapping of INPKTSSAUNUSEDSA
 *
 * This struct can be used to set the values for INPKTSSAUNUSEDSA and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyPktNosaCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSAUNUSEDSA_RXSECYPKTNOSACNT
} Ra01IfRsMcsCseRxMemInpktssaunusedsa_t;


/* @brief Read a table entry and convert it to a INPKTSSAUNUSEDSA struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssaunusedsa_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssaunusedsa struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSAUNUSEDSA memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssaunusedsa_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssaunusedsa( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssaunusedsa_t * data );



/* @brief Struct mapping of INPKTSSANOTUSINGSAERROR
 *
 * This struct can be used to set the values for INPKTSSANOTUSINGSAERROR and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxSecyPktNosaerrorCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSSANOTUSINGSAERROR_RXSECYPKTNOSAERRORCNT
} Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t;


/* @brief Read a table entry and convert it to a INPKTSSANOTUSINGSAERROR struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktssanotusingsaerror struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSSANOTUSINGSAERROR memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktssanotusingsaerror( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t * data );



/* @brief Struct mapping of INPKTSFLOWIDTCAMMISS
 *
 * This struct can be used to set the values for INPKTSFLOWIDTCAMMISS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxPortPktFlowidTcamMissCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSFLOWIDTCAMMISS_RXPORTPKTFLOWIDTCAMMISSCNT
} Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t;


/* @brief Read a table entry and convert it to a INPKTSFLOWIDTCAMMISS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktsflowidtcammiss struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSFLOWIDTCAMMISS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktsflowidtcammiss( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t * data );



/* @brief Struct mapping of INPKTSPARSEERR
 *
 * This struct can be used to set the values for INPKTSPARSEERR and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxPortPktParseErrCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSPARSEERR_RXPORTPKTPARSEERRCNT
} Ra01IfRsMcsCseRxMemInpktsparseerr_t;


/* @brief Read a table entry and convert it to a INPKTSPARSEERR struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktsparseerr_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktsparseerr struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSPARSEERR memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktsparseerr_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktsparseerr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsparseerr_t * data );



/* @brief Struct mapping of INPKTSEARLYPREEMPTERR
 *
 * This struct can be used to set the values for INPKTSEARLYPREEMPTERR and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxPortPktEarlyPreemptErrCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSEARLYPREEMPTERR_RXPORTPKTEARLYPREEMPTERRCNT
} Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t;


/* @brief Read a table entry and convert it to a INPKTSEARLYPREEMPTERR struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktsearlypreempterr struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSEARLYPREEMPTERR memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktsearlypreempterr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t * data );



/* @brief Struct mapping of INPKTSFLOWIDTCAMHIT
 *
 * This struct can be used to set the values for INPKTSFLOWIDTCAMHIT and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_rx_mem_slave
 */
typedef struct {
    uint64_t        rxFlowidPktFlowidTcamHitCnt;  // @sa RA01_MF_RS_MCS_CSE_RX_MEM_INPKTSFLOWIDTCAMHIT_RXFLOWIDPKTFLOWIDTCAMHITCNT
} Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t;


/* @brief Read a table entry and convert it to a INPKTSFLOWIDTCAMHIT struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseRxMemInpktsflowidtcamhit struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_RX_MEM_SLAVE's INPKTSFLOWIDTCAMHIT memory table
 * and reformat it into the passed in Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_rx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseRxMemInpktsflowidtcamhit( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t * data );



/* @brief Struct mapping of IFOUTCOMMONOCTETS
 *
 * This struct can be used to set the values for IFOUTCOMMONOCTETS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txUnctlOctetCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_IFOUTCOMMONOCTETS_TXUNCTLOCTETCNT
} Ra01IfRsMcsCseTxMemIfoutcommonoctets_t;


/* @brief Read a table entry and convert it to a IFOUTCOMMONOCTETS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemIfoutcommonoctets_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemIfoutcommonoctets struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's IFOUTCOMMONOCTETS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemIfoutcommonoctets_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemIfoutcommonoctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutcommonoctets_t * data );



/* @brief Struct mapping of IFOUTUNCTLOCTETS
 *
 * This struct can be used to set the values for IFOUTUNCTLOCTETS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txUnctlOctetCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_IFOUTUNCTLOCTETS_TXUNCTLOCTETCNT
} Ra01IfRsMcsCseTxMemIfoutunctloctets_t;


/* @brief Read a table entry and convert it to a IFOUTUNCTLOCTETS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemIfoutunctloctets_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemIfoutunctloctets struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's IFOUTUNCTLOCTETS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemIfoutunctloctets_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemIfoutunctloctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutunctloctets_t * data );



/* @brief Struct mapping of IFOUTCTLOCTETS
 *
 * This struct can be used to set the values for IFOUTCTLOCTETS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txCtlOctetCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_IFOUTCTLOCTETS_TXCTLOCTETCNT
} Ra01IfRsMcsCseTxMemIfoutctloctets_t;


/* @brief Read a table entry and convert it to a IFOUTCTLOCTETS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemIfoutctloctets_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemIfoutctloctets struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's IFOUTCTLOCTETS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemIfoutctloctets_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemIfoutctloctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutctloctets_t * data );



/* @brief Struct mapping of IFOUTUNCTLUCPKTS
 *
 * This struct can be used to set the values for IFOUTUNCTLUCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txUnctlPktUcastCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_IFOUTUNCTLUCPKTS_TXUNCTLPKTUCASTCNT
} Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t;


/* @brief Read a table entry and convert it to a IFOUTUNCTLUCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemIfoutunctlucpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's IFOUTUNCTLUCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemIfoutunctlucpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t * data );



/* @brief Struct mapping of IFOUTUNCTLMCPKTS
 *
 * This struct can be used to set the values for IFOUTUNCTLMCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txUnctlPktMcastCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_IFOUTUNCTLMCPKTS_TXUNCTLPKTMCASTCNT
} Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t;


/* @brief Read a table entry and convert it to a IFOUTUNCTLMCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemIfoutunctlmcpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's IFOUTUNCTLMCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemIfoutunctlmcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t * data );



/* @brief Struct mapping of IFOUTUNCTLBCPKTS
 *
 * This struct can be used to set the values for IFOUTUNCTLBCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txUnctlPktBcastCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_IFOUTUNCTLBCPKTS_TXUNCTLPKTBCASTCNT
} Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t;


/* @brief Read a table entry and convert it to a IFOUTUNCTLBCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemIfoutunctlbcpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's IFOUTUNCTLBCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemIfoutunctlbcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t * data );



/* @brief Struct mapping of IFOUTCTLUCPKTS
 *
 * This struct can be used to set the values for IFOUTCTLUCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txCtlPktUcastCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_IFOUTCTLUCPKTS_TXCTLPKTUCASTCNT
} Ra01IfRsMcsCseTxMemIfoutctlucpkts_t;


/* @brief Read a table entry and convert it to a IFOUTCTLUCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemIfoutctlucpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemIfoutctlucpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's IFOUTCTLUCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemIfoutctlucpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemIfoutctlucpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutctlucpkts_t * data );



/* @brief Struct mapping of IFOUTCTLMCPKTS
 *
 * This struct can be used to set the values for IFOUTCTLMCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txCtlPktMcastCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_IFOUTCTLMCPKTS_TXCTLPKTMCASTCNT
} Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t;


/* @brief Read a table entry and convert it to a IFOUTCTLMCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemIfoutctlmcpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's IFOUTCTLMCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemIfoutctlmcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t * data );



/* @brief Struct mapping of IFOUTCTLBCPKTS
 *
 * This struct can be used to set the values for IFOUTCTLBCPKTS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txCtlPktBcastCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_IFOUTCTLBCPKTS_TXCTLPKTBCASTCNT
} Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t;


/* @brief Read a table entry and convert it to a IFOUTCTLBCPKTS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemIfoutctlbcpkts struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's IFOUTCTLBCPKTS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemIfoutctlbcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t * data );



/* @brief Struct mapping of OUTPKTSSECYUNTAGGED
 *
 * This struct can be used to set the values for OUTPKTSSECYUNTAGGED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txSecyPktUntaggedCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSSECYUNTAGGED_TXSECYPKTUNTAGGEDCNT
} Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t;


/* @brief Read a table entry and convert it to a OUTPKTSSECYUNTAGGED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktssecyuntagged struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSSECYUNTAGGED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktssecyuntagged( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t * data );



/* @brief Struct mapping of OUTPKTSSECYTOOLONG
 *
 * This struct can be used to set the values for OUTPKTSSECYTOOLONG and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txSecyPktToolongCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSSECYTOOLONG_TXSECYPKTTOOLONGCNT
} Ra01IfRsMcsCseTxMemOutpktssecytoolong_t;


/* @brief Read a table entry and convert it to a OUTPKTSSECYTOOLONG struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktssecytoolong_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktssecytoolong struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSSECYTOOLONG memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktssecytoolong_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktssecytoolong( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssecytoolong_t * data );



/* @brief Struct mapping of OUTPKTSSECYNOACTIVESA
 *
 * This struct can be used to set the values for OUTPKTSSECYNOACTIVESA and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txSecyPktNoactivesaCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSSECYNOACTIVESA_TXSECYPKTNOACTIVESACNT
} Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t;


/* @brief Read a table entry and convert it to a OUTPKTSSECYNOACTIVESA struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktssecynoactivesa struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSSECYNOACTIVESA memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktssecynoactivesa( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t * data );



/* @brief Struct mapping of OUTPKTSCTRLPORTDISABLED
 *
 * This struct can be used to set the values for OUTPKTSCTRLPORTDISABLED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txSecyPktCtrlPortDisabledCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSCTRLPORTDISABLED_TXSECYPKTCTRLPORTDISABLEDCNT
} Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t;


/* @brief Read a table entry and convert it to a OUTPKTSCTRLPORTDISABLED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSCTRLPORTDISABLED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktsctrlportdisabled( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t * data );



/* @brief Struct mapping of OUTOCTETSSCPROTECTED
 *
 * This struct can be used to set the values for OUTOCTETSSCPROTECTED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txSecyOctetProtectedCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTOCTETSSCPROTECTED_TXSECYOCTETPROTECTEDCNT
} Ra01IfRsMcsCseTxMemOutoctetsscprotected_t;


/* @brief Read a table entry and convert it to a OUTOCTETSSCPROTECTED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutoctetsscprotected_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutoctetsscprotected struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTOCTETSSCPROTECTED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutoctetsscprotected_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutoctetsscprotected( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutoctetsscprotected_t * data );



/* @brief Struct mapping of OUTOCTETSSCENCRYPTED
 *
 * This struct can be used to set the values for OUTOCTETSSCENCRYPTED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txSecyOctetEncryptedCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTOCTETSSCENCRYPTED_TXSECYOCTETENCRYPTEDCNT
} Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t;


/* @brief Read a table entry and convert it to a OUTOCTETSSCENCRYPTED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutoctetsscencrypted struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTOCTETSSCENCRYPTED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutoctetsscencrypted( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t * data );



/* @brief Struct mapping of OUTPKTSSAPROTECTED
 *
 * This struct can be used to set the values for OUTPKTSSAPROTECTED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txScPktProtectedCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSSAPROTECTED_TXSCPKTPROTECTEDCNT
} Ra01IfRsMcsCseTxMemOutpktssaprotected_t;


/* @brief Read a table entry and convert it to a OUTPKTSSAPROTECTED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktssaprotected_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktssaprotected struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSSAPROTECTED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktssaprotected_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktssaprotected( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssaprotected_t * data );



/* @brief Struct mapping of OUTPKTSSAENCRYPTED
 *
 * This struct can be used to set the values for OUTPKTSSAENCRYPTED and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txScPktEncryptedCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSSAENCRYPTED_TXSCPKTENCRYPTEDCNT
} Ra01IfRsMcsCseTxMemOutpktssaencrypted_t;


/* @brief Read a table entry and convert it to a OUTPKTSSAENCRYPTED struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktssaencrypted_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktssaencrypted struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSSAENCRYPTED memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktssaencrypted_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktssaencrypted( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssaencrypted_t * data );



/* @brief Struct mapping of OUTPKTSFLOWIDTCAMMISS
 *
 * This struct can be used to set the values for OUTPKTSFLOWIDTCAMMISS and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txPortPktFlowidTcamMissCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSFLOWIDTCAMMISS_TXPORTPKTFLOWIDTCAMMISSCNT
} Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t;


/* @brief Read a table entry and convert it to a OUTPKTSFLOWIDTCAMMISS struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSFLOWIDTCAMMISS memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktsflowidtcammiss( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t * data );



/* @brief Struct mapping of OUTPKTSPARSEERR
 *
 * This struct can be used to set the values for OUTPKTSPARSEERR and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txPortPktParseErrCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSPARSEERR_TXPORTPKTPARSEERRCNT
} Ra01IfRsMcsCseTxMemOutpktsparseerr_t;


/* @brief Read a table entry and convert it to a OUTPKTSPARSEERR struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktsparseerr_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktsparseerr struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSPARSEERR memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktsparseerr_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktsparseerr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsparseerr_t * data );



/* @brief Struct mapping of OUTPKTSSECTAGINSERTIONERR
 *
 * This struct can be used to set the values for OUTPKTSSECTAGINSERTIONERR and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txPortPktSectagInsertionErrCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSSECTAGINSERTIONERR_TXPORTPKTSECTAGINSERTIONERRCNT
} Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t;


/* @brief Read a table entry and convert it to a OUTPKTSSECTAGINSERTIONERR struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSSECTAGINSERTIONERR memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktssectaginsertionerr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t * data );



/* @brief Struct mapping of OUTPKTSEARLYPREEMPTERR
 *
 * This struct can be used to set the values for OUTPKTSEARLYPREEMPTERR and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txPortPktEarlyPreemptErrCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSEARLYPREEMPTERR_TXPORTPKTEARLYPREEMPTERRCNT
} Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t;


/* @brief Read a table entry and convert it to a OUTPKTSEARLYPREEMPTERR struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktsearlypreempterr struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSEARLYPREEMPTERR memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktsearlypreempterr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t * data );



/* @brief Struct mapping of OUTPKTSFLOWIDTCAMHIT
 *
 * This struct can be used to set the values for OUTPKTSFLOWIDTCAMHIT and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cse_tx_mem_slave
 */
typedef struct {
    uint64_t        txFlowidPktFlowidTcamHitCnt;  // @sa RA01_MF_RS_MCS_CSE_TX_MEM_OUTPKTSFLOWIDTCAMHIT_TXFLOWIDPKTFLOWIDTCAMHITCNT
} Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t;


/* @brief Read a table entry and convert it to a OUTPKTSFLOWIDTCAMHIT struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CSE_TX_MEM_SLAVE's OUTPKTSFLOWIDTCAMHIT memory table
 * and reformat it into the passed in Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cse_tx_mem_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCseTxMemOutpktsflowidtcamhit( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t * data );



/* @brief Struct mapping of secy_map_mem
 *
 * This struct can be used to set the values for secy_map_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 */
typedef struct {
    bool            ctrlPkt;  // @sa RA01_MF_RS_MCS_CPM_RX_SECY_MAP_MEM_CTRLPKT
    uint8_t         secy;     // @sa RA01_MF_RS_MCS_CPM_RX_SECY_MAP_MEM_SECY
} Ra01IfRsMcsCpmRxSecyMap_t;

/* @brief Format secy_map_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSecyMap_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSecyMap struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmRxSecyMap_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmRxSecyMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSecyMap_t * data );

/* @brief Read a table entry and convert it to a secy_map_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSecyMap_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSecyMap struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_RX_SLAVE's SECY_MAP_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmRxSecyMap_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmRxSecyMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSecyMap_t * data );



/* @brief Struct mapping of secy_plcy_mem
 *
 * This struct can be used to set the values for secy_plcy_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 */
typedef struct {
    bool            controlledPortEnabled;  // @sa RA01_MF_RS_MCS_CPM_RX_SECY_PLCY_MEM_CONTROLLEDPORTENABLED
    bool            preSectagAuthEnable;    // @sa RA01_MF_RS_MCS_CPM_RX_SECY_PLCY_MEM_PRESECTAGAUTHENABLE
    bool            replayProtect;          // @sa RA01_MF_RS_MCS_CPM_RX_SECY_PLCY_MEM_REPLAYPROTECT
    uint8_t         cipher;                 // @sa RA01_MF_RS_MCS_CPM_RX_SECY_PLCY_MEM_CIPHER
    uint8_t         confidentialityOffset;  // @sa RA01_MF_RS_MCS_CPM_RX_SECY_PLCY_MEM_CONFIDENTIALITYOFFSET
    uint8_t         stripSectagIcv;         // @sa RA01_MF_RS_MCS_CPM_RX_SECY_PLCY_MEM_STRIPSECTAGICV
    uint8_t         validateFrames;         // @sa RA01_MF_RS_MCS_CPM_RX_SECY_PLCY_MEM_VALIDATEFRAMES
    uint32_t        replayWindow;           // @sa RA01_MF_RS_MCS_CPM_RX_SECY_PLCY_MEM_REPLAYWINDOW
    uint32_t        reserved;               // @sa RA01_MF_RS_MCS_CPM_RX_SECY_PLCY_MEM_RESERVED
} Ra01IfRsMcsCpmRxSecyPlcy_t;

/* @brief Format secy_plcy_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSecyPlcy_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSecyPlcy struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmRxSecyPlcy_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmRxSecyPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSecyPlcy_t * data );

/* @brief Read a table entry and convert it to a secy_plcy_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSecyPlcy_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSecyPlcy struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_RX_SLAVE's SECY_PLCY_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmRxSecyPlcy_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmRxSecyPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSecyPlcy_t * data );



/* @brief Struct mapping of sa_map_mem
 *
 * This struct can be used to set the values for sa_map_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 */
typedef struct {
    bool            saInUse;  // @sa RA01_MF_RS_MCS_CPM_RX_SA_MAP_MEM_SAINUSE
    uint8_t         saIndex;  // @sa RA01_MF_RS_MCS_CPM_RX_SA_MAP_MEM_SAINDEX
} Ra01IfRsMcsCpmRxSaMap_t;

/* @brief Format sa_map_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSaMap_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSaMap struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmRxSaMap_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmRxSaMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaMap_t * data );

/* @brief Read a table entry and convert it to a sa_map_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSaMap_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSaMap struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_RX_SLAVE's SA_MAP_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmRxSaMap_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmRxSaMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaMap_t * data );



/* @brief Struct mapping of sa_plcy_mem
 *
 * This struct can be used to set the values for sa_plcy_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 */
typedef struct {
    uint32_t        ssci;         // @sa RA01_MF_RS_MCS_CPM_RX_SA_PLCY_MEM_SSCI
    uint8_t         hashkey[16];  // @sa RA01_MF_RS_MCS_CPM_RX_SA_PLCY_MEM_HASHKEY
    uint8_t         sak[32];      // @sa RA01_MF_RS_MCS_CPM_RX_SA_PLCY_MEM_SAK
    uint8_t         salt[12];     // @sa RA01_MF_RS_MCS_CPM_RX_SA_PLCY_MEM_SALT
} Ra01IfRsMcsCpmRxSaPlcy_t;

/* @brief Format sa_plcy_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSaPlcy_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSaPlcy struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmRxSaPlcy_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmRxSaPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaPlcy_t * data );

/* @brief Read a table entry and convert it to a sa_plcy_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSaPlcy_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSaPlcy struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_RX_SLAVE's SA_PLCY_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmRxSaPlcy_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmRxSaPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaPlcy_t * data );



/* @brief Struct mapping of sa_pn_table_mem
 *
 * This struct can be used to set the values for sa_pn_table_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 */
typedef struct {
    uint64_t        nextPn;  // @sa RA01_MF_RS_MCS_CPM_RX_SA_PN_TABLE_MEM_NEXTPN
} Ra01IfRsMcsCpmRxSaPnTable_t;

/* @brief Format sa_pn_table_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSaPnTable_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSaPnTable struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmRxSaPnTable_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmRxSaPnTable( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaPnTable_t * data );

/* @brief Read a table entry and convert it to a sa_pn_table_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxSaPnTable_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxSaPnTable struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_RX_SLAVE's SA_PN_TABLE_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmRxSaPnTable_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmRxSaPnTable( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaPnTable_t * data );



/* @brief Struct mapping of flowid_tcam_data
 *
 * This struct can be used to set the values for flowid_tcam_data and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 */
typedef struct {
    bool            express;              // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_EXPRESS
    bool            port;                 // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_PORT
    uint8_t         innerVlanType;        // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_INNERVLANTYPE
    uint8_t         numTags;              // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_NUMTAGS
    uint8_t         outerPriority;        // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_OUTERPRIORITY
    uint8_t         outerVlanType;        // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_OUTERVLANTYPE
    uint8_t         packetType;           // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_PACKETTYPE
    uint8_t         secondOuterPriority;  // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_SECONDOUTERPRIORITY
    uint8_t         tagMatchBitmap;       // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_TAGMATCHBITMAP
    uint16_t        bonusData;            // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_BONUSDATA
    uint16_t        etherType;            // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_ETHERTYPE
    uint32_t        outerTagId;           // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_OUTERTAGID
    uint32_t        secondOuterTagId;     // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_SECONDOUTERTAGID
    uint64_t        macDa;                // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_MACDA
    uint64_t        macSa;                // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_MACSA
} Ra01IfRsMcsCpmRxFlowidTcamData_t;

/* @brief Format flowid_tcam_data table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxFlowidTcamData_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxFlowidTcamData struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmRxFlowidTcamData_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmRxFlowidTcamData( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxFlowidTcamData_t * data );

/* @brief Read a table entry and convert it to a flowid_tcam_data struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxFlowidTcamData_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxFlowidTcamData struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_RX_SLAVE's FLOWID_TCAM_DATA memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmRxFlowidTcamData_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmRxFlowidTcamData( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxFlowidTcamData_t * data );



/* @brief Struct mapping of flowid_tcam_mask
 *
 * This struct can be used to set the values for flowid_tcam_mask and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 */
typedef struct {
    bool            maskExpress;              // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKEXPRESS
    bool            maskPort;                 // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKPORT
    uint8_t         maskInnerVlanType;        // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKINNERVLANTYPE
    uint8_t         maskNumTags;              // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKNUMTAGS
    uint8_t         maskOuterPriority;        // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKOUTERPRIORITY
    uint8_t         maskOuterVlanType;        // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKOUTERVLANTYPE
    uint8_t         maskPacketType;           // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKPACKETTYPE
    uint8_t         maskSecondOuterPriority;  // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKSECONDOUTERPRIORITY
    uint8_t         maskTagMatchBitmap;       // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKTAGMATCHBITMAP
    uint16_t        maskBonusData;            // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKBONUSDATA
    uint16_t        maskEtherType;            // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKETHERTYPE
    uint32_t        maskOuterTagId;           // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKOUTERTAGID
    uint32_t        maskSecondOuterTagId;     // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKSECONDOUTERTAGID
    uint64_t        maskMacDa;                // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKMACDA
    uint64_t        maskMacSa;                // @sa RA01_MF_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_MASKMACSA
} Ra01IfRsMcsCpmRxFlowidTcamMask_t;

/* @brief Format flowid_tcam_mask table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxFlowidTcamMask_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxFlowidTcamMask struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmRxFlowidTcamMask_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmRxFlowidTcamMask( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxFlowidTcamMask_t * data );

/* @brief Read a table entry and convert it to a flowid_tcam_mask struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxFlowidTcamMask_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxFlowidTcamMask struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_RX_SLAVE's FLOWID_TCAM_MASK memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmRxFlowidTcamMask_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmRxFlowidTcamMask( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxFlowidTcamMask_t * data );



/* @brief Struct mapping of sc_cam
 *
 * This struct can be used to set the values for sc_cam and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 */
typedef struct {
    uint8_t         secy;  // @sa RA01_MF_RS_MCS_CPM_RX_SC_CAM_SECY
    uint64_t        sci;   // @sa RA01_MF_RS_MCS_CPM_RX_SC_CAM_SCI
} Ra01IfRsMcsCpmRxScCam_t;

/* @brief Format sc_cam table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxScCam_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxScCam struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmRxScCam_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmRxScCam( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScCam_t * data );

/* @brief Read a table entry and convert it to a sc_cam struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxScCam_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxScCam struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_RX_SLAVE's SC_CAM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmRxScCam_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmRxScCam( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScCam_t * data );



/* @brief Struct mapping of sc_timer_mem
 *
 * This struct can be used to set the values for sc_timer_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_rx_slave
 */
typedef struct {
    bool            enable;                     // @sa RA01_MF_RS_MCS_CPM_RX_SC_TIMER_MEM_ENABLE
    uint8_t         actualTimeoutThreshSelect;  // @sa RA01_MF_RS_MCS_CPM_RX_SC_TIMER_MEM_ACTUALTIMEOUTTHRESHSELECT
    uint8_t         preTimeoutThreshSelect;     // @sa RA01_MF_RS_MCS_CPM_RX_SC_TIMER_MEM_PRETIMEOUTTHRESHSELECT
    uint32_t        timer;                      // @sa RA01_MF_RS_MCS_CPM_RX_SC_TIMER_MEM_TIMER
} Ra01IfRsMcsCpmRxScTimer_t;

/* @brief Format sc_timer_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxScTimer_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxScTimer struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmRxScTimer_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmRxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScTimer_t * data );

/* @brief Read a table entry and convert it to a sc_timer_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmRxScTimer_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmRxScTimer struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_RX_SLAVE's SC_TIMER_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmRxScTimer_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_rx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmRxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScTimer_t * data );



/* @brief Struct mapping of secy_map_mem
 *
 * This struct can be used to set the values for secy_map_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 */
typedef struct {
    bool            auxiliaryPlcy;  // @sa RA01_MF_RS_MCS_CPM_TX_SECY_MAP_MEM_AUXILIARYPLCY
    bool            ctrlPkt;        // @sa RA01_MF_RS_MCS_CPM_TX_SECY_MAP_MEM_CTRLPKT
    uint8_t         sc;             // @sa RA01_MF_RS_MCS_CPM_TX_SECY_MAP_MEM_SC
    uint8_t         secy;           // @sa RA01_MF_RS_MCS_CPM_TX_SECY_MAP_MEM_SECY
} Ra01IfRsMcsCpmTxSecyMap_t;

/* @brief Format secy_map_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSecyMap_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSecyMap struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmTxSecyMap_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmTxSecyMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSecyMap_t * data );

/* @brief Read a table entry and convert it to a secy_map_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSecyMap_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSecyMap struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_TX_SLAVE's SECY_MAP_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmTxSecyMap_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmTxSecyMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSecyMap_t * data );



/* @brief Struct mapping of secy_plcy_mem
 *
 * This struct can be used to set the values for secy_plcy_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 */
typedef struct {
    bool            controlledPortEnabled;  // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_CONTROLLEDPORTENABLED
    bool            preSectagAuthEnable;    // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_PRESECTAGAUTHENABLE
    bool            protectFrames;          // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_PROTECTFRAMES
    bool            sectagInsertMode;       // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_SECTAGINSERTMODE
    uint8_t         cipher;                 // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_CIPHER
    uint8_t         confidentialityOffset;  // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_CONFIDENTIALITYOFFSET
    uint8_t         sectagOffset;           // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_SECTAGOFFSET
    uint8_t         sectagTci;              // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_SECTAGTCI
    uint16_t        mtu;                    // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_MTU
    uint16_t        reserved;               // @sa RA01_MF_RS_MCS_CPM_TX_SECY_PLCY_MEM_RESERVED
} Ra01IfRsMcsCpmTxSecyPlcy_t;

/* @brief Format secy_plcy_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSecyPlcy_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSecyPlcy struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmTxSecyPlcy_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmTxSecyPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSecyPlcy_t * data );

/* @brief Read a table entry and convert it to a secy_plcy_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSecyPlcy_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSecyPlcy struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_TX_SLAVE's SECY_PLCY_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmTxSecyPlcy_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmTxSecyPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSecyPlcy_t * data );



/* @brief Struct mapping of sa_map_mem
 *
 * This struct can be used to set the values for sa_map_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 */
typedef struct {
    bool            autoRekeyEnable;  // @sa RA01_MF_RS_MCS_CPM_TX_SA_MAP_MEM_AUTOREKEYENABLE
    bool            saIndex0Vld;      // @sa RA01_MF_RS_MCS_CPM_TX_SA_MAP_MEM_SAINDEX0VLD
    bool            saIndex1Vld;      // @sa RA01_MF_RS_MCS_CPM_TX_SA_MAP_MEM_SAINDEX1VLD
    bool            txSaActive;       // @sa RA01_MF_RS_MCS_CPM_TX_SA_MAP_MEM_TXSAACTIVE
    uint8_t         saIndex0;         // @sa RA01_MF_RS_MCS_CPM_TX_SA_MAP_MEM_SAINDEX0
    uint8_t         saIndex1;         // @sa RA01_MF_RS_MCS_CPM_TX_SA_MAP_MEM_SAINDEX1
    uint64_t        sectagSci;        // @sa RA01_MF_RS_MCS_CPM_TX_SA_MAP_MEM_SECTAGSCI
} Ra01IfRsMcsCpmTxSaMap_t;

/* @brief Format sa_map_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSaMap_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSaMap struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmTxSaMap_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmTxSaMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaMap_t * data );

/* @brief Read a table entry and convert it to a sa_map_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSaMap_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSaMap struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_TX_SLAVE's SA_MAP_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmTxSaMap_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmTxSaMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaMap_t * data );



/* @brief Struct mapping of sa_plcy_mem
 *
 * This struct can be used to set the values for sa_plcy_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 */
typedef struct {
    uint8_t         sectagAn;     // @sa RA01_MF_RS_MCS_CPM_TX_SA_PLCY_MEM_SECTAGAN
    uint32_t        ssci;         // @sa RA01_MF_RS_MCS_CPM_TX_SA_PLCY_MEM_SSCI
    uint8_t         hashkey[16];  // @sa RA01_MF_RS_MCS_CPM_TX_SA_PLCY_MEM_HASHKEY
    uint8_t         sak[32];      // @sa RA01_MF_RS_MCS_CPM_TX_SA_PLCY_MEM_SAK
    uint8_t         salt[12];     // @sa RA01_MF_RS_MCS_CPM_TX_SA_PLCY_MEM_SALT
} Ra01IfRsMcsCpmTxSaPlcy_t;

/* @brief Format sa_plcy_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSaPlcy_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSaPlcy struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmTxSaPlcy_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmTxSaPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaPlcy_t * data );

/* @brief Read a table entry and convert it to a sa_plcy_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSaPlcy_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSaPlcy struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_TX_SLAVE's SA_PLCY_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmTxSaPlcy_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmTxSaPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaPlcy_t * data );



/* @brief Struct mapping of sa_pn_table_mem
 *
 * This struct can be used to set the values for sa_pn_table_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 */
typedef struct {
    uint64_t        nextPn;  // @sa RA01_MF_RS_MCS_CPM_TX_SA_PN_TABLE_MEM_NEXTPN
} Ra01IfRsMcsCpmTxSaPnTable_t;

/* @brief Format sa_pn_table_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSaPnTable_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSaPnTable struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmTxSaPnTable_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmTxSaPnTable( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaPnTable_t * data );

/* @brief Read a table entry and convert it to a sa_pn_table_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxSaPnTable_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxSaPnTable struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_TX_SLAVE's SA_PN_TABLE_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmTxSaPnTable_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmTxSaPnTable( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaPnTable_t * data );



/* @brief Struct mapping of flowid_tcam_data
 *
 * This struct can be used to set the values for flowid_tcam_data and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 */
typedef struct {
    bool            express;              // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_EXPRESS
    bool            port;                 // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_PORT
    uint8_t         innerVlanType;        // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_INNERVLANTYPE
    uint8_t         numTags;              // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_NUMTAGS
    uint8_t         outerPriority;        // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_OUTERPRIORITY
    uint8_t         outerVlanType;        // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_OUTERVLANTYPE
    uint8_t         packetType;           // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_PACKETTYPE
    uint8_t         secondOuterPriority;  // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_SECONDOUTERPRIORITY
    uint8_t         tagMatchBitmap;       // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_TAGMATCHBITMAP
    uint16_t        bonusData;            // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_BONUSDATA
    uint16_t        etherType;            // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_ETHERTYPE
    uint32_t        outerTagId;           // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_OUTERTAGID
    uint32_t        secondOuterTagId;     // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_SECONDOUTERTAGID
    uint64_t        macDa;                // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_MACDA
    uint64_t        macSa;                // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_MACSA
} Ra01IfRsMcsCpmTxFlowidTcamData_t;

/* @brief Format flowid_tcam_data table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxFlowidTcamData_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxFlowidTcamData struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmTxFlowidTcamData_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmTxFlowidTcamData( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxFlowidTcamData_t * data );

/* @brief Read a table entry and convert it to a flowid_tcam_data struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxFlowidTcamData_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxFlowidTcamData struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_TX_SLAVE's FLOWID_TCAM_DATA memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmTxFlowidTcamData_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmTxFlowidTcamData( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxFlowidTcamData_t * data );



/* @brief Struct mapping of flowid_tcam_mask
 *
 * This struct can be used to set the values for flowid_tcam_mask and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 */
typedef struct {
    bool            maskExpress;              // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKEXPRESS
    bool            maskPort;                 // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKPORT
    uint8_t         maskInnerVlanType;        // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKINNERVLANTYPE
    uint8_t         maskNumTags;              // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKNUMTAGS
    uint8_t         maskOuterPriority;        // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKOUTERPRIORITY
    uint8_t         maskOuterVlanType;        // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKOUTERVLANTYPE
    uint8_t         maskPacketType;           // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKPACKETTYPE
    uint8_t         maskSecondOuterPriority;  // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKSECONDOUTERPRIORITY
    uint8_t         maskTagMatchBitmap;       // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKTAGMATCHBITMAP
    uint16_t        maskBonusData;            // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKBONUSDATA
    uint16_t        maskEtherType;            // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKETHERTYPE
    uint32_t        maskOuterTagId;           // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKOUTERTAGID
    uint32_t        maskSecondOuterTagId;     // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKSECONDOUTERTAGID
    uint64_t        maskMacDa;                // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKMACDA
    uint64_t        maskMacSa;                // @sa RA01_MF_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_MASKMACSA
} Ra01IfRsMcsCpmTxFlowidTcamMask_t;

/* @brief Format flowid_tcam_mask table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxFlowidTcamMask_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxFlowidTcamMask struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmTxFlowidTcamMask_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmTxFlowidTcamMask( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxFlowidTcamMask_t * data );

/* @brief Read a table entry and convert it to a flowid_tcam_mask struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxFlowidTcamMask_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxFlowidTcamMask struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_TX_SLAVE's FLOWID_TCAM_MASK memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmTxFlowidTcamMask_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmTxFlowidTcamMask( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxFlowidTcamMask_t * data );



/* @brief Struct mapping of sc_timer_mem
 *
 * This struct can be used to set the values for sc_timer_mem and then passed to
 * the cooked to raw convertion function to format them for writing to the device.
 *
 * NOTE: the struct is not packed. Packing a bit field is not a portable way to convert data.
 *
 *
 * @ingroup mcs_api_rs_mcs_cpm_tx_slave
 */
typedef struct {
    bool            enable;                     // @sa RA01_MF_RS_MCS_CPM_TX_SC_TIMER_MEM_ENABLE
    bool            startGo;                    // @sa RA01_MF_RS_MCS_CPM_TX_SC_TIMER_MEM_STARTGO
    bool            startMode;                  // @sa RA01_MF_RS_MCS_CPM_TX_SC_TIMER_MEM_STARTMODE
    uint8_t         actualTimeoutThreshSelect;  // @sa RA01_MF_RS_MCS_CPM_TX_SC_TIMER_MEM_ACTUALTIMEOUTTHRESHSELECT
    uint8_t         preTimeoutThreshSelect;     // @sa RA01_MF_RS_MCS_CPM_TX_SC_TIMER_MEM_PRETIMEOUTTHRESHSELECT
    uint32_t        timer;                      // @sa RA01_MF_RS_MCS_CPM_TX_SC_TIMER_MEM_TIMER
} Ra01IfRsMcsCpmTxScTimer_t;

/* @brief Format sc_timer_mem table entry to and write to device
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxScTimer_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxScTimer struct to format and write
 *
 * This function will convert the passed in Ra01IfRsMcsCpmTxScTimer_t
 * struct to a series of register writes and use the hardware write function in rmsDev_p to write it to 
 * the device.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01WriteRsMcsCpmTxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxScTimer_t * data );

/* @brief Read a table entry and convert it to a sc_timer_mem struct
 *
 * @param[in]   rmsDev_p    The hardware handle for the target device Ra01IfRsMcsCpmTxScTimer_t
 * @param[in]   index       Index in the table to write to
 * @param[in]   data        Pointer to a Ra01IfRsMcsCpmTxScTimer struct to be written to.
 *
 * This function will read an entry from the RS_MCS_CPM_TX_SLAVE's SC_TIMER_MEM memory table
 * and reformat it into the passed in Ra01IfRsMcsCpmTxScTimer_t pointer.
 *
 * @retval 0       success
 * @retval -EFAULT could not map the register(s) for rs_mcs_cpm_tx_slave
 * @retval -EINVAL bad input value or a null input pointer
 */
DLL_PUBLIC int Ra01ReadRsMcsCpmTxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxScTimer_t * data );


/********************************************************************************
 *     P R O T O T Y P E S
 ********************************************************************************/

/* @brief Read a value from a register
 *
 * @param[in]  rmsDev_p     The hardware handle for the target device
 * @param[in]  offset       Offset of register in the map
 * @param[in]  size         Size of register in units of register widths
 * @param[in]  value        Pointer to a location to put the value read
 *
 * @retval     0       Success
 * @retval    -EFAULT  Failure: ra01_reg_id is out of range
 *
 * Look up the register address and read value.
 */
int ra01_read_reg_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned size, ra01_register_t volatile * value );

/* @brief Read a value from a 32-bit register
 *
 * @param[in]  rmsDev_p     The hardware handle for the target device
 * @param[in]  offset       Offset of register in the map
 * @param[in]  value        Pointer to a location to put the value read
 *
 * @retval     0       Success
 * @retval    -EFAULT  Failure: ra01_reg_id is out of range
 *
 * Look up the register address and read value. This is only for use on a 32-bit register 
 * on a 64-bit device. It is recommended that the register specific functions be used.
 */
int ra01_read_reg32_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, uint32_t volatile * value );

/* @brief write a value to a register
 *
 * @param[in]  rmsDev_p     The hardware handle for the target device
 * @param[in]  offset       Offset of register in the map
 * @param[in]  size         Size of register in units of register widths
 * @param[in]  value        Value to write
 *
 * @retval     0       Success
 * @retval    -EACCES  Failure: Register is not writeable
 * @retval    -EFAULT  Failure: ra01_reg_id is out of range
 *
 * Look up the register address and write value.
 */
int ra01_write_reg_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned size, ra01_register_t value );

/* @brief write a value to a 32-bit register
 *
 * @param[in]  rmsDev_p     The hardware handle for the target device
 * @param[in]  offset       Offset of register in the map
 * @param[in]  value        Value to write
 *
 * @retval     0       Success
 * @retval    -EACCES  Failure: Register is not writeable
 * @retval    -EFAULT  Failure: ra01_reg_id is out of range
 *
 * Look up the register address and write value. This is only for use on a 32-bit register 
 * on a 64-bit device. It is recommended that the register specific functions be used.
 */
int ra01_write_reg32_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, uint32_t value );


/* @brief Read a memory field
 * 
 * @param[inout] rmsDev_p  Pointer for hardware read use.
 * @param[in]    offset    register offset in the map
 * @param[in]    step      size of the register in units of APB width
 * @param[in]    size      size of the register in units of register widths
 * @param[in]    index     The index of the specifc register in a bank of registers
 * @param[in]    buffer    Pointer to memory to hold the data.
 * 
 * @retval 0        Success
 * @retval -EFAULT  id is out of range.
 * @retval -int     Return valud from HW read function.
 * 
 * Read the memory register specified by id into buffer. buffer must be large enough to hold the 
 * data, no checks are performed. The function uses the supplied HW access function to perform the
 * actual read.
 *
 */
int ra01_read_mem_field_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned step, unsigned size, unsigned index, ra01_register_t volatile * buffer );

/* @brief Update a memory block
 *
 * @param[in]  buffer      A pointer to a register array representing the memory register
 * @param[in]  offset      Array of offsets of the fields within the memory
 * @param[in]  msb         Array of msbs of the fields within the register
 * @param[in]  lsb         Array of lsbs of the fields within the register
 * @param[in]  value       The values of the fields to be written
 * @param[in]  num_fields  Number of fields to update 
 *
 * This function does not perform any physical access to the device, its purpose is to modify the contents 
 * of the host's copy of the devices memory (i.e. the modify in a read-modify-write sequence). The size of
 * buffer must be the same as the base register that mf is a part of. Note that value can span register size
 * boundaries. 
 *
 */
void ra01_update_mem_field_by_id( ra01_register_t * buffer, uintAddrBus_t * offset, unsigned * msb, unsigned * lsb, ra01_register_t ** value, unsigned num_fields );

/* @brief Read data from a memory block
 *
 * @param[in]  buffer   A pointer to a register array representing the memory register
 * @param[in]  offset   Offset of the register within the memory
 * @param[in]  msb      msb of the field within the register
 * @param[in]  lsb      lsb of the field within the register
 * @param[out] value    A location for the value being read
 * @param[in]  type     The type of data expected to be read into value
 *
 * This register reads a field of a memory register stored in buffer. The size of the value is determined from the field
 * lookup of mf. The value buffer must be large enough to extract the correct value. All values are normalized.
 */
int ra01_extract_mem_field_by_id( ra01_register_t const * buffer, uintAddrBus_t offset, unsigned msb, unsigned lsb, void * value, ra01_mem_access_type_t type );

/* @brief write a memory register to the device
 * 
 * @param[inout] rmsDev_p  Pointer for hardware read use.
 * @param[in]    offset    register offset in the map
 * @param[in]    step      size of the register in units of APB width
 * @param[in]    size      size of the register in units of register width
 * @param[in]    index     The index to identify the specific register in a bank of registers
 * @param[in]    buffer    The data to be written.
 * 
 * @retval  0       Succcess
 * @retval -EFAULT  id is out of range
 *
 * Write buffer out to the memory register identified by id and index. This function uses the supplied hardware 
 * access fucntions to perform the physical device write.
 */
int ra01_write_mem_field_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned step, unsigned size, unsigned index, ra01_register_t const * buffer );

/********************************************************************************
 *     H E L P E R S
 ********************************************************************************/

/* @brief Get the minimum of 2 unsigned values
 *
 * @param x  An unsigned value
 * @param y  Another unsigned value
 *
 * @retval unsigned the lesser of x and y.
 * 
 * NOTE: This function is for type unsigned and not for general use.
 * 
 * A function-like macro with multiple parameters is generally understood to be a bad idea.
 */
static inline unsigned u_min (unsigned x, unsigned y ) 
{
    return (x < y) ? x : y;
}

/* @brief Build a mack from msb:lsb inclusive.
 *
 * @param[in] msb Most Significant Bit position
 * @param[in] lsb Least Significant Bit position
 *
 * @returns A mask with [mbs:lsb] (inclusive) bits set.
 *
 * For example: ra01_make_mask( 6, 3 ) returns 0x0...078 (8b'01111000)
 *
 * The math may be confusing at first (using the function call above with 8 bit registers):
 *   1a. Set [(msb-1):0] using (1 << msb) - 1               yields 0b'00111111
 *   1b. Set msb using | (1 << msb)                         yields 0b'01000000 | 0b'00111111 = 0b'01111111
 *   2.  Clear [(lsb-1):0] using ~((1 << lsb) - 1)          yields ~0b'00000111 = 0b'11111000
 *   3.  And 1. and 2. together to get the result           yields 0b'01111111 & 0b'11111000 = 0'b01111000
 */
static inline ra01_register_t ra01_make_mask ( unsigned msb, unsigned lsb )
{
    // (1 << (msb + 1)) would work but runs the risk of overflowing 
    return ((((ra01_register_t)1U << msb) - 1) | ((ra01_register_t)1U << msb)) & (~(((ra01_register_t)1U << lsb) - 1));
}

/* @brief set a bit field
 * 
 * @param[inout] reg    Pointer to the register to set the bits in.
 * @param[in]    msb    Most Significant Bit position of the bitfield (inclusive)
 * @param[in]    lsb    Least Significant Bit position of the bit field (inclusive)
 * @param[in]    value  The value to be set in the bitfield (unshifted!)
 * 
 * retval ra01_register_t The value of *reg  after the shift.
 *
 * NOTE: This function will not span ra01_register_t boundaries to set the bit field. If the values of msb or lsb lie
 * outside the range of bits in a ra01_register_t use ra01_set_bit_field_array
 *
 */
static inline ra01_register_t ra01_set_bit_field( ra01_register_t * reg, 
                                                  unsigned msb, 
                                                  unsigned lsb, 
                                                  ra01_register_t value )
{
    ra01_register_t mask = ra01_make_mask( msb, lsb );

    *reg  = (*reg  & ~mask) | ((value << lsb) & mask);

    return *reg ;
}

/* @brief Extract a bit field from a buffer
 * 
 * @param[in]  reg        The register data
 * @param[in]  msb        Array of MSBs of the fields to extract
 * @param[in]  lsb        Array of LSBs of the fields to extract
 * @param[out] value      Array of values to be returned
 * @param[in]  num_fields Number of fields to extract from the register
 * 
 * This function is used to parse a value from a register previously read from the device. It does not 
 * perform any device access. The purpose is to allow muliple fields to be extracted from a register without 
 * rereading it. Use ra01_read_bit_field_by_id to read a bitfield from the device.
 *
 */
void  ra01_get_bit_field_by_id( ra01_register_t reg, unsigned * msb, unsigned * lsb, ra01_register_t * value, unsigned num_fields);

/* @brief Extract bits
 * 
 * @param[in]  reg    The 64 uint to extract some bits from
 * @param[in]  msb    The most significant bit of the field to extract
 * @param[in]  lsb    The least significant bit of the field to extract
 *
 * @returns ra01_register_t The value extracted.
 *
 * mask of the range [msb:lsb] from reg and soft right by lsb bits. 
 */
static inline ra01_register_t ra01_get_bit_field( ra01_register_t reg, 
                                                  unsigned msb, 
                                                  unsigned lsb )
{
    ra01_register_t mask = ra01_make_mask( msb, lsb );

    return (reg & mask) >> lsb;
}

/* @brief Update a bit field in a buffer
 * 
 * @param[in]  reg         The register data
 * @param[in]  msb         Array of MSBs of the fields within the register
 * @param[in]  msb         Array of LSBs of the fields within the register
 * @param[out] value       Array of values to be written (one for each field)
 * @param[in]  num_fields  Number of fields to set
 * 
 * @retval  0       Success
 * @retval -EFAULT  id does not correspond to a bit field
 * 
 * Update the value of a specific field in a memory buffer which is a local copy of a 
 * device memory register. The purpose of this function is the "modify" in a read-modify-write
 * sequence. To write a bit field directly to the device use ra01_write_bit_field_by_id.
 *
 * NOTE: This function sets the bitfield in *reg, but does not use the .ra01_reg_id.offset.
 * NOTE: The value parameter should not be shifted, ra01UpdateBitFieldById will perform any required shifting
 *       and masking.
 *
 */
int ra01_set_bit_field_by_id( ra01_register_t * reg, unsigned * msb, unsigned * lsb, ra01_register_t * value, unsigned num_fields );


/* @brief set a bitfield in an array
 * 
 * @param[inout] array  A pointer to an array to set the bitfield in
 * @param[in]    size   The number of ra01_register_t units in array
 * @param[in]    msb    Most Significant Bit of the bitfield (inclusive)
 * @param[in]    lsb    Least Significant Bit position of the bit field (inclusive)
 * @param[in]    value  The value to be set in the bitfield (unshifted!)
 *
 * @retval array  The pointer array from the input (for chaining)
 *
 * NOTE: the size parameter is sizeof(array)/sizeof(ra01_register_t)
 */
static inline ra01_register_t * ra01_set_bit_field_array( ra01_register_t * array, 
                                                          size_t size, 
                                                          unsigned msb, 
                                                          unsigned lsb, 
                                                          ra01_register_t value )
{
    size_t reg_bits = 8 * sizeof( ra01_register_t );
    unsigned loc = lsb / reg_bits;      // index in array[] where the lsb can be found
    
    if( loc < size && msb > lsb ) {
        ra01_register_t * p = &(array[loc]);
        lsb -= reg_bits * loc;
        msb -= reg_bits * loc;
        do { 
            unsigned m = u_min( msb, reg_bits - 1 );
            ra01_set_bit_field( p, m, lsb, value );
            value >>= (m - lsb) + 1;
            p++;
            lsb = 0;
            msb -= u_min( (unsigned int)reg_bits, msb );
        } while (msb > 0 );
    }

    return array;
}

/* @brief Perform a Read-modify-write operation on a register based on the specified bitfield.
 * 
 * @param[inout] rmsDev_p  Pointer for hardware read use.
 * @param[in]    offset    Offset of register in the map
 * @param[in]    size      Size of register in units of register widths
 * @param[in]    msb       Most Significant Bit of the bitfield (inclusive)
 * @param[in]    lsb       Least Significant Bit position of the bit field (inclusive)
 * @param[in]    value     The value to write to the bit field identified by bf
 * 
 * @retval  0      Success
 * @retval -EFAULT Bad ID value
 * @retval -EACCES Read only register
 * 
 * Update a specific bit field on the device.
 * 
 */
 int ra01_write_bit_field_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned size, unsigned msb, unsigned lsb, ra01_register_t value );

/* @brief Read a register and extract a bit field
 *
 * @param[inout] rmsDev_p  Pointer for hardware read use.
 * @param[in]    offset    Offset of register in the map
 * @param[in]    size      Size of register in units of register widths
 * @param[in]    msb       Most Significant Bit of the bitfield (inclusive)
 * @param[in]    lsb       Least Significant Bit position of the bit field (inclusive)
 * @param[in]    value     Pointer to the location to write the data
 * 
 * @retval  0      Success
 * @retval -EFAULT Bad ID value
 * 
 * Read a register and extract a bit field from it.
 * 
 */
int ra01_read_bit_field_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned size, unsigned msb, unsigned lsb, ra01_register_t * value );

#ifdef __cplusplus
}
#endif
#endif // MCS_INTERNALS_NOCT_H

