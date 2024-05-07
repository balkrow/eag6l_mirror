/*
 *
 * ppa_fw_threads_defs.h
 *
 * PPA fw threads definitions
 *
 */
#ifndef __ppa_fw_threads_defs_h
#define __ppa_fw_threads_defs_h


#ifdef ASIC_SIMULATION
    #ifndef CPU_BE
        /* get the definitions dedicated for little endian CPU */
        #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/ppa_fw_common_strc_defs_little_endian.h"
        #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/Default/ppa_fw_threads_strc_defs_little_endian.h"
    #else /*!CPU_BE*/
        /* remove next line if actually running ASIC_SIMULATION in 'big endian CPU' ... not likely ! */
        #error  "ASIC_SIMULATION : 'CPU_BE' defined ???"
    #endif /*!CPU_BE*/
#else /*!ASIC_SIMULATION*/
    #include "ppa_fw_common_strc_defs.h"
    #include "ppa_fw_threads_strc_defs.h"
#endif /*!ASIC_SIMULATION*/


/**************************************************************************
 * Function prototypes
 **************************************************************************/
void PPA_FW(THR0_DoNothing)();
void PPA_FW(THR1_SRv6_End_Node)();
void PPA_FW(THR2_SRv6_Source_Node_1_segment)();
void PPA_FW(THR3_SRv6_Source_Node_First_Pass_2_3_segments)();
void PPA_FW(THR4_SRv6_Source_Node_Second_Pass_3_segments)();
void PPA_FW(THR5_SRv6_Source_Node_Second_Pass_2_segments)();
void PPA_FW(THR11_VXLAN_GPB_SourceGroupPolicyID)();
void PPA_FW(THR12_MPLS_SR_NO_EL)();
void PPA_FW(THR13_MPLS_SR_ONE_EL)();
void PPA_FW(THR14_MPLS_SR_TWO_EL)();
void PPA_FW(THR15_MPLS_SR_THREE_EL)();
void PPA_FW(THR16_SGT_NetAddMSB)();
void PPA_FW(THR17_SGT_NetFix)();
void PPA_FW(THR18_SGT_NetRemove)();
void PPA_FW(THR19_SGT_eDSAFix)();
void PPA_FW(THR20_SGT_eDSARemove)();
void PPA_FW(THR21_SGT_GBPFixIPv4)();
void PPA_FW(THR22_SGT_GBPFixIPv6)();
void PPA_FW(THR23_SGT_GBPRemoveIPv4)();
void PPA_FW(THR24_SGT_GBPRemoveIPv6)();
void PPA_FW(THR25_PTP_Phy_1_Step)();
void PPA_FW(THR53_SRV6_End_Node_GSID_COC32)();
void PPA_FW(THR58_IPSEC_PTP)();
void PPA_FW(THR59_SLS_Test)();
void PPA_FW(THR60_DropAllTraffic)();
void PPA_FW(THR64_Erspan_TypeII_SameDevMirroring_Ipv4)();
void PPA_FW(THR65_Erspan_TypeII_SameDevMirroring_Ipv6)();
void PPA_FW(THR68_Erspan_TypeI_Ipv4)();
void PPA_FW(THR69_Erspan_TypeI_Ipv6)();
void PPA_FW(THR70_radio_header)();
void PPA_FW(THR81_Erspan_TypeIII_SameDevMirroring_IPv4)();
void PPA_FW(THR82_Erspan_TypeIII_SameDevMirroring_IPv6_FirstPass)();
void PPA_FW(THR83_Erspan_TypeIII_SameDevMirroring_IPv6_SecondPass)();
void PPA_FW(THR84_Erspan_TypeIII_OrigDevMirroring)();
void PPA_FW(THR85_Erspan_TypeIII_TrgDevMirroring_IPv4_Tagged)();
void PPA_FW(THR86_Erspan_TypeIII_TrgDevMirroring_IPv4_Untagged)();
void PPA_FW(THR87_Erspan_TypeIII_TrgDevMirroring_IPv6_Tagged)();
void PPA_FW(THR88_Erspan_TypeIII_TrgDevMirroring_IPv6_Untagged)();
void PPA_FW(THR93_MPLS_LSR_In_Stacking_System_Thread)();

#if RECOVER_FROM_EXCEPTION
INLINE void ac5pRecoveFromException();
#endif
/* Required for profile measurements */
void       _exit();

#endif /* __ppa_fw_threads_defs_h */
