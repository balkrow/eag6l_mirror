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
* @file smemAasAddrSpace_CC_plus.h
*
* @brief Private definition for Aas address space.
*
*
* @version   1
********************************************************************************
*/

#ifndef __smemAasAddrSpace_CC_plus
#define __smemAasAddrSpace_CC_plus

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
https://marvell.sharepoint.com/:x:/r/sites/switching/chipdesign/seahawk/Shared Documents/Design/AXI/CC+/Seahawk_Address_Space_5.4.xlsm?d=w1f84fb9db2e24c818837352b89861837&csf=1&web=1&e=m2Olk9
*/
/*        Unit Name             Start Address        Allocated Size        Allocated Units        Needed Addr Bits        Block */
#define AAS_ADDR_DP0_PDS                            0x00000000        /*2        MB        21        pds_main_block                         */
#define AAS_ADDR_GOP0_4P_ANP1_0                    0x00200000        /*256        KB        18        epi_4p_block                          */
#define AAS_ADDR_GOP0_4P_ANP1_1                    0x00240000        /*256        KB        18        epi_4p_block                          */
#define AAS_ADDR_GOP0_4P_ANP1_2                    0x00280000        /*256        KB        18        epi_4p_block                          */
#define AAS_ADDR_GOP0_4P_ANP1_3                    0x002C0000        /*256        KB        18        epi_4p_block                          */
#define AAS_ADDR_GOP0_4P_PCS100_0                0x00300000        /*128        KB        17        epi_4p_block                            */
#define AAS_ADDR_GOP0_4P_PCS100_1                0x00320000        /*128        KB        17        epi_4p_block                            */
#define AAS_ADDR_GOP0_4P_PCS100_2                0x00340000        /*128        KB        17        epi_4p_block                            */
#define AAS_ADDR_GOP0_4P_PCS100_3                0x00360000        /*128        KB        17        epi_4p_block                            */
#define AAS_ADDR_GOP0_4P_MAC100_0                0x00380000        /*64        KB        16        epi_4p_block                             */
#define AAS_ADDR_GOP0_4P_MAC100_1                0x00390000        /*64        KB        16        epi_4p_block                             */
#define AAS_ADDR_GOP0_4P_MAC100_2                0x003A0000        /*64        KB        16        epi_4p_block                             */
#define AAS_ADDR_GOP0_4P_MAC100_3                0x003B0000        /*64        KB        16        epi_4p_block                             */
#define AAS_ADDR_GOP0_4P_MAC_TAI                    0x003C0000        /*64        KB        16        epi_4p_block                          */
#define AAS_ADDR_GOP0_4P_MIF                        0x003D0000        /*64        KB        16        epi_4p_block                          */
#define AAS_ADDR_GOP0_4P_LED                        0x003E0000        /*64        KB        16        epi_4p_block                          */
#define AAS_ADDR_DP0_SDQ                            0x00400000        /*2080        KB        22        txqs_block                          */
#define AAS_ADDR_DP0_TXQS_TAI                    0x00608000        /*32        KB        15        txqs_block                               */
#define AAS_ADDR_DP0_QFC                            0x00610000        /*64        KB        16        txqs_block                            */
#define AAS_ADDR_DP0_TXF                            0x00620000        /*64        KB        16        tx_block                              */
#define AAS_ADDR_DP0_TXD                            0x00630000        /*64        KB        16        tx_block                              */
#define AAS_ADDR_DP0_RX                            0x00640000        /*64        KB        16        rx_block                               */
#define AAS_ADDR_GOP0_16P_PCS800                    0x00660000        /*128        KB        17        desk_block                           */
#define AAS_ADDR_GOP0_163E_TAI_0                    0x00680000        /*64        KB        16        pca_163e_block                        */
#define AAS_ADDR_GOP0_163E_TAI_1                    0x00690000        /*64        KB        16        pca_163e_block                        */
#define AAS_ADDR_GOP0_163E_TAI_2                    0x006A0000        /*64        KB        16        pca_163e_block                        */
#define AAS_ADDR_GOP0_CTSU                        0x006B0000        /*64        KB        16        pca_163e_block                          */
#define AAS_ADDR_GOP0_MACSEC_163E                0x006C0000        /*128        KB        17        pca_163e_block                          */
#define AAS_ADDR_GOP0_MACSEC_WRP_163E            0x006E0000        /*32        KB        15        pca_163e_block                           */
#define AAS_ADDR_GOP0_TX_UPACK                    0x006E8000        /*32        KB        15        pca_163e_block                          */
#define AAS_ADDR_GOP0_TX_EDGE                    0x006F0000        /*32        KB        15        pca_163e_block                           */
#define AAS_ADDR_GOP0_SFF                        0x006F8000        /*32        KB        15        pca_163e_block                           */
#define AAS_ADDR_GOP0_MACSEC_164I                0x00700000        /*384        KB        19        pca_164i_block                          */
#define AAS_ADDR_GOP0_MACSEC_WRP_164I            0x00760000        /*64        KB        16        pca_164i_block                           */
#define AAS_ADDR_GOP0_RX_PACK                    0x00770000        /*32        KB        15        pca_164i_block                           */
#define AAS_ADDR_GOP0_RX_EDGE                    0x00778000        /*32        KB        15        pca_164i_block                           */
#define AAS_ADDR_GOP0_MACSEC_163I                0x00780000        /*128        KB        17        pca_163i_block                          */
#define AAS_ADDR_GOP0_MACSEC_WRP_163I            0x007A0000        /*64        KB        16        pca_163i_block                           */
#define AAS_ADDR_GOP0_ING_SHM                    0x007B0000        /*64        KB        16        pca_163i_block                           */
#define AAS_ADDR_GOP0_SATG                        0x007C0000        /*64        KB        16        pca_163i_block                          */
#define AAS_ADDR_GOP0_16P_SDW_56G_X4_0            0x00800000        /*512        KB        19        sdw_200g_block                         */
#define AAS_ADDR_GOP0_16P_SDW_56G_X4_1            0x00880000        /*512        KB        19        sdw_200g_block_dummy_0                 */
#define AAS_ADDR_GOP0_4P_SDW_56G_X4                0x00900000        /*512        KB        19        sdw_200g_block_dummy_1                */
#define AAS_ADDR_GOP0_16P_SDW_112G_X4_0            0x00980000        /*512        KB        19        sdw_400g_block                        */
#define AAS_ADDR_GOP0_16P_SDW_112G_X4_1            0x00A00000        /*512        KB        19        sdw_400g_block_dummy_0                */
#define AAS_ADDR_GOP0_16P_ANP8_0                    0x00A80000        /*256        KB        18        anp_block                            */
#define AAS_ADDR_GOP0_16P_ANP8_1                    0x00AC0000        /*256        KB        18        anp_block                            */
#define AAS_ADDR_GOP0_164E_TAI                    0x00C00000        /*128        KB        17        pca_164e_block                         */
#define AAS_ADDR_GOP0_LMU_0                        0x00C20000        /*128        KB        17        pca_164e_block                        */
#define AAS_ADDR_GOP0_LMU_1                        0x00C40000        /*128        KB        17        pca_164e_block                        */
#define AAS_ADDR_GOP0_LMU_2                        0x00C60000        /*128        KB        17        pca_164e_block                        */
#define AAS_ADDR_GOP0_LMU_3                        0x00C80000        /*128        KB        17        pca_164e_block                        */
#define AAS_ADDR_GOP0_LMU_4                        0x00CA0000        /*128        KB        17        pca_164e_block                        */
#define AAS_ADDR_GOP0_LMU_5                        0x00CC0000        /*128        KB        17        pca_164e_block                        */
#define AAS_ADDR_GOP0_LMU_6                        0x00CE0000        /*128        KB        17        pca_164e_block                        */
#define AAS_ADDR_GOP0_LMU_7                        0x00D00000        /*128        KB        17        pca_164e_block                        */
#define AAS_ADDR_GOP0_PCA_PIZARB                    0x00D20000        /*128        KB        17        pca_164e_block                       */
#define AAS_ADDR_GOP0_SHM_EGR                    0x00D40000        /*128        KB        17        pca_164e_block                          */
#define AAS_ADDR_GOP0_SATC                        0x00D60000        /*128        KB        17        pca_164e_block                         */
#define AAS_ADDR_GOP0_MACSEC_WRP_164E            0x00D80000        /*256        KB        18        pca_164e_block                          */
#define AAS_ADDR_GOP0_MACSEC_164E                0x00DC0000        /*256        KB        18        pca_164e_block                          */
#define AAS_ADDR_GOP0_16P_MAC800_0                0x00E00000        /*64        KB        16        mac_9p_block                            */
#define AAS_ADDR_GOP0_16P_MAC800_0_TAI            0x00E10000        /*64        KB        16        mac_9p_block                            */
#define AAS_ADDR_GOP0_16P_LED_0                    0x00E20000        /*64        KB        16        mac_9p_block                           */
#define AAS_ADDR_GOP0_16P_MIF_0                    0x00E30000        /*64        KB        16        mac_9p_block                           */
#define AAS_ADDR_GOP0_16P_MAC800_1                0x00E40000        /*64        KB        16        mac_9p_block_dummy_0                    */
#define AAS_ADDR_GOP0_16P_MAC800_1_TAI            0x00E50000        /*64        KB        16        mac_9p_block_dummy_0                    */
#define AAS_ADDR_GOP0_16P_LED_1                    0x00E60000        /*64        KB        16        mac_9p_block_dummy_0                   */
#define AAS_ADDR_GOP0_16P_MIF_1                    0x00E70000        /*64        KB        16        mac_9p_block_dummy_0                   */
#define AAS_ADDR_DP1_PDS                            0x01000000        /*2        MB        21        pds_main_block_dummy_0                 */
#define AAS_ADDR_GOP1_4P_ANP1_0                    0x01200000        /*256        KB        18        epi_4p_block_dummy_0                  */
#define AAS_ADDR_GOP1_4P_ANP1_1                    0x01240000        /*256        KB        18        epi_4p_block_dummy_0                  */
#define AAS_ADDR_GOP1_4P_ANP1_2                    0x01280000        /*256        KB        18        epi_4p_block_dummy_0                  */
#define AAS_ADDR_GOP1_4P_ANP1_3                    0x012C0000        /*256        KB        18        epi_4p_block_dummy_0                  */
#define AAS_ADDR_GOP1_4P_PCS100_0                0x01300000        /*128        KB        17        epi_4p_block_dummy_0                    */
#define AAS_ADDR_GOP1_4P_PCS100_1                0x01320000        /*128        KB        17        epi_4p_block_dummy_0                    */
#define AAS_ADDR_GOP1_4P_PCS100_2                0x01340000        /*128        KB        17        epi_4p_block_dummy_0                    */
#define AAS_ADDR_GOP1_4P_PCS100_3                0x01360000        /*128        KB        17        epi_4p_block_dummy_0                    */
#define AAS_ADDR_GOP1_4P_MAC100_0                0x01380000        /*64        KB        16        epi_4p_block_dummy_0                     */
#define AAS_ADDR_GOP1_4P_MAC100_1                0x01390000        /*64        KB        16        epi_4p_block_dummy_0                     */
#define AAS_ADDR_GOP1_4P_MAC100_2                0x013A0000        /*64        KB        16        epi_4p_block_dummy_0                     */
#define AAS_ADDR_GOP1_4P_MAC100_3                0x013B0000        /*64        KB        16        epi_4p_block_dummy_0                     */
#define AAS_ADDR_GOP1_4P_MAC_TAI                    0x013C0000        /*64        KB        16        epi_4p_block_dummy_0                  */
#define AAS_ADDR_GOP1_4P_MIF                        0x013D0000        /*64        KB        16        epi_4p_block_dummy_0                  */
#define AAS_ADDR_GOP1_4P_LED                        0x013E0000        /*64        KB        16        epi_4p_block_dummy_0                  */
#define AAS_ADDR_DP1_SDQ                            0x01400000        /*2080        KB        22        txqs_block_dummy_0                  */
#define AAS_ADDR_DP1_TXQS_TAI                    0x01608000        /*32        KB        15        txqs_block_dummy_0                       */
#define AAS_ADDR_DP1_QFC                            0x01610000        /*64        KB        16        txqs_block_dummy_0                    */
#define AAS_ADDR_DP1_TXF                            0x01620000        /*64        KB        16        tx_block_dummy_0                      */
#define AAS_ADDR_DP1_TXD                            0x01630000        /*64        KB        16        tx_block_dummy_0                      */
#define AAS_ADDR_DP1_RX                            0x01640000        /*64        KB        16        rx_block_dummy_0                       */
#define AAS_ADDR_GOP1_16P_PCS800                    0x01660000        /*128        KB        17        desk_block_dummy_0                   */
#define AAS_ADDR_GOP1_163E_TAI_0                    0x01680000        /*64        KB        16        pca_163e_block_dummy_0                */
#define AAS_ADDR_GOP1_163E_TAI_1                    0x01690000        /*64        KB        16        pca_163e_block_dummy_0                */
#define AAS_ADDR_GOP1_163E_TAI_2                    0x016A0000        /*64        KB        16        pca_163e_block_dummy_0                */
#define AAS_ADDR_GOP1_CTSU                        0x016B0000        /*64        KB        16        pca_163e_block_dummy_0                  */
#define AAS_ADDR_GOP1_MACSEC_163E                0x016C0000        /*128        KB        17        pca_163e_block_dummy_0                  */
#define AAS_ADDR_GOP1_MACSEC_WRP_163E            0x016E0000        /*32        KB        15        pca_163e_block_dummy_0                   */
#define AAS_ADDR_GOP1_TX_UPACK                    0x016E8000        /*32        KB        15        pca_163e_block_dummy_0                  */
#define AAS_ADDR_GOP1_TX_EDGE                    0x016F0000        /*32        KB        15        pca_163e_block_dummy_0                   */
#define AAS_ADDR_GOP1_SFF                        0x016F8000        /*32        KB        15        pca_163e_block_dummy_0                   */
#define AAS_ADDR_GOP1_MACSEC_164I                0x01700000        /*384        KB        19        pca_164i_block_dummy_0                  */
#define AAS_ADDR_GOP1_MACSEC_WRP_164I            0x01760000        /*64        KB        16        pca_164i_block_dummy_0                   */
#define AAS_ADDR_GOP1_RX_PACK                    0x01770000        /*32        KB        15        pca_164i_block_dummy_0                   */
#define AAS_ADDR_GOP1_RX_EDGE                    0x01778000        /*32        KB        15        pca_164i_block_dummy_0                   */
#define AAS_ADDR_GOP1_MACSEC_163I                0x01780000        /*128        KB        17        pca_163i_block_dummy_0                  */
#define AAS_ADDR_GOP1_MACSEC_WRP_163I            0x017A0000        /*64        KB        16        pca_163i_block_dummy_0                   */
#define AAS_ADDR_GOP1_ING_SHM                    0x017B0000        /*64        KB        16        pca_163i_block_dummy_0                   */
#define AAS_ADDR_GOP1_SATG                        0x017C0000        /*64        KB        16        pca_163i_block_dummy_0                  */
#define AAS_ADDR_GOP1_16P_SDW_56G_X4_0            0x01800000        /*512        KB        19        sdw_200g_block_dummy_2                 */
#define AAS_ADDR_GOP1_16P_SDW_56G_X4_1            0x01880000        /*512        KB        19        sdw_200g_block_dummy_3                 */
#define AAS_ADDR_GOP1_4P_SDW_56G_X4                0x01900000        /*512        KB        19        sdw_200g_block_dummy_4                */
#define AAS_ADDR_GOP1_16P_SDW_112G_X4_0            0x01980000        /*512        KB        19        sdw_400g_block_dummy_1                */
#define AAS_ADDR_GOP1_16P_SDW_112G_X4_1            0x01A00000        /*512        KB        19        sdw_400g_block_dummy_2                */
#define AAS_ADDR_GOP1_16P_ANP8_0                    0x01A80000        /*256        KB        18        anp_block_dummy_0                    */
#define AAS_ADDR_GOP1_16P_ANP8_1                    0x01AC0000        /*256        KB        18        anp_block_dummy_0                    */
#define AAS_ADDR_GOP1_CPU_SDW_56G_X1                0x01B00000        /*512        KB        19        cpu_port_block                       */
#define AAS_ADDR_GOP1_CPU_ANP1                    0x01B80000        /*256        KB        18        cpu_port_block                         */
#define AAS_ADDR_GOP1_CPU_MAC100                    0x01BC0000        /*64        KB        16        cpu_port_block                        */
#define AAS_ADDR_GOP1_CPU_MAC_TAI                0x01BD0000        /*64        KB        16        cpu_port_block                           */
#define AAS_ADDR_GOP1_CPU_PCS100                    0x01BE0000        /*32        KB        15        cpu_port_block                        */
#define AAS_ADDR_GOP1_CPU_LED                    0x01BE8000        /*32        KB        15        cpu_port_block                           */
#define AAS_ADDR_GOP1_CPU_MIF                    0x01BF0000        /*64        KB        16        cpu_port_block                           */
#define AAS_ADDR_GOP1_164E_TAI                    0x01C00000        /*128        KB        17        pca_164e_block_dummy_0                 */
#define AAS_ADDR_GOP1_LMU_0                        0x01C20000        /*128        KB        17        pca_164e_block_dummy_0                */
#define AAS_ADDR_GOP1_LMU_1                        0x01C40000        /*128        KB        17        pca_164e_block_dummy_0                */
#define AAS_ADDR_GOP1_LMU_2                        0x01C60000        /*128        KB        17        pca_164e_block_dummy_0                */
#define AAS_ADDR_GOP1_LMU_3                        0x01C80000        /*128        KB        17        pca_164e_block_dummy_0                */
#define AAS_ADDR_GOP1_LMU_4                        0x01CA0000        /*128        KB        17        pca_164e_block_dummy_0                */
#define AAS_ADDR_GOP1_LMU_5                        0x01CC0000        /*128        KB        17        pca_164e_block_dummy_0                */
#define AAS_ADDR_GOP1_LMU_6                        0x01CE0000        /*128        KB        17        pca_164e_block_dummy_0                */
#define AAS_ADDR_GOP1_LMU_7                        0x01D00000        /*128        KB        17        pca_164e_block_dummy_0                */
#define AAS_ADDR_GOP1_PCA_PIZARB                    0x01D20000        /*128        KB        17        pca_164e_block_dummy_0               */
#define AAS_ADDR_GOP1_SHM_EGR                    0x01D40000        /*128        KB        17        pca_164e_block_dummy_0                  */
#define AAS_ADDR_GOP1_SATC                        0x01D60000        /*128        KB        17        pca_164e_block_dummy_0                 */
#define AAS_ADDR_GOP1_MACSEC_WRP_164E            0x01D80000        /*256        KB        18        pca_164e_block_dummy_0                  */
#define AAS_ADDR_GOP1_MACSEC_164E                0x01DC0000        /*256        KB        18        pca_164e_block_dummy_0                  */
#define AAS_ADDR_GOP1_16P_MAC800_0                0x01E00000        /*64        KB        16        mac_9p_block_dummy_1                    */
#define AAS_ADDR_GOP1_16P_MAC800_0_TAI            0x01E10000        /*64        KB        16        mac_9p_block_dummy_1                    */
#define AAS_ADDR_GOP1_16P_LED_0                    0x01E20000        /*64        KB        16        mac_9p_block_dummy_1                   */
#define AAS_ADDR_GOP1_16P_MIF_0                    0x01E30000        /*64        KB        16        mac_9p_block_dummy_1                   */
#define AAS_ADDR_GOP1_16P_MAC800_1                0x01E40000        /*64        KB        16        mac_9p_block_dummy_2                    */
#define AAS_ADDR_GOP1_16P_MAC800_1_TAI            0x01E50000        /*64        KB        16        mac_9p_block_dummy_2                    */
#define AAS_ADDR_GOP1_16P_LED_1                    0x01E60000        /*64        KB        16        mac_9p_block_dummy_2                   */
#define AAS_ADDR_GOP1_16P_MIF_1                    0x01E70000        /*64        KB        16        mac_9p_block_dummy_2                   */
#define AAS_ADDR_DP2_PDS                            0x02000000        /*2        MB        21        pds_main_block_dummy_1                 */
#define AAS_ADDR_GOP2_4P_ANP1_0                    0x02200000        /*256        KB        18        epi_4p_block_dummy_1                  */
#define AAS_ADDR_GOP2_4P_ANP1_1                    0x02240000        /*256        KB        18        epi_4p_block_dummy_1                  */
#define AAS_ADDR_GOP2_4P_ANP1_2                    0x02280000        /*256        KB        18        epi_4p_block_dummy_1                  */
#define AAS_ADDR_GOP2_4P_ANP1_3                    0x022C0000        /*256        KB        18        epi_4p_block_dummy_1                  */
#define AAS_ADDR_GOP2_4P_PCS100_0                0x02300000        /*128        KB        17        epi_4p_block_dummy_1                    */
#define AAS_ADDR_GOP2_4P_PCS100_1                0x02320000        /*128        KB        17        epi_4p_block_dummy_1                    */
#define AAS_ADDR_GOP2_4P_PCS100_2                0x02340000        /*128        KB        17        epi_4p_block_dummy_1                    */
#define AAS_ADDR_GOP2_4P_PCS100_3                0x02360000        /*128        KB        17        epi_4p_block_dummy_1                    */
#define AAS_ADDR_GOP2_4P_MAC100_0                0x02380000        /*64        KB        16        epi_4p_block_dummy_1                     */
#define AAS_ADDR_GOP2_4P_MAC100_1                0x02390000        /*64        KB        16        epi_4p_block_dummy_1                     */
#define AAS_ADDR_GOP2_4P_MAC100_2                0x023A0000        /*64        KB        16        epi_4p_block_dummy_1                     */
#define AAS_ADDR_GOP2_4P_MAC100_3                0x023B0000        /*64        KB        16        epi_4p_block_dummy_1                     */
#define AAS_ADDR_GOP2_4P_MAC_TAI                    0x023C0000        /*64        KB        16        epi_4p_block_dummy_1                  */
#define AAS_ADDR_GOP2_4P_MIF                        0x023D0000        /*64        KB        16        epi_4p_block_dummy_1                  */
#define AAS_ADDR_GOP2_4P_LED                        0x023E0000        /*64        KB        16        epi_4p_block_dummy_1                  */
#define AAS_ADDR_DP2_SDQ                            0x02400000        /*2080        KB        22        txqs_block_dummy_1                  */
#define AAS_ADDR_DP2_TXQS_TAI                    0x02608000        /*32        KB        15        txqs_block_dummy_1                       */
#define AAS_ADDR_DP2_QFC                            0x02610000        /*64        KB        16        txqs_block_dummy_1                    */
#define AAS_ADDR_DP2_TXF                            0x02620000        /*64        KB        16        tx_block_dummy_1                      */
#define AAS_ADDR_DP2_TXD                            0x02630000        /*64        KB        16        tx_block_dummy_1                      */
#define AAS_ADDR_DP2_RX                            0x02640000        /*64        KB        16        rx_block_dummy_1                       */
#define AAS_ADDR_GOP2_16P_PCS800                    0x02660000        /*128        KB        17        desk_block_dummy_1                   */
#define AAS_ADDR_GOP2_163E_TAI_0                    0x02680000        /*64        KB        16        pca_163e_fe_block                     */
#define AAS_ADDR_GOP2_163E_TAI_1                    0x02690000        /*64        KB        16        pca_163e_fe_block                     */
#define AAS_ADDR_GOP2_163E_TAI_2                    0x026A0000        /*64        KB        16        pca_163e_fe_block                     */
#define AAS_ADDR_GOP2_CTSU                        0x026B0000        /*64        KB        16        pca_163e_fe_block                       */
#define AAS_ADDR_GOP2_MACSEC_163E                0x026C0000        /*128        KB        17        pca_163e_fe_block                       */
#define AAS_ADDR_GOP2_MACSEC_WRP_163E            0x026E0000        /*32        KB        15        pca_163e_fe_block                        */
#define AAS_ADDR_GOP2_TX_UPACK                    0x026E8000        /*32        KB        15        pca_163e_fe_block                       */
#define AAS_ADDR_GOP2_TX_EDGE                    0x026F0000        /*32        KB        15        pca_163e_fe_block                        */
#define AAS_ADDR_GOP2_SFF                        0x026F8000        /*32        KB        15        pca_163e_fe_block                        */
#define AAS_ADDR_GOP2_MACSEC_164I                0x02700000        /*384        KB        19        pca_164i_fe_block                       */
#define AAS_ADDR_GOP2_MACSEC_WRP_164I            0x02760000        /*64        KB        16        pca_164i_fe_block                        */
#define AAS_ADDR_GOP2_RX_PACK                    0x02770000        /*32        KB        15        pca_164i_fe_block                        */
#define AAS_ADDR_GOP2_RX_EDGE                    0x02778000        /*32        KB        15        pca_164i_fe_block                        */
#define AAS_ADDR_GOP2_MACSEC_163I                0x02780000        /*128        KB        17        pca_163i_fe_block                       */
#define AAS_ADDR_GOP2_MACSEC_WRP_163I            0x027A0000        /*64        KB        16        pca_163i_fe_block                        */
#define AAS_ADDR_GOP2_ING_SHM                    0x027B0000        /*64        KB        16        pca_163i_fe_block                        */
#define AAS_ADDR_GOP2_SATG                        0x027C0000        /*64        KB        16        pca_163i_fe_block                       */
#define AAS_ADDR_GOP2_16P_SDW_56G_X4_0            0x02800000        /*512        KB        19        sdw_200g_block_dummy_5                 */
#define AAS_ADDR_GOP2_16P_SDW_56G_X4_1            0x02880000        /*512        KB        19        sdw_200g_block_dummy_6                 */
#define AAS_ADDR_GOP2_4P_SDW_56G_X4                0x02900000        /*512        KB        19        sdw_200g_block_dummy_7                */
#define AAS_ADDR_GOP2_16P_SDW_112G_X4_0            0x02980000        /*512        KB        19        sdw_400g_block_dummy_3                */
#define AAS_ADDR_GOP2_16P_SDW_112G_X4_1            0x02A00000        /*512        KB        19        sdw_400g_block_dummy_4                */
#define AAS_ADDR_GOP2_16P_ANP8_0                    0x02A80000        /*256        KB        18        anp_block_dummy_1                    */
#define AAS_ADDR_GOP2_16P_ANP8_1                    0x02AC0000        /*256        KB        18        anp_block_dummy_1                    */
#define AAS_ADDR_GOP2_80P_MAC_TAI_0                0x02B00000        /*64        KB        16        mac_32p_fe_block                       */
#define AAS_ADDR_GOP2_80P_MAC25_0                0x02B10000        /*64        KB        16        mac_32p_fe_block                         */
#define AAS_ADDR_GOP2_80P_MAC25_1                0x02B20000        /*64        KB        16        mac_32p_fe_block                         */
#define AAS_ADDR_GOP2_80P_MAC25_2                0x02B30000        /*64        KB        16        mac_32p_fe_block                         */
#define AAS_ADDR_GOP2_80P_MAC25_3                0x02B40000        /*64        KB        16        mac_32p_fe_block                         */
#define AAS_ADDR_GOP2_80P_MIF_0                    0x02B50000        /*64        KB        16        mac_32p_fe_block                       */
#define AAS_ADDR_GOP2_80P_MIF_1                    0x02B60000        /*64        KB        16        mac_32p_fe_block                       */
#define AAS_ADDR_GOP2_80P_LED_0                    0x02B70000        /*32        KB        15        mac_32p_fe_block                       */
#define AAS_ADDR_GOP2_80P_LED_1                    0x02B78000        /*32        KB        15        mac_32p_fe_block                       */
#define AAS_ADDR_GOP2_80P_MAC_TAI_1                0x02B80000        /*64        KB        16        mac_32p_fe_block_dummy_0               */
#define AAS_ADDR_GOP2_80P_MAC25_4                0x02B90000        /*64        KB        16        mac_32p_fe_block_dummy_0                 */
#define AAS_ADDR_GOP2_80P_MAC25_5                0x02BA0000        /*64        KB        16        mac_32p_fe_block_dummy_0                 */
#define AAS_ADDR_GOP2_80P_MAC25_6                0x02BB0000        /*64        KB        16        mac_32p_fe_block_dummy_0                 */
#define AAS_ADDR_GOP2_80P_MAC25_7                0x02BC0000        /*64        KB        16        mac_32p_fe_block_dummy_0                 */
#define AAS_ADDR_GOP2_80P_MIF_2                    0x02BD0000        /*64        KB        16        mac_32p_fe_block_dummy_0               */
#define AAS_ADDR_GOP2_80P_MIF_3                    0x02BE0000        /*64        KB        16        mac_32p_fe_block_dummy_0               */
#define AAS_ADDR_GOP2_80P_LED_2                    0x02BF0000        /*32        KB        15        mac_32p_fe_block_dummy_0               */
#define AAS_ADDR_GOP2_80P_LED_3                    0x02BF8000        /*32        KB        15        mac_32p_fe_block_dummy_0               */
#define AAS_ADDR_GOP2_164E_TAI                    0x02C00000        /*128        KB        17        pca_eshm_fe_block                      */
#define AAS_ADDR_GOP2_LMU_0                        0x02C20000        /*128        KB        17        pca_eshm_fe_block                     */
#define AAS_ADDR_GOP2_LMU_1                        0x02C40000        /*128        KB        17        pca_eshm_fe_block                     */
#define AAS_ADDR_GOP2_LMU_2                        0x02C60000        /*128        KB        17        pca_eshm_fe_block                     */
#define AAS_ADDR_GOP2_LMU_3                        0x02C80000        /*128        KB        17        pca_eshm_fe_block                     */
#define AAS_ADDR_GOP2_LMU_4                        0x02CA0000        /*128        KB        17        pca_eshm_fe_block                     */
#define AAS_ADDR_GOP2_LMU_5                        0x02CC0000        /*128        KB        17        pca_eshm_fe_block                     */
#define AAS_ADDR_GOP2_LMU_6                        0x02CE0000        /*128        KB        17        pca_eshm_fe_block                     */
#define AAS_ADDR_GOP2_LMU_7                        0x02D00000        /*128        KB        17        pca_eshm_fe_block                     */
#define AAS_ADDR_GOP2_PCA_PIZARB                    0x02D20000        /*128        KB        17        pca_eshm_fe_block                    */
#define AAS_ADDR_GOP2_SHM_EGR                    0x02D40000        /*128        KB        17        pca_eshm_fe_block                       */
#define AAS_ADDR_GOP2_SATC                        0x02D60000        /*128        KB        17        pca_eshm_fe_block                      */
#define AAS_ADDR_GOP2_MACSEC_WRP_164E            0x02D80000        /*256        KB        18        pca_164e_fe_block                       */
#define AAS_ADDR_GOP2_MACSEC_164E                0x02DC0000        /*256        KB        18        pca_164e_fe_block                       */
#define AAS_ADDR_GOP2_16P_MAC800_0                0x02E00000        /*64        KB        16        mac_9p_fe_block                         */
#define AAS_ADDR_GOP2_16P_MAC800_0_TAI            0x02E10000        /*64        KB        16        mac_9p_fe_block                         */
#define AAS_ADDR_GOP2_16P_LED_0                    0x02E20000        /*64        KB        16        mac_9p_fe_block                        */
#define AAS_ADDR_GOP2_16P_MIF_0                    0x02E30000        /*64        KB        16        mac_9p_fe_block                        */
#define AAS_ADDR_GOP2_16P_MAC800_1                0x02E40000        /*64        KB        16        mac_9p_fe_block_dummy_0                 */
#define AAS_ADDR_GOP2_16P_MAC800_1_TAI            0x02E50000        /*64        KB        16        mac_9p_fe_block_dummy_0                 */
#define AAS_ADDR_GOP2_16P_LED_1                    0x02E60000        /*64        KB        16        mac_9p_fe_block_dummy_0                */
#define AAS_ADDR_GOP2_16P_MIF_1                    0x02E70000        /*64        KB        16        mac_9p_fe_block_dummy_0                */
#define AAS_ADDR_DP3_PDS                            0x03000000        /*2        MB        21        pds_main_block_dummy_2                 */
#define AAS_ADDR_GOP3_4P_ANP1_0                    0x03200000        /*256        KB        18        epi_4p_block_dummy_2                  */
#define AAS_ADDR_GOP3_4P_ANP1_1                    0x03240000        /*256        KB        18        epi_4p_block_dummy_2                  */
#define AAS_ADDR_GOP3_4P_ANP1_2                    0x03280000        /*256        KB        18        epi_4p_block_dummy_2                  */
#define AAS_ADDR_GOP3_4P_ANP1_3                    0x032C0000        /*256        KB        18        epi_4p_block_dummy_2                  */
#define AAS_ADDR_GOP3_4P_PCS100_0                0x03300000        /*128        KB        17        epi_4p_block_dummy_2                    */
#define AAS_ADDR_GOP3_4P_PCS100_1                0x03320000        /*128        KB        17        epi_4p_block_dummy_2                    */
#define AAS_ADDR_GOP3_4P_PCS100_2                0x03340000        /*128        KB        17        epi_4p_block_dummy_2                    */
#define AAS_ADDR_GOP3_4P_PCS100_3                0x03360000        /*128        KB        17        epi_4p_block_dummy_2                    */
#define AAS_ADDR_GOP3_4P_MAC100_0                0x03380000        /*64        KB        16        epi_4p_block_dummy_2                     */
#define AAS_ADDR_GOP3_4P_MAC100_1                0x03390000        /*64        KB        16        epi_4p_block_dummy_2                     */
#define AAS_ADDR_GOP3_4P_MAC100_2                0x033A0000        /*64        KB        16        epi_4p_block_dummy_2                     */
#define AAS_ADDR_GOP3_4P_MAC100_3                0x033B0000        /*64        KB        16        epi_4p_block_dummy_2                     */
#define AAS_ADDR_GOP3_4P_MAC_TAI                    0x033C0000        /*64        KB        16        epi_4p_block_dummy_2                  */
#define AAS_ADDR_GOP3_4P_MIF                        0x033D0000        /*64        KB        16        epi_4p_block_dummy_2                  */
#define AAS_ADDR_GOP3_4P_LED                        0x033E0000        /*64        KB        16        epi_4p_block_dummy_2                  */
#define AAS_ADDR_DP3_SDQ                            0x03400000        /*2080        KB        22        txqs_block_dummy_2                  */
#define AAS_ADDR_DP3_TXQS_TAI                    0x03608000        /*32        KB        15        txqs_block_dummy_2                       */
#define AAS_ADDR_DP3_QFC                            0x03610000        /*64        KB        16        txqs_block_dummy_2                    */
#define AAS_ADDR_DP3_TXF                            0x03620000        /*64        KB        16        tx_block_dummy_2                      */
#define AAS_ADDR_DP3_TXD                            0x03630000        /*64        KB        16        tx_block_dummy_2                      */
#define AAS_ADDR_DP3_RX                            0x03640000        /*64        KB        16        rx_block_dummy_2                       */
#define AAS_ADDR_GOP3_16P_PCS800                    0x03660000        /*128        KB        17        desk_block_dummy_2                   */
#define AAS_ADDR_GOP3_163E_TAI_0                    0x03680000        /*64        KB        16        pca_163e_block_dummy_1                */
#define AAS_ADDR_GOP3_163E_TAI_1                    0x03690000        /*64        KB        16        pca_163e_block_dummy_1                */
#define AAS_ADDR_GOP3_163E_TAI_2                    0x036A0000        /*64        KB        16        pca_163e_block_dummy_1                */
#define AAS_ADDR_GOP3_CTSU                        0x036B0000        /*64        KB        16        pca_163e_block_dummy_1                  */
#define AAS_ADDR_GOP3_MACSEC_163E                0x036C0000        /*128        KB        17        pca_163e_block_dummy_1                  */
#define AAS_ADDR_GOP3_MACSEC_WRP_163E            0x036E0000        /*32        KB        15        pca_163e_block_dummy_1                   */
#define AAS_ADDR_GOP3_TX_UPACK                    0x036E8000        /*32        KB        15        pca_163e_block_dummy_1                  */
#define AAS_ADDR_GOP3_TX_EDGE                    0x036F0000        /*32        KB        15        pca_163e_block_dummy_1                   */
#define AAS_ADDR_GOP3_SFF                        0x036F8000        /*32        KB        15        pca_163e_block_dummy_1                   */
#define AAS_ADDR_GOP3_MACSEC_164I                0x03700000        /*384        KB        19        pca_164i_block_dummy_1                  */
#define AAS_ADDR_GOP3_MACSEC_WRP_164I            0x03760000        /*64        KB        16        pca_164i_block_dummy_1                   */
#define AAS_ADDR_GOP3_RX_PACK                    0x03770000        /*32        KB        15        pca_164i_block_dummy_1                   */
#define AAS_ADDR_GOP3_RX_EDGE                    0x03778000        /*32        KB        15        pca_164i_block_dummy_1                   */
#define AAS_ADDR_GOP3_MACSEC_163I                0x03780000        /*128        KB        17        pca_163i_block_dummy_1                  */
#define AAS_ADDR_GOP3_MACSEC_WRP_163I            0x037A0000        /*64        KB        16        pca_163i_block_dummy_1                   */
#define AAS_ADDR_GOP3_ING_SHM                    0x037B0000        /*64        KB        16        pca_163i_block_dummy_1                   */
#define AAS_ADDR_GOP3_SATG                        0x037C0000        /*64        KB        16        pca_163i_block_dummy_1                  */
#define AAS_ADDR_GOP3_16P_SDW_56G_X4_0            0x03800000        /*512        KB        19        sdw_200g_block_dummy_8                 */
#define AAS_ADDR_GOP3_16P_SDW_56G_X4_1            0x03880000        /*512        KB        19        sdw_200g_block_dummy_9                 */
#define AAS_ADDR_GOP3_4P_SDW_56G_X4                0x03900000        /*512        KB        19        sdw_200g_block_dummy_10               */
#define AAS_ADDR_GOP3_16P_SDW_112G_X4_0            0x03980000        /*512        KB        19        sdw_400g_block_dummy_5                */
#define AAS_ADDR_GOP3_16P_SDW_112G_X4_1            0x03A00000        /*512        KB        19        sdw_400g_block_dummy_6                */
#define AAS_ADDR_GOP3_16P_ANP8_0                    0x03A80000        /*256        KB        18        anp_block_dummy_2                    */
#define AAS_ADDR_GOP3_16P_ANP8_1                    0x03AC0000        /*256        KB        18        anp_block_dummy_2                    */
#define AAS_ADDR_GOP3_164E_TAI                    0x03C00000        /*128        KB        17        pca_164e_block_dummy_1                 */
#define AAS_ADDR_GOP3_LMU_0                        0x03C20000        /*128        KB        17        pca_164e_block_dummy_1                */
#define AAS_ADDR_GOP3_LMU_1                        0x03C40000        /*128        KB        17        pca_164e_block_dummy_1                */
#define AAS_ADDR_GOP3_LMU_2                        0x03C60000        /*128        KB        17        pca_164e_block_dummy_1                */
#define AAS_ADDR_GOP3_LMU_3                        0x03C80000        /*128        KB        17        pca_164e_block_dummy_1                */
#define AAS_ADDR_GOP3_LMU_4                        0x03CA0000        /*128        KB        17        pca_164e_block_dummy_1                */
#define AAS_ADDR_GOP3_LMU_5                        0x03CC0000        /*128        KB        17        pca_164e_block_dummy_1                */
#define AAS_ADDR_GOP3_LMU_6                        0x03CE0000        /*128        KB        17        pca_164e_block_dummy_1                */
#define AAS_ADDR_GOP3_LMU_7                        0x03D00000        /*128        KB        17        pca_164e_block_dummy_1                */
#define AAS_ADDR_GOP3_PCA_PIZARB                    0x03D20000        /*128        KB        17        pca_164e_block_dummy_1               */
#define AAS_ADDR_GOP3_SHM_EGR                    0x03D40000        /*128        KB        17        pca_164e_block_dummy_1                  */
#define AAS_ADDR_GOP3_SATC                        0x03D60000        /*128        KB        17        pca_164e_block_dummy_1                 */
#define AAS_ADDR_GOP3_MACSEC_WRP_164E            0x03D80000        /*256        KB        18        pca_164e_block_dummy_1                  */
#define AAS_ADDR_GOP3_MACSEC_164E                0x03DC0000        /*256        KB        18        pca_164e_block_dummy_1                  */
#define AAS_ADDR_GOP3_16P_MAC800_0                0x03E00000        /*64        KB        16        mac_9p_block_dummy_3                    */
#define AAS_ADDR_GOP3_16P_MAC800_0_TAI            0x03E10000        /*64        KB        16        mac_9p_block_dummy_3                    */
#define AAS_ADDR_GOP3_16P_LED_0                    0x03E20000        /*64        KB        16        mac_9p_block_dummy_3                   */
#define AAS_ADDR_GOP3_16P_MIF_0                    0x03E30000        /*64        KB        16        mac_9p_block_dummy_3                   */
#define AAS_ADDR_GOP3_16P_MAC800_1                0x03E40000        /*64        KB        16        mac_9p_block_dummy_4                    */
#define AAS_ADDR_GOP3_16P_MAC800_1_TAI            0x03E50000        /*64        KB        16        mac_9p_block_dummy_4                    */
#define AAS_ADDR_GOP3_16P_LED_1                    0x03E60000        /*64        KB        16        mac_9p_block_dummy_4                   */
#define AAS_ADDR_GOP3_16P_MIF_1                    0x03E70000        /*64        KB        16        mac_9p_block_dummy_4                   */
#define AAS_ADDR_DP4_PDS                            0x04000000        /*2        MB        21        pds_epb_main_block                     */
#define AAS_ADDR_GOP2_80P_XC_0                    0x04200000        /*512        KB        19        fe_xc_block                            */
#define AAS_ADDR_GOP2_80P_XC_TAI_0                0x04280000        /*256        KB        18        fe_xc_block                            */
#define AAS_ADDR_EPB                                0x042C0000        /*256        KB        18        epb_block                            */
#define AAS_ADDR_GOP2_80P_XC_1                    0x04300000        /*512        KB        19        fe_xc_block_dummy_0                    */
#define AAS_ADDR_GOP2_80P_XC_TAI_1                0x04380000        /*256        KB        18        fe_xc_block_dummy_0                    */
#define AAS_ADDR_GOP2_80P_CODERS                    0x043C0000        /*256        KB        18        fe_coders_block                      */
#define AAS_ADDR_GOP2_80P_SHIM_0                    0x04400000        /*256        KB        18        fe_shim_block                        */
#define AAS_ADDR_GOP2_80P_SHIM_TAI_0                0x04440000        /*256        KB        18        fe_shim_block                        */
#define AAS_ADDR_GOP2_80P_SHIM_1                    0x04480000        /*256        KB        18        fe_shim_block_dummy_0                */
#define AAS_ADDR_GOP2_80P_SHIM_TAI_1                0x044C0000        /*256        KB        18        fe_shim_block_dummy_0                */
#define AAS_ADDR_DP4_TXF                            0x04620000        /*64        KB        16        tx_block_dummy_3                      */
#define AAS_ADDR_DP4_TXD                            0x04630000        /*64        KB        16        tx_block_dummy_3                      */
#define AAS_ADDR_DP4_RX                            0x04640000        /*64        KB        16        rx_block_dummy_3                       */
#define AAS_ADDR_IA                                0x04660000        /*64        KB        16        ia_block                               */
#define AAS_ADDR_TTI_LU                            0x04670000        /*64        KB        16        tti_lu_block                           */
#define AAS_ADDR_PPU1                            0x04700000        /*512        KB        19        pcl_block                               */
#define AAS_ADDR_PCL                                0x04780000        /*512        KB        19        pcl_block                            */
#define AAS_ADDR_EM0                                0x04800000        /*512        KB        19        em_block                             */
#define AAS_ADDR_EM1                                0x04880000        /*512        KB        19        em_block                             */
#define AAS_ADDR_EM2                                0x04900000        /*512        KB        19        em_block                             */
#define AAS_ADDR_EM3                                0x04980000        /*512        KB        19        em_block                             */
#define AAS_ADDR_EM4                                0x04A00000        /*512        KB        19        em_block_dummy_0                     */
#define AAS_ADDR_EM5                                0x04A80000        /*512        KB        19        em_block_dummy_0                     */
#define AAS_ADDR_EM6                                0x04B00000        /*512        KB        19        em_block_dummy_0                     */
#define AAS_ADDR_EM7                                0x04B80000        /*512        KB        19        em_block_dummy_0                     */
#define AAS_ADDR_CNC1                            0x05000000        /*8        MB        23        smem1_block                               */
#define AAS_ADDR_CNC3                            0x05800000        /*8        MB        23        smem1_block                               */
#define AAS_ADDR_LPM                                0x06000000        /*16        MB        24        l2i_ipvx_block                        */
#define AAS_ADDR_L2I                                0x07000000        /*8        MB        23        l2i_ipvx_block                         */
#define AAS_ADDR_IPVX                            0x07800000        /*8        MB        23        l2i_ipvx_block                            */
#define AAS_ADDR_IPLR0                            0x08000000        /*32        MB        25        iplr_mll_block                          */
#define AAS_ADDR_IPLR1                            0x0A000000        /*32        MB        25        iplr_mll_block                          */
#define AAS_ADDR_MLL                                0x0C000000        /*36        MB        26        iplr_mll_block                        */
#define AAS_ADDR_IPLR_TAI                        0x0E400000        /*4        MB        22        iplr_mll_block                            */
#define AAS_ADDR_CNC0                            0x0F000000        /*8        MB        23        smem0_block                               */
#define AAS_ADDR_CNC2                            0x0F800000        /*7        MB        23        smem0_block                               */
#define AAS_ADDR_SHM0                            0x0FF00000        /*1        MB        20        smem0_block                               */
#define AAS_ADDR_DDR0_PHY                        0x10000000        /*32        MB        25        lpddr5x_phy_block                        */
#define AAS_ADDR_DDR0                            0x12000000        /*2        MB        21        lpddr5x_mctl_block                        */
#define AAS_ADDR_DDR0_IFBIST                        0x12200000        /*1        MB        20        lpddr5x_mctl_block                     */
#define AAS_ADDR_DDR0_MCTL                        0x12300000        /*1        MB        20        lpddr5x_mctl_block                       */
#define AAS_ADDR_TTI                                0x12800000        /*4        MB        22        tti_block                              */
#define AAS_ADDR_TTI_TAI_0                        0x12C00000        /*1        MB        20        tti_block                                */
#define AAS_ADDR_TTI_TAI_1                        0x12D00000        /*1        MB        20        tti_block                                */
#define AAS_ADDR_TTI_TAI_2                        0x12E00000        /*1        MB        20        tti_block                                */
#define AAS_ADDR_EGF_SHT                            0x13000000        /*8        MB        23        egf_block                              */
#define AAS_ADDR_EGF_QAG                            0x13800000        /*3        MB        22        egf_block                              */
#define AAS_ADDR_EGF_EFT                            0x13B00000        /*1        MB        20        egf_block                              */
#define AAS_ADDR_IPE                                0x14000000        /*62        MB        26        ipe_block                             */
#define AAS_ADDR_PPU2                            0x17E00000        /*1        MB        20        ipe_block                                 */
#define AAS_ADDR_IPE_TAI                            0x17F00000        /*1        MB        20        ipe_block                              */
#define AAS_ADDR_DDR1_PHY                        0x18000000        /*32        MB        25        lpddr5x_phy_block_dummy_0                */
#define AAS_ADDR_DDR1                            0x1A000000        /*2        MB        21        lpddr5x_mctl_block_dummy_0                */
#define AAS_ADDR_DDR1_IFBIST                        0x1A200000        /*1        MB        20        lpddr5x_mctl_block_dummy_0             */
#define AAS_ADDR_DDR1_MCTL                        0x1A300000        /*1        MB        20        lpddr5x_mctl_block_dummy_0               */
#define AAS_ADDR_TCAM                            0x1A800000        /*8        MB        23        tcam_block                                */
#define AAS_ADDR_PHA                                0x1B000000        /*8        MB        23        pha_block                              */
#define AAS_ADDR_PHA_TAI                            0x1B800000        /*2        MB        21        pha_block                              */
#define AAS_ADDR_EPLR                            0x1C000000        /*32        MB        25        eoam_block                               */
#define AAS_ADDR_PHB                                0x1E000000        /*16        MB        24        eoam_block                            */
#define AAS_ADDR_ERMRK                            0x1F000000        /*4        MB        22        eoam_block                               */
#define AAS_ADDR_EOAM                            0x1F400000        /*4        MB        22        eoam_block                                */
#define AAS_ADDR_EOAM_TAI_0                        0x1F800000        /*256        KB        18        eoam_block                            */
#define AAS_ADDR_EOAM_TAI_1                        0x1F840000        /*256        KB        18        eoam_block                            */
#define AAS_ADDR_EOAM_TAI_2                        0x1F880000        /*256        KB        18        eoam_block                            */
#define AAS_ADDR_EPLR_TAI                        0x1F8C0000        /*256        KB        18        eoam_block                              */
#define AAS_ADDR_DDR2_PHY                        0x20000000        /*32        MB        25        lpddr5x_phy_block_dummy_1                */
#define AAS_ADDR_DDR2                            0x22000000        /*2        MB        21        lpddr5x_mctl_block_dummy_1                */
#define AAS_ADDR_DDR2_IFBIST                        0x22200000        /*1        MB        20        lpddr5x_mctl_block_dummy_1             */
#define AAS_ADDR_DDR2_MCTL                        0x22300000        /*1        MB        20        lpddr5x_mctl_block_dummy_1               */
#define AAS_ADDR_IOAM                            0x22800000        /*4        MB        22        upper_ing_block                           */
#define AAS_ADDR_SMU                                0x22C00000        /*2        MB        21        upper_ing_block                        */
#define AAS_ADDR_MPCL                            0x22E00000        /*1        MB        20        upper_ing_block                           */
#define AAS_ADDR_UPPER_ING_TAI                    0x22F00000        /*1        MB        20        upper_ing_block                          */
#define AAS_ADDR_PREQ                            0x23000000        /*4        MB        22        epcl_block                                */
#define AAS_ADDR_EPCL                            0x23400000        /*2        MB        21        epcl_block                                */
#define AAS_ADDR_EMX                                0x23600000        /*512        KB        19        epcl_block                           */
#define AAS_ADDR_EM_ILM0                            0x23680000        /*512        KB        19        epcl_block                           */
#define AAS_ADDR_EQ                                0x24000000        /*24        MB        25        eq_block                               */
#define AAS_ADDR_DDR3_PHY                        0x28000000        /*32        MB        25        lpddr5x_phy_block_dummy_2                */
#define AAS_ADDR_DDR3                            0x2A000000        /*2        MB        21        lpddr5x_mctl_block_dummy_2                */
#define AAS_ADDR_DDR3_IFBIST                        0x2A200000        /*1        MB        20        lpddr5x_mctl_block_dummy_2             */
#define AAS_ADDR_DDR3_MCTL                        0x2A300000        /*1        MB        20        lpddr5x_mctl_block_dummy_2               */
#define AAS_ADDR_HA                                0x30000000        /*124        MB        27        ha_block                              */
#define AAS_ADDR_HA_TAI                            0x37C00000        /*1        MB        20        ha_block                                */
#define AAS_ADDR_FDB                                0x38000000        /*34        MB        26        hbu_block                             */
#define AAS_ADDR_BMA                                0x3A200000        /*2        MB        21        hbu_block                              */
#define AAS_ADDR_HBU                                0x3A400000        /*1        MB        20        hbu_block                              */
#define AAS_ADDR_EREP                            0x3A500000        /*1        MB        20        hbu_block                                 */
#define AAS_ADDR_GPW0                            0x3C000000        /*256        KB        18        gpw_center_block                        */
#define AAS_ADDR_GPW1                            0x3C040000        /*256        KB        18        gpw_center_block                        */
#define AAS_ADDR_GPW2                            0x3C080000        /*256        KB        18        gpw_center_block                        */
#define AAS_ADDR_GPW3                            0x3C0C0000        /*256        KB        18        gpw_center_block                        */
#define AAS_ADDR_GPW4                            0x3C100000        /*256        KB        18        gpw_center_block                        */
#define AAS_ADDR_WA0                                0x3C140000        /*256        KB        18        gpw_center_block                     */
#define AAS_ADDR_WA1                                0x3C180000        /*256        KB        18        gpw_center_block                     */
#define AAS_ADDR_PB_COUNTER                        0x3C1C0000        /*256        KB        18        gpw_center_block                      */
#define AAS_ADDR_GCR0                            0x3C200000        /*256        KB        18        gcr_block                               */
#define AAS_ADDR_GCR1                            0x3C240000        /*256        KB        18        gcr_block                               */
#define AAS_ADDR_GCR2                            0x3C280000        /*256        KB        18        gcr_block                               */
#define AAS_ADDR_GCR3                            0x3C2C0000        /*256        KB        18        gcr_block                               */
#define AAS_ADDR_GCR4                            0x3C300000        /*256        KB        18        gcr_block                               */
#define AAS_ADDR_NPM0                            0x3C340000        /*256        KB        18        npm_block                               */
#define AAS_ADDR_NPM1                            0x3C380000        /*256        KB        18        npm_block_dummy_0                       */
#define AAS_ADDR_SMB0                            0x3C3C0000        /*256        KB        18        smb_block                               */
#define AAS_ADDR_SMB1                            0x3C400000        /*256        KB        18        smb_block_dummy_0                       */
#define AAS_ADDR_SMB2                            0x3C440000        /*256        KB        18        smb_block_dummy_1                       */
#define AAS_ADDR_GPR0                            0x3C480000        /*256        KB        18        gpr2_block                              */
#define AAS_ADDR_GPR1                            0x3C4C0000        /*256        KB        18        gpr2_block                              */
#define AAS_ADDR_GPR2                            0x3C500000        /*256        KB        18        gpr3_block                              */
#define AAS_ADDR_GPR3                            0x3C540000        /*256        KB        18        gpr3_block                              */
#define AAS_ADDR_GPR4                            0x3C580000        /*256        KB        18        gpr3_block                              */
#define AAS_ADDR_PSI                                0x3C800000        /*4        MB        22        psi_block                              */
#define AAS_ADDR_PDX_PDITX                        0x3CC00000        /*128        KB        17        pdx_block                              */
#define AAS_ADDR_PDX_PACKER                        0x3CC20000        /*64        KB        16        pdx_block                              */
#define AAS_ADDR_PFCC                            0x3CC30000        /*64        KB        16        pdx_block                                */
#define AAS_ADDR_DFX_SERVER                        0x3D000000        /*1        MB        20        server_block                            */
#define AAS_ADDR_EXT_IPs_SERVER                    0x3D100000        /*1        MB        20        server_block                            */
#define AAS_ADDR_GOP1_4P_MASTER_S_TAI            0x3D200000        /*256        KB        18        server_block                            */
#define AAS_ADDR_GOP1_4P_MASTER_TAI_0            0x3D240000        /*256        KB        18        server_block                            */
#define AAS_ADDR_GOP1_4P_MASTER_TAI_1            0x3D280000        /*256        KB        18        server_block                            */
#define AAS_ADDR_GOP1_4P_MASTER_TAI_2            0x3D2C0000        /*256        KB        18        server_block                            */

#define AAS_ADDR_CENTRAL_TOTAL                   0x3C000000       /*       64        MB        26       */

#define BASE_DUMMY                      0x3EF00000

/* dummy for GM */
#define AAS_ADDR_MG0                    BASE_DUMMY               /*1M*/
#define AAS_ADDR_MG1                    (AAS_ADDR_MG0 + _1M)     /*1M*/
#define AAS_ADDR_MIB                    0x7ff00000               /*512K*/
#define AAS_ADDR_WM_IPC                 BASE_DUMMY               /*1K*/ /* A unit for WM so CPSS can sent IPC commands to WM :  needed in Sip7 as MG unit not exists */

#define KB            (0x400)
#define MB            (0x100000)

#define AAS_UNIT_SIZE_DP0_PDS                      (      2        *      MB         )
#define AAS_UNIT_SIZE_GOP0_4P_ANP1_0               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP0_4P_ANP1_1               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP0_4P_ANP1_2               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP0_4P_ANP1_3               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP0_4P_PCS100_0             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP0_4P_PCS100_1             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP0_4P_PCS100_2             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP0_4P_PCS100_3             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP0_4P_MAC100_0             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP0_4P_MAC100_1             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP0_4P_MAC100_2             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP0_4P_MAC100_3             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP0_4P_MAC_TAI              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP0_4P_MIF                  (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP0_4P_LED                  (     64        *      KB        )
#define AAS_UNIT_SIZE_DP0_SDQ                      (     2080        *      KB      )
#define AAS_UNIT_SIZE_DP0_TXQS_TAI                 (  32        *      KB           )
#define AAS_UNIT_SIZE_DP0_QFC                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP0_TXF                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP0_TXD                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP0_RX                       (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP0_16P_PCS800              (     128        *      KB       )
#define AAS_UNIT_SIZE_GOP0_163E_TAI_0              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP0_163E_TAI_1              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP0_163E_TAI_2              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP0_CTSU                    (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP0_MACSEC_163E             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP0_MACSEC_WRP_163E         (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP0_TX_UPACK                (   32        *      KB          )
#define AAS_UNIT_SIZE_GOP0_TX_EDGE                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP0_SFF                     (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP0_MACSEC_164I             (  384        *      KB          )
#define AAS_UNIT_SIZE_GOP0_MACSEC_WRP_164I         (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP0_RX_PACK                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP0_RX_EDGE                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP0_MACSEC_163I             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP0_MACSEC_WRP_163I         (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP0_ING_SHM                 (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP0_SATG                    (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP0_16P_SDW_56G_X4_0        (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP0_16P_SDW_56G_X4_1        (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP0_4P_SDW_56G_X4           (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP0_16P_SDW_112G_X4_0       (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP0_16P_SDW_112G_X4_1       (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP0_16P_ANP8_0              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP0_16P_ANP8_1              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP0_164E_TAI                (   128        *      KB         )
#define AAS_UNIT_SIZE_GOP0_LMU_0                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP0_LMU_1                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP0_LMU_2                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP0_LMU_3                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP0_LMU_4                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP0_LMU_5                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP0_LMU_6                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP0_LMU_7                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP0_PCA_PIZARB              (     128        *      KB       )
#define AAS_UNIT_SIZE_GOP0_SHM_EGR                 (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP0_SATC                    (   128        *      KB         )
#define AAS_UNIT_SIZE_GOP0_MACSEC_WRP_164E         (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP0_MACSEC_164E             (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP0_16P_MAC800_0            (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP0_16P_MAC800_0_TAI        (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP0_16P_LED_0               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP0_16P_MIF_0               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP0_16P_MAC800_1            (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP0_16P_MAC800_1_TAI        (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP0_16P_LED_1               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP0_16P_MIF_1               (    64        *      KB         )
#define AAS_UNIT_SIZE_DP1_PDS                      (     2        *      MB         )
#define AAS_UNIT_SIZE_GOP1_4P_ANP1_0               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP1_4P_ANP1_1               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP1_4P_ANP1_2               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP1_4P_ANP1_3               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP1_4P_PCS100_0             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP1_4P_PCS100_1             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP1_4P_PCS100_2             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP1_4P_PCS100_3             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP1_4P_MAC100_0             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP1_4P_MAC100_1             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP1_4P_MAC100_2             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP1_4P_MAC100_3             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP1_4P_MAC_TAI              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP1_4P_MIF                  (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP1_4P_LED                  (     64        *      KB        )
#define AAS_UNIT_SIZE_DP1_SDQ                      (     2080        *      KB      )
#define AAS_UNIT_SIZE_DP1_TXQS_TAI                 (  32        *      KB           )
#define AAS_UNIT_SIZE_DP1_QFC                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP1_TXF                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP1_TXD                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP1_RX                       (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP1_16P_PCS800              (     128        *      KB       )
#define AAS_UNIT_SIZE_GOP1_163E_TAI_0              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP1_163E_TAI_1              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP1_163E_TAI_2              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP1_CTSU                    (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP1_MACSEC_163E             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP1_MACSEC_WRP_163E         (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP1_TX_UPACK                (   32        *      KB          )
#define AAS_UNIT_SIZE_GOP1_TX_EDGE                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP1_SFF                     (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP1_MACSEC_164I             (  384        *      KB          )
#define AAS_UNIT_SIZE_GOP1_MACSEC_WRP_164I         (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP1_RX_PACK                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP1_RX_EDGE                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP1_MACSEC_163I             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP1_MACSEC_WRP_163I         (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP1_ING_SHM                 (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP1_SATG                    (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP1_16P_SDW_56G_X4_0        (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP1_16P_SDW_56G_X4_1        (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP1_4P_SDW_56G_X4           (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP1_16P_SDW_112G_X4_0       (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP1_16P_SDW_112G_X4_1       (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP1_16P_ANP8_0              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP1_16P_ANP8_1              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP1_CPU_SDW_56G_X1          (     512        *      KB       )
#define AAS_UNIT_SIZE_GOP1_CPU_ANP1                (   256        *      KB         )
#define AAS_UNIT_SIZE_GOP1_CPU_MAC100              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP1_CPU_MAC_TAI             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP1_CPU_PCS100              (     32        *      KB        )
#define AAS_UNIT_SIZE_GOP1_CPU_LED                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP1_CPU_MIF                 (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP1_164E_TAI                (   128        *      KB         )
#define AAS_UNIT_SIZE_GOP1_LMU_0                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP1_LMU_1                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP1_LMU_2                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP1_LMU_3                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP1_LMU_4                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP1_LMU_5                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP1_LMU_6                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP1_LMU_7                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP1_PCA_PIZARB              (     128        *      KB       )
#define AAS_UNIT_SIZE_GOP1_SHM_EGR                 (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP1_SATC                    (   128        *      KB         )
#define AAS_UNIT_SIZE_GOP1_MACSEC_WRP_164E         (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP1_MACSEC_164E             (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP1_16P_MAC800_0            (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP1_16P_MAC800_0_TAI        (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP1_16P_LED_0               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP1_16P_MIF_0               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP1_16P_MAC800_1            (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP1_16P_MAC800_1_TAI        (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP1_16P_LED_1               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP1_16P_MIF_1               (    64        *      KB         )
#define AAS_UNIT_SIZE_DP2_PDS                      (     2        *      MB         )
#define AAS_UNIT_SIZE_GOP2_4P_ANP1_0               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP2_4P_ANP1_1               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP2_4P_ANP1_2               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP2_4P_ANP1_3               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP2_4P_PCS100_0             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP2_4P_PCS100_1             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP2_4P_PCS100_2             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP2_4P_PCS100_3             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP2_4P_MAC100_0             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_4P_MAC100_1             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_4P_MAC100_2             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_4P_MAC100_3             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_4P_MAC_TAI              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP2_4P_MIF                  (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP2_4P_LED                  (     64        *      KB        )
#define AAS_UNIT_SIZE_DP2_SDQ                      (     2080        *      KB      )
#define AAS_UNIT_SIZE_DP2_TXQS_TAI                 (  32        *      KB           )
#define AAS_UNIT_SIZE_DP2_QFC                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP2_TXF                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP2_TXD                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP2_RX                       (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_16P_PCS800              (     128        *      KB       )
#define AAS_UNIT_SIZE_GOP2_163E_TAI_0              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP2_163E_TAI_1              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP2_163E_TAI_2              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP2_CTSU                    (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP2_MACSEC_163E             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP2_MACSEC_WRP_163E         (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP2_TX_UPACK                (   32        *      KB          )
#define AAS_UNIT_SIZE_GOP2_TX_EDGE                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP2_SFF                     (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP2_MACSEC_164I             (  384        *      KB          )
#define AAS_UNIT_SIZE_GOP2_MACSEC_WRP_164I         (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_RX_PACK                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP2_RX_EDGE                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP2_MACSEC_163I             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP2_MACSEC_WRP_163I         (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_ING_SHM                 (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_SATG                    (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP2_16P_SDW_56G_X4_0        (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP2_16P_SDW_56G_X4_1        (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP2_4P_SDW_56G_X4           (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP2_16P_SDW_112G_X4_0       (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP2_16P_SDW_112G_X4_1       (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP2_16P_ANP8_0              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP2_16P_ANP8_1              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP2_80P_MAC_TAI_0           (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_MAC25_0             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_80P_MAC25_1             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_80P_MAC25_2             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_80P_MAC25_3             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_80P_MIF_0               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_MIF_1               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_LED_0               (    32        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_LED_1               (    32        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_MAC_TAI_1           (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_MAC25_4             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_80P_MAC25_5             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_80P_MAC25_6             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_80P_MAC25_7             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP2_80P_MIF_2               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_MIF_3               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_LED_2               (    32        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_LED_3               (    32        *      KB         )
#define AAS_UNIT_SIZE_GOP2_164E_TAI                (   128        *      KB         )
#define AAS_UNIT_SIZE_GOP2_LMU_0                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP2_LMU_1                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP2_LMU_2                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP2_LMU_3                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP2_LMU_4                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP2_LMU_5                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP2_LMU_6                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP2_LMU_7                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP2_PCA_PIZARB              (     128        *      KB       )
#define AAS_UNIT_SIZE_GOP2_SHM_EGR                 (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP2_SATC                    (   128        *      KB         )
#define AAS_UNIT_SIZE_GOP2_MACSEC_WRP_164E         (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP2_MACSEC_164E             (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP2_16P_MAC800_0            (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP2_16P_MAC800_0_TAI        (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP2_16P_LED_0               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_16P_MIF_0               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_16P_MAC800_1            (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP2_16P_MAC800_1_TAI        (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP2_16P_LED_1               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP2_16P_MIF_1               (    64        *      KB         )
#define AAS_UNIT_SIZE_DP3_PDS                      (     2        *      MB         )
#define AAS_UNIT_SIZE_GOP3_4P_ANP1_0               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP3_4P_ANP1_1               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP3_4P_ANP1_2               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP3_4P_ANP1_3               (    256        *      KB        )
#define AAS_UNIT_SIZE_GOP3_4P_PCS100_0             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP3_4P_PCS100_1             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP3_4P_PCS100_2             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP3_4P_PCS100_3             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP3_4P_MAC100_0             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP3_4P_MAC100_1             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP3_4P_MAC100_2             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP3_4P_MAC100_3             (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP3_4P_MAC_TAI              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP3_4P_MIF                  (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP3_4P_LED                  (     64        *      KB        )
#define AAS_UNIT_SIZE_DP3_SDQ                      (     2080        *      KB      )
#define AAS_UNIT_SIZE_DP3_TXQS_TAI                 (  32        *      KB           )
#define AAS_UNIT_SIZE_DP3_QFC                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP3_TXF                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP3_TXD                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP3_RX                       (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP3_16P_PCS800              (     128        *      KB       )
#define AAS_UNIT_SIZE_GOP3_163E_TAI_0              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP3_163E_TAI_1              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP3_163E_TAI_2              (     64        *      KB        )
#define AAS_UNIT_SIZE_GOP3_CTSU                    (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP3_MACSEC_163E             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP3_MACSEC_WRP_163E         (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP3_TX_UPACK                (   32        *      KB          )
#define AAS_UNIT_SIZE_GOP3_TX_EDGE                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP3_SFF                     (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP3_MACSEC_164I             (  384        *      KB          )
#define AAS_UNIT_SIZE_GOP3_MACSEC_WRP_164I         (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP3_RX_PACK                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP3_RX_EDGE                 (  32        *      KB           )
#define AAS_UNIT_SIZE_GOP3_MACSEC_163I             (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP3_MACSEC_WRP_163I         (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP3_ING_SHM                 (  64        *      KB           )
#define AAS_UNIT_SIZE_GOP3_SATG                    (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP3_16P_SDW_56G_X4_0        (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP3_16P_SDW_56G_X4_1        (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP3_4P_SDW_56G_X4           (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP3_16P_SDW_112G_X4_0       (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP3_16P_SDW_112G_X4_1       (    512        *      KB        )
#define AAS_UNIT_SIZE_GOP3_16P_ANP8_0              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP3_16P_ANP8_1              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP3_164E_TAI                (   128        *      KB         )
#define AAS_UNIT_SIZE_GOP3_LMU_0                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP3_LMU_1                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP3_LMU_2                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP3_LMU_3                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP3_LMU_4                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP3_LMU_5                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP3_LMU_6                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP3_LMU_7                   (    128        *      KB        )
#define AAS_UNIT_SIZE_GOP3_PCA_PIZARB              (     128        *      KB       )
#define AAS_UNIT_SIZE_GOP3_SHM_EGR                 (  128        *      KB          )
#define AAS_UNIT_SIZE_GOP3_SATC                    (   128        *      KB         )
#define AAS_UNIT_SIZE_GOP3_MACSEC_WRP_164E         (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP3_MACSEC_164E             (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP3_16P_MAC800_0            (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP3_16P_MAC800_0_TAI        (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP3_16P_LED_0               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP3_16P_MIF_0               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP3_16P_MAC800_1            (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP3_16P_MAC800_1_TAI        (   64        *      KB          )
#define AAS_UNIT_SIZE_GOP3_16P_LED_1               (    64        *      KB         )
#define AAS_UNIT_SIZE_GOP3_16P_MIF_1               (    64        *      KB         )
#define AAS_UNIT_SIZE_DP4_PDS                      (     2        *      MB         )
#define AAS_UNIT_SIZE_GOP2_80P_XC_0                (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_XC_TAI_0            (   256        *      KB         )
#define AAS_UNIT_SIZE_EPB                          (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP2_80P_XC_1                (   512        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_XC_TAI_1            (   256        *      KB         )
#define AAS_UNIT_SIZE_GOP2_80P_CODERS              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP2_80P_SHIM_0              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP2_80P_SHIM_TAI_0          (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP2_80P_SHIM_1              (     256        *      KB       )
#define AAS_UNIT_SIZE_GOP2_80P_SHIM_TAI_1          (     256        *      KB       )
#define AAS_UNIT_SIZE_DP4_TXF                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP4_TXD                      (     64        *      KB        )
#define AAS_UNIT_SIZE_DP4_RX                       (    64        *      KB         )
#define AAS_UNIT_SIZE_IA                           (    64        *      KB         )
#define AAS_UNIT_SIZE_TTI_LU                       (    64        *      KB         )
#define AAS_UNIT_SIZE_PPU1                         (  512        *      KB          )
#define AAS_UNIT_SIZE_PCL                          (     512        *      KB       )
#define AAS_UNIT_SIZE_EM0                          (     512        *      KB       )
#define AAS_UNIT_SIZE_EM1                          (     512        *      KB       )
#define AAS_UNIT_SIZE_EM2                          (     512        *      KB       )
#define AAS_UNIT_SIZE_EM3                          (     512        *      KB       )
#define AAS_UNIT_SIZE_EM4                          (     512        *      KB       )
#define AAS_UNIT_SIZE_EM5                          (     512        *      KB       )
#define AAS_UNIT_SIZE_EM6                          (     512        *      KB       )
#define AAS_UNIT_SIZE_EM7                          (     512        *      KB       )
#define AAS_UNIT_SIZE_CNC1                         (  8        *      MB            )
#define AAS_UNIT_SIZE_CNC3                         (  8        *      MB            )
#define AAS_UNIT_SIZE_LPM                          (     16        *      MB        )
#define AAS_UNIT_SIZE_L2I                          (     8        *      MB         )
#define AAS_UNIT_SIZE_IPVX                         (  8        *      MB            )
#define AAS_UNIT_SIZE_IPLR0                        (   32        *      MB          )
#define AAS_UNIT_SIZE_IPLR1                        (   32        *      MB          )
#define AAS_UNIT_SIZE_MLL                          (     36        *      MB        )
#define AAS_UNIT_SIZE_IPLR_TAI                     (  4        *      MB            )
#define AAS_UNIT_SIZE_CNC0                         (  8        *      MB            )
#define AAS_UNIT_SIZE_CNC2                         (  7        *      MB            )
#define AAS_UNIT_SIZE_SHM0                         (  1        *      MB            )
#define AAS_UNIT_SIZE_DDR0_PHY                     (  32        *      MB           )
#define AAS_UNIT_SIZE_DDR0                         (  2        *      MB            )
#define AAS_UNIT_SIZE_DDR0_IFBIST                  (     1        *      MB         )
#define AAS_UNIT_SIZE_DDR0_MCTL                    (   1        *      MB           )
#define AAS_UNIT_SIZE_TTI                          (     4        *      MB         )
#define AAS_UNIT_SIZE_TTI_TAI_0                    (   1        *      MB           )
#define AAS_UNIT_SIZE_TTI_TAI_1                    (   1        *      MB           )
#define AAS_UNIT_SIZE_TTI_TAI_2                    (   1        *      MB           )
#define AAS_UNIT_SIZE_EGF_SHT                      (     8        *      MB         )
#define AAS_UNIT_SIZE_EGF_QAG                      (     3        *      MB         )
#define AAS_UNIT_SIZE_EGF_EFT                      (     1        *      MB         )
#define AAS_UNIT_SIZE_IPE                          (     62        *      MB        )
#define AAS_UNIT_SIZE_PPU2                         (  1        *      MB            )
#define AAS_UNIT_SIZE_IPE_TAI                      (     1        *      MB         )
#define AAS_UNIT_SIZE_DDR1_PHY                     (  32        *      MB           )
#define AAS_UNIT_SIZE_DDR1                         (  2        *      MB            )
#define AAS_UNIT_SIZE_DDR1_IFBIST                  (     1        *      MB         )
#define AAS_UNIT_SIZE_DDR1_MCTL                    (   1        *      MB           )
#define AAS_UNIT_SIZE_TCAM                         (  8        *      MB            )
#define AAS_UNIT_SIZE_PHA                          (     8        *      MB         )
#define AAS_UNIT_SIZE_PHA_TAI                      (     2        *      MB         )
#define AAS_UNIT_SIZE_EPLR                         (  32        *      MB           )
#define AAS_UNIT_SIZE_PHB                          (     16        *      MB        )
#define AAS_UNIT_SIZE_ERMRK                        (   4        *      MB           )
#define AAS_UNIT_SIZE_EOAM                         (  4        *      MB            )
#define AAS_UNIT_SIZE_EOAM_TAI_0                   (    256        *      KB        )
#define AAS_UNIT_SIZE_EOAM_TAI_1                   (    256        *      KB        )
#define AAS_UNIT_SIZE_EOAM_TAI_2                   (    256        *      KB        )
#define AAS_UNIT_SIZE_EPLR_TAI                     (  256        *      KB          )
#define AAS_UNIT_SIZE_DDR2_PHY                     (  32        *      MB           )
#define AAS_UNIT_SIZE_DDR2                         (  2        *      MB            )
#define AAS_UNIT_SIZE_DDR2_IFBIST                  (     1        *      MB         )
#define AAS_UNIT_SIZE_DDR2_MCTL                    (   1        *      MB           )
#define AAS_UNIT_SIZE_IOAM                         (  4        *      MB            )
#define AAS_UNIT_SIZE_SMU                          (     2        *      MB         )
#define AAS_UNIT_SIZE_MPCL                         (  1        *      MB            )
#define AAS_UNIT_SIZE_UPPER_ING_TAI                (   1        *      MB           )
#define AAS_UNIT_SIZE_PREQ                         (  4        *      MB            )
#define AAS_UNIT_SIZE_EPCL                         (  2        *      MB            )
#define AAS_UNIT_SIZE_EMX                          (     512        *      KB       )
#define AAS_UNIT_SIZE_EM_ILM0                      (     512        *      KB       )
#define AAS_UNIT_SIZE_EQ                           (    24        *      MB         )
#define AAS_UNIT_SIZE_DDR3_PHY                     (  32        *      MB           )
#define AAS_UNIT_SIZE_DDR3                         (  2        *      MB            )
#define AAS_UNIT_SIZE_DDR3_IFBIST                  (     1        *      MB         )
#define AAS_UNIT_SIZE_DDR3_MCTL                    (   1        *      MB           )
#define AAS_UNIT_SIZE_HA                           (    124        *      MB        )
#define AAS_UNIT_SIZE_HA_TAI                       (    1        *      MB          )
#define AAS_UNIT_SIZE_FDB                          (     34        *      MB        )
#define AAS_UNIT_SIZE_BMA                          (     2        *      MB         )
#define AAS_UNIT_SIZE_HBU                          (     1        *      MB         )
#define AAS_UNIT_SIZE_EREP                         (  1        *      MB            )
#define AAS_UNIT_SIZE_GPW0                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GPW1                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GPW2                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GPW3                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GPW4                         (  256        *      KB          )
#define AAS_UNIT_SIZE_WA0                          (     256        *      KB       )
#define AAS_UNIT_SIZE_WA1                          (     256        *      KB       )
#define AAS_UNIT_SIZE_PB_COUNTER                   (    256        *      KB        )
#define AAS_UNIT_SIZE_GCR0                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GCR1                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GCR2                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GCR3                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GCR4                         (  256        *      KB          )
#define AAS_UNIT_SIZE_NPM0                         (  256        *      KB          )
#define AAS_UNIT_SIZE_NPM1                         (  256        *      KB          )
#define AAS_UNIT_SIZE_SMB0                         (  256        *      KB          )
#define AAS_UNIT_SIZE_SMB1                         (  256        *      KB          )
#define AAS_UNIT_SIZE_SMB2                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GPR0                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GPR1                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GPR2                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GPR3                         (  256        *      KB          )
#define AAS_UNIT_SIZE_GPR4                         (  256        *      KB          )
#define AAS_UNIT_SIZE_PSI                          (     4        *      MB         )
#define AAS_UNIT_SIZE_PDX_PDITX                    (   128        *      KB         )
#define AAS_UNIT_SIZE_PDX_PACKER                   (    64        *      KB         )
#define AAS_UNIT_SIZE_PFCC                         (  64        *      KB           )
#define AAS_UNIT_SIZE_DFX_SERVER                   (    1        *      MB          )
#define AAS_UNIT_SIZE_EXT_IPs_SERVER               (    1        *      MB          )
#define AAS_UNIT_SIZE_GOP1_4P_MASTER_S_TAI         (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP1_4P_MASTER_TAI_0         (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP1_4P_MASTER_TAI_1         (  256        *      KB          )
#define AAS_UNIT_SIZE_GOP1_4P_MASTER_TAI_2         (  256        *      KB          )
#define AAS_UNIT_SIZE_MG0                          (    1        *      MB          )
#define AAS_UNIT_SIZE_MG1                          (    1        *      MB          )
#define AAS_UNIT_SIZE_MIB                          (    512      *      KB          )
#define AAS_UNIT_SIZE_WM_IPC                       (    1        *      KB          )

/*
https://marvell.sharepoint.com/:x:/r/sites/switching/chipdesign/seahawk/_layouts/15/Doc.aspx?sourcedoc=%7B0A3F27E1-B78D-44CE-BF25-DAE20BF40821%7D&file=Seahawk_Address_Space_4.9.xlsm&wdLOR=cC3487BFC-6A2B-4D76-BDC2-98111972E346&action=default&mobileredirect=true
*/

/*Unit                                          Start Addr   Size        Size Unit        */
#define AAS_ADDR_CNM_PCIe_MAC_DBI               0x7C000000   /*    4        MB  */
#define AAS_ADDR_CNM_PCIe_PRAM                  0x7C400000   /*  128        KB  */
#define AAS_ADDR_CNM_PCIe_SDW_RFU               0x7C420000   /*  128        KB  */
#define AAS_ADDR_CNM_PCIe_COMPHY                0x7C440000   /*  256        KB  */
#define AAS_ADDR_CNM_PCIe_RFU                   0x7C480000   /*   64        KB  */
#define AAS_ADDR_CNM_IDE_hconfig                0x7C490000   /*   64        KB  */
#define AAS_ADDR_CNM_IDE_AES_hconfig            0x7C4A0000   /*   64        KB  */
#define AAS_ADDR_CNM_IUNIT                      0x7C4B0000   /*   64        KB  */
#define AAS_ADDR_CNM_CNM_Interrupt              0x7C4C0000   /* 64        KB    */
#define AAS_ADDR_CNM_CnM_RFU                    0x7C4D0000   /* 64        KB    */
#define AAS_ADDR_CNM_MPP_RFU                    0x7C4E0000   /* 64        KB    */
#define AAS_ADDR_CNM_AAC_0                      0x7C4F0000   /* 64        KB    */
#define AAS_ADDR_CNM_AAC_1                      0x7C500000   /* 64        KB    */
#define AAS_ADDR_CNM_Address_Decoder_iNIC       0x7C510000   /* 4        KB     */
#define AAS_ADDR_CNM_Address_Decoder_PCIe       0x7C511000   /* 4        KB     */
#define AAS_ADDR_CNM_Address_Decoder_iunit      0x7C512000   /*  4        KB    */
#define AAS_ADDR_CNM_Address_Decoder_GDMA0      0x7C513000   /*  4        KB    */
#define AAS_ADDR_CNM_Address_Decoder_GDMA1      0x7C514000   /*  4        KB    */
#define AAS_ADDR_CNM_Address_Decoder_GDMA2      0x7C515000   /*  4        KB    */
#define AAS_ADDR_CNM_Address_Decoder_GDMA3      0x7C516000   /*  4        KB    */
#define AAS_ADDR_CNM_Address_Decoder_EHSM_DMA   0x7C517000   /*  4        KB    */
#define AAS_ADDR_CNM_Address_Decoder_AAC_0      0x7C519000   /*  4        KB    */
#define AAS_ADDR_CNM_Address_Decoder_AAC_1      0x7C51A000   /*  4        KB    */
#define AAS_ADDR_CNM_Address_Decoder_AMB        0x7C51B000   /* 4        KB     */
#define AAS_ADDR_CNM_XSMI_0                     0x7C520000   /* 64        KB    */
#define AAS_ADDR_CNM_XSMI_1                     0x7C530000   /* 64        KB    */
#define AAS_ADDR_CNM_XSMI_2                     0x7C540000   /* 64        KB    */
#define AAS_ADDR_CNM_SMI_0                      0x7C560000   /* 64        KB    */
#define AAS_ADDR_CNM_SMI_1                      0x7C570000   /* 64        KB    */
#define AAS_ADDR_CNM_SMI_2                      0x7C580000   /* 64        KB    */
#define AAS_ADDR_CNM_QSPI_0                     0x7C5A0000   /* 32        KB    */
#define AAS_ADDR_CNM_QSPI_1                     0x7C5A8000   /* 32        KB    */
#define AAS_ADDR_CNM_QSPI_2                     0x7C5B0000   /* 32        KB    */
#define AAS_ADDR_CNM_GDMA_0                     0x7C5C0000   /* 32        KB    */
#define AAS_ADDR_CNM_GDMA_1                     0x7C5C8000   /* 32        KB    */
#define AAS_ADDR_CNM_GDMA_2                     0x7C5D0000   /* 32        KB    */
#define AAS_ADDR_CNM_GDMA_3                     0x7C5D8000   /* 32        KB    */
#define AAS_ADDR_CNM_GDMA_Dispatcher            0x7C5F0000   /* 16        KB    */
#define AAS_ADDR_CNM_RUNIT                      0x7C610000   /* 64        KB    */
#define AAS_ADDR_CNM_MSIX_RFU                   0x7C620000   /* 32        KB    */
#define AAS_ADDR_CNM_PRAM_ROM                   0x7C640000   /*     128        KB*/
#define AAS_ADDR_CNM_eHSM_ommand                0x7C660000   /*    16        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AXIM_cm7_0 0x7C710000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AXIM_cm7_1 0x7C711000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AXIM_cm7_2 0x7C712000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AXIM_cm7_3 0x7C713000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AXIM_cm7_4 0x7C714000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AXIM_cm7_5 0x7C715000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AXIM_cm7_6 0x7C716000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AHBP_cm7_0 0x7C717000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AHBP_cm7_1 0x7C718000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AHBP_cm7_2 0x7C719000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AHBP_cm7_3 0x7C71A000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AHBP_cm7_4 0x7C71B000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AHBP_cm7_5 0x7C71C000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_Decoder_AHBP_cm7_6 0x7C71D000   /*     4        KB */

#define AAS_ADDR_CNM_Address_WD_RFU_AXIM_CM7_0  0x7C727000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AXIM_CM7_1  0x7C728000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AXIM_CM7_2  0x7C729000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AXIM_CM7_3  0x7C72A000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AXIM_CM7_4  0x7C72B000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AXIM_CM7_5  0x7C72C000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AXIM_CM7_6  0x7C72D000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AHBP_CM7_0  0x7C72E000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AHBP_CM7_1  0x7C72F000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AHBP_CM7_2  0x7C730000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AHBP_CM7_3  0x7C731000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AHBP_CM7_4  0x7C732000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AHBP_CM7_5  0x7C733000   /*     4        KB  */
#define AAS_ADDR_CNM_Address_WD_RFU_AHBP_CM7_6  0x7C734000   /*     4        KB  */

#define AAS_ADDR_CNM_CM7_RFU                    0x7C760000   /*     128        KB*/
#define AAS_ADDR_CNM_CM7_0_DTCM                 0x7DA00000   /*    32        KB  */
#define AAS_ADDR_CNM_CM7_1_DTCM                 0x7DA10000   /*    32        KB  */
#define AAS_ADDR_CNM_CM7_2_DTCM                 0x7DA20000   /*    32        KB  */
#define AAS_ADDR_CNM_CM7_3_DTCM                 0x7DA30000   /*    32        KB  */
#define AAS_ADDR_CNM_CM7_4_DTCM                 0x7DA40000   /*    32        KB  */
#define AAS_ADDR_CNM_CM7_5_DTCM                 0x7DA50000   /*    32        KB  */
#define AAS_ADDR_CNM_CM7_6_DTCM                 0x7DA60000   /*    32        KB  */
#define AAS_ADDR_CNM_ROM                        0x7DA80000   /*     128        KB*/
#define AAS_ADDR_CNM_Instruction_SRAM           0x7DAA0000   /*    32        KB  */
#define AAS_ADDR_CNM_Instruction_SRAM_Spare     0x7DAA8000   /*    32        KB  */
#define AAS_ADDR_CNM_SRAM0                      0x7DC00000   /*     2        MB  */
#define AAS_ADDR_CNM_SRAM1                      0x7DE00000   /*     2        MB  */

/*iNIC Address Space*/
#define AAS_ADDR_CNM_pcie_axi_cnfg_s            0x7C000000   /*     4        MB  */
#define AAS_ADDR_CNM_iunit_s                    0x7C4B0000   /*     64        KB */
#define AAS_ADDR_CNM_cnm_rfu_s                  0x7C4C0000   /*     192        KB*/
#define AAS_ADDR_CNM_aac_0_s                    0x7C4F0000   /*     64        KB */
#define AAS_ADDR_CNM_aac_1_s                    0x7C500000   /*     64        KB */
#define AAS_ADDR_CNM_adec_inic_s                0x7C510000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_pcie_s                0x7C511000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_iunit_s               0x7C512000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_gdma_0_s              0x7C513000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_gdma_1_s              0x7C514000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_gdma_2_s              0x7C515000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_gdma_3_s              0x7C516000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_ehsm_dma_s            0x7C517000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_aac_0_s               0x7C519000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_aac_1_s               0x7C51A000   /*     4        KB  */
#define AAS_ADDR_CNM_adec_amb_s                 0x7C51B000   /*     4        KB  */
#define AAS_ADDR_CNM_xsmi_s                     0x7C520000   /*     256        KB*/
#define AAS_ADDR_CNM_smi_s                      0x7C560000   /*     256        KB*/
#define AAS_ADDR_CNM_qspi_0_s                   0x7C5A0000   /*      32        KB*/
#define AAS_ADDR_CNM_qspi_1_s                   0x7C5A8000   /*      32        KB*/
#define AAS_ADDR_CNM_qspi_2_s                   0x7C5B0000   /*      32        KB*/
#define AAS_ADDR_CNM_gdma_0_s                   0x7C5C0000   /*      32        KB*/
#define AAS_ADDR_CNM_gdma_1_s                   0x7C5C8000   /*      32        KB*/
#define AAS_ADDR_CNM_gdma_2_s                   0x7C5D0000   /*      32        KB*/
#define AAS_ADDR_CNM_gdma_3_s                   0x7C5D8000   /*      32        KB*/
#define AAS_ADDR_CNM_gdma_dispatch_rfu_s        0x7C5F0000   /*      16        KB*/
#define AAS_ADDR_CNM_amb_s                      0x7C600000   /*     128        KB*/
#define AAS_ADDR_CNM_msix_client_s              0x7C620000   /*     32        KB */
#define AAS_ADDR_CNM_pram_rom_s                 0x7C640000   /*     128        KB*/
#define AAS_ADDR_CNM_ehsm_cmd_s                 0x7C660000   /*     4        KB  */
#define AAS_ADDR_CNM_wd_rfu_fc_slv_s            0x7C661000   /*     4        KB  */
#define AAS_ADDR_CNM_wd_rfu_pcie_mstr_s         0x7C662000   /*     4        KB  */
#define AAS_ADDR_CNM_wd_rfu_pcie_slv_s          0x7C663000   /*     4        KB  */
#define AAS_ADDR_CNM_wd_rfu_pcie_axi_conf_s     0x7C664000   /*     4        KB  */
#define AAS_ADDR_CNM_s_inic_s                   0x7C700000   /*       1        MB*/
#define AAS_ADDR_CNM_enic_s                     0x7C800000   /*     24        MB */


/*eNIC address space - after address decoders +              DAM*/
/*S_eNIC address space for all ports*/
#define AAS_ADDR_CNM_cm7_0_s                    0x7DA00000   /*     32        KB */
#define AAS_ADDR_CNM_cm7_1_s                    0x7DA10000   /*     32        KB */
#define AAS_ADDR_CNM_cm7_2_s                    0x7DA20000   /*     32        KB */
#define AAS_ADDR_CNM_cm7_3_s                    0x7DA30000   /*     32        KB */
#define AAS_ADDR_CNM_cm7_4_s                    0x7DA40000   /*     32        KB */
#define AAS_ADDR_CNM_cm7_5_s                    0x7DA50000   /*     32        KB */
#define AAS_ADDR_CNM_cm7_6_s                    0x7DA60000   /*     32        KB */
#define AAS_ADDR_CNM_rom_s                      0x7DA80000   /*    128        KB */
#define AAS_ADDR_CNM_inst_sram_s                0x7DAA0000   /*     32        KB */
#define AAS_ADDR_CNM_sram_0_s                   0x7DC00000   /*      2        MB */
#define AAS_ADDR_CNM_sram_1_s                   0x7DE00000   /*      2        MB */
/*S_iNIC address space all ports*/
#define AAS_ADDR_CNM_cm7_rfu_s                  0x7C760000   /*     64        KB */
#define AAS_ADDR_CNM_adec_cm7_axim_0_s          0x7C710000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_axim_1_s          0x7C711000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_axim_2_s          0x7C712000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_axim_3_s          0x7C713000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_axim_4_s          0x7C714000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_axim_5_s          0x7C715000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_axim_6_s          0x7C716000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_ahbp_0_s          0x7C717000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_ahbp_1_s          0x7C718000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_ahbp_2_s          0x7C719000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_ahbp_3_s          0x7C71A000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_ahbp_4_s          0x7C71B000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_ahbp_5_s          0x7C71C000   /*      4        KB */
#define AAS_ADDR_CNM_adec_cm7_ahbp_6_s          0x7C71D000   /*      4        KB */

#define AAS_UNIT_SIZE_CNM_PCIe_MAC_DBI              (    4    *    MB   )
#define AAS_UNIT_SIZE_CNM_PCIe_PRAM                 (  128    *    KB   )
#define AAS_UNIT_SIZE_CNM_PCIe_SDW_RFU              (  128    *    KB   )
#define AAS_UNIT_SIZE_CNM_PCIe_COMPHY               (  256    *    KB   )
#define AAS_UNIT_SIZE_CNM_PCIe_RFU                  (   64    *    KB   )
#define AAS_UNIT_SIZE_CNM_IDE_hconfig               (   64    *    KB   )
#define AAS_UNIT_SIZE_CNM_IDE_AES_hconfig           (   64    *    KB   )
#define AAS_UNIT_SIZE_CNM_IUNIT                     (   64    *    KB   )
#define AAS_UNIT_SIZE_CNM_CNM_Interrupt             ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_CnM_RFU                   ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_MPP_RFU                   ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_AAC_0                     ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_AAC_1                     ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_iNIC      ( 4       * KB      )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_PCIe      ( 4       * KB      )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_iunit     (  4      *  KB     )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_GDMA0     (  4      *  KB     )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_GDMA1     (  4      *  KB     )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_GDMA2     (  4      *  KB     )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_GDMA3     (  4      *  KB     )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_EHSM_DMA  (  4      *  KB     )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AAC_0     (  4      *  KB     )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AAC_1     (  4      *  KB     )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AMB       ( 4       * KB      )
#define AAS_UNIT_SIZE_CNM_XSMI_0                    ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_XSMI_1                    ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_XSMI_2                    ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_SMI_0                     ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_SMI_1                     ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_SMI_2                     ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_QSPI_0                    ( 32      *  KB     )
#define AAS_UNIT_SIZE_CNM_QSPI_1                    ( 32      *  KB     )
#define AAS_UNIT_SIZE_CNM_QSPI_2                    ( 32      *  KB     )
#define AAS_UNIT_SIZE_CNM_GDMA_0                    ( 32      *  KB     )
#define AAS_UNIT_SIZE_CNM_GDMA_1                    ( 32      *  KB     )
#define AAS_UNIT_SIZE_CNM_GDMA_2                    ( 32      *  KB     )
#define AAS_UNIT_SIZE_CNM_GDMA_3                    ( 32      *  KB     )
#define AAS_UNIT_SIZE_CNM_GDMA_Dispatcher           ( 16      *  KB     )
#define AAS_UNIT_SIZE_CNM_RUNIT                     ( 64      *  KB     )
#define AAS_UNIT_SIZE_CNM_MSIX_RFU                  ( 32      *  KB     )
#define AAS_UNIT_SIZE_CNM_PRAM_ROM                  (   128   *     KB  )
#define AAS_UNIT_SIZE_CNM_eHSM_ommand               (    16   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AXIM_cm7_0 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AXIM_cm7_1 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AXIM_cm7_2 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AXIM_cm7_3 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AXIM_cm7_4 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AXIM_cm7_5 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AXIM_cm7_6 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AHBP_cm7_0 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AHBP_cm7_1 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AHBP_cm7_2 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AHBP_cm7_3 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AHBP_cm7_4 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AHBP_cm7_5 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_Decoder_AHBP_cm7_6 (    4   *     KB  )

#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AXIM_CM7_0  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AXIM_CM7_1  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AXIM_CM7_2  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AXIM_CM7_3  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AXIM_CM7_4  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AXIM_CM7_5  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AXIM_CM7_6  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AHBP_CM7_0  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AHBP_CM7_1  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AHBP_CM7_2  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AHBP_CM7_3  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AHBP_CM7_4  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AHBP_CM7_5  (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_Address_WD_RFU_AHBP_CM7_6  (    4   *     KB  )

#define AAS_UNIT_SIZE_CNM_CM7_RFU                    (    128 *       KB)
#define AAS_UNIT_SIZE_CNM_CM7_0_DTCM                 (   32   *     KB  )
#define AAS_UNIT_SIZE_CNM_CM7_1_DTCM                 (   32   *     KB  )
#define AAS_UNIT_SIZE_CNM_CM7_2_DTCM                 (   32   *     KB  )
#define AAS_UNIT_SIZE_CNM_CM7_3_DTCM                 (   32   *     KB  )
#define AAS_UNIT_SIZE_CNM_CM7_4_DTCM                 (   32   *     KB  )
#define AAS_UNIT_SIZE_CNM_CM7_5_DTCM                 (   32   *     KB  )
#define AAS_UNIT_SIZE_CNM_CM7_6_DTCM                 (   32   *     KB  )
#define AAS_UNIT_SIZE_CNM_ROM                        (    128 *       KB)
#define AAS_UNIT_SIZE_CNM_Instruction_SRAM           (   32   *     KB  )
#define AAS_UNIT_SIZE_CNM_Instruction_SRAM_Spare     (   32   *     KB  )
#define AAS_UNIT_SIZE_CNM_SRAM0                      (    2   *     MB  )
#define AAS_UNIT_SIZE_CNM_SRAM1                      (    2   *     MB  )

/*iNIC Address Space*/
#define AAS_UNIT_SIZE_CNM_pcie_axi_cnfg_s            (    4   *     MB  )
#define AAS_UNIT_SIZE_CNM_iunit_s                    (    64  *      KB )
#define AAS_UNIT_SIZE_CNM_cnm_rfu_s                  (    192 *       KB)
#define AAS_UNIT_SIZE_CNM_aac_0_s                    (    64  *      KB )
#define AAS_UNIT_SIZE_CNM_aac_1_s                    (    64  *      KB )
#define AAS_UNIT_SIZE_CNM_adec_inic_s                (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_pcie_s                (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_iunit_s               (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_gdma_0_s              (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_gdma_1_s              (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_gdma_2_s              (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_gdma_3_s              (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_ehsm_dma_s            (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_aac_0_s               (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_aac_1_s               (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_adec_amb_s                 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_xsmi_s                     (    256 *       KB)
#define AAS_UNIT_SIZE_CNM_smi_s                      (    256 *       KB)
#define AAS_UNIT_SIZE_CNM_qspi_0_s                   (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_qspi_1_s                   (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_qspi_2_s                   (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_gdma_0_s                   (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_gdma_1_s                   (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_gdma_2_s                   (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_gdma_3_s                   (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_gdma_dispatch_rfu_s        (     16 *       KB)
#define AAS_UNIT_SIZE_CNM_amb_s                      (    128 *       KB)
#define AAS_UNIT_SIZE_CNM_msix_client_s              (    32  *      KB )
#define AAS_UNIT_SIZE_CNM_pram_rom_s                 (    128 *       KB)
#define AAS_UNIT_SIZE_CNM_ehsm_cmd_s                 (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_wd_rfu_fc_slv_s            (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_wd_rfu_pcie_mstr_s         (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_wd_rfu_pcie_slv_s          (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_wd_rfu_pcie_axi_conf_s     (    4   *     KB  )
#define AAS_UNIT_SIZE_CNM_s_inic_s                   (      1 *       MB)
#define AAS_UNIT_SIZE_CNM_enic_s                     (    24  *      MB )


/*eNIC address space - after address decoders +              DAM*/
/*S_eNIC address space for all ports*/
#define AAS_UNIT_SIZE_CNM_cm7_0_s                    (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_cm7_1_s                    (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_cm7_2_s                    (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_cm7_3_s                    (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_cm7_4_s                    (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_cm7_5_s                    (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_cm7_6_s                    (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_rom_s                      (    128 *       KB)
#define AAS_UNIT_SIZE_CNM_inst_sram_s                (     32 *       KB)
#define AAS_UNIT_SIZE_CNM_sram_0_s                   (      2 *       MB)
#define AAS_UNIT_SIZE_CNM_sram_1_s                   (      2 *       MB)
/*S_iNIC address space all ports*/
#define AAS_UNIT_SIZE_CNM_cm7_rfu_s                  (     64 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_axim_0_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_axim_1_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_axim_2_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_axim_3_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_axim_4_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_axim_5_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_axim_6_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_ahbp_0_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_ahbp_1_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_ahbp_2_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_ahbp_3_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_ahbp_4_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_ahbp_5_s          (      4 *       KB)
#define AAS_UNIT_SIZE_CNM_adec_cm7_ahbp_6_s          (      4 *       KB)

#define AAS_ADDR_SERVER         AAS_ADDR_DFX_SERVER
#define AAS_UNIT_SIZE_SERVER    AAS_UNIT_SIZE_DFX_SERVER

#define AAS_ADDR_CNM_Address_Decoder       AAS_ADDR_CNM_Address_Decoder_iNIC
#define AAS_UNIT_SIZE_Address_Decoder      (AAS_UNIT_SIZE_Address_Decoder_iNIC * 11)/*11 mini-units of address decoders*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __smemAasAddrSpace_CC_plus */

