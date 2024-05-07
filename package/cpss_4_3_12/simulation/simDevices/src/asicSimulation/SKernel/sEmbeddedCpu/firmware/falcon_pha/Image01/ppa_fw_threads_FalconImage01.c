/*------------------------------------------------------------
(C) Copyright Marvell International Ltd.
All Rights Reserved
------------------------------------------------------------*/

/*
 * ppa_fw_threads_FalconImage01.c
 *
 *  PPA fw threads functions
 *
 */

#ifdef ASIC_SIMULATION
    #ifdef _VISUALC
        #pragma warning(disable: 4214) /* nonstandard extension used : bit field types other than int */
        #pragma warning(disable: 4244) /* conversion from 'uint32_t' to 'uint16_t', possible loss of data */
        #pragma warning(disable: 4146) /* unary minus operator applied to unsigned type, result still unsigned */

    #endif /*_VISUALC*/
#endif /*ASIC_SIMULATION*/

/* ==========================
  Include headers section
  =========================== */
#ifdef ASIC_SIMULATION
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/Image01/ppa_fw_image_info.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_base_types.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/Image01/ppa_fw_accelerator_commands.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/Image01/ppa_fw_threads_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_exception.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/wm_asic_sim_defs.h"
static SKERNEL_DEVICE_OBJECT * devObjPtr;
/* empty implementation for simulation */
#define packet_swap_sequence_reset_stack()
#else
#include "ppa_fw_image_info.h"
#include "ppa_fw_base_types.h"
#include "ppa_fw_defs.h"
#include "ppa_fw_accelerator_commands.h"
#include "ppa_fw_threads_defs.h"
#include "ppa_fw_ppn2mg.h"
#include "ppa_fw_exception.h"
#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#include <xtensa/sim.h>


/********************************************************************************************************************//**
 * funcname        pragma_no_reorder
 * inparam         None
 * return          None
 * description     Activate compiler pragma no_reorder command.
 *                 Telling the compiler not to change order of instructions at this point.
 ************************************************************************************************************************/
INLINE void pragma_no_reorder(){
            #pragma no_reorder
    }

/********************************************************************************************************************//**
 * funcname        pragma_frequency_hint_never
 * inparam         None
 * return          None
 * description     Activate compiler pragma frequency_hint command.
 *                 Telling the compiler that this code is hardly executed
 ************************************************************************************************************************/
INLINE void pragma_frequency_hint_never(){
            #pragma frequency_hint NEVER
    }

/********************************************************************************************************************//**
 * funcname        pragma_frequency_hint_frequent
 * inparam         None
 * return          None
 * description     Activate compiler pragma frequency_hint command.
 *                 Telling the compiler that this code is most likely to be executed
 ************************************************************************************************************************/
INLINE void pragma_frequency_hint_frequent(){
            #pragma frequency_hint FREQUENT
    }

/********************************************************************************************************************//**
 * funcname        first_packet_swap_sequence
 * inparam         None
 * return          None
 * description     Activate the below sequence: send the processing packet, gets new one, load the new PC thread and jump to it
 ************************************************************************************************************************/
INLINE void packet_swap_sequence()
{
    uint32_t   ppa_deq  ;

    /* step 1 - swap instruction, activate packet Out & packet In machines (Enqueue packet) */
    PPA_FW_SP_WRITE(PKT_SWAP_INST_ADDR,0);
    /* step 2 - flush store before activating the blocking load */
    #pragma flush_memory
    /* step 3 - initialize a15 for pc blocking load.  load start of new packet scenario PC. */
    ppa_deq=PPA_FW_SP_READ(PKT_LOAD_PC_INST_ADDR);
    /* steps 4  - jump to start of scenario - start execution */
    asm volatile("JX %0" :: "a"(ppa_deq));
}

/********************************************************************************************************************//**
 * funcname        packet_swap_sequence_reset_stack
 * inparam         None
 * return          None
 * description     Activate the below sequence: send the processing packet, gets new one, load the new PC thread and jump to it
 *                 Function changes stack pointer to be like before call of the caller function
 ************************************************************************************************************************/
INLINE void packet_swap_sequence_reset_stack()
{
    uint32_t   ppa_deq  ;

    /* step 1 - swap instruction, activate packet Out & packet In machines (Enqueue packet) */
    PPA_FW_SP_WRITE(PKT_SWAP_INST_ADDR,0);
    /* step 2 - flush store before activating the blocking load */
    #pragma flush_memory
    /* step 3 - initialize a15 for pc blocking load.  load start of new packet scenario PC. */
    ppa_deq=PPA_FW_SP_READ(PKT_LOAD_PC_INST_ADDR);
    /* steps 4  - restore stack pointer and jump to start of scenario - start execution */
    asm volatile("movi	sp, __stack - 0x10 \n\t"
    			 "JX %0" :: "a"(ppa_deq));

}

#endif /*!ASIC_SIMULATION*/


/* ==========================
  Threads section
  =========================== */

#ifdef ASIC_SIMULATION
/********************************************************************************************************************//**
* funcname        FalconImage01_get_fw_version
* inparam         fwImageId    PHA firmware image ID
* return          None
* description     Get PHA fw version and save it into SP memory (used in WM)
************************************************************************************************************************/
void FalconImage01_get_fw_version(GT_U32 fwImageId)
{
    /* Get falcon fw version and save it into fixed address in SP memory */
    falcon_get_fw_version(fwImageId, ((VER_YEAR<<24) | (VER_MONTH<<16) | (VER_IN_MONTH<<8) | VER_DEBUG ));
}
#endif /*ASIC_SIMULATION*/



/********************************************************************************************************************//**
 * funcname        THR0_DoNothing
 * inparam         None
 * return          None
 * description     No modifications to the packet and no additions or remove of any tags or headers.
 *                 Packet is out as it is in.
 *                 Note:
 *                 - this thread is mapped to a fix address and should not be changed.
 *                 - allows to execute the thread while fw version is being upgraded.
 *                 - do not use accelerator commands since it will change thread's code
 *                   since in each build accelerator commands might be in different location
 ************************************************************************************************************************/
__attribute__ ((section(".iram0.text")))
__attribute__ ((aligned (16)))
void PPA_FW(THR0_DoNothing)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(0);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR1_SRv6_End_Node
 * inparam      None
 * return       None
 * description  Covers the IPv6 Segment Routing (SR) End Node use case
 *              cfg template:    not in use(16B)
 *              targetPortTable: not in use(4B)
 *              Incoming packet: Expansion space(32B),IPv6(40B),IPv6 Ext. hdrs(n*8B),SRH(8B),Segment0(16B),Segment1(16B),Segment2(16B)
 *              Output packet:   Expansion space(32B),IPv6(40B),IPv6 Ext. hdrs(n*8B),SRH(8B),Segment0(16B),Segment1(16B),Segment2(16B)
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 32B Expansion space + 128B packet
 *              Use Desc<SR EH Offset> to find the location of the fields.
 *              Decrement and update the SR Header<Segments Left> field
 *              Update Ipv6 destination IP with the selected segment list IP.
 *              Do it only if packet size is still under the maximum allowed size of 128 bytes otherwise drop.
 *              NOTE: Since required Segment list can reach to location beyond 127B need to set offset register
 *                    in order for the accelerator command to work correctly
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR1_SRv6_End_Node)() {
    /*xt_iss_client_command("profile","enable");            */
    /*xt_iss_client_command("profile","disable"); _exit(0); */

    uint8_t segments_left,ipv6_eh_size,segments_left_ofst;
    uint32_t selected_segment_list_ofst;

    /* Get pointer to descriptor */
    struct ppa_in_desc*  thr1_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Indicates, in 8 bytes units, where the Segment Routing Extension Header is located relative to end of IPv6 Base header.
       Basically it tells us if there are IPv6 Extension headers or not and their sizes. 0xn - SR routing header is 8*n bytes after the base header */
    __LOG_FW(("Indicates, in 8 bytes units, where the Segment Routing Extension Header is located relative to end of IPv6 Base header."));
    ipv6_eh_size = thr1_desc_ptr->phal2ppa.sr_eh_offset*EIGHT_BYTES;
    __LOG_PARAM_FW(ipv6_eh_size);

    /* Calculate offset to SRH<Segment left> field */
    __LOG_FW(("Calculate offset to SRH<Segment left> field"));
    segments_left_ofst = IPV6_SR_EXPANSION_SPACE_SIZE + IPV6_BASIC_HEADER_SIZE + ipv6_eh_size + SEGMENT_LEFT_OFST_IN_SR_HDR ;
    __LOG_PARAM_FW(segments_left_ofst);

    /* Check that SRH<Segment left> is in the range of PPA buffer (160B = Exp space:32B + input packet:128B) otherwise drop */
    __LOG_FW(("Check that SRH<Segment left> is in the range of PPA buffer (160B = Exp space:32B + input packet:128B) otherwise drop"));
    if ( segments_left_ofst < (IPV6_SR_EXPANSION_SPACE_SIZE+IPv6_SR_PKT_MAX_LENGTH) )
    {
		/* Read SRH<Segment left> index. Decrement by 1 to point to the right Segment list */
        __LOG_FW(("Read SRH<Segment left> index. Decrement by 1 to point to the right Segment list"));
        segments_left = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + segments_left_ofst) - 1;
        __LOG_PARAM_FW(segments_left);

		/* Calculates the offset to the start of the required Segment list */
        __LOG_FW(("Calculates the offset to the start of the required Segment list"));
        selected_segment_list_ofst = IPV6_SR_EXPANSION_SPACE_SIZE + IPV6_BASIC_HEADER_SIZE + ipv6_eh_size + SR_BASIC_HEADER_SIZE + (SEGMENT_LIST_SIZE*segments_left);
        __LOG_PARAM_FW(selected_segment_list_ofst);

        /* Check Segment list is in the range of PPA buffer. Since it's 16B long max valid offset is 144B (160-16) otherwise drop */
        __LOG_FW(("Check Segment list is in the range of PPA buffer. Since it's 16B long max valid offset is 144B (160-16) otherwise drop"));
		if (selected_segment_list_ofst <= (IPV6_SR_EXPANSION_SPACE_SIZE+IPv6_SR_PKT_MAX_LENGTH-SEGMENT_LIST_SIZE))
		{
            /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
               Since in this use case might be a case in which Segment list location exceeds 127 bytes need to use PPN offset register.
               By default this register is used all the time and the default value is zero.
               This time we will set it to point to IPv6 which means that the pkt src and dst will be relative to 32B
               HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
            __LOG_FW(("Set offset register to point to IPv6"));
            PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 32);
            __PRAGMA_NO_REORDER

			/* Update IPv6 destination address from the segment list */
			__LOG_FW(("Update IPv6 destination address from the segment list"));
			ACCEL_CMD_LOAD_PKT_SRC(COPY_BYTES_THR1_SRv6_End_Node_OFFSET32_PKT_srv6_seg0__dip0_high_LEN16_TO_PKT_IPv6_Header__dip0_high_OFFSET, ( (PKT<<7) | (selected_segment_list_ofst-32) ));
			__PRAGMA_NO_REORDER
			ACCEL_CMD_TRIG(COPY_BYTES_THR1_SRv6_End_Node_OFFSET32_PKT_srv6_seg0__dip0_high_LEN16_TO_PKT_IPv6_Header__dip0_high_OFFSET, 0);

			/* Update SRH<Segment left> field after decrement it by 1 */
			__LOG_FW(("Update SRH<Segment left> field after decrement it by 1"));
            PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + segments_left_ofst,segments_left);
			__LOG_PARAM_FW(segments_left);

            /* Set offset register back to zero (default value)  */
            __LOG_FW(("Set offset register back to zero (default value)"));
            PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 0);
		}
		else
		{   /* Drop the packet */
			__LOG_FW(("Drop the packet use case"));
            __PRAGMA_FREQUENCY_HINT_NEVER

			/* Packet command allocated by FW set to Hard drop  */
			__LOG_FW(("Packet command allocated by FW set to Hard drop "));
			thr1_desc_ptr->ppa_internal_desc.fw_packet_command = HARD_DROP;
			__LOG_PARAM_FW(thr1_desc_ptr->ppa_internal_desc.fw_packet_command);
		}
    }
    else
    {   /* Drop the packet */
        __LOG_FW(("Drop the packet use case"));
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Packet command allocated by FW set to Hard drop  */
        __LOG_FW(("Packet command allocated by FW set to Hard drop "));
        thr1_desc_ptr->ppa_internal_desc.fw_packet_command = HARD_DROP;
        __LOG_PARAM_FW(thr1_desc_ptr->ppa_internal_desc.fw_packet_command);
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(1);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR2_SRv6_Source_Node_1_segment
 * inparam      None
 * return       None
 * description  Support generation of SRv6 Source node 1 segment
 *              cfg template:    new ipv6 sip (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(32B),IPv6(40B),UDP(8B),VXLAN(8B), Passenger
 *              hdr out: Extension space(08B),IPv6(40B),SRH(8B),Segment0(16B),UDP(8B),VXLAN(8B),Passenger
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment
 *              Firmware adds 24 bytes of SRH(8B) + Segment0(16B)
 *              Update IPv6 SIP with new value taken from the CFG template
 *              Updates IPv6<Payload Length> += 24
 *              Copy IPv6 SIP to Segment0
 *              Copy Thread<Template> to IPv6 SIP
 *              Set SRH header with specific values defined by architect
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR2_SRv6_Source_Node_1_segment)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move IPv6 header left by 24 bytes to make room for SRH(8B) + SR segment(16B)
      - Need to do it in 2 commands since need to move more than 32B (IPv6 is 40B)
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Move left the first 32 bytes of IPv6 */
    __LOG_FW(("Move left the first 32 bytes using accelerator cmd"));
    ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN32_FROM_PKT_IPv6_Header__version_OFFSET, 0);
	__PRAGMA_NO_REORDER
    /* Move left the last 8 bytes of IPv6 */
    ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN8_FROM_PKT_IPv6_Header__version_PLUS32_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update SR Segment with origin IPv6 SIP
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy IPv6 SIP to Segment0 */
    __LOG_FW(("Copy IPv6 SIP to Segment0 using accelerator cmd"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__sip0_high_MINUS24_LEN16_TO_PKT_SRv6_Segment0_Header__dip0_high_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update SR Header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Use max constant 11 bits command to set SRH<Routing Type>=4 and SRH<Segments Left>=1. Clear rest bit to zero. */
    __LOG_FW(("Use max constant 11 bits command to set SRH<Routing Type>=4 and SRH<Segments Left>=1. Clear rest bit to zero."));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x401_LEN11_TO_PKT_SRv6_Header__routing_type_PLUS5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* SRH<Next Header>= IPv6<Next Header> Can not use copy bits command to copy bits from pkt to pkt therefore use copy bytes which must copy minimum 2B
       therefore copy also IPv6<hope_limit> to SRH<hdr_ext_len> which will later be overrun with correct value */
    __LOG_FW(("SRH<Next Header>= IPv6<Next Header> Can not use copy bits command to copy bits from pkt to pkt therefore use copy bytes which must copy minimum 2B"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__next_header_MINUS24_LEN2_TO_PKT_SRv6_Header__next_header_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* SRH<Hdr Ext Len>=2 */
    __LOG_FW(("SRH<Hdr Ext Len>=2"));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2_LEN8_TO_PKT_SRv6_Header__hdr_ext_len_OFFSET, 0);

    /* SRH<Last Entry>=0, SRH<Flags>=0, SRH<Tag>=0 (clear 4 LS bytes to zero) */
    __LOG_FW(("SRH<Last Entry>=0, SRH<Flags>=0, SRH<Tag>=0 (clear 4 LS bytes to zero) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x0_LEN8_TO_PKT_SRv6_Header__last_entry_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update origin IPv6 header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy Thread<Template> to IPv6 SIP */
    __LOG_FW(("Copy Thread<Template> to IPv6 SIP using accelerator cmd"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR2_SRv6_Source_Node_1_segment_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET, 0);

    /* Increment IP header payload length field by 24B.  IPv6 Header<payload length>= IPv6 Header<payload Length> + 24 */
    __LOG_FW(("Increment IP header payload length field by 24B.  IPv6 Header<payload length>= IPv6 Header<payload Length> + 24 using accelerator cmd"));
    ACCEL_CMD_TRIG(ADD_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);

    /* IPv6<Next Header>=43 (0x2B) */
    __LOG_FW(("IPv6<Next Header>=43 (0x2B) using accelerator cmd"));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2B_LEN8_TO_PKT_IPv6_Header__next_header_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Updates Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). In this case plus 24 bytes */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement). In this case plus 24 bytes"));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);
    __PRAGMA_NO_REORDER


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(2);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR3_SRv6_Source_Node_First_Pass_2_3_segments
 * inparam      None
 * return       None
 * description  Handles first pass thread for 2 or 3 segments.
 *              Adds one segment of 16B. SRH will be added in second pass with the option to add maximum of 2 more segments.
 *              cfg template:      new ipv6 sip (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(32B),L2(12B),eDSA(16B),ET(2B),IPv6(40B),Passenger
 *              hdr out: Extension space(16B),L2(12B),eDSA(16B),ET(2B),IPv6(40B),Segment2(16B),Passenger
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment
 *              Firmware adds 16B after IPv6. Those bytes are marked as Segment2 (deepest segment).
 *              It copies IPv6 SIP to Segment2
 *              It copies Thread<Template> to IPv6 SIP
 *              Sets eDSA<target ePort>++
 *              eDSA<Is Trg Phy Port Valid>=0
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR3_SRv6_Source_Node_First_Pass_2_3_segments)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move L2,eDSA,ET and IPv6 headers left by 16 bytes to make room for Segment2(16B)
      - Need to do it in 3 commands since need to move more than 32B (L2(12B) + eDSA(16B) + ET(2B) + IPv6(40B) is 70B)
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Move left the first 32 bytes */
    __LOG_FW(("Move left the first 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
	__PRAGMA_NO_REORDER
    /* Move the next 32 bytes */
    __LOG_FW(("Move left the next 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_PLUS32_OFFSET, 0);
	__PRAGMA_NO_REORDER
    /* Move the last 6 bytes */
    __LOG_FW(("Move left the last 6 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN6_FROM_PKT_mac_header__mac_da_47_32_PLUS64_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update SR Segment with origin IPv6 SIP
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy IPv6 SIP to Segment2 (MINUS16 in command since we already moved the packet left) */
    __LOG_FW(("Copy IPv6 SIP to Segment2 (MINUS16 in command since we already moved the packet left)."));
    ACCEL_CMD_TRIG(COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_PKT_IPv6_Header__sip0_high_MINUS16_LEN16_TO_PKT_SRv6_Segment2_Header__dip0_high_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update origin IPv6 header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy CFG<Template> to IPv6 SIP */
    __LOG_FW(("Copy CFG<Template> to IPv6 SIP"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update eDSA tag
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* eDSA<target ePort>++ */
    __LOG_FW(("eDSA<target ePort>++"));
    ACCEL_CMD_TRIG(ADD_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x1_LEN16_TO_PKT_eDSA_fwd_w3__Trg_ePort_15_0_OFFSET, 0);
    /* eDSA<Is Trg Phy Port Valid>=0 */
    __LOG_FW(("eDSA<Is Trg Phy Port Valid>=0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w2__IsTrgPhyPortValid_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). In this case plus 16 bytes */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement). In this case plus 16 bytes"));
    ACCEL_CMD_TRIG(COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x10_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);
    __PRAGMA_NO_REORDER


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(3);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR4_SRv6_Source_Node_Second_Pass_3_segments
 * inparam      None
 * return       None
 * description  Handles second pass thread for 3 segments.
 *              Basically adds SRH(8B) + Segment0(16B) + Segment1(16B).
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),Generic TS(54B),L2(14B or 18B),IPv6(40B),Segment2(16B)
 *              hdr out: Extension space(62B),L2(14B or 18B),IPv6(40B),SRH(8B),Segment0(16B),Segment1(16B),Segment2(16B)
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              Use Desc<Egress Tag state> to determine if Layer2 header is 14 bytes or 18 bytes
 *              Add 40 Bytes after IPv6 Base Header for SRH, Segment0 and Segment1
 *              IPv6BaseHeader<Payload Length>+= 56 (It includes the 16B added in the first pass + 40B added here)
 *              Set SRH Base, Segment0 and Segment1 from Generic TS<data (36B),EthType(2B) & VLAN(1B)>
 *              Update Desc<fw_bc_modification> field to -14B (0xF2) since we add 40B but remove 54B of Generic TS
 *              NOTE: - Since this thread handles packet with and without vlan there are 2 types of accelerators to support both cases
 *                      these accelerators are placed one after the other. First one handles no vlan and second (next 4B) handles with vlan
 *                    - Require set of offset register at some point to enable accelerator command to reach location beyond 127 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR4_SRv6_Source_Node_Second_Pass_3_segments)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint8_t  egressTagState;          /* Holds Desc<egress_tag_state> which indicates if vlan exist */
    uint8_t  ipv6NextHeader;          /* Holds IPv6 next header field value */
    uint16_t geneircTsDataEthertype;  /* Holds Generic TS Data Ethertype field value */
    uint8_t  geneircTsDataVid;        /* Holds Generic TS Data Vlan ID LS byte value */

    /* Get pointers to descriptor */
    struct ppa_in_desc* thr4_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


    /* Read Desc<egress_tag_state> to know if vlan exist or not */
    __LOG_FW(("Read Desc<egress_tag_state> to know if vlan exist or not"));
    egressTagState = thr4_desc_ptr->phal2ppa.egress_tag_state;
    __LOG_PARAM_FW(thr4_desc_ptr->phal2ppa.egress_tag_state);

    /* Get Generic_TS_Data.vid[7:0] */
    __LOG_FW(("Get Generic_TS_Data.vid[7:0]"));
    geneircTsDataVid = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 63);
    __LOG_PARAM_FW(geneircTsDataVid);

    /* Get Generic_TS_Data.ethertype  */
    __LOG_FW(("Get Generic_TS_Data.ethertype"));
    geneircTsDataEthertype = PPA_FW_SP_SHORT_READ(FALCON_PKT_REGs_lo + 64);
    __LOG_PARAM_FW(geneircTsDataEthertype);


    /* Check if packet is with or without vlan */
    __LOG_FW(("Check if packet is with or without vlan"));
    if(egressTagState == 0)
    {   /* no vlan */
        __LOG_FW(("no vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122);
        __LOG_PARAM_FW(ipv6NextHeader);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_CONST_0xF2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Move Generic TS(54B),L2(14B) and IPv6(40B) headers left by 40 bytes to make room for SRH(8B) + Container0(16B) + Container1(16B)
          - Need to do it in 4 commands since need to move more than 32B (108B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 12 bytes */
        __LOG_FW(("Move left the next 12 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case the location of some of the packet's fields exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 40B which means that the pkt src and dst will be relative to 8B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 8);
		__PRAGMA_NO_REORDER

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 116, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 117, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 119, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0 and Container1
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Copy Generic TS< Data Bytes [31:16]> to Container 0
           Copy Generic TS< Data Bytes [15:0]> to Container 1 */
        __LOG_FW(("Copy Generic TS< Data Bytes [31:16]> to Container 0. Copy Generic TS< Data Bytes [15:0]> to Container 1"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
		__PRAGMA_NO_REORDER
     }
     else if(egressTagState == 1)
     {  /* with vlan */
        __LOG_FW(("with vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122 + 4);
        __LOG_PARAM_FW(ipv6NextHeader);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_CONST_0xF2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Move Generic TS(54B),L2(14B or 18B) and IPv6(40B) headers left by 40 bytes to make room for SRH(8B) + Container0(16B) + Container1(16B)
          - Need to do it in 4 commands since need to move more than 32B (108B or 112B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 16 bytes */
        __LOG_FW(("Move left the next 16 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case location of last 32B of IPv6 exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 40B which means that the pkt src and dst will be relative to 8B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 8);
		__PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 116 + 4, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 117 + 4, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 119 + 4, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0 and Container1
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Copy Generic TS< Data Bytes [31:16]> to Container 0
           Copy Generic TS< Data Bytes [15:0]> to Container 1 */
        __LOG_FW(("Copy Generic TS< Data Bytes [31:16]> to Container 0. Copy Generic TS< Data Bytes [15:0]> to Container 1"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
		__PRAGMA_NO_REORDER
     }
     else
     {  /* not valid: packet should not be changed */
        __LOG_FW(("not valid: packet should not be changed"));

        __LOG_FW((" Desc<Egress Tag state> is not '0' nor '1' "));
        __LOG_PARAM_FW(thr4_desc_ptr->phal2ppa.egress_tag_state);
     }

     /* Set offset register back to zero (default value) */
     __LOG_FW(("Set offset register back to zero (default value)"));
     PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(4);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR5_SRv6_Source_Node_Second_Pass_2_segments
 * inparam      None
 * return       None
 * description  Handles second pass thread for 2 segments.
 *              Basically adds SRH(8B) + Segment0.
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),Generic TS(54B),L2(14B or 18B),IPv6(40B),Segment1(16B)
 *              hdr out: Extension space(78B),L2(14B or 18B),IPv6(40B),SRH(8B),Segment0(16B),Segment1(16B)
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              Use Desc<Egress Tag state> to determine if Layer2 header is 14 bytes or 18 bytes
 *              Add 8 Bytes after IPv6 Base Header for SRH and Segment0
 *              IPv6BaseHeader<Payload Length>+= 40 (It includes the 16B added in the first pass and 24B added here)
 *              Set SRH and Segment0 according to Generic TS.
 *              Update Desc<fw_bc_modification> field to -30B (0xE2) since we add 24B but remove 54B of Generic TS
 *              NOTE: - Since this thread handles packet with and without vlan there are 2 types of accelerators to support both cases
 *                      these accelerators are placed one after the other. First one handles no vlan and second (next 4B) handles with vlan
 *                    - Require set of offset register at some point to enable accelerator command to reach location beyond 127 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR5_SRv6_Source_Node_Second_Pass_2_segments)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint8_t  egressTagState;          /* Holds Desc<egress_tag_state> which indicates if vlan exist */
    uint8_t  ipv6NextHeader;          /* Holds IPv6 next header field value */
    uint16_t geneircTsDataEthertype;  /* Holds Generic TS Data Ethertype field value */
    uint8_t  geneircTsDataVid;        /* Holds Generic TS Data Vlan ID LS byte value */

    /* Get pointers to descriptor */
    struct ppa_in_desc* thr5_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


    /* Read Desc<egress_tag_state> to know if vlan exist or not */
    __LOG_FW(("Read Desc<egress_tag_state> to know if vlan exist or not"));
    egressTagState = thr5_desc_ptr->phal2ppa.egress_tag_state;
    __LOG_PARAM_FW(thr5_desc_ptr->phal2ppa.egress_tag_state);

    /* Get Generic_TS_Data.vid[7:0] */
    __LOG_FW(("Get Generic_TS_Data.vid[7:0]"));
    geneircTsDataVid = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 63);
    __LOG_PARAM_FW(geneircTsDataVid);

    /* Get Generic_TS_Data.ethertype  */
    __LOG_FW(("Get Generic_TS_Data.ethertype"));
    geneircTsDataEthertype = PPA_FW_SP_SHORT_READ(FALCON_PKT_REGs_lo + 64);
    __LOG_PARAM_FW(geneircTsDataEthertype);


    /* Check if packet is with or without vlan */
    __LOG_FW(("Check if packet is with or without vlan"));
    if(egressTagState == 0)
    {   /* no vlan */
        __LOG_FW(("no vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122);
        __LOG_PARAM_FW(ipv6NextHeader);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_CONST_0xE2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Move Generic TS(54B),L2(14B) and IPv6(40B) headers left by 24 bytes to make room for SRH(8B) + Container0(16B)
          - Need to do it in 4 commands since need to move more than 32B (108B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 12 bytes */
        __LOG_FW(("Move left the next 12 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case the location of some of the packet's fields exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 24B which means that the pkt src and dst will be relative to 24B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 24);
		__PRAGMA_NO_REORDER

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 132, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 133, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 135, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Copy Generic TS< Data Bytes [15:0]> to Container 0 */
        __LOG_FW(("Copy Generic TS< Data Bytes [15:0]> to Container 0"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
		__PRAGMA_NO_REORDER
     }
     else if(egressTagState == 1)
     {  /* with vlan */
        __LOG_FW(("with vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122 + 4);
        __LOG_PARAM_FW(ipv6NextHeader);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_CONST_0xE2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Move Generic TS(54B),L2(18B) and IPv6(40B) headers left by 24 bytes to make room for SRH(8B) + Container0(16B)
          - Need to do it in 4 commands since need to move more than 32B (112B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 16 bytes */
        __LOG_FW(("Move left the next 16 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case location of last 32B of IPv6 exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 24B which means that the pkt src and dst will be relative to 24B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 24);
		__PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 132 + 4, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 133 + 4, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 135 + 4, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Copy Generic TS< Data Bytes [15:0]> to Container 0 */
        __LOG_FW(("Copy Generic TS< Data Bytes [15:0]> to Container 0"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
		__PRAGMA_NO_REORDER
     }
     else
     {  /* not valid: packet should not be changed */
        __LOG_FW(("not valid: packet should not be changed"));

        __LOG_FW((" Desc<Egress Tag state> is not '0' nor '1' "));
        __LOG_PARAM_FW(thr5_desc_ptr->phal2ppa.egress_tag_state);
     }

     /* Set offset register back to zero (default value) */
     __LOG_FW(("Set offset register back to zero (default value)"));
     PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(5);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4
 * inparam      None
 * return       None
 * description  ERSPAN over IPv4 tunnel for ePort-based CC (Centralized Chassis) while Egress port of the ERSPAN packet is (LC) Line Card port
 *              Encapsulated Remote Switch Port Analyzer (ERSPAN) allows packets to be mirrored to a remote analyzer over a tunnel encapsulation.
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv4 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring
 *              - In this case the Egress port of the ERSPAN packet is LC (Line Card) port
 *
 *              cfg template:    Not in use (16B). It is used by fw to first hold L2 parameters and then eDSA tag.
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | TO_ANALYZER eDSA tag(16B) | payload |
 *              hdr out: | Extension space (22B) | MAC addresses(12B) | TO_ANALYZER extended DSA tag(8B) | ETH type(2B) | IPv4(20B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              Remove eDSA tag.
 *              Update IPv4 length and calculate IPv4 cs.
 *              Update HW that 42 bytes are added
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    uint32_t    srcTrgDev;
    uint32_t    sessionId;
    uint32_t    smemAnalyzerTemplateBaseAddr;
    uint32_t    deviceIdVal;
    uint32_t    vlanVid;
    uint8_t     getDeviceIdFromPkt;
    uint32_t    smemAnalayzerTemplateOfst;
    uint8_t     descRxSniff;
    uint32_t    smemGlobalVal;
    uint8_t     VoQThread;


    /* Get pointers to descriptor,cfg & packet */
    struct ppa_in_desc* thr6_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_cfg* thr6_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_cfg);
    struct thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_in_hdr* thr6_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_in_hdr);


    /* Get source or target device field value from packet input in eDSA tag */
    __LOG_FW(("Get source or target device field value from packet input in eDSA tag"));
    srcTrgDev = (thr6_in_hdr_ptr->eDSA_w3_ToAnalyzer.SrcTrg_Dev_11_5<<5) | (thr6_in_hdr_ptr->eDSA_w0_ToAnalyzer.SrcTrg_Dev_4_0);
    __LOG_PARAM_FW(srcTrgDev);

    /* Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory */
    __LOG_FW(("Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory"));
    getDeviceIdFromPkt = thr6_in_hdr_ptr->eDSA_w2_ToAnalyzer.Reserved & 0x1;
    __LOG_PARAM_FW(getDeviceIdFromPkt);

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr6_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr6_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr6_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 20B of IPv4 header from shared memory
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 20B of IPv4 header from shared memory"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 60) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | MAC DA SA | reserved | UP | CFI | VID |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | MAC DA SA | reserved | UP | CFI | VID |"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));
	__PRAGMA_NO_REORDER


    /* Get vlan vid from template. Was copied earlier from shared memory */
    __LOG_FW(("Get vlan vid from template. Was copied earlier from shared memory"));
    vlanVid = thr6_cfg_ptr->cc_erspan_template.vlan_vid;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set extended TO_ANALYZER 8B DSA values
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set extended TO_ANALYZER 8B DSA values"));

    /* Set TagCommand field to 2. Plus clear to zero all other fields in the range of 32 bits */
    __LOG_FW(("Set TagCommand field to 2. Plus clear to zero all other fields in the range of 32 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x2_LEN2_TO_PKT_extended_DSA_w0_ToAnalyzer__TagCommand_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

   /* Set SrcTrg_Tagged field to 0 if VID taken from the template is 0 otherwise set it to 1 */
   __LOG_FW(("Set SrcTrg_Tagged field to 0 if VID taken from the template is 0 otherwise set it to 1"));
   if(vlanVid != 0)
   {
       /* Set SrcTrg_Tagged to '1' */
       __LOG_FW(("Set SrcTrg_Tagged to '1'"));
       ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x1_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrg_Tagged_OFFSET, 0);
   }

    /* Set extendedDSATagW0.Extend0 field to '1' plus clear other bits in 32 bits range to zero (clear extendedDSATagW0.Extend1 bit)*/
    __LOG_FW(("Set extendedDSATagW0.Extend0 field to '1' plus clear other bits in 32 bits range to zero (clear extendedDSATagW0.Extend1 bit)"));
	ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x1_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__Extend0_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

    /* Copy CFI field from Thread Template into packet extended TO_ANALYZER tag */
    __LOG_FW(("Copy CFI field from Thread Template into packet extended TO_ANALYZER tag"));
	ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_cfi_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__CFI_OFFSET, 0);

    /* Copy up field from Thread Template into packet extended TO_ANALYZER tag */
    __LOG_FW(("Copy UP field from Thread Template into packet extended TO_ANALYZER tag"));
	ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_up_LEN3_TO_PKT_extended_DSA_w0_ToAnalyzer__UP_OFFSET, 0);

    /* Copy vid field from Thread Template into packet extended TO_ANALYZER tag */
    __LOG_FW(("Copy vid field from Thread Template into packet extended TO_ANALYZER tag"));
	ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_vid_LEN12_TO_PKT_extended_DSA_w0_ToAnalyzer__VID_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Tunnel MAC addresses
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Tunnel MAC addresses"));

    /* Set Tunnel MAC addresses from Thread Template */
    __LOG_FW(("Copy the Tunnel MAC addresses from Thread Template"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_LEN12_TO_PKT_mac_header_outer__mac_da_47_32_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template
     - Remove packet eDSA tag (16B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template"));
    __LOG_FW(("Remove packet eDSA tag (16B)"));

    /* Copy 16 bytes of packet eDSA tag into template */
    __LOG_FW(("Copy 16 bytes of packet eDSA tag into template"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_PKT_eDSA_w0_ToAnalyzer__TagCommand_LEN16_TO_CFG_cc_erspan_template__mac_da_47_32_OFFSET, 0);
   	__PRAGMA_NO_REORDER

    /* Remove the TO_ANALYZER eDSA tag by moving right the original MAC addresses by 16 bytes */
    ACCEL_CMD_TRIG(SHIFTRIGHT_16_BYTES_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
   	__PRAGMA_NO_REORDER

    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Continue to set extended TO_ANALYZER 8B DSA values assuming eDSA tag is now placed in template.
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Continue to set extended TO_ANALYZER 8B DSA values assuming eDSA tag is now placed in template"));

    /* Extract from Global parameters (taken from shared memory) the VoQ Thread indication */
    __LOG_FW(("Extract from Global parameters (taken from shared memory) the VoQ Thread indication"));
    VoQThread = (smemGlobalVal >> 8) & 0xFF;

    __LOG_PARAM_FW(VoQThread);

    if (VoQThread == 1)
    {
        /* extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[9:6] (take it from template byte 6 plus 6 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[9:6] (take it from template byte 6 plus 6 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS6_LEN4_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Dev_PLUS1_OFFSET, 0);

        /* extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[5] (take it from template byte 6 plus 10 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[5] (take it from template byte 6 plus 10 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS10_LEN1_TO_PKT_extended_DSA_w1_ToAnalyzer__SrcTrgAnalyzer_Port_5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

        /* extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[4:0] (take it from template byte 6 plus 11 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[4:0] (take it from template byte 6 plus 11 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS11_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Port_4_0_OFFSET, 0);
    }
    else
    {
        /* extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Device)[4:0] (take it from template byte 4 plus 13 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Device)[4:0] (take it from template byte 4 plus 13 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_15_0_PLUS13_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Dev_OFFSET, 0);

        /* extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer ePort)[5] (take it from template byte 14 plus 3 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer ePort)[5] (take it from template byte 14 plus 3 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_cfi_LEN1_TO_PKT_extended_DSA_w1_ToAnalyzer__SrcTrgAnalyzer_Port_5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

        /* extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer ePort)[4:0] (take it from template byte 14 plus 4 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer ePort)[4:0] (take it from template byte 14 plus 4 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_cfi_PLUS1_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Port_4_0_OFFSET, 0);
    }


    /* extendedDSATag.rx sniff = eDSA TO_ANALYZER(rx sniff) (take it from template byte 0 plus 13 bits) */
    __LOG_FW(("extendedDSATag.rx sniff = eDSA TO_ANALYZER(rx sniff) (take it from template byte 0 plus 13 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS13_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__RxSniff_OFFSET, 0);

    /* extendedDSATag.DropOnSource,PacketIsLooped = eDSA TO_ANALYZER(DropOnSource,PacketIsLooped) (take it from template byte 4 plus 2 bits) */
    __LOG_FW(("extendedDSATag.DropOnSource,PacketIsLooped = eDSA TO_ANALYZER(DropOnSource,PacketIsLooped) (take it from template byte 4 plus 2 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_15_0_PLUS2_LEN2_TO_PKT_extended_DSA_w1_ToAnalyzer__DropOnSource_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x0800 (IPv4)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x0800 (IPv4)"));

    /* EthType 0x0800. Set 0x08, LSB 0x00 was already cleared by previous commands with SET_REMAIN_FIELDS_TO_ZERO */
    __LOG_FW(("EthType 0x0800. Set 0x08, LSB 0x00 was already cleared by previous commands with SET_REMAIN_FIELDS_TO_ZERO "));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x08_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Assign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits */
    __LOG_FW(("AAssign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Assign Desc<LM Counter> into GRE Header Sequence Number field */
    __LOG_FW(("Assign Desc<LM Counter> into the <Sequence Number> field in the GRE header"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

     /* Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0 */
     __LOG_FW(("Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0"));

     /* First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7) */
     __LOG_FW(("First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7)"));
     ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS7_LEN3_TO_PKT_ERSPAN_type_II_header__Index_PLUS3_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	 __PRAGMA_NO_REORDER

     /* Check if need to overrun it since need to take it from shared memory */
     if (getDeviceIdFromPkt == 0)
     {
         /* Extract from Global parameters (taken from shared memory) the Falcon Device ID */
         __LOG_FW(("Extract from Global parameters (taken from shared memory) the Falcon Device ID"));
         deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>2;
         __LOG_PARAM_FW(deviceIdVal);

         PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 76) , deviceIdVal  );
     }
   	 __PRAGMA_NO_REORDER

    /* Check if RX or TX mirroring */
    __LOG_FW(("Assign ERSPAN.Index"));
    if (descRxSniff == 1)
    {
        /* RX mirroring */
        __LOG_FW(("RX mirroring"));

        if ( srcTrgDev <= 7 )
        {   /* RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7) */
            __LOG_FW(("RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7)"));

            /* Set Index Bits [19:18] (type) to '1' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '1'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x1_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits) */
            __LOG_FW(("Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS15_LEN14_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);
        }
        else
        {   /* RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15) */
            __LOG_FW(("RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15)"));

            /* Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits) */
            __LOG_FW(("Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Src Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Src Phy Port)"));

            /* Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits) */
            __LOG_FW(("Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits) */
            __LOG_FW(("Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)*/
            __LOG_FW(("Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero */
        __LOG_FW(("Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , (sessionId >>10) & 0x3FF  );
        __LOG_PARAM_FW((sessionId >>10) & 0x3FF);
    }
    else
    {
        /* TX mirroring */
        __LOG_FW(("TX mirroring"));

        if (srcTrgDev <= 15)
        {   /* TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15) */
            __LOG_FW(("TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15)"));

            /* Set Index Bits [13:9] (target device). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3) */
            __LOG_FW(("Take it from packet eDSA.word2.reserved2 (template byte 0 bits offset 3)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Trg Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Trg Phy Port)"));

            /* Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)*/
            __LOG_FW(("Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4) */
            __LOG_FW(("Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }
        else
        {   /* TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16) */
            __LOG_FW(("TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16)"));

            /* Set Index Bits [19:18] (type) to '2' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '2'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x2_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (Trg Dev)*/
            __LOG_FW(("Set Index Bits [13:0] (Trg Dev)"));

            /* Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9) */
            __LOG_FW(("Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_vid_PLUS5_LEN7_TO_PKT_ERSPAN_type_II_header__Index_PLUS8_OFFSET, 0);

            /* Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero */
        __LOG_FW(("Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , sessionId & 0x3FF  );
        __LOG_PARAM_FW(sessionId & 0x3FF);

    }
   	__PRAGMA_NO_REORDER

    /* Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits) */
    __LOG_FW(("Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_31_16_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0 */
    __LOG_FW(("Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update Desc<Egress Byte Count>
     - Update packet IPv4 length field
     - Calculate IPv4 checksum
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update Desc<Egress Byte Count> + update IPv4 length field + calculate IPv4 checksum"));


    /* Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+20 */
    __LOG_FW(("Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+20"));

    /* First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b) plus clear to zero 2 MS bits */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b) plus clear to zero 2 MS bits"));
	ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv4_Header__total_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv4 total Length +=20 */
    __LOG_FW(("IPv4 total Length +=20"));
    ACCEL_CMD_TRIG(ADD_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x14_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Desc<Egress Byte Count> +=42 */
    __LOG_FW(("Desc<Egress Byte Count> +=42"));
    ACCEL_CMD_TRIG(ADD_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x2A_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

    /* Compute IPv4 CS */
    __LOG_FW(("Compute IPv4 CS"));

    /* Sum first 8 bytes of IPv4 */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__version_OFFSET, 0);

    /* Skip cs field and add to sum 2B of ttl and protocol */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_LEN2_FROM_PKT_IPv4_Header__ttl_OFFSET, 0);

    /* Sum the last 8 bytes of IPv4 (src and dest IP addresses fields) */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__sip_high_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+42 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+42 bytes)."));
    thr6_desc_ptr->ppa_internal_desc.fw_bc_modification = 42;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(6);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6
 * inparam      None
 * return       None
 * description  ERSPAN over IPv6 tunnel for ePort-based CC (Centralized Chassis) while Egress port of the ERSPAN packet is (LC) Line Card port
 *              Encapsulated Remote Switch Port Analyzer (ERSPAN) allows packets to be mirrored to a remote analyzer over a tunnel encapsulation.
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv6 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring
 *              - In this case the Egress port of the ERSPAN packet is LC (Line Card) port
 *
 *              cfg template:    Not in use (16B). It is used by fw to first hold L2 parameters and then eDSA tag.
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | TO_ANALYZER eDSA tag(16B) | payload |
 *              hdr out: | Extension space (2B)  | MAC addresses(12B) | TO_ANALYZER extended DSA tag(8B) | ETH type(2B) | IPv6(40B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              Remove eDSA tag.
 *              Update IPv6 length.
 *              Update HW that 62 bytes are added
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6)() {
     /*xt_iss_client_command("profile","enable");             */
     /*xt_iss_client_command("profile","disable"); _exit(0);   */


    uint32_t    srcTrgDev;
    uint32_t    sessionId;
    uint32_t    smemAnalyzerTemplateBaseAddr;
    uint32_t    deviceIdVal;
    uint32_t    vlanVid;
    uint8_t     getDeviceIdFromPkt;
    uint32_t    smemAnalayzerTemplateOfst;
    uint8_t     descRxSniff;
    uint32_t    smemGlobalVal;
    uint8_t     VoQThread;


    /* Get pointers to descriptor,cfg & packet */
    struct ppa_in_desc* thr7_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_cfg* thr7_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_cfg);
    struct thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_in_hdr* thr7_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_in_hdr);

    /* Get source or target device field value from packet input in eDSA tag */
    __LOG_FW(("Get source or target device field value from packet input in eDSA tag"));
    srcTrgDev = (thr7_in_hdr_ptr->eDSA_w3_ToAnalyzer.SrcTrg_Dev_11_5<<5) | (thr7_in_hdr_ptr->eDSA_w0_ToAnalyzer.SrcTrg_Dev_4_0);
    __LOG_PARAM_FW(srcTrgDev);

    /* Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory */
    __LOG_FW(("Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory"));
    getDeviceIdFromPkt = thr7_in_hdr_ptr->eDSA_w2_ToAnalyzer.Reserved & 0x1;
    __LOG_PARAM_FW(getDeviceIdFromPkt);

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr7_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr7_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr7_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 40B of IPv6 header from shared memory
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 40B of IPv6 header from shared memory"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 24) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 28) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 32) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 36) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 60) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | MAC DA SA | reserved | UP | CFI | VID |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | MAC DA SA | reserved | UP | CFI | VID |"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));
	__PRAGMA_NO_REORDER


    /* Get vlan vid from template. Was copied earlier from shared memory */
    __LOG_FW(("Get vlan vid from template. Was copied earlier from shared memory"));
    vlanVid = thr7_cfg_ptr->cc_erspan_template.vlan_vid;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set extended TO_ANALYZER 8B DSA values
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set extended TO_ANALYZER 8B DSA values"));

    /* Set TagCommand field to 2. Plus clear to zero all other fields in the range of 32 bits */
    __LOG_FW(("Set TagCommand field to 2. Plus clear to zero all other fields in the range of 32 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x2_LEN2_TO_PKT_extended_DSA_w0_ToAnalyzer__TagCommand_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

   /* Set SrcTrg_Tagged field to 0 if VID taken from the template is 0 otherwise set it to 1 */
   __LOG_FW(("Set SrcTrg_Tagged field to 0 if VID taken from the template is 0 otherwise set it to 1"));
   if(vlanVid != 0)
   {
       /* Set SrcTrg_Tagged to '1' */
       __LOG_FW(("Set SrcTrg_Tagged to '1'"));
       ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x1_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrg_Tagged_OFFSET, 0);
   }

    /* Set extendedDSATagW0.Extend0 field to '1' plus clear other bits in 32 bits range to zero (clear extendedDSATagW0.Extend1 bit)*/
    __LOG_FW(("Set extendedDSATagW0.Extend0 field to '1' plus clear other bits in 32 bits range to zero (clear extendedDSATagW0.Extend1 bit)"));
	ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x1_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__Extend0_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

    /* Copy CFI field from Thread Template into packet extended TO_ANALYZER tag */
    __LOG_FW(("Copy CFI field from Thread Template into packet extended TO_ANALYZER tag"));
	ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_cfi_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__CFI_OFFSET, 0);

    /* Copy up field from Thread Template into packet extended TO_ANALYZER tag */
    __LOG_FW(("Copy UP field from Thread Template into packet extended TO_ANALYZER tag"));
	ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_up_LEN3_TO_PKT_extended_DSA_w0_ToAnalyzer__UP_OFFSET, 0);

    /* Copy vid field from Thread Template into packet extended TO_ANALYZER tag */
    __LOG_FW(("Copy vid field from Thread Template into packet extended TO_ANALYZER tag"));
	ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_vid_LEN12_TO_PKT_extended_DSA_w0_ToAnalyzer__VID_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Tunnel MAC addresses
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Tunnel MAC addresses"));

    /* Set Tunnel MAC addresses from Thread Template */
    __LOG_FW(("Copy the Tunnel MAC addresses from Thread Template"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_LEN12_TO_PKT_mac_header_outer__mac_da_47_32_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template
     - Remove packet eDSA tag (16B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template"));
    __LOG_FW(("Remove packet eDSA tag (16B)"));

    /* Copy 16 bytes of packet eDSA tag into template */
    __LOG_FW(("Copy 16 bytes of packet eDSA tag into template"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_PKT_eDSA_w0_ToAnalyzer__TagCommand_LEN16_TO_CFG_cc_erspan_template__mac_da_47_32_OFFSET, 0);
   	__PRAGMA_NO_REORDER

    /* Remove the TO_ANALYZER eDSA tag by moving right the original MAC addresses by 16 bytes */
    ACCEL_CMD_TRIG(SHIFTRIGHT_16_BYTES_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
   	__PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Continue to set extended TO_ANALYZER 8B DSA values assuming eDSA tag is now placed in template.
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Continue to set extended TO_ANALYZER 8B DSA values assuming eDSA tag is now placed in template"));

    /* Extract from Global parameters (taken from shared memory) the VoQ Thread indication */
    __LOG_FW(("Extract from Global parameters (taken from shared memory) the VoQ Thread indication"));
    VoQThread = (smemGlobalVal >> 8) & 0xFF;
    __LOG_PARAM_FW(VoQThread);

    if (VoQThread == 1)
    {
        /* extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[9:6] (take it from template byte 6 plus 6 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[9:6] (take it from template byte 6 plus 6 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS6_LEN4_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Dev_PLUS1_OFFSET, 0);

        /* extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[5] (take it from template byte 6 plus 10 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[5] (take it from template byte 6 plus 10 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS10_LEN1_TO_PKT_extended_DSA_w1_ToAnalyzer__SrcTrgAnalyzer_Port_5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

        /* extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[4:0] (take it from template byte 6 plus 11 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[4:0] (take it from template byte 6 plus 11 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS11_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Port_4_0_OFFSET, 0);
    }
    else
    {
        /* extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Device)[4:0] (take it from template byte 4 plus 13 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Device)[4:0] (take it from template byte 4 plus 13 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_15_0_PLUS13_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Dev_OFFSET, 0);

        /* extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer ePort)[5] (take it from template byte 14 plus 3 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer ePort)[5] (take it from template byte 14 plus 3 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_cfi_LEN1_TO_PKT_extended_DSA_w1_ToAnalyzer__SrcTrgAnalyzer_Port_5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

        /* extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer ePort)[4:0] (take it from template byte 14 plus 4 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer ePort)[4:0] (take it from template byte 14 plus 4 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_cfi_PLUS1_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Port_4_0_OFFSET, 0);
    }


    /* extendedDSATag.rx sniff = eDSA TO_ANALYZER(rx sniff) (take it from template byte 0 plus 13 bits) */
    __LOG_FW(("extendedDSATag.rx sniff = eDSA TO_ANALYZER(rx sniff) (take it from template byte 0 plus 13 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS13_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__RxSniff_OFFSET, 0);

    /* extendedDSATag.DropOnSource,PacketIsLooped = eDSA TO_ANALYZER(DropOnSource,PacketIsLooped) (take it from template byte 4 plus 2 bits) */
    __LOG_FW(("extendedDSATag.DropOnSource,PacketIsLooped = eDSA TO_ANALYZER(DropOnSource,PacketIsLooped) (take it from template byte 4 plus 2 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_15_0_PLUS2_LEN2_TO_PKT_extended_DSA_w1_ToAnalyzer__DropOnSource_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x86dd (IPv6)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x86dd (IPv6)"));

    /* EthType 0x86dd. Do it with 2 accelerators since maximum bits for const command is 11 bits  */
    __LOG_FW(("EthType 0x86dd. Do it with 2 accelerators since maximum bits for const command is 11 bits "));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x86_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0xdd_LEN8_TO_PKT_ethertype_header__ethertype_PLUS8_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Assign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits */
    __LOG_FW(("AAssign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Assign Desc<LM Counter> into GRE Header Sequence Number field */
    __LOG_FW(("Assign Desc<LM Counter> into the <Sequence Number> field in the GRE header"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

     /* Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0 */
     __LOG_FW(("Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0"));

     /* First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7) */
     __LOG_FW(("First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7)"));
     ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS7_LEN3_TO_PKT_ERSPAN_type_II_header__Index_PLUS3_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	 __PRAGMA_NO_REORDER

     /* Check if need to overrun it since need to take it from shared memory */
     if (getDeviceIdFromPkt == 0)
     {
         /* Extract from Global parameters (taken from shared memory) the Falcon Device ID */
         __LOG_FW(("Extract from Global parameters (taken from shared memory) the Falcon Device ID"));
         deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>2;
         __LOG_PARAM_FW(deviceIdVal);

         PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 76) , deviceIdVal  );
     }
   	 __PRAGMA_NO_REORDER

    /* Check if RX or TX mirroring */
    __LOG_FW(("Assign ERSPAN.Index"));
    if (descRxSniff == 1)
    {
        /* RX mirroring */
        __LOG_FW(("RX mirroring"));

        if ( srcTrgDev <= 7 )
        {   /* RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7) */
            __LOG_FW(("RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7)"));

            /* Set Index Bits [19:18] (type) to '1' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '1'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x1_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits) */
            __LOG_FW(("Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS15_LEN14_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);
        }
        else
        {   /* RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15) */
            __LOG_FW(("RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15)"));

            /* Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits) */
            __LOG_FW(("Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Src Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Src Phy Port)"));

            /* Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits) */
            __LOG_FW(("Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits) */
            __LOG_FW(("Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)*/
            __LOG_FW(("Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero */
        __LOG_FW(("Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , (sessionId >>10) & 0x3FF  );
        __LOG_PARAM_FW((sessionId >>10) & 0x3FF);
    }
    else
    {
        /* TX mirroring */
        __LOG_FW(("TX mirroring"));

        if (srcTrgDev <= 15)
        {   /* TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15) */
            __LOG_FW(("TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15)"));

            /* Set Index Bits [13:9] (target device). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3) */
            __LOG_FW(("Take it from packet eDSA.word2.reserved2 (template byte 0 bits offset 3)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Trg Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Trg Phy Port)"));

            /* Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)*/
            __LOG_FW(("Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4) */
            __LOG_FW(("Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }
        else
        {   /* TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16) */
            __LOG_FW(("TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16)"));

            /* Set Index Bits [19:18] (type) to '2' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '2'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x2_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (Trg Dev)*/
            __LOG_FW(("Set Index Bits [13:0] (Trg Dev)"));

            /* Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9) */
            __LOG_FW(("Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_vid_PLUS5_LEN7_TO_PKT_ERSPAN_type_II_header__Index_PLUS8_OFFSET, 0);

            /* Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero */
        __LOG_FW(("Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , sessionId & 0x3FF  );
        __LOG_PARAM_FW(sessionId & 0x3FF);

    }
   	__PRAGMA_NO_REORDER

    /* Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits) */
    __LOG_FW(("Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_31_16_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0 */
    __LOG_FW(("Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update Desc<Egress Byte Count>
     - Update packet IPv6 length field
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update Desc<Egress Byte Count> + update IPv6 length field"));


    /* Set IPv6 Header<payload length> = Desc<Egress Byte Count> */
    __LOG_FW(("Set IPv6 Header<payload length> = Desc<Egress Byte Count>"));

    /* First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b) plus clear to zero 2 MS bits */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b) plus clear to zero 2 MS bits"));
	ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv6_Header__payload_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Desc<Egress Byte Count> +=62 */
    __LOG_FW(("Desc<Egress Byte Count> +=62"));
    ACCEL_CMD_TRIG(ADD_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x3E_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+62 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+62 bytes)."));
    thr7_desc_ptr->ppa_internal_desc.fw_bc_modification = 62;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(7);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4
 * inparam      None
 * return       None
 * description  ERSPAN over IPv4 tunnel for ePort-based CC (Centralized Chassis) while Egress port of the ERSPAN packet is Falcon direct port
 *              Encapsulated Remote Switch Port Analyzer (ERSPAN) allows packets to be mirrored to a remote analyzer over a tunnel encapsulation.
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv4 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring
 *              - In this case the Egress port of the ERSPAN packet is Falcon direct port
 *
 *              cfg template:    Not in use (16B). It is used by fw to first hold L2 parameters and then eDSA tag.
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | TO_ANALYZER eDSA tag(16B) | payload |
 *              2 options: with vlan tag and without vlan tag
 *              hdr out(with vlan): | Extension space (26B) | MAC addresses(12B) | VLAN tag(4B) | ETH type(2B) | IPv4(20B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              hdr out(no vlan):   | Extension space (30B) | MAC addresses(12B) | ETH type(2B) | IPv4(20B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              Remove eDSA tag.
 *              Update IPv4 length and calculate IPv4 cs.
 *              Update HW that 34B(no vlan) or 38B(with vlan) are added
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    uint32_t    srcTrgDev;
    uint32_t    sessionId;
    uint32_t    smemAnalyzerTemplateBaseAddr;
    uint32_t    deviceIdVal;
    uint32_t    vlanVid;
    uint8_t     getDeviceIdFromPkt;
    uint32_t    smemAnalayzerTemplateOfst;
    uint8_t     descRxSniff;
    uint32_t    smemGlobalVal;


    /* Get pointers to descriptor,cfg & packet */
    struct ppa_in_desc* thr8_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_cfg* thr8_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_cfg);
    struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_in_hdr* thr8_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_in_hdr);

    /* Get source or target device field value from packet input in eDSA tag */
    __LOG_FW(("Get source or target device field value from packet input in eDSA tag"));
    srcTrgDev = (thr8_in_hdr_ptr->eDSA_w3_ToAnalyzer.SrcTrg_Dev_11_5<<5) | (thr8_in_hdr_ptr->eDSA_w0_ToAnalyzer.SrcTrg_Dev_4_0);
    __LOG_PARAM_FW(srcTrgDev);

    /* Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory */
    __LOG_FW(("Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory"));
    getDeviceIdFromPkt = thr8_in_hdr_ptr->eDSA_w2_ToAnalyzer.Reserved & 0x1;
    __LOG_PARAM_FW(getDeviceIdFromPkt);

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr8_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr8_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr8_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Copy 16B of L2 parameters from shared memory into CFG template: | MAC DA SA | TPID | UP | CFI | VID |
      - Since L2 location in packet output is not 32 bits align it is copied first by core to template and then by accelerator to packet output
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Copy 16B of L2 parameters from shared memory into CFG template: | MAC DA SA | TPID | UP | CFI | VID |"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 16));
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Copy 20B of IPv4 header from shared memory into packet output
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Copy 20B of IPv4 header from shared memory into packet output"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 60) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));

    /* Get vlan vid from template. Was copied earlier from shared memory */
    __LOG_FW(("Get vlan vid from template. Was copied earlier from shared memory"));
    vlanVid = thr8_cfg_ptr->cc_erspan_template.vlan_vid;
    __LOG_PARAM_FW(vlanVid);

    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x0800 (IPv4)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x0800 (IPv4)"));

    /* EthType 0x0800. Set 0x08, LSB 0x00 is cleared by SET_REMAIN_FIELDS_TO_ZERO data */
    __LOG_FW(("EthType 0x0800. Set 0x08, LSB 0x00 is cleared by SET_REMAIN_FIELDS_TO_ZERO data "));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x08_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update Desc<Egress Byte Count>
     - Update packet IPv4 length field
     - Set MAC DA SA and vlan tag if exist
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update Desc<Egress Byte Count> + update IPv4 length field"));


    /* Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+20 */
    __LOG_FW(("Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+20"));

    /* First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b) plus clear to zero 2 MS bits */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b) plus clear to zero 2 MS bits"));
	ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv4_Header__total_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv4 total Length +=20 */
    __LOG_FW(("IPv4 total Length +=20"));
    ACCEL_CMD_TRIG(ADD_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x14_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);

    if(vlanVid == 0)
    { /* no vlan */
      __LOG_FW(("no vlan"));

        /* Desc<Egress Byte Count> +=34 */
        __LOG_FW(("Desc<Egress Byte Count> +=34"));
        ACCEL_CMD_TRIG(ADD_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x22_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (+34 bytes). */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+34 bytes)."));
        thr8_desc_ptr->ppa_internal_desc.fw_bc_modification = 34;

        /* Set tunnel MAC addresses from template (copied earlier from shared memory to template) */
        __LOG_FW(("Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template)"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_LEN12_TO_PKT_no_vlan__mac_header_outer__mac_da_47_32_OFFSET, 0);
    }
    else
    { /* with vlan */
      __LOG_FW(("with vlan"));

        /* Desc<Egress Byte Count> +=38 */
        __LOG_FW(("Desc<Egress Byte Count> +=38"));
        ACCEL_CMD_TRIG(ADD_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x26_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (+38 bytes). */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+38 bytes)."));
        thr8_desc_ptr->ppa_internal_desc.fw_bc_modification = 38;

        /* Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template) */
        __LOG_FW(("Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template)"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_LEN16_TO_PKT_with_vlan__mac_header_outer__mac_da_47_32_OFFSET, 0);
    }


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template
     - Remove packet eDSA tag (16B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template"));
    __LOG_FW(("Remove packet eDSA tag (16B)"));

    /* Copy 16 bytes of packet eDSA tag into template */
    __LOG_FW(("Copy 16 bytes of packet eDSA tag into template"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_PKT_eDSA_w0_ToAnalyzer__TagCommand_LEN16_TO_CFG_cc_erspan_template__mac_da_47_32_OFFSET, 0);
   	__PRAGMA_NO_REORDER

    /* Remove the TO_ANALYZER eDSA tag by moving right the original MAC addresses by 16 bytes */
    ACCEL_CMD_TRIG(SHIFTRIGHT_16_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
   	__PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Assign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits */
    __LOG_FW(("AAssign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Assign Desc<LM Counter> into GRE Header Sequence Number field */
    __LOG_FW(("Assign Desc<LM Counter> into the <Sequence Number> field in the GRE header"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

     /* Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0 */
     __LOG_FW(("Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0"));

     /* First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7) */
     __LOG_FW(("First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7)"));
     ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS7_LEN3_TO_PKT_ERSPAN_type_II_header__Index_PLUS3_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	 __PRAGMA_NO_REORDER

     /* Check if need to overrun it since need to take it from shared memory */
     if (getDeviceIdFromPkt == 0)
     {
         /* Extract from Global parameters (taken from shared memory) the Falcon Device ID */
         __LOG_FW(("Extract from Global parameters (taken from shared memory) the Falcon Device ID"));
         deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>2;
         __LOG_PARAM_FW(deviceIdVal);

         PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 76) , deviceIdVal  );
     }
   	 __PRAGMA_NO_REORDER

    /* Check if RX or TX mirroring */
    __LOG_FW(("Assign ERSPAN.Index"));
    if (descRxSniff == 1)
    {
        /* RX mirroring */
        __LOG_FW(("RX mirroring"));

        if ( srcTrgDev <= 7 )
        {   /* RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7) */
            __LOG_FW(("RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7)"));

            /* Set Index Bits [19:18] (type) to '1' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '1'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x1_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits) */
            __LOG_FW(("Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS15_LEN14_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);
        }
        else
        {   /* RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15) */
            __LOG_FW(("RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15)"));

            /* Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits) */
            __LOG_FW(("Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Src Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Src Phy Port)"));

            /* Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits) */
            __LOG_FW(("Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits) */
            __LOG_FW(("Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)*/
            __LOG_FW(("Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero */
        __LOG_FW(("Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , (sessionId >>10) & 0x3FF  );
        __LOG_PARAM_FW((sessionId >>10) & 0x3FF);
    }
    else
    {
        /* TX mirroring */
        __LOG_FW(("TX mirroring"));

        if (srcTrgDev <= 15)
        {   /* TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15) */
            __LOG_FW(("TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15)"));

            /* Set Index Bits [13:9] (target device). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3) */
            __LOG_FW(("Take it from packet eDSA.word2.reserved2 (template byte 0 bits offset 3)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Trg Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Trg Phy Port)"));

            /* Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)*/
            __LOG_FW(("Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4) */
            __LOG_FW(("Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }
        else
        {   /* TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16) */
            __LOG_FW(("TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16)"));

            /* Set Index Bits [19:18] (type) to '2' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '2'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x2_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (Trg Dev)*/
            __LOG_FW(("Set Index Bits [13:0] (Trg Dev)"));

            /* Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9) */
            __LOG_FW(("Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__vlan_vid_PLUS5_LEN7_TO_PKT_ERSPAN_type_II_header__Index_PLUS8_OFFSET, 0);

            /* Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero */
        __LOG_FW(("Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , sessionId & 0x3FF  );
        __LOG_PARAM_FW(sessionId & 0x3FF);

    }
   	__PRAGMA_NO_REORDER

    /* Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits) */
    __LOG_FW(("Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_31_16_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0 */
    __LOG_FW(("Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Calculate IPv4 checksum
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate IPv4 checksum"));


    /* Compute IPv4 CS */
    __LOG_FW(("Compute IPv4 CS"));

    /* Sum first 8 bytes of IPv4 */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__version_OFFSET, 0);

    /* Skip cs field and add to sum 2B of ttl and protocol */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_LEN2_FROM_PKT_IPv4_Header__ttl_OFFSET, 0);

    /* Sum the last 8 bytes of IPv4 (src and dest IP addresses fields) */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__sip_high_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(8);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6
 * inparam      None
 * return       None
 * description  ERSPAN over IPv6 tunnel for ePort-based CC (Centralized Chassis) while Egress port of the ERSPAN packet is Falcon direct port
 *              Encapsulated Remote Switch Port Analyzer (ERSPAN) allows packets to be mirrored to a remote analyzer over a tunnel encapsulation.
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv6 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring
 *              - In this case the Egress port of the ERSPAN packet is Falcon direct port
 *
 *              cfg template:    Not in use (16B). It is used by fw to first hold L2 parameters and then eDSA tag.
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | TO_ANALYZER eDSA tag(16B) | payload |
 *              2 options: with vlan tag and without vlan tag
 *              hdr out(with vlan): | Extension space (06B) | MAC addresses(12B) | VLAN tag(4B) | ETH type(2B) | IPv6(40B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              hdr out(no vlan):   | Extension space (10B) | MAC addresses(12B) | ETH type(2B) | IPv6(40B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              Remove eDSA tag.
 *              Update IPv6 length
 *              Update HW that 54B(no vlan) or 58B(with vlan) are added
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    uint32_t    srcTrgDev;
    uint32_t    sessionId;
    uint32_t    smemAnalyzerTemplateBaseAddr;
    uint32_t    deviceIdVal;
    uint32_t    vlanVid;
    uint8_t     getDeviceIdFromPkt;
    uint32_t    smemAnalayzerTemplateOfst;
    uint8_t     descRxSniff;
    uint32_t    smemGlobalVal;


    /* Get pointers to descriptor,cfg & packet */
    struct ppa_in_desc* thr9_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_cfg* thr9_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_cfg);
    struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_in_hdr* thr9_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_in_hdr);

    /* Get source or target device field value from packet input in eDSA tag */
    __LOG_FW(("Get source or target device field value from packet input in eDSA tag"));
    srcTrgDev = (thr9_in_hdr_ptr->eDSA_w3_ToAnalyzer.SrcTrg_Dev_11_5<<5) | (thr9_in_hdr_ptr->eDSA_w0_ToAnalyzer.SrcTrg_Dev_4_0);
    __LOG_PARAM_FW(srcTrgDev);

    /* Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory */
    __LOG_FW(("Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory"));
    getDeviceIdFromPkt = thr9_in_hdr_ptr->eDSA_w2_ToAnalyzer.Reserved & 0x1;
    __LOG_PARAM_FW(getDeviceIdFromPkt);

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr9_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr9_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr9_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Copy 16B of L2 parameters from shared memory into CFG template: | MAC DA SA | TPID | UP | CFI | VID |
      - Since L2 location in packet output is not 32 bits align it is copied first by core to template and then by accelerator to packet output
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Copy 16B of L2 parameters from shared memory into CFG template: | MAC DA SA | TPID | UP | CFI | VID |"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 16));
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Copy 40B of IPv6 header from shared memory into packet output
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Copy 40B of IPv6 header from shared memory into packet output"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 24) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 28) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 32) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 36) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 60) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));

    /* Get vlan vid from template. Was copied earlier from shared memory */
    __LOG_FW(("Get vlan vid from template. Was copied earlier from shared memory"));
    vlanVid = thr9_cfg_ptr->cc_erspan_template.vlan_vid;
    __LOG_PARAM_FW(vlanVid);

    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x86dd (IPv6)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x86dd (IPv6)"));

    /* EthType 0x86dd. Do it with 2 accelerators since maximum bits for const command is 11 bits  */
    __LOG_FW(("EthType 0x86dd. Do it with 2 accelerators since maximum bits for const command is 11 bits "));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x86_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0xdd_LEN8_TO_PKT_ethertype_header__ethertype_PLUS8_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update Desc<Egress Byte Count>
     - Update packet IPv6 length field
     - Set MAC DA SA and vlan tag if exist
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update Desc<Egress Byte Count> + update IPv6 length field"));

    /* Set IPv6 Header<Payload Length> = Desc<Egress Byte Count> */
    __LOG_FW(("Set IPv6 Header<Payload Length>= Desc<Egress Byte Count>"));
	ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv6_Header__payload_length_PLUS2_OFFSET, 0);

    if(vlanVid == 0)
    { /* no vlan */
      __LOG_FW(("no vlan"));

        /* Desc<Egress Byte Count> +=54 */
        __LOG_FW(("Desc<Egress Byte Count> +=54"));
        ACCEL_CMD_TRIG(ADD_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x36_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (+54 bytes). */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+54 bytes)."));
        thr9_desc_ptr->ppa_internal_desc.fw_bc_modification = 54;

        /* Set tunnel MAC addresses from template (copied earlier from shared memory to template) */
        __LOG_FW(("Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template)"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_LEN12_TO_PKT_no_vlan__mac_header_outer__mac_da_47_32_OFFSET, 0);
    }
    else
    { /* with vlan */
      __LOG_FW(("with vlan"));

        /* Desc<Egress Byte Count> +=58 */
        __LOG_FW(("Desc<Egress Byte Count> +=58"));
        ACCEL_CMD_TRIG(ADD_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x3a_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (+58 bytes). */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+58 bytes)."));
        thr9_desc_ptr->ppa_internal_desc.fw_bc_modification = 58;

        /* Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template) */
        __LOG_FW(("Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template)"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_LEN16_TO_PKT_with_vlan__mac_header_outer__mac_da_47_32_OFFSET, 0);
    }


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template
     - Remove packet eDSA tag (16B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template"));
    __LOG_FW(("Remove packet eDSA tag (16B)"));

    /* Copy 16 bytes of packet eDSA tag into template */
    __LOG_FW(("Copy 16 bytes of packet eDSA tag into template"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_PKT_eDSA_w0_ToAnalyzer__TagCommand_LEN16_TO_CFG_cc_erspan_template__mac_da_47_32_OFFSET, 0);
   	__PRAGMA_NO_REORDER

    /* Remove the TO_ANALYZER eDSA tag by moving right the original MAC addresses by 16 bytes */
    ACCEL_CMD_TRIG(SHIFTRIGHT_16_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
   	__PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Assign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits */
    __LOG_FW(("AAssign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Assign Desc<LM Counter> into GRE Header Sequence Number field */
    __LOG_FW(("Assign Desc<LM Counter> into the <Sequence Number> field in the GRE header"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

     /* Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0 */
     __LOG_FW(("Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0"));

     /* First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7) */
     __LOG_FW(("First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7)"));
     ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS7_LEN3_TO_PKT_ERSPAN_type_II_header__Index_PLUS3_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	 __PRAGMA_NO_REORDER

     /* Check if need to overrun it since need to take it from shared memory */
     if (getDeviceIdFromPkt == 0)
     {
         /* Extract from Global parameters (taken from shared memory) the Falcon Device ID */
         __LOG_FW(("Extract from Global parameters (taken from shared memory) the Falcon Device ID"));
         deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>2;
         __LOG_PARAM_FW(deviceIdVal);

         PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 76) , deviceIdVal  );
     }
   	 __PRAGMA_NO_REORDER

    /* Check if RX or TX mirroring */
    __LOG_FW(("Assign ERSPAN.Index"));
    if (descRxSniff == 1)
    {
        /* RX mirroring */
        __LOG_FW(("RX mirroring"));

        if ( srcTrgDev <= 7 )
        {   /* RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7) */
            __LOG_FW(("RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7)"));

            /* Set Index Bits [19:18] (type) to '1' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '1'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x1_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits) */
            __LOG_FW(("Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS15_LEN14_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);
        }
        else
        {   /* RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15) */
            __LOG_FW(("RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15)"));

            /* Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits) */
            __LOG_FW(("Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Src Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Src Phy Port)"));

            /* Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits) */
            __LOG_FW(("Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits) */
            __LOG_FW(("Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)*/
            __LOG_FW(("Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero */
        __LOG_FW(("Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , (sessionId >>10) & 0x3FF  );
        __LOG_PARAM_FW((sessionId >>10) & 0x3FF);
    }
    else
    {
        /* TX mirroring */
        __LOG_FW(("TX mirroring"));

        if (srcTrgDev <= 15)
        {   /* TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15) */
            __LOG_FW(("TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15)"));

            /* Set Index Bits [13:9] (target device). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3) */
            __LOG_FW(("Take it from packet eDSA.word2.reserved2 (template byte 0 bits offset 3)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Trg Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Trg Phy Port)"));

            /* Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)*/
            __LOG_FW(("Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4) */
            __LOG_FW(("Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }
        else
        {   /* TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16) */
            __LOG_FW(("TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16)"));

            /* Set Index Bits [19:18] (type) to '2' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '2'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x2_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (Trg Dev)*/
            __LOG_FW(("Set Index Bits [13:0] (Trg Dev)"));

            /* Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9) */
            __LOG_FW(("Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__vlan_vid_PLUS5_LEN7_TO_PKT_ERSPAN_type_II_header__Index_PLUS8_OFFSET, 0);

            /* Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)"));
     		ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero */
        __LOG_FW(("Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , sessionId & 0x3FF  );
        __LOG_PARAM_FW(sessionId & 0x3FF);

    }
   	__PRAGMA_NO_REORDER

    /* Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits) */
    __LOG_FW(("Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_31_16_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0 */
    __LOG_FW(("Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(9);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR10_Cc_Erspan_TypeII_SrcDevMirroring
 * inparam      None
 * return       None
 * description  ePort-based CC (Centralized Chassis) for source device mirroring.
 *              Thread to process ERSPAN Type II Source Device Mirroring.
 *
 *              cfg template:    Not in use (16B). It is used by fw to hold device ID value.
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | eDSA tag(16B) | payload |
 *              hdr out: | Extension space (64B) | MAC addresses(12B) | eDSA tag(16B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *              Firmware fill eDSA.W2.reserved bits 22:24 with device ID value
 *              Also set eDSA.W2.reserved bit 21 to '1' to indicate that Falcon Device ID was set.
 *              The header remains with the same size no addition here.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR10_Cc_Erspan_TypeII_SrcDevMirroring)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    deviceIdVal;
    uint32_t    smemGlobalVal ;


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));


    /* Extract from Global parameters the Falcon Device ID */
    __LOG_FW(("Extract from Global parameters the Falcon Device ID"));
    deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>16;
    __LOG_PARAM_FW(deviceIdVal);

    /* Write it first to template */
    __LOG_FW(("Write it first to template"));
    PPA_FW_SP_WRITE( FALCON_CFG_REGs_lo , deviceIdVal  );
    __PRAGMA_NO_REORDER

    /* Now set eDSA.word2 bits 24:22 with the Falcon Device ID which is now placed in template */
    __LOG_FW(("Now set eDSA.word2 bits 24:22 with the Falcon Device ID which is now placed in template"));
    ACCEL_CMD_TRIG(COPY_BITS_THR10_Cc_Erspan_TypeII_SrcDevMirroring_CFG_HA_Table_reserved_space__reserved_0_PLUS29_LEN3_TO_PKT_eDSA_w2_ToAnalyzer__Reserved_PLUS6_OFFSET, 0);

    /* Set '1' in eDSA.word2 bit 21 to indicate that Falcon Device ID was set */
    __LOG_FW(("Set '1' in eDSA.word2 bit 21 to indicate that Falcon Device ID was set"));
    ACCEL_CMD_TRIG(COPY_BITS_THR10_Cc_Erspan_TypeII_SrcDevMirroring_CONST_0x1_LEN1_TO_PKT_eDSA_w2_ToAnalyzer__Reserved_PLUS9_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(10);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR11_VXLAN_GPB_SourceGroupPolicyID
 * inparam      None
 * return       None
 * description  Thread to support VXLAN-GPB (Group Policy Based) when the Source Group Policy ID is carried in the Desc<Copy Reserved> field.
 *              The <Copy Reserved> bits that are allocated for the Source Group Policy ID should be copied to the VXLAN-GBP field <Group Policy ID>.
 *              Support IPv4 and IPv6 packets.
 *
 *              cfg template:    | reserved 0 (4B)|
 *                               | reserved 1 (4B)|
 *                               | reserved 2 (4B)|
 *                               | reserved 3 (22b) | CopyReservedMSB (5b) | CopyReservedLSB (5b) |
 *
 *                               CopyReservedMSB:
 *                               - Indicates fw the MS bit of the Group Policy ID value inside Desc<copy reserved> field.
 *                               - Meaning which bit is the last of Group Policy value inside Desc<copy reserved> field.
 *                               - Applicable values: For Falcon it should be 2:19. It should be checked by the CPSS API and not by fw.
 *                               CopyReservedLSB:
 *                               - Indicates fw the LS bit of the Group Policy ID value inside Desc<copy reserved> field.
 *                               - Meaning from which bit the Group Policy value starts inside Desc<copy reserved> field.
 *                               - Applicable values: For Falcon it should be 2:19. It should be checked by the CPSS API and not by fw.
 *
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space (32B) | IPv4/IPv6(20B/40B) | UDP(8B) | VXLAN-GPB(8B) | original packet |
 *              hdr out: Extension space (32B) | IPv4/IPv6(20B/40B) | UDP(8B) | VXLAN-GPB(8B) | original packet |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment
 *
 *              Firmware actions:
 *              - Extracts Group Policy ID value out from Desc<copy reserved>
 *              - Copies it into packet VXLAN header<Group Policy ID>
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR11_VXLAN_GPB_SourceGroupPolicyID)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    uint32_t    groupPolicyIdLength;
    uint32_t    groupPolicyIdmask;
    uint16_t    groupPolicyIdValue;

    /* Get pointers to descriptor & input packet (can be either IPv4 or IPv6) */
    struct thr11_vxlan_gpb_cfg* thr11_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr11_vxlan_gpb_cfg);
    struct thr11_vxlan_gpb_in_hdr_ipv4* thr11_ipv4_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr11_vxlan_gpb_in_hdr_ipv4);
    struct thr11_vxlan_gpb_in_hdr_ipv6* thr11_ipv6_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr11_vxlan_gpb_in_hdr_ipv6);


    /* Use CopyReservedMSB and CopyReservedLSB from template to check number of bits of Group Policy ID value */
    __LOG_FW(("Use CopyReservedMSB and CopyReservedLSB from template to check number of bits of Group Policy ID value"));
    __LOG_PARAM_FW(thr11_cfg_ptr->vxlan_gpb_template.CopyReservedMSB);
    __LOG_PARAM_FW(thr11_cfg_ptr->vxlan_gpb_template.CopyReservedLSB);
    groupPolicyIdLength = (thr11_cfg_ptr->vxlan_gpb_template.CopyReservedMSB - thr11_cfg_ptr->vxlan_gpb_template.CopyReservedLSB) + 1;
    __LOG_PARAM_FW(groupPolicyIdLength);

    /* Set Group Policy ID mask compatible to number of bits */
    __LOG_FW(("Set Group Policy ID mask compatible to number of bits"));
    groupPolicyIdmask = (1 << groupPolicyIdLength) - 1 ;
    __LOG_PARAM_FW(groupPolicyIdmask);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Extract Group Policy ID value from Desc<copy_reserved> field
      -
      -  1. Read 32 bits from descriptor while the 20 MS bits are copy_reserved field
      -  2. Move right by 12 bits so copy_reserved field starts from bit#0
      -  3. Now move right by CopyReservedLSB bits so the Group Policy ID value from copy_reserved field will start from bit #0
      -  4. Mask to get Group Policy ID value
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
      __LOG_FW(("Extract Group Policy ID value from Desc<copy_reserved> field"));

    __LOG_PARAM_FW(PPA_FW_SP_READ(FALCON_DESC_REGs_lo + 12));
    groupPolicyIdValue = (PPA_FW_SP_READ(FALCON_DESC_REGs_lo + 12) >> (12 + thr11_cfg_ptr->vxlan_gpb_template.CopyReservedLSB) ) & groupPolicyIdmask ;
    __LOG_PARAM_FW(groupPolicyIdValue);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Write Group Policy ID value into packet VXLAN header<Group_Policy_ID>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
      __LOG_FW(("Write Group Policy ID value into packet VXLAN header<Group_Policy_ID>"));

    /* Check packet IP version (IPv4 or IPv6) */
    __LOG_FW(("Check packet IP version (IPv4 or IPv6)"));
    __LOG_PARAM_FW(thr11_ipv4_in_hdr_ptr->IPv4_Header.version);
    if (thr11_ipv4_in_hdr_ptr->IPv4_Header.version == 4)
    {
        /* Packet is IPv4. Set VXLAN header<Group_Policy_ID> with Group Policy ID */
        __LOG_FW(("Packet is IPv4. Set VXLAN header<Group_Policy_ID> with Group Policy ID"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 62) , groupPolicyIdValue );
    }
    else if (thr11_ipv6_in_hdr_ptr->IPv6_Header.version == 6)
    {
        /* Packet is IPv6. Set VXLAN header<Group_Policy_ID> with Group Policy ID */
        __LOG_FW(("Packet is IPv6. Set VXLAN header<Group_Policy_ID> with Group Policy ID"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 62 + 20) , groupPolicyIdValue );
    }
    else
    {
        /* WARNING WARNING WARNING: Invalid packet since no IPv4 and no IPv6 !!! Do not change the packet */
        __LOG_FW(("WARNING WARNING WARNING: Invalid packet since no IPv4 and no IPv6 !!! Do not change the packet"));
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(11);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR12_MPLS_SR_NO_EL
 * inparam      None
 * return       None
 * description  Ingress packets are bridged and associated with Generic TS (Contains labels and data how to generate the MPLS tunnel)
 *              PHA process the Generic Tunnel to generate the required MPLS SR header
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),data(4B),L1(4B),L2(4B),L3(4B),L4(4B),L5(4B),L6(4B),L7(4B),L8(4B)
 *              hdr out: Extension space(52B),L1(4B),L2(4B),L3(4B),L4(4B),L5(4B),L6(4B),L7(4B),L8(4B)
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              MPLS data includes information regarding the offsets to the new labels.
 *              In this specific thread no EL (Entropy Label) is added however MPLS data is removed and origin labels might be removed as well.
 *              Also BoS bit of last label (the one preceding the passenger) is set to '1' to indicate last label.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR12_MPLS_SR_NO_EL)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    /* Get pointer to descriptor  */
    struct ppa_in_desc* thr12_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr12_MPLS_NO_EL_in_hdr* thr12_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr12_MPLS_NO_EL_in_hdr);

    /* Set <BoS> in last label (adjacent to passenger) */
    __LOG_FW(("Set <BoS> in last label (adjacent to passenger)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR12_MPLS_SR_NO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (two's complement). 48B is the incoming L3 offset. */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement). 48B is the incoming L3 offset."));
    thr12_desc_ptr->ppa_internal_desc.fw_bc_modification = (48 - thr12_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr12_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr12_desc_ptr->ppa_internal_desc.fw_bc_modification);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed "));
    falcon_stack_overflow_checker(12);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR13_MPLS_SR_ONE_EL
 * inparam      None
 * return       None
 * description  Ingress packets are bridged and associated with Generic TS (Contains labels and data how to generate the MPLS tunnel)
 *              PHA process the Generic Tunnel to generate the required MPLS SR header
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),data,L1,L2,L3,L4,L5,L6,L7,L8
 *              hdr out: Extension space(44B),L1,L2,L3,L4,L5,L6,ELI_1,EL_1,L7,L8
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              MPLS data includes information regarding the offsets to the new labels.
 *              In this specific thread one pair of ELI(Entropy Label Indicator) and EL(Entropy Label) is added.
 *              MPLS data is removed and origin labels might be removed as well.
 *              Also BoS bit of last label (the one preceding the passenger) is set to '1' to indicate last label.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR13_MPLS_SR_ONE_EL)() {
    /*xt_iss_client_command("profile","enable");            */
    /*xt_iss_client_command("profile","disable"); _exit(0); */

    uint8_t  eli1_cmd_byte_position ;

    /* Get pointer to descriptor  */
    struct ppa_in_desc* thr13_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr13_MPLS_ONE_EL_in_hdr* thr13_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr13_MPLS_ONE_EL_in_hdr);

    /* Indicates HW about the new starting point of the packet header (two's complement)  */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement) "));
    thr13_desc_ptr->ppa_internal_desc.fw_bc_modification = (48-thr13_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr13_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr13_desc_ptr->ppa_internal_desc.fw_bc_modification);

    /* Get the destination position which points to ELI label. Set MS bit to 1 to define pkt type for accelerator command usage */
    __LOG_FW(("Get the destination position which points to ELI label. Set MS bit to 1 to define pkt type for accelerator command usage"));
    eli1_cmd_byte_position = ( (PKT<<7)|thr13_in_hdr_ptr->MPLS_data.EL1_ofst ) ;
    __LOG_PARAM_FW(thr13_in_hdr_ptr->MPLS_data.EL1_ofst);
    __LOG_PARAM_FW(eli1_cmd_byte_position);

    /*--------------------------------------------------------------------------------------------------------------------------------
      - Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels
      --------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending label's offset) to make room for EL & ELI labels"));

    /* Note: To save cycles I prepared predefined commands of 8 options. All options move the labels to the same destination
             but each option with different size from 4B (1 label) till 32B (all 8 labels).  Minimum offset to add the new labels
             is after first label and maximum should be after origin label 8 */
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET + (thr13_in_hdr_ptr->MPLS_data.EL1_ofst-48), 0);
	__PRAGMA_NO_REORDER

    /*--------------------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      --------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Trigger the command: Copy 4B of preceding label into ELI label */
    __LOG_FW(("Trigger the command: Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_LOAD_PKT_DEST_AND_PKT_SRC(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_OFFSET, \
                                           eli1_cmd_byte_position,(eli1_cmd_byte_position-4) ) ;
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli1_cmd_byte_position+1) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli1_cmd_byte_position+2) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*--------------------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1  Label value[18:0] = Desc<Hash[11:0]>
      --------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label: TC, TTL, BoS =0 , Label value[19]=1  Label value[18:0] = Desc<Hash[11:0]>"));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position+4 );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli1_cmd_byte_position+1) + 4);
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*--------------------------------------------------------------------------------------------------------------------------------
      -  Update last label (BoS=1)
      --------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update last label (BoS=1) "));

    /* Set <BoS> in last label (adjacent to passenger) */
    __LOG_FW(("Set <BoS> in last label (adjacent to passenger)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /* Return this accelerator command to its origin value so it can be used also in SGT thread */
    ACCEL_CMD_LOAD(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_VALUE);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(13);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR14_MPLS_SR_TWO_EL
 * inparam      None
 * return       None
 * description  Ingress packets are bridged and associated with Generic TS (Contains labels and data how to generate the MPLS tunnel)
 *              PHA process the Generic Tunnel to generate the required MPLS SR header
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not is use (4B)
 *              hdr in:  Extension space(48B),data,L1,L2,L3,L4,L5,L6,L7,L8
 *              hdr out: Extension space(36B),L1,L2,L3,L4,L5,L6,ELI_2,EL_2,L7,L8,ELI_1,EL_1
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              MPLS data includes information regarding the offsets to the new labels.
 *              In this specific thread two pairs of ELI(Entropy Label Indicator) and EL(Entropy Label) are added.
 *              MPLS data is removed and origin labels might be removed as well.
 *              Also BoS bit of last label (the one preceding the passenger) is set to '1' to indicate last label.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR14_MPLS_SR_TWO_EL)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    uint8_t  eli1_cmd_byte_position, eli2_cmd_byte_position, eli1_ofst, eli2_ofst, cpy_preceding_lbl1_cmd_ofst, cpy_preceding_lbl2_cmd_ofst ;

    /* Get pointer to descriptor  */
    struct ppa_in_desc* thr14_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr14_MPLS_TWO_EL_in_hdr* thr14_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr14_MPLS_TWO_EL_in_hdr);

    /* Indicates HW about the new starting point of the packet header (two's complement)  */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement) "));
    thr14_desc_ptr->ppa_internal_desc.fw_bc_modification = (48-thr14_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr14_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr14_desc_ptr->ppa_internal_desc.fw_bc_modification);

    /* Get the destination position which points to ELI1 label. Set MS bit to 1 to define pkt type for accelerator command usage
       Get the offset to the command of copy preceding label according to ELI offset minus 4.
       First copy command starts from offset 28 since first location to add ELI is after L1 therefore L1 is the preceding label to copy from.
       L1 can be shifted left maximum 8*3 for case of 3 MPLS labels in this case its offset is 28B so this is the first preceding copy command */
    __LOG_FW(("Get the destination position which points to ELI label. Set MS bit to 1 to define pkt type for accelerator command usage"));
    __LOG_FW(("Get the offset to the command of copy preceding label according to ELI offset minus 4. First copy command starts from offset 28"));
    eli1_cmd_byte_position = ( (PKT<<7)|thr14_in_hdr_ptr->MPLS_data.EL1_ofst ) ;
    eli1_ofst = thr14_in_hdr_ptr->MPLS_data.EL1_ofst;
    cpy_preceding_lbl1_cmd_ofst = (eli1_ofst - 4) - 28 ;
    __LOG_PARAM_FW(eli1_cmd_byte_position);
    __LOG_PARAM_FW(eli1_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl1_cmd_ofst);

    /* Get offsets same as done for EL1 */
    __LOG_FW(("Get offsets same as done for EL1"));
    eli2_cmd_byte_position = ( (PKT<<7)|thr14_in_hdr_ptr->MPLS_data.EL2_ofst ) ;
    eli2_ofst = thr14_in_hdr_ptr->MPLS_data.EL2_ofst;
    cpy_preceding_lbl2_cmd_ofst = (eli2_ofst - 4) - 28 ;
    __LOG_PARAM_FW(eli2_cmd_byte_position);
    __LOG_PARAM_FW(eli2_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl2_cmd_ofst);


    /*------------------------------------------------------------------------------------------------------------------
      -  Handle first EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle first EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    /* Note: To save cycles I prepared predefined commands of 8 options. All options move the labels to the same destination
             but each option with different size from 4B (1 label) till 32B (all 8 labels).  Minimum offset to add the new labels
             after first label and maximum should be after origin label 8 */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET + (eli1_ofst-48), 0);
	__PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl1_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_OFFSET + cpy_preceding_lbl1_cmd_ofst, 0);
	__PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli1_cmd_byte_position+1) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli1_cmd_byte_position+2) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1   Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label: TC, TTL, BoS =0, Label value[19]=1 Label value[18:0] = Desc<Hash[11:0]"));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position+4 );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli1_cmd_byte_position+1) + 4);
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------
      -  Handle second EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle second EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET + (eli2_ofst-40), 0);
	__PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl2_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_OFFSET + cpy_preceding_lbl2_cmd_ofst, 0);
	__PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli2_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli2_cmd_byte_position+1) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli2_cmd_byte_position+2) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1   Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label: TC, TTL, BoS =0, Label value[19]=1 Label value[18:0] = Desc<Hash[11:0]"));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli2_cmd_byte_position+4 );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli2_cmd_byte_position+1) + 4);
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update last label (BoS=1)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update last label (BoS=1) "));

    /* Set <BoS> in last label (adjacent to passenger) */
    __LOG_FW(("Set <BoS> in last label (adjacent to passenger)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /* Return this accelerator command to its origin value so it can be used also in SGT thread */
    ACCEL_CMD_LOAD(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_VALUE);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(14);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR15_MPLS_SR_THREE_EL
 * inparam      None
 * return       None
 * description  Ingress packets are bridged and associated with Generic TS (Contains labels and data how to generate the MPLS tunnel)
 *              PHA process the Generic Tunnel to generate the required MPLS SR header
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not is use (4B)
 *              hdr in:  Extension space(48B),data,L1,L2,L3,L4,L5,L6,L7,L8
 *              hdr out: Extension space(28B),L1,L2,L3,ELI_3,EL_3,L4,L5,L6,ELI_2,EL_2,L7,L8,ELI_1,EL_1
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              MPLS data includes information regarding the offsets to the new labels.
 *              In this specific thread three pairs of ELI(Entropy Label Indicator) and EL(Entropy Label) are added.
 *              MPLS data is removed and origin labels might be removed as well.
 *              Also BoS bit of last label (the one preceding the passenger) is set to '1' to indicate last label.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR15_MPLS_SR_THREE_EL)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    uint8_t eli1_cmd_byte_position, eli2_cmd_byte_position, eli3_cmd_byte_position, eli1_ofst, eli2_ofst, eli3_ofst, cpy_preceding_lbl1_cmd_ofst, cpy_preceding_lbl2_cmd_ofst, cpy_preceding_lbl3_cmd_ofst ;

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr15_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr15_MPLS_THREE_EL_in_hdr* thr15_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr15_MPLS_THREE_EL_in_hdr);

    /* Indicates HW about the new starting point of the packet header (two's complement) */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement) "));
    thr15_desc_ptr->ppa_internal_desc.fw_bc_modification = (48-thr15_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr15_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr15_desc_ptr->ppa_internal_desc.fw_bc_modification);

    /* Get the destination position which points to ELI1 label. Set MS bit to 1 to define pkt type for accelerator command usage
       Get the offset to the command of copy preceding label according to ELI offset minus 4.
       First copy command starts from offset 28 since first location to add ELI is after L1 therefore L1 is the preceding label to copy from.
       L1 can be shifted left maximum 8*3 for case of 3 MPLS labels in this case its offset is 28B so this is the first preceding copy command */
    __LOG_FW(("Get the destination position which points to ELI label. Set MS bit to 1 to define pkt type for accelerator command usage"));
    __LOG_FW(("Get the offset to the command of copy preceding label according to ELI offset minus 4. First copy command starts from offset 28"));
    eli1_ofst = thr15_in_hdr_ptr->MPLS_data.EL1_ofst;
    cpy_preceding_lbl1_cmd_ofst = (eli1_ofst - 4) - 28 ;
    eli1_cmd_byte_position = ( (PKT<<7)|eli1_ofst) ;
    __LOG_PARAM_FW(eli1_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl1_cmd_ofst);
    __LOG_PARAM_FW(eli1_cmd_byte_position);

    /* Get offsets same as done for EL1 */
    __LOG_FW(("Get offsets same as done for EL1"));
    eli2_ofst = thr15_in_hdr_ptr->MPLS_data.EL2_ofst;
    cpy_preceding_lbl2_cmd_ofst = (eli2_ofst - 4) - 28 ;
    eli2_cmd_byte_position = ( (PKT<<7)|eli2_ofst) ;
    __LOG_PARAM_FW(eli2_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl2_cmd_ofst);
    __LOG_PARAM_FW(eli2_cmd_byte_position);

    /* Get offsets same as done for EL1 */
    __LOG_FW(("Get offsets same as done for EL1"));
    eli3_ofst = thr15_in_hdr_ptr->MPLS_data.EL3_ofst;
    cpy_preceding_lbl3_cmd_ofst = (eli3_ofst - 4) - 28 ;
    eli3_cmd_byte_position = ( (PKT<<7)|eli3_ofst) ;
    __LOG_PARAM_FW(eli3_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl3_cmd_ofst);
    __LOG_PARAM_FW(eli3_cmd_byte_position);

    /*------------------------------------------------------------------------------------------------------------------
      -  Handle first EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle first EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    /* Note: To save cycles I prepared predefined commands of 8 options. All options move the labels to the same destination
             but each option with different size from 4B (1 label) till 32B (all 8 labels).  Minimum offset to add the new labels
             after first label and maximum should be after origin label 8 */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET + (eli1_ofst-48), 0);
	__PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl1_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_MPLS_label_2__label_val_OFFSET + cpy_preceding_lbl1_cmd_ofst, 0);
	__PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli1_cmd_byte_position+1) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli1_cmd_byte_position+2) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1  Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label:  TC, TTL, BoS =0 , Label value[19]=1  Label value[18:0] = Desc<Hash[11:0]"));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position+4 );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli1_cmd_byte_position+1) + 4);
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Handle second EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle second EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET + (eli2_ofst-40), 0);
	__PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl2_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_MPLS_label_2__label_val_OFFSET + cpy_preceding_lbl2_cmd_ofst, 0);
	__PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli2_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli2_cmd_byte_position+1) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli2_cmd_byte_position+2) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1 Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label:  TC, TTL, BoS =0 , Label value[19]=1 Label value[18:0] = Desc<Hash[11:0] "));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli2_cmd_byte_position+4 );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli2_cmd_byte_position+1) + 4);
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Handle third EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle second EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET + (eli3_ofst-32), 0);
	__PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl2_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_MPLS_label_2__label_val_OFFSET + cpy_preceding_lbl3_cmd_ofst, 0);
	__PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli3_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli3_cmd_byte_position+1) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli3_cmd_byte_position+2) );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1   Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label:  TC, TTL, BoS =0 , Label value[19]=1   Label value[18:0] = Desc<Hash[11:0]  "));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli3_cmd_byte_position+4 );
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli3_cmd_byte_position+1) + 4);
	__PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update last label (BoS=1)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update last label (BoS=1) "));

    /* Set <BoS> in last label (adjacent to passenger) */
    __LOG_FW(("Set <BoS> in last label (adjacent to passenger)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /* Return this accelerator command to its origin value so it can be used also in SGT thread */
    ACCEL_CMD_LOAD(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_VALUE);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(15);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR46_SFLOW_IPv4
 * inparam      None
 * return       None
 * description  sFlow is a technology for monitoring traffic in data networks containing switches and routers.
 *              It requires ingress/egress mirrored packets to be sent over an IPv4-UDP tunnel to a “collector” in the network.
 *              Following the UDP header there is SFLOW header which is followed by the mirrored packet as the payload.
 *              The L2-IPv4-UDP tunnel encapsulation is added by the Falcon Tunnel-start in the HA unit.
 *              The SFLOW header is inserted by the PHA firmware after the UDP header.
 *
 *              cfg template: 4 LSB holds IPv4 agent address rest 12 bytes are reserved
 *              src & trg entries: Not in use (4B)
 *              hdr in:    | Extension space (64B) | IPv4(20B) | UDP(8B) | payload |
 *              hdr out:   | Extension space (36B) | IPv4(20B) | UDP(8B) | sFlow header(28B) | payload |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmwares adds sFlow header of 28 bytes size and sets it according to architect's definitions.
 *              Updates HW that 28 bytes were added
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR46_SFLOW_IPv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    ts_sec;               /* timestamp in seconds */
    uint32_t    ts_nano_sec;          /* timestamp in nano seconds */
    uint32_t    ts_sec_new;           /* holds timestamp in seconds after reading it for the second time to check wraparound */
    uint32_t    ts_sec_to_msec;       /* holds seconds value in milliseconds units */
    uint32_t    ts_nano_sec_to_msec;  /* holds nano seconds value in milliseconds units */
    uint32_t    uptime;               /* holds uptime which is the time (in ms) since system boot */
    uint8_t     tai_sel;              /* TAI select index 0 or 1 */

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr46_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Get ptp TAI IF select */
    __LOG_FW(("Get ptp TAI IF select"));
    tai_sel = thr46_desc_ptr->phal2ppa.ptp_tai_select;
    __LOG_PARAM_FW(tai_sel);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Shift packet left to make room for sFlow header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Move left IPv4(20B) + UDP(8B) by 28 bytes to make room for sFlow header */
    __LOG_FW(("Move left IPv4(20B) + UDP(8B) by 28 bytes to make room for sFlow header"));
    ACCEL_CMD_TRIG(SHIFTLEFT_28_BYTES_THR46_SFLOW_IPv4_LEN28_FROM_PKT_IPv4_Header__version_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow header. Each entry is 32 bits word.
      - | version(4B) | agent_ip_version(4B) | agent_ip_address(4B) | sub_agent_id(4B) | sequence_number(4B) | uptime(4B) | samples_number(4B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow header. |version(4B)|agent_ip_version(4B)|agent_ip_address(4B)|sub_agent_id(4B)|sequence_number(4B)|uptime(4B)|samples_number(4B)| "));


    /* Set sFlow.version to fixed value 0x5 */
    __LOG_FW(("Set sFlow.version to fixed value 0x5"));
    ACCEL_CMD_TRIG(COPY_BITS_THR46_SFLOW_IPv4_CONST_0x5_LEN3_TO_PKT_sflow_ipv4_header__version_PLUS29_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Set sFlow.agent_ip_version to 0x1 to indicate IPv4 */
    __LOG_FW(("Set sFlow.agent_ip_version to 0x1 to indicate IPv4"));
    ACCEL_CMD_TRIG(COPY_BITS_THR46_SFLOW_IPv4_CONST_0x1_LEN1_TO_PKT_sflow_ipv4_header__agent_ip_version_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Copy Agent IPv4 address from template */
    __LOG_FW(("Copy Agent IPv4 address from template"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR46_SFLOW_IPv4_CFG_sflow_ipv4_template__agent_ip_address_LEN4_TO_PKT_sflow_ipv4_header__agent_ip_address_OFFSET, 0);

    /* Set sub Agent ID. Take it from pha metadata assigned by EPCL action */
    __LOG_FW(("Set sub Agent ID . Take it from pha metadata assigned by EPCL action"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR46_SFLOW_IPv4_DESC_phal2ppa__pha_metadata_LEN4_TO_PKT_sflow_ipv4_header__sub_agent_id_OFFSET, 0);

    /* Set sequence number. Take it from Desc<LM counter> */
    __LOG_FW(("Set sequence number. Take it from Desc<LM counter>"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR46_SFLOW_IPv4_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_sflow_ipv4_header__sequence_number_OFFSET, 0);

    /* Set number of samples to fixed value '1' */
    __LOG_FW(("Set number of samples to fixed value '1'"));
    ACCEL_CMD_TRIG(COPY_BITS_THR46_SFLOW_IPv4_CONST_0x1_LEN1_TO_PKT_sflow_ipv4_header__samples_number_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IP length field:                         IPv4 Header<Total Length>= IPv4 Header<Total Length> + 28
      - Calculate IPv4 checksum (incremental update):   New checksum = ~( ~(old checksum) + ~(old Total Length) + (new Total Length) )
      - Update UDP length field:                        UDP<length> = UDP<length> + 28
      - Update HW that packet was increased by 28B      Desc<fw_bc_modification> = 28
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IP length(+28B), calculate new IPv4 CS, update udp length(+28B) and update HW packet was increased by 28B"));

    /* Sum old IP Total Length */
    __LOG_FW(("Sum old IP Total Length using accelerator cmd"));
    ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR46_SFLOW_IPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);
	__PRAGMA_NO_REORDER
    /* Update new IP header total length: IPv4<Total Length> + 28 */
    __LOG_FW(("Update new IP header total length: IPv4<Total Length> + 28"));
    ACCEL_CMD_TRIG(ADD_BITS_THR46_SFLOW_IPv4_CONST_0x1C_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);
	__PRAGMA_NO_REORDER
    /* Sum new Total Length */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR46_SFLOW_IPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);
    /* Sum old IPv4 CS */
    __LOG_FW(("Sum old IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR46_SFLOW_IPv4_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, 0);
	__PRAGMA_NO_REORDER
    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR46_SFLOW_IPv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);

    /* Update new UDP length. UDP<length> + 28 */
    __LOG_FW(("Update new UDP length. UDP<length> + 28"));
    ACCEL_CMD_TRIG(ADD_BITS_THR46_SFLOW_IPv4_CONST_0x1C_LEN16_TO_PKT_udp_header__Length_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+28 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+28 bytes)."));
    thr46_desc_ptr->ppa_internal_desc.fw_bc_modification = 28;


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow.Uptime in milliseconds according to TAI.sec and TAI.ns
      - The “uptime” is the time (in ms) since system boot.
      - In our case we have 18 bits of seconds and 30 bits of ns that is ~3 days
      - Since 18b for seconds we assume that multiplication of 1000 to convert to milliseconds should not be bigger than 32b size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow.Uptime in milliseconds according to TAI.sec and TAI.ns "));

    /* Read TOD word 1 which represents seconds field [31:0] */
    __LOG_FW(("Read TOD word 1 which represents seconds field [31:0]"));
    ts_sec = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec);
    /* Read TOD word 0 which represents nano seconds field */
    __LOG_FW(("Read TOD word 0 which represents nano seconds field"));
    ts_nano_sec = READ_TOD_IF_REG(TOD_WORD_0, tai_sel);
    __LOG_PARAM_FW(ts_nano_sec);
	__PRAGMA_NO_REORDER

    /* Check for wraparound */
    __LOG_FW(("Read again seconds (TOD word 1) to check for wraparound"));
    ts_sec_new = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec_new);
    /* < 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound */
    __LOG_FW(("< 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound"));
    if ( (ts_sec_new != ts_sec) && (ts_nano_sec < 500000000) )
    {
        __LOG_FW(("Update ts_sec with new value since wraparound occurred"));
        ts_sec = ts_sec_new ;
        __LOG_PARAM_FW(ts_sec);
    }

    /* Convert seconds to milliseconds units */
    __LOG_FW(("Convert seconds to milliseconds units"));
    ts_sec_to_msec = ts_sec * 1000 ;

    /* Convert nano seconds to milliseconds units */
    __LOG_FW(("Convert nano seconds to milliseconds units"));
    ts_nano_sec_to_msec = ts_nano_sec / 1000000 ;

    /* Calculate uptime in milliseconds */
    __LOG_FW(("Calculate uptime in milliseconds"));
    uptime = ts_sec_to_msec + ts_nano_sec_to_msec ;

    /* Set uptime in sFlow header */
    __LOG_FW(("Set uptime in sFlow header"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 84 , uptime );


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(46);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR47_SFLOW_IPv6
 * inparam      None
 * return       None
 * description  sFlow is a technology for monitoring traffic in data networks containing switches and routers.
 *              It requires ingress/egress mirrored packets to be sent over an IPv6-UDP tunnel to a “collector” in the network.
 *              Following the UDP header there is SFLOW header which is followed by the mirrored packet as the payload.
 *              The L2-IPv6-UDP tunnel encapsulation is added by the Falcon Tunnel-start in the HA unit.
 *              The SFLOW header is inserted by the PHA firmware after the UDP header.
 *
 *              cfg template: holds IPv6 agent address (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:    | Extension space (64B) | IPv6(40B) | UDP(8B) | payload |
 *              hdr out:   | Extension space (24B) | IPv6(40B) | UDP(8B) | sFlow header(40B) | payload |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmwares adds sFlow header of 40 bytes size and sets it according to architect's definitions.
 *              Updates HW that 40 bytes were added
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR47_SFLOW_IPv6)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    ts_sec;               /* timestamp in seconds */
    uint32_t    ts_nano_sec;          /* timestamp in nano seconds */
    uint32_t    ts_sec_new;           /* holds timestamp in seconds after reading it for the second time to check wraparound */
    uint32_t    ts_sec_to_msec;       /* holds seconds value in milliseconds units */
    uint32_t    ts_nano_sec_to_msec;  /* holds nano seconds value in milliseconds units */
    uint32_t    uptime;               /* holds uptime which is the time (in ms) since system boot */
    uint8_t     tai_sel;              /* TAI select index 0 or 1 */

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr47_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Get ptp TAI IF select */
    __LOG_FW(("Get ptp TAI IF select"));
    tai_sel = thr47_desc_ptr->phal2ppa.ptp_tai_select;
    __LOG_PARAM_FW(tai_sel);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Shift packet left to make room for sFlow header
      - Move left IPv6(40B) + UDP(8B) by 40 bytes.
      - Need to do it with 2 commands since maximum bytes to copy is 32 bytes
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Move left IPv4(40B) + UDP(8B) by 40 bytes to make room for sFlow header"));

    /* First, move left 32 bytes of IPv6 by 40 bytes */
    __LOG_FW(("First, move left 32 bytes of IPv6 by 40 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR47_SFLOW_IPv6_LEN32_FROM_PKT_IPv6_Header__version_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Now, move left the last 8 bytes of IPv6 + 8 bytes of UDP header by 40 bytes */
    __LOG_FW(("Now, move left the last 8 bytes of IPv6 + 8 bytes of UDP header by 40 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR47_SFLOW_IPv6_LEN16_FROM_PKT_IPv6_Header__version_PLUS32_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow header. Each entry is 32 bits word except for agent IPv6 address which is 16 bytes
      - | version(4B) | agent_ip_version(4B) | agent_ip_address(16B) | sub_agent_id(4B) | sequence_number(4B) | uptime(4B) | samples_number(4B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow header. |version(4B)|agent_ip_version(16B)|agent_ip_address(4B)|sub_agent_id(4B)|sequence_number(4B)|uptime(4B)|samples_number(4B)| "));


    /* Set sFlow.version to fixed value 0x5 */
    __LOG_FW(("Set sFlow.version to fixed value 0x5"));
    ACCEL_CMD_TRIG(COPY_BITS_THR47_SFLOW_IPv6_CONST_0x5_LEN3_TO_PKT_sflow_ipv6_header__version_PLUS29_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Set sFlow.agent_ip_version to 0x2 to indicate IPv6 */
    __LOG_FW(("Set sFlow.agent_ip_version to 0x2 to indicate IPv6"));
    ACCEL_CMD_TRIG(COPY_BITS_THR47_SFLOW_IPv6_CONST_0x2_LEN2_TO_PKT_sflow_ipv6_header__agent_ip_version_PLUS30_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Copy Agent IPv6 address from template */
    __LOG_FW(("Copy Agent IPv6 address from template"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR47_SFLOW_IPv6_CFG_sflow_ipv6_template__agent_ip_address_127_96_LEN16_TO_PKT_sflow_ipv6_header__agent_ip_address_127_96_OFFSET, 0);

    /* Set sub Agent ID. Take it from pha metadata assigned by EPCL action */
    __LOG_FW(("Set sub Agent ID . Take it from pha metadata assigned by EPCL action"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR47_SFLOW_IPv6_DESC_phal2ppa__pha_metadata_LEN4_TO_PKT_sflow_ipv6_header__sub_agent_id_OFFSET, 0);

    /* Set sequence number. Take it from Desc<LM counter> */
    __LOG_FW(("Set sequence number. Take it from Desc<LM counter>"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR47_SFLOW_IPv6_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_sflow_ipv6_header__sequence_number_OFFSET, 0);

    /* Set number of samples to fixed value '1' */
    __LOG_FW(("Set number of samples to fixed value '1'"));
    ACCEL_CMD_TRIG(COPY_BITS_THR47_SFLOW_IPv6_CONST_0x1_LEN1_TO_PKT_sflow_ipv6_header__samples_number_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IP length field:                     IPv6 Header<Payload Length>= IPv6 Header<Payload Length> + 40
      - Update UDP length field:                    UDP<length> = UDP<length> + 40
      - Update HW that packet was increased by 40B  Desc<fw_bc_modification> = 40
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IPv6 length(+40B), update udp length(+40B) and update HW packet was increased by 40B "));

    /* Update new IP header payload length: IPv6<Payload Length> + 40 */
    __LOG_FW(("Update new IP header payload length: IPv6<Payload Length> + 40"));
    ACCEL_CMD_TRIG(ADD_BITS_THR47_SFLOW_IPv6_CONST_0x28_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);

    /* Update new UDP length. UDP<length> + 40 */
    __LOG_FW(("Update new UDP length. UDP<length> + 40"));
    ACCEL_CMD_TRIG(ADD_BITS_THR47_SFLOW_IPv6_CONST_0x28_LEN16_TO_PKT_udp_header__Length_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+40 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+40 bytes)."));
    thr47_desc_ptr->ppa_internal_desc.fw_bc_modification = 40;


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow.Uptime in milliseconds according to TAI.sec and TAI.ns
      - The “uptime” is the time (in ms) since system boot.
      - In our case we have 18 bits of seconds and 30 bits of ns that is ~3 days
      - Since 18b for seconds we assume that multiplication of 1000 to convert to milliseconds should not be bigger than 32b size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow.Uptime in milliseconds according to TAI.sec and TAI.ns "));


    /* Read TOD word 1 which represents seconds field [31:0] */
    __LOG_FW(("Read TOD word 1 which represents seconds field [31:0]"));
    ts_sec = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec);
    /* Read TOD word 0 which represents nano seconds field */
    __LOG_FW(("Read TOD word 0 which represents nano seconds field"));
    ts_nano_sec = READ_TOD_IF_REG(TOD_WORD_0, tai_sel);
    __LOG_PARAM_FW(ts_nano_sec);
	__PRAGMA_NO_REORDER

    /* Check for wraparound */
    __LOG_FW(("Read again seconds (TOD word 1) to check for wraparound"));
    ts_sec_new = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec_new);
    /* < 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound */
    __LOG_FW(("< 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound"));
    if ( (ts_sec_new != ts_sec) && (ts_nano_sec < 500000000) )
    {
        __LOG_FW(("Update ts_sec with new value since wraparound occurred"));
        ts_sec = ts_sec_new ;
        __LOG_PARAM_FW(ts_sec);
    }

    /* Convert seconds to milliseconds units */
    __LOG_FW(("Convert seconds to milliseconds units"));
    ts_sec_to_msec = ts_sec * 1000 ;

    /* Convert nano seconds to milliseconds units */
    __LOG_FW(("Convert nano seconds to milliseconds units"));
    ts_nano_sec_to_msec = ts_nano_sec / 1000000 ;

    /* Calculate uptime in milliseconds */
    __LOG_FW(("Calculate uptime in milliseconds"));
    uptime = ts_sec_to_msec + ts_nano_sec_to_msec ;

    /* Set uptime in sFlow header */
    __LOG_FW(("Set uptime in sFlow header"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 104 , uptime );


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(47);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR48_SRV6_Best_Effort
 * inparam      None
 * return       None
 * description  This thread is used for SRv6 BE (Best Effort) tunnel type.
 *              It has only IPv6 encapsulation without SRH extension header.
 *              Note: this thread is dedicated for H3C only !!!
 *
 *              cfg template: Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:    | Extension space (32B) | IPv6(40B) | payload |
 *              hdr out:   | Extension space (32B) | IPv6(40B) | payload |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              No reduction or addition of bytes only updates of IPv6.DIP as follows:
 *              IPv6.DIP before change:
 *                  | Locator (80b) | reserved 16b | source id last bit (8b) | source id first bit (8b) | Function: service id (16b) |
 *              IPv6.DIP after change:
 *                  | Locator (80b) | Function: service id (16b) | SGT ID (16b) | APP ID (10b) | zero (6b) |
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR48_SRV6_Best_Effort)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    uint8_t     sourceIdLastBit;      /* Holds the last bit location of source id value in Desc<sst id> field */
    uint8_t     sourceIdFirstBit;     /* Holds the first bit location of source id value in Desc<sst id> field */
    uint8_t     sourceIdLength;       /* Holds the number of bits of source id value from Desc<sst id> field */
    uint32_t    sourceIdMask;         /* Holds the mask bits that is used to extract source id value from Desc<sst id> field */
    uint16_t    sourceIdValue;        /* Holds the value of source id taken from Desc<sst id> field */

    /* Get pointer to descriptor and input packet */
    struct ppa_in_desc* thr48_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr48_srv6_best_effort_in_hdr* thr48_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr48_srv6_best_effort_in_hdr);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Get SGT-ID value
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get SGT-ID value"));

    /* Extract the First-Bit (IPv6.DIP[23:16]) and Last-Bit (IPv6.DIP[31:24]) of the location of Source-ID value in 12b Desc<sst_id> */
    __LOG_FW(("Extract the First-Bit (IPv6.DIP[23:16]) and Last-Bit (IPv6.DIP[31:24]) of the location of Source-ID value in 12b Desc<sst_id>"));
    sourceIdLastBit =  (thr48_in_hdr_ptr->IPv6_Header.dip3_high >> 8) & 0xFF;
    sourceIdFirstBit = thr48_in_hdr_ptr->IPv6_Header.dip3_high & 0xFF;
    __LOG_PARAM_FW(thr48_in_hdr_ptr->IPv6_Header.dip3_high);
    __LOG_PARAM_FW(sourceIdLastBit);
    __LOG_PARAM_FW(sourceIdFirstBit);

    /* Get length of Source-ID value in Desc<sst_id> */
    __LOG_FW(("Get length of Source-ID value in Desc<sst_id>"));
    sourceIdLength = (sourceIdLastBit - sourceIdFirstBit) + 1;
   __LOG_PARAM_FW(sourceIdLength);

    /* Get Source ID mask compatible to number of bits */
    __LOG_FW(("Get Source ID mask compatible to number of bits"));
    sourceIdMask = (1 << sourceIdLength) - 1 ;
    __LOG_PARAM_FW(sourceIdMask);

    /* Extract valid Source ID value from Desc<sst-id> */
      __LOG_FW(("Extract valid Source ID value from Desc<sst-id>"));
    sourceIdValue = ( (thr48_desc_ptr->phal2ppa.sst_id) >> sourceIdFirstBit ) & sourceIdMask;
    __LOG_PARAM_FW(thr48_desc_ptr->phal2ppa.sst_id);
    __LOG_PARAM_FW(sourceIdValue);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set Function value
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set Function value"));

    /* Copy the service-ID from IPv6.DIP[15:0] to Function location in IPv6.DIP[47:32] */
    __LOG_FW(("Copy the service-ID from IPv6.DIP[15:0] to Function location in IPv6.DIP[47:32]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR48_SRV6_Best_Effort_PKT_IPv6_Header__dip3_low_LEN2_TO_PKT_IPv6_Header__dip2_low_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set APP-ID + reserved value
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set APP-ID + reserved value "));

    /* Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6] */
    __LOG_FW(("Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6]"));
    ACCEL_CMD_TRIG(COPY_BITS_THR48_SRV6_Best_Effort_DESC_phal2ppa__copy_reserved_PLUS8_LEN10_TO_PKT_IPv6_Header__dip3_low_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set SGT-ID value
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set SGT-ID value"));

    /* Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)
       SGT ID location in packet: 32B (extension space) + 36B(IPv6 till DIP 24B + Locator 80b + Function 16b) */
    __LOG_FW(("Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)"));
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 32 + 36) , sourceIdValue );


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(48);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR49_SRV6_Source_Node_1_CONTAINER
 * inparam      None
 * return       None
 * description  Source node single pass processing with 1 SRH Container adding SRH with 1 x 128b Container (4 x 32b G-SIDs)
 *              Container content is assigned by the IPv6 Tunnel-Start <SIP> and is copied by fw to the container location
 *              The actual SIP address is assigned by the Thread template.
 *              Note: this thread is dedicated for H3C only !!!
 *
 *              cfg template: 1 x 128b Container (4 x 32b G-SIDs)  (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:    | Extension space (32B) | IPv6(40B) | payload |
 *              hdr out:   | Extension space (8B)  | IPv6(40B) | SRH(8B)| Container(16B) | payload |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmware actions:
 *              - add 24 bytes for SRH(8B) + Container(16B)
 *              - copy packet IPv6 SIP to Container
 *              - update IPv6<Payload Length> += 24
 *              - update packet IPv6 SIP with new value taken from the thread's template
 *              - set SRH header with specific values defined by architect's definitions
 *              - update IPv6.DIP with SGT-ID and APP-ID values according to architect's definitions
 *              - update HW that 24 bytes are added
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR49_SRV6_Source_Node_1_CONTAINER)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint8_t     sourceIdLastBit;      /* Holds the last bit location of source id value in Desc<sst id> field */
    uint8_t     sourceIdFirstBit;     /* Holds the first bit location of source id value in Desc<sst id> field */
    uint8_t     sourceIdLength;       /* Holds the number of bits of source id value from Desc<sst id> field */
    uint32_t    sourceIdMask;         /* Holds the mask bits that is used to extract source id value from Desc<sst id> field */
    uint16_t    sourceIdValue;        /* Holds the value of source id taken from Desc<sst id> field */

    /* Get pointer to descriptor and input packet */
    struct ppa_in_desc* thr49_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr49_srv6_source_node_1_container_in_hdr* thr49_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr49_srv6_source_node_1_container_in_hdr);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set SGT-ID value in IPv6.DIP
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get SGT-ID value"));

    /* Extract the First-Bit (IPv6.DIP[23:16]) and Last-Bit (IPv6.DIP[31:24]) of the location of Source-ID value in 12b Desc<sst_id> */
    __LOG_FW(("Extract the First-Bit (IPv6.DIP[23:16]) and Last-Bit (IPv6.DIP[31:24]) of the location of Source-ID value in 12b Desc<sst_id>"));
    sourceIdLastBit =  (thr49_in_hdr_ptr->IPv6_Header.dip3_high >> 8) & 0xFF;
    sourceIdFirstBit = thr49_in_hdr_ptr->IPv6_Header.dip3_high & 0xFF;
    __LOG_PARAM_FW(thr49_in_hdr_ptr->IPv6_Header.dip3_high);
    __LOG_PARAM_FW(sourceIdLastBit);
    __LOG_PARAM_FW(sourceIdFirstBit);

    /* Get length of Source-ID value in Desc<sst_id> */
    __LOG_FW(("Get length of Source-ID value in Desc<sst_id>"));
    sourceIdLength = (sourceIdLastBit - sourceIdFirstBit) + 1;
   __LOG_PARAM_FW(sourceIdLength);

    /* Get Source ID mask compatible to number of bits */
    __LOG_FW(("Get Source ID mask compatible to number of bits"));
    sourceIdMask = (1 << sourceIdLength) - 1 ;
    __LOG_PARAM_FW(sourceIdMask);

    /* Extract valid Source ID value from Desc<sst-id> */
      __LOG_FW(("Extract valid Source ID value from Desc<sst-id>"));
    sourceIdValue = ( (thr49_desc_ptr->phal2ppa.sst_id) >> sourceIdFirstBit ) & sourceIdMask;
    __LOG_PARAM_FW(thr49_desc_ptr->phal2ppa.sst_id);
    __LOG_PARAM_FW(sourceIdValue);

    /* Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)
       SGT ID location in packet: 32B (extension space) + 36B(IPv6 till DIP 24B + Locator 80b + Function 16b) */
    __LOG_FW(("Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)"));
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 32 + 36) , sourceIdValue );
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move IPv6 header left by 24 bytes to make room for SRH(8B) + 1 container(16B)
      - Need to do it in 2 commands since need to move more than 32B (IPv6 is 40B)
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Move left the first 32 bytes of IPv6 */
    __LOG_FW(("Move left the first 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR49_SRV6_Source_Node_1_CONTAINER_LEN32_FROM_PKT_IPv6_Header__version_OFFSET, 0);
	__PRAGMA_NO_REORDER
    /* Move left the last 8 bytes of IPv6 */
    __LOG_FW(("Move left the last 8 bytes of IPv6"));
    ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR49_SRV6_Source_Node_1_CONTAINER_LEN8_FROM_PKT_IPv6_Header__version_PLUS32_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set APP-ID value in IPv6.DIP
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set APP-ID"));

    /* Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6] */
    __LOG_FW(("Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6]"));
    ACCEL_CMD_TRIG(COPY_BITS_THR49_SRV6_Source_Node_1_CONTAINER_DESC_phal2ppa__copy_reserved_PLUS8_LEN10_TO_PKT_IPv6_Header__dip3_low_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update Container content with origin IPv6 SIP
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy IPv6.SIP to Container 0 */
    __LOG_FW(("Copy IPv6.SIP to Container 0"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR49_SRV6_Source_Node_1_CONTAINER_PKT_IPv6_Header__sip0_high_MINUS24_LEN16_TO_PKT_SRv6_Segment0_Header__dip0_high_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update SR Header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Use max constant 11 bits command to set SRH<Routing Type>=4 and SRH<Segments Left>=1. Clear rest bit to zero. */
    __LOG_FW(("Use max constant 11 bits command to set SRH<Routing Type>=4 and SRH<Segments Left>=1. Clear rest bit to zero."));
    ACCEL_CMD_TRIG(COPY_BITS_THR49_SRV6_Source_Node_1_CONTAINER_CONST_0x401_LEN11_TO_PKT_SRv6_Header__routing_type_PLUS5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* SRH<Next Header>= IPv6<Next Header> Can not use copy bits command to copy bits from pkt to pkt therefore use copy bytes which must copy minimum 2B
       therefore copy also IPv6<hope_limit> to SRH<hdr_ext_len> which will later be overrun with correct value */
    __LOG_FW(("SRH<Next Header>= IPv6<Next Header> Can not use copy bits command to copy bits from pkt to pkt therefore use copy bytes which must copy minimum 2B"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR49_SRV6_Source_Node_1_CONTAINER_PKT_IPv6_Header__next_header_MINUS24_LEN2_TO_PKT_SRv6_Header__next_header_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* SRH<Hdr Ext Len>=2 */
    __LOG_FW(("SRH<Hdr Ext Len>=2"));
    ACCEL_CMD_TRIG(COPY_BITS_THR49_SRV6_Source_Node_1_CONTAINER_CONST_0x2_LEN8_TO_PKT_SRv6_Header__hdr_ext_len_OFFSET, 0);

    /* SRH<Last Entry>=0, SRH<Flags>=0, SRH<Tag>=0 (clear 4 LS bytes to zero) */
    __LOG_FW(("SRH<Last Entry>=0, SRH<Flags>=0, SRH<Tag>=0 (clear 4 LS bytes to zero) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR49_SRV6_Source_Node_1_CONTAINER_CONST_0x0_LEN8_TO_PKT_SRv6_Header__last_entry_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update origin IPv6 header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy Thread<Template> to IPv6 SIP */
    __LOG_FW(("Copy Thread<Template> to IPv6 SIP using accelerator cmd"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR49_SRV6_Source_Node_1_CONTAINER_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET, 0);

    /* Increment IP header payload length field by 24B.  IPv6 Header<payload length>= IPv6 Header<payload Length> + 24 */
    __LOG_FW(("Increment IP header payload length field by 24B.  IPv6 Header<payload length>= IPv6 Header<payload Length> + 24 using accelerator cmd"));
    ACCEL_CMD_TRIG(ADD_BITS_THR49_SRV6_Source_Node_1_CONTAINER_CONST_0x18_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);

    /* IPv6<Next Header>=43 (0x2B) */
    __LOG_FW(("IPv6<Next Header>=43 (0x2B) using accelerator cmd"));
    ACCEL_CMD_TRIG(COPY_BITS_THR49_SRV6_Source_Node_1_CONTAINER_CONST_0x2B_LEN8_TO_PKT_IPv6_Header__next_header_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Updates Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). In this case plus 24 bytes */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement). In this case plus 24 bytes"));
    ACCEL_CMD_TRIG(COPY_BITS_THR49_SRV6_Source_Node_1_CONTAINER_CONST_0x18_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(49);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR50_SRV6_Source_Node_First_Pass_1_CONTAINER
 * inparam      None
 * return       None
 * description  Part of double pass processing with SRH up to 3 x 128b Containers (12 x 32b G-SIDs)
 *              This thread handles source node of first pass processing in which 1 container is being added
 *              without the SRH (will be added in second pass).
 *              Container content is assigned by the IPv6 Tunnel-Start <SIP> and is copied by fw to the container location
 *              The actual SIP address is assigned by the Thread template.
 *              Note: this thread is dedicated for H3C only !!!
 *
 *              cfg template: 1 x 128b Container (4 x 32b G-SIDs)  (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:    | Extension space (32B) | L2 header(12B) | eDSA(16B) | ET(2B) | IPv6(40B) | payload |
 *              hdr out:   | Extension space (16B) | L2 header(12B) | eDSA(16B) | ET(2B) | IPv6(40B) | Container2(16B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmware actions:
 *              - adds 16 bytes for Container 2 (deepest container)
 *              - copy packet IPv6 SIP into Container 2 location
 *              - updates packet IPv6 SIP with new value taken from the thread's template
 *              - eDSA.Trg ePort += 1
 *              - eDSA.Is Trg Phy Port Valid = 0
 *              - updates HW that 16 bytes are added
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR50_SRV6_Source_Node_First_Pass_1_CONTAINER)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move L2,eDSA,ET and IPv6 headers left by 16 bytes to make room for Container2(16B)
      - Need to do it in 3 commands since need to move more than 32B (L2(12B) + eDSA(16B) + ET(2B) + IPv6(40B) is 70B)
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Move left the first 32 bytes */
    __LOG_FW(("Move left the first 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_16_BYTES_THR50_SRV6_Source_Node_First_Pass_1_CONTAINER_LEN32_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
	__PRAGMA_NO_REORDER
    /* Move the next 32 bytes */
    __LOG_FW(("Move left the next 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_16_BYTES_THR50_SRV6_Source_Node_First_Pass_1_CONTAINER_LEN32_FROM_PKT_mac_header__mac_da_47_32_PLUS32_OFFSET, 0);
	__PRAGMA_NO_REORDER
    /* Move the last 6 bytes */
    __LOG_FW(("Move left the last 6 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_16_BYTES_THR50_SRV6_Source_Node_First_Pass_1_CONTAINER_LEN6_FROM_PKT_mac_header__mac_da_47_32_PLUS64_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update Container 2 with packet IPv6 SIP
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy IPv6 SIP to Container2 (MINUS16 in command since we already moved the packet left) */
    __LOG_FW(("Copy IPv6 SIP to Container2 (MINUS16 in command since we already moved the packet left)"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR50_SRV6_Source_Node_First_Pass_1_CONTAINER_PKT_IPv6_Header__sip0_high_MINUS16_LEN16_TO_PKT_SRv6_Segment2_Header__dip0_high_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update packet IPv6 header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy CFG<Template> to IPv6 SIP */
    __LOG_FW(("Copy CFG<Template> to IPv6 SIP"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR50_SRV6_Source_Node_First_Pass_1_CONTAINER_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update eDSA tag
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* eDSA.Trg ePort += 1 */
    __LOG_FW(("eDSA.Trg ePort += 1"));
    ACCEL_CMD_TRIG(ADD_BITS_THR50_SRV6_Source_Node_First_Pass_1_CONTAINER_CONST_0x1_LEN16_TO_PKT_eDSA_fwd_w3__Trg_ePort_15_0_OFFSET, 0);
    /* eDSA<Is Trg Phy Port Valid>=0 */
    __LOG_FW(("eDSA<Is Trg Phy Port Valid>=0 using accelerator cmd"));
    ACCEL_CMD_TRIG(COPY_BITS_THR50_SRV6_Source_Node_First_Pass_1_CONTAINER_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w2__IsTrgPhyPortValid_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Updates Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). In this case plus 16 bytes */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement). In this case plus 16 bytes"));
    ACCEL_CMD_TRIG(COPY_BITS_THR50_SRV6_Source_Node_First_Pass_1_CONTAINER_CONST_0x10_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(50);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER
 * inparam      None
 * return       None
 * description  Part of double pass processing with SRH up to 3 x 128b Containers (12 x 32b G-SIDs)
 *              This thread handles source node of second pass processing in which SRH and 2 containers are added
 *              on top of the already existing container from first pass establishing 3 containers total.
 *              SRH and the 2 containers are set using the Generic TS Data.
 *              Note: this thread is dedicated for H3C only !!!
 *
 *              cfg template: Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),Generic TS(54B),L2(14B or 18B),IPv6(40B),Container2(16B)
 *              hdr out: Extension space(62B),L2(14B or 18B),IPv6(40B),SRH(8B),Container0(16B),Container1(16B),Container2(16B)
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 112 bytes + 48 bytes Header increment space
 *
 *              Firmware actions:
 *              - Adds 40 bytes for SRH and 2 containers
 *              - Updates IPv6 header (IPv6BaseHeader<Payload Length>+= 56, IPv6<Next Header>=43, SGT-ID & APP-ID)
 *              - Set SRH and containers content from Generic TS Data
 *              - Updates HW that 14 bytes are removed
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint8_t  ofstToVlanAcc;           /* Holds offset to the right accelerator command. 0:no vlan, 4:with vlan */
    uint8_t  ipv6NextHeader;          /* Holds IPv6 next header field value */
    uint16_t geneircTsDataEthertype;  /* Holds Generic TS Data Ethertype field value */
    uint8_t  geneircTsDataVid;        /* Holds Generic TS Data Vlan ID LS byte value */
    uint8_t  sourceIdLastBit;         /* Holds the last bit location of source id value in Desc<sst id> field */
    uint8_t  sourceIdFirstBit;        /* Holds the first bit location of source id value in Desc<sst id> field */
    uint8_t  sourceIdLength;          /* Holds the number of bits of source id value from Desc<sst id> field */
    uint32_t sourceIdMask;            /* Holds the mask bits that is used to extract source id value from Desc<sst id> field */
    uint16_t sourceIdValue;           /* Holds the value of source id taken from Desc<sst id> field */

    /* Get pointers to descriptor */
    struct ppa_in_desc* thr51_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


    /* Update offset in order to trigger the right accelerators which take into account addition or no addition of 4B vlan */
    __LOG_FW(("Update offset in order to trigger the right accelerators which take into account addition or no addition of 4B vlan"));
    ofstToVlanAcc = (thr51_desc_ptr->phal2ppa.egress_tag_state)<<2;
    __LOG_PARAM_FW(thr51_desc_ptr->phal2ppa.egress_tag_state);
    __LOG_PARAM_FW(ofstToVlanAcc);
    __PRAGMA_NO_REORDER

    /* Get Generic_TS_Data.vid[7:0] */
    __LOG_FW(("Get Generic_TS_Data.vid[7:0]"));
    geneircTsDataVid = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 63);
    __LOG_PARAM_FW(geneircTsDataVid);

    /* Get Generic_TS_Data.ethertype  */
    __LOG_FW(("Get Generic_TS_Data.ethertype"));
    geneircTsDataEthertype = PPA_FW_SP_SHORT_READ(FALCON_PKT_REGs_lo + 64);
    __LOG_PARAM_FW(geneircTsDataEthertype);

    /* Get IPv6.Next Header */
    __LOG_FW(("Get IPv6.Next Header"));
    ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122 + ofstToVlanAcc);
    __LOG_PARAM_FW(ipv6NextHeader);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Get SGT-ID value
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get SGT-ID value"));

    /* Extract the First-Bit (IPv6.DIP[23:16]) and Last-Bit (IPv6.DIP[31:24]) of the location of Source-ID value in 12b Desc<sst_id> */
    __LOG_FW(("Extract the First-Bit (IPv6.DIP[23:16]) and Last-Bit (IPv6.DIP[31:24]) of the location of Source-ID value in 12b Desc<sst_id>"));
    sourceIdLastBit = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 152 + ofstToVlanAcc);
    sourceIdFirstBit = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 153 + ofstToVlanAcc);
    __LOG_PARAM_FW(sourceIdLastBit);
    __LOG_PARAM_FW(sourceIdFirstBit);
    __PRAGMA_NO_REORDER

    /* Get length of Source-ID value in Desc<sst_id> */
    __LOG_FW(("Get length of Source-ID value in Desc<sst_id>"));
    sourceIdLength = (sourceIdLastBit - sourceIdFirstBit) + 1;
    __LOG_PARAM_FW(sourceIdLength);

    /* Get Source ID mask compatible to number of bits */
    __LOG_FW(("Get Source ID mask compatible to number of bits"));
    sourceIdMask = (1 << sourceIdLength) - 1 ;
    __LOG_PARAM_FW(sourceIdMask);

    /* Extract valid Source ID value from Desc<sst-id> */
    __LOG_FW(("Extract valid Source ID value from Desc<sst-id>"));
    sourceIdValue = ( (thr51_desc_ptr->phal2ppa.sst_id) >> sourceIdFirstBit ) & sourceIdMask;
    __LOG_PARAM_FW(thr51_desc_ptr->phal2ppa.sst_id);
    __LOG_PARAM_FW(sourceIdValue);
    __PRAGMA_NO_REORDER


    /* Check if packet is with or without vlan */
    __LOG_FW(("Check if packet is with or without vlan"));
    if(ofstToVlanAcc == 0)
    {   /* no vlan */
        __LOG_FW(("no vlan"));

        /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

       /* Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_CONST_0xF2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Move Generic TS(54B),L2(14B) and IPv6(40B) headers left by 40 bytes to make room for SRH(8B) + Container0(16B) + Container1(16B)
          - Need to do it in 4 commands since need to move more than 32B (108B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 12 bytes */
        __LOG_FW(("Move left the next 12 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case the location of some of the packet's fields exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 40B which means that the pkt src and dst will be relative to 8B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 8);
		__PRAGMA_NO_REORDER


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 116, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 117, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 119, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET, 0);


	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_CONST_0x38_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET, 0);

        /* Set APP-ID value in IPv6.DIP:  Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6] */
        __LOG_FW(("Set APP-ID value in IPv6.DIP:  Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6]"));
        ACCEL_CMD_TRIG(COPY_BITS_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_DESC_phal2ppa__copy_reserved_PLUS8_LEN10_TO_PKT_no_vlan__IPv6_Header__dip3_low_OFFSET, 0);

        /* Set SGT-ID value in IPv6.DIP: Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)
           SGT ID location in packet: 32B (extension space) + 36B(IPv6 till DIP 24B + Locator 80b + Function 16b) */
        __LOG_FW(("Set SGT-ID value in IPv6.DIP: Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 112) , sourceIdValue );


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0 and Container1
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* Copy Generic TS< Data Bytes [31:16]> to Container 0
           Copy Generic TS< Data Bytes [15:0]> to Container 1 */
        __LOG_FW(("Copy Generic TS< Data Bytes [31:16]> to Container 0. Copy Generic TS< Data Bytes [15:0]> to Container 1"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
		__PRAGMA_NO_REORDER
     }
     else if(ofstToVlanAcc == 4)
     {  /* with vlan */
        __LOG_FW(("with vlan"));

        /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_CONST_0xF2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);


	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Move Generic TS(54B),L2(14B or 18B) and IPv6(40B) headers left by 40 bytes to make room for SRH(8B) + Container0(16B) + Container1(16B)
          - Need to do it in 4 commands since need to move more than 32B (108B or 112B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 16 bytes */
        __LOG_FW(("Move left the next 16 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case location of last 32B of IPv6 exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 40B which means that the pkt src and dst will be relative to 8B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 8);
		__PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET, 0);
        __PRAGMA_NO_REORDER


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

       /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 116 + 4, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 117 + 4, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 119 + 4, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET, 0);


	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

       /* IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_CONST_0x38_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET, 0);

        /* Set APP-ID value in IPv6.DIP:  Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6] */
        __LOG_FW(("Set APP-ID value in IPv6.DIP:  Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6]"));
        ACCEL_CMD_TRIG(COPY_BITS_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_DESC_phal2ppa__copy_reserved_PLUS8_LEN10_TO_PKT_with_vlan__IPv6_Header__dip3_low_OFFSET, 0);

        /* Set SGT-ID value in IPv6.DIP: Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)
           SGT ID location in packet: 32B (extension space) + 36B(IPv6 till DIP 24B + Locator 80b + Function 16b) */
        __LOG_FW(("Set SGT-ID value in IPv6.DIP: Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 112 + 4) , sourceIdValue );
        __PRAGMA_NO_REORDER


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0 and Container1
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

       /* Copy Generic TS< Data Bytes [31:16]> to Container 0
           Copy Generic TS< Data Bytes [15:0]> to Container 1 */
        __LOG_FW(("Copy Generic TS< Data Bytes [31:16]> to Container 0. Copy Generic TS< Data Bytes [15:0]> to Container 1"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
		__PRAGMA_NO_REORDER
     }
     else
     {  /* not valid: packet should not be changed */
        __LOG_FW(("not valid: packet should not be changed"));

        __LOG_FW((" Desc<Egress Tag state> is not '0' nor '1' "));
        __LOG_PARAM_FW(thr51_desc_ptr->phal2ppa.egress_tag_state);
     }


     /* Set offset register back to zero (default value) */
     __LOG_FW(("Set offset register back to zero (default value)"));
     PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(51);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER
 * inparam      None
 * return       None
 * description  Part of double pass processing with SRH up to 3 x 128b Containers (12 x 32b G-SIDs)
 *              This thread handles source node of second pass processing in which SRH and one container are added
 *              on top of the already existing container from first pass establishing 2 containers total.
 *              SRH and the container are set using the Generic TS Data.
 *              Note: this thread is dedicated for H3C only !!!
 *
 *              cfg template: Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),Generic TS(54B),L2(14B or 18B),IPv6(40B),Container1(16B)
 *              hdr out: Extension space(78B),L2(14B or 18B),IPv6(40B),SRH(8B),Container0(16B),Container1(16B)
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 112 bytes + 48 bytes Header increment space
 *
 *              Firmware actions:
 *              - Adds 24 bytes for SRH and one container
 *              - Updates IPv6 header (IPv6BaseHeader<Payload Length>+= 40, IPv6<Next Header>=43, SGT-ID & APP-ID)
 *              - Set SRH and container content from Generic TS Data
 *              - Updates HW that 30 bytes are removed
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint8_t  ofstToVlanAcc;           /* Holds offset to the right accelerator command. 0:no vlan, 4:with vlan */
    uint8_t  ipv6NextHeader;          /* Holds IPv6 next header field value */
    uint16_t geneircTsDataEthertype;  /* Holds Generic TS Data Ethertype field value */
    uint8_t  geneircTsDataVid;        /* Holds Generic TS Data Vlan ID LS byte value */
    uint8_t  sourceIdLastBit;         /* Holds the last bit location of source id value in Desc<sst id> field */
    uint8_t  sourceIdFirstBit;        /* Holds the first bit location of source id value in Desc<sst id> field */
    uint8_t  sourceIdLength;          /* Holds the number of bits of source id value from Desc<sst id> field */
    uint32_t sourceIdMask;            /* Holds the mask bits that is used to extract source id value from Desc<sst id> field */
    uint16_t sourceIdValue;           /* Holds the value of source id taken from Desc<sst id> field */

    /* Get pointers to descriptor */
    struct ppa_in_desc* thr52_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


    /* Update offset in order to trigger the right accelerators which take into account addition or no addition of 4B vlan */
    __LOG_FW(("Update offset in order to trigger the right accelerators which take into account addition or no addition of 4B vlan"));
    ofstToVlanAcc = (thr52_desc_ptr->phal2ppa.egress_tag_state)<<2;
    __LOG_PARAM_FW(thr52_desc_ptr->phal2ppa.egress_tag_state);
    __LOG_PARAM_FW(ofstToVlanAcc);
    __PRAGMA_NO_REORDER

    /* Get Generic_TS_Data.vid[7:0] */
    __LOG_FW(("Get Generic_TS_Data.vid[7:0]"));
    geneircTsDataVid = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 63);
    __LOG_PARAM_FW(geneircTsDataVid);

    /* Get Generic_TS_Data.ethertype  */
    __LOG_FW(("Get Generic_TS_Data.ethertype"));
    geneircTsDataEthertype = PPA_FW_SP_SHORT_READ(FALCON_PKT_REGs_lo + 64);
    __LOG_PARAM_FW(geneircTsDataEthertype);

    /* Get IPv6.Next Header */
    __LOG_FW(("Get IPv6.Next Header"));
    ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122 + ofstToVlanAcc);
    __LOG_PARAM_FW(ipv6NextHeader);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Get SGT-ID value
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get SGT-ID value"));

    /* Extract the First-Bit (IPv6.DIP[23:16]) and Last-Bit (IPv6.DIP[31:24]) of the location of Source-ID value in 12b Desc<sst_id> */
    __LOG_FW(("Extract the First-Bit (IPv6.DIP[23:16]) and Last-Bit (IPv6.DIP[31:24]) of the location of Source-ID value in 12b Desc<sst_id>"));
    sourceIdLastBit = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 152 + ofstToVlanAcc);
    sourceIdFirstBit = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 153 + ofstToVlanAcc);
    __LOG_PARAM_FW(sourceIdLastBit);
    __LOG_PARAM_FW(sourceIdFirstBit);
    __PRAGMA_NO_REORDER

    /* Get length of Source-ID value in Desc<sst_id> */
    __LOG_FW(("Get length of Source-ID value in Desc<sst_id>"));
    sourceIdLength = (sourceIdLastBit - sourceIdFirstBit) + 1;
    __LOG_PARAM_FW(sourceIdLength);

    /* Get Source ID mask compatible to number of bits */
    __LOG_FW(("Get Source ID mask compatible to number of bits"));
    sourceIdMask = (1 << sourceIdLength) - 1 ;
    __LOG_PARAM_FW(sourceIdMask);

    /* Extract valid Source ID value from Desc<sst-id> */
    __LOG_FW(("Extract valid Source ID value from Desc<sst-id>"));
    sourceIdValue = ( (thr52_desc_ptr->phal2ppa.sst_id) >> sourceIdFirstBit ) & sourceIdMask;
    __LOG_PARAM_FW(thr52_desc_ptr->phal2ppa.sst_id);
    __LOG_PARAM_FW(sourceIdValue);
    __PRAGMA_NO_REORDER


    /* Check if packet is with or without vlan */
    __LOG_FW(("Check if packet is with or without vlan"));
    if(ofstToVlanAcc == 0)
    {   /* no vlan */
        __LOG_FW(("no vlan"));

        /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

       /* Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_CONST_0xE2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Move Generic TS(54B),L2(14B) and IPv6(40B) headers left by 24 bytes to make room for SRH(8B) + Container0(16B)
          - Need to do it in 4 commands since need to move more than 32B (108B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 12 bytes */
        __LOG_FW(("Move left the next 12 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case the location of some of the packet's fields exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 24B which means that the pkt src and dst will be relative to 24B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 24);
		__PRAGMA_NO_REORDER


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 132, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 133, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 135, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET, 0);


	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_CONST_0x28_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET, 0);

        /* Set APP-ID value in IPv6.DIP:  Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6] */
        __LOG_FW(("Set APP-ID value in IPv6.DIP:  Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6]"));
        ACCEL_CMD_TRIG(COPY_BITS_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_DESC_phal2ppa__copy_reserved_PLUS8_LEN10_TO_PKT_no_vlan__IPv6_Header__dip3_low_OFFSET, 0);

        /* Set SGT-ID value in IPv6.DIP: Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)
           SGT ID location in packet: 32B (extension space) + 36B(IPv6 till DIP 24B + Locator 80b + Function 16b) */
        __LOG_FW(("Set SGT-ID value in IPv6.DIP: Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 128) , sourceIdValue );


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* Copy Generic TS< Data Bytes [15:0]> to Container 0 */
        __LOG_FW(("Copy Generic TS< Data Bytes [15:0]> to Container 0"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
		__PRAGMA_NO_REORDER
     }
     else if(ofstToVlanAcc == 4)
     {  /* with vlan */
        __LOG_FW(("with vlan"));

        /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_CONST_0xE2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);


	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Move Generic TS(54B),L2(18B) and IPv6(40B) headers left by 24 bytes to make room for SRH(8B) + Container0(16B)
          - Need to do it in 4 commands since need to move more than 32B (112B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 16 bytes */
        __LOG_FW(("Move left the next 16 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case location of last 32B of IPv6 exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 24B which means that the pkt src and dst will be relative to 24B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 24);
		__PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET, 0);
        __PRAGMA_NO_REORDER


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

       /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 132 + 4, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 133 + 4, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 135 + 4, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET, 0);


	    /*------------------------------------------------------------------------------------------------------------------------------------------------
	      - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

       /* IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_CONST_0x28_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET, 0);

        /* Set APP-ID value in IPv6.DIP:  Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6] */
        __LOG_FW(("Set APP-ID value in IPv6.DIP:  Copy Desc<Copy-Reserved[11:2]> to APP-ID location in IPv6.DIP[15:6]"));
        ACCEL_CMD_TRIG(COPY_BITS_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_DESC_phal2ppa__copy_reserved_PLUS8_LEN10_TO_PKT_with_vlan__IPv6_Header__dip3_low_OFFSET, 0);

        /* Set SGT-ID value in IPv6.DIP: Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)
           SGT ID location in packet: 32B (extension space) + 36B(IPv6 till DIP 24B + Locator 80b + Function 16b) */
        __LOG_FW(("Set SGT-ID value in IPv6.DIP: Copy Source ID value to SGT-ID location in IPv6.DIP[31:16] (was used to store the FirstBit and LastBit)"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 128 + 4) , sourceIdValue );
        __PRAGMA_NO_REORDER


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0
          ------------------------------------------------------------------------------------------------------------------------------------------------*/

       /* Copy Generic TS< Data Bytes [15:0]> to Container 0 */
        __LOG_FW(("Copy Generic TS< Data Bytes [15:0]> to Container 0"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
		__PRAGMA_NO_REORDER
     }
     else
     {  /* not valid: packet should not be changed */
        __LOG_FW(("not valid: packet should not be changed"));

        __LOG_FW((" Desc<Egress Tag state> is not '0' nor '1' "));
        __LOG_PARAM_FW(thr52_desc_ptr->phal2ppa.egress_tag_state);
     }


     /* Set offset register back to zero (default value) */
     __LOG_FW(("Set offset register back to zero (default value)"));
     PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(52);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR53_SRV6_End_Node_GSID_COC32
 * inparam      None
 * return       None
 * description  This thread covers G-SID CoC32 end node processing
 *              It modifies the IPv6 DIP with the next G-SID from SRH segment list.
 *              The thread is triggered by ePort configuration.
 *
 *              cfg template: Byte 0 should hold DIP Common Prefix length (in bits units)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(32B),IPv6(40B),SRH(8B),Container0(16B),Container1(16B),Container2(16B)
 *              hdr out: Extension space(32B),IPv6(40B),SRH(8B),Container0(16B),Container1(16B),Container2(16B)
 *
 *              While IPv6 packet with DIP containing Common Prefix(variable length) + G-SID(4B) + SI(2b)
 *
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmware actions:
 *              - Updates SI and SL indexes
 *              - Updates IPv6 DIP with Next G-SID
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR53_SRV6_End_Node_GSID_COC32) () {
    /*xt_iss_client_command("profile","enable");            */
    /*xt_iss_client_command("profile","disable"); _exit(0); */

    uint8_t sl;                    /* Indicates the required container from SRH container list */
    uint8_t si;                    /* Indicates the next G-SID to be used from the required SRH container */
    uint8_t NextGSID;              /* Holds G-SID value */
    uint32_t commonPrefixLength;   /* Holds Common Prefix Length in bytes */
    uint32_t src;                  /* Holds source offset (next G-SID from SRH container list) */
    uint32_t dest;                 /* Holds destination offset (where to copy next G-SID) */


    /* Get pointer to cfg & packet */
    struct thr53_SRV6_End_Node_GSID_COC32_in_hdr *thr53_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr53_SRV6_End_Node_GSID_COC32_in_hdr);
    struct thr53_SRV6_End_Node_GSID_COC32_cfg *thr53_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr53_SRV6_End_Node_GSID_COC32_cfg);
    struct thr53_SRV6_End_Node_GSID_COC32_out_hdr *thr53_out_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr53_SRV6_End_Node_GSID_COC32_out_hdr);

    /*-------------------------------------------------------------------------------------------------------------------------
      - Get SI & SL parameters
      -------------------------------------------------------------------------------------------------------------------------*/

    /* Get SI[1:0] = IPv6 DIP[1:0] (2 lsbits) */
    __LOG_FW(("Get SI[1:0] = IPv6 DIP[1:0] (2 lsbits)"));
    si = thr53_in_hdr_ptr->IPv6_Header.dip3_low&0x3;
    __LOG_PARAM_FW(si);

    /* Get SL[7:0] = SL field in SRH */
    __LOG_FW(("Get SL[7:0] = SL field in SRH"));
    sl = thr53_in_hdr_ptr->SRv6_Header.segments_left;
    __LOG_PARAM_FW(sl);

    /* Check SI value */
    __LOG_FW(("Check SI value"));
    if (si==0)
    {   /* SI=0 therefore no more G-SIDs in current container */
        __LOG_FW(("SI=0 therefore no more G-SIDs in current container"));

        /* Decrement SL, the next container */
        __LOG_FW(("Decrement SL, the next container"));
        sl--;

        /* Set SI to 3 */
        __LOG_FW(("Set SI to 3"));
        si=3;

        /* Copy updated SL into SRH SL field */
        __LOG_FW(("Copy updated SL into SRH SL field"));
        thr53_out_hdr_ptr->SRv6_Header.segments_left = sl;
    }
    else
    {   /* SI!=0 therefore current container still has G-SIDs */
        __LOG_FW(("SI!=0 therefore current container still has G-SIDs"));

        /* Decrement SI by 1 */
        __LOG_FW(("Decrement SI by 1"));
        si--;
    }


    /*-------------------------------------------------------------------------------------------------------------------------
      - Update IPv6 DIP with Next G-SID
      -------------------------------------------------------------------------------------------------------------------------*/

    /* Current_Container_Offset = extension space(32B) + IPv6(40B) + SRH(8B) + (SL*16) + Current_Container_Offset[SI] -- SI selects the 32b G-SID */
    __LOG_FW(("Current_Container_Offset = extension space(32B) + IPv6(40B) + SRH(8B) + (SL*16) + Current_Container_Offset[SI] -- SI selects the 32b G-SID "));
    src = 80 + sl*16 + si*4;
    __LOG_PARAM_FW(src);

    /* Get common prefix length from template byte 0 (in bits units) */
    __LOG_FW(("Get common prefix length from template byte 0 (in bits units)"));
    commonPrefixLength = thr53_cfg_ptr->SRV6_GSID_COC32_template.commonPrefixLength/8;
    __LOG_PARAM_FW(thr53_cfg_ptr->SRV6_GSID_COC32_template.commonPrefixLength);
    __LOG_PARAM_FW(commonPrefixLength);

    /* Get destination offset for G-SID location: extension space(32B) + IPv6.DIP(24B) + common prefix length (bytesoffset) */
    __LOG_FW(("Get destination offset for G-SID location: extension space(32B) + IPv6.DIP(24B) + common prefix length (bytesoffset)"));
    dest =  56 + commonPrefixLength;
    __LOG_PARAM_FW(dest);


    /* Copy 32b Next_G-SID into IPv6 DIP at offset Common_Prefix_Length */
    __LOG_FW(("Copy 32b Next_G-SID into IPv6 DIP at offset Common_Prefix_Length"));

    NextGSID = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo+src);
    __PRAGMA_NO_REORDER

    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo+dest,NextGSID);
    __PRAGMA_NO_REORDER

    NextGSID = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo+src+1);
    __PRAGMA_NO_REORDER

    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo+dest+1,NextGSID);
    __PRAGMA_NO_REORDER

    NextGSID = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo+src+2);
    __PRAGMA_NO_REORDER

    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo+dest+2,NextGSID);
    __PRAGMA_NO_REORDER

    NextGSID = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo+src+3);
    __PRAGMA_NO_REORDER

    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo+dest+3,NextGSID);
    __PRAGMA_NO_REORDER


    /*-------------------------------------------------------------------------------------------------------------------------
      - Update new SI value
      -------------------------------------------------------------------------------------------------------------------------*/

    /* Updates new SI value into IPv6 DIP[1:0] */
    __LOG_FW(("Updates new SI value into IPv6 DIP[1:0]"));
    thr53_out_hdr_ptr->IPv6_Header.dip3_low = (thr53_out_hdr_ptr->IPv6_Header.dip3_low&0xFFFC)|si;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(53);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET
 * inparam      None
 * return       None
 * description  sFlow is a technology for monitoring traffic in data networks containing switches and routers.
 *              It requires ingress/egress mirrored packets to be sent over an IPv4-UDP tunnel to a collector in the network.
 *              Following the UDP header there is SFLOW header which is followed by the mirrored packet as the payload.
 *              Since the total IP-UDP-sFlowV5 encapsulation exceeds 128B, which is the maximum header increase in Falcon, the packet
 *              is processed by 2 passes in the control pipe.
 *              1st pass inserts the 5 sFlow headers by the PHA firmware into the packet and adds a L2 header.
 *              2nd pass removes the incoming L2 header and adds IPv4/6 UDP tunnel encapsulation by the Falcon Tunnel-start in the HA unit.
 *
 *              cfg template: 4 LSB holds IPv4 agent address, next 4 bytes holds sample data header and rest 8 bytes are reserved
 *              src & trg entries: Not in use (4B)
 *              hdr in:    | Extension space (64B) | Generic TS Data (54B) | payload |
 *              hdr out:   | Extension space (8B)  | MAC DA(6B) | MAC SA(6B) | Vlan(4B) | Ether Type(2B) | sFlow header(28B) | Data Format Header (8B) | Flow Sample Header (32B) | Flow Record Header (8B) | Raw Sample Header (16B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmwares actions:
 *              - Adds sFlow header + Data Format Header + Flow Sample Header + Flow Record Header + Raw Sample Header and sets it according to architect's definitions.
 *              - Updates HW that 56 bytes were added
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET)() {
    /* xt_iss_client_command("profile","enable");            */
    /* xt_iss_client_command("profile","disable"); _exit(0); */

    uint32_t    ts_sec;                             /* timestamp in seconds */
    uint32_t    ts_nano_sec;                        /* timestamp in nano seconds */
    uint32_t    ts_sec_new;                         /* holds timestamp in seconds after reading it for the second time to check wraparound */
    uint32_t    ts_sec_to_msec;                     /* holds seconds value in milliseconds units */
    uint32_t    ts_nano_sec_to_msec;                /* holds nano seconds value in milliseconds units */
    uint32_t    uptime;                             /* holds uptime which is the time (in ms) since system boot */
    uint8_t     tai_sel;                            /* TAI select index 0 or 1 */
    uint32_t    sampleSeqNum;                       /* holds sample Sequence Number taken from shared memory */
    uint32_t    dropsNum;                           /* holds drops number taken from shared memory */
    uint32_t    smemSrcPortSampleSeqTableEntryAddr; /* holds address of specific entry in src port sample sequence number table in shared memory */
    uint32_t    smemSrcPortDropsTableEntryAddr;     /* holds address of specific entry in src port drops table in shared memory */
    uint32_t    smemSrcPortIndx;                    /* holds index (0:127) to src port table in shared memory */
	uint32_t    datagramSeqNum;                     /* holds datagram Sequence Number taken from shared memory */
	uint8_t     smemTargetCollectorIndx;            /* holds index (0:15) to datagram header sequence number table in shared memory */
    uint32_t    smemDgramHdrSeqTableEntryAddr;      /* holds address of specific entry in datagram header sequence number table in shared memory */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr57_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

#ifdef ASIC_SIMULATION
    /* Print out the input packet */
    __LOG_FW(("Print out the input packet"));
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 4) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 8) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 12) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 16) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 20) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 24) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 28) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 32) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 36) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 40) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 44) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 48) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 52) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 56) );
#endif /* ASIC_SIMULATION */

    /* Get ptp TAI IF select */
    __LOG_FW(("Get ptp TAI IF select"));
    tai_sel = thr57_desc_ptr->phal2ppa.ptp_tai_select;
    __LOG_PARAM_FW(tai_sel);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Shift packet left to make room for sFlow headers
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Move left Generic TS Data by 56 bytes to make room for sFlow headers */
    __LOG_FW(("Move left Generic TS Data by 56 bytes to make room for sFlow headers"));

    ACCEL_CMD_TRIG(SHIFTLEFT_56_BYTES_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_LEN18_FROM_PKT_Generic_TS_Data__mac_da_47_32_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Initialize sFlow Header Fields to be zero
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Use Generic TS Data unused fields 32 bytes starting from data_35_32 to initialize sFlow headers fields with 0 */
    __LOG_FW(("Use Generic TS Data unused fields 32 bytes starting from data_35_32 to initialize sFlow headers fields with 0"));

    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_PKT_Generic_TS_Data__data_35_32_LEN32_TO_PKT_sflow_ipv4_header__version_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_PKT_Generic_TS_Data__data_35_32_LEN32_TO_PKT_sflow_data_format_header__sample_length_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_PKT_Generic_TS_Data__data_35_32_LEN28_TO_PKT_sflow_flow_sample_header__number_of_records_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow header (28B). 7 words each of 32 bits.
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow header. 7 words each of 32 bits."));

    /* Word0: sFlow Version. Set to fixed value 0x5 */
    __LOG_FW(("Word0: sFlow Version. Set to fixed value 0x5"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x5_LEN3_TO_PKT_sflow_ipv4_header__version_PLUS29_OFFSET, 0);

    /* Word1: IP version. Set to fixed value 0x1 (IPv4) */
    __LOG_FW(("Word1: IP version. Set to fixed value 0x1 (IPv4)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x1_LEN1_TO_PKT_sflow_ipv4_header__agent_ip_version_PLUS31_OFFSET, 0);

    /* Word2: Agent IPv4 Address. Set to template[31:0] */
    __LOG_FW(("Word2: Agent IPv4 Address. Set to template[31:0]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CFG_sflow_ipv4_template__agent_ip_address_LEN4_TO_PKT_sflow_ipv4_header__agent_ip_address_OFFSET, 0);

    /* Word3: Sub Agent ID. Set to 0 */
    __LOG_FW(("Word3: Sub Agent ID. Set to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x0_LEN1_TO_PKT_sflow_ipv4_header__sub_agent_id_PLUS31_OFFSET, 0);

    /* Word4: Datagram Header Sequence Number. Set to SharedMem.sFlowDatagramSequenceTable[31:0]. Increment SharedMem.sFlowDatagramSequenceTable[31:0] */
    __LOG_FW(("Word4: Datagram Header Sequence Number. Set to SharedMem.sFlowDatagramSequenceTable[31:0]. Increment SharedMem.sFlowDatagramSequenceTable[31:0]"));

    /* Get sFlow Tunnel ID representing the target collectors from the incoming packet VLAN-ID.
	   Get it from the incoming packet vid. Applicable values are 0:15. */
	__LOG_FW(("Get sFlow Tunnel ID from packet VLAN-ID Generic_TS_Data.vid[7:0]"));
	smemTargetCollectorIndx = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 23) & 0xF;
	__LOG_PARAM_FW(smemTargetCollectorIndx);

    /* Get shared memory address for datagram header sequence number table */
    __LOG_FW(("Get shared memory address for datagram header sequence number table"));
    smemDgramHdrSeqTableEntryAddr = DRAM_SFLOW_DATAGRAM_HEADER_SEQ_NUM_TABLE_ADDR + smemTargetCollectorIndx * 4;
    __LOG_PARAM_FW(smemDgramHdrSeqTableEntryAddr);

    /* Read datagram sequence number from shared memory */
    __LOG_FW(("Read datagram sequence number from shared memory"));
    datagramSeqNum = PPA_FW_SP_READ(smemDgramHdrSeqTableEntryAddr);
    __LOG_PARAM_FW(datagramSeqNum);
	__PRAGMA_NO_REORDER

    /* Set datagram sequence number in sFlow Header (word 4) */
    __LOG_FW(("Set datagram sequence number in sFlow Header (word 4)"));
    PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 42 , datagramSeqNum >> 16 );
	PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 44 , datagramSeqNum );

    /* Increment datagram sequence number by 1 and save it in shared memory datagram header sequence number table */
    __LOG_FW(("Increment datagram sequence number by 1 and save it in shared memory datagram header sequence number table"));
    PPA_FW_SP_WRITE( smemDgramHdrSeqTableEntryAddr , datagramSeqNum + 1 );
    __LOG_PARAM_FW(datagramSeqNum + 1);

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      Word5: Uptime in milliseconds. Set according to TAI.sec and TAI.ns.
      - The uptime is the time (in ms) since system boot.
      - In our case we have 18 bits of seconds and 30 bits of ns that is ~3 days
      - Since 18b for seconds we assume that multiplication of 1000 to convert to milliseconds should not be bigger than 32b size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Word5: Uptime in milliseconds. Set according to TAI.sec and TAI.ns."));

    /* Read TOD word 1 which represents seconds field [31:0] */
    __LOG_FW(("Read TOD word 1 which represents seconds field [31:0]"));
    ts_sec = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec);
    /* Read TOD word 0 which represents nano seconds field */
    __LOG_FW(("Read TOD word 0 which represents nano seconds field"));
    ts_nano_sec = READ_TOD_IF_REG(TOD_WORD_0, tai_sel);
    __LOG_PARAM_FW(ts_nano_sec);
	__PRAGMA_NO_REORDER

    /* Check for wraparound */
    __LOG_FW(("Read again seconds (TOD word 1) to check for wraparound"));
    ts_sec_new = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec_new);

    /* < 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound */
    __LOG_FW(("< 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound"));
    if ( (ts_sec_new != ts_sec) && (ts_nano_sec < 500000000) )
    {
        __LOG_FW(("Update ts_sec with new value since wraparound occurred"));
        ts_sec = ts_sec_new ;
        __LOG_PARAM_FW(ts_sec);
    }

    /* Convert seconds to milliseconds units */
    __LOG_FW(("Convert seconds to milliseconds units"));
    ts_sec_to_msec = ts_sec * 1000 ;

    /* Convert nano seconds to milliseconds units */
    __LOG_FW(("Convert nano seconds to milliseconds units"));
    ts_nano_sec_to_msec = ts_nano_sec / 1000000 ;

    /* Calculate uptime in milliseconds */
    __LOG_FW(("Calculate uptime in milliseconds"));
    uptime = ts_sec_to_msec + ts_nano_sec_to_msec ;

    /* Set uptime in sFlow header */
    __LOG_FW(("Set uptime in sFlow header"));
    PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 46 , uptime >>16 );
	PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 48 , uptime );

    /* Word6: Number of samples. Set to fixed value 1 */
    __LOG_FW(("Word6: Number of samples. Set to fixed value 1"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x1_LEN1_TO_PKT_sflow_ipv4_header__samples_number_PLUS31_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set Sample Data with sFlow Data Format Header (4B) and Sample Length. 2 words each of 32 bits.
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set Sample Data with sFlow Data Format Header (4B) and Sample Length. 2 words each of 32 bits."));

    /* Word7: Data Format. Set to template[63:32] */
    __LOG_FW(("Word7: Data Format. Set to template[63:32]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CFG_sflow_ipv4_template__data_format_header_LEN4_TO_PKT_sflow_data_format_header__smi_private_enterprise_code_OFFSET, 0);

    /* Word8: Set Sample Length to desc<egress_byte_count> - 36B(added by Generic TS) + 56B (32B Flow Sample Header + 8B Flow Record Header + 16B Raw Packet Header) */
    __LOG_FW(("Word8: Set Sample Length to desc<egress_byte_count> - 36B(added by Generic TS) + 56B (32B Flow Sample Header + 8B Flow Record Header + 16B Raw Packet Header"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_sflow_data_format_header__sample_length_PLUS18_OFFSET, 0);
	__PRAGMA_NO_REORDER
	ACCEL_CMD_TRIG(ADD_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x14_LEN14_TO_PKT_sflow_data_format_header__sample_length_PLUS18_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow Flow Sample Header (32B). 8 words each of 32 bits.
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow Flow Sample Header (32B). 8 words each of 32 bits."));

    /* Word9: Sample Sequence Number. Set to SharedMem.SrcPort[31:0]. Increment SharedMem.SrcPort[31:0] */
    __LOG_FW(("Word9: Sample Sequence Number. Set to SharedMem.SrcPort[31:0]. Increment SharedMem.SrcPort[31:0]"));

    /* Get index to shared memory sample sequence source port table. Get it from Desc.local_dev_src_port. Applicable values are 0:127. */
    __LOG_FW(("Get index to shared memory sample sequence source port table. Get it from Desc.local_dev_src_port. Applicable values are 0:127."));
    smemSrcPortIndx = (thr57_desc_ptr->phal2ppa.local_dev_src_port & 0x7F);
    __LOG_PARAM_FW(thr57_desc_ptr->phal2ppa.local_dev_src_port);
    __LOG_PARAM_FW(smemSrcPortIndx);

    /* Get shared memory address compatible to packet's source port sample sequence */
    __LOG_FW(("Get shared memory address compatible to packet's source port sample sequence"));
    smemSrcPortSampleSeqTableEntryAddr = DRAM_SFLOW_SOURCE_PORT_SAMPLE_SEQ_NUM_TABLE_ADDR + smemSrcPortIndx*4;
    __LOG_PARAM_FW(smemSrcPortSampleSeqTableEntryAddr);

    /* Read sample sequence number from shared memory */
    __LOG_FW(("Read sample sequence number from shared memory"));
    sampleSeqNum = PPA_FW_SP_READ(smemSrcPortSampleSeqTableEntryAddr);
    __LOG_PARAM_FW(sampleSeqNum);
	__PRAGMA_NO_REORDER

    /* Set sample sequence number in sFlow Flow Sample Header (word 9) */
    __LOG_FW(("Set sample sequence number in sFlow Flow Sample Header (word 9)"));
    PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 62 , sampleSeqNum >> 16);
	PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 64 , sampleSeqNum );

    /* Increment sample sequence number by 1 and save it in shared memory source port table */
    __LOG_FW(("Increment sample sequence number by 1 and save it in shared memory source port table"));
    PPA_FW_SP_WRITE( smemSrcPortSampleSeqTableEntryAddr , sampleSeqNum + 1 );
    __LOG_PARAM_FW(sampleSeqNum + 1);

    /* Word11: Sampling Rate. Set to EPCL Metadata[31:0] */
    __LOG_FW(("Word11: Sampling Rate. Set to EPCL Metadata[31:0]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_DESC_phal2ppa__pha_metadata_LEN4_TO_PKT_sflow_flow_sample_header__sampling_rate_OFFSET, 0);

    /* Word12: Sampling Pool. Set to SharedMem.SrcPort[31:0]. Same as sample sequence number, includes drops */
    __LOG_FW(("Word12: Sampling Pool. Set to SharedMem.SrcPort[31:0]. Same as sample sequence number, includes drops"));
    PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 74 , sampleSeqNum >> 16);
	PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 76 , sampleSeqNum );

    /* Word13: Drops. If <egress packet command> == HARD/SOFT DROP then Increment SharedMem.DropTable[Local Dev Src Port] */
    __LOG_FW(("Word13: Drops. If <egress packet command> == HARD/SOFT DROP then Increment SharedMem.DropTable[Local Dev Src Port]"));

    /* Get shared memory address of drops counter compatible to packet's source port. Use the same index for sequence number */
    __LOG_FW(("Get shared memory address of drops counter compatible to packet's source port. Use the same index for sequence number"));
    smemSrcPortDropsTableEntryAddr = DRAM_SFLOW_SOURCE_PORT_DROPS_TABLE_ADDR + smemSrcPortIndx * 4;
    __LOG_PARAM_FW(smemSrcPortDropsTableEntryAddr);

    /* Read Drops counter number from shared memory */
    __LOG_FW(("Read Drops counter number from shared memory"));
    dropsNum = PPA_FW_SP_READ(smemSrcPortDropsTableEntryAddr);
    __LOG_PARAM_FW(dropsNum);

    /* Check if packet Hard/Soft drop */
    __LOG_FW(("Check if packet Hard/Soft drop"));
    __LOG_PARAM_FW(thr57_desc_ptr->phal2ppa.egress_packet_cmd);
    if( (thr57_desc_ptr->phal2ppa.egress_packet_cmd == 3/*HARD DROP*/) || (thr57_desc_ptr->phal2ppa.egress_packet_cmd == 4/*SOFT DROP*/))
    {
        /* Packet is dropped therefore increment Drops number by 1 */
        __LOG_FW(("Packet is dropped therefore increment Drops number by 1"));
        dropsNum +=1;

        /* Save it in shared memory */
        __LOG_FW(("Save it in shared memory"));
        PPA_FW_SP_WRITE(smemSrcPortDropsTableEntryAddr, dropsNum);
    }
    /* Set Drops number in output packet in sFlow Flow Sample Header.drops (word 13) */
    __LOG_FW(("Set sample sequence number in output packet in sFlow Flow Sample Header.drops (word 13)"));
    PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 78 , dropsNum >> 16);
	PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 80 , dropsNum );
    __LOG_PARAM_FW(dropsNum);

    /* Word14: Input Interface. Set to LocalDevSrcPort */
    __LOG_FW(("Word14: Input Interface. Set to LocalDevSrcPort"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_DESC_phal2ppa__local_dev_src_port_LEN10_TO_PKT_sflow_flow_sample_header__input_PLUS22_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Word10: Source ID Type + Index. Set to LocalDevSrcPort. */
    __LOG_FW(("Word10: Source ID Type + Index. Set to LocalDevSrcPort."));
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_PKT_expansion_space__reserved_0_PLUS82_LEN4_TO_PKT_sflow_flow_sample_header__source_id_type_OFFSET, 0);

    /* Word15: Output Interface. Set to 0 */
    __LOG_FW(("Word15: Output Interface. Set to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x0_LEN1_TO_PKT_sflow_flow_sample_header__output_PLUS31_OFFSET, 0);

    /* Word16: Set Number of records to 1 */
    __LOG_FW(("Word16: Set Number of records to 1"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x1_LEN1_TO_PKT_sflow_flow_sample_header__number_of_records_PLUS31_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow Flow Record Header (8B). 2 words each of 32 bits.
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow Flow Record Header (8B). 2 words each of 32 bits."));

    /* Word17: Data Format. Set to template[63:32] */
    __LOG_FW(("Word17: Data Format. Set to template[63:32]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CFG_sflow_ipv4_template__data_format_header_LEN4_TO_PKT_sflow_flow_record_header__smi_private_enterprise_code_OFFSET, 0);

    /* Word18: Set Data Length to desc<egress_byte_count> - 36B(added by Generic TS) + 16B Raw Packet Header */
    __LOG_FW(("Word18: Set Data Length to desc<egress_byte_count> - 36B(added by Generic TS) + 16B Raw Packet Header"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_sflow_flow_record_header__data_length_PLUS18_OFFSET, 0);
	__PRAGMA_NO_REORDER
	ACCEL_CMD_TRIG(SUB_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x14_LEN14_TO_PKT_sflow_flow_record_header__data_length_PLUS18_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow Raw Packet Header (16B). 4 words each of 32 bits.
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow Raw Packet Header (16B). 4 words each of 32 bits."));

    /* Word19: Header Protocol. Set to 1(Ethernet) */
    __LOG_FW(("Word19: Header Protocol. Set to 1(Ethernet"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x1_LEN1_TO_PKT_sflow_raw_sample_header__header_protocol_PLUS31_OFFSET, 0);

    /* Word20: Set Frame Length to desc<egress_byte_count> - 36B(added by Generic TS) */
    __LOG_FW(("Word20: Set Frame Length to desc<egress_byte_count> - 36B(added by Generic TS)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_sflow_raw_sample_header__frame_length_PLUS18_OFFSET, 0);
	__PRAGMA_NO_REORDER
	ACCEL_CMD_TRIG(SUB_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x24_LEN14_TO_PKT_sflow_raw_sample_header__frame_length_PLUS18_OFFSET, 0);

    /* Word21: Stripped. Set to 4(CRC was removed) */
    __LOG_FW(("Word21: Stripped. Set to 4(CRC was removed"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x4_LEN8_TO_PKT_sflow_raw_sample_header__stripped_PLUS24_OFFSET, 0);

    /* Word22: Set Header Size to desc<egress_byte_count> - 36B(added by Generic TS) + 4 */
    __LOG_FW(("Word22: Set Header Size to desc<egress_byte_count> - 36B(added by Generic TS) + 4"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_sflow_raw_sample_header__header_size_PLUS18_OFFSET, 0);
	__PRAGMA_NO_REORDER
	ACCEL_CMD_TRIG(SUB_BITS_THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_CONST_0x20_LEN14_TO_PKT_sflow_raw_sample_header__header_size_PLUS18_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+56 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+56 bytes)."));
    thr57_desc_ptr->ppa_internal_desc.fw_bc_modification = 56;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(57);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname        THR60_DropAllTraffic
 * inparam         None
 * return          None
 * description     Thread that is used to drop any packet that goes through PPA
 *                 Firmware actions:
 *                 - Set Desc<egress_packet_cmd> to 0x3 (Hard Drop)
 *                 - Set Desc<CPU code> to 183
 *                 Note:
 *                 - this thread is mapped to a fix address and should not be changed.
 *                 - allows to execute the thread while fw version is being upgraded.
 *                 - do not use accelerator commands since it will change thread's code
 *                   since in each build accelerator commands might be in different location
 ************************************************************************************************************************/
__attribute__ ((section(".iram0.text")))
__attribute__ ((aligned (16)))
void PPA_FW(THR60_DropAllTraffic)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    /* Get pointer to descriptor */
    struct ppa_in_desc*  thr60_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Set Desc<egress_packet_cmd> to 0x3 (Hard Drop) */
    __LOG_FW(("Set Desc<egress_packet_cmd> to 0x3 (Hard Drop)"));
    thr60_desc_ptr->phal2ppa.egress_packet_cmd = 0x3;

    /* Set Desc<CPU code> to 183 */
    __LOG_FW(("Set Desc<CPU code> to 183"));
    thr60_desc_ptr->phal2ppa.cpu_code = 183;

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(60);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR61_save_target_port_info
 * inparam      None
 * return       None
 * description  Save target port information (device & physical port) into descriptor copy reserved field
 *
 *              cfg template: No usage (16B)
 *              src & trg entries: No usage (4B)
 *              hdr in:    don't care
 *              hdr out:   don't care
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes +  32 bytes Header increment space
 *
 *              Firmwares does the following.
 *              - do not touch packet only descriptor
 *              - copy Desc<target dev[7:0]> to Desc<copy reserved[17:10]>
 *              - copy Desc<target phy port[7:0]> to Desc<copy reserved[9:2]>
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR61_save_target_port_info)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    /* Copy Desc<target dev[7:0]> to Desc<copy reserved[17:10]> */
    __LOG_FW(("Copy Desc<target dev[7:0]> to Desc<copy reserved[17:10]>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR61_save_target_port_info_DESC_phal2ppa__trg_dev_PLUS2_LEN8_TO_DESC_phal2ppa__copy_reserved_PLUS2_OFFSET, 0);

    /* Copy Desc<target phy port[7:0]> to Desc<copy reserved[9:2]> */
    __LOG_FW(("Copy Desc<target phy port[7:0]> to Desc<copy reserved[9:2]>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR61_save_target_port_info_DESC_phal2ppa__trg_phy_port_PLUS2_LEN8_TO_DESC_phal2ppa__copy_reserved_PLUS10_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(61);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR62_enhanced_sFlow_fill_remain_IPv4
 * inparam      None
 * return       None
 * description  sFlow is a technology for monitoring traffic in data networks containing switches and routers.
 *              The information of input/output port & ingress/egress timestamp of the original packet are sent to a controller.
 *              The controller analyzes the message delay according to the information and draws the forwarding delay jitter curve,
 *              so that users can understand the network transmission quality.
 *
 *              This solution requires usage of loopback port
 *              - in first pass sFlow headers are added by PHA (done by THR66_enhanced_sFlow thread)
 *              - in second pass after packet was loopback to pipeline again
 *                - IPv4-UDP headers are added by HA
 *                - specific fields in IPv4, udp and sFlow are updated by PHA (done by this thread)
 *
 *              cfg template: | reserved(15B) | src device id(1B) |
 *              src & trg entries: No usage (4B)
 *              hdr in:  |Extension space(32B)|IPv4(20B)|UDP(8B)|sFlow shim header(16B)|RX_TS_SHIM(14B)|TX_TS_SHIM(14B)|mirrored packet|
 *              hdr out: |Extension space(32B)|IPv4(20B)|UDP(8B)|sFlow shim header(16B)|RX_TS_SHIM(14B)|TX_TS_SHIM(14B)|mirrored packet|
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmwares actions: no change in packet size. Sets specific fields in packet and descriptor.
 *              - Sets RX_TS_SHIM<port_number>
 *              - Sets RX_TS_SHIM<IP_high>
 *              - Calculates RX_TS_SHIM<checksum>
 *              - Sets TX_TS_SHIM<port_number>
 *              - Sets TX_TS_SHIM<IP_low>
 *              - Calculates TX_TS_SHIM<checksum>
 *              - Sets Desc<Egress Checksum Mode>
 *              - Updates IPv4 length to +44 bytes
 *              - Updates UDP length to +44 bytes
 *              - Calculates IPv4 CS
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR62_enhanced_sFlow_fill_remain_IPv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


#ifdef ASIC_SIMULATION
    /* Print out the input packet */
    __LOG_FW(("Print out the input packet"));
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 4) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 8) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 12) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 16) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 20) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 24) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 28) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 32) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 36) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 40) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 44) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 48) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 52) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 56) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 60) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 64) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 68) );
#endif /* ASIC_SIMULATION */


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update RX_TS_SHIM<port_number> field
      - Update RX_TS_SHIM<IP_high> field
      - Calculate checksum of RX_TS_SHIM
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Set RX port number (7 bits): If Desc<Orig is Trunk> copy it from Desc<Orig Src Trunk ID> else copy it from Desc<orig_src_eport>
       Since these 2 fields are muxed, start from same position and required only 7 LS bits, no need to check condition of Desc<Orig is Trunk> */
    __LOG_FW(("Set RX port number (7 bits): If Desc<Orig is Trunk> copy it from Desc<Orig Src Trunk ID> else copy it from Desc<orig_src_eport>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR62_enhanced_sFlow_fill_remain_IPv4_DESC_phal2ppa__orig_src_eport_PLUS6_LEN7_TO_PKT_enhanced_sflow_rx_ts_shim_header__port_number_OFFSET, 0);

    /* Set IP_high(2B). Copy it from Desc<pha_metadata[31:16]> */
    __LOG_FW(("Set IP_high(2B). Copy it from Desc<pha_metadata[31:16]>"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR62_enhanced_sFlow_fill_remain_IPv4_DESC_phal2ppa__pha_metadata_LEN2_TO_PKT_enhanced_sflow_rx_ts_shim_header__ip_high_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Calculate checksum of RX_TS_SHIM (14B) */
    __LOG_FW(("Calculate checksum of RX_TS_SHIM (14B)"));

    /* Skip cs field and sum rx timestamp (6B) */
    __LOG_FW(("Skip cs field and sum rx timestamp (6B)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR62_enhanced_sFlow_fill_remain_IPv4_LEN6_FROM_PKT_enhanced_sflow_rx_ts_shim_header__rx_timestamp_47_32_OFFSET, 0);

    /* Sum last 4 bytes of RX_TS_SHIM (includes reserved,ip_high,port_number and port direction) */
    __LOG_FW(("Sum last 4 bytes of RX_TS_SHIM (includes reserved,ip_high,port_number and port direction)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR62_enhanced_sFlow_fill_remain_IPv4_LEN4_FROM_PKT_enhanced_sflow_rx_ts_shim_header__reserved_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Store checksum result in checksum field of RX_TS_SHIM header */
    __LOG_FW(("Store checksum result in checksum field of RX_TS_SHIM header"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR62_enhanced_sFlow_fill_remain_IPv4_TO_PKT_enhanced_sflow_rx_ts_shim_header__checksum_PLUS2_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update TX_TS_SHIM<port_number> field
      - Update TX_TS_SHIM<IP_low> field
      - Calculate checksum of TX_TS_SHIM
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Set TX port number (7 bits): copy from Desc<Local Dev target phy port [6:0]> */
    __LOG_FW(("Set TX port number (7 bits): copy from Desc<Local Dev target phy port [6:0]>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR62_enhanced_sFlow_fill_remain_IPv4_DESC_phal2ppa__local_dev_trg_phy_port_PLUS3_LEN7_TO_PKT_enhanced_sflow_tx_ts_shim_header__port_number_OFFSET, 0);

    /* Set IP_low. Copy it from Desc<pha_metadata[15:0] */
    __LOG_FW(("Set IP_low. Copy it from Desc<pha_metadata[15:0]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR62_enhanced_sFlow_fill_remain_IPv4_DESC_phal2ppa__pha_metadata_PLUS2_LEN2_TO_PKT_enhanced_sflow_tx_ts_shim_header__ip_low_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Calculate checksum of TX_TS_SHIM (14B) */
    __LOG_FW(("Calculate checksum of TX_TS_SHIM (14B)"));

    /* Skip cs field and sum tx timestamp (6B) */
    __LOG_FW(("Skip cs field and sum tx timestamp (6B)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR62_enhanced_sFlow_fill_remain_IPv4_LEN6_FROM_PKT_enhanced_sflow_tx_ts_shim_header__tx_timestamp_47_32_OFFSET, 0);

    /* Sum last 4 bytes of TX_TS_SHIM (includes reserved,ip_high,port_number and port direction) */
    __LOG_FW(("Sum last 4 bytes of TX_TS_SHIM (includes reserved,ip_high,port_number and port direction)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR62_enhanced_sFlow_fill_remain_IPv4_LEN4_FROM_PKT_enhanced_sflow_tx_ts_shim_header__reserved_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Store checksum result in checksum field of TX_TS_SHIM header */
    __LOG_FW(("Store checksum result in checksum field of TX_TS_SHIM header"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR62_enhanced_sFlow_fill_remain_IPv4_TO_PKT_enhanced_sflow_tx_ts_shim_header__checksum_PLUS2_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Desc<Egress Checksum Mode> = ChecksumField (0x1)
      - UDP new length +=44
      - IPv4 new length +=44
      - calculate new IPv4 checksum: incremental update = ~( ~(old checksum) + ~(old Total Length) + (new Total Length) )
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Desc<Egress Checksum Mode>= ChecksumField (0x1) */
    __LOG_FW(("Desc<Egress Checksum Mode>= ChecksumField (0x1)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR62_enhanced_sFlow_fill_remain_IPv4_CONST_0x1_LEN1_TO_DESC_phal2ppa__egress_checksum_mode_OFFSET, 0);

    /* Update new UDP length. UDP<length> + 44 */
    __LOG_FW(("Update new UDP length. UDP<length> + 44"));
    ACCEL_CMD_TRIG(ADD_BITS_THR62_enhanced_sFlow_fill_remain_IPv4_CONST_0x2C_LEN16_TO_PKT_udp_header__Length_OFFSET, 0);

    /* Sum old IP Total Length */
    __LOG_FW(("Sum old IP Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR62_enhanced_sFlow_fill_remain_IPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Update new IP header total length: IPv4<Total Length> + 44 */
    __LOG_FW(("Update new IP header total length: IPv4<Total Length> + 44"));
    ACCEL_CMD_TRIG(ADD_BITS_THR62_enhanced_sFlow_fill_remain_IPv4_CONST_0x2C_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Sum new Total Length */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR62_enhanced_sFlow_fill_remain_IPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);

    /* Sum old IPv4 CS */
    __LOG_FW(("Sum old IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR62_enhanced_sFlow_fill_remain_IPv4_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR62_enhanced_sFlow_fill_remain_IPv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(62);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR63_enhanced_sFlow_fill_remain_IPv6
 * inparam      None
 * return       None
 * description  sFlow is a technology for monitoring traffic in data networks containing switches and routers.
 *              The information of input/output port & ingress/egress timestamp of the original packet are sent to a controller.
 *              The controller analyzes the message delay according to the information and draws the forwarding delay jitter curve,
 *              so that users can understand the network transmission quality.
 *
 *              This solution requires usage of loopback port
 *              - in first pass sFlow headers are added by PHA (done by THR66_enhanced_sFlow thread)
 *              - in second pass after packet was loopback to pipeline again
 *                - IPv6-UDP headers are added by HA
 *                - specific fields in IPv6, udp and sFlow are updated by PHA (done by this thread)
 *
 *              cfg template: | reserved(15B) | src device id(1B) |
 *              src & trg entries: No usage (4B)
 *              hdr in:  |Extension space(32B)|IPv6(40B)|UDP(8B)|sFlow shim header(16B)|RX_TS_SHIM(14B)|TX_TS_SHIM(14B)|mirrored packet|
 *              hdr out: |Extension space(32B)|IPv6(40B)|UDP(8B)|sFlow shim header(16B)|RX_TS_SHIM(14B)|TX_TS_SHIM(14B)|mirrored packet|
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmwares actions: no change in packet size. Sets specific fields in packet and descriptor.
 *              - Sets RX_TS_SHIM<port_number>
 *              - Sets RX_TS_SHIM<IP_high>
 *              - Calculates RX_TS_SHIM<checksum>
 *              - Sets TX_TS_SHIM<port_number>
 *              - Sets TX_TS_SHIM<IP_low>
 *              - Calculates TX_TS_SHIM<checksum>
 *              - Sets Desc<Egress Checksum Mode>
 *              - Updates IPv6 length to +44 bytes
 *              - Updates UDP length to +44 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR63_enhanced_sFlow_fill_remain_IPv6)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


#ifdef ASIC_SIMULATION
    /* Print out the input packet */
    __LOG_FW(("Print out the input packet"));
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 4) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 8) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 12) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 16) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 20) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 24) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 28) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 32) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 36) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 40) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 44) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 48) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 52) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 56) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 60) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 64) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 68) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 72) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 76) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 80) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 84) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 32 + 88) );
#endif /* ASIC_SIMULATION */


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update RX_TS_SHIM<port_number> field
      - Update RX_TS_SHIM<IP_high> field
      - Calculate checksum of RX_TS_SHIM
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Set RX port number (7 bits): If Desc<Orig is Trunk> copy it from Desc<Orig Src Trunk ID> else copy it from Desc<orig_src_eport>
       Since these 2 fields are muxed, start from same position and required only 7 LS bits, no need to check condition of Desc<Orig is Trunk> */
    __LOG_FW(("Set RX port number (7 bits): If Desc<Orig is Trunk> copy it from Desc<Orig Src Trunk ID> else copy it from Desc<orig_src_eport>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR63_enhanced_sFlow_fill_remain_IPv6_DESC_phal2ppa__orig_src_eport_PLUS6_LEN7_TO_PKT_enhanced_sflow_rx_ts_shim_header__port_number_OFFSET, 0);


    /* Set IP_high(2B). Copy it from Desc<pha_metadata[31:16]> */
    __LOG_FW(("Set IP_high(2B). Copy it from Desc<pha_metadata[31:16]>"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR63_enhanced_sFlow_fill_remain_IPv6_DESC_phal2ppa__pha_metadata_LEN2_TO_PKT_enhanced_sflow_rx_ts_shim_header__ip_high_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Calculate checksum of RX_TS_SHIM (14B) */
    __LOG_FW(("Calculate checksum of RX_TS_SHIM (14B)"));

    /* Skip cs field and sum rx timestamp (6B) */
    __LOG_FW(("Skip cs field and sum rx timestamp (6B)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR63_enhanced_sFlow_fill_remain_IPv6_LEN6_FROM_PKT_enhanced_sflow_rx_ts_shim_header__rx_timestamp_47_32_OFFSET, 0);

    /* Sum last 4 bytes of RX_TS_SHIM (includes reserved,ip_high,port_number and port direction) */
    __LOG_FW(("Sum last 4 bytes of RX_TS_SHIM (includes reserved,ip_high,port_number and port direction)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR63_enhanced_sFlow_fill_remain_IPv6_LEN4_FROM_PKT_enhanced_sflow_rx_ts_shim_header__reserved_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Store checksum result in checksum field of RX_TS_SHIM header */
    __LOG_FW(("Store checksum result in checksum field of RX_TS_SHIM header"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR63_enhanced_sFlow_fill_remain_IPv6_TO_PKT_enhanced_sflow_rx_ts_shim_header__checksum_PLUS2_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update TX_TS_SHIM<port_number> field
      - Update TX_TS_SHIM<IP_low> field
      - Calculate checksum of TX_TS_SHIM
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Set TX port number (7 bits): copy from Desc<Local Dev target phy port [6:0]> */
    __LOG_FW(("Set TX port number (7 bits): copy from Desc<Local Dev target phy port [6:0]>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR63_enhanced_sFlow_fill_remain_IPv6_DESC_phal2ppa__local_dev_trg_phy_port_PLUS3_LEN7_TO_PKT_enhanced_sflow_tx_ts_shim_header__port_number_OFFSET, 0);

    /* Set IP_low. Copy it from Desc<pha_metadata[15:0] */
    __LOG_FW(("Set IP_low. Copy it from Desc<pha_metadata[15:0]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR63_enhanced_sFlow_fill_remain_IPv6_DESC_phal2ppa__pha_metadata_PLUS2_LEN2_TO_PKT_enhanced_sflow_tx_ts_shim_header__ip_low_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Calculate checksum of TX_TS_SHIM (14B) */
    __LOG_FW(("Calculate checksum of TX_TS_SHIM (14B)"));

    /* Skip cs field and sum tx timestamp (6B) */
    __LOG_FW(("Skip cs field and sum tx timestamp (6B)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR63_enhanced_sFlow_fill_remain_IPv6_LEN6_FROM_PKT_enhanced_sflow_tx_ts_shim_header__tx_timestamp_47_32_OFFSET, 0);

    /* Sum last 4 bytes of TX_TS_SHIM (includes reserved,ip_high,port_number and port direction) */
    __LOG_FW(("Sum last 4 bytes of TX_TS_SHIM (includes reserved,ip_high,port_number and port direction)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR63_enhanced_sFlow_fill_remain_IPv6_LEN4_FROM_PKT_enhanced_sflow_tx_ts_shim_header__reserved_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Store checksum result in checksum field of TX_TS_SHIM header */
    __LOG_FW(("Store checksum result in checksum field of TX_TS_SHIM header"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR63_enhanced_sFlow_fill_remain_IPv6_TO_PKT_enhanced_sflow_tx_ts_shim_header__checksum_PLUS2_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
        - Desc<Egress Checksum Mode> = ChecksumField (0x1)
        - UDP new length +=44
        - Update new IPv6 length +=44
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Desc<Egress Checksum Mode>= ChecksumField (0x1) */
    __LOG_FW(("Desc<Egress Checksum Mode>= ChecksumField (0x1)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR63_enhanced_sFlow_fill_remain_IPv6_CONST_0x1_LEN1_TO_DESC_phal2ppa__egress_checksum_mode_OFFSET, 0);

    /* Update new UDP length. UDP<length> + 44 */
    __LOG_FW(("Update new UDP length. UDP<length> + 44"));
    ACCEL_CMD_TRIG(ADD_BITS_THR63_enhanced_sFlow_fill_remain_IPv6_CONST_0x2C_LEN16_TO_PKT_udp_header__Length_OFFSET, 0);

    /* Update new IP header payload length: IPv6<Payload Length> + 44 */
    __LOG_FW(("Update new IP header payload length: IPv6<Payload Length> + 44"));
    ACCEL_CMD_TRIG(ADD_BITS_THR63_enhanced_sFlow_fill_remain_IPv6_CONST_0x2C_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(63);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR64_Erspan_TypeII_SameDevMirroring_Ipv4
 * inparam      None
 * return       None
 * description  ERSPAN over IPv4 tunnel. Encapsulated Remote Switch Port Analyzer (ERSPAN)
 *              allows packets to be mirrored to a remote analyzer over a tunnel encapsulation.
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv4 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring
 *              - Support handling vlan tagged or untagged
 *
 *              cfg template:    Not in use (16B). It is used by fw to hold L2 parameters copied from shared memory.
 *              src & trg entries: PHA Target port table holds original packet target physical port for Egress mirroring (2B)
 *                                 PHA Source port table holds original packet source physical port for Ingress mirroring (2B)
 *
 *              hdr in:  | Extension space (64B) | payload |
 *              hdr out: | Extension space (14B or 10B) | MAC addresses(12B) | VLAN(4B) or not | ET(2B) | IPv4(20B) | GRE(8B) | ERSPAN Type II(8B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              - Add IPv4 and L2 headers which are copied from shared memory
 *              - Add GRE & ERSPAN Type II and set them according to Architect's definitions
 *              - Set IPv4 length and calculate IPv4 cs.
 *              - Update Desc<egress_byte_count> +=54(with vlan) or 50(without vlan)
 *              - Update Desc<fw_bc_modification> +=54(with vlan) or 50(without vlan)
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR64_Erspan_TypeII_SameDevMirroring_Ipv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    sessionId;                      /* holds Ingress or Egress session ID (10 bits)                   */
    uint32_t    smemAnalyzerTemplateBaseAddr;   /* holds base address of sFlow templates in shared memory         */
    uint32_t    smemAnalayzerTemplateOfst;      /* holds base address of specific sFlow template in shared memory */
    uint32_t    descRxSniff;                    /* holds value of Desc<rx_sniff> field                            */
    uint32_t    vlanVid;                        /* holds value of vlan vid taken from shared memory template      */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr64_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr64_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr64_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr64_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template
      - To avoid 32 bits alignment issues (packet starts after 10 bytes since Ethertype included), copy it first to CFG template and then to packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 20B of IPv4 header from shared memory and place them in output packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 20B of IPv4 header from shared memory and place them in output packet"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 28) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 32) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 36) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);

     /* Extract 10 bits of Ingress or Egress session ID according to Desc<rx_sniff> field */
     __LOG_FW(("Extract 10 bits of Ingress or Egress session ID according to Desc<rx_sniff> field"));
     sessionId = (sessionId >>(10*descRxSniff)) & 0x3FF;
     __LOG_PARAM_FW(sessionId);
     __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Get VLAN vid
     ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Get vlan vid from template */
    __LOG_FW(("Get vlan vid from template"));
    vlanVid = PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 12) & 0xFFF;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x0800 (IPv4)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x0800 (IPv4)"));

    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x08_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Set GRE.flags to 0x1000. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Set GRE.flags to 0x1000. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Set GRE.Protocol to 0x88BE */
    __LOG_FW(("Set GRE.Protocol to 0x88BE"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Set GRE.Sequence Number to Desc<LM Counter> */
    __LOG_FW(("Set GRE.Sequence Number to Desc<LM Counter>"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

    /* Set Ingress or Egress ERSPAN header.sessionID (extracted earlier according to Desc<rx_sniff>).
       Write it as 4B so rest fields (Ver,VLAN,COS,En,T) are cleared to zero */
    __LOG_FW(("Set Ingress or Egress ERSPAN header.sessionID (extracted earlier according to Desc<rx_sniff>)."
              "Write it as 4B so rest fields (Ver,VLAN,COS,En,T) are cleared to zero"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , sessionId );

    /* Set 20  bits of ERSPAN.index with PHA target/Source Physical Port bits [15:0], rest 4 bits are cleared to zero.
       if descRxSniff = 0 trigger target port accelerator if 1 trigger source port accelerator (next command 4B offset) */
    __LOG_FW(("Set 20  bits of ERSPAN.index with PHA target/Source Physical Port bits [15:0], rest 4 bits are cleared to zero."
              "if descRxSniff = 0 trigger target port accelerator if 1 trigger source port accelerator (next command 4B offset)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CFG_srcTrgPortEntry__targetPortEntry_LEN16_TO_PKT_ERSPAN_type_II_header__Index_PLUS4_OFFSET + 4*descRxSniff, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Set ERSPAN.Ver to 1(4b) and ERSPAN.VLAN(12b) to 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Set ERSPAN.Ver to 1(4b) and ERSPAN.VLAN(12b) to 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS to Desc<QoS Mapped UP> */
    __LOG_FW(("Set ERSPAN.COS to Desc<QoS Mapped UP>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_DESC_phal2ppa__qos_mapped_up_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Set ERSPAN.En to 3(2b) */
    __LOG_FW(("Assign ERSPAN.En to 3(2b)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x3_LEN2_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);

    /* Set ERSPAN.T(1b) to Desc<truncated> */
    __LOG_FW(("Assign ERSPAN.T(1b) to Desc<truncated>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_DESC_phal2ppa__truncated_LEN1_TO_PKT_ERSPAN_type_II_header__T_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update packet IPv4 length field
     - Calculate IPv4 checksum
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IPv4 length field + calculate IPv4 checksum"));


    /* Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+36 */
    __LOG_FW(("Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+36"));

    /* First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b).
       Assumption 2 MS bits were already cleared by application when setting IPv4<Total length> field in shared memory */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b)"));
	ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv4_Header__total_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv4 total Length +=36 */
    __LOG_FW(("IPv4 total Length +=36"));
    ACCEL_CMD_TRIG(ADD_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x24_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Compute IPv4 CS */
    __LOG_FW(("Compute IPv4 CS"));

    /* Sum first 8 bytes of IPv4 */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__version_OFFSET, 0);

    /* Skip cs field and add to sum 2B of ttl and protocol */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_LEN2_FROM_PKT_IPv4_Header__ttl_OFFSET, 0);

    /* Sum the last 8 bytes of IPv4 (src and dest IP addresses fields) */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__sip_high_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set L2 header: MAC DA SA + vlan tag or no vlan tag
     - Update Desc<Egress Byte Count> +=50 or 54(with vlan)
     - Update Desc<fw_bc_modification> +=50 or 54(with vlan)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set L2 header with or without vlan tag, Update Desc<Egress Byte Count> +=50, Update Desc<fw_bc_modification> +=50"));

    /* Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan */
    __LOG_FW(("Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan"));
    if (vlanVid == 0)
    { /* No vlan */

        /* Indicates HW about the new starting point of the packet header (+50 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+50 bytes)"));
        thr64_desc_ptr->ppa_internal_desc.fw_bc_modification = 50;

        /* No vlan, just copy MAC 12 bytes from thread's template */
        __LOG_FW(("No vlan, just copy MAC 12 bytes from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CFG_erspan_template__mac_da_47_32_LEN12_TO_PKT_IPv4_Header__version_MINUS14_OFFSET, 0);

        /* Desc<Egress Byte Count> +=50 */
        __LOG_FW(("Desc<Egress Byte Count> +=50"));
        ACCEL_CMD_TRIG(ADD_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x32_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }
    else
    {/* With vlan */

        /* Indicates HW about the new starting point of the packet header (+54 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+54 bytes)"));
        thr64_desc_ptr->ppa_internal_desc.fw_bc_modification = 54;

        /* With vlan, copy MAC(12B) + vlan tag(4B) from thread's template */
        __LOG_FW(("With vlan, copy MAC(12B) + vlan tag(4B) from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CFG_erspan_template__mac_da_47_32_LEN16_TO_PKT_IPv4_Header__version_MINUS18_OFFSET, 0);

        /* Desc<Egress Byte Count> +=54 */
        __LOG_FW(("Desc<Egress Byte Count> +=54"));
        ACCEL_CMD_TRIG(ADD_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x36_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(64);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR65_Erspan_TypeII_SameDevMirroring_Ipv6
 * inparam      None
 * return       None
 * description  ERSPAN over IPv6 tunnel. Encapsulated Remote Switch Port Analyzer (ERSPAN)
 *              allows packets to be mirrored to a remote analyzer over a tunnel encapsulation.
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv6 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring
 *              - Support handling vlan tagged or untagged
 *              NOTE: since in case of IPv6 tunneling need to add 74 bytes and PHA fw can only add maximum 64 bytes
 *                    HA adds dummy RSPAN (4B) and dummy E-Tag(8B) to be used as place holder when packet arrives to PHA fw.
 *                    PHA fw will then overrun this area with GRE(4LSB) & ERSPAN II headers.
 *
 *              cfg template:    Not in use (16B). It is used by fw to hold L2 parameters copied from shared memory.
 *              src & trg entries: PHA Target port table holds original packet target physical port for Egress mirroring (2B)
 *                                 PHA Source port table holds original packet source physical port for Ingress mirroring (2B)
 *
 *              hdr in:  | Extension space (64B) | origin MAC(12B) | dummy RSPAN tag(4B) | dummy E-Tag (8B) | payload |
 *              hdr out: | Extension space (6B or 2B) | MAC(12B) | VLAN(4B) or not | ET(2B) | IPv6(40B) | GRE(8B) | ERSPAN Type II(8B) | origin MAC(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              - Copy origin MAC into dummy E-tag & RSPAN
 *              - Add IPv6 and L2 headers which are copied from shared memory
 *              - Add GRE & ERSPAN Type II and set them according to Architect's definitions
 *              - Set IPv6 payload length
 *              - Update Desc<egress_byte_count> +=62(with vlan) or 58(without vlan)
 *              - Update Desc<fw_bc_modification> +=62(with vlan) or 58(without vlan)
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR65_Erspan_TypeII_SameDevMirroring_Ipv6)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    sessionId;                     /* holds Ingress or Egress session ID (10 bits)                   */
    uint32_t    smemAnalyzerTemplateBaseAddr;  /* holds base address of sFlow templates in shared memory         */
    uint32_t    smemAnalayzerTemplateOfst;     /* holds base address of specific sFlow template in shared memory */
    uint32_t    descRxSniff;                   /* holds value of Desc<rx_sniff> field                            */
    uint32_t    vlanVid;                       /* holds value of vlan vid taken from shared memory template      */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr65_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

#ifdef ASIC_SIMULATION
    /* Print out the input packet */
    __LOG_FW(("Print out the input packet"));
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 4) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 8) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 12) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 16) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 20) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 24) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 28) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 32) );
#endif /* ASIC_SIMULATION */

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr65_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr65_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr65_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template
      - To avoid 32 bits alignment issues (packet starts after 2 bytes since Ethertype included), copy it first to CFG template and then to packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 40B of IPv6 header from shared memory and place them in output packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 40B of IPv6 header from shared memory and place them in output packet"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 20) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 24) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 28) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 32) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 36) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);

     /* Extract 10 bits of Ingress or Egress session ID according to Desc<rx_sniff> field */
     __LOG_FW(("Extract 10 bits of Ingress or Egress session ID according to Desc<rx_sniff> field"));
     sessionId = (sessionId >>(10*descRxSniff)) & 0x3FF;
     __LOG_PARAM_FW(sessionId);
     __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Get VLAN vid
     ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Get vlan vid from template */
    __LOG_FW(("Get vlan vid from template"));
    vlanVid = PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 12) & 0xFFF;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Move origin MAC addresses into dummy RSPAN & E-Tag section
     ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Move right origin MAC addresses by 12 bytes and overrun dummy RSPAN & E-tag */
    __LOG_FW(("Move right origin MAC addresses by 12 bytes and overrun dummy RSPAN & E-tag"));
    ACCEL_CMD_TRIG(SHIFTRIGHT_12_BYTES_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
   	__PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x86dd (IPv6)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x86dd (IPv6)"));

    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x86_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0xDD_LEN8_TO_PKT_ethertype_header__ethertype_PLUS8_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Set GRE.flags to 0x1000. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Set GRE.flags to 0x1000. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Set GRE.Protocol to 0x88BE */
    __LOG_FW(("Set GRE.Protocol to 0x88BE"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Set GRE.Sequence Number to Desc<LM Counter> */
    __LOG_FW(("Set GRE.Sequence Number to Desc<LM Counter>"));
	ACCEL_CMD_TRIG(COPY_BYTES_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

    /* Set Ingress or Egress ERSPAN header.sessionID (extracted earlier according to Desc<rx_sniff>).
       Write it as 4B so rest fields (Ver,VLAN,COS,En,T) are cleared to zero */
    __LOG_FW(("Set Ingress or Egress ERSPAN header.sessionID (extracted earlier according to Desc<rx_sniff>)."
              "Write it as 4B so rest fields (Ver,VLAN,COS,En,T) are cleared to zero"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 68) , sessionId );

    /* Set 20  bits of ERSPAN.index with PHA target/Source Physical Port bits [15:0], rest 4 bits are cleared to zero.
       if descRxSniff = 0 trigger target port accelerator if 1 trigger source port accelerator (next command 4B offset) */
    __LOG_FW(("Set 20  bits of ERSPAN.index with PHA target/Source Physical Port bits [15:0], rest 4 bits are cleared to zero."
              "if descRxSniff = 0 trigger target port accelerator if 1 trigger source port accelerator (next command 4B offset)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CFG_srcTrgPortEntry__targetPortEntry_LEN16_TO_PKT_ERSPAN_type_II_header__Index_PLUS4_OFFSET + 4*descRxSniff, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Set ERSPAN.Ver to 1(4b) and ERSPAN.VLAN(12b) to 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Set ERSPAN.Ver to 1(4b) and ERSPAN.VLAN(12b) to 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS to Desc<QoS Mapped UP> */
    __LOG_FW(("Set ERSPAN.COS to Desc<QoS Mapped UP>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_DESC_phal2ppa__qos_mapped_up_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Set ERSPAN.En to 3(2b) */
    __LOG_FW(("Assign ERSPAN.En to 3(2b)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x3_LEN2_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);

    /* Set ERSPAN.T(1b) to Desc<truncated> */
    __LOG_FW(("Assign ERSPAN.T(1b) to Desc<truncated>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_DESC_phal2ppa__truncated_LEN1_TO_PKT_ERSPAN_type_II_header__T_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update packet IPv6 payload length field
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IPv6 payload length field"));


    /* Set IPv6 Header<Payload Length> = Desc<Egress Byte Count>+16 */
    __LOG_FW(("Set IPv6 Header<Payload Length>= Desc<Egress Byte Count>+16"));

    /* First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b).
       Assumption 2 MS bits were already cleared by application when setting IPv6<Payload length> field in shared memory */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b)"));
	ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv6_Header__payload_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv6 payload Length +=16 */
    __LOG_FW(("IPv6 payload Length +=16"));
    ACCEL_CMD_TRIG(ADD_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x10_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);
	__PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set L2 header: MAC DA SA + vlan tag or no vlan tag
     - Update Desc<Egress Byte Count> +=58 or 62(with vlan)
     - Update Desc<fw_bc_modification> +=58 or 62(with vlan)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set L2 header with or without vlan tag, Update Desc<Egress Byte Count> +=58 or 62(with vlan), Update Desc<fw_bc_modification> +=58 or 62(with vlan)"));

    /* Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan */
    __LOG_FW(("Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan"));
    if (vlanVid == 0)
    { /* No vlan */

        /* Indicates HW about the new starting point of the packet header (+58 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+58 bytes)"));
        thr65_desc_ptr->ppa_internal_desc.fw_bc_modification = 58;

        /* No vlan, just copy MAC 12 bytes from thread's template */
        __LOG_FW(("No vlan, just copy MAC 12 bytes from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CFG_erspan_template__mac_da_47_32_LEN12_TO_PKT_IPv6_Header__version_MINUS14_OFFSET, 0);

        /* Desc<Egress Byte Count> +=58 */
        __LOG_FW(("Desc<Egress Byte Count> +=58"));
        ACCEL_CMD_TRIG(ADD_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x3A_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }
    else
    {/* With vlan */

        /* Indicates HW about the new starting point of the packet header (+62 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+62 bytes)"));
        thr65_desc_ptr->ppa_internal_desc.fw_bc_modification = 62;

        /* With vlan, copy MAC(12B) + vlan tag(4B) from thread's template */
        __LOG_FW(("With vlan, copy MAC(12B) + vlan tag(4B) from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CFG_erspan_template__mac_da_47_32_LEN16_TO_PKT_IPv6_Header__version_MINUS18_OFFSET, 0);

        /* Desc<Egress Byte Count> +=62 */
        __LOG_FW(("Desc<Egress Byte Count> +=62"));
        ACCEL_CMD_TRIG(ADD_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x3E_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(65);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR66_enhanced_sFlow
 * inparam      None
 * return       None
 * description  sFlow is a technology for monitoring traffic in data networks containing switches and routers.
 *              The information of input/output port & ingress/egress timestamp of the original packet are sent to a controller.
 *              The controller analyzes the message delay according to the information and draws the forwarding delay jitter curve,
 *              so that users can understand the network transmission quality.
 *
 *              This solution requires usage of loopback port
 *              - in first pass sFlow headers are added by PHA (done by this thread)
 *              - in second pass after packet was loopback to pipeline again
 *                - IPv4/Ipv6-UDP headers are added by HA
 *                - specific fields in IPv4/Ipv6, udp and sFlow are updated by PHA
 *                  (done by THR62_enhanced_sFlow_fill_remain_ipv4 and THR63_enhanced_sFlow_fill_remain_ipv6)
 *
 *              cfg template: | reserved(15B) should be zero | src device id(1B) |
 *              src & trg entries: No usage (4B)
 *              hdr in:    | Extension space (64B) | mirrored packet |
 *              hdr out:   | Extension space (20)  | sFlow shim header(16B) | RX_TS_SHIM (14B) | TX_TS_SHIM (14B) | mirrored packet |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmwares actions:
 *              - Adds sFlow shim header + RX TS shim header + TX TS shim header
 *              - Sets sFlow headers fields according to architect's definitions.
 *              - Updates HW that 44 bytes were added
 *              - Enable PTP TSU
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR66_enhanced_sFlow)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t  ts_sec;                    /* timestamp in seconds                                                                */
    uint32_t  ts_sec_new;                /* holds timestamp in seconds after reading it for the second time to check wraparound */
    uint32_t  ts_nano_sec;               /* timestamp in nano seconds                                                           */
    uint8_t   tai_sel;                   /* TAI select index 0 or 1                                                             */
    uint8_t   smemSeqNumIndx;            /* holds index to sequence number table in shared memory                               */
    uint32_t  smemSeqNumTableEntryAddr;  /* holds address to specific entry in sequence number table in shared memory           */
    uint32_t  seqNumVal;                 /* holds sequence number value                                                         */

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr66_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Get ptp TAI IF select */
    __LOG_FW(("Get ptp TAI IF select"));
    tai_sel = thr66_desc_ptr->phal2ppa.ptp_tai_select;
    __LOG_PARAM_FW(tai_sel);

    /* Get index to shared memory sequence number table. Get it from Desc<flow_id> which reflects target analyzer ID. Applicable values are 0:6. */
    __LOG_FW(("Get index to shared memory sequence number table. Get it from Desc<flow_id> which reflects target analyzer ID. Applicable values are 0:6."));
    smemSeqNumIndx = (thr66_desc_ptr->phal2ppa.flow_id & 0x7);
    __LOG_PARAM_FW(thr66_desc_ptr->phal2ppa.flow_id);
    __LOG_PARAM_FW(smemSeqNumIndx);

    /* Get shared memory address compatible to packet's flow id */
    __LOG_FW(("Get shared memory address compatible to packet's flow id"));
    smemSeqNumTableEntryAddr = DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR + smemSeqNumIndx*4;
    __LOG_PARAM_FW(smemSeqNumTableEntryAddr);

    /* Read sequence number from shared memory */
    __LOG_FW(("Read sequence number from shared memory"));
    seqNumVal = PPA_FW_SP_READ(smemSeqNumTableEntryAddr);
    __LOG_PARAM_FW(seqNumVal);
	__PRAGMA_NO_REORDER

    /* Increment sequence number by 1 */
    __LOG_FW(("Increment sequence number by 1"));
    seqNumVal += 1;
    __LOG_PARAM_FW(seqNumVal);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow shim header (16B)
        Word0 |                 version                     |
        Word1 | src_modid | src_port | dst_modid | dst_port |
        Word2 | flag      | reserved |     user_metadata    |
        Word3 |              sequence_number                |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow shim header. 4 words each of 32 bits."));

    /* Clear Word0, Word1 & word2 */
    __LOG_FW(("Clear Word0, Word1 & word2"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR66_enhanced_sFlow_CFG_enhanced_sflow_template__reserved_0_LEN12_TO_PKT_enhanced_sflow_shim_header__version_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Word0: sFlow Version. Set to fixed value 0x5 */
    __LOG_FW(("Word0: sFlow Version. Set to fixed value 0x5"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_CONST_0x5_LEN3_TO_PKT_enhanced_sflow_shim_header__version_PLUS29_OFFSET, 0);

    /* Word1(bits[31:24]): Set Src_modid (dev id of sample port). Copy it from template[7:0] */
    __LOG_FW(("Word1 (bits[31:24]):Set Src_modid (dev id of sample port). Copy it from template[7:0>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_CFG_enhanced_sflow_template__src_device_id_LEN8_TO_PKT_enhanced_sflow_shim_header__src_modid_OFFSET, 0);

    /* Word1(bits[23:16]): Set Src_port.
       If Desc<Orig is Trunk> copy it from Desc<Orig Src Trunk ID> else copy it from Desc<orig_src_eport>
       Since these 2 fields are muxed, start from same position and required only 8 LS bits, no need to check condition of Desc<Orig is Trunk> */
    __LOG_FW(("Word1 (bits[23:16]):Set Src_port. If Desc<Orig is Trunk> copy it from Desc<Orig Src Trunk ID> else copy it from Desc<orig_src_eport>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_DESC_phal2ppa__orig_src_eport_PLUS5_LEN8_TO_PKT_enhanced_sflow_shim_header__src_port_OFFSET, 0);

    /* Word1(bits[15:0]): Set Dst_modid (from Desc<copy_reserved[17:10]>) and Dst_port (from Desc<copy_reserved[9:2]>) */
    __LOG_FW(("Word1(bits[15:0]): Set Dst_modid (from Desc<copy_reserved[17:10]>) and Dst_port (from Desc<copy_reserved[9:2]>)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_DESC_phal2ppa__copy_reserved_PLUS2_LEN16_TO_PKT_enhanced_sflow_shim_header__dst_modid_OFFSET, 0);

    /* Word2: Clear flag, reserved & user_metadata (already done) */
    __LOG_FW(("Word2: Clear flag, reserved & user_metadata (already done)"));

    /* Word3: Set sequence number:
       - Save updated sequence number value in shared memory seq num table
       - Set sequence number in packet  */
    __LOG_FW(("Word3: Set sequence number: Save updated counter in shared memory seq num table & set it in packet"));
    PPA_FW_SP_WRITE(smemSeqNumTableEntryAddr,seqNumVal);
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 32 , seqNumVal );


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow RX timestamp shim header (14B)
        Word4 |               RX TS CS                       |
        Word5 | rx_timestamp_47_32      | rx_timestamp_31_16 |
        Word6 | rx_timestamp_15_0       | reserved | ip_high |
        Word7 | ip_high | port(num,dir) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow RX timestamp shim header (14B)"));

    /* Clear word4,word5 and word6 */
    __LOG_FW(("Clear word4,word5 and word6"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR66_enhanced_sFlow_CFG_enhanced_sflow_template__reserved_0_LEN12_TO_PKT_enhanced_sflow_rx_ts_shim_header__checksum_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Word5 (bits[31:16]): Set rx timestamp[47:32] (16 bits of seconds part) */
    __LOG_FW(("Word5 (bits[31:16]): Set rx timestamp[47:32] (16 bits of seconds part)"));

    /* Read TOD word 1 which represents seconds field [31:0] */
    __LOG_FW(("Read TOD word 1 which represents seconds field [31:0]"));
    ts_sec = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec);

    /* Read TOD word 0 which represents nano seconds field */
    __LOG_FW(("Read TOD word 0 which represents nano seconds field"));
    ts_nano_sec = READ_TOD_IF_REG(TOD_WORD_0, tai_sel);
    __LOG_PARAM_FW(ts_nano_sec);
	__PRAGMA_NO_REORDER

    /* Check for wraparound */
    __LOG_FW(("Read again seconds (TOD word 1) to check for wraparound"));
    ts_sec_new = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec_new);

    /* < 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound */
    __LOG_FW(("< 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound"));
    if ( (ts_sec_new != ts_sec) && (ts_nano_sec < 500000000) )
    {
        __LOG_FW(("Update ts_sec with new value since wraparound occurred"));
        ts_sec = ts_sec_new ;
        __LOG_PARAM_FW(ts_sec);
    }

    /* If not equal, mean TAI second time is later than RX more than one second */
    __LOG_FW(("If not equal, mean TAI second time is later than RX more than one second"));
    __LOG_PARAM_FW(ts_sec);
    __LOG_PARAM_FW(thr66_desc_ptr->phal2ppa.timestamp);
    if ( (ts_sec & 0x3) != ((thr66_desc_ptr->phal2ppa.timestamp >> 30) & 0x3) )
    {
        ts_sec -=1;
    }

    /* Extract low 16 bits */
    __LOG_FW(("Extract low 16 bits"));
    ts_sec &= 0xFFFF;

    /* Set timestamp seconds in sFlow header */
    __LOG_FW(("Set timestamp seconds in sFlow header"));
    PPA_FW_SP_SHORT_WRITE( FALCON_PKT_REGs_lo + 40 , ts_sec );

    /* Word5 (bits[15:0]): Set rx timestamp[31:16] (0,0, 14 high bits of nanosec part) */
    __LOG_FW(("Word5 (bits[15:0]): Set rx timestamp[31:16] (0,0, 14 high bits of nanosec part)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_DESC_phal2ppa__timestamp_PLUS2_LEN14_TO_PKT_enhanced_sflow_rx_ts_shim_header__rx_timestamp_31_16_PLUS2_OFFSET, 0);

    /* Word6 (bits[31:16] & bits[15:8]): Set rx timestamp[15:0] (16 low bits of nanosec part) plus reserved field which was already cleared */
    __LOG_FW(("Word6 (bits[31:16] & bits[15:8]): Set rx timestamp[15:0] (16 low bits of nanosec part) plus reserved field which was already cleared"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_DESC_phal2ppa__timestamp_PLUS16_LEN16_TO_PKT_enhanced_sflow_rx_ts_shim_header__rx_timestamp_15_0_OFFSET, 0);

    /* Word6 (bits[7:0]) & Clear MS byte of IP_high. Was already cleared */
    __LOG_FW(("Word6 (bits[7:0]) & Clear MS byte of IP_high. Was already cleared"));

    /* Word7: Clear |ip_high | port(num,dir) | TX TS CS(2MSB) | and set port direction (1 bit) to '0' */
    __LOG_FW(("Word7: Clear |ip_high | port(num,dir) | TX TS CS(2MSB) | and set port direction (1 bit) to '0'"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_CONST_0x0_LEN1_TO_PKT_enhanced_sflow_rx_ts_shim_header__port_direction_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
	__PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow TX timestamp shim header (14B)
        Word7 |                        | TX TS CS              |
        Word8 | TX TS CS               | tx_timestamp_47_32    |
        Word9 | tx_timestamp_31_16     | tx_timestamp_15_0     |
        Word10| reserved |      ip_low         | port(num,dir) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow TX timestamp shim header (14B)"));

    /* Clear Word8,Word9 & Word10 (TX_TS_CS, tx_timestamp,reserved,ip_low & port info)*/
    __LOG_FW(("Clear Word8,Word9 & Word10 (TX_TS_CS, tx_timestamp,reserved,ip_low & port info)"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR66_enhanced_sFlow_CFG_enhanced_sflow_template__reserved_0_LEN12_TO_PKT_enhanced_sflow_tx_ts_shim_header__checksum_PLUS2_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Word10 (bit[0]): Set target port direction to '1' */
    __LOG_FW(("Word10 (bit[0]): Set target port direction to '1'"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_CONST_0x1_LEN1_TO_PKT_enhanced_sflow_tx_ts_shim_header__port_direction_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Enable PTP TSU
        - Desc<MAC Timestamping En>    = 1
        - Desc<PTP Packet Format>      = PTPv2 (0x0)
        - Desc<PTP Action>             = AddTime (0x4)
        - Desc<Timestamp Mask Profile> = 0x2
        - Desc<Timestamp Offset>       = 34 ( sFlow shim(16B) + rx ts shim(14B) + tx ts cs(4B) )
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Enable PTP TSU"));

    /* Desc<MAC Timestamping En>=1 */
    __LOG_FW(("Desc<MAC Timestamping En>=1"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_CONST_0x1_LEN1_TO_DESC_phal2ppa__mac_timestamping_en_OFFSET, 0);

    /* Desc<PTP Packet Format>=PTPv2 (0x0) */
    __LOG_FW(("Desc<PTP Packet Format>=PTPv2 (0x0)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_CONST_0x0_LEN3_TO_DESC_phal2ppa__ptp_packet_format_OFFSET, 0);

    /* Desc<PTP Action>=AddTime (0x4) */
    __LOG_FW(("Desc<PTP Action>=AddTime (0x4)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_CONST_0x4_LEN4_TO_DESC_phal2ppa__ptp_action_OFFSET, 0);

    /* Desc<Timestamp Mask Profile>=2 */
    __LOG_FW(("Desc<Timestamp Mask Profile>=2"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_CONST_0x2_LEN3_TO_DESC_phal2ppa__timestamp_mask_profile_OFFSET, 0);

    /* Set Desc<Timestamp Offset> which points to the byte offset of “Tx Timestamp” field of TX_TS_SHIM relative to the start of the packet
       Set to fix offset value of 34B ( SFLOW_SHIM_HDR(16B)+RX_TS_SHIM(14B)+TX_CS(4B) ) */
    __LOG_FW(("Set Desc<Timestamp Offset> which points to the byte offset of “Tx Timestamp” field of TX_TS_SHIM relative to the start of the packet"));
    __LOG_FW(("Set to fix offset value of 34B ( SFLOW_SHIM_HDR(16B)+RX_TS_SHIM(14B)+TX_CS(4B) )"));
    ACCEL_CMD_TRIG(COPY_BITS_THR66_enhanced_sFlow_CONST_0x22_LEN8_TO_DESC_phal2ppa__timestamp_offset_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+44 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+44 bytes)."));
    thr66_desc_ptr->ppa_internal_desc.fw_bc_modification = 44;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(66);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR67_Forward_DSA_Routed_Clear
 * inparam      None
 * return       None
 * description  Thread that is used to clear Forward DSA<Routed> bit.
 *              cfg template     : Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  | Extension space (32B) | MAC addresses(12B) | eDSA tag(16B) |
 *              hdr out: | Extension space (32B) | MAC addresses(12B) | eDSA tag(16B) |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment
 *              Firmware fills eDSA.W1.Routed bit with 0
 *              Sets eDSA<Routed>=0
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              The header remains with the same size no addition here.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR67_Forward_DSA_Routed_Clear)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update eDSA tag
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* eDSA<Routed> = 0 */
    __LOG_FW(("eDSA<Routed> = 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR67_Forward_DSA_Routed_Clear_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w1__Routed_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(67);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR68_Erspan_TypeI_Ipv4
 * inparam      None
 * return       None
 * description  ERSPAN over IPv4 tunnel. Encapsulated Remote Switch Port Analyzer (ERSPAN)
 *              allows packets to be mirrored to a remote analyzer over a tunnel encapsulation.
 *              - ERSPAN Type I is used
 *              - ERSPAN is sent over an IPv4 GRE tunnel
 *              - Support handling vlan tagged or untagged
 *
 *              cfg template:    Not in use (16B). It is used by fw to hold L2 parameters copied from shared memory.
 *              src & trg entries:  Not in use (4B)
 *
 *              hdr in:  | Extension space (64B) | payload |
 *              hdr out: | Extension space (22B or 26B) | MAC addresses(12B) | VLAN(4B) or not | ET(2B) | IPv4(20B) | GRE(4B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              - Add IPv4 and L2 headers which are copied from shared memory
 *              - Add GRE and set it according to Architect's definitions
 *              - Set IPv4 length and calculate IPv4 cs.
 *              - Update Desc<egress_byte_count> +=42(with vlan) or 38(without vlan)
 *              - Update Desc<fw_bc_modification> +=42(with vlan) or 38(without vlan)
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR68_Erspan_TypeI_Ipv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    smemAnalyzerTemplateBaseAddr;  /* holds base address of sFlow templates in shared memory         */
    uint32_t    smemAnalayzerTemplateOfst;     /* holds base address of specific sFlow template in shared memory */
    uint32_t    vlanVid;                       /* holds value of vlan vid taken from shared memory template      */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr68_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr68_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr68_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template
      - To avoid 32 bits alignment issues (packet starts after 22 bytes since Ethertype included), copy it first to CFG template and then to packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0)  , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4)  , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8)  , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 12) );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 16) );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 20B of IPv4 header from shared memory and place them in output packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 20B of IPv4 header from shared memory and place them in output packet"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));
     __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Get VLAN vid
     ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Get vlan vid from template */
    __LOG_FW(("Get vlan vid from template"));
    vlanVid = PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 12) & 0xFFF;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x0800 (IPv4)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x0800 (IPv4)"));

    ACCEL_CMD_TRIG(COPY_BITS_THR68_Erspan_TypeI_Ipv4_CONST_0x08_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (4B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (4B)"));

    /* Set GRE.Protocol MS byte to 0x88 and clear GRE.flags to 0x0 */
    __LOG_FW(("Set GRE.Protocol MS byte to 0x88 and clear GRE.flags to 0x0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR68_Erspan_TypeI_Ipv4_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Set GRE.Protocol LS byte to 0xBE */
    __LOG_FW(("Set GRE.Protocol LS byte to 0xBE"));
    ACCEL_CMD_TRIG(COPY_BITS_THR68_Erspan_TypeI_Ipv4_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update packet IPv4 length field
     - Calculate IPv4 checksum
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IPv4 length field + calculate IPv4 checksum"));


    /* Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+24 */
    __LOG_FW(("Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+24"));

    /* First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b).
       Assumption 2 MS bits were already cleared by application when setting IPv4<Total length> field in shared memory */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b)"));
	ACCEL_CMD_TRIG(COPY_BITS_THR68_Erspan_TypeI_Ipv4_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv4_Header__total_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv4 total Length +=24 */
    __LOG_FW(("IPv4 total Length +=24"));
    ACCEL_CMD_TRIG(ADD_BITS_THR68_Erspan_TypeI_Ipv4_CONST_0x18_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Compute IPv4 CS */
    __LOG_FW(("Compute IPv4 CS"));

    /* Sum first 8 bytes of IPv4 */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR68_Erspan_TypeI_Ipv4_LEN8_FROM_PKT_IPv4_Header__version_OFFSET, 0);

    /* Skip cs field and add to sum 2B of ttl and protocol */
    __LOG_FW(("Skip cs field and add to sum 2B of ttl and protocol"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR68_Erspan_TypeI_Ipv4_LEN2_FROM_PKT_IPv4_Header__ttl_OFFSET, 0);

    /* Sum the last 8 bytes of IPv4 (src and dest IP addresses fields) */
    __LOG_FW(("Sum the last 8 bytes of IPv4 (src and dest IP addresses fields)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR68_Erspan_TypeI_Ipv4_LEN8_FROM_PKT_IPv4_Header__sip_high_OFFSET, 0);
	__PRAGMA_NO_REORDER

    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR68_Erspan_TypeI_Ipv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set L2 header: MAC DA SA + vlan tag or no vlan tag
     - Update Desc<Egress Byte Count> +=38 or 42(with vlan)
     - Update Desc<fw_bc_modification> +=38 or 42(with vlan)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set L2 header with/without vlan tag, Update Desc<Egress Byte Count> +=42/38, Update Desc<fw_bc_modification> +=42/38"));

    /* Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan */
    __LOG_FW(("Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan"));
    if (vlanVid == 0)
    { /* No vlan */

        /* Indicates HW about the new starting point of the packet header (+38 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+38 bytes)"));
        thr68_desc_ptr->ppa_internal_desc.fw_bc_modification = 38;

        /* No vlan, just copy MAC 12 bytes from thread's template */
        __LOG_FW(("No vlan, just copy MAC 12 bytes from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR68_Erspan_TypeI_Ipv4_CFG_erspan_template__mac_da_47_32_LEN12_TO_PKT_IPv4_Header__version_MINUS14_OFFSET, 0);

        /* Desc<Egress Byte Count> +=38 */
        __LOG_FW(("Desc<Egress Byte Count> +=38"));
        ACCEL_CMD_TRIG(ADD_BITS_THR68_Erspan_TypeI_Ipv4_CONST_0x26_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }
    else
    {/* With vlan */

        /* Indicates HW about the new starting point of the packet header (+42 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+42 bytes)"));
        thr68_desc_ptr->ppa_internal_desc.fw_bc_modification = 42;

        /* With vlan, copy MAC(12B) + vlan tag(4B) from thread's template */
        __LOG_FW(("With vlan, copy MAC(12B) + vlan tag(4B) from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR68_Erspan_TypeI_Ipv4_CFG_erspan_template__mac_da_47_32_LEN16_TO_PKT_IPv4_Header__version_MINUS18_OFFSET, 0);

        /* Desc<Egress Byte Count> +=42 */
        __LOG_FW(("Desc<Egress Byte Count> +=42"));
        ACCEL_CMD_TRIG(ADD_BITS_THR68_Erspan_TypeI_Ipv4_CONST_0x2A_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(68);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR69_Erspan_TypeI_Ipv6
 * inparam      None
 * return       None
 * description  ERSPAN over IPv6 tunnel. Encapsulated Remote Switch Port Analyzer (ERSPAN)
 *              allows packets to be mirrored to a remote analyzer over a tunnel encapsulation.
 *              - ERSPAN Type I is used
 *              - ERSPAN is sent over an IPv6 GRE tunnel
 *              - Support handling vlan tagged or untagged
 *
 *              cfg template:    Not in use (16B). It is used by fw to hold L2 parameters copied from shared memory.
 *              src & trg entries:  Not in use (4B)
 *
 *              hdr in:  | Extension space (64B) | payload |
 *              hdr out: | Extension space (2B or 6B) | MAC addresses(12B) | VLAN(4B) or not | ET(2B) | IPv6(40B) | GRE(4B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              - Add IPv6 and L2 headers which are copied from shared memory
 *              - Add GRE and set it according to Architect's definitions
 *              - Set IPv6 length
 *              - Update Desc<egress_byte_count> +=62(with vlan) or 58(without vlan)
 *              - Update Desc<fw_bc_modification> +=62(with vlan) or 58(without vlan)
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR69_Erspan_TypeI_Ipv6)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    smemAnalyzerTemplateBaseAddr;  /* holds base address of sFlow templates in shared memory         */
    uint32_t    smemAnalayzerTemplateOfst;     /* holds base address of specific sFlow template in shared memory */
    uint32_t    vlanVid;                       /* holds value of vlan vid taken from shared memory template      */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr69_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr69_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr69_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template
      - To avoid 32 bits alignment issues (packet starts after 2 bytes since Ethertype included), copy it first to CFG template and then to packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0)  , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4)  , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8)  , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 12) );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 16) );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 40B of IPv6 header from shared memory and place them in output packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 40B of IPv6 header from shared memory and place them in output packet"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 20) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 24) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 28) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 32) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 36) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36));
     __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Get VLAN vid
     ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Get vlan vid from template */
    __LOG_FW(("Get vlan vid from template"));
    vlanVid = PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 12) & 0xFFF;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x86DD (IPv6)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x86DD (IPv6)"));

    ACCEL_CMD_TRIG(COPY_BITS_THR69_Erspan_TypeI_Ipv6_CONST_0x86_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR69_Erspan_TypeI_Ipv6_CONST_0xDD_LEN8_TO_PKT_ethertype_header__ethertype_PLUS8_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (4B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (4B)"));

    /* Set GRE.Protocol MS byte to 0x88 and clear GRE.flags to 0x0 */
    __LOG_FW(("Set GRE.Protocol MS byte to 0x88 and clear GRE.flags to 0x0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR69_Erspan_TypeI_Ipv6_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
   	__PRAGMA_NO_REORDER

    /* Set GRE.Protocol LS byte to 0xBE */
    __LOG_FW(("Set GRE.Protocol LS byte to 0xBE"));
    ACCEL_CMD_TRIG(COPY_BITS_THR69_Erspan_TypeI_Ipv6_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update packet IPv6 length field
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IPv6 length field"));


    /* Set IPv6 Header<Payload Length> = Desc<Egress Byte Count>+4 */
    __LOG_FW(("Set IPv6 Header<Payload Length> = Desc<Egress Byte Count>+4"));

    /* First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b).
       Assumption 2 MS bits were already cleared by application when setting IPv6<Payload Length> field in shared memory */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b)"));
	ACCEL_CMD_TRIG(COPY_BITS_THR69_Erspan_TypeI_Ipv6_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv6_Header__payload_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv6 Payload Length +=4 */
    __LOG_FW(("IPv6 Payload Length +=4"));
    ACCEL_CMD_TRIG(ADD_BITS_THR69_Erspan_TypeI_Ipv6_CONST_0x4_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set L2 header: MAC DA SA + vlan tag or no vlan tag
     - Update Desc<Egress Byte Count> +=58 or 62(with vlan)
     - Update Desc<fw_bc_modification> +=58 or 62(with vlan)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set L2 header with/without vlan tag, Update Desc<Egress Byte Count> +=62/58, Update Desc<fw_bc_modification> +=62/58"));

    /* Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan */
    __LOG_FW(("Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan"));
    if (vlanVid == 0)
    { /* No vlan */

        /* Indicates HW about the new starting point of the packet header (+58 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+58 bytes)"));
        thr69_desc_ptr->ppa_internal_desc.fw_bc_modification = 58;

        /* No vlan, just copy MAC 12 bytes from thread's template */
        __LOG_FW(("No vlan, just copy MAC 12 bytes from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR69_Erspan_TypeI_Ipv6_CFG_erspan_template__mac_da_47_32_LEN12_TO_PKT_IPv6_Header__version_MINUS14_OFFSET, 0);

        /* Desc<Egress Byte Count> +=58 */
        __LOG_FW(("Desc<Egress Byte Count> +=58"));
        ACCEL_CMD_TRIG(ADD_BITS_THR69_Erspan_TypeI_Ipv6_CONST_0x3A_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }
    else
    {/* With vlan */

        /* Indicates HW about the new starting point of the packet header (+62 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+62 bytes)"));
        thr69_desc_ptr->ppa_internal_desc.fw_bc_modification = 62;

        /* With vlan, copy MAC(12B) + vlan tag(4B) from thread's template */
        __LOG_FW(("With vlan, copy MAC(12B) + vlan tag(4B) from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR69_Erspan_TypeI_Ipv6_CFG_erspan_template__mac_da_47_32_LEN16_TO_PKT_IPv6_Header__version_MINUS18_OFFSET, 0);

        /* Desc<Egress Byte Count> +=62 */
        __LOG_FW(("Desc<Egress Byte Count> +=62"));
        ACCEL_CMD_TRIG(ADD_BITS_THR69_Erspan_TypeI_Ipv6_CONST_0x3E_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(69);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR71_IFA_Header
 * inparam      None
 * return       None
 * description  Handle Inband Flow Analyzer (IFA) for Ingress node Pass#1 with IPv4/IPv6 UDP/TCP packet use case
 *              It is used to monitor and analyze packets as they enter and exit the network.
 *              It collects data at each hop and at the end of path it is sent to collector to analyze.
 *              This specific thread adds IFA header, RX timestamp metadata and vlan with local source port.
 *
 *              cfg template: [15:12]: Probe maker 1
 *                            [11:08]: Probe maker 2
 *                            [    7]: Hop limit
 *                            [06:05]: Telemetry Request Vector
 *                            [04:03]: Sender Handle
 *                            [02:00]: Reserved
 *              src & trg entries:  Not in use (4B)
 *
 *              hdr in:  | Extension space (64B) | MAC DA SA | VLAN | ET | IPv4/IPv6 | UDP/TCP | payload |
 *              hdr out: | Extension space (0B)  | MAC DA SA | VLAN Outer | VLAN | ET | IPv4/IPv6 | UDP/TCP | IFA HDR(28B) | IFA MD(32) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware does the following:
 *              - Adds 28 bytes IFA header and set values according to architect's definitions
 *              - Adds 32 bytes IFA Metadata. Set RX timestamp and clear all rest.
 *              - Updates IP and UDP lengths and calculate checksums
 *              - Adds 4 bytes of vlan and set vid with Desc<Local Dev Src Port>
 *              - Updates Desc<fw_bc_modification> +=64(vlan+IFA header+IFA MD)
 *
 *              Note: accelerators are written as IPv4 UDP but they are used also for IPv6 and TCP
 *                    accelerators are updated during thread with the compatible offsets
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR71_IFA_Header)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    l3Offset;              /* IPv4/IPv6 byte offset                      */
    uint32_t    l4Offset;              /* UDP/TCP byte offset relative to l3Offset   */
    uint32_t    pktType;               /* IPv4 or IPv6                               */
    uint32_t    l3ProtocolOffset;      /* byte offset of L3 Protocol                 */
    uint32_t    l3ProtocolValue;       /* L3 Protocol (udp or tcp)                   */
    uint32_t    ifaHeaderStartOffset;  /* IFA header offset relative to packet start */
    uint32_t    l4ChecksumOffset;      /* UDP/TCP checksum field byte offset         */
    uint32_t    l3LengthOffset;        /* IP length field byte offset                */
    uint32_t    l3LengthValue;         /* IP length field value                      */
    uint32_t    l3CsOffset;            /* IPv4 checksum field byte offset            */
    uint32_t    l4LengthOffset;        /* UDP length field byte offset               */
    uint32_t    l4LengthValue;         /* UDP length field value                     */
    uint32_t    ifaMdStartOffset;      /* IFA Metadata byte offset                   */
    uint32_t    bytesToShift;          /* number of bytes to shift left              */
    uint32_t    incomingL4CsValue;     /* current/old UDP checksum                   */
    uint32_t    ts_31_30;              /* holds Desc<timestamp[31:30]> value         */
    uint32_t    tai_sel;               /* TAI select index 0 or 1                    */
    uint32_t    ts_sec;                /* timestamp in seconds                       */
    uint32_t    isIngressTimestamp;    /* add or not ingress timestamp to IFA MD     */
    uint32_t    smemFlowInfoBaseAddr;  /* base address of Flow Entry in Shared RAM   */
    uint32_t    ifaEntryWord;          /* IFA flow entry word                        */
    uint16_t    twoBytes;              /* content of two bytes                       */
    uint32_t    flagsAndActionsVector; /* holds Flags, Action Vector from template   */

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr71_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr71_ifa_header_cfg* thr71_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr71_ifa_header_cfg);


#ifdef ASIC_SIMULATION
    /* To make debug easier I print out the input packet as it comes to firmware so it can be seen in WM log
       It skips the expansion space and prints out the maximum bytes that can be (mac,vlan,et,IPv6,TCP) */
    __LOG_FW(("To make debug easier I print out the input packet as it comes to firmware so it can be seen in WM log"));
    __LOG_FW(("It skips the expansion space and prints out the maximum bytes that can be (mac,vlan,et,IPv6,TCP)"));
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 0) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 4) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 8) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 12) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 16) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 20) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 24) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 28) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 32) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 36) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 40) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 44) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 48) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 52) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 56) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 60) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 64) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 68) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 72) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_64B_SIZE + 76) );
#endif /* ASIC_SIMULATION */

    /* Find L3, L4 offsets and packet type */
    __LOG_FW(("Find L3, L4 offsets and packet type"));
    l3Offset = thr71_desc_ptr->phal2ppa.egress_outer_l3_offset;
    l4Offset = thr71_desc_ptr->phal2ppa.l4_offset;
    pktType  = thr71_desc_ptr->phal2ppa.egress_outer_packet_type;
    isIngressTimestamp = thr71_cfg_ptr->ifa_header_template.telemetryRequestVector_ingressTimestamp;
    __LOG_PARAM_FW(l3Offset);
    __LOG_PARAM_FW(l4Offset);
    __LOG_PARAM_FW(pktType);
    __LOG_PARAM_FW(isIngressTimestamp);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Check IFA packet is valid for processing
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Check IFA packet is valid for processing"));

    if (pktType == 0) /* IPv4 */
    {
        l3ProtocolOffset = EXPANSION_SPACE_64B_SIZE + l3Offset + 9;
        l3ProtocolValue  = PPA_FW_SP_BYTE_READ( (FALCON_PKT_REGs_lo + l3ProtocolOffset) );
    }
    else if (pktType == 1) /* IPv6 */
    {
        l3ProtocolOffset = EXPANSION_SPACE_64B_SIZE + l3Offset + 6;
        l3ProtocolValue  = PPA_FW_SP_BYTE_READ( (FALCON_PKT_REGs_lo + l3ProtocolOffset) );
    }
    else
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6 */
        __LOG_FW(("Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6"));
        thr71_desc_ptr->phal2ppa.egress_packet_cmd = HARD_DROP;
        __LOG_PARAM_FW(thr71_desc_ptr->phal2ppa.egress_packet_cmd);

        goto exit_thread71;
    }

    if ((l3ProtocolValue != 6) && (l3ProtocolValue != 17))
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Set Desc<egress_packet_cmd> to drop since IFA must be over UDP or TCP */
        __LOG_FW(("Set Desc<egress_packet_cmd> to drop since IFA must be over UDP or TCP"));
        thr71_desc_ptr->phal2ppa.egress_packet_cmd = HARD_DROP;
        __LOG_PARAM_FW(thr71_desc_ptr->phal2ppa.egress_packet_cmd);

        goto exit_thread71;
    }
    __LOG_PARAM_FW(l3ProtocolOffset);
    __LOG_PARAM_FW(l3ProtocolValue);
    __LOG_PARAM_FW(thr71_desc_ptr->phal2ppa.egress_packet_cmd);


    if (pktType == 0) /* IPv4 */
    {
        __LOG_FW(("Packet of IPv4 type: handle IPv4 actions"));

        /* Find L3 length and L3 checksum offsets */
        __LOG_FW(("Find L3 length and L3 checksum offsets"));
        l3LengthOffset   = EXPANSION_SPACE_64B_SIZE + l3Offset + 2;
        l3CsOffset       = EXPANSION_SPACE_64B_SIZE + l3Offset + 10;
        __LOG_PARAM_FW(l3LengthOffset);
        __LOG_PARAM_FW(l3CsOffset);


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update IP length field: +60 (IFA header + IFA MD)
          - Calculate IPv4 checksum (incremental update):   New checksum = ~( ~(old checksum) + ~(old Total Length) + (new Total Length) )
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        __LOG_FW(("Update IP length field + calculate IPv4 checksum"));

        /* Since packet fields are not in constant location, we build in front the 32 bits accelerator and during thread
           after we calculate the location we overwrite only source/dest field in the accelerator command */
        __LOG_FW(("Update source/dest fields of some global accelerator commands"));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, ( (PKT<<7) | l3LengthOffset ));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR71_IFA_Header_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, ( (PKT<<7) | l3LengthOffset ));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, ( (PKT<<7) | l3CsOffset ));
        ACCEL_CMD_LOAD_PKT_DEST(CSUM_STORE_IP_THR71_IFA_Header_TO_PKT_IPv4_Header__header_checksum_OFFSET, ( (PKT<<7) | l3CsOffset ));
	    __PRAGMA_NO_REORDER

        /* Sum old Total Length */
        __LOG_FW(("Sum old Total Length"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Update IPv4<Total Length> */
        __LOG_FW(("Update IPv4<Total Length>"));
        l3LengthValue = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l3LengthOffset) );
        __LOG_PARAM_FW(l3LengthValue);
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + l3LengthOffset), l3LengthValue + 60 );
	    __PRAGMA_NO_REORDER

        /* Sum new Total Length */
        __LOG_FW(("Sum new Total Length using accelerator cmd"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR71_IFA_Header_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);

        /* Sum current/old IPv4 CS */
        __LOG_FW(("Sum current/old IPv4 CS using accelerator cmd"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Store the new IPv4 CS */
        ACCEL_CMD_TRIG(CSUM_STORE_IP_THR71_IFA_Header_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);

        /* Update current/old L4 length (decrement IP header since UDP length does not include it) */
        __LOG_FW(("Update current/old L4 length (decrement IP header since UDP length does not include it)"));
        l4LengthValue = l3LengthValue - IPV4_BASIC_HEADER_SIZE;
    }
    else /* IPv6 */
    {
        __LOG_FW(("Packet of IPv6 type: handle IPv6 actions"));

        /* Find L3 length offset */
        __LOG_FW(("Find L3 length offset"));
        l3LengthOffset   = EXPANSION_SPACE_64B_SIZE + l3Offset + 4;
        __LOG_PARAM_FW(l3LengthOffset);

        /* Update IPv6<Payload Length> */
        __LOG_FW(("Update IPv6<Payload Length>"));
        l3LengthValue = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l3LengthOffset) );
        __LOG_PARAM_FW(l3LengthValue);
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + l3LengthOffset), l3LengthValue + 60 );

        /* Update current/old L4 length */
        __LOG_FW(("Update current/old L4 length"));
        l4LengthValue = l3LengthValue;
    }

    if (l3ProtocolValue == 17) /* UDP */
    {
        __LOG_FW(("Packet of UDP type: handle UDP actions"));

        /* Find IFA header,L4 checksum, L4 length offsets and incoming UDP CS value */
        __LOG_FW(("Find IFA header,L4 checksum, L4 length offsets and incoming UDP CS value"));
        ifaHeaderStartOffset = l3Offset + l4Offset + UDP_HDR_SIZE;
        l4ChecksumOffset     = EXPANSION_SPACE_64B_SIZE + l3Offset + l4Offset + 6;
        l4LengthOffset       = EXPANSION_SPACE_64B_SIZE + l3Offset + l4Offset + 4;
        incomingL4CsValue    = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l4ChecksumOffset) );
        __LOG_PARAM_FW(ifaHeaderStartOffset);
        __LOG_PARAM_FW(l4ChecksumOffset);
        __LOG_PARAM_FW(l4LengthOffset);
        __LOG_PARAM_FW(incomingL4CsValue);


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update UDP length field
          - Start UDP CS calculation   New checksum = ~( ~(old checksum) + ~(old value of a 16-bit field) + (new value of a 16-bit field)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        __LOG_FW(("Update UDP length, start to calculate new UDP CS "));

        /* Update source fields of accelerator commands */
        __LOG_FW(("Update source fields of accelerator commands"));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, ( (PKT<<7) | l4ChecksumOffset ));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_udp_header__Length_OFFSET, ( (PKT<<7) | l4LengthOffset ));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR71_IFA_Header_LEN2_FROM_PKT_udp_header__Length_OFFSET, ( (PKT<<7) | l4LengthOffset ));
	    __PRAGMA_NO_REORDER

        /* Sum old L4 checksum (2 Bytes) */
        __LOG_FW(("Sum old L4 checksum (2 Bytes)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, 0);

        /* Sum old L4 length (2 Bytes) (IP Pseudo Header) */
        __LOG_FW(("Sum old L4 length (2 Bytes) (IP Pseudo Header)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);

        /* Sum old UDP length (2 Bytes) from udp header */
        __LOG_FW(("Sum old UDP length (2 Bytes) from udp header"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);

        /* Update new UDP length */
        __LOG_FW(("Update new UDP length"));
        l4LengthValue = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l4LengthOffset) );
        __LOG_PARAM_FW(l4LengthValue);
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + l4LengthOffset), l4LengthValue + 60 );
        __PRAGMA_NO_REORDER

        /* Sum new UDP Length */
        __LOG_FW(("Sum new UDP Length using accelerator cmd"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR71_IFA_Header_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);
    }
    else /* TCP */
    {
        __LOG_FW(("Packet of TCP type: handle TCP actions"));

        /* Find IFA header and L4 checksum offsets */
        __LOG_FW(("Find IFA header and L4 checksum offsets"));
        ifaHeaderStartOffset = l3Offset + l4Offset + TCP_HDR_SIZE;
        l4ChecksumOffset     = EXPANSION_SPACE_64B_SIZE + l3Offset + l4Offset + 16;
        __LOG_PARAM_FW(ifaHeaderStartOffset);
        __LOG_PARAM_FW(l4ChecksumOffset);

        /* Set incoming L4 checksum to some none zero value so it will be written to packet field */
        __LOG_FW(("Set incoming L4 checksum to some none zero value so it will be written to packet field"));
        incomingL4CsValue = 0x1;


        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Start TCP CS calculation   New checksum = ~( ~(old checksum) + ~(old value of a 16-bit field) + (new value of a 16-bit field)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        __LOG_FW(("Start TCP CS calculation"));

        /* Sum old TCP checksum (2 Bytes) */
        __LOG_FW(("Sum old TCP checksum (2 Bytes)"));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, ( (PKT<<7) | l4ChecksumOffset ));
	    __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, 0);

        /* Set template reserved area with TCP length */
        __LOG_FW(("Set template reserved area with TCP length"));
        PPA_FW_SP_SHORT_WRITE((FALCON_CFG_REGs_lo + 14), l4LengthValue);
	    __PRAGMA_NO_REORDER

        /* Sum old TCP length (2 Bytes) (IP Pseudo Header) */
        __LOG_FW(("Sum old TCP length (2 Bytes) (IP Pseudo Header)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR71_IFA_Header_LEN2_FROM_CFG_ifa_header_template__reserved_PLUS1_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Update new TCP length */
        __LOG_FW(("Update new TCP length"));
        ACCEL_CMD_TRIG(ADD_BITS_THR71_IFA_Header_CONST_0x3C_LEN16_TO_CFG_ifa_header_template__reserved_PLUS1_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Sum new TCP Length */
        __LOG_FW(("Sum new TCP Length"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR71_IFA_Header_LEN2_FROM_CFG_ifa_header_template__reserved_PLUS1_OFFSET, 0);
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move left input packet (MAC,vlan,ET,IP,UDP) by 28 + 32 bytes to make room for IFA header + IFA metadata
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Move left input packet by 28 + 32 bytes to make room for IFA header + IFA metadata"));

    /* Move left the first 32 bytes */
    __LOG_FW(("Move left the first 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_60_BYTES_THR71_IFA_Header_LEN32_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Get the remaining number of bytes to shift.
       - ifaHeaderStartOffset holds number of bytes which needs to be shifted left (MAC,vlan,ET,IP,UDP)
       - decrement this number by 32 since we already shift left 32 bytes */
    __LOG_FW(("Get the remaining number of bytes to shift"));
    __LOG_FW(("- faHeaderStartOffset holds number of bytes which needs to be shifted left (MAC,vlan,ET,IP,UDP)"));
    __LOG_FW(("- decrement this number by 32 since we already shift left 32 bytes"));
    bytesToShift = ifaHeaderStartOffset - 32;

    /* Move left the rest of the packet */
    __LOG_FW(("Move left the rest of the packet"));
    if (bytesToShift > 32)
    {
        ACCEL_CMD_TRIG(SHIFTLEFT_60_BYTES_THR71_IFA_Header_LEN32_FROM_PKT_mac_header__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER
        bytesToShift -= 32;
        ACCEL_CMD_LOAD_LENGTH(SHIFTLEFT_60_BYTES_THR71_IFA_Header_LEN2_FROM_PKT_mac_header__mac_da_47_32_PLUS64_OFFSET,bytesToShift-1);
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTLEFT_60_BYTES_THR71_IFA_Header_LEN2_FROM_PKT_mac_header__mac_da_47_32_PLUS64_OFFSET, 0);
    }
    else
    {
        ACCEL_CMD_LOAD_LENGTH(SHIFTLEFT_60_BYTES_THR71_IFA_Header_LEN2_FROM_PKT_mac_header__mac_da_47_32_PLUS32_OFFSET,bytesToShift-1);
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTLEFT_60_BYTES_THR71_IFA_Header_LEN2_FROM_PKT_mac_header__mac_da_47_32_PLUS32_OFFSET, 0);
    }

    /* Update real location of IFA header relative to start of packet expansion space after we moved the packet left
       - add 4 bytes since 64 bytes of expansion space - shift left by 60 bytes = 4  */
    __LOG_FW(("Update real location of IFA header relative to start of packet expansion space after we moved the packet left"));
    __LOG_FW(("- add 4 bytes since 64 bytes of expansion space - shift left by 60 bytes = 4"));
    ifaHeaderStartOffset +=4;
    __PRAGMA_NO_REORDER

    /* Get base address of the IFA Flow table entry in shared memory */
    __LOG_FW(("Get base address of the IFA Flow table entry in shared memory"));
    smemFlowInfoBaseAddr = DRAM_IFA_FLOW_INFO_TABLE_ADDR + (thr71_desc_ptr->phal2ppa.pha_metadata * DRAM_IFA_FLOW_INFO_TABLE_ENTRY_SIZE);
    __LOG_PARAM_FW(smemFlowInfoBaseAddr);

    /* Get IFA Flow entry information from shared memory
       Read word 1 which includes
                    bits [7:0]   : Hop Limit
                    bits [15:8]  : Egress Port
                    bits [20:16] : Ctrl
                    bits [22:21] : Rep
     */
    __LOG_FW(("Get IFA Flow entry word 1 from shared memory"));
    ifaEntryWord = PPA_FW_SP_READ(smemFlowInfoBaseAddr + 4);
    __LOG_PARAM_FW(ifaEntryWord);

    /* Copy Hop Limit from IFA entry to Template memory */
    __LOG_FW(("Copy Hop Limit from IFA entry to Template memory"));
    thr71_cfg_ptr->ifa_header_template.hopLimit = ifaEntryWord & 0xFF;
    __LOG_PARAM_FW(thr71_cfg_ptr->ifa_header_template.hopLimit);
    __PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Insert 28B IFA Header at ifaHeaderStartOffset
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Insert 28B IFA Header at ifaHeaderStartOffset"));

    /* Update source/dest fields of accelerator commands */
    __LOG_FW(("Update source/dest fields of accelerator commands"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__probeMarker1_LEN8_TO_PKT_ifa_header__probeMarker1_OFFSET,  ( (PKT<<7) | ifaHeaderStartOffset));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__telemetryRequestVector_rsvd_LEN2_TO_PKT_ifa_header__telemetryRequestVector_rsvd_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset + 12)));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR71_IFA_Header_CFG_ifa_header_template__hopLimit_LEN8_TO_PKT_ifa_header__hopLimit_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+16)));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__senderHandle_LEN2_TO_PKT_ifa_header__senderHandle_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset + 24)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR71_IFA_Header_LEN8_FROM_PKT_ifa_header__probeMarker1_OFFSET, ( (PKT<<7) | ifaHeaderStartOffset));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR71_IFA_Header_LEN8_FROM_PKT_ifa_header__probeMarker1_PLUS8_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR71_IFA_Header_LEN8_FROM_PKT_ifa_header__probeMarker1_PLUS16_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+16)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR71_IFA_Header_LEN4_FROM_PKT_ifa_header__probeMarker1_PLUS24_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+24)));
    __PRAGMA_NO_REORDER

    /* IFA Header Word0 + Word1: Probe Marker = Thread Template data bytes [15:8] */
    __LOG_FW(("IFA Header Word0 + Word1: Probe Marker = Thread Template data bytes [15:8]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__probeMarker1_LEN8_TO_PKT_ifa_header__probeMarker1_OFFSET, 0);

    /* IFA Header Word2: Ver=1, MType=1, TID=1, Flag, rest are zeros */
    __LOG_FW(("IFA Header Word2: Ver=1, MType=1, TID=1, Flag, rest are zeros"));
    twoBytes = 0x1001 |
            /* Ctrl */ (((ifaEntryWord >> 16) & 0x1f) << 3) |
            /* Rep  */ (((ifaEntryWord >> 21) & 0x3) << 10);
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 8), twoBytes );

    /* Read Flag and Action Vector from template */
    __LOG_FW(("Read Flag and Action Vector from template"));
    flagsAndActionsVector = thr71_cfg_ptr->ifa_header_template.reserved;
    __LOG_PARAM_FW(flagsAndActionsVector);

    /* Flag is in bits 7:0 of last word of template. TID = 1. */
    twoBytes = (flagsAndActionsVector & 0xFF) | 0x0100;
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 10), twoBytes);

    /* IFA Header Word3: TelemetryRequestVector = Thread Template data bytes [6:5] + TelemetryActionVector from template */
    __LOG_FW(("IFA Header Word3: TelemetryRequestVector = Thread Template data bytes [6:5] + TelemetryActionVector from template "));
    ACCEL_CMD_TRIG(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__telemetryRequestVector_rsvd_LEN2_TO_PKT_ifa_header__telemetryRequestVector_rsvd_OFFSET, 0);

    /* TelemetryActionVector is in bits 23:8 of flagsAndActionsVector */
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 14), (flagsAndActionsVector >> 8) );

    /* IFA Header Word4: Hop Limit = Thread Template data byte [7] Rest of Word4 = 0x0 */
    __LOG_FW(("IFA Header Word4: Hop Limit = Thread Template data byte [7] Rest of Word4 = 0x0"));
    /* First clear Word4 to 0x0 */
    __LOG_FW(("First clear Word4 to 0x0"));
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 16), 0x0 );
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 18), 0x0 );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR71_IFA_Header_CFG_ifa_header_template__hopLimit_LEN8_TO_PKT_ifa_header__hopLimit_OFFSET, 0);

    /* Get IFA Flow entry information from shared memory
       Read word 0 which includes
                 bits [15:0] : Max Length
                 bits [31:16]: Sender Handle     */
    __LOG_FW(("Get IFA Flow entry word 0 from shared memory"));
    ifaEntryWord = PPA_FW_SP_READ(smemFlowInfoBaseAddr);
    __LOG_PARAM_FW(ifaEntryWord);

    /* Copy Sender Handle from IFA entry to Template memory */
    __LOG_FW(("Copy Hop Limit from IFA entry to Template memory"));
    thr71_cfg_ptr->ifa_header_template.senderHandle = ifaEntryWord >> 24;
    __LOG_PARAM_FW(thr71_cfg_ptr->ifa_header_template.senderHandle);
    thr71_cfg_ptr->ifa_header_template.senderHandle_lsb = ifaEntryWord >> 16;
    __LOG_PARAM_FW(thr71_cfg_ptr->ifa_header_template.senderHandle_lsb);
    __PRAGMA_NO_REORDER

    /* IFA Header Word5: Maximum Length Current Length = 0 */
    __LOG_FW(("IFA Header Word5: Maximum Length, Current Length = 0"));
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 20), (ifaEntryWord & 0xFFFF) );
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 22), 0x0000 );

    /* IFA Header Word6: IFA Header Word6: IFAparameter. SenderHandle=Thread Template data bytes [4:3] sequenceNumber = 0 */
    __LOG_FW(("IFA Header Word6: IFA Header Word6: IFAparameter. SenderHandle=Thread Template data bytes [4:3] sequenceNumber = 0"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__senderHandle_LEN2_TO_PKT_ifa_header__senderHandle_OFFSET, 0);
    /*sequenceNumber = 0*/
    PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 26), 0x0000 );
    __PRAGMA_NO_REORDER

    /* Checksum new IFA header (first 8 bytes) */
    __LOG_FW(("Checksum new IFA header (first 8 bytes)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR71_IFA_Header_LEN8_FROM_PKT_ifa_header__probeMarker1_OFFSET, 0);

    /* Checksum new IFA header (next 8 bytes) */
    __LOG_FW(("Checksum new IFA header (next 8 bytes)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR71_IFA_Header_LEN8_FROM_PKT_ifa_header__probeMarker1_PLUS8_OFFSET, 0);

    /* Checksum new IFA header (next 8 bytes) */
    __LOG_FW(("Checksum new IFA header (next 8 bytes)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR71_IFA_Header_LEN8_FROM_PKT_ifa_header__probeMarker1_PLUS16_OFFSET, 0);

    /* Checksum new IFA header (last 4 bytes) */
    __LOG_FW(("Checksum new IFA header (last 4 bytes)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR71_IFA_Header_LEN4_FROM_PKT_ifa_header__probeMarker1_PLUS24_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Insert 32B IFA Metadata at ifaMdStartOffset
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Insert 32B IFA Metadata at ifaMdStartOffset"));

    /* Find IFA Metadata offset */
    __LOG_FW(("Find IFA Metadata offset"));
    ifaMdStartOffset = ifaHeaderStartOffset + IFA_HDR_SIZE;
    __LOG_PARAM_FW(ifaMdStartOffset);

    /* Clear 32B of IFA Metadata */
    __LOG_FW(("Clear 32B of IFA Metadata"));

    /* First clear template which is no longer needed then trigger accelerator to copy 'zero' into metadata area */
    __LOG_FW(("First clear template which is no longer needed then trigger accelerator to copy 'zero' into metadata area"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__probeMarker1_LEN16_TO_PKT_ifa_md__deviceId_OFFSET,  ( (PKT<<7) | ifaMdStartOffset));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__probeMarker1_LEN16_TO_PKT_ifa_md__deviceId_PLUS16_OFFSET,  ( (PKT<<7) | (ifaMdStartOffset+16)));
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0), 0x0 );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4), 0x0 );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8), 0x0 );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12), 0x0 );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__probeMarker1_LEN16_TO_PKT_ifa_md__deviceId_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BYTES_THR71_IFA_Header_CFG_ifa_header_template__probeMarker1_LEN16_TO_PKT_ifa_md__deviceId_PLUS16_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Handle RX timestamp if required */
    __LOG_FW(("Handle RX timestamp if required"));
    if(isIngressTimestamp == 1)
    {
        /* Get Desc<Timestamp[31:30] */
        __LOG_FW(("Get Desc<Timestamp[31:30] "));
        ts_31_30 = (thr71_desc_ptr->phal2ppa.timestamp >> 30) & 0x3;
        __LOG_PARAM_FW(ts_31_30);

        /* Get ptp TAI IF select */
        __LOG_FW(("Get ptp TAI IF select"));
        tai_sel = thr71_desc_ptr->phal2ppa.ptp_tai_select;
        __LOG_PARAM_FW(tai_sel);

        /* Read TOD word 1 which represents seconds field [31:0] */
        __LOG_FW(("Read TOD word 1 which represents seconds field [31:0]"));
        ts_sec = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
        __LOG_PARAM_FW(ts_sec);

        /* Clear 2 LS bits of TAI.Seconds */
        __LOG_FW(("Clear 2 LS bits of TAI.Seconds"));
        ts_sec &= 0xFFFFFFFC;

        /* Set 2 LS bits with Desc<timestamp[31:30]> */
        __LOG_FW(("Set 2 LS bits with Desc<timestamp[31:30]"));
        ts_sec +=ts_31_30;

        /* IFA Metadata: Set 32 bits of seconds RX timestamp */
        __LOG_FW(("IFA Metadata: Set 32 bits of seconds RX timestamp"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 10), (ts_sec >> 16   ) );
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 12), ts_sec );

        /* IFA Metadata: Rx Timestamp nano[29:0] = desc<Timestamp[29:0]> */
        __LOG_FW(("IFA MD: Rx Timestamp nano[29:0] = desc<Timestamp[29:0]>"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 14), ( (thr71_desc_ptr->phal2ppa.timestamp >> 16)&0x3FFF) );
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 16), thr71_desc_ptr->phal2ppa.timestamp );

        /* Checksum new RX timestamp (first 2 MS bytes are zero therefore 8 bytes) */
        __LOG_FW(("Checksum new RX timestamp (first 2 MS bytes are zero therefore 8 bytes)"));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR71_IFA_Header_LEN8_FROM_PKT_ifa_md__rxTimestampU_PLUS2_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+10) ));
	    __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR71_IFA_Header_LEN8_FROM_PKT_ifa_md__rxTimestampU_PLUS2_OFFSET, 0);
    }

    /* Calculate and update UDP checksum only if origin cs is not zero (from RFC768) */
    __LOG_FW(("Calculate and update UDP checksum only if origin cs is not zero (from RFC768)"));
    if(incomingL4CsValue != 0)
    {   /* Origin/old UDP CS != 0 */

       /* Update new location of L4 cs offset after packet was shifted left by 60 bytes */
        __LOG_FW(("Update new location of L4 cs offset after packet was shifted left by 60 bytes"));
       l4ChecksumOffset -=60;

        /* Store the new UDP CS */
        __LOG_FW(("Store the new UDP CS using accelerator cmd"));
        ACCEL_CMD_LOAD_PKT_DEST(CSUM_STORE_UDP_THR71_IFA_Header_TO_PKT_udp_header__Checksum_OFFSET, ((PKT<<7) | l4ChecksumOffset) );
	    __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR71_IFA_Header_TO_PKT_udp_header__Checksum_OFFSET, 0);
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Insert Outer VLAN Tag
        At offset 12B (after MAC SA), insert 4B outer VLAN tag with TPID=0x8100, VID = desc<Local Dev Src Port>, CFI=0, PCP=0
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Move left MAC DA and MAC SA by 4 bytes. MAC DA after first shift should be located now in expansion_space.reserved_1 */
    __LOG_FW(("Move left MAC DA and MAC SA by 4 bytes. MAC DA after first shift should be located now in expansion_space.reserved_1"));
    ACCEL_CMD_TRIG(SHIFTLEFT_4_BYTES_THR71_IFA_Header_LEN12_FROM_PKT_expansion_space__reserved_1_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Set TPID to 0x8100 + clear rest of fields */
    __LOG_FW(("Set TPID to 0x8100 + clear rest of fields"));
    PPA_FW_SP_WRITE(FALCON_PKT_REGs_lo + 12, 0x81000000);
    __PRAGMA_NO_REORDER

    /* VID = desc<Local Dev Src Port> */
    __LOG_FW(("VID = desc<Local Dev Src Port>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR71_IFA_Header_DESC_phal2ppa__local_dev_src_port_LEN10_TO_PKT_vlan_header__vid_PLUS2_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+64 bytes) */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+64 bytes)"));
    thr71_desc_ptr->ppa_internal_desc.fw_bc_modification = 64;

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(71);
#endif

exit_thread71:
    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR72_IFA_Ingress_Metadata
 * inparam      None
 * return       None
 * description  Handle Inband Flow Analyzer (IFA) for Ingress node Pass#2 with IPv4/IPv6 UDP/TCP packet use case
 *              It is used to monitor and analyze packets as they enter and exit the network.
 *              It collects data at each hop and at the end of path it is sent to collector to analyze.
 *              This specific thread updates IFA Header and IFA Metadata according to Telemetry Request bits Vector
 *
 *              cfg template: [15:12]: Device ID
 *                            [11:03]: Reserved
 *                            [    2]: metadataInsertSequence, egressPortDropCountingMode
 *                            [01:00]: Reserved
 *              src & trg entries: PHA Source port table holds packet source port (2B)
 *                                 PHA Target port table holds packet target port (2B)
 *
 *              hdr in:  | Extension space (32B) | IPv4/IPv6 | UDP/TCP | IFA HDR(28B) | IFA MD(32) | payload |
 *              hdr out: | Extension space (32B) | IPv4/IPv6 | UDP/TCP | IFA HDR(28B) | IFA MD(32) | payload |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmware does the following:
 *              - Checks packet validity
 *              - Updates IFA header according to architect's definitions
 *              - Updates rest of IFA Metadata according to Telemetry Request Vector
 *              - Calculates L4 checksum
 *              - Triggers GOP to add Tx Timestamp and update UDP/TCP checksum
 *
 *              Note: accelerators are written as IPv4 UDP but they are used also for IPv6 and TCP
 *                    accelerators are updated during thread with the compatible offsets
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR72_IFA_Ingress_Metadata)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    pktType;                    /* IPv4 or IPv6                                                  */
    uint32_t    l4Offset;                   /* UDP/TCP byte offset relative to l3Offset                      */
    uint32_t    l3Offset;                   /* IP bytes offset relative to start of packet                   */
    uint32_t    l3ProtocolOffset;           /* byte offset of L3 Protocol                                    */
    uint32_t    l3ProtocolValue;            /* L3 Protocol (udp or tcp)                                      */
    uint32_t    ifaHeaderStartOffset;       /* IFA header offset relative to packet start                    */
    uint32_t    l4ChecksumOffset;           /* UDP/TCP checksum field byte offset                            */
    uint32_t    ifaMdStartOffset;           /* IFA Metadata byte offset                                      */
    uint32_t    incomingL4CsValue;          /* current/old UDP checksum                                      */
    uint32_t    ttlOffset;                  /* IP TTL field offset in bytes                                  */
    uint32_t    ecnBitsShift;               /* Bits number to shift ECN to be in 2LS bits                    */
    uint32_t    hopLimitOffset;             /* bytes offset to IFA header hop limit field                    */
    uint32_t    hopCountOffset;             /* bytes offset to IFA header hop count field                    */
    uint32_t    maxLengthOffset;            /* bytes offset to IFA header max length field                   */
    uint32_t    currentLengthOffset;        /* bytes offset to IFA header current length field               */
    uint32_t    hopLimitValue;              /* holds Hop limit value                                         */
    uint32_t    hopCountValue;              /* holds Hop count value                                         */
    uint32_t    currentLengthValue;         /* holds current length value                                    */
    uint32_t    telemetryRequestVector;     /* holds bits vector of Telemetry Request Vector                 */
    uint32_t    ttlValue;                   /* holds TTL value                                               */
    uint32_t    ecnValue;                   /* holds ECN value                                               */
    uint32_t    cnValue;                    /* holds CN value                                                */
    uint32_t    smemEgressPortInfoOfst;     /* Bytes offset to Egress Port Info table entry in shared memory */
    uint32_t    smemEgressPortInfoBaseAddr; /* Address of entry of Egress Port Info table in shared memory   */
    uint32_t    timestampOffset;            /* Bytes offset to start of IFA MD Egress timestamp              */
    uint32_t    egressPortInfo;             /* Holds egress port information taken from shared memory        */
    uint32_t    smemFlowInfoEgressPortAddr; /* Address of Egress Port field in Flow Entry in Shared RAM      */
    uint32_t    egressPortTableIndex;       /* index to Egress Port Counters table in Shared RAM             */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr72_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

#ifdef ASIC_SIMULATION
    /* To make debug easier I print out the input packet as it comes to firmware so it can be seen in WM log
       It skips the expansion space and prints out the maximum bytes that can be (IPv6,TCP,IFA hdr,IFA MD) */
    __LOG_FW(("To make debug easier I print out the input packet as it comes to firmware so it can be seen in WM log"));
    __LOG_FW(("It skips the expansion space and prints out the maximum bytes that can be (IPv6,TCP,IFA hdr,IFA MD)"));
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 0) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 4) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 8) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 12) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 16) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 20) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 24) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 28) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 32) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 36) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 40) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 44) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 48) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 52) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 56) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 60) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 64) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 68) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 72) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 76) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 80) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 84) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 88) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 92) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 96) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 100) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 104) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 108) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 112) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 116) );
#endif /* ASIC_SIMULATION */


    /* Find L3 and L4 offsets and packet type */
    __LOG_FW(("Find L3 and L4 offsets and packet type"));
    l3Offset = thr72_desc_ptr->phal2ppa.egress_outer_l3_offset;
    l4Offset = thr72_desc_ptr->phal2ppa.l4_offset;
    pktType  = thr72_desc_ptr->phal2ppa.egress_outer_packet_type;
    __LOG_PARAM_FW(l3Offset);
    __LOG_PARAM_FW(l4Offset);
    __LOG_PARAM_FW(pktType);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set offsets plus check IFA packet is valid for processing
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set offsets plus check IFA packet is valid for processing"));

    if (pktType == 0) /* IPv4 */
    {
        __LOG_FW(("Packet of IPv4 type"));

        /* Find IPv4 offsets */
        __LOG_FW(("Find IPv4 offsets"));
        ttlOffset        = EXPANSION_SPACE_32B_SIZE+8;
        l3ProtocolOffset = EXPANSION_SPACE_32B_SIZE+9;
        l3ProtocolValue  = PPA_FW_SP_BYTE_READ( (FALCON_PKT_REGs_lo + l3ProtocolOffset) );
        ecnBitsShift     = 0;
    }
    else if (pktType == 1) /* IPv6 */
    {
        __LOG_FW(("Packet of IPv6 type"));

        /* Find IPv6 offsets */
        __LOG_FW(("Find IPv6 offsets"));
        ttlOffset        = EXPANSION_SPACE_32B_SIZE+7;
        l3ProtocolOffset = EXPANSION_SPACE_32B_SIZE+6;
        l3ProtocolValue  = PPA_FW_SP_BYTE_READ( (FALCON_PKT_REGs_lo + l3ProtocolOffset) );
        ecnBitsShift     = 4;
    }
    else
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6 */
        __LOG_FW(("Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6"));
        thr72_desc_ptr->phal2ppa.egress_packet_cmd = HARD_DROP;
        __LOG_PARAM_FW(thr72_desc_ptr->phal2ppa.egress_packet_cmd);

        goto exit_thread72;
    }

    if ((l3ProtocolValue != 6) && (l3ProtocolValue != 17))
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Set Desc<egress_packet_cmd> to drop since IFA must be over UDP or TCP */
        __LOG_FW(("Set Desc<egress_packet_cmd> to drop since IFA must be over UDP or TCP"));
        thr72_desc_ptr->phal2ppa.egress_packet_cmd = HARD_DROP;
        __LOG_PARAM_FW(thr72_desc_ptr->phal2ppa.egress_packet_cmd);

        goto exit_thread72;
    }

    /* Print offsets for WM simulation  */
    __LOG_PARAM_FW(ttlOffset);
    __LOG_PARAM_FW(l3ProtocolValue);
    __LOG_PARAM_FW(l3ProtocolOffset);
    __LOG_PARAM_FW(ecnBitsShift);
    __LOG_PARAM_FW(thr72_desc_ptr->phal2ppa.egress_packet_cmd);

    if (l3ProtocolValue == 17) /* UDP */
    {
        __LOG_FW(("Packet of UDP type"));

        /* Find IFA header, L4 checksum offsets, incoming UDP CS value and timestamp offset */
        __LOG_FW(("Find IFA header, L4 checksum offsets, incoming UDP CS value and timestamp offset"));
        ifaHeaderStartOffset = EXPANSION_SPACE_32B_SIZE + l4Offset + UDP_HDR_SIZE;
        l4ChecksumOffset     = EXPANSION_SPACE_32B_SIZE + l4Offset + 6;
        incomingL4CsValue    = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l4ChecksumOffset) );
        timestampOffset      = l3Offset + l4Offset + UDP_HDR_SIZE + IFA_HDR_SIZE + 18; /*relative to start of packet*/
        __LOG_PARAM_FW(ifaHeaderStartOffset);
        __LOG_PARAM_FW(l4ChecksumOffset);
        __LOG_PARAM_FW(incomingL4CsValue);
        __LOG_PARAM_FW(timestampOffset);
    }
    else /* TCP */
    {
        __LOG_FW(("Packet of TCP type"));

        /* Set incoming L4 checksum to some none zero value so it will be written to packet field */
        __LOG_FW(("Set incoming L4 checksum to some none zero value so it will be written to packet field"));
        incomingL4CsValue = 0x1;

        /* Find IFA header, L4 checksum and timestamp offsets */
        __LOG_FW(("Find IFA header, L4 checksum and timestamp offsets"));
        ifaHeaderStartOffset = EXPANSION_SPACE_32B_SIZE + l4Offset + TCP_HDR_SIZE;
        l4ChecksumOffset     = EXPANSION_SPACE_32B_SIZE + l4Offset + 16;
        timestampOffset      = l3Offset + l4Offset + TCP_HDR_SIZE + IFA_HDR_SIZE + 18; /*relative to start of packet*/
        __LOG_PARAM_FW(ifaHeaderStartOffset);
        __LOG_PARAM_FW(l4ChecksumOffset);
        __LOG_PARAM_FW(timestampOffset);
    }

    /* Find IFA Metadata offset */
    __LOG_FW(("Find IFA Metadata offset"));
    ifaMdStartOffset = ifaHeaderStartOffset + IFA_HDR_SIZE;
    __LOG_PARAM_FW(ifaMdStartOffset);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IFA Header Fields
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IFA Header Fields"));

    /* Find offset in bytes for some of IFA header fields: hopLimit, hopCount and currentLength */
    __LOG_FW(("Find offset in bytes for some of IFA header fields: hopLimit, hopCount and currentLength"));
    hopLimitOffset      = ifaHeaderStartOffset + 16;
    hopCountOffset      = ifaHeaderStartOffset + 17;
    maxLengthOffset     = ifaHeaderStartOffset + 20;
    currentLengthOffset = ifaHeaderStartOffset + 22;
    __LOG_PARAM_FW(hopLimitOffset);
    __LOG_PARAM_FW(hopCountOffset);
    __LOG_PARAM_FW(maxLengthOffset);
    __LOG_PARAM_FW(currentLengthOffset);


    /* Since packet fields are not in constant location, we build in front the 32 bits accelerator and during thread
       after we calculate the location we overwrite only source/dest field in the accelerator command */
    __LOG_FW(("Update source/dest fields of some global accelerator commands"));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, ( (PKT<<7) | l4ChecksumOffset ));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, ( (PKT<<7) | hopLimitOffset ));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, ( (PKT<<7) | currentLengthOffset ));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, ( (PKT<<7) | hopLimitOffset));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, ( (PKT<<7) | currentLengthOffset));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR72_IFA_Ingress_Metadata_CFG_ifa_metadata_template__deviceId_LEN4_TO_PKT_ifa_md__deviceId_OFFSET, ( (PKT<<7) | ifaMdStartOffset));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x01_LEN4_TO_PKT_ifa_md__tid_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+4)));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR72_IFA_Ingress_Metadata_DESC_phal2ppa__tc_LEN3_TO_PKT_ifa_md__queueId_PLUS5_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+7)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_OFFSET, ( (PKT<<7) | ifaMdStartOffset));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN6_FROM_PKT_ifa_md__egrPortUtilization_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+22)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN4_FROM_PKT_ifa_md__egPortDropL_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+28)));
    ACCEL_CMD_LOAD_PKT_DEST(CSUM_STORE_UDP_THR72_IFA_Ingress_Metadata_TO_PKT_udp_header__Checksum_OFFSET, ((PKT<<7) | l4ChecksumOffset) );
    __PRAGMA_NO_REORDER

    /* Sum old L4 checksum (2 Bytes) */
    __LOG_FW(("Sum old L4 checksum (2 Bytes)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, 0);

    /* Get values of hopLimit and hopCount */
    __LOG_FW(("Get values of hopLimit and hopCount"));
    hopLimitValue = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + hopLimitOffset);
    hopCountValue = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + hopCountOffset);
    __LOG_PARAM_FW(hopLimitValue);
    __LOG_PARAM_FW(hopCountValue);

    /* Check Hop limit */
    __LOG_FW(("Check Hop limit"));
    if (hopCountValue == hopLimitValue)
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Update source/dest fields of accelerator commands */
        __LOG_FW(("Update source/dest fields of accelerator commands"));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8) ));
        ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__e_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8)));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8)));
       __PRAGMA_NO_REORDER

        /* Checksum old e field */
        __LOG_FW(("Checksum old e field"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Update e field */
        __LOG_FW(("Update e field"));
        ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__e_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Checksum new IFA header e field */
        __LOG_FW(("Checksum new IFA header e field"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, 0);

        /* Calculate and update L4 checksum only if origin cs is not zero (from RFC768) */
        __LOG_FW(("Calculate and update L4 checksum only if origin cs is not zero (from RFC768)"));
        if(incomingL4CsValue != 0)
        {   /* Origin/old UDP CS != 0 */

            /* Store the new UDP CS */
            __LOG_FW(("Store the new UDP CS using accelerator cmd"));
            ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR72_IFA_Ingress_Metadata_TO_PKT_udp_header__Checksum_OFFSET, 0);
        }

        goto exit_thread72;
    }
    else
    {
        /* Checksum old hopCount (add also hopLimit since cmd requires minimum 2B) */
        __LOG_FW(("Sum old hopCount (add also hopLimit since cmd requires minimum 2B)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Increment Hop count by 1 */
        __LOG_FW(("Increment Hop count by 1"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + hopCountOffset,hopCountValue+1);
    }

    /* Get values of currentLength and maxLength */
    __LOG_FW(("Get values of currentLength and maxLength"));
    currentLengthValue = PPA_FW_SP_SHORT_READ(FALCON_PKT_REGs_lo + currentLengthOffset);
    __LOG_PARAM_FW(currentLengthValue);

    /* Check max packet length, max limit = 0x400 */
    __LOG_FW(("Check max packet length, max limit = 0x400"));
    if ( (currentLengthValue + 32) > 0x400)
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Update source/dest fields of accelerator commands */
        __LOG_FW(("Update source/dest fields of accelerator commands"));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+10) ));
        ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__flag_PLUS7_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+11)));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+10)));
        __PRAGMA_NO_REORDER

        /* Checksum old flag Overflow field (start from rsvd field since need cmd requires minimum 2B) */
        __LOG_FW(("Checksum old flag Overflow field (start from rsvd field since need cmd requires minimum 2B)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Set Overflow bit */
        __LOG_FW(("Set Overflow bit"));
        ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__flag_PLUS7_OFFSET, 0);

        /* Checksum new IFA header flag Overflow field */
        __LOG_FW(("Checksum new IFA header flag Overflow field"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, 0);

        /* Calculate and update L4 checksum only if origin cs is not zero (from RFC768) */
        __LOG_FW(("Calculate and update L4P checksum only if origin cs is not zero (from RFC768)"));
        if(incomingL4CsValue != 0)
        {   /* Origin/old UDP CS != 0 */

            /* Store the new UDP CS */
            __LOG_FW(("Store the new UDP CS using accelerator cmd"));
            ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR72_IFA_Ingress_Metadata_TO_PKT_udp_header__Checksum_OFFSET, 0);
        }

        goto exit_thread72;
    }
    else
    {
        /* Checksum old currentLength */
        __LOG_FW(("Checksum old currentLength"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Increment currentLength by 32 bytes*/
        __LOG_FW(("Increment currentLength by 32 bytes"));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + currentLengthOffset,currentLengthValue+32);
    }

    /* Checksum new IFA header hopCount+hopLimit */
    __LOG_FW(("Checksum new IFA header hopCount+hopLimit"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, 0);

    /* Checksum new IFA header currentLength */
    __LOG_FW(("Checksum new IFA header currentLength"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Fill remain 32B IFA Metadata after IFA Header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Fill remain 32B IFA Metadata after IFA Header"));

    /* Get 8 bits of Telemetry Request Vector. Each bit represents request for specific metadata */
    __LOG_FW(("Get 8 bits of Telemetry Request Vector. Each bit represents request for specific metadata"));
    telemetryRequestVector = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 13);
    __LOG_PARAM_FW(telemetryRequestVector);

    /* Handle Device Identifier (bit#0) */
    __LOG_FW(("Handle Device ID (bit#0)"));
    if ( (telemetryRequestVector & 0x1) == 1)
    {
        /* Set Device ID from template */
        __LOG_FW(("Set Device ID from template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR72_IFA_Ingress_Metadata_CFG_ifa_metadata_template__deviceId_LEN4_TO_PKT_ifa_md__deviceId_OFFSET, 0);
    }

    /* Set TID to 1 */
    __LOG_FW(("Set TID to 1"));
    ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x01_LEN4_TO_PKT_ifa_md__tid_OFFSET, 0);

    /* Set TTL to IPv4.Ttl or IPv6.hop_limit */
    __LOG_FW(("Set TTL to IPv4.Ttl or IPv6.hop_limit"));
    ttlValue = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + ttlOffset);
    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 6,ttlValue);
    __LOG_PARAM_FW(ttlValue);

    /* Handle Queue Congestion Status (bit#6) */
    __LOG_FW(("Handle Queue Congestion Status (bit#6)"));
    if ( ((telemetryRequestVector>>6) & 0x1) == 1)
    {
        /* Set Queue-ID = desc<TC> */
        __LOG_FW(("Set Queue-ID = desc<TC>"));
        ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_DESC_phal2ppa__tc_LEN3_TO_PKT_ifa_md__queueId_PLUS5_OFFSET, 0);

        /* Get packet IP ecn 2 bits */
        __LOG_FW(("Get packet IP ecn 2 bits"));
        ecnValue = ((PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_32B_SIZE + 1))>>ecnBitsShift)&0x3;
        __LOG_PARAM_FW(ecnValue);

        /* Set IFA metadata CN field(4 bits) with IP ecn */
        __LOG_FW(("Set IFA metadata CN field(4 bits) with IP ecn"));
        cnValue = (PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + ifaMdStartOffset + 4))&0xF0;
        cnValue +=ecnValue;
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 4,cnValue);
    }

    /* Handle Ingress Egress Port ID (bit#1) */
    __LOG_FW(("Handle Ingress Egress Port ID (bit#1)"));
    if ( ((telemetryRequestVector>>1) & 0x1) == 1)
    {
        /* Set Ingress & Egress port numbers. Get them from PHA source/target port tables */
        __LOG_FW(("Set Ingress & Egress port numbers. Get them from PHA source/target port tables"));
        PPA_FW_SP_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 24, PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 16) );
        __LOG_PARAM_FW(PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 16));
    }

    /* Get IFA Flow entry information from shared memory
       Read word 1 which includes
                    bits [7:0]   : Hop Limit
                    bits [15:8]  : Egress Port
                    bits [20:16] : Ctrl
                    bits [22:21] : Rep
     */

    /* Get base address of the IFA Flow table entry Egress Port in shared memory */
    __LOG_FW(("Get address of the IFA Flow table entry Egress Port in shared memory"));
    smemFlowInfoEgressPortAddr = DRAM_IFA_FLOW_INFO_TABLE_ADDR +
    		      (thr72_desc_ptr->phal2ppa.pha_metadata * DRAM_IFA_FLOW_INFO_TABLE_ENTRY_SIZE)
	              + 4 /* word1*/ + 2 /* byte2*/;
    __LOG_PARAM_FW(smemFlowInfoEgressPortAddr);

    __LOG_FW(("Get IFA Flow entry Egress Port from shared memory"));
    egressPortTableIndex = PPA_FW_SP_BYTE_READ(smemFlowInfoEgressPortAddr);
    __LOG_PARAM_FW(egressPortTableIndex);

    /* Egress port index to be used as the index to Egress Port Info table in shared memory */
    __LOG_FW(("Egress port index to be used as the index to Egress Port Info table in shared memory"));
    smemEgressPortInfoOfst = egressPortTableIndex * 8;
    __LOG_PARAM_FW(smemEgressPortInfoOfst);

    /* Get base address of the required Egress Port Info table entry in shared memory */
    __LOG_FW(("Get base address of the required Egress Port Info table entry in shared memory"));
    smemEgressPortInfoBaseAddr = DRAM_IFA_EGRESS_PORT_INFO_TABLE_ADDR + smemEgressPortInfoOfst;
    __LOG_PARAM_FW(smemEgressPortInfoBaseAddr);

    /* Get egress port information from shared memory.
       Read 4MSB which includes |port utilization(2B)|reserved(1B)|drop counter(1B)| */
    __LOG_FW(("Get egress port information from shared memory."));
    __LOG_FW(("Read 4MSB which includes |port utilization(2B)|reserved(1B)|drop counter(1B)|"));
    egressPortInfo = PPA_FW_SP_READ(smemEgressPortInfoBaseAddr);
    __LOG_PARAM_FW(egressPortInfo);

    /* Handle Egress Port Tx Utilization (bit#7) */
    __LOG_FW(("Handle Egress Port Tx Utilization (bit#7)"));
    if ( ((telemetryRequestVector>>7) & 0x1) == 1)
    {
        /* Egress port utilization: get it from shared memory and set it in packet IFA MD */
        __LOG_FW(("Egress port utilization: get it from shared memory and set it in packet IFA MD"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 22), (egressPortInfo>>16) );
        __LOG_PARAM_FW((egressPortInfo>>16));
    }

    /* Egress port drop counter: get 1 MS byte from shared memory and set it in packet IFA MD */
    __LOG_FW(("Egress port drop counter: get 1 MS byte from shared memory and set it in packet IFA MD"));
    PPA_FW_SP_BYTE_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 5), egressPortInfo );

    /* Egress port drop counter: get 4 LS bytes from shared memory and set it in packet IFA MD */
    __LOG_FW(("Egress port drop counter: get 4 LS bytes from shared memory and set it in packet IFA MD"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 28)  , PPA_FW_SP_READ(smemEgressPortInfoBaseAddr + 4)  );
    __LOG_PARAM_FW((PPA_FW_SP_READ(smemEgressPortInfoBaseAddr + 4)));

    /* Checksum new IFA metadata (first 8 bytes) */
    __LOG_FW(("Checksum new IFA metadata (first 8 bytes)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_OFFSET, 0);

    /* Checksum new IFA metadata (egrPortUtilization,ingressPortId and egressPortId) */
    __LOG_FW(("Checksum new IFA metadata (egrPortUtilization,ingressPortId and egressPortId)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN6_FROM_PKT_ifa_md__egrPortUtilization_OFFSET, 0);

    /* Checksum new IFA metadata (egPortDropL) */
    __LOG_FW(("Checksum new IFA metadata (egPortDropL)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR72_IFA_Ingress_Metadata_LEN4_FROM_PKT_ifa_md__egPortDropL_OFFSET, 0);

    /* Calculate and update UDP checksum only if origin cs is not zero (from RFC768) */
    __LOG_FW(("Calculate and update UDP checksum only if origin cs is not zero (from RFC768)"));
    if(incomingL4CsValue != 0)
    {   /* Origin/old UDP CS != 0 */

        /* Store the new UDP CS */
        __LOG_FW(("Store the new UDP CS using accelerator cmd"));
        ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR72_IFA_Ingress_Metadata_TO_PKT_udp_header__Checksum_OFFSET, 0);
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Trigger GOP to add Tx Timestamp and update UDP/TCP checksum
        - Desc<MAC Timestamping En>    = 1
        - Desc<PTP Packet Format>      = PTPv2 (0x0)
        - Desc<PTP Action>             = AddTime (0x4)
        - Desc<Timestamp Mask Profile> = 0x2
        - Desc<Timestamp Offset>       = IFA Metadata offset + 18 bytes
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Enable PTP TSU"));

    if ( ((telemetryRequestVector>>5) & 0x1) == 1)
    {
        /* Desc<MAC Timestamping En>=1 */
        __LOG_FW(("Desc<MAC Timestamping En>=1"));
        ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x1_LEN1_TO_DESC_phal2ppa__mac_timestamping_en_OFFSET, 0);

        /* Desc<PTP Packet Format>=PTPv2 (0x0) */
        __LOG_FW(("Desc<PTP Packet Format>=PTPv2 (0x0)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x0_LEN3_TO_DESC_phal2ppa__ptp_packet_format_OFFSET, 0);

        /* Desc<PTP Action>=AddTime (0x4) */
        __LOG_FW(("Desc<PTP Action>=AddTime (0x4)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x4_LEN4_TO_DESC_phal2ppa__ptp_action_OFFSET, 0);

        /* Byte offset set of the “Tx Timestamp Nano Upper” field relative to the start of the packet */
        __LOG_FW(("Byte offset set of the “Tx Timestamp Nano Upper” field relative to the start of the packet"));
        thr72_desc_ptr->phal2ppa.timestamp_offset = timestampOffset;

        if(incomingL4CsValue != 0)
        {   /* Origin/old UDP CS != 0 */

  		    /* Desc<Egress Checksum Mode>=ChecksumField (0x1), Desc<Timestamp Mask Profile>=2, Desc<UDP Checksum Update En>=1 */
            __LOG_FW(("Desc<Egress Checksum Mode>=ChecksumField (0x1), Desc<Timestamp Mask Profile>=2, Desc<UDP Checksum Update En>=1"));
			ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x15_LEN5_TO_DESC_phal2ppa__egress_checksum_mode_OFFSET, 0);

  		    /* Calculate L4 checksum relative to start of packet. Decrement expansion space and increment L3 offset */
            __LOG_FW(("Calculate L4 checksum relative to start of packet. Decrement expansion space and increment L3 offset"));
            l4ChecksumOffset = l3Offset + (l4ChecksumOffset-EXPANSION_SPACE_32B_SIZE);
            __LOG_PARAM_FW(l4ChecksumOffset);

            /* Set Desc<udp_checksum> with l4ChecksumOffset. First write it to template then use accelerator to copy to descriptor */
            __LOG_FW(("Set Desc<udp_checksum> with l4ChecksumOffset. First write it to template then use accelerator to copy to descriptor"));
            PPA_FW_SP_BYTE_WRITE((FALCON_CFG_REGs_lo + 15), l4ChecksumOffset);
	        __PRAGMA_NO_REORDER
			ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CFG_ifa_metadata_template__reserved6_LEN8_TO_DESC_phal2ppa__udp_checksum_offset_OFFSET, 0);
        }
        else
        {
			/* Set Desc<Timestamp Mask Profile>=2(010b), Clear Desc<UDP Checksum Update En>=0(0b)  */
			ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x4_LEN4_TO_DESC_phal2ppa__timestamp_mask_profile_OFFSET, 0);
        }
    }
    else
    {
        /* Desc<MAC Timestamping En>=0 */
        __LOG_FW(("Desc<MAC Timestamping En>=0"));
        ACCEL_CMD_TRIG(COPY_BITS_THR72_IFA_Ingress_Metadata_CONST_0x0_LEN1_TO_DESC_phal2ppa__mac_timestamping_en_OFFSET, 0);
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(72);
#endif

exit_thread72:
    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence_reset_stack();
}


/*************************************************************************************************************************************//**
 * funcname     THR73_IFA_Transit_Metadata
 * inparam      None
 * return       None
 * description  Handle Inband Flow Analyzer (IFA) for Ingress node Pass#2 with IPv4/IPv6 UDP/TCP packet use case
 *              It is used to monitor and analyze packets as they enter and exit the network.
 *              It collects data at each hop and at the end of path it is sent to collector to analyze.
 *              This specific thread updates IFA Header and IFA Metadata according to Telemetry Request bits Vector
 *
 *              cfg template: [15:12]: Device ID
 *                            [11:03]: Reserved
 *                            [    2]: metadataInsertSequence, egressPortDropCountingMode
 *                            [01:00]: Reserved
 *              src & trg entries: PHA Source port table holds packet source port (2B)
 *                                 PHA Target port table holds packet target port (2B)
 *
 *              hdr in:  | Extension space (48)  | IPv4/IPv6 | UDP/TCP | IFA HDR(28B) | payload |
 *              hdr out: | Extension space (16B) | IPv4/IPv6 | UDP/TCP | IFA HDR(28B) | IFA MD(32) | payload |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment space
 *
 *              Firmware does the following:
 *              - Checks packet validity
 *              - Updates IFA header according to architect's definitions
 *              - Add 32 bytes of IFA Metadata and set it according to Telemetry Request Vector
 *              - Updates L3 and L4 new lengths
 *              - Calculates IPv4 and L4 checksums
 *              - Triggers GOP to add Tx Timestamp and update UDP/TCP checksum
 *
 *              Note: accelerators are written as IPv4 UDP but they are used also for IPv6 and TCP
 *                    accelerators are updated during thread with the compatible offsets
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR73_IFA_Transit_Metadata)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    pktType;                    /* IPv4 or IPv6                                                  */
    uint32_t    l4Offset;                   /* UDP/TCP byte offset relative to l3Offset                      */
    uint32_t    l3Offset;                   /* IP bytes offset relative to start of packet                   */
    uint32_t    l3ProtocolOffset;           /* byte offset of L3 Protocol                                    */
    uint32_t    l3ProtocolValue;            /* L3 Protocol (udp or tcp)                                      */
    uint32_t    ifaHeaderStartOffset;       /* IFA header offset relative to packet start                    */
    uint32_t    l4ChecksumOffset;           /* UDP/TCP checksum field byte offset                            */
    uint32_t    ifaMdStartOffset;           /* IFA Metadata byte offset                                      */
    uint32_t    incomingL4CsValue;          /* current/old UDP checksum                                      */
    uint32_t    ttlOffset;                  /* IP TTL field offset in bytes                                  */
    uint32_t    ecnBitsShift;               /* Bits number to shift ECN to be in 2LS bits                    */
    uint32_t    hopLimitOffset;             /* bytes offset to IFA header hop limit field                    */
    uint32_t    hopCountOffset;             /* bytes offset to IFA header hop count field                    */
    uint32_t    maxLengthOffset;            /* bytes offset to IFA header max length field                   */
    uint32_t    currentLengthOffset;        /* bytes offset to IFA header current length field               */
    uint32_t    hopLimitValue;              /* holds Hop limit value                                         */
    uint32_t    hopCountValue;              /* holds Hop count value                                         */
    uint32_t    currentLengthValue;         /* holds current length value                                    */
    uint32_t    telemetryRequestVector;     /* holds bits vector of Telemetry Request Vector                 */
    uint32_t    ttlValue;                   /* holds TTL value                                               */
    uint32_t    ecnValue;                   /* holds ECN value                                               */
    uint32_t    cnValue;                    /* holds CN value                                                */
    uint32_t    smemEgressPortInfoOfst;     /* Bytes offset to Egress Port Info table entry in shared memory */
    uint32_t    smemEgressPortInfoBaseAddr; /* Address of entry of Egress Port Info table in shared memory   */
    uint32_t    timestampOffset;            /* Bytes offset to start of IFA MD Egress timestamp              */
    uint32_t    l3LengthOffset;             /* IP length field byte offset                                   */
    uint32_t    l3LengthValue;              /* IP length field value                                         */
    uint32_t    l3CsOffset ZERO_VAR_FOR_SIM;             /* IPv4 checksum field byte offset                               */
    uint32_t    ts_31_30;                   /* holds Desc<timestamp[31:30]> value                            */
    uint32_t    tai_sel;                    /* TAI select index 0 or 1                                       */
    uint32_t    ts_sec;                     /* timestamp in seconds                                          */
    uint32_t    l4LengthOffset ZERO_VAR_FOR_SIM;           /* byte offset UDP/TCP length                                    */
    uint32_t    l4LengthValue;              /* UDP/TCP length value                                          */
    uint32_t    bytesToShift;               /* number of bytes to shift left                                 */
    uint32_t    egressPortInfo;             /* Holds egress port information taken from shared memory        */
    uint32_t    smemFlowInfoEgressPortAddr; /* Address of Egress Port field in Flow Entry in Shared RAM      */
    uint32_t    egressPortTableIndex;       /* index to Egress Port Counters table in Shared RAM             */

    /* Get pointers to cfg and descriptor */
    struct ppa_in_desc* thr73_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

#ifdef ASIC_SIMULATION
    /* To make debug easier I print out the input packet as it comes to firmware so it can be seen in WM log
       It skips the expansion space and prints out the maximum bytes that can be (IPv6,TCP,IFA hdr) */
    __LOG_FW(("To make debug easier I print out the input packet as it comes to firmware so it can be seen in WM log"));
    __LOG_FW(("It skips the expansion space and prints out the maximum bytes that can be (IPv6,TCP,IFA hdr)"));
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 0) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 4) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 8) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 12) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 16) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 20) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 24) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 28) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 32) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 36) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 40) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 44) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 48) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 52) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 56) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 60) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 64) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 68) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 72) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 76) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 80) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 84) );
#endif /* ASIC_SIMULATION */


    /* Find L3 and L4 offsets and packet type */
    __LOG_FW(("Find L3 and L4 offsets and packet type"));
    l3Offset = thr73_desc_ptr->phal2ppa.egress_outer_l3_offset;
    l4Offset = thr73_desc_ptr->phal2ppa.l4_offset;
    pktType  = thr73_desc_ptr->phal2ppa.egress_outer_packet_type;
    __LOG_PARAM_FW(l3Offset);
    __LOG_PARAM_FW(l4Offset);
    __LOG_PARAM_FW(pktType);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set offsets plus check IFA packet is valid for processing
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set offsets plus check IFA packet is valid for processing"));

    if (pktType == 0) /* IPv4 */
    {
        __LOG_FW(("Packet of IPv4 type"));

        /* Find IPv4 offsets */
        __LOG_FW(("Find IPv4 offsets"));
        l3LengthOffset   = EXPANSION_SPACE_48B_SIZE + 2;
        l3CsOffset       = EXPANSION_SPACE_48B_SIZE + 10;
        ttlOffset        = 16 + 8; /*after shifting packet to the left. 16 is the number of bytes left in expansion space*/
        l3ProtocolOffset = EXPANSION_SPACE_48B_SIZE + 9;
        l3ProtocolValue  = PPA_FW_SP_BYTE_READ( (FALCON_PKT_REGs_lo + l3ProtocolOffset) );
        ecnBitsShift     = 0;
    }
    else if (pktType == 1) /* IPv6 */
    {
        __LOG_FW(("Packet of IPv6 type"));

        /* Find IPv6 offsets */
        __LOG_FW(("Find IPv6 offsets"));
        l3LengthOffset   = EXPANSION_SPACE_48B_SIZE + 4;
        ttlOffset        = 16 + 7; /*after shifting packet to the left. 16 is the number of bytes left in expansion space*/
        l3ProtocolOffset = EXPANSION_SPACE_48B_SIZE + 6;
        l3ProtocolValue  = PPA_FW_SP_BYTE_READ( (FALCON_PKT_REGs_lo + l3ProtocolOffset) );
        ecnBitsShift     = 4;
    }
    else
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6 only */
        __LOG_FW(("Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6 only"));
        thr73_desc_ptr->phal2ppa.egress_packet_cmd = HARD_DROP;
        __LOG_PARAM_FW(thr73_desc_ptr->phal2ppa.egress_packet_cmd);

        goto exit_thread73;
    }

    if (l3ProtocolValue == 17) /* UDP */
    {
        __LOG_FW(("Packet of UDP type"));

        ifaHeaderStartOffset = 16 + l4Offset + UDP_HDR_SIZE; /*after shifting packet to the left. 16 is the number of bytes left in expansion space*/
        l4LengthOffset       = EXPANSION_SPACE_48B_SIZE + l4Offset + 4;
        l4ChecksumOffset     = EXPANSION_SPACE_48B_SIZE + l4Offset + 6;
        incomingL4CsValue    = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l4ChecksumOffset) );
        timestampOffset      = l3Offset + l4Offset + UDP_HDR_SIZE + IFA_HDR_SIZE + 18; /*relative to start of packet*/
    }
    else if (l3ProtocolValue == 6) /* TCP */
    {
        __LOG_FW(("Packet of TCP type"));

        /* Set incoming L4 checksum to some none zero value so it will be written to packet field */
        __LOG_FW(("Set incoming L4 checksum to some none zero value so it will be written to packet field"));
        incomingL4CsValue    = 0x1;
        ifaHeaderStartOffset = 16 + l4Offset + TCP_HDR_SIZE; /*after shifting packet to the left. 16 is the number of bytes left in expansion space*/
        l4ChecksumOffset     = EXPANSION_SPACE_48B_SIZE + l4Offset + 16;
        timestampOffset      = l3Offset + l4Offset + TCP_HDR_SIZE + IFA_HDR_SIZE + 18; /*relative to start of packet*/
    }
    else
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6 */
        __LOG_FW(("Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6"));
        thr73_desc_ptr->phal2ppa.egress_packet_cmd = HARD_DROP;
        __LOG_PARAM_FW(thr73_desc_ptr->phal2ppa.egress_packet_cmd);

        goto exit_thread73;
    }
    /* Find IFA header and metadata offsets */
    __LOG_FW(("Find IFA header and metadata offsets"));
    hopLimitOffset      = ifaHeaderStartOffset + 16;
    hopCountOffset      = ifaHeaderStartOffset + 17;
    maxLengthOffset     = ifaHeaderStartOffset + 20;
    currentLengthOffset = ifaHeaderStartOffset + 22;
    ifaMdStartOffset    = ifaHeaderStartOffset + IFA_HDR_SIZE;

    /* Print offsets for WM simulation  */
    __LOG_PARAM_FW(ttlOffset);
    __LOG_PARAM_FW(l3ProtocolValue);
    __LOG_PARAM_FW(l3ProtocolOffset);
    __LOG_PARAM_FW(ecnBitsShift);
    __LOG_PARAM_FW(thr73_desc_ptr->phal2ppa.egress_packet_cmd);
    __LOG_PARAM_FW(l3LengthOffset);
    __LOG_PARAM_FW(l3CsOffset);
    __LOG_PARAM_FW(ifaHeaderStartOffset);
    __LOG_PARAM_FW(l4ChecksumOffset);
    __LOG_PARAM_FW(incomingL4CsValue);
    __LOG_PARAM_FW(timestampOffset);
    __LOG_PARAM_FW(l4LengthOffset);
    __LOG_PARAM_FW(ifaMdStartOffset);
    __LOG_PARAM_FW(hopLimitOffset);
    __LOG_PARAM_FW(hopCountOffset);
    __LOG_PARAM_FW(maxLengthOffset);
    __LOG_PARAM_FW(currentLengthOffset);

    /* Since packet fields are not in constant location, we build in front the 32 bits accelerator and during thread
       after we calculate the location we overwrite only source/dest field in the accelerator command */
    __LOG_FW(("Update source/dest fields of some global accelerator commands"));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, ( (PKT<<7) | l4ChecksumOffset ));
    __PRAGMA_NO_REORDER
    l4ChecksumOffset -=32;
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, ( (PKT<<7) | hopLimitOffset ));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, ( (PKT<<7) | currentLengthOffset ));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, ( (PKT<<7) | hopLimitOffset));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, ( (PKT<<7) | currentLengthOffset));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR73_IFA_Transit_Metadata_CFG_ifa_metadata_template__deviceId_LEN4_TO_PKT_ifa_md__deviceId_OFFSET, ( (PKT<<7) | ifaMdStartOffset));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x01_LEN4_TO_PKT_ifa_md__tid_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+4)));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR73_IFA_Transit_Metadata_DESC_phal2ppa__tc_LEN3_TO_PKT_ifa_md__queueId_PLUS5_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+7)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_OFFSET, ( (PKT<<7) | ifaMdStartOffset));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS8_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+8)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS16_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+16)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS24_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+24)));
    ACCEL_CMD_LOAD_PKT_DEST(CSUM_STORE_UDP_THR73_IFA_Transit_Metadata_TO_PKT_udp_header__Checksum_OFFSET, ((PKT<<7) | l4ChecksumOffset) );
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IP length field: add IFA Metadata (+32B)
      - Calculate IPv4 checksum (incremental update):   New checksum = ~( ~(old checksum) + ~(old Total Length) + (new Total Length) )
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IP length field(+32B) + calculate IPv4 checksum"));

    if (pktType == 0) /* IPv4 */
    {
        __LOG_FW(("Packet of IPv4 type: handle IPv4 actions"));

        /* Update source/dest fields of accelerator commands */
        __LOG_FW(("Update source/dest fields of accelerator commands"));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, ( (PKT<<7) | l3LengthOffset ));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, ( (PKT<<7) | l3LengthOffset ));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, ( (PKT<<7) | l3CsOffset ));
        ACCEL_CMD_LOAD_PKT_DEST(CSUM_STORE_IP_THR73_IFA_Transit_Metadata_TO_PKT_IPv4_Header__header_checksum_OFFSET, ( (PKT<<7) | l3CsOffset ));
	    __PRAGMA_NO_REORDER

        /* Sum old Total Length */
        __LOG_FW(("Sum old Total Length"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Update IPv4<Total Length> */
        __LOG_FW(("Update IPv4<Total Length>"));
        l3LengthValue = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l3LengthOffset) );
        __LOG_PARAM_FW(l3LengthValue);
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + l3LengthOffset), l3LengthValue + 32 );
	    __PRAGMA_NO_REORDER

        /* Sum new Total Length */
        __LOG_FW(("Sum new Total Length"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);

        /* Sum current/old IPv4 CS */
        __LOG_FW(("Sum current/old IPv4 CS"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Store the new IPv4 CS */
        __LOG_FW(("Store the new IPv4 CS"));
        ACCEL_CMD_TRIG(CSUM_STORE_IP_THR73_IFA_Transit_Metadata_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);

        /* Update current/old L4 length (decrement IP header since UDP length does not include it) */
        __LOG_FW(("Update current/old L4 length (decrement IP header since UDP length does not include it)"));
        l4LengthValue = l3LengthValue - IPV4_BASIC_HEADER_SIZE;
    }
    else /* IPv6 */
    {
        __LOG_FW(("Packet of IPv6 type: handle IPv6 actions"));

        /* Update IPv6<Payload Length> (+32B) */
        __LOG_FW(("Update IPv6<Payload Length> (+32B)"));
        l3LengthValue = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l3LengthOffset) );
        __LOG_PARAM_FW(l3LengthValue);
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + l3LengthOffset), l3LengthValue + 32 );

        /* Update current/old L4 length */
        __LOG_FW(("Update current/old L4 length"));
        l4LengthValue = l3LengthValue;
    }

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update L4 length field
      - Start L4 CS calculation   New checksum = ~( ~(old checksum) + ~(old value of a 16-bit field) + (new value of a 16-bit field)
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update L4 length, start to calculate new L4 CS "));
    if (l3ProtocolValue == 17) /* UDP */
    {
        __LOG_FW(("Packet of UDP type"));

        /* Update source fields of accelerator commands */
        __LOG_FW(("Update source fields of accelerator commands"));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, ( (PKT<<7) | l4LengthOffset ));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, ( (PKT<<7) | l4LengthOffset ));
	    __PRAGMA_NO_REORDER

        /* Sum old UDP checksum */
        __LOG_FW(("Sum old UDP checksum"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, 0);

        /* Sum old UDP length twice (IP Pseudo Header) */
        __LOG_FW(("Sum old UDP length (2 Bytes) (IP Pseudo Header)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);

        /* Update new UDP length */
        __LOG_FW(("Update new UDP length"));
        l4LengthValue = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l4LengthOffset) );
        __LOG_PARAM_FW(l4LengthValue);
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + l4LengthOffset), l4LengthValue + 32 );
        __PRAGMA_NO_REORDER

        /* Sum new UDP Length twice (another one for IP Pseudo Header) */
        __LOG_FW(("Sum new UDP Length twice (another one for IP Pseudo Header)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);
    }
    else /* TCP */
    {
        __LOG_FW(("Packet of TCP type"));

        /* Sum old TCP checksum */
        __LOG_FW(("Sum old TCP checksum"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, 0);

        /* Although TCP length is not part of TCP header need to cs since it is part of IP Pseudo Header */
        /* Add it to template reserved space so we can add it to calculation */
        __LOG_FW(("Although TCP length is not part of TCP header need to cs since it is part of IP Pseudo Header"));
        __LOG_FW(("Add it to template reserved space so we can add it to calculation"));
        PPA_FW_SP_SHORT_WRITE((FALCON_CFG_REGs_lo + 14), l4LengthValue);
	    __PRAGMA_NO_REORDER

        /* Sum old TCP length (IP Pseudo Header) */
        __LOG_FW(("Sum old TCP length (IP Pseudo Header)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_CFG_ifa_metadata_template__reserved5_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Update new TCP length (+32B) */
        __LOG_FW(("Update new TCP length (+32B)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR73_IFA_Transit_Metadata_CONST_0x20_LEN16_TO_CFG_ifa_metadata_template__reserved5_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Sum new TCP Length */
        __LOG_FW(("Sum new TCP Length"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_CFG_ifa_metadata_template__reserved5_OFFSET, 0);
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move left input packet (IP,UDP,IFA hdr) by 32 bytes to make room for new IFA metadata
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Move left input packet by 32 bytes to make room for new IFA metadata"));

    /* Move left the first 32 bytes */
    __LOG_FW(("Move left the first 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_32_BYTES_THR73_IFA_Transit_Metadata_LEN32_FROM_PKT_IPv4_Header__version_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Get the remaining number of bytes to shift.
       - ifaHeaderStartOffset holds number of bytes which needs to be shifted left (IP,UDP,IFA_Header)
       - plus 16 bytes where L3 header is in PHA packet header memory after movement
       - decrement this number by (32 + 16) since we already shift left 32 bytes */
    __LOG_FW(("Get the remaining number of bytes to shift"));
    __LOG_FW(("- faHeaderStartOffset holds number of bytes which needs to be shifted left (IP,UDP,IFA_Header)"));
    __LOG_FW(("- plus 16 bytes where L3 header is in PHA packet header memory after movement"));
    __LOG_FW(("- decrement this number by (32 + 16) since we already shift left 32 bytes"));
    bytesToShift = ifaMdStartOffset - 48;

    /* Move left the rest of the packet */
    __LOG_FW(("Move left the rest of the packet"));
    if (bytesToShift > 32)
    {
        ACCEL_CMD_TRIG(SHIFTLEFT_32_BYTES_THR73_IFA_Transit_Metadata_LEN32_FROM_PKT_IPv4_Header__version_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER
        bytesToShift -= 32;
        ACCEL_CMD_LOAD_LENGTH(SHIFTLEFT_32_BYTES_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__version_PLUS64_OFFSET,bytesToShift-1);
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTLEFT_32_BYTES_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__version_PLUS64_OFFSET, 0);
    }
    else
    {
        ACCEL_CMD_LOAD_LENGTH(SHIFTLEFT_32_BYTES_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__version_PLUS32_OFFSET,bytesToShift-1);
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTLEFT_32_BYTES_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_IPv4_Header__version_PLUS32_OFFSET, 0);
    }
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IFA Header Fields
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IFA Header Fields"));

    /* Get values of hopLimit and hopCount */
    __LOG_FW(("Get values of hopLimit and hopCount"));
    hopLimitValue = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + hopLimitOffset);
    hopCountValue = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + hopCountOffset);
    __LOG_PARAM_FW(hopLimitValue);
    __LOG_PARAM_FW(hopCountValue);

    /* Check Hop limit */
    __LOG_FW(("Check Hop limit"));
    if (hopCountValue == hopLimitValue)
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Update source/dest fields of accelerator commands */
        __LOG_FW(("Update source/dest fields of accelerator commands"));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8) ));
        ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__e_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8)));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8)));
        __PRAGMA_NO_REORDER

        /* Checksum old e field (must take 2 bytes) */
        __LOG_FW(("Checksum old e field (must take 2 bytes)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Update e field */
        __LOG_FW(("Update e field"));
        ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__e_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Checksum new IFA header e field */
        __LOG_FW(("Checksum new IFA header e field"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, 0);

        /* Calculate and update L4 checksum only if origin cs is not zero (from RFC768) */
        __LOG_FW(("Calculate and update L4 checksum only if origin cs is not zero (from RFC768)"));
        if(incomingL4CsValue != 0)
        {   /* Origin/old UDP CS != 0 */

            /* Store the new UDP CS */
            __LOG_FW(("Store the new UDP CS using accelerator cmd"));
            ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR73_IFA_Transit_Metadata_TO_PKT_udp_header__Checksum_OFFSET, 0);
        }

        goto exit_thread73;
    }
    else
    {
        /* Checksum old hopCount (add also hopLimit since cmd requires minimum 2B) */
        __LOG_FW(("Sum old hopCount (add also hopLimit since cmd requires minimum 2B)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Increment Hop count by 1 */
        __LOG_FW(("Increment Hop count by 1"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + hopCountOffset,hopCountValue+1);
    }

    /* Get values of currentLength and maxLength */
    __LOG_FW(("Get values of currentLength and maxLength"));
    currentLengthValue = PPA_FW_SP_SHORT_READ(FALCON_PKT_REGs_lo + currentLengthOffset);
    __LOG_PARAM_FW(currentLengthValue);

    /* Check max packet length, max limit = 0x400 */
    __LOG_FW(("Check max packet length, max limit = 0x400"));
    if ( (currentLengthValue + 32) > 0x400)
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Update source/dest fields of accelerator commands */
        __LOG_FW(("Update source/dest fields of accelerator commands"));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+10) ));
        ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__flag_PLUS7_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+11)));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+10)));
        __PRAGMA_NO_REORDER

        /* Checksum old flag Overflow field (start from rsvd field since need cmd requires minimum 2B) */
        __LOG_FW(("Checksum old flag Overflow field (start from rsvd field since need cmd requires minimum 2B)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Set Overflow bit */
        __LOG_FW(("Set Overflow bit"));
        ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__flag_PLUS7_OFFSET, 0);

        /* Checksum new IFA header flag Overflow field */
        __LOG_FW(("Checksum new IFA header flag Overflow field"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, 0);

        /* Calculate and update L4 checksum only if origin cs is not zero (from RFC768) */
        __LOG_FW(("Calculate and update L4P checksum only if origin cs is not zero (from RFC768)"));
        if(incomingL4CsValue != 0)
        {   /* Origin/old UDP CS != 0 */

            /* Store the new UDP CS */
            __LOG_FW(("Store the new UDP CS using accelerator cmd"));
            ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR73_IFA_Transit_Metadata_TO_PKT_udp_header__Checksum_OFFSET, 0);
        }

        goto exit_thread73;
    }
    else
    {
        /* Checksum old currentLength */
        __LOG_FW(("Checksum old currentLength"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Increment currentLength by 32 bytes*/
        __LOG_FW(("Increment currentLength by 32 bytes"));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + currentLengthOffset,currentLengthValue+32);
    }

    /* Checksum new IFA header hopCount+hopLimit */
    __LOG_FW(("Checksum new IFA header hopCount+hopLimit"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, 0);

    /* Checksum new IFA header currentLength */
    __LOG_FW(("Checksum new IFA header currentLength"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Fill 32B IFA Metadata after IFA Header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Fill 32B IFA Metadata after IFA Header"));

    /* Get 8 bits of Telemetry Request Vector. Each bit represents request for specific metadata */
    __LOG_FW(("Get 8 bits of Telemetry Request Vector. Each bit represents request for specific metadata"));
    telemetryRequestVector = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 13);
    __LOG_PARAM_FW(telemetryRequestVector);

    /* First clear 24B of IFA Metadata (last 8B are set anyway) */
    __LOG_FW(("First clear 24B of IFA Metadata (last 8B are set anyway)"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 0), 0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 4), 0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 8), 0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 12),0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 16),0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 20),0x0 );
    __PRAGMA_NO_REORDER

    /* Handle Device Identifier (bit#0) */
    __LOG_FW(("Handle Device ID (bit#0)"));
    if ( (telemetryRequestVector & 0x1) == 1)
    {
        /* Set Device ID from template */
        __LOG_FW(("Set Device ID from template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR73_IFA_Transit_Metadata_CFG_ifa_metadata_template__deviceId_LEN4_TO_PKT_ifa_md__deviceId_OFFSET, 0);
    }

    /* Set TID to 1 */
    __LOG_FW(("Set TID to 1"));
    ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x01_LEN4_TO_PKT_ifa_md__tid_OFFSET, 0);

    /* Set TTL to IPv4.Ttl or IPv6.hop_limit */
    __LOG_FW(("Set TTL to IPv4.Ttl or IPv6.hop_limit"));
    ttlValue = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + ttlOffset);
    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 6,ttlValue);
    __LOG_PARAM_FW(ttlValue);

    /* Handle Queue Congestion Status (bit#6) */
    __LOG_FW(("Handle Queue Congestion Status (bit#6)"));
    if ( ((telemetryRequestVector>>6) & 0x1) == 1)
    {
        /* Set Queue-ID = desc<TC> */
        __LOG_FW(("Set Queue-ID = desc<TC>"));
        ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_DESC_phal2ppa__tc_LEN3_TO_PKT_ifa_md__queueId_PLUS5_OFFSET, 0);

        /* Get packet IP ecn 2 bits */
        __LOG_FW(("Get packet IP ecn 2 bits"));
        ecnValue = ((PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 16 + 1))>>ecnBitsShift)&0x3;
        __LOG_PARAM_FW(ecnValue);

        /* Set IFA metadata CN field(4 bits) with IP ecn */
        __LOG_FW(("Set IFA metadata CN field(4 bits) with IP ecn"));
        cnValue = (PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + ifaMdStartOffset + 4))&0xF0;
        cnValue +=ecnValue;
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 4,cnValue);
    }

    /* Handle Ingress Egress Port ID (bit#1) */
    __LOG_FW(("Handle Ingress Egress Port ID (bit#1)"));
    if ( ((telemetryRequestVector>>1) & 0x1) == 1)
    {
        /* Set Ingress & Egress port numbers. Get them from PHA source/target port tables */
        __LOG_FW(("Set Ingress & Egress port numbers. Get them from PHA source/target port tables"));
        PPA_FW_SP_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 24, PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 16) );
        __LOG_PARAM_FW(PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 16));
    }

    /* Get IFA Flow entry information from shared memory
           Read word 1 which includes
                        bits [7:0]   : Hop Limit
                        bits [15:8]  : Egress Port
                        bits [20:16] : Ctrl
                        bits [22:21] : Rep
    */

	/* Get base address of the IFA Flow table entry Egress Port in shared memory */
	__LOG_FW(("Get address of the IFA Flow table entry Egress Port in shared memory"));
	smemFlowInfoEgressPortAddr = DRAM_IFA_FLOW_INFO_TABLE_ADDR +
				  (thr73_desc_ptr->phal2ppa.pha_metadata * DRAM_IFA_FLOW_INFO_TABLE_ENTRY_SIZE)
	              + 4 /* word1*/ + 2 /* byte2*/;
	__LOG_PARAM_FW(smemFlowInfoEgressPortAddr);

	__LOG_FW(("Get IFA Flow entry Egress Port from shared memory"));
	egressPortTableIndex = PPA_FW_SP_BYTE_READ(smemFlowInfoEgressPortAddr);
	__LOG_PARAM_FW(egressPortTableIndex);

	/* Egress port index to be used as the index to Egress Port Info table in shared memory */
	__LOG_FW(("Egress port index to be used as the index to Egress Port Info table in shared memory"));
	smemEgressPortInfoOfst = egressPortTableIndex * 8;
    __LOG_PARAM_FW(smemEgressPortInfoOfst);

    /* Get base address of the required Egress Port Info table entry in shared memory */
    __LOG_FW(("Get base address of the required Egress Port Info table entry in shared memory"));
    smemEgressPortInfoBaseAddr = DRAM_IFA_EGRESS_PORT_INFO_TABLE_ADDR + smemEgressPortInfoOfst;
    __LOG_PARAM_FW(smemEgressPortInfoBaseAddr);

    /* Get egress port information from shared memory.
       Read 4MSB which includes |port utilization(2B)|reserved(1B)|drop counter(1B)| */
    __LOG_FW(("Get egress port information from shared memory."));
    __LOG_FW(("Read 4MSB which includes |port utilization(2B)|reserved(1B)|drop counter(1B)|"));
    egressPortInfo = PPA_FW_SP_READ(smemEgressPortInfoBaseAddr);
    __LOG_PARAM_FW(egressPortInfo);

    /* Handle Egress Port Tx Utilization (bit#7) */
    __LOG_FW(("Handle Egress Port Tx Utilization (bit#7)"));
    if ( ((telemetryRequestVector>>7) & 0x1) == 1)
    {
        /* Egress port utilization: get it from shared memory and set it in packet IFA MD */
        __LOG_FW(("Egress port utilization: get it from shared memory and set it in packet IFA MD"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 22)  , (egressPortInfo>>16) );
        __LOG_PARAM_FW((egressPortInfo>>16));
    }

    /* Handle RX timestamp (bit#4) */
    __LOG_FW(("Handle RX timestamp (bit#4)"));
    if ( ((telemetryRequestVector>>4) & 0x1) == 1)
    {
        /* Get Desc<Timestamp[31:30] */
        __LOG_FW(("Get Desc<Timestamp[31:30] "));
        ts_31_30 = (thr73_desc_ptr->phal2ppa.timestamp >> 30) & 0x3;
        __LOG_PARAM_FW(ts_31_30);

        /* Get ptp TAI IF select */
        __LOG_FW(("Get ptp TAI IF select"));
        tai_sel = thr73_desc_ptr->phal2ppa.ptp_tai_select;
        __LOG_PARAM_FW(tai_sel);

        /* Read TOD word 1 which represents seconds field [31:0] */
        __LOG_FW(("Read TOD word 1 which represents seconds field [31:0]"));
        ts_sec = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
        __LOG_PARAM_FW(ts_sec);

        /* Clear 2 LS bits of TAI.Seconds */
        __LOG_FW(("Clear 2 LS bits of TAI.Seconds"));
        ts_sec &= 0xFFFFFFFC;

        /* Set 2 LS bits with Desc<timestamp[31:30]> */
        __LOG_FW(("Set 2 LS bits with Desc<timestamp[31:30]"));
        ts_sec +=ts_31_30;

        /* IFA Metadata: Set 32 bits of seconds RX timestamp */
        __LOG_FW(("IFA Metadata: Set 32 bits of seconds RX timestamp"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 10), (ts_sec >> 16   ) );
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 12), ts_sec );

        /* IFA Metadata: Rx Timestamp nano[29:0] = desc<Timestamp[29:0]> */
        __LOG_FW(("IFA MD: Rx Timestamp nano[29:0] = desc<Timestamp[29:0]>"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 14), ( (thr73_desc_ptr->phal2ppa.timestamp >> 16)&0x3FFF) );
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 16), thr73_desc_ptr->phal2ppa.timestamp );
    }

    /* Egress port drop counter: get 1 MS byte from shared memory and set it in packet IFA MD */
    __LOG_FW(("Egress port drop counter: get 1 MS byte from shared memory and set it in packet IFA MD"));
    PPA_FW_SP_BYTE_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 5), egressPortInfo );

    /* Egress port drop counter: get 4 LS bytes from shared memory and set it in packet IFA MD */
    __LOG_FW(("Egress port drop counter: get 4 LS bytes from shared memory and set it in packet IFA MD"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 28)  , PPA_FW_SP_READ(smemEgressPortInfoBaseAddr + 4)  );
    __LOG_PARAM_FW((PPA_FW_SP_READ(smemEgressPortInfoBaseAddr + 4)));
    __PRAGMA_NO_REORDER

    /* Checksum new IFA metadata (32 bytes) */
    __LOG_FW(("Checksum new IFA metadata (32 bytes)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_OFFSET, 0);
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS8_OFFSET, 0);
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS16_OFFSET, 0);
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR73_IFA_Transit_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS24_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Calculate and update UDP checksum only if origin cs is not zero (from RFC768) */
    __LOG_FW(("Calculate and update UDP checksum only if origin cs is not zero (from RFC768)"));
    if(incomingL4CsValue != 0)
    {   /* Origin/old UDP CS != 0 */

        /* Store the new UDP CS */
        __LOG_FW(("Store the new UDP CS using accelerator cmd"));
        ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR73_IFA_Transit_Metadata_TO_PKT_udp_header__Checksum_OFFSET, 0);
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Trigger GOP to add Tx Timestamp and update UDP/TCP checksum
        - Desc<MAC Timestamping En>    = 1
        - Desc<PTP Packet Format>      = PTPv2 (0x0)
        - Desc<PTP Action>             = AddTime (0x4)
        - Desc<Timestamp Mask Profile> = 0x2
        - Desc<Timestamp Offset>       = IFA Metadata offset + 18 bytes
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Enable PTP TSU"));

    if ( ((telemetryRequestVector>>5) & 0x1) == 1)
    {
        /* Desc<MAC Timestamping En>=1 */
        __LOG_FW(("Desc<MAC Timestamping En>=1"));
        ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x1_LEN1_TO_DESC_phal2ppa__mac_timestamping_en_OFFSET, 0);

        /* Desc<PTP Packet Format>=PTPv2 (0x0) */
        __LOG_FW(("Desc<PTP Packet Format>=PTPv2 (0x0)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x0_LEN3_TO_DESC_phal2ppa__ptp_packet_format_OFFSET, 0);

        /* Desc<PTP Action>=AddTime (0x4) */
        __LOG_FW(("Desc<PTP Action>=AddTime (0x4)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x4_LEN4_TO_DESC_phal2ppa__ptp_action_OFFSET, 0);

        /* Byte offset set of the “Tx Timestamp Nano Upper” field relative to the start of the packet */
        __LOG_FW(("Byte offset set of the “Tx Timestamp Nano Upper” field relative to the start of the packet"));
        thr73_desc_ptr->phal2ppa.timestamp_offset = timestampOffset;

        if(incomingL4CsValue != 0)
        {   /* Origin/old UDP CS != 0 */

  		    /* Desc<Egress Checksum Mode>=ChecksumField (0x1), Desc<Timestamp Mask Profile>=2, Desc<UDP Checksum Update En>=1 */
            __LOG_FW(("Desc<Egress Checksum Mode>=ChecksumField (0x1), Desc<Timestamp Mask Profile>=2, Desc<UDP Checksum Update En>=1"));
			ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x15_LEN5_TO_DESC_phal2ppa__egress_checksum_mode_OFFSET, 0);

  		    /* Calculate L4 checksum relative to start of packet. Value of l4ChecksumOffset is offset of field in PHA header space.
  		     * First L3 byte starts from byte# 16 in this memory. Need to compensate l4ChecksumOffset by 16 to get offset from L3 header.  */
            __LOG_FW(("Calculate L4 checksum relative to start of packet"));
            l4ChecksumOffset = l3Offset + l4ChecksumOffset - 16;
            __LOG_PARAM_FW(l4ChecksumOffset);

            /* Set Desc<udp_checksum> with l4ChecksumOffset. First write it to template then use accelerator to copy to descriptor */
            __LOG_FW(("Set Desc<udp_checksum> with l4ChecksumOffset. First write it to template then use accelerator to copy to descriptor"));
            PPA_FW_SP_BYTE_WRITE((FALCON_CFG_REGs_lo + 15), l4ChecksumOffset);
	        __PRAGMA_NO_REORDER
			ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CFG_ifa_metadata_template__reserved6_LEN8_TO_DESC_phal2ppa__udp_checksum_offset_OFFSET, 0);
        }
        else
        {
			/* Set Desc<Timestamp Mask Profile>=2(010b), Clear Desc<UDP Checksum Update En>=0(0b) */
            __LOG_FW(("Set Desc<Timestamp Mask Profile>=2(010b), Clear Desc<UDP Checksum Update En>=0(0b)"));
			ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x4_LEN4_TO_DESC_phal2ppa__timestamp_mask_profile_OFFSET, 0);
        }
    }
    else
    {
        /* Bypass egress timestamp, Desc<MAC Timestamping En>=0 */
        __LOG_FW(("Bypass egress timestamp, Desc<MAC Timestamping En>=0"));
        ACCEL_CMD_TRIG(COPY_BITS_THR73_IFA_Transit_Metadata_CONST_0x0_LEN1_TO_DESC_phal2ppa__mac_timestamping_en_OFFSET, 0);
    }

    /* Indicates HW about the new starting point of the packet header (+32 bytes) */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+32 bytes)"));
    thr73_desc_ptr->ppa_internal_desc.fw_bc_modification = 32;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(73);
#endif

exit_thread73:
    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence_reset_stack();
}


/*************************************************************************************************************************************//**
 * funcname     THR74_IFA_Egress_Node_Mirror
 * inparam      None
 * return       None
 * description  Handle Inband Flow Analyzer (IFA) for Egress node Pass#1 with IPv4/IPv6 UDP/TCP packet use case
 *              It is used to monitor and analyze packets as they enter and exit the network.
 *              It collects data at each hop and at the end of path it is sent to collector to analyze.
 *              This specific thread copies metadata into the “Copy Reserved” descriptor and triggers egress
 *              mirroring replication in EREP.
 *
 *              cfg template: [15:1]: Reserved
 *                            [   0]: |reserved (bits[7:3])|IFA analyzer ID (bits[2:0])|
 *              src & trg entries: PHA Source port table holds packet source port (2B)
 *                                 PHA Target port table holds packet target port (2B)
 *
 *              hdr in:  don't care
 *              hdr out: don't care
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware does the following:
 *              - Set Desc<Copy Reserved[19:17]> with 3 bits Desc<TC>
 *              - Set Desc<Copy Reserved[16:7]> with 10 bits Desc<Local Dev Trg Port>
 *              - Set Desc<Analyzer Index> with template data byte 0 [2:0]
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR74_IFA_Egress_Node_Mirror)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    /* Set Desc<Copy Reserved[19:17]> with 3 bits Desc<TC> */
    __LOG_FW(("Set Desc<Copy Reserved[19:17]> with 3 bits Desc<TC>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR74_IFA_Egress_Node_Mirror_DESC_phal2ppa__tc_LEN3_TO_DESC_phal2ppa__copy_reserved_OFFSET, 0);

    /* Set Desc<Copy Reserved[16:7]> with 10 bits Desc<Local Dev Trg Port> */
    __LOG_FW(("Set Desc<Copy Reserved[16:7]> with 10 bits Desc<Local Dev Trg Port>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR74_IFA_Egress_Node_Mirror_DESC_phal2ppa__local_dev_trg_phy_port_LEN10_TO_DESC_phal2ppa__copy_reserved_PLUS3_OFFSET, 0);

    /* Set Desc<Analyzer Index> with template data byte 0 [2:0] */
    __LOG_FW(("Set Desc<Analyzer Index> with template data byte 0 [2:0]"));
    ACCEL_CMD_TRIG(COPY_BITS_THR74_IFA_Egress_Node_Mirror_CFG_ifa_egress_node_mirror_template__ifaAnalyzerId_LEN3_TO_DESC_phal2ppa__analyzer_index_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(74);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR75_IFA_Egress_Node_Metadata
 * inparam      None
 * return       None
 * description  Handle Inband Flow Analyzer (IFA) for Egress node Pass#1 with IPv4/IPv6 UDP/TCP packet use case
 *              It is used to monitor and analyze packets as they enter and exit the network.
 *              It collects data at each hop and at the end of path it is sent to collector to analyze.
 *              This specific thread updates IFA Header and add IFA Metadata according to Telemetry Request bits Vector
 *
 *              cfg template: [15:12]: Device ID
 *                            [11:03]: Reserved
 *                            [    2]: metadataInsertSequence, egressPortDropCountingMode
 *                            [01:00]: Reserved
 *              src & trg entries: PHA Source port table holds packet source port (2B)
 *                                 PHA Target port table holds packet target port (2B)
 *
 *              hdr in:  | Extension space (48)  | IPv4/IPv6 | UDP/TCP | IFA HDR(28B) | payload |
 *              hdr out: | Extension space (16B) | IPv4/IPv6 | UDP/TCP | IFA HDR(28B) | IFA MD(32) | payload |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment space
 *
 *              Firmware does the following:
 *              - Checks packet validity
 *              - Updates IFA header according to architect's definitions
 *              - Add 32 bytes of IFA Metadata and set it according to Telemetry Request Vector
 *              - Updates L3 and L4 new lengths
 *              - Calculates IPv4 and L4 checksums
 *              - Triggers GOP to add Tx Timestamp and update UDP/TCP checksum
 *
 *              Note: accelerators are written as IPv4 UDP but they are used also for IPv6 and TCP
 *                    accelerators are updated during thread with the compatible offsets
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR75_IFA_Egress_Node_Metadata)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    pktType;                    /* IPv4 or IPv6                                                  */
    uint32_t    l4Offset;                   /* UDP/TCP byte offset relative to l3Offset                      */
    uint32_t    l3Offset;                   /* IP bytes offset relative to start of packet                   */
    uint32_t    l3ProtocolOffset;           /* Byte offset of L3 Protocol                                    */
    uint32_t    l3ProtocolValue;            /* L3 Protocol (udp or tcp)                                      */
    uint32_t    ifaHeaderStartOffset;       /* IFA header offset relative to packet start                    */
    uint32_t    l4ChecksumOffset;           /* UDP/TCP checksum field byte offset                            */
    uint32_t    ifaMdStartOffset;           /* IFA Metadata byte offset                                      */
    uint32_t    incomingL4CsValue;          /* Current/old UDP checksum                                      */
    uint32_t    ttlOffset;                  /* IP TTL field offset in bytes                                  */
    uint32_t    ecnBitsShift;               /* Bits number to shift ECN to be in 2LS bits                    */
    uint32_t    hopLimitOffset;             /* Bytes offset to IFA header hop limit field                    */
    uint32_t    hopCountOffset;             /* Bytes offset to IFA header hop count field                    */
    uint32_t    maxLengthOffset;            /* Bytes offset to IFA header max length field                   */
    uint32_t    currentLengthOffset;        /* Bytes offset to IFA header current length field               */
    uint32_t    hopLimitValue;              /* Holds Hop limit value                                         */
    uint32_t    hopCountValue;              /* Holds Hop count value                                         */
    uint32_t    currentLengthValue;         /* Holds current length value                                    */
    uint32_t    telemetryRequestVector;     /* Holds bits vector of Telemetry Request Vector                 */
    uint32_t    ttlValue;                   /* Holds TTL value                                               */
    uint32_t    ecnValue;                   /* Holds ECN value                                               */
    uint32_t    cnValue;                    /* Holds CN value                                                */
    uint32_t    smemEgressPortInfoOfst;     /* Bytes offset to Egress Port Info table entry in shared memory */
    uint32_t    smemEgressPortInfoBaseAddr; /* Address of entry of Egress Port Info table in shared memory   */
    uint32_t    timestampOffset;            /* Bytes offset to start of IFA MD Egress timestamp              */
    uint32_t    l3LengthOffset;             /* IP length field byte offset                                   */
    uint32_t    l3LengthValue;              /* IP length field value                                         */
    uint32_t    l3CsOffset ZERO_VAR_FOR_SIM;             /* IPv4 checksum field byte offset                               */
    uint32_t    ts_31_30;                   /* Holds Desc<timestamp[31:30]> value                            */
    uint32_t    tai_sel;                    /* TAI select index 0 or 1                                       */
    uint32_t    ts_sec;                     /* Timestamp in seconds                                          */
    uint32_t    l4LengthOffset ZERO_VAR_FOR_SIM;           /* Byte offset UDP/TCP length                                    */
    uint32_t    l4LengthValue;              /* UDP/TCP length value                                          */
    uint32_t    bytesToShift;               /* Number of bytes to shift left                                 */
    uint32_t    origEgressPort;             /* Egress port of the origin packet                              */
    uint32_t    egressPortInfo;             /* Holds egress port information taken from shared memory        */
    uint32_t    smemFlowInfoEgressPortAddr; /* Address of Egress Port field in Flow Entry in Shared RAM      */
    uint32_t    egressPortTableIndex;       /* index to Egress Port Counters table in Shared RAM             */

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr75_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


#ifdef ASIC_SIMULATION
    /* To make debug easier I print out the input packet as it comes to firmware so it can be seen in WM log
       It skips the expansion space and prints out the maximum bytes that can be (IPv6,TCP,IFA hdr) */
    __LOG_FW(("To make debug easier I print out the input packet as it comes to firmware so it can be seen in WM log"));
    __LOG_FW(("It skips the expansion space and prints out the maximum bytes that can be (IPv6,TCP,IFA hdr)"));
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 0) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 4) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 8) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 12) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 16) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 20) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 24) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 28) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 32) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 36) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 40) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 44) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 48) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 52) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 56) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 60) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 64) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 68) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 72) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 76) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 80) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + EXPANSION_SPACE_48B_SIZE + 84) );
#endif /* ASIC_SIMULATION */


    /* Find L3,L4 offsets and packet type */
    __LOG_FW(("Find L3,L4 offsets and packet type"));
    l3Offset = thr75_desc_ptr->phal2ppa.egress_outer_l3_offset;
    l4Offset = thr75_desc_ptr->phal2ppa.l4_offset;
    pktType  = thr75_desc_ptr->phal2ppa.egress_outer_packet_type;
    __LOG_PARAM_FW(l3Offset);
    __LOG_PARAM_FW(l4Offset);
    __LOG_PARAM_FW(pktType);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set L3 offsets plus check IFA packet is valid for processing
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set offsets plus check IFA packet is valid for processing"));

    if (pktType == 0) /* IPv4 */
    {
        __LOG_FW(("Packet of IPv4 type"));

        /* Find IPv4 offsets */
        __LOG_FW(("Find IPv4 offsets"));
        l3LengthOffset   = EXPANSION_SPACE_48B_SIZE + 2;
        l3CsOffset       = EXPANSION_SPACE_48B_SIZE + 10;
        ttlOffset        = 16 + 8; /*after shifting packet to the left. 16 is the number of bytes left in expansion space*/
        l3ProtocolOffset = EXPANSION_SPACE_48B_SIZE + 9;
        l3ProtocolValue  = PPA_FW_SP_BYTE_READ( (FALCON_PKT_REGs_lo + l3ProtocolOffset) );
        ecnBitsShift     = 0;
    }
    else if (pktType == 1) /* IPv6 */
    {
        __LOG_FW(("Packet of IPv6 type"));

        /* Find IPv6 offsets */
        __LOG_FW(("Find IPv6 offsets"));
        l3LengthOffset   = EXPANSION_SPACE_48B_SIZE + 4;
        ttlOffset        = 16 + 7; /*after shifting packet to the left. 16 is the number of bytes left in expansion space*/
        l3ProtocolOffset = EXPANSION_SPACE_48B_SIZE + 6;
        l3ProtocolValue  = PPA_FW_SP_BYTE_READ( (FALCON_PKT_REGs_lo + l3ProtocolOffset) );
        ecnBitsShift     = 4;
    }
    else
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6 only */
        __LOG_FW(("Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6 only"));
        thr75_desc_ptr->phal2ppa.egress_packet_cmd = HARD_DROP;
        __LOG_PARAM_FW(thr75_desc_ptr->phal2ppa.egress_packet_cmd);

        goto exit_thread75;
    }

    if (l3ProtocolValue == 17) /* UDP */
    {
        __LOG_FW(("Packet of UDP type"));

        ifaHeaderStartOffset = 16 + l4Offset + UDP_HDR_SIZE; /*after shifting packet to the left. 16 is the number of bytes left in expansion space*/
        l4LengthOffset       = EXPANSION_SPACE_48B_SIZE + l4Offset + 4;
        l4ChecksumOffset     = EXPANSION_SPACE_48B_SIZE + l4Offset + 6;
        incomingL4CsValue    = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l4ChecksumOffset) );
        timestampOffset      = l3Offset + l4Offset + UDP_HDR_SIZE + IFA_HDR_SIZE + 18; /*relative to start of packet*/
    }
    else if (l3ProtocolValue == 6) /* TCP */
    {
        __LOG_FW(("Packet of TCP type"));

        /* Set incoming L4 checksum to some none zero value so it will be written to packet field */
        __LOG_FW(("Set incoming L4 checksum to some none zero value so it will be written to packet field"));
        incomingL4CsValue    = 0x1;
        ifaHeaderStartOffset = 16 + l4Offset + TCP_HDR_SIZE; /*after shifting packet to the left. 16 is the number of bytes left in expansion space*/
        l4ChecksumOffset     = EXPANSION_SPACE_48B_SIZE + l4Offset + 16;
        timestampOffset      = l3Offset + l4Offset + TCP_HDR_SIZE + IFA_HDR_SIZE + 18; /*relative to start of packet*/
    }
    else
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6 */
        __LOG_FW(("Set Desc<egress_packet_cmd> to drop since IFA must be over IPv4 or IPv6"));
        thr75_desc_ptr->phal2ppa.egress_packet_cmd = HARD_DROP;
        __LOG_PARAM_FW(thr75_desc_ptr->phal2ppa.egress_packet_cmd);

        goto exit_thread75;
    }

    /* Find IFA header and metadata offsets */
    __LOG_FW(("Find IFA header and metadata offsets"));
    hopLimitOffset      = ifaHeaderStartOffset + 16;
    hopCountOffset      = ifaHeaderStartOffset + 17;
    maxLengthOffset     = ifaHeaderStartOffset + 20;
    currentLengthOffset = ifaHeaderStartOffset + 22;
    ifaMdStartOffset    = ifaHeaderStartOffset + IFA_HDR_SIZE;

    /* Print offsets for WM simulation  */
    __LOG_PARAM_FW(ttlOffset);
    __LOG_PARAM_FW(l3ProtocolValue);
    __LOG_PARAM_FW(l3ProtocolOffset);
    __LOG_PARAM_FW(ecnBitsShift);
    __LOG_PARAM_FW(thr75_desc_ptr->phal2ppa.egress_packet_cmd);
    __LOG_PARAM_FW(l3LengthOffset);
    __LOG_PARAM_FW(l3CsOffset);
    __LOG_PARAM_FW(ifaHeaderStartOffset);
    __LOG_PARAM_FW(l4ChecksumOffset);
    __LOG_PARAM_FW(incomingL4CsValue);
    __LOG_PARAM_FW(timestampOffset);
    __LOG_PARAM_FW(l4LengthOffset);
    __LOG_PARAM_FW(ifaMdStartOffset);
    __LOG_PARAM_FW(hopLimitOffset);
    __LOG_PARAM_FW(hopCountOffset);
    __LOG_PARAM_FW(maxLengthOffset);
    __LOG_PARAM_FW(currentLengthOffset);

    /* Since packet fields are not in constant location, we build in front the 32 bits accelerator and during thread
       after we calculate the location we overwrite only source/dest field in the accelerator command */
    __LOG_FW(("Update source/dest fields of some global accelerator commands"));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, ( (PKT<<7) | l4ChecksumOffset ));
    __PRAGMA_NO_REORDER
    l4ChecksumOffset -=32;
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, ( (PKT<<7) | hopLimitOffset ));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, ( (PKT<<7) | currentLengthOffset ));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, ( (PKT<<7) | hopLimitOffset));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, ( (PKT<<7) | currentLengthOffset));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR75_IFA_Egress_Node_Metadata_CFG_ifa_metadata_template__deviceId_LEN4_TO_PKT_ifa_md__deviceId_OFFSET, ( (PKT<<7) | ifaMdStartOffset));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x01_LEN4_TO_PKT_ifa_md__tid_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+4)));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR75_IFA_Egress_Node_Metadata_DESC_phal2ppa__copy_reserved_LEN3_TO_PKT_ifa_md__queueId_PLUS5_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+7)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_OFFSET, ( (PKT<<7) | ifaMdStartOffset));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS8_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+8)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS16_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+16)));
    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS24_OFFSET, ( (PKT<<7) | (ifaMdStartOffset+24)));
    ACCEL_CMD_LOAD_PKT_DEST(CSUM_STORE_UDP_THR75_IFA_Egress_Node_Metadata_TO_PKT_udp_header__Checksum_OFFSET, ((PKT<<7) | l4ChecksumOffset) );
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IP length field: add IFA Metadata (+32B)
      - Calculate IPv4 checksum (incremental update):   New checksum = ~( ~(old checksum) + ~(old Total Length) + (new Total Length) )
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IP length field(+32B) + calculate IPv4 checksum"));

    if (pktType == 0) /* IPv4 */
    {
        __LOG_FW(("Packet of IPv4 type: handle IPv4 actions"));

        /* Update source/dest fields of accelerator commands */
        __LOG_FW(("Update source/dest fields of accelerator commands"));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, ( (PKT<<7) | l3LengthOffset ));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, ( (PKT<<7) | l3LengthOffset ));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, ( (PKT<<7) | l3CsOffset ));
        ACCEL_CMD_LOAD_PKT_DEST(CSUM_STORE_IP_THR75_IFA_Egress_Node_Metadata_TO_PKT_IPv4_Header__header_checksum_OFFSET, ( (PKT<<7) | l3CsOffset ));
	    __PRAGMA_NO_REORDER

        /* Sum old IPv4 Total Length */
        __LOG_FW(("Sum old Total Length"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Update IPv4<Total Length> */
        __LOG_FW(("Update IPv4<Total Length>"));
        l3LengthValue = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l3LengthOffset) );
        __LOG_PARAM_FW(l3LengthValue);
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + l3LengthOffset), l3LengthValue + 32 );
	    __PRAGMA_NO_REORDER

        /* Sum new Total Length */
        __LOG_FW(("Sum new Total Length"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);

        /* Sum current/old IPv4 CS */
        __LOG_FW(("Sum current/old IPv4 CS"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Store the new IPv4 CS */
        __LOG_FW(("Store the new IPv4 CS"));
        ACCEL_CMD_TRIG(CSUM_STORE_IP_THR75_IFA_Egress_Node_Metadata_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);

        /* Update current/old L4 length (decrement IP header since UDP length does not include it) */
        __LOG_FW(("Update current/old L4 length (decrement IP header since UDP length does not include it)"));
        l4LengthValue = l3LengthValue - IPV4_BASIC_HEADER_SIZE;
    }
    else /* IPv6 */
    {
        __LOG_FW(("Packet of IPv6 type: handle IPv6 actions"));

        /* Update IPv6<Payload Length> (+32B) */
        __LOG_FW(("Update IPv6<Payload Length> (+32B)"));
        l3LengthValue = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l3LengthOffset) );
        __LOG_PARAM_FW(l3LengthValue);
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + l3LengthOffset), l3LengthValue + 32 );

        /* Update current/old L4 length */
        __LOG_FW(("Update current/old L4 length"));
        l4LengthValue = l3LengthValue;
    }

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update L4 length field
      - Start L4 CS calculation   New checksum = ~( ~(old checksum) + ~(old value of a 16-bit field) + (new value of a 16-bit field)
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update L4 length, start to calculate new L4 CS "));
    if (l3ProtocolValue == 17) /* UDP */
    {
        __LOG_FW(("Packet of UDP type"));

        /* Update source fields of accelerator commands */
        __LOG_FW(("Update source fields of accelerator commands"));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, ( (PKT<<7) | l4LengthOffset ));
	    ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, ( (PKT<<7) | l4LengthOffset ));
	    __PRAGMA_NO_REORDER

        /* Sum old UDP checksum */
        __LOG_FW(("Sum old UDP checksum"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, 0);

        /* Sum old UDP length twice (another one for IP Pseudo Header) */
        __LOG_FW(("Sum old UDP length twice (another one for IP Pseudo Header)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);

        /* Update new UDP length */
        __LOG_FW(("Update new UDP length"));
        l4LengthValue = PPA_FW_SP_SHORT_READ( (FALCON_PKT_REGs_lo + l4LengthOffset) );
        __LOG_PARAM_FW(l4LengthValue);
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + l4LengthOffset), l4LengthValue + 32 );
        __PRAGMA_NO_REORDER

        /* Sum new UDP Length twice (another one for IP Pseudo Header) */
        __LOG_FW(("Sum new UDP Length twice (another one for IP Pseudo Header)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_udp_header__Length_OFFSET, 0);
    }
    else /* TCP */
    {
        __LOG_FW(("Packet of TCP type"));

        /* Sum old TCP checksum */
        __LOG_FW(("Sum old TCP checksum"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_udp_header__Checksum_OFFSET, 0);

        /* Although TCP length is not part of TCP header need to cs since it is part of IP Pseudo Header */
        /* Add it to template reserved space so we can add it to calculation */
        __LOG_FW(("Although TCP length is not part of TCP header need to cs since it is part of IP Pseudo Header"));
        __LOG_FW(("Add it to template reserved space so we can add it to calculation"));
        PPA_FW_SP_SHORT_WRITE((FALCON_CFG_REGs_lo + 14), l4LengthValue);
	    __PRAGMA_NO_REORDER

        /* Sum old TCP length (IP Pseudo Header) */
        __LOG_FW(("Sum old TCP length (IP Pseudo Header)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_CFG_ifa_metadata_template__reserved5_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Update new TCP length (+32B) */
        __LOG_FW(("Update new TCP length (+32B)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x20_LEN16_TO_CFG_ifa_metadata_template__reserved5_OFFSET, 0);
	    __PRAGMA_NO_REORDER

        /* Sum new TCP Length */
        __LOG_FW(("Sum new TCP Length"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_CFG_ifa_metadata_template__reserved5_OFFSET, 0);
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move left input packet (IP,UDP,IFA hdr) by 32 bytes to make room for new IFA metadata
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Move left input packet by 32 bytes to make room for new IFA metadata"));

    /* Move left the first 32 bytes */
    __LOG_FW(("Move left the first 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_32_BYTES_THR75_IFA_Egress_Node_Metadata_LEN32_FROM_PKT_IPv4_Header__version_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Get the remaining number of bytes to shift.
       - ifaHeaderStartOffset holds number of bytes which needs to be shifted left (IP,UDP,IFA_Header)
       - plus 16 bytes where L3 header is in PHA packet header memory after movement
       - decrement this number by (32 + 16) since we already shift left 32 bytes */
    __LOG_FW(("Get the remaining number of bytes to shift"));
    __LOG_FW(("- faHeaderStartOffset holds number of bytes which needs to be shifted left (IP,UDP,IFA_Header)"));
    __LOG_FW(("- plus 16 bytes where L3 header is in PHA packet header memory after movement"));
    __LOG_FW(("- decrement this number by (32 + 16) since we already shift left 32 bytes"));
    bytesToShift = ifaMdStartOffset - 48;

    /* Move left the rest of the packet */
    __LOG_FW(("Move left the rest of the packet"));
    if (bytesToShift > 32)
    {
        ACCEL_CMD_LOAD_LENGTH(SHIFTLEFT_32_BYTES_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__version_PLUS64_OFFSET,bytesToShift-1);
        ACCEL_CMD_TRIG(SHIFTLEFT_32_BYTES_THR75_IFA_Egress_Node_Metadata_LEN32_FROM_PKT_IPv4_Header__version_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER
        bytesToShift -= 32;
        ACCEL_CMD_TRIG(SHIFTLEFT_32_BYTES_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__version_PLUS64_OFFSET, 0);
    }
    else
    {
        ACCEL_CMD_LOAD_LENGTH(SHIFTLEFT_32_BYTES_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__version_PLUS32_OFFSET,bytesToShift-1);
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTLEFT_32_BYTES_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_IPv4_Header__version_PLUS32_OFFSET, 0);
    }
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IFA Header Fields
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IFA Header Fields"));

    /* Get values of hopLimit and hopCount */
    __LOG_FW(("Get values of hopLimit and hopCount"));
    hopLimitValue = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + hopLimitOffset);
    hopCountValue = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + hopCountOffset);
    __LOG_PARAM_FW(hopLimitValue);
    __LOG_PARAM_FW(hopCountValue);

    /* Check Hop limit */
    __LOG_FW(("Check Hop limit"));
    if (hopCountValue == hopLimitValue)
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Update source/dest fields of accelerator commands */
        __LOG_FW(("Update source/dest fields of accelerator commands"));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8) ));
        ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__e_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8)));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+8)));
        __PRAGMA_NO_REORDER

        /* Checksum old e field (must take 2 bytes) */
        __LOG_FW(("Checksum old e field (must take 2 bytes)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Update e field */
        __LOG_FW(("Update e field"));
        ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__e_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Checksum new IFA header e field */
        __LOG_FW(("Checksum new IFA header e field"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__ver_OFFSET, 0);

        /* Calculate and update L4 checksum only if origin cs is not zero (from RFC768) */
        __LOG_FW(("Calculate and update L4 checksum only if origin cs is not zero (from RFC768)"));
        if(incomingL4CsValue != 0)
        {   /* Origin/old UDP CS != 0 */

            /* Store the new UDP CS */
            __LOG_FW(("Store the new UDP CS"));
            ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR75_IFA_Egress_Node_Metadata_TO_PKT_udp_header__Checksum_OFFSET, 0);
        }

        goto exit_thread75;
    }
    else
    {
        /* Checksum old hopCount (add also hopLimit since cmd requires minimum 2B) */
        __LOG_FW(("Sum old hopCount (add also hopLimit since cmd requires minimum 2B)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Increment Hop count by 1 */
        __LOG_FW(("Increment Hop count by 1"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + hopCountOffset,hopCountValue+1);
    }

    /* Get values of currentLength and maxLength */
    __LOG_FW(("Get values of currentLength and maxLength"));
    currentLengthValue = PPA_FW_SP_SHORT_READ(FALCON_PKT_REGs_lo + currentLengthOffset);
    __LOG_PARAM_FW(currentLengthValue);

    /* Check max packet length, max limit = 0x400 */
    __LOG_FW(("Check max packet length, max limit = 0x400"));
    if ( (currentLengthValue + 32) > 0x400)
    {
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Update source/dest fields of accelerator commands */
        __LOG_FW(("Update source/dest fields of accelerator commands"));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+10) ));
        ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__flag_PLUS7_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+11)));
        ACCEL_CMD_LOAD_PKT_SRC(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, ( (PKT<<7) | (ifaHeaderStartOffset+10)));
        __PRAGMA_NO_REORDER

        /* Checksum old flag Overflow field (start from rsvd field since need cmd requires minimum 2B) */
        __LOG_FW(("Checksum old flag Overflow field (start from rsvd field since need cmd requires minimum 2B)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Set Overflow bit */
        __LOG_FW(("Set Overflow bit"));
        ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x01_LEN1_TO_PKT_ifa_header__flag_PLUS7_OFFSET, 0);

        /* Checksum new IFA header flag Overflow field */
        __LOG_FW(("Checksum new IFA header flag Overflow field"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__rsvd_OFFSET, 0);

        /* Calculate and update L4 checksum only if origin cs is not zero (from RFC768) */
        __LOG_FW(("Calculate and update L4P checksum only if origin cs is not zero (from RFC768)"));
        if(incomingL4CsValue != 0)
        {   /* Origin/old UDP CS != 0 */

            /* Store the new UDP CS */
            __LOG_FW(("Store the new UDP CS using accelerator cmd"));
            ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR75_IFA_Egress_Node_Metadata_TO_PKT_udp_header__Checksum_OFFSET, 0);
        }

        goto exit_thread75;
    }
    else
    {
        /* Checksum old currentLength */
        __LOG_FW(("Checksum old currentLength"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Increment currentLength by 32 bytes*/
        __LOG_FW(("Increment currentLength by 32 bytes"));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + currentLengthOffset,currentLengthValue+32);
    }

    /* Checksum new IFA header hopCount+hopLimit */
    __LOG_FW(("Checksum new IFA header hopCount+hopLimit"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__hopLimit_OFFSET, 0);

    /* Checksum new IFA header currentLength */
    __LOG_FW(("Checksum new IFA header currentLength"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN2_FROM_PKT_ifa_header__currentLength_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Fill 32B IFA Metadata after IFA Header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Fill 32B IFA Metadata after IFA Header"));

    /* Get 8 bits of Telemetry Request Vector. Each bit represents request for specific metadata */
    __LOG_FW(("Get 8 bits of Telemetry Request Vector. Each bit represents request for specific metadata"));
    telemetryRequestVector = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + ifaHeaderStartOffset + 13);
    __LOG_PARAM_FW(telemetryRequestVector);

    /* First clear 24B of IFA Metadata (last 8B are set anyway) */
    __LOG_FW(("First clear 24B of IFA Metadata (last 8B are set anyway)"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 0), 0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 4), 0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 8), 0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 12),0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 16),0x0 );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 20),0x0 );
    __PRAGMA_NO_REORDER

    /* Handle Device Identifier (bit#0) */
    __LOG_FW(("Handle Device ID (bit#0)"));
    if ( (telemetryRequestVector & 0x1) == 1)
    {
        /* Set Device ID from template */
        __LOG_FW(("Set Device ID from template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR75_IFA_Egress_Node_Metadata_CFG_ifa_metadata_template__deviceId_LEN4_TO_PKT_ifa_md__deviceId_OFFSET, 0);
    }

    /* Set TID to 1 */
    __LOG_FW(("Set TID to 1"));
    ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x01_LEN4_TO_PKT_ifa_md__tid_OFFSET, 0);

    /* Set TTL to IPv4.Ttl or IPv6.hop_limit */
    __LOG_FW(("Set TTL to IPv4.Ttl or IPv6.hop_limit"));
    ttlValue = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + ttlOffset);
    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 6,ttlValue);
    __LOG_PARAM_FW(ttlValue);

    /* Handle Queue Congestion Status (bit#6) */
    __LOG_FW(("Handle Queue Congestion Status (bit#6)"));
    if ( ((telemetryRequestVector>>6) & 0x1) == 1)
    {
        /* Set Queue-ID = desc<TC> of origin packet */
        __LOG_FW(("Set Queue-ID = desc<TC>"));
        ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_DESC_phal2ppa__copy_reserved_LEN3_TO_PKT_ifa_md__queueId_PLUS5_OFFSET, 0);

        /* Get packet IP ecn 2 bits */
        __LOG_FW(("Get packet IP ecn 2 bits"));
        ecnValue = ((PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 16 + 1))>>ecnBitsShift)&0x3;
        __LOG_PARAM_FW(ecnValue);

        /* Set IFA metadata CN field(4 bits) with IP ecn */
        __LOG_FW(("Set IFA metadata CN field(4 bits) with IP ecn"));
        cnValue = (PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + ifaMdStartOffset + 4))&0xF0;
        cnValue +=ecnValue;
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 4,cnValue);
    }

    /* Get origin packet egress port which was saved in earlier thread in Desc<copy_reserved> */
    __LOG_FW(("Get origin packet egress port which was saved in earlier thread in Desc<copy_reserved>"));
    origEgressPort = (thr75_desc_ptr->phal2ppa.copy_reserved >> 7) & 0x3FF;
    __LOG_PARAM_FW(thr75_desc_ptr->phal2ppa.copy_reserved);
    __LOG_PARAM_FW(origEgressPort);

    /* Handle Ingress Egress Port ID (bit#1) */
    __LOG_FW(("Handle Ingress Egress Port ID (bit#1)"));
    if ( ((telemetryRequestVector>>1) & 0x1) == 1)
    {
        /* Set Ingress & Egress port numbers. Get ingress from PHA source port tables and egress from Desc<copy reserved[16:7]> */
        __LOG_FW(("Set Ingress & Egress port numbers. Get ingress from PHA source port tables and egress from Desc<copy reserved[16:7]>"));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 24, PPA_FW_SP_SHORT_READ(FALCON_CFG_REGs_lo + 16) );
        __LOG_PARAM_FW(PPA_FW_SP_SHORT_READ(FALCON_CFG_REGs_lo + 16));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + ifaMdStartOffset + 26, origEgressPort);
        __LOG_PARAM_FW(origEgressPort);
    }

    /* Get IFA Flow entry information from shared memory
           Read word 1 which includes
                        bits [7:0]   : Hop Limit
                        bits [15:8]  : Egress Port
                        bits [20:16] : Ctrl
                        bits [22:21] : Rep
	 */

	/* Get base address of the IFA Flow table entry Egress Port in shared memory */
	__LOG_FW(("Get address of the IFA Flow table entry Egress Port in shared memory"));
	smemFlowInfoEgressPortAddr = DRAM_IFA_FLOW_INFO_TABLE_ADDR +
				  (thr75_desc_ptr->phal2ppa.pha_metadata * DRAM_IFA_FLOW_INFO_TABLE_ENTRY_SIZE)
	              + 4 /* word1*/ + 2 /* byte2*/;
	__LOG_PARAM_FW(smemFlowInfoEgressPortAddr);

	__LOG_FW(("Get IFA Flow entry Egress Port from shared memory"));
	egressPortTableIndex = PPA_FW_SP_BYTE_READ(smemFlowInfoEgressPortAddr);
	__LOG_PARAM_FW(egressPortTableIndex);

	/* Egress port index to be used as the index to Egress Port Info table in shared memory */
	__LOG_FW(("Egress port index to be used as the index to Egress Port Info table in shared memory"));
	smemEgressPortInfoOfst = egressPortTableIndex * 8;
    __LOG_PARAM_FW(smemEgressPortInfoOfst);

    /* Get base address of the required Egress Port Info table entry in shared memory */
    __LOG_FW(("Get base address of the required Egress Port Info table entry in shared memory"));
    smemEgressPortInfoBaseAddr = DRAM_IFA_EGRESS_PORT_INFO_TABLE_ADDR + smemEgressPortInfoOfst;
    __LOG_PARAM_FW(smemEgressPortInfoBaseAddr);

    /* Get egress port information from shared memory.
       Read 4MSB which includes |port utilization(2B)|reserved(1B)|drop counter(1B)| */
    __LOG_FW(("Get egress port information from shared memory."));
    __LOG_FW(("Read 4MSB which includes |port utilization(2B)|reserved(1B)|drop counter(1B)|"));
    egressPortInfo = PPA_FW_SP_READ(smemEgressPortInfoBaseAddr);
    __LOG_PARAM_FW(egressPortInfo);

    /* Handle Egress Port Tx Utilization (bit#7) */
    __LOG_FW(("Handle Egress Port Tx Utilization (bit#7)"));
    if ( ((telemetryRequestVector>>7) & 0x1) == 1)
    {
        /* Egress port utilization: get it from shared memory and set it in packet IFA MD */
        __LOG_FW(("Egress port utilization: get it from shared memory and set it in packet IFA MD"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 22)  , (egressPortInfo>>16)  );
       __LOG_PARAM_FW((egressPortInfo>>16));
    }

    /* Handle RX timestamp (bit#4) */
    __LOG_FW(("Handle RX timestamp (bit#4)"));
    if ( ((telemetryRequestVector>>4) & 0x1) == 1)
    {
        /* Get Desc<Timestamp[31:30] */
        __LOG_FW(("Get Desc<Timestamp[31:30] "));
        ts_31_30 = (thr75_desc_ptr->phal2ppa.timestamp >> 30) & 0x3;
        __LOG_PARAM_FW(ts_31_30);

        /* Get ptp TAI IF select */
        __LOG_FW(("Get ptp TAI IF select"));
        tai_sel = thr75_desc_ptr->phal2ppa.ptp_tai_select;
        __LOG_PARAM_FW(tai_sel);

        /* Read TOD word 1 which represents seconds field [31:0] */
        __LOG_FW(("Read TOD word 1 which represents seconds field [31:0]"));
        ts_sec = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
        __LOG_PARAM_FW(ts_sec);

        /* Clear 2 LS bits of TAI.Seconds */
        __LOG_FW(("Clear 2 LS bits of TAI.Seconds"));
        ts_sec &= 0xFFFFFFFC;

        /* Set 2 LS bits with Desc<timestamp[31:30]> */
        __LOG_FW(("Set 2 LS bits with Desc<timestamp[31:30]"));
        ts_sec +=ts_31_30;

        /* IFA Metadata: Set 32 bits of seconds RX timestamp */
        __LOG_FW(("IFA Metadata: Set 32 bits of seconds RX timestamp"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 10), (ts_sec >> 16   ) );
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 12), ts_sec );

        /* IFA Metadata: Rx Timestamp nano[29:0] = desc<Timestamp[29:0]> */
        __LOG_FW(("IFA MD: Rx Timestamp nano[29:0] = desc<Timestamp[29:0]>"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 14), ( (thr75_desc_ptr->phal2ppa.timestamp >> 16)&0x3FFF) );
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 16), thr75_desc_ptr->phal2ppa.timestamp );
    }

    /* Egress port drop counter: get 1 MS byte from shared memory and set it in packet IFA MD */
    __LOG_FW(("Egress port drop counter: get 1 MS byte from shared memory and set it in packet IFA MD"));
    PPA_FW_SP_BYTE_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 5)  , egressPortInfo  );

    /* Egress port drop counter: get 4 LS bytes from shared memory and set it in packet IFA MD */
    __LOG_FW(("Egress port drop counter: get 4 LS bytes from shared memory and set it in packet IFA MD"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + ifaMdStartOffset + 28)  , PPA_FW_SP_READ(smemEgressPortInfoBaseAddr + 4)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemEgressPortInfoBaseAddr + 4));
    __PRAGMA_NO_REORDER

    /* Checksum new IFA metadata (32 bytes) */
    __LOG_FW(("Checksum new IFA metadata (32 bytes)"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_OFFSET, 0);
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS8_OFFSET, 0);
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS16_OFFSET, 0);
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR75_IFA_Egress_Node_Metadata_LEN8_FROM_PKT_ifa_md__deviceId_PLUS24_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Calculate and update UDP checksum only if origin cs is not zero (from RFC768) */
    __LOG_FW(("Calculate and update UDP checksum only if origin cs is not zero (from RFC768)"));
    if(incomingL4CsValue != 0)
    {   /* Origin/old UDP CS != 0 */

        /* Store the new UDP CS */
        __LOG_FW(("Store the new UDP CS using accelerator cmd"));
        ACCEL_CMD_TRIG(CSUM_STORE_UDP_THR75_IFA_Egress_Node_Metadata_TO_PKT_udp_header__Checksum_OFFSET, 0);
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Trigger GOP to add Tx Timestamp and update UDP/TCP checksum
        - Desc<MAC Timestamping En>    = 1
        - Desc<PTP Packet Format>      = PTPv2 (0x0)
        - Desc<PTP Action>             = AddTime (0x4)
        - Desc<Timestamp Mask Profile> = 0x2
        - Desc<Timestamp Offset>       = IFA Metadata offset + 18 bytes
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Enable PTP TSU"));

    if ( ((telemetryRequestVector>>5) & 0x1) == 1)
    {
        /* Desc<MAC Timestamping En>=1 */
        __LOG_FW(("Desc<MAC Timestamping En>=1"));
        ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x1_LEN1_TO_DESC_phal2ppa__mac_timestamping_en_OFFSET, 0);

        /* Desc<PTP Packet Format>=PTPv2 (0x0) */
        __LOG_FW(("Desc<PTP Packet Format>=PTPv2 (0x0)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x0_LEN3_TO_DESC_phal2ppa__ptp_packet_format_OFFSET, 0);

        /* Desc<PTP Action>=AddTime (0x4) */
        __LOG_FW(("Desc<PTP Action>=AddTime (0x4)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x4_LEN4_TO_DESC_phal2ppa__ptp_action_OFFSET, 0);

        /* Byte offset set of the “Tx Timestamp Nano Upper” field relative to the start of the packet */
        __LOG_FW(("Byte offset set of the “Tx Timestamp Nano Upper” field relative to the start of the packet"));
        thr75_desc_ptr->phal2ppa.timestamp_offset = timestampOffset;

        if(incomingL4CsValue != 0)
        {   /* Origin/old UDP CS != 0 */

  		    /* Calculate L4 checksum relative to start of packet. Value of l4ChecksumOffset is offset of field in PHA header space.
  		     * First L3 byte starts from byte# 16 in this memory. Need to compensate l4ChecksumOffset by 16 to get offset from L3 header.  */
            __LOG_FW(("Calculate L4 checksum relative to start of packet"));
            l4ChecksumOffset = l3Offset + l4ChecksumOffset - 16;
            __LOG_PARAM_FW(l4ChecksumOffset);

  		    /* Desc<Egress Checksum Mode>=ChecksumField (0x1), Desc<Timestamp Mask Profile>=2, Desc<UDP Checksum Update En>=1 */
            __LOG_FW(("Desc<Egress Checksum Mode>=ChecksumField (0x1), Desc<Timestamp Mask Profile>=2, Desc<UDP Checksum Update En>=1"));
			ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x15_LEN5_TO_DESC_phal2ppa__egress_checksum_mode_OFFSET, 0);

            /* Set Desc<udp_checksum> with l4ChecksumOffset. First write it to template then use accelerator to copy to descriptor */
            __LOG_FW(("Set Desc<udp_checksum> with l4ChecksumOffset. First write it to template then use accelerator to copy to descriptor"));
            PPA_FW_SP_BYTE_WRITE((FALCON_CFG_REGs_lo + 15), l4ChecksumOffset);
	        __PRAGMA_NO_REORDER
			ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CFG_ifa_metadata_template__reserved6_LEN8_TO_DESC_phal2ppa__udp_checksum_offset_OFFSET, 0);
        }
        else
        {
			/* Set Desc<Timestamp Mask Profile>=2(010b), Clear Desc<UDP Checksum Update En>=0(0b) */
            __LOG_FW(("Set Desc<Timestamp Mask Profile>=2(010b), Clear Desc<UDP Checksum Update En>=0(0b)"));
			ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x4_LEN4_TO_DESC_phal2ppa__timestamp_mask_profile_OFFSET, 0);
        }
    }
    else
    {
        /* Bypass egress timestamp, Desc<MAC Timestamping En>=0 */
        __LOG_FW(("Bypass egress timestamp, Desc<MAC Timestamping En>=0"));
        ACCEL_CMD_TRIG(COPY_BITS_THR75_IFA_Egress_Node_Metadata_CONST_0x0_LEN1_TO_DESC_phal2ppa__mac_timestamping_en_OFFSET, 0);
    }

    /* Indicates HW about the new starting point of the packet header (+32 bytes) */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+32 bytes)"));
    thr75_desc_ptr->ppa_internal_desc.fw_bc_modification = 32;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(75);
#endif

exit_thread75:
    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence_reset_stack();
}


/*************************************************************************************************************************************//**
 * funcname     ENABLE_THR90_Router_SA_Modification_Thread
 * inparam      None
 * return       None
 * description  Thread outer L2 header MAC SA
 *
 *              cfg template:    Not in use (16B).
 *              src & trg entries: Not in use (2B)
 *                                 Not in use (2B)
 *
 *              hdr in:     | Extension space (32B) | outer_MAC Header(12B) |
 *              hdr out:    | Extension space (32B) | outer_MAC Header(12B) |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmwares actions:
 *               1.	Set outer L2.SMAC 6 Bytes:
 *                    pha_metadata[31:0] =MAC SA[31:0]
 *                    copy_reserved[19:4]=MAC SA[47:32]
 *
 *
 *
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR90_Router_SA_Modification_Thread)() {
    /* xt_iss_client_command("profile","enable");
    xt_iss_client_command("profile","disable"); _exit(0);  */

    __LOG_FW(("Copy 32b from pha_metadata to outer mac SA LSB"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR90_Router_SA_Modification_Thread_DESC_phal2ppa__pha_metadata_LEN4_TO_PKT_mac_header__mac_sa_31_16_OFFSET, 0);
    __LOG_FW(("Copy 16b from copy_reserved to outer mac SA MSB "));
    ACCEL_CMD_TRIG(COPY_BITS_THR90_Router_SA_Modification_Thread_DESC_phal2ppa__copy_reserved_LEN16_TO_PKT_mac_header__mac_sa_47_32_OFFSET, 0);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(90);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}

/*************************************************************************************************************************************//**
 * funcname     ENABLE_THR91_INQA_ECN_Thread
 * inparam      None
 * return       None
 * description  Thread outer L3
 *
 *              cfg template:    Not in use (16B)
 *
 *              src & trg entries: Not in use (2B)
 *                                 Not in use (2B)
 *
 *              hdr in:     | Extension space (32B) | IPv4(20B) | payload(16B)
 *              hdr out:    | Extension space (32B) | IPv4(20B) | payload(16B)
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmwares actions:
 *               1. copy desc<copy_reserved[4:3] to IPv4 ECN 2 bits
 *               2. copy desc<copy_reserved[5] to IPv4 flag_reserved bit
 *
 *
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR91_INQA_ECN_Thread)() {
    /* xt_iss_client_command("profile","enable");
    xt_iss_client_command("profile","disable"); _exit(0);  */
    struct ppa_in_desc* thr91_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr91_INQA_ECN_Thread_out_hdr* thr91_out_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr91_INQA_ECN_Thread_out_hdr);
    uint8_t ipDesc = thr91_desc_ptr->phal2ppa.copy_reserved;

    __LOG_FW(("IPv4.ecn = desc.copy_reserved[4:3]"));
    thr91_out_hdr_ptr->IPv4_Header.ecn = (ipDesc >> 3) & 0x3;
    __LOG_FW(("IPv4.flag_reserved = desc.copy_reserved[5]"));
    thr91_out_hdr_ptr->IPv4_Header.flag_reserved = (ipDesc >> 5) & 0x1;

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(91);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}

/*************************************************************************************************************************************//**
 * funcname     ENABLE_THR92_VXLAN_DCI_VNI_Remapping_Thread
 * inparam      None
 * return       None
 * description  Thread outer L3
 *
 *              cfg template:    Not in use (16B)
 *
 *              src & trg entries: Not in use (2B)
 *                                 Not in use (2B)
 *
 *              hdr in (IPv4\6):     | Extension space (32B) | IPv4/6(20/40 B) | UDP(8B) | VXLAN(8B) |
 *              hdr out (IPv4\6):    | Extension space (32B) | IPv4/6(20/40 B) | UDP(8B) | VXLAN(8B) |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmwares actions:
 *               1. out header vxlan.VNI = pha_metadata[23:0] according to IP version.
 *
 *
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR92_VXLAN_DCI_VNI_Remapping_Thread)() {
    /* xt_iss_client_command("profile","enable");
    xt_iss_client_command("profile","disable"); _exit(0);  */
    uint32_t ip_ver = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + THR92_IN_IP__VER_OFFSET) >> 4;

    if (ip_ver == 4)
    {
        __LOG_FW(("IPv4 vxlan.VNI = desc.pha_metadata[23:0]"));
        ACCEL_CMD_TRIG(COPY_BITS_THR92_VXLAN_DCI_VNI_Remapping_Thread_DESC_phal2ppa__pha_metadata_PLUS8_LEN24_TO_PKT_ipv4__vxlan_header__VNI_OFFSET , 0);
    }
    else /* IPv6 */
    {
        __LOG_FW(("IPv6 vxlan.VNI = desc.pha_metadata[23:0]"));
        ACCEL_CMD_TRIG(COPY_BITS_THR92_VXLAN_DCI_VNI_Remapping_Thread_DESC_phal2ppa__pha_metadata_PLUS8_LEN24_TO_PKT_ipv6__vxlan_header__VNI_OFFSET , 0);
    }

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(92);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


#if RECOVER_FROM_EXCEPTION
/********************************************************************************************************************//**
 * funcname        falconRecoveFromException
 * inparam         None
 * return          None
 * description     Recover from core exception by executing packet swap sequence to send current packet and get the next one.
 *                 fw_drop bit is set in order to indicate that this packet should be dropped.
 *                 NOTE: can use the following instruction to generate an exception for testing => asm volatile ("syscall");
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void falconRecoveFromException ()
{
    /* Get pointer to descriptor */
    struct ppa_in_desc* desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Discard the packet. Indicates it by setting fw_drop field in descriptor to '1' */
    __LOG_FW(("Discard the packet. Indicates it by setting fw_drop field in descriptor to '1' "));
    desc_ptr->ppa_internal_desc.fw_drop_code = 0x01;
    __LOG_PARAM_FW(desc_ptr->internal_desc.fw_drop_code);

    /* Notify MG that PPN is recovered from an exception event  */
    __LOG_FW(("Notify MG that PPN is recovered from an exception event"));
    ppn2mg_failure_event(PPN2MG_PPN_EXCEPTION_RECOVER_EVENT,0,0,DONT_STOP_PPN);

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}
#endif	/* RECOVER_FROM_EXCEPTION */



extern void invalidFirmwareThread();

THREAD_TYPE phaThreadsTypeFalconImage01[] = {
    /* 0*/  PPA_FW(THR0_DoNothing)
    /* 1*/ ,PPA_FW(THR1_SRv6_End_Node)
    /* 2*/ ,PPA_FW(THR2_SRv6_Source_Node_1_segment)
    /* 3*/ ,PPA_FW(THR3_SRv6_Source_Node_First_Pass_2_3_segments)
    /* 4*/ ,PPA_FW(THR4_SRv6_Source_Node_Second_Pass_3_segments)
    /* 5*/ ,PPA_FW(THR5_SRv6_Source_Node_Second_Pass_2_segments)
    /* 6*/ ,PPA_FW(THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4)
    /* 7*/ ,PPA_FW(THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6)
    /* 8*/ ,PPA_FW(THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4)
    /* 9*/ ,PPA_FW(THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6)
    /*10*/ ,PPA_FW(THR10_Cc_Erspan_TypeII_SrcDevMirroring)
    /*11*/ ,PPA_FW(THR11_VXLAN_GPB_SourceGroupPolicyID)
    /*12*/ ,PPA_FW(THR12_MPLS_SR_NO_EL)
    /*13*/ ,PPA_FW(THR13_MPLS_SR_ONE_EL)
    /*14*/ ,PPA_FW(THR14_MPLS_SR_TWO_EL)
    /*15*/ ,PPA_FW(THR15_MPLS_SR_THREE_EL)
    /*16*/ ,invalidFirmwareThread
    /*17*/ ,invalidFirmwareThread
    /*18*/ ,invalidFirmwareThread
    /*19*/ ,invalidFirmwareThread
    /*20*/ ,invalidFirmwareThread
    /*21*/ ,invalidFirmwareThread
    /*22*/ ,invalidFirmwareThread
    /*23*/ ,invalidFirmwareThread
    /*24*/ ,invalidFirmwareThread
    /*25*/ ,invalidFirmwareThread
    /*26*/ ,invalidFirmwareThread
    /*27*/ ,invalidFirmwareThread
    /*28*/ ,invalidFirmwareThread
    /*29*/ ,invalidFirmwareThread
    /*30*/ ,invalidFirmwareThread
    /*31*/ ,invalidFirmwareThread
    /*32*/ ,invalidFirmwareThread
    /*33*/ ,invalidFirmwareThread
    /*34*/ ,invalidFirmwareThread
    /*35*/ ,invalidFirmwareThread
    /*36*/ ,invalidFirmwareThread
    /*37*/ ,invalidFirmwareThread
    /*38*/ ,invalidFirmwareThread
    /*39*/ ,invalidFirmwareThread
    /*40*/ ,invalidFirmwareThread
    /*41*/ ,invalidFirmwareThread
    /*42*/ ,invalidFirmwareThread
    /*43*/ ,invalidFirmwareThread
    /*44*/ ,invalidFirmwareThread
    /*45*/ ,invalidFirmwareThread
    /*46*/ ,PPA_FW(THR46_SFLOW_IPv4)
    /*47*/ ,PPA_FW(THR47_SFLOW_IPv6)
    /*48*/ ,PPA_FW(THR48_SRV6_Best_Effort)
    /*49*/ ,PPA_FW(THR49_SRV6_Source_Node_1_CONTAINER)
    /*50*/ ,PPA_FW(THR50_SRV6_Source_Node_First_Pass_1_CONTAINER)
    /*51*/ ,PPA_FW(THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER)
    /*52*/ ,PPA_FW(THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER)
    /*53*/ ,PPA_FW(THR53_SRV6_End_Node_GSID_COC32)
    /*54*/ ,invalidFirmwareThread
    /*55*/ ,invalidFirmwareThread
    /*56*/ ,invalidFirmwareThread
    /*57*/ ,PPA_FW(THR57_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET)
    /*58*/ ,invalidFirmwareThread
    /*59*/ ,invalidFirmwareThread
    /*60*/ ,PPA_FW(THR60_DropAllTraffic)
    /*61*/ ,PPA_FW(THR61_save_target_port_info)
    /*62*/ ,PPA_FW(THR62_enhanced_sFlow_fill_remain_IPv4)
    /*63*/ ,PPA_FW(THR63_enhanced_sFlow_fill_remain_IPv6)
    /*64*/ ,PPA_FW(THR64_Erspan_TypeII_SameDevMirroring_Ipv4)
    /*65*/ ,PPA_FW(THR65_Erspan_TypeII_SameDevMirroring_Ipv6)
    /*66*/ ,PPA_FW(THR66_enhanced_sFlow)
    /*67*/ ,PPA_FW(THR67_Forward_DSA_Routed_Clear)
    /*68*/ ,PPA_FW(THR68_Erspan_TypeI_Ipv4)
    /*69*/ ,PPA_FW(THR69_Erspan_TypeI_Ipv6)
    /*70*/ ,invalidFirmwareThread
    /*71*/ ,PPA_FW(THR71_IFA_Header)
    /*72*/ ,PPA_FW(THR72_IFA_Ingress_Metadata)
    /*73*/ ,PPA_FW(THR73_IFA_Transit_Metadata)
    /*74*/ ,PPA_FW(THR74_IFA_Egress_Node_Mirror)
    /*75*/ ,PPA_FW(THR75_IFA_Egress_Node_Metadata)
    /*76*/ ,invalidFirmwareThread
    /*77*/ ,invalidFirmwareThread
    /*78*/ ,invalidFirmwareThread
    /*79*/ ,invalidFirmwareThread
    /*80*/ ,invalidFirmwareThread
    /*81*/ ,invalidFirmwareThread
    /*82*/ ,invalidFirmwareThread
    /*83*/ ,invalidFirmwareThread
    /*84*/ ,invalidFirmwareThread
    /*85*/ ,invalidFirmwareThread
    /*86*/ ,invalidFirmwareThread
    /*87*/ ,invalidFirmwareThread
    /*88*/ ,invalidFirmwareThread
    /*89*/ ,invalidFirmwareThread
    /*90*/ ,PPA_FW(THR90_Router_SA_Modification_Thread)
    /*91*/ ,PPA_FW(THR91_INQA_ECN_Thread)
    /*92*/ ,PPA_FW(THR92_VXLAN_DCI_VNI_Remapping_Thread)
    /*93*/ ,invalidFirmwareThread
    /*94*/ ,invalidFirmwareThread
    /*95*/ ,invalidFirmwareThread
    /*96*/ ,invalidFirmwareThread
    /*97*/ ,invalidFirmwareThread
    /*98*/ ,invalidFirmwareThread
    /*99*/ ,invalidFirmwareThread
};
