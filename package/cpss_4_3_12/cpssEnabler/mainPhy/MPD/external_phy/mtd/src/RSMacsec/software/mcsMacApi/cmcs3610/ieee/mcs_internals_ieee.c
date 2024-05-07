
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
// * $Rev: 
// * $Date: 
// ******************************************************************************


#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "mcs_internals_ieee.h"

//-------------------------------------------------------------------------------
// Register Access
//-------------------------------------------------------------------------------

   
                  
                                                   
                         
                               
                                                                                                                                           
             
             
                                  
                           
                                                         
                                      
/* Read a value from a register
 */
 int ra01_read_reg_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned size, ra01_register_t volatile * value )
{
    int ret = -EFAULT;
    uint32_t volatile * d = (uint32_t volatile *)value;
    ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
    if( ret < 0 ) { return ret; }
    // Only read the MSB if there are some...
    if( size > 1 ) {
        ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
    }

    return ret;
}

/* write a value to a register
 */
 int ra01_write_reg_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned size, ra01_register_t value )
{
    int ret = -EFAULT;  // bad address

    uint32_t const * d = (uint32_t const *)&value;
    ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)(*d++) );
    if( ret < 0 ) { return ret; }
    // Only read the MSB if there are some...
    if( size > 1 ) {
        ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t)(*d) );
    }

    return ret;
}

/* This part has 64-bit registers and 32-bit access, but has 32-bit registers because reasons. 
 */
int ra01_write_reg32_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, uint32_t value )
{
    int ret = -EFAULT;  // bad address
    ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)value );
    return ret;
}

int ra01_read_reg32_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, uint32_t volatile * value )
{
    int ret = -EFAULT;
    ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)value );
    return ret;
}

int ra01_set_bit_field_by_id( ra01_register_t * reg, unsigned * msb, unsigned * lsb, ra01_register_t * value, unsigned num_fields ) 
{
    unsigned       i;
    int            ret = -EFAULT;

    for(i=0; i<num_fields; i++) {
       ra01_register_t mask = ra01_make_mask( msb[i], lsb[i] );

       // check that the value fits in the mask..
       if( ((~mask) & (value[i] << lsb[i])) == 0 ) {
           *reg = ra01_set_bit_field( reg, msb[i], lsb[i], value[i] );
           ret = 0;
       } else {
           ret = -EINVAL;
       }
    }


    return ret;
}

#include <stdio.h>
#include <inttypes.h>

void ra01_get_bit_field_by_id( ra01_register_t reg, unsigned * msb, unsigned * lsb, ra01_register_t * value, unsigned num_fields ) 
{
    unsigned       i;
    for(i=0; i<num_fields; i++) {
       value[i] = ra01_get_bit_field( reg, msb[i], lsb[i] );
    }
}

int ra01_write_bit_field_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned size, unsigned msb, unsigned lsb, ra01_register_t value ) 
{
    int             ret = -EFAULT;
    ra01_register_t reg = 0;

    ret = ra01_read_reg_by_id( rmsDev_p, offset, size, &reg );
    if( ret < 0 ) return ret;

    ret = ra01_set_bit_field_by_id( &reg, &msb, &lsb, &value,1 );
    if( ret < 0 ) return ret;

    ret = ra01_write_reg_by_id( rmsDev_p, offset, size, reg );
    if( ret < 0 ) return ret;

    return ret;
}

int ra01_read_bit_field_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned size, unsigned msb, unsigned lsb, ra01_register_t * value ) 
{
    int             ret = -EFAULT;
    ra01_register_t reg = 0;

    ret = ra01_read_reg_by_id( rmsDev_p, offset, size, &reg );
    if( ret < 0 ) return ret;

    ra01_get_bit_field_by_id( reg, &msb,&lsb, value,1 );
    if( ret < 0 ) return ret;

    return ret;
}

/* Helpers for MEM access 
 */
int ra01_read_mem_field_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned step, unsigned size, unsigned index, ra01_register_t volatile * buffer ) 
{
    int      ret = 0;

    uintptr_t l_offset = offset + index * step;
    unsigned i;
    uint32_t volatile * d = (uint32_t volatile *)buffer;

    for( i = 0; (i < size) ; i++ ) {
        ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, (uintAddrBus_t)l_offset + (uintAddrBus_t)(i * sizeof( *d )), (uintRegAccess_t*)d++ );
        if( ret < 0 ) { return ret; }
    }

    return ret;
}

static void bit_bash( ra01_register_t const * src, unsigned s_pos, ra01_register_t * dst, unsigned d_pos, unsigned bits )
{
    unsigned size = sizeof( ra01_register_t ) * 8;

    do {
        unsigned start = (s_pos > d_pos) ? s_pos : d_pos;
        unsigned end = (size > (bits + start) ? (bits + start) : size) - 1;
        unsigned run = (end - start) + 1;
        ra01_register_t mask = 0;

        if( d_pos == 0 ) {
            // new element in value.
            mask = ~ra01_make_mask( run - 1, 0 );
            *dst &= mask;
            *dst |= (*src & ra01_make_mask( end, start )) >> start;
            if( run == size ) {
                dst++;
                d_pos = 0;
            } else {
                d_pos = run;
            }
            s_pos = 0;
            src++;
        } else if( s_pos == 0 ) {
            // adding to previous value
            mask = ~ra01_make_mask( end, start );
            *dst &= mask;
            *dst |= (*src & ra01_make_mask( end - start, 0 )) << start;
            d_pos = 0;
            s_pos = (size - start) % size;
            dst++;
        } else {
            return;
        }
        if( (run > bits) || (run == 0) ) {
            return;
        } 
        bits -= run;
    } while( bits );
}

/* Insert bits into a bit stream from value.
 * Example:
 *   set_bits( dst, 24, 16, src ) 
 *   will copy src[8:0] to dst[24:16]
 */
static void set_bits( ra01_register_t * stream, unsigned msb, unsigned lsb, ra01_register_t const * value ) 
{
    unsigned size = sizeof( ra01_register_t ) * 8;
    unsigned bits = (msb - lsb) + 1;

    if( lsb > msb ) {
        return;
    }

    // Adjust the start to fall into a register 
    while( lsb >= size ) {
        lsb -= size;
        msb -= size;
        stream++;
    }

    bit_bash( value, 0, stream, lsb, bits );
}

/* Extract bits from a bit stream to a target. 
 * Example:
 *   get_bits( src, 24, 16, dst ) 
 *   will copy src[24:16] to dst[8:0]
 */
static void get_bits( ra01_register_t const * stream, unsigned msb, unsigned lsb, ra01_register_t * target )
{
    unsigned size = sizeof( ra01_register_t ) * 8;
    unsigned bits = msb - lsb + 1;

    if( lsb > msb ) {
        return;
    }

    // find the right starting word in stream:
    while( lsb >= size ) {
        lsb -= size;
        msb -= size;
        stream++;
    }

    bit_bash( stream, lsb, target, 0, bits );
}

void ra01_update_mem_field_by_id( ra01_register_t * field, uintAddrBus_t * offset, unsigned * msb, unsigned * lsb, ra01_register_t ** value, unsigned num_fields ) 
{
    unsigned i;
    unsigned size = sizeof( ra01_register_t ) * 8;
    for(i=0; i<num_fields; i++) {
       set_bits( field, msb[i] + offset[i]*size, lsb[i] + offset[i]*size, value[i] );
    }
}

int ra01_extract_mem_field_by_id( ra01_register_t const * field, uintAddrBus_t offset, unsigned msb, unsigned lsb, void * target, ra01_mem_access_type_t type )
{
    unsigned size = sizeof(ra01_register_t ) * 8;

    switch(type) {
        case RA01_TYPE_BOOL:
            {
                bool * v = (bool *)target;
                ra01_register_t tmp = ra01_get_bit_field( field[offset], msb, lsb );
                *v = (tmp & 1) == 1;
            }
            break;
        case RA01_TYPE_REGISTER:
            {
                ra01_register_t * v = (ra01_register_t *)target;

                *v = ra01_get_bit_field( field[offset], msb, lsb );
            }
            break;
        case RA01_TYPE_MEMORY:
            {
               int l_msb = msb + (offset * size);
               int l_lsb = lsb + (offset * size);
               get_bits( field, l_msb, l_lsb, target );
            }
            break;
        default:
            return -EFAULT;
    }

    return 0;
}

int ra01_write_mem_field_by_id( RmsDev_t * rmsDev_p, uintAddrBus_t offset, unsigned step, unsigned size, unsigned index, ra01_register_t const  * buffer )
{
    int ret = 0;
    uintptr_t l_offset = offset + index * step;
    
    unsigned i;
    uint32_t const * d = (uint32_t const *)buffer;

    for( i = 0; (i < size); i++ ) {
        ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, (uintAddrBus_t)l_offset + (uintAddrBus_t)(i * sizeof( *d )), (uintRegAccess_t)(*d++) );
        if( ret < 0 ) { return ret; }
    }

    return ret;
}

//-------------------------------------------------------------------------------
// Converters between raw and cooked formats
//-------------------------------------------------------------------------------
     
           

























































 











































 
static void ra01_rs_mcs_cpm_rx_secy_map_to_raw (Ra01IfRsMcsCpmRxSecyMap_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[2];
    unsigned msb[2];
    unsigned lsb[2];
    ra01_register_t* values[2];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 4;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->secy);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 5;
    lsb[1] = 5;
    values[1] = (ra01_register_t*)&(p->ctrlPkt);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 2);
}


static void ra01_rs_mcs_cpm_rx_secy_plcy_to_raw (Ra01IfRsMcsCpmRxSecyPlcy_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[9];
    unsigned msb[9];
    unsigned lsb[9];
    ra01_register_t* values[9];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 0;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->controlledPortEnabled);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 2;
    lsb[1] = 1;
    values[1] = (ra01_register_t*)&(p->validateFrames);
     offset[2] = ((uintAddrBus_t)0x00000000);
    msb[2] = 4;
    lsb[2] = 3;
    values[2] = (ra01_register_t*)&(p->stripSectagIcv);
     offset[3] = ((uintAddrBus_t)0x00000000);
    msb[3] = 8;
    lsb[3] = 5;
    values[3] = (ra01_register_t*)&(p->cipher);
     offset[4] = ((uintAddrBus_t)0x00000000);
    msb[4] = 15;
    lsb[4] = 9;
    values[4] = (ra01_register_t*)&(p->confidentialityOffset);
     offset[5] = ((uintAddrBus_t)0x00000000);
    msb[5] = 16;
    lsb[5] = 16;
    values[5] = (ra01_register_t*)&(p->preSectagAuthEnable);
     offset[6] = ((uintAddrBus_t)0x00000000);
    msb[6] = 17;
    lsb[6] = 17;
    values[6] = (ra01_register_t*)&(p->replayProtect);
     offset[7] = ((uintAddrBus_t)0x00000000);
    msb[7] = 49;
    lsb[7] = 18;
    values[7] = (ra01_register_t*)&(p->replayWindow);
     offset[8] = ((uintAddrBus_t)0x00000000);
    msb[8] = 72;
    lsb[8] = 50;
    values[8] = (ra01_register_t*)&(p->reserved);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 9);
}


static void ra01_rs_mcs_cpm_rx_sa_map_to_raw (Ra01IfRsMcsCpmRxSaMap_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[2];
    unsigned msb[2];
    unsigned lsb[2];
    ra01_register_t* values[2];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 5;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->saIndex);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 6;
    lsb[1] = 6;
    values[1] = (ra01_register_t*)&(p->saInUse);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 2);
}


static void ra01_rs_mcs_cpm_rx_sa_plcy_to_raw (Ra01IfRsMcsCpmRxSaPlcy_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[4];
    unsigned msb[4];
    unsigned lsb[4];
    ra01_register_t* values[4];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 255;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->sak);
     offset[1] = ((uintAddrBus_t)0x00000004);
    msb[1] = 127;
    lsb[1] = 0;
    values[1] = (ra01_register_t*)&(p->hashkey);
     offset[2] = ((uintAddrBus_t)0x00000006);
    msb[2] = 95;
    lsb[2] = 0;
    values[2] = (ra01_register_t*)&(p->salt);
     offset[3] = ((uintAddrBus_t)0x00000007);
    msb[3] = 63;
    lsb[3] = 32;
    values[3] = (ra01_register_t*)&(p->ssci);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 4);
}


static void ra01_rs_mcs_cpm_rx_sa_pn_table_to_raw (Ra01IfRsMcsCpmRxSaPnTable_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[1];
    unsigned msb[1];
    unsigned lsb[1];
    ra01_register_t* values[1];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 63;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->nextPn);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 1);
}


static void ra01_rs_mcs_cpm_rx_flowid_tcam_data_to_raw (Ra01IfRsMcsCpmRxFlowidTcamData_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[15];
    unsigned msb[15];
    unsigned lsb[15];
    ra01_register_t* values[15];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 47;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->macDa);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 95;
    lsb[1] = 48;
    values[1] = (ra01_register_t*)&(p->macSa);
     offset[2] = ((uintAddrBus_t)0x00000001);
    msb[2] = 47;
    lsb[2] = 32;
    values[2] = (ra01_register_t*)&(p->etherType);
     offset[3] = ((uintAddrBus_t)0x00000001);
    msb[3] = 67;
    lsb[3] = 48;
    values[3] = (ra01_register_t*)&(p->outerTagId);
     offset[4] = ((uintAddrBus_t)0x00000002);
    msb[4] = 7;
    lsb[4] = 4;
    values[4] = (ra01_register_t*)&(p->outerPriority);
     offset[5] = ((uintAddrBus_t)0x00000002);
    msb[5] = 27;
    lsb[5] = 8;
    values[5] = (ra01_register_t*)&(p->secondOuterTagId);
     offset[6] = ((uintAddrBus_t)0x00000002);
    msb[6] = 31;
    lsb[6] = 28;
    values[6] = (ra01_register_t*)&(p->secondOuterPriority);
     offset[7] = ((uintAddrBus_t)0x00000002);
    msb[7] = 47;
    lsb[7] = 32;
    values[7] = (ra01_register_t*)&(p->bonusData);
     offset[8] = ((uintAddrBus_t)0x00000002);
    msb[8] = 55;
    lsb[8] = 48;
    values[8] = (ra01_register_t*)&(p->tagMatchBitmap);
     offset[9] = ((uintAddrBus_t)0x00000002);
    msb[9] = 59;
    lsb[9] = 56;
    values[9] = (ra01_register_t*)&(p->packetType);
     offset[10] = ((uintAddrBus_t)0x00000002);
    msb[10] = 62;
    lsb[10] = 60;
    values[10] = (ra01_register_t*)&(p->outerVlanType);
     offset[11] = ((uintAddrBus_t)0x00000002);
    msb[11] = 65;
    lsb[11] = 63;
    values[11] = (ra01_register_t*)&(p->innerVlanType);
     offset[12] = ((uintAddrBus_t)0x00000003);
    msb[12] = 8;
    lsb[12] = 2;
    values[12] = (ra01_register_t*)&(p->numTags);
     offset[13] = ((uintAddrBus_t)0x00000003);
    msb[13] = 9;
    lsb[13] = 9;
    values[13] = (ra01_register_t*)&(p->express);
     offset[14] = ((uintAddrBus_t)0x00000003);
    msb[14] = 10;
    lsb[14] = 10;
    values[14] = (ra01_register_t*)&(p->port);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 15);
}


static void ra01_rs_mcs_cpm_rx_flowid_tcam_mask_to_raw (Ra01IfRsMcsCpmRxFlowidTcamMask_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[15];
    unsigned msb[15];
    unsigned lsb[15];
    ra01_register_t* values[15];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 47;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->maskMacDa);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 95;
    lsb[1] = 48;
    values[1] = (ra01_register_t*)&(p->maskMacSa);
     offset[2] = ((uintAddrBus_t)0x00000001);
    msb[2] = 47;
    lsb[2] = 32;
    values[2] = (ra01_register_t*)&(p->maskEtherType);
     offset[3] = ((uintAddrBus_t)0x00000001);
    msb[3] = 67;
    lsb[3] = 48;
    values[3] = (ra01_register_t*)&(p->maskOuterTagId);
     offset[4] = ((uintAddrBus_t)0x00000002);
    msb[4] = 7;
    lsb[4] = 4;
    values[4] = (ra01_register_t*)&(p->maskOuterPriority);
     offset[5] = ((uintAddrBus_t)0x00000002);
    msb[5] = 27;
    lsb[5] = 8;
    values[5] = (ra01_register_t*)&(p->maskSecondOuterTagId);
     offset[6] = ((uintAddrBus_t)0x00000002);
    msb[6] = 31;
    lsb[6] = 28;
    values[6] = (ra01_register_t*)&(p->maskSecondOuterPriority);
     offset[7] = ((uintAddrBus_t)0x00000002);
    msb[7] = 47;
    lsb[7] = 32;
    values[7] = (ra01_register_t*)&(p->maskBonusData);
     offset[8] = ((uintAddrBus_t)0x00000002);
    msb[8] = 55;
    lsb[8] = 48;
    values[8] = (ra01_register_t*)&(p->maskTagMatchBitmap);
     offset[9] = ((uintAddrBus_t)0x00000002);
    msb[9] = 59;
    lsb[9] = 56;
    values[9] = (ra01_register_t*)&(p->maskPacketType);
     offset[10] = ((uintAddrBus_t)0x00000002);
    msb[10] = 62;
    lsb[10] = 60;
    values[10] = (ra01_register_t*)&(p->maskOuterVlanType);
     offset[11] = ((uintAddrBus_t)0x00000002);
    msb[11] = 65;
    lsb[11] = 63;
    values[11] = (ra01_register_t*)&(p->maskInnerVlanType);
     offset[12] = ((uintAddrBus_t)0x00000003);
    msb[12] = 8;
    lsb[12] = 2;
    values[12] = (ra01_register_t*)&(p->maskNumTags);
     offset[13] = ((uintAddrBus_t)0x00000003);
    msb[13] = 9;
    lsb[13] = 9;
    values[13] = (ra01_register_t*)&(p->maskExpress);
     offset[14] = ((uintAddrBus_t)0x00000003);
    msb[14] = 10;
    lsb[14] = 10;
    values[14] = (ra01_register_t*)&(p->maskPort);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 15);
}


static void ra01_rs_mcs_cpm_rx_sc_cam_to_raw (Ra01IfRsMcsCpmRxScCam_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[2];
    unsigned msb[2];
    unsigned lsb[2];
    ra01_register_t* values[2];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 63;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->sci);
     offset[1] = ((uintAddrBus_t)0x00000001);
    msb[1] = 4;
    lsb[1] = 0;
    values[1] = (ra01_register_t*)&(p->secy);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 2);
}


static void ra01_rs_mcs_cpm_rx_sc_timer_to_raw (Ra01IfRsMcsCpmRxScTimer_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[4];
    unsigned msb[4];
    unsigned lsb[4];
    ra01_register_t* values[4];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 31;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->timer);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 33;
    lsb[1] = 32;
    values[1] = (ra01_register_t*)&(p->preTimeoutThreshSelect);
     offset[2] = ((uintAddrBus_t)0x00000000);
    msb[2] = 35;
    lsb[2] = 34;
    values[2] = (ra01_register_t*)&(p->actualTimeoutThreshSelect);
     offset[3] = ((uintAddrBus_t)0x00000000);
    msb[3] = 36;
    lsb[3] = 36;
    values[3] = (ra01_register_t*)&(p->enable);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 4);
}

 
static void ra01_rs_mcs_cpm_tx_secy_map_to_raw (Ra01IfRsMcsCpmTxSecyMap_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[4];
    unsigned msb[4];
    unsigned lsb[4];
    ra01_register_t* values[4];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 4;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->secy);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 5;
    lsb[1] = 5;
    values[1] = (ra01_register_t*)&(p->ctrlPkt);
     offset[2] = ((uintAddrBus_t)0x00000000);
    msb[2] = 10;
    lsb[2] = 6;
    values[2] = (ra01_register_t*)&(p->sc);
     offset[3] = ((uintAddrBus_t)0x00000000);
    msb[3] = 11;
    lsb[3] = 11;
    values[3] = (ra01_register_t*)&(p->auxiliaryPlcy);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 4);
}


static void ra01_rs_mcs_cpm_tx_secy_plcy_to_raw (Ra01IfRsMcsCpmTxSecyPlcy_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[10];
    unsigned msb[10];
    unsigned lsb[10];
    ra01_register_t* values[10];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 0;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->controlledPortEnabled);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 1;
    lsb[1] = 1;
    values[1] = (ra01_register_t*)&(p->protectFrames);
     offset[2] = ((uintAddrBus_t)0x00000000);
    msb[2] = 5;
    lsb[2] = 2;
    values[2] = (ra01_register_t*)&(p->cipher);
     offset[3] = ((uintAddrBus_t)0x00000000);
    msb[3] = 12;
    lsb[3] = 6;
    values[3] = (ra01_register_t*)&(p->confidentialityOffset);
     offset[4] = ((uintAddrBus_t)0x00000000);
    msb[4] = 13;
    lsb[4] = 13;
    values[4] = (ra01_register_t*)&(p->preSectagAuthEnable);
     offset[5] = ((uintAddrBus_t)0x00000000);
    msb[5] = 14;
    lsb[5] = 14;
    values[5] = (ra01_register_t*)&(p->sectagInsertMode);
     offset[6] = ((uintAddrBus_t)0x00000000);
    msb[6] = 21;
    lsb[6] = 15;
    values[6] = (ra01_register_t*)&(p->sectagOffset);
     offset[7] = ((uintAddrBus_t)0x00000000);
    msb[7] = 27;
    lsb[7] = 22;
    values[7] = (ra01_register_t*)&(p->sectagTci);
     offset[8] = ((uintAddrBus_t)0x00000000);
    msb[8] = 43;
    lsb[8] = 28;
    values[8] = (ra01_register_t*)&(p->mtu);
     offset[9] = ((uintAddrBus_t)0x00000000);
    msb[9] = 59;
    lsb[9] = 44;
    values[9] = (ra01_register_t*)&(p->reserved);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 10);
}


static void ra01_rs_mcs_cpm_tx_sa_map_to_raw (Ra01IfRsMcsCpmTxSaMap_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[7];
    unsigned msb[7];
    unsigned lsb[7];
    ra01_register_t* values[7];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 5;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->saIndex0);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 11;
    lsb[1] = 6;
    values[1] = (ra01_register_t*)&(p->saIndex1);
     offset[2] = ((uintAddrBus_t)0x00000000);
    msb[2] = 12;
    lsb[2] = 12;
    values[2] = (ra01_register_t*)&(p->autoRekeyEnable);
     offset[3] = ((uintAddrBus_t)0x00000000);
    msb[3] = 13;
    lsb[3] = 13;
    values[3] = (ra01_register_t*)&(p->saIndex0Vld);
     offset[4] = ((uintAddrBus_t)0x00000000);
    msb[4] = 14;
    lsb[4] = 14;
    values[4] = (ra01_register_t*)&(p->saIndex1Vld);
     offset[5] = ((uintAddrBus_t)0x00000000);
    msb[5] = 15;
    lsb[5] = 15;
    values[5] = (ra01_register_t*)&(p->txSaActive);
     offset[6] = ((uintAddrBus_t)0x00000000);
    msb[6] = 79;
    lsb[6] = 16;
    values[6] = (ra01_register_t*)&(p->sectagSci);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 7);
}


static void ra01_rs_mcs_cpm_tx_sa_plcy_to_raw (Ra01IfRsMcsCpmTxSaPlcy_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[5];
    unsigned msb[5];
    unsigned lsb[5];
    ra01_register_t* values[5];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 255;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->sak);
     offset[1] = ((uintAddrBus_t)0x00000004);
    msb[1] = 127;
    lsb[1] = 0;
    values[1] = (ra01_register_t*)&(p->hashkey);
     offset[2] = ((uintAddrBus_t)0x00000006);
    msb[2] = 95;
    lsb[2] = 0;
    values[2] = (ra01_register_t*)&(p->salt);
     offset[3] = ((uintAddrBus_t)0x00000007);
    msb[3] = 63;
    lsb[3] = 32;
    values[3] = (ra01_register_t*)&(p->ssci);
     offset[4] = ((uintAddrBus_t)0x00000008);
    msb[4] = 1;
    lsb[4] = 0;
    values[4] = (ra01_register_t*)&(p->sectagAn);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 5);
}


static void ra01_rs_mcs_cpm_tx_sa_pn_table_to_raw (Ra01IfRsMcsCpmTxSaPnTable_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[1];
    unsigned msb[1];
    unsigned lsb[1];
    ra01_register_t* values[1];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 63;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->nextPn);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 1);
}


static void ra01_rs_mcs_cpm_tx_flowid_tcam_data_to_raw (Ra01IfRsMcsCpmTxFlowidTcamData_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[15];
    unsigned msb[15];
    unsigned lsb[15];
    ra01_register_t* values[15];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 47;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->macDa);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 95;
    lsb[1] = 48;
    values[1] = (ra01_register_t*)&(p->macSa);
     offset[2] = ((uintAddrBus_t)0x00000001);
    msb[2] = 47;
    lsb[2] = 32;
    values[2] = (ra01_register_t*)&(p->etherType);
     offset[3] = ((uintAddrBus_t)0x00000001);
    msb[3] = 67;
    lsb[3] = 48;
    values[3] = (ra01_register_t*)&(p->outerTagId);
     offset[4] = ((uintAddrBus_t)0x00000002);
    msb[4] = 7;
    lsb[4] = 4;
    values[4] = (ra01_register_t*)&(p->outerPriority);
     offset[5] = ((uintAddrBus_t)0x00000002);
    msb[5] = 27;
    lsb[5] = 8;
    values[5] = (ra01_register_t*)&(p->secondOuterTagId);
     offset[6] = ((uintAddrBus_t)0x00000002);
    msb[6] = 31;
    lsb[6] = 28;
    values[6] = (ra01_register_t*)&(p->secondOuterPriority);
     offset[7] = ((uintAddrBus_t)0x00000002);
    msb[7] = 47;
    lsb[7] = 32;
    values[7] = (ra01_register_t*)&(p->bonusData);
     offset[8] = ((uintAddrBus_t)0x00000002);
    msb[8] = 55;
    lsb[8] = 48;
    values[8] = (ra01_register_t*)&(p->tagMatchBitmap);
     offset[9] = ((uintAddrBus_t)0x00000002);
    msb[9] = 59;
    lsb[9] = 56;
    values[9] = (ra01_register_t*)&(p->packetType);
     offset[10] = ((uintAddrBus_t)0x00000002);
    msb[10] = 62;
    lsb[10] = 60;
    values[10] = (ra01_register_t*)&(p->outerVlanType);
     offset[11] = ((uintAddrBus_t)0x00000002);
    msb[11] = 65;
    lsb[11] = 63;
    values[11] = (ra01_register_t*)&(p->innerVlanType);
     offset[12] = ((uintAddrBus_t)0x00000003);
    msb[12] = 8;
    lsb[12] = 2;
    values[12] = (ra01_register_t*)&(p->numTags);
     offset[13] = ((uintAddrBus_t)0x00000003);
    msb[13] = 9;
    lsb[13] = 9;
    values[13] = (ra01_register_t*)&(p->express);
     offset[14] = ((uintAddrBus_t)0x00000003);
    msb[14] = 10;
    lsb[14] = 10;
    values[14] = (ra01_register_t*)&(p->port);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 15);
}


static void ra01_rs_mcs_cpm_tx_flowid_tcam_mask_to_raw (Ra01IfRsMcsCpmTxFlowidTcamMask_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[15];
    unsigned msb[15];
    unsigned lsb[15];
    ra01_register_t* values[15];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 47;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->maskMacDa);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 95;
    lsb[1] = 48;
    values[1] = (ra01_register_t*)&(p->maskMacSa);
     offset[2] = ((uintAddrBus_t)0x00000001);
    msb[2] = 47;
    lsb[2] = 32;
    values[2] = (ra01_register_t*)&(p->maskEtherType);
     offset[3] = ((uintAddrBus_t)0x00000001);
    msb[3] = 67;
    lsb[3] = 48;
    values[3] = (ra01_register_t*)&(p->maskOuterTagId);
     offset[4] = ((uintAddrBus_t)0x00000002);
    msb[4] = 7;
    lsb[4] = 4;
    values[4] = (ra01_register_t*)&(p->maskOuterPriority);
     offset[5] = ((uintAddrBus_t)0x00000002);
    msb[5] = 27;
    lsb[5] = 8;
    values[5] = (ra01_register_t*)&(p->maskSecondOuterTagId);
     offset[6] = ((uintAddrBus_t)0x00000002);
    msb[6] = 31;
    lsb[6] = 28;
    values[6] = (ra01_register_t*)&(p->maskSecondOuterPriority);
     offset[7] = ((uintAddrBus_t)0x00000002);
    msb[7] = 47;
    lsb[7] = 32;
    values[7] = (ra01_register_t*)&(p->maskBonusData);
     offset[8] = ((uintAddrBus_t)0x00000002);
    msb[8] = 55;
    lsb[8] = 48;
    values[8] = (ra01_register_t*)&(p->maskTagMatchBitmap);
     offset[9] = ((uintAddrBus_t)0x00000002);
    msb[9] = 59;
    lsb[9] = 56;
    values[9] = (ra01_register_t*)&(p->maskPacketType);
     offset[10] = ((uintAddrBus_t)0x00000002);
    msb[10] = 62;
    lsb[10] = 60;
    values[10] = (ra01_register_t*)&(p->maskOuterVlanType);
     offset[11] = ((uintAddrBus_t)0x00000002);
    msb[11] = 65;
    lsb[11] = 63;
    values[11] = (ra01_register_t*)&(p->maskInnerVlanType);
     offset[12] = ((uintAddrBus_t)0x00000003);
    msb[12] = 8;
    lsb[12] = 2;
    values[12] = (ra01_register_t*)&(p->maskNumTags);
     offset[13] = ((uintAddrBus_t)0x00000003);
    msb[13] = 9;
    lsb[13] = 9;
    values[13] = (ra01_register_t*)&(p->maskExpress);
     offset[14] = ((uintAddrBus_t)0x00000003);
    msb[14] = 10;
    lsb[14] = 10;
    values[14] = (ra01_register_t*)&(p->maskPort);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 15);
}


static void ra01_rs_mcs_cpm_tx_sc_timer_to_raw (Ra01IfRsMcsCpmTxScTimer_t const * p, ra01_register_t * raw )
{

    uintAddrBus_t offset[6];
    unsigned msb[6];
    unsigned lsb[6];
    ra01_register_t* values[6];
    ra01_register_t                tmp;
    (void) tmp;

    offset[0] = ((uintAddrBus_t)0x00000000);
    msb[0] = 31;
    lsb[0] = 0;
    values[0] = (ra01_register_t*)&(p->timer);
     offset[1] = ((uintAddrBus_t)0x00000000);
    msb[1] = 33;
    lsb[1] = 32;
    values[1] = (ra01_register_t*)&(p->preTimeoutThreshSelect);
     offset[2] = ((uintAddrBus_t)0x00000000);
    msb[2] = 35;
    lsb[2] = 34;
    values[2] = (ra01_register_t*)&(p->actualTimeoutThreshSelect);
     offset[3] = ((uintAddrBus_t)0x00000000);
    msb[3] = 36;
    lsb[3] = 36;
    values[3] = (ra01_register_t*)&(p->enable);
     offset[4] = ((uintAddrBus_t)0x00000000);
    msb[4] = 37;
    lsb[4] = 37;
    values[4] = (ra01_register_t*)&(p->startMode);
     offset[5] = ((uintAddrBus_t)0x00000000);
    msb[5] = 38;
    lsb[5] = 38;
    values[5] = (ra01_register_t*)&(p->startGo);
     ra01_update_mem_field_by_id( raw, offset,msb,lsb, values, 6);
}

 



 int Ra01GetRsMcsTopVersion( RmsDev_t * rmsDev_p, 
     Ra01RsMcsTopVersion_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000000);


        // We are reading 24-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->major = (uint8_t)(reg & 0xff);
       value->minor = (uint8_t)((reg & 0xff00) >> 8);
       value->patch = (uint8_t)((reg & 0xff0000) >> 16);

    return ret;
}






 int Ra01AccRsMcsTopPortReset( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsTopPortReset_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000004) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->portReset) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->portReset = (uint8_t)(reg & 0x1);
    }

    return ret;
}






 int Ra01AccRsMcsTopPortConfig( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsTopPortConfig_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000008) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->parseDepth) & 0x7)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 3-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->parseDepth = (uint8_t)(reg & 0x7);
    }

    return ret;
}






 int Ra01AccRsMcsTopChannelConfig( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsTopChannelConfig_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 2) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x0000000c) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->chBypass) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->chBypass = (uint8_t)(reg & 0x1);
    }

    return ret;
}










 int Ra01AccRsMcsTopMcsBpCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsTopMcsBpCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000014);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rxPolicyFifoCredits) & 0xf)
        | (((ra01_register_t)(value->rxDataFifoCredits) << 8) & 0x3f00)
        | (((ra01_register_t)(value->txPolicyFifoCredits) << 16) & 0xf0000)
        | (((ra01_register_t)(value->txDataFifoCredits) << 24) & 0x3f000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 20-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rxPolicyFifoCredits = (uint8_t)(reg & 0xf);
       value->rxDataFifoCredits = (uint8_t)((reg & 0x3f00) >> 8);
       value->txPolicyFifoCredits = (uint8_t)((reg & 0xf0000) >> 16);
       value->txDataFifoCredits = (uint8_t)((reg & 0x3f000000) >> 24);
    }

    return ret;
}






 int Ra01AccRsMcsTopOutputAdapterCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsTopOutputAdapterCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000018);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rxAfullThreshold) & 0xf)
        | (((ra01_register_t)(value->txAfullThreshold) << 8) & 0xf00)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 8-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rxAfullThreshold = (uint8_t)(reg & 0xf);
       value->txAfullThreshold = (uint8_t)((reg & 0xf00) >> 8);
    }

    return ret;
}






 int Ra01AccRsMcsTopScratch( RmsDev_t * rmsDev_p, 
     Ra01RsMcsTopScratch_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x0000001c);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scratch) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scratch = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsTopDbgMuxSel( RmsDev_t * rmsDev_p, 
     Ra01RsMcsTopDbgMuxSel_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000020);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->seg0) & 0xf)
        | (((ra01_register_t)(value->seg1) << 8) & 0xf00)
        | (((ra01_register_t)(value->seg2) << 16) & 0xf0000)
        | (((ra01_register_t)(value->seg3) << 24) & 0xf000000)
        | (((ra01_register_t)(value->en) << 31) & 0x80000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 17-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->seg0 = (uint8_t)(reg & 0xf);
       value->seg1 = (uint8_t)((reg & 0xf00) >> 8);
       value->seg2 = (uint8_t)((reg & 0xf0000) >> 16);
       value->seg3 = (uint8_t)((reg & 0xf000000) >> 24);
       value->en = (uint8_t)((reg & 0x80000000) >> 31);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01GetRsMcsTopRsMcsTopSlaveIntSum( RmsDev_t * rmsDev_p, 
     Ra01RsMcsTopRsMcsTopSlaveIntSum_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000024);


        // We are reading 8-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rsMcsBbeSlaveInterrupt = (uint8_t)(reg & 0x3);
       value->rsMcsCpmRxSlaveInterrupt = (uint8_t)((reg & 0x4) >> 2);
       value->rsMcsCpmTxSlaveInterrupt = (uint8_t)((reg & 0x8) >> 3);
       value->rsMcsPabSlaveInterrupt = (uint8_t)((reg & 0x30) >> 4);
       value->rsMcsPexSlaveInterrupt = (uint8_t)((reg & 0xc0) >> 6);

    return ret;
}






 int Ra01AccRsMcsTopRsMcsTopSlaveIntSumEnb( RmsDev_t * rmsDev_p, 
     Ra01RsMcsTopRsMcsTopSlaveIntSumEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000028);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rsMcsBbeSlaveInterruptEnb) & 0x3)
        | (((ra01_register_t)(value->rsMcsCpmRxSlaveInterruptEnb) << 2) & 0x4)
        | (((ra01_register_t)(value->rsMcsCpmTxSlaveInterruptEnb) << 3) & 0x8)
        | (((ra01_register_t)(value->rsMcsPabSlaveInterruptEnb) << 4) & 0x30)
        | (((ra01_register_t)(value->rsMcsPexSlaveInterruptEnb) << 6) & 0xc0)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 8-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rsMcsBbeSlaveInterruptEnb = (uint8_t)(reg & 0x3);
       value->rsMcsCpmRxSlaveInterruptEnb = (uint8_t)((reg & 0x4) >> 2);
       value->rsMcsCpmTxSlaveInterruptEnb = (uint8_t)((reg & 0x8) >> 3);
       value->rsMcsPabSlaveInterruptEnb = (uint8_t)((reg & 0x30) >> 4);
       value->rsMcsPexSlaveInterruptEnb = (uint8_t)((reg & 0xc0) >> 6);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsTopRsMcsTopSlaveIntSumRaw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsTopRsMcsTopSlaveIntSumRaw_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x0000002c);


        // We are reading 8-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rsMcsBbeSlaveInterrupt = (uint8_t)(reg & 0x3);
       value->rsMcsCpmRxSlaveInterrupt = (uint8_t)((reg & 0x4) >> 2);
       value->rsMcsCpmTxSlaveInterrupt = (uint8_t)((reg & 0x8) >> 3);
       value->rsMcsPabSlaveInterrupt = (uint8_t)((reg & 0x30) >> 4);
       value->rsMcsPexSlaveInterrupt = (uint8_t)((reg & 0xc0) >> 6);

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsTopRsMcsTopSlaveIntSumIntrRw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsTopRsMcsTopSlaveIntSumIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000030);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rsMcsBbeSlaveInterrupt) & 0x3)
        | (((ra01_register_t)(value->rsMcsCpmRxSlaveInterrupt) << 2) & 0x4)
        | (((ra01_register_t)(value->rsMcsCpmTxSlaveInterrupt) << 3) & 0x8)
        | (((ra01_register_t)(value->rsMcsPabSlaveInterrupt) << 4) & 0x30)
        | (((ra01_register_t)(value->rsMcsPexSlaveInterrupt) << 6) & 0xc0)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 8-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rsMcsBbeSlaveInterrupt = (uint8_t)(reg & 0x3);
       value->rsMcsCpmRxSlaveInterrupt = (uint8_t)((reg & 0x4) >> 2);
       value->rsMcsCpmTxSlaveInterrupt = (uint8_t)((reg & 0x8) >> 3);
       value->rsMcsPabSlaveInterrupt = (uint8_t)((reg & 0x30) >> 4);
       value->rsMcsPexSlaveInterrupt = (uint8_t)((reg & 0xc0) >> 6);
    }

    return ret;
}


#endif // MCS_API_DEBUG

 


 int Ra01AccRsMcsGaeFipsReset( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsReset_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000038);
     offsets[1] = ((uintAddrBus_t)0x000000d8);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->fipsReset) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->fipsReset = (uint8_t)(reg & 0x1);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsMode( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsMode_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x0000003c);
     offsets[1] = ((uintAddrBus_t)0x000000dc);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ctrMode) & 0x1)
        | (((ra01_register_t)(value->keylen) << 1) & 0x2)
        | (((ra01_register_t)(value->channel) << 2) & 0xc)
        | (((ra01_register_t)(value->ecb) << 4) & 0x10)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->ctrMode = (uint8_t)(reg & 0x1);
       value->keylen = (uint8_t)((reg & 0x2) >> 1);
       value->channel = (uint8_t)((reg & 0xc) >> 2);
       value->ecb = (uint8_t)((reg & 0x10) >> 4);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsCtl( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsCtl_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000040);
     offsets[1] = ((uintAddrBus_t)0x000000e0);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->nextIcv) & 0x1)
        | (((ra01_register_t)(value->lenRound) << 1) & 0x2)
        | (((ra01_register_t)(value->blockIsAad) << 2) & 0x4)
        | (((ra01_register_t)(value->eop) << 3) & 0x8)
        | (((ra01_register_t)(value->blockSize) << 4) & 0xf0)
        | (((ra01_register_t)(value->vld) << 8) & 0x100)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 9-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->nextIcv = (uint8_t)(reg & 0x1);
       value->lenRound = (uint8_t)((reg & 0x2) >> 1);
       value->blockIsAad = (uint8_t)((reg & 0x4) >> 2);
       value->eop = (uint8_t)((reg & 0x8) >> 3);
       value->blockSize = (uint8_t)((reg & 0xf0) >> 4);
       value->vld = (uint8_t)((reg & 0x100) >> 8);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsIvBits9564( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsIvBits9564_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000044);
     offsets[1] = ((uintAddrBus_t)0x000000e4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->data) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->data = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsIvBits630( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsIvBits630_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000048);
     offsets[1] = ((uintAddrBus_t)0x000000e8);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dataLsb) & 0xffffffff)
        | (((ra01_register_t)(value->dataMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsCtr( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsCtr_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000050);
     offsets[1] = ((uintAddrBus_t)0x000000f0);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->fipsCtr) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->fipsCtr = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsSakBits255192( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsSakBits255192_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000058);
     offsets[1] = ((uintAddrBus_t)0x000000f8);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dataLsb) & 0xffffffff)
        | (((ra01_register_t)(value->dataMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsSakBits191128( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsSakBits191128_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000060);
     offsets[1] = ((uintAddrBus_t)0x00000100);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dataLsb) & 0xffffffff)
        | (((ra01_register_t)(value->dataMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsSakBits12764( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsSakBits12764_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000068);
     offsets[1] = ((uintAddrBus_t)0x00000108);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dataLsb) & 0xffffffff)
        | (((ra01_register_t)(value->dataMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsSakBits630( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsSakBits630_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000070);
     offsets[1] = ((uintAddrBus_t)0x00000110);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dataLsb) & 0xffffffff)
        | (((ra01_register_t)(value->dataMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsHashkeyBits12764( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsHashkeyBits12764_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000078);
     offsets[1] = ((uintAddrBus_t)0x00000118);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dataLsb) & 0xffffffff)
        | (((ra01_register_t)(value->dataMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsHashkeyBits630( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsHashkeyBits630_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000080);
     offsets[1] = ((uintAddrBus_t)0x00000120);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dataLsb) & 0xffffffff)
        | (((ra01_register_t)(value->dataMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsBlockBits12764( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsBlockBits12764_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000088);
     offsets[1] = ((uintAddrBus_t)0x00000128);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dataLsb) & 0xffffffff)
        | (((ra01_register_t)(value->dataMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsBlockBits630( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsBlockBits630_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000090);
     offsets[1] = ((uintAddrBus_t)0x00000130);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dataLsb) & 0xffffffff)
        | (((ra01_register_t)(value->dataMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsGaeFipsStart( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsStart_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000098);
     offsets[1] = ((uintAddrBus_t)0x00000138);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->fipsStart) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->fipsStart = (uint8_t)(reg & 0x1);
    }

    return ret;
}






 int Ra01GetRsMcsGaeFipsResultBlockBits12764( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsResultBlockBits12764_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000000a0);
     offsets[1] = ((uintAddrBus_t)0x00000140);
 
   offset = offsets[instance];

         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);

    return ret;
}






 int Ra01GetRsMcsGaeFipsResultBlockBits630( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsResultBlockBits630_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000000a8);
     offsets[1] = ((uintAddrBus_t)0x00000148);
 
   offset = offsets[instance];

         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);

    return ret;
}






 int Ra01GetRsMcsGaeFipsResultPass( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsResultPass_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000000b0);
     offsets[1] = ((uintAddrBus_t)0x00000150);
 
   offset = offsets[instance];

        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->fipsResultPass = (uint8_t)(reg & 0x1);

    return ret;
}






 int Ra01GetRsMcsGaeFipsResultIcvBits12764( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsResultIcvBits12764_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000000b8);
     offsets[1] = ((uintAddrBus_t)0x00000158);
 
   offset = offsets[instance];

         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);

    return ret;
}






 int Ra01GetRsMcsGaeFipsResultIcvBits630( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeFipsResultIcvBits630_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000000c0);
     offsets[1] = ((uintAddrBus_t)0x00000160);
 
   offset = offsets[instance];

         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->dataLsb = (uint32_t)(reg & 0xffffffff);
       value->dataMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsGaeDbgMuxSel( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeDbgMuxSel_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000000c8);
     offsets[1] = ((uintAddrBus_t)0x00000168);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->seg0) & 0x1f)
        | (((ra01_register_t)(value->seg1) << 8) & 0x1f00)
        | (((ra01_register_t)(value->seg2) << 16) & 0x1f0000)
        | (((ra01_register_t)(value->seg3) << 24) & 0x1f000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 20-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->seg0 = (uint8_t)(reg & 0x1f);
       value->seg1 = (uint8_t)((reg & 0x1f00) >> 8);
       value->seg2 = (uint8_t)((reg & 0x1f0000) >> 16);
       value->seg3 = (uint8_t)((reg & 0x1f000000) >> 24);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsGaeDebugStatus( RmsDev_t * rmsDev_p, 
     Ra01RsMcsGaeInstance_t instance, 
     Ra01RsMcsGaeDebugStatus_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000000d0);
     offsets[1] = ((uintAddrBus_t)0x00000170);
 
   offset = offsets[instance];

         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);

    return ret;
}


#endif // MCS_API_DEBUG

  


 int Ra01AccRsMcsBbeBbeInt( RmsDev_t * rmsDev_p, 
     Ra01RsMcsBbeInstance_t instance, 
     Ra01RsMcsBbeBbeInt_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000178);
     offsets[1] = ((uintAddrBus_t)0x000001a0);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dfifoOverflow) & 0x1)
        | (((ra01_register_t)(value->plfifoOverflow) << 1) & 0x2)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->dfifoOverflow = (uint8_t)(reg & 0x1);
       value->plfifoOverflow = (uint8_t)((reg & 0x2) >> 1);
    }

    return ret;
}






 int Ra01AccRsMcsBbeBbeIntEnb( RmsDev_t * rmsDev_p, 
     Ra01RsMcsBbeInstance_t instance, 
     Ra01RsMcsBbeBbeIntEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x0000017c);
     offsets[1] = ((uintAddrBus_t)0x000001a4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dfifoOverflowEnb) & 0x1)
        | (((ra01_register_t)(value->plfifoOverflowEnb) << 1) & 0x2)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->dfifoOverflowEnb = (uint8_t)(reg & 0x1);
       value->plfifoOverflowEnb = (uint8_t)((reg & 0x2) >> 1);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsBbeBbeIntRaw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsBbeInstance_t instance, 
     Ra01RsMcsBbeBbeIntRaw_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000180);
     offsets[1] = ((uintAddrBus_t)0x000001a8);
 
   offset = offsets[instance];

        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->dfifoOverflow = (uint8_t)(reg & 0x1);
       value->plfifoOverflow = (uint8_t)((reg & 0x2) >> 1);

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsBbeBbeIntIntrRw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsBbeInstance_t instance, 
     Ra01RsMcsBbeBbeIntIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000184);
     offsets[1] = ((uintAddrBus_t)0x000001ac);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->dfifoOverflow) & 0x1)
        | (((ra01_register_t)(value->plfifoOverflow) << 1) & 0x2)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->dfifoOverflow = (uint8_t)(reg & 0x1);
       value->plfifoOverflow = (uint8_t)((reg & 0x2) >> 1);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsBbeHwInit( RmsDev_t * rmsDev_p, 
     Ra01RsMcsBbeInstance_t instance, 
     Ra01RsMcsBbeHwInit_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000188);
     offsets[1] = ((uintAddrBus_t)0x000001b0);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->trigger) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->trigger = (uint8_t)(reg & 0x1);
    }

    return ret;
}






 int Ra01AccRsMcsBbePaddingCtl( RmsDev_t * rmsDev_p, 
     Ra01RsMcsBbeInstance_t instance, 
     Ra01RsMcsBbePaddingCtl_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x0000018c);
     offsets[1] = ((uintAddrBus_t)0x000001b4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->minPktSize) & 0xf)
        | (((ra01_register_t)(value->paddingEn) << 4) & 0x10)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->minPktSize = (uint8_t)(reg & 0xf);
       value->paddingEn = (uint8_t)((reg & 0x10) >> 4);
    }

    return ret;
}






 int Ra01GetRsMcsBbePreemptFilterCreditCorrCount( RmsDev_t * rmsDev_p, 
     Ra01RsMcsBbeInstance_t instance, 
     unsigned index,
     Ra01RsMcsBbePreemptFilterCreditCorrCount_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000190) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x000001b8) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->creditCorrCount = (uint8_t)(reg & 0x1f);

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsBbeDbgMuxSel( RmsDev_t * rmsDev_p, 
     Ra01RsMcsBbeInstance_t instance, 
     Ra01RsMcsBbeDbgMuxSel_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000194);
     offsets[1] = ((uintAddrBus_t)0x000001bc);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->seg0) & 0x7f)
        | (((ra01_register_t)(value->seg1) << 8) & 0x7f00)
        | (((ra01_register_t)(value->seg2) << 16) & 0x7f0000)
        | (((ra01_register_t)(value->seg3) << 24) & 0x7f000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 28-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->seg0 = (uint8_t)(reg & 0x7f);
       value->seg1 = (uint8_t)((reg & 0x7f00) >> 8);
       value->seg2 = (uint8_t)((reg & 0x7f0000) >> 16);
       value->seg3 = (uint8_t)((reg & 0x7f000000) >> 24);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsBbeDebugStatus( RmsDev_t * rmsDev_p, 
     Ra01RsMcsBbeInstance_t instance, 
     Ra01RsMcsBbeDebugStatus_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000198);
     offsets[1] = ((uintAddrBus_t)0x000001c0);
 
   offset = offsets[instance];

         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);

    return ret;
}


#endif // MCS_API_DEBUG

  


 int Ra01AccRsMcsPabPabInt( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     Ra01RsMcsPabPabInt_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001c8);
     offsets[1] = ((uintAddrBus_t)0x00000200);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->overflow) & 0x3)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->overflow = (uint8_t)(reg & 0x3);
    }

    return ret;
}






 int Ra01AccRsMcsPabPabIntEnb( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     Ra01RsMcsPabPabIntEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001cc);
     offsets[1] = ((uintAddrBus_t)0x00000204);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->overflowEnb) & 0x3)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->overflowEnb = (uint8_t)(reg & 0x3);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsPabPabIntRaw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     Ra01RsMcsPabPabIntRaw_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001d0);
     offsets[1] = ((uintAddrBus_t)0x00000208);
 
   offset = offsets[instance];

        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->overflow = (uint8_t)(reg & 0x3);

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsPabPabIntIntrRw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     Ra01RsMcsPabPabIntIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001d4);
     offsets[1] = ((uintAddrBus_t)0x0000020c);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->overflow) & 0x3)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->overflow = (uint8_t)(reg & 0x3);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsPabHwInit( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     Ra01RsMcsPabHwInit_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001d8);
     offsets[1] = ((uintAddrBus_t)0x00000210);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->trigger) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->trigger = (uint8_t)(reg & 0x1);
    }

    return ret;
}






 int Ra01AccRsMcsPabDelayCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     unsigned index,
     Ra01RsMcsPabDelayCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001dc) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x00000214) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->fixedLatency) & 0x3ffff)
        | (((ra01_register_t)(value->enablePreemptFixedLatency) << 28) & 0x10000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 19-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->fixedLatency = (uint32_t)(reg & 0x3ffff);
       value->enablePreemptFixedLatency = (uint8_t)((reg & 0x10000000) >> 28);
    }

    return ret;
}






 int Ra01AccRsMcsPabFifoCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     unsigned index,
     Ra01RsMcsPabFifoCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001e0) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x00000218) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->aeThExp) & 0x3f)
        | (((ra01_register_t)(value->aeThPre) << 16) & 0x3f0000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 12-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->aeThExp = (uint8_t)(reg & 0x3f);
       value->aeThPre = (uint8_t)((reg & 0x3f0000) >> 16);
    }

    return ret;
}






 int Ra01AccRsMcsPabIncludeTagCnt( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     Ra01RsMcsPabIncludeTagCnt_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001e4);
     offsets[1] = ((uintAddrBus_t)0x0000021c);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->includeTagCnt) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->includeTagCnt = (uint8_t)(reg & 0x1);
    }

    return ret;
}






 int Ra01AccRsMcsPabTxTrunc( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     Ra01RsMcsPabTxTrunc_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001e8);
     offsets[1] = ((uintAddrBus_t)0x00000220);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->enableTrunc) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->enableTrunc = (uint8_t)(reg & 0x1);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsPabDbgMuxSel( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     Ra01RsMcsPabDbgMuxSel_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001ec);
     offsets[1] = ((uintAddrBus_t)0x00000224);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->seg0) & 0x3f)
        | (((ra01_register_t)(value->seg1) << 8) & 0x3f00)
        | (((ra01_register_t)(value->seg2) << 16) & 0x3f0000)
        | (((ra01_register_t)(value->seg3) << 24) & 0x3f000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 24-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->seg0 = (uint8_t)(reg & 0x3f);
       value->seg1 = (uint8_t)((reg & 0x3f00) >> 8);
       value->seg2 = (uint8_t)((reg & 0x3f0000) >> 16);
       value->seg3 = (uint8_t)((reg & 0x3f000000) >> 24);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsPabDebugStatus( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     Ra01RsMcsPabDebugStatus_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001f0);
     offsets[1] = ((uintAddrBus_t)0x00000228);
 
   offset = offsets[instance];

         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsPabFifoSkidCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPabInstance_t instance, 
     unsigned index,
     Ra01RsMcsPabFifoSkidCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000001f8) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x00000230) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->bbePabSkidExp) & 0x3f)
        | (((ra01_register_t)(value->bbePabSkidPre) << 16) & 0x3f0000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 12-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->bbePabSkidExp = (uint8_t)(reg & 0x3f);
       value->bbePabSkidPre = (uint8_t)((reg & 0x3f0000) >> 16);
    }

    return ret;
}



  


 int Ra01AccRsMcsPexPexInt( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexPexInt_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000238);
     offsets[1] = ((uintAddrBus_t)0x00000428);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->earlyPreemptErr) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->earlyPreemptErr = (uint8_t)(reg & 0x1);
    }

    return ret;
}






 int Ra01AccRsMcsPexPexIntEnb( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexPexIntEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x0000023c);
     offsets[1] = ((uintAddrBus_t)0x0000042c);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->earlyPreemptErrEnb) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->earlyPreemptErrEnb = (uint8_t)(reg & 0x1);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsPexPexIntRaw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexPexIntRaw_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000240);
     offsets[1] = ((uintAddrBus_t)0x00000430);
 
   offset = offsets[instance];

        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->earlyPreemptErr = (uint8_t)(reg & 0x1);

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsPexPexIntIntrRw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexPexIntIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000244);
     offsets[1] = ((uintAddrBus_t)0x00000434);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->earlyPreemptErr) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->earlyPreemptErr = (uint8_t)(reg & 0x1);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsPexPexConfiguration( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexPexConfiguration_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000248);
     offsets[1] = ((uintAddrBus_t)0x00000438);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->vlanAfterCustom) & 0x1)
        | (((ra01_register_t)(value->nonDixErr) << 1) & 0x2)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->vlanAfterCustom = (uint8_t)(reg & 0x1);
       value->nonDixErr = (uint8_t)((reg & 0x2) >> 1);
    }

    return ret;
}






 int Ra01AccRsMcsPexVlanCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexVlanCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 8) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x0000024c) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x0000043c) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->vlanEn) & 0x1)
        | (((ra01_register_t)(value->vlanEtype) << 1) & 0x1fffe)
        | (((ra01_register_t)(value->vlanIndx) << 17) & 0xe0000)
        | (((ra01_register_t)(value->vlanSize) << 20) & 0xf00000)
        | (((ra01_register_t)(value->isVlan) << 24) & 0x1000000)
        | (((ra01_register_t)(value->vlanBonus) << 25) & 0x2000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 26-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->vlanEn = (uint8_t)(reg & 0x1);
       value->vlanEtype = (uint16_t)((reg & 0x1fffe) >> 1);
       value->vlanIndx = (uint8_t)((reg & 0xe0000) >> 17);
       value->vlanSize = (uint8_t)((reg & 0xf00000) >> 20);
       value->isVlan = (uint8_t)((reg & 0x1000000) >> 24);
       value->vlanBonus = (uint8_t)((reg & 0x2000000) >> 25);
    }

    return ret;
}


































 int Ra01AccRsMcsPexPortConfig( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexPortConfig_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x0000026c) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x0000045c) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->preamble) & 0x3)
        | (((ra01_register_t)(value->vlanTagRelModeSel) << 2) & 0x1c)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->preamble = (uint8_t)(reg & 0x3);
       value->vlanTagRelModeSel = (uint8_t)((reg & 0x1c) >> 2);
    }

    return ret;
}






 int Ra01AccRsMcsPexMplsCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexMplsCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000270) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x00000460) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->mplsEn) & 0x1)
        | (((ra01_register_t)(value->mplsEtype) << 1) & 0x1fffe)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 17-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->mplsEn = (uint8_t)(reg & 0x1);
       value->mplsEtype = (uint16_t)((reg & 0x1fffe) >> 1);
    }

    return ret;
}


















 int Ra01AccRsMcsPexSectagCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexSectagCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000280) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x00000470) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sectagEn) & 0x1)
        | (((ra01_register_t)(value->sectagEtype) << 1) & 0x1fffe)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 17-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sectagEn = (uint8_t)(reg & 0x1);
       value->sectagEtype = (uint16_t)((reg & 0x1fffe) >> 1);
    }

    return ret;
}






 int Ra01AccRsMcsPexMcsHeaderCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexMcsHeaderCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000288);
     offsets[1] = ((uintAddrBus_t)0x00000478);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rxMcsHdrEn) & 0x1)
        | (((ra01_register_t)(value->rxMcsHeader) << 1) & 0x1fffe)
        | (((ra01_register_t)(value->txMcsHdrEn) << 17) & 0x20000)
        | (((ra01_register_t)(value->txMcsHeader) << 18) & 0x3fffc0000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->rxMcsHdrEn = (uint8_t)(reg & 0x1);
       value->rxMcsHeader = (uint16_t)((reg & 0x1fffe) >> 1);
       value->txMcsHdrEn = (uint8_t)((reg & 0x20000) >> 17);
       value->txMcsHeader = (uint16_t)((reg & 0x3fffc0000) >> 18);
    }

    return ret;
}






 int Ra01AccRsMcsPexIpv4Cfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexIpv4Cfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000290);
     offsets[1] = ((uintAddrBus_t)0x00000480);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ipv4En) & 0x1)
        | (((ra01_register_t)(value->ipv4Etype) << 1) & 0x1fffe)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 17-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->ipv4En = (uint8_t)(reg & 0x1);
       value->ipv4Etype = (uint16_t)((reg & 0x1fffe) >> 1);
    }

    return ret;
}






 int Ra01AccRsMcsPexIpv6Cfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexIpv6Cfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000294);
     offsets[1] = ((uintAddrBus_t)0x00000484);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ipv6En) & 0x1)
        | (((ra01_register_t)(value->ipv6Etype) << 1) & 0x1fffe)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 17-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->ipv6En = (uint8_t)(reg & 0x1);
       value->ipv6Etype = (uint16_t)((reg & 0x1fffe) >> 1);
    }

    return ret;
}






 int Ra01AccRsMcsPexUdpCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexUdpCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000298);
     offsets[1] = ((uintAddrBus_t)0x00000488);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->udpEn) & 0x1)
        | (((ra01_register_t)(value->udpProtNum) << 1) & 0x1fe)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 9-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->udpEn = (uint8_t)(reg & 0x1);
       value->udpProtNum = (uint8_t)((reg & 0x1fe) >> 1);
    }

    return ret;
}






 int Ra01AccRsMcsPexPtpCfg( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexPtpCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 2) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x0000029c) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x0000048c) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ptpEn) & 0x1)
        | (((ra01_register_t)(value->ptpPort) << 1) & 0x1fffe)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 17-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->ptpEn = (uint8_t)(reg & 0x1);
       value->ptpPort = (uint16_t)((reg & 0x1fffe) >> 1);
    }

    return ret;
}










 int Ra01AccRsMcsPexCtlPktRuleEtype( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexCtlPktRuleEtype_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 8) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000002a4) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x00000494) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ruleEtype) & 0xffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 16-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->ruleEtype = (uint16_t)(reg & 0xffff);
    }

    return ret;
}


































 int Ra01AccRsMcsPexCtlPktRuleDa( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexCtlPktRuleDa_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 8) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x000002c8) + ((uintAddrBus_t)index*8);
     offsets[1] = ((uintAddrBus_t)0x000004b8) + ((uintAddrBus_t)index*8);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ruleDaLsb) & 0xffffffff)
        | (((ra01_register_t)(value->ruleDaMsb) << 32) & 0xffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->ruleDaLsb = (uint32_t)(reg & 0xffffffff);
       value->ruleDaMsb = (uint16_t)((reg & 0xffff00000000) >> 32);
    }

    return ret;
}


































 int Ra01AccRsMcsPexCtlPktRuleDaRangeMin( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexCtlPktRuleDaRangeMin_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000308) + ((uintAddrBus_t)index*16);
     offsets[1] = ((uintAddrBus_t)0x000004f8) + ((uintAddrBus_t)index*16);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ruleRangeMinLsb) & 0xffffffff)
        | (((ra01_register_t)(value->ruleRangeMinMsb) << 32) & 0xffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->ruleRangeMinLsb = (uint32_t)(reg & 0xffffffff);
       value->ruleRangeMinMsb = (uint16_t)((reg & 0xffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsPexCtlPktRuleDaRangeMax( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexCtlPktRuleDaRangeMax_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000310) + ((uintAddrBus_t)index*16);
     offsets[1] = ((uintAddrBus_t)0x00000500) + ((uintAddrBus_t)index*16);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ruleRangeMaxLsb) & 0xffffffff)
        | (((ra01_register_t)(value->ruleRangeMaxMsb) << 32) & 0xffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->ruleRangeMaxLsb = (uint32_t)(reg & 0xffffffff);
       value->ruleRangeMaxMsb = (uint16_t)((reg & 0xffff00000000) >> 32);
    }

    return ret;
}






























 int Ra01AccRsMcsPexCtlPktRuleComboMin( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexCtlPktRuleComboMin_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000378) + ((uintAddrBus_t)index*32);
     offsets[1] = ((uintAddrBus_t)0x00000568) + ((uintAddrBus_t)index*32);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ruleComboMinLsb) & 0xffffffff)
        | (((ra01_register_t)(value->ruleComboMinMsb) << 32) & 0xffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->ruleComboMinLsb = (uint32_t)(reg & 0xffffffff);
       value->ruleComboMinMsb = (uint16_t)((reg & 0xffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsPexCtlPktRuleComboMax( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexCtlPktRuleComboMax_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000380) + ((uintAddrBus_t)index*32);
     offsets[1] = ((uintAddrBus_t)0x00000570) + ((uintAddrBus_t)index*32);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ruleComboMaxLsb) & 0xffffffff)
        | (((ra01_register_t)(value->ruleComboMaxMsb) << 32) & 0xffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->ruleComboMaxLsb = (uint32_t)(reg & 0xffffffff);
       value->ruleComboMaxMsb = (uint16_t)((reg & 0xffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsPexCtlPktRuleComboEt( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexCtlPktRuleComboEt_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000388) + ((uintAddrBus_t)index*32);
     offsets[1] = ((uintAddrBus_t)0x00000578) + ((uintAddrBus_t)index*32);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ruleComboEt) & 0xffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 16-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->ruleComboEt = (uint16_t)(reg & 0xffff);
    }

    return ret;
}










































 int Ra01AccRsMcsPexCtlPktRuleDaPrefix( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexCtlPktRuleDaPrefix_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000408);
     offsets[1] = ((uintAddrBus_t)0x000005f8);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ruleDaPrefixLsb) & 0xffffffff)
        | (((ra01_register_t)(value->ruleDaPrefixMsb) << 32) & 0xfff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->ruleDaPrefixLsb = (uint32_t)(reg & 0xffffffff);
       value->ruleDaPrefixMsb = (uint16_t)((reg & 0xfff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsPexCtlPktRuleEnable( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexCtlPktRuleEnable_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000410) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x00000600) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->ctlPktRuleEtypeEn) & 0xff)
        | (((ra01_register_t)(value->ctlPktRuleDaEn) << 8) & 0xff00)
        | (((ra01_register_t)(value->ctlPktRuleDaRangeEn) << 16) & 0xf0000)
        | (((ra01_register_t)(value->ctlPktRuleComboEn) << 20) & 0xf00000)
        | (((ra01_register_t)(value->ctlPktRuleMacEn) << 24) & 0x1000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 25-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->ctlPktRuleEtypeEn = (uint8_t)(reg & 0xff);
       value->ctlPktRuleDaEn = (uint8_t)((reg & 0xff00) >> 8);
       value->ctlPktRuleDaRangeEn = (uint8_t)((reg & 0xf0000) >> 16);
       value->ctlPktRuleComboEn = (uint8_t)((reg & 0xf00000) >> 20);
       value->ctlPktRuleMacEn = (uint8_t)((reg & 0x1000000) >> 24);
    }

    return ret;
}






 int Ra01AccRsMcsPexEarlyPreemptFilterCtrl( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     unsigned index,
     Ra01RsMcsPexEarlyPreemptFilterCtrl_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000414) + ((uintAddrBus_t)index*4);
     offsets[1] = ((uintAddrBus_t)0x00000604) + ((uintAddrBus_t)index*4);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->earlyPreemptFilterEnable) & 0x1)
        | (((ra01_register_t)(value->earlyPreemptMinNumWords) << 1) & 0x1e)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->earlyPreemptFilterEnable = (uint8_t)(reg & 0x1);
       value->earlyPreemptMinNumWords = (uint8_t)((reg & 0x1e) >> 1);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsPexDbgMuxSel( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexDbgMuxSel_t * value,
     ra01_reg_access_op_t op) 
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000418);
     offsets[1] = ((uintAddrBus_t)0x00000608);
 
   offset = offsets[instance];

    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->seg0) & 0x7f)
        | (((ra01_register_t)(value->seg1) << 8) & 0x7f00)
        | (((ra01_register_t)(value->seg2) << 16) & 0x7f0000)
        | (((ra01_register_t)(value->seg3) << 24) & 0x7f000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 28-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->seg0 = (uint8_t)(reg & 0x7f);
       value->seg1 = (uint8_t)((reg & 0x7f00) >> 8);
       value->seg2 = (uint8_t)((reg & 0x7f0000) >> 16);
       value->seg3 = (uint8_t)((reg & 0x7f000000) >> 24);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsPexDebugStatus( RmsDev_t * rmsDev_p, 
     Ra01RsMcsPexInstance_t instance, 
     Ra01RsMcsPexDebugStatus_t * value)
{
    int                             ret = 0;
    ra01_register_t                 reg = 0;



   uintAddrBus_t offsets[2];
   uintAddrBus_t offset;

    offsets[0] = ((uintAddrBus_t)0x00000420);
     offsets[1] = ((uintAddrBus_t)0x00000610);
 
   offset = offsets[instance];

         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);

    return ret;
}


#endif // MCS_API_DEBUG

  
#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsCseRxReserved0( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCseRxReserved0_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000618) + ((uintAddrBus_t)index*8);


        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->reserved = (uint8_t)(reg & 0x1);

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCseRxCtrl( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCseRxCtrl_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000634);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->cpuCseClrOnRd) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->cpuCseClrOnRd = (uint8_t)(reg & 0x1);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCseRxDbgMuxSel( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCseRxDbgMuxSel_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000638);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->seg0) & 0xffff)
        | (((ra01_register_t)(value->seg1) << 16) & 0xffff0000)
        | (((ra01_register_t)(value->seg2) << 32) & 0xffff00000000)
        | (((ra01_register_t)(value->seg3) << 48) & 0xffff000000000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsCseRxDebugStatus( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCseRxDebugStatus_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000640);


         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCseRxStatsClear( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCseRxStatsClear_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000648);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rxSecyGo) & 0x1)
        | (((ra01_register_t)(value->rxScGo) << 1) & 0x2)
        | (((ra01_register_t)(value->rxPortGo) << 2) & 0x4)
        | (((ra01_register_t)(value->rxFlowidGo) << 3) & 0x8)
        | (((ra01_register_t)(value->rxSaGo) << 4) & 0x10)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rxSecyGo = (uint8_t)(reg & 0x1);
       value->rxScGo = (uint8_t)((reg & 0x2) >> 1);
       value->rxPortGo = (uint8_t)((reg & 0x4) >> 2);
       value->rxFlowidGo = (uint8_t)((reg & 0x8) >> 3);
       value->rxSaGo = (uint8_t)((reg & 0x10) >> 4);
    }

    return ret;
}



 
#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsCseTxReserved0( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCseTxReserved0_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000650) + ((uintAddrBus_t)index*8);


        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->reserved = (uint8_t)(reg & 0x1);

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCseTxCtrl( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCseTxCtrl_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x0000066c);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->cpuCseClrOnRd) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->cpuCseClrOnRd = (uint8_t)(reg & 0x1);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCseTxDbgMuxSel( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCseTxDbgMuxSel_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000670);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->seg0) & 0xffff)
        | (((ra01_register_t)(value->seg1) << 16) & 0xffff0000)
        | (((ra01_register_t)(value->seg2) << 32) & 0xffff00000000)
        | (((ra01_register_t)(value->seg3) << 48) & 0xffff000000000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsCseTxDebugStatus( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCseTxDebugStatus_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000678);


         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCseTxStatsClear( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCseTxStatsClear_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00000680);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->txSecyGo) & 0x1)
        | (((ra01_register_t)(value->txScGo) << 1) & 0x2)
        | (((ra01_register_t)(value->txPortGo) << 2) & 0x4)
        | (((ra01_register_t)(value->txFlowidGo) << 3) & 0x8)
        | (((ra01_register_t)(value->txSaGo) << 4) & 0x10)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->txSecyGo = (uint8_t)(reg & 0x1);
       value->txScGo = (uint8_t)((reg & 0x2) >> 1);
       value->txPortGo = (uint8_t)((reg & 0x4) >> 2);
       value->txFlowidGo = (uint8_t)((reg & 0x8) >> 3);
       value->txSaGo = (uint8_t)((reg & 0x10) >> 4);
    }

    return ret;
}



 static void ra01_rs_mcs_cse_rx_mem_ifinunctloctets_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemIfinunctloctets_t * p )
{
    p->rxUnctlOctetCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemIfinunctloctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinunctloctets_t * data )
{
    int                                       ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_ifinunctloctets_t  raw;
    ra01_register_t *                         reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLOCTETS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000688,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_ifinunctloctets_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_ifinctloctets_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemIfinctloctets_t * p )
{
    p->rxCtlOctetCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemIfinctloctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinctloctets_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_ifinctloctets_t raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_IFINCTLOCTETS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000788,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_ifinctloctets_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_ifinunctlucpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemIfinunctlucpkts_t * p )
{
    p->rxUnctlPktUcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemIfinunctlucpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinunctlucpkts_t * data )
{
    int                                       ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_ifinunctlucpkts_t  raw;
    ra01_register_t *                         reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLUCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000888,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_ifinunctlucpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_ifinunctlmcpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t * p )
{
    p->rxUnctlPktMcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemIfinunctlmcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t * data )
{
    int                                       ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_ifinunctlmcpkts_t  raw;
    ra01_register_t *                         reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLMCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000988,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_ifinunctlmcpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_ifinunctlbcpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t * p )
{
    p->rxUnctlPktBcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemIfinunctlbcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t * data )
{
    int                                       ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_ifinunctlbcpkts_t  raw;
    ra01_register_t *                         reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_IFINUNCTLBCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000a88,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_ifinunctlbcpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_ifinctlucpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemIfinctlucpkts_t * p )
{
    p->rxCtlPktUcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemIfinctlucpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinctlucpkts_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_ifinctlucpkts_t raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_IFINCTLUCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000b88,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_ifinctlucpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_ifinctlmcpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemIfinctlmcpkts_t * p )
{
    p->rxCtlPktMcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemIfinctlmcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinctlmcpkts_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_ifinctlmcpkts_t raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_IFINCTLMCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000c88,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_ifinctlmcpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_ifinctlbcpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemIfinctlbcpkts_t * p )
{
    p->rxCtlPktBcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemIfinctlbcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemIfinctlbcpkts_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_ifinctlbcpkts_t raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_IFINCTLBCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000d88,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_ifinctlbcpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssecyuntaggedornotag_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t * p )
{
    p->rxSecyPktUntaggedCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssecyuntaggedornotag( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t * data )
{
    int                                                 ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssecyuntaggedornotag_t  raw;
    ra01_register_t *                                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYUNTAGGEDORNOTAG_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000e88,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssecyuntaggedornotag_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssecybadtag_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssecybadtag_t * p )
{
    p->rxSecyPktBadtagCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssecybadtag( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecybadtag_t * data )
{
    int                                        ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssecybadtag_t  raw;
    ra01_register_t *                          reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYBADTAG_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00000f88,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssecybadtag_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssecyctl_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssecyctl_t * p )
{
    p->rxSecyPktCtlCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssecyctl( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecyctl_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssecyctl_t raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYCTL_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001088,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssecyctl_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssecytaggedctl_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t * p )
{
    p->rxSecyPktTaggedCtlCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssecytaggedctl( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t * data )
{
    int                                           ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssecytaggedctl_t  raw;
    ra01_register_t *                             reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYTAGGEDCTL_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001188,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssecytaggedctl_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssecyunknownsci_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t * p )
{
    p->rxSecyPktNosaCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssecyunknownsci( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t * data )
{
    int                                            ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssecyunknownsci_t  raw;
    ra01_register_t *                              reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYUNKNOWNSCI_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001288,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssecyunknownsci_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssecynosci_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssecynosci_t * p )
{
    p->rxSecyPktNosaerrorCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssecynosci( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssecynosci_t * data )
{
    int                                       ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssecynosci_t  raw;
    ra01_register_t *                         reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSECYNOSCI_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001388,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssecynosci_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktsctrlportdisabled_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t * p )
{
    p->rxSecyPktCtrlPortDisabledCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktsctrlportdisabled( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t * data )
{
    int                                              ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktsctrlportdisabled_t  raw;
    ra01_register_t *                                reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSCTRLPORTDISABLED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001488,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktsctrlportdisabled_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inoctetsscvalidate_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInoctetsscvalidate_t * p )
{
    p->rxSecyOctetValidatedCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInoctetsscvalidate( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInoctetsscvalidate_t * data )
{
    int                                          ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inoctetsscvalidate_t  raw;
    ra01_register_t *                            reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INOCTETSSCVALIDATE_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001588,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inoctetsscvalidate_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inoctetsscdecrypted_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t * p )
{
    p->rxSecyOctetDecryptedCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInoctetsscdecrypted( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t * data )
{
    int                                           ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inoctetsscdecrypted_t  raw;
    ra01_register_t *                             reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INOCTETSSCDECRYPTED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001688,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inoctetsscdecrypted_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktsscunchecked_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktsscunchecked_t * p )
{
    p->rxScPktUncheckedCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktsscunchecked( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsscunchecked_t * data )
{
    int                                         ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktsscunchecked_t  raw;
    ra01_register_t *                           reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSCUNCHECKED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001788,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktsscunchecked_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssclateordelayed_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssclateordelayed_t * p )
{
    p->rxScPktLateCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssclateordelayed( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssclateordelayed_t * data )
{
    int                                             ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssclateordelayed_t  raw;
    ra01_register_t *                               reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSCLATEORDELAYED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001888,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssclateordelayed_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssccamhit_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssccamhit_t * p )
{
    p->rxScPktScCamHitCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssccamhit( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssccamhit_t * data )
{
    int                                      ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssccamhit_t  raw;
    ra01_register_t *                        reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSCCAMHIT_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001988,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssccamhit_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssaok_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssaok_t * p )
{
    p->rxScPktOkCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssaok( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssaok_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssaok_t    raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSAOK_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001a88,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssaok_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssainvalid_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssainvalid_t * p )
{
    p->rxScPktInvalidCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssainvalid( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssainvalid_t * data )
{
    int                                       ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssainvalid_t  raw;
    ra01_register_t *                         reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSAINVALID_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001c88,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssainvalid_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssanotvalid_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssanotvalid_t * p )
{
    p->rxScPktNotvalidCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssanotvalid( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssanotvalid_t * data )
{
    int                                        ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssanotvalid_t  raw;
    ra01_register_t *                          reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSANOTVALID_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00001e88,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssanotvalid_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssaunusedsa_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssaunusedsa_t * p )
{
    p->rxSecyPktNosaCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssaunusedsa( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssaunusedsa_t * data )
{
    int                                        ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssaunusedsa_t  raw;
    ra01_register_t *                          reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSAUNUSEDSA_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002088,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssaunusedsa_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktssanotusingsaerror_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t * p )
{
    p->rxSecyPktNosaerrorCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktssanotusingsaerror( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t * data )
{
    int                                               ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktssanotusingsaerror_t  raw;
    ra01_register_t *                                 reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSSANOTUSINGSAERROR_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002288,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktssanotusingsaerror_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktsflowidtcammiss_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t * p )
{
    p->rxPortPktFlowidTcamMissCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktsflowidtcammiss( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t * data )
{
    int                                            ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktsflowidtcammiss_t  raw;
    ra01_register_t *                              reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSFLOWIDTCAMMISS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002488,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktsflowidtcammiss_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktsparseerr_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktsparseerr_t * p )
{
    p->rxPortPktParseErrCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktsparseerr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsparseerr_t * data )
{
    int                                      ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktsparseerr_t  raw;
    ra01_register_t *                        reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSPARSEERR_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002490,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktsparseerr_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktsearlypreempterr_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t * p )
{
    p->rxPortPktEarlyPreemptErrCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktsearlypreempterr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t * data )
{
    int                                             ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktsearlypreempterr_t  raw;
    ra01_register_t *                               reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSEARLYPREEMPTERR_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002498,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktsearlypreempterr_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_rx_mem_inpktsflowidtcamhit_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t * p )
{
    p->rxFlowidPktFlowidTcamHitCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseRxMemInpktsflowidtcamhit( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t * data )
{
    int                                           ret = 0;
    
    ra01_rs_mcs_cse_rx_mem_inpktsflowidtcamhit_t  raw;
    ra01_register_t *                             reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_RX_MEM_INPKTSFLOWIDTCAMHIT_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000024a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_rx_mem_inpktsflowidtcamhit_to_cooked( reg, data );
    return ret;
}

 static void ra01_rs_mcs_cse_tx_mem_ifoutcommonoctets_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemIfoutcommonoctets_t * p )
{
    p->txUnctlOctetCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemIfoutcommonoctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutcommonoctets_t * data )
{
    int                                         ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_ifoutcommonoctets_t  raw;
    ra01_register_t *                           reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_IFOUTCOMMONOCTETS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000025a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_ifoutcommonoctets_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_ifoutunctloctets_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemIfoutunctloctets_t * p )
{
    p->txUnctlOctetCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemIfoutunctloctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutunctloctets_t * data )
{
    int                                        ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_ifoutunctloctets_t  raw;
    ra01_register_t *                          reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLOCTETS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000026a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_ifoutunctloctets_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_ifoutctloctets_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemIfoutctloctets_t * p )
{
    p->txCtlOctetCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemIfoutctloctets( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutctloctets_t * data )
{
    int                                      ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_ifoutctloctets_t  raw;
    ra01_register_t *                        reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLOCTETS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000027a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_ifoutctloctets_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_ifoutunctlucpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t * p )
{
    p->txUnctlPktUcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemIfoutunctlucpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t * data )
{
    int                                        ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_ifoutunctlucpkts_t  raw;
    ra01_register_t *                          reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLUCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000028a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_ifoutunctlucpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_ifoutunctlmcpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t * p )
{
    p->txUnctlPktMcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemIfoutunctlmcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t * data )
{
    int                                        ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_ifoutunctlmcpkts_t  raw;
    ra01_register_t *                          reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLMCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000029a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_ifoutunctlmcpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_ifoutunctlbcpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t * p )
{
    p->txUnctlPktBcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemIfoutunctlbcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t * data )
{
    int                                        ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_ifoutunctlbcpkts_t  raw;
    ra01_register_t *                          reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_IFOUTUNCTLBCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002aa0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_ifoutunctlbcpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_ifoutctlucpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemIfoutctlucpkts_t * p )
{
    p->txCtlPktUcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemIfoutctlucpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutctlucpkts_t * data )
{
    int                                      ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_ifoutctlucpkts_t  raw;
    ra01_register_t *                        reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLUCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002ba0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_ifoutctlucpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_ifoutctlmcpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t * p )
{
    p->txCtlPktMcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemIfoutctlmcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t * data )
{
    int                                      ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_ifoutctlmcpkts_t  raw;
    ra01_register_t *                        reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLMCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002ca0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_ifoutctlmcpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_ifoutctlbcpkts_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t * p )
{
    p->txCtlPktBcastCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemIfoutctlbcpkts( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t * data )
{
    int                                      ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_ifoutctlbcpkts_t  raw;
    ra01_register_t *                        reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_IFOUTCTLBCPKTS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002da0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_ifoutctlbcpkts_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktssecyuntagged_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t * p )
{
    p->txSecyPktUntaggedCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktssecyuntagged( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t * data )
{
    int                                           ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktssecyuntagged_t  raw;
    ra01_register_t *                             reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECYUNTAGGED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002ea0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktssecyuntagged_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktssecytoolong_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktssecytoolong_t * p )
{
    p->txSecyPktToolongCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktssecytoolong( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssecytoolong_t * data )
{
    int                                          ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktssecytoolong_t  raw;
    ra01_register_t *                            reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECYTOOLONG_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00002fa0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktssecytoolong_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktssecynoactivesa_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t * p )
{
    p->txSecyPktNoactivesaCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktssecynoactivesa( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t * data )
{
    int                                             ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktssecynoactivesa_t  raw;
    ra01_register_t *                               reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECYNOACTIVESA_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000030a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktssecynoactivesa_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktsctrlportdisabled_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t * p )
{
    p->txSecyPktCtrlPortDisabledCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktsctrlportdisabled( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t * data )
{
    int                                               ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktsctrlportdisabled_t  raw;
    ra01_register_t *                                 reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSCTRLPORTDISABLED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000031a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktsctrlportdisabled_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outoctetsscprotected_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutoctetsscprotected_t * p )
{
    p->txSecyOctetProtectedCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutoctetsscprotected( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutoctetsscprotected_t * data )
{
    int                                            ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outoctetsscprotected_t  raw;
    ra01_register_t *                              reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTOCTETSSCPROTECTED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000032a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outoctetsscprotected_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outoctetsscencrypted_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t * p )
{
    p->txSecyOctetEncryptedCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutoctetsscencrypted( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t * data )
{
    int                                            ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outoctetsscencrypted_t  raw;
    ra01_register_t *                              reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTOCTETSSCENCRYPTED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000033a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outoctetsscencrypted_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktssaprotected_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktssaprotected_t * p )
{
    p->txScPktProtectedCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktssaprotected( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssaprotected_t * data )
{
    int                                          ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktssaprotected_t  raw;
    ra01_register_t *                            reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSAPROTECTED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000034a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktssaprotected_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktssaencrypted_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktssaencrypted_t * p )
{
    p->txScPktEncryptedCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktssaencrypted( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssaencrypted_t * data )
{
    int                                          ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktssaencrypted_t  raw;
    ra01_register_t *                            reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSAENCRYPTED_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000036a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktssaencrypted_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktsflowidtcammiss_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t * p )
{
    p->txPortPktFlowidTcamMissCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktsflowidtcammiss( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t * data )
{
    int                                             ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktsflowidtcammiss_t  raw;
    ra01_register_t *                               reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSFLOWIDTCAMMISS_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000038a0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktsflowidtcammiss_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktsparseerr_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktsparseerr_t * p )
{
    p->txPortPktParseErrCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktsparseerr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsparseerr_t * data )
{
    int                                       ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktsparseerr_t  raw;
    ra01_register_t *                         reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSPARSEERR_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000038a8,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktsparseerr_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktssectaginsertionerr_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t * p )
{
    p->txPortPktSectagInsertionErrCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktssectaginsertionerr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t * data )
{
    int                                                 ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktssectaginsertionerr_t  raw;
    ra01_register_t *                                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSSECTAGINSERTIONERR_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000038b0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktssectaginsertionerr_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktsearlypreempterr_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t * p )
{
    p->txPortPktEarlyPreemptErrCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktsearlypreempterr( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t * data )
{
    int                                              ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktsearlypreempterr_t  raw;
    ra01_register_t *                                reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSEARLYPREEMPTERR_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000038b8,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktsearlypreempterr_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cse_tx_mem_outpktsflowidtcamhit_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t * p )
{
    p->txFlowidPktFlowidTcamHitCnt = (uint64_t)(raw[0x00000000]);

}


int Ra01ReadRsMcsCseTxMemOutpktsflowidtcamhit( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t * data )
{
    int                                            ret = 0;
    
    ra01_rs_mcs_cse_tx_mem_outpktsflowidtcamhit_t  raw;
    ra01_register_t *                              reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CSE_TX_MEM_OUTPKTSFLOWIDTCAMHIT_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x000038c0,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cse_tx_mem_outpktsflowidtcamhit_to_cooked( reg, data );
    return ret;
}

 


 int Ra01AccRsMcsCpmRxCpmRxInt( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxCpmRxInt_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039c0);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sectagVEq1) & 0x1)
        | (((ra01_register_t)(value->sectagEEq0CEq1) << 1) & 0x2)
        | (((ra01_register_t)(value->sectagSlGte48) << 2) & 0x4)
        | (((ra01_register_t)(value->sectagEsEq1ScEq1) << 3) & 0x8)
        | (((ra01_register_t)(value->sectagScEq1ScbEq1) << 4) & 0x10)
        | (((ra01_register_t)(value->packetXpnEq0) << 5) & 0x20)
        | (((ra01_register_t)(value->pnThreshReached) << 6) & 0x40)
        | (((ra01_register_t)(value->scExpiryPreTimeout) << 7) & 0x80)
        | (((ra01_register_t)(value->scExpiryActualTimeout) << 8) & 0x100)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 9-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sectagVEq1 = (uint8_t)(reg & 0x1);
       value->sectagEEq0CEq1 = (uint8_t)((reg & 0x2) >> 1);
       value->sectagSlGte48 = (uint8_t)((reg & 0x4) >> 2);
       value->sectagEsEq1ScEq1 = (uint8_t)((reg & 0x8) >> 3);
       value->sectagScEq1ScbEq1 = (uint8_t)((reg & 0x10) >> 4);
       value->packetXpnEq0 = (uint8_t)((reg & 0x20) >> 5);
       value->pnThreshReached = (uint8_t)((reg & 0x40) >> 6);
       value->scExpiryPreTimeout = (uint8_t)((reg & 0x80) >> 7);
       value->scExpiryActualTimeout = (uint8_t)((reg & 0x100) >> 8);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxCpmRxIntEnb( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxCpmRxIntEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039c4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sectagVEq1Enb) & 0x1)
        | (((ra01_register_t)(value->sectagEEq0CEq1Enb) << 1) & 0x2)
        | (((ra01_register_t)(value->sectagSlGte48Enb) << 2) & 0x4)
        | (((ra01_register_t)(value->sectagEsEq1ScEq1Enb) << 3) & 0x8)
        | (((ra01_register_t)(value->sectagScEq1ScbEq1Enb) << 4) & 0x10)
        | (((ra01_register_t)(value->packetXpnEq0Enb) << 5) & 0x20)
        | (((ra01_register_t)(value->pnThreshReachedEnb) << 6) & 0x40)
        | (((ra01_register_t)(value->scExpiryPreTimeoutEnb) << 7) & 0x80)
        | (((ra01_register_t)(value->scExpiryActualTimeoutEnb) << 8) & 0x100)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 9-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sectagVEq1Enb = (uint8_t)(reg & 0x1);
       value->sectagEEq0CEq1Enb = (uint8_t)((reg & 0x2) >> 1);
       value->sectagSlGte48Enb = (uint8_t)((reg & 0x4) >> 2);
       value->sectagEsEq1ScEq1Enb = (uint8_t)((reg & 0x8) >> 3);
       value->sectagScEq1ScbEq1Enb = (uint8_t)((reg & 0x10) >> 4);
       value->packetXpnEq0Enb = (uint8_t)((reg & 0x20) >> 5);
       value->pnThreshReachedEnb = (uint8_t)((reg & 0x40) >> 6);
       value->scExpiryPreTimeoutEnb = (uint8_t)((reg & 0x80) >> 7);
       value->scExpiryActualTimeoutEnb = (uint8_t)((reg & 0x100) >> 8);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsCpmRxCpmRxIntRaw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxCpmRxIntRaw_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039c8);


        // We are reading 9-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sectagVEq1 = (uint8_t)(reg & 0x1);
       value->sectagEEq0CEq1 = (uint8_t)((reg & 0x2) >> 1);
       value->sectagSlGte48 = (uint8_t)((reg & 0x4) >> 2);
       value->sectagEsEq1ScEq1 = (uint8_t)((reg & 0x8) >> 3);
       value->sectagScEq1ScbEq1 = (uint8_t)((reg & 0x10) >> 4);
       value->packetXpnEq0 = (uint8_t)((reg & 0x20) >> 5);
       value->pnThreshReached = (uint8_t)((reg & 0x40) >> 6);
       value->scExpiryPreTimeout = (uint8_t)((reg & 0x80) >> 7);
       value->scExpiryActualTimeout = (uint8_t)((reg & 0x100) >> 8);

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmRxCpmRxIntIntrRw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxCpmRxIntIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039cc);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sectagVEq1) & 0x1)
        | (((ra01_register_t)(value->sectagEEq0CEq1) << 1) & 0x2)
        | (((ra01_register_t)(value->sectagSlGte48) << 2) & 0x4)
        | (((ra01_register_t)(value->sectagEsEq1ScEq1) << 3) & 0x8)
        | (((ra01_register_t)(value->sectagScEq1ScbEq1) << 4) & 0x10)
        | (((ra01_register_t)(value->packetXpnEq0) << 5) & 0x20)
        | (((ra01_register_t)(value->pnThreshReached) << 6) & 0x40)
        | (((ra01_register_t)(value->scExpiryPreTimeout) << 7) & 0x80)
        | (((ra01_register_t)(value->scExpiryActualTimeout) << 8) & 0x100)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 9-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sectagVEq1 = (uint8_t)(reg & 0x1);
       value->sectagEEq0CEq1 = (uint8_t)((reg & 0x2) >> 1);
       value->sectagSlGte48 = (uint8_t)((reg & 0x4) >> 2);
       value->sectagEsEq1ScEq1 = (uint8_t)((reg & 0x8) >> 3);
       value->sectagScEq1ScbEq1 = (uint8_t)((reg & 0x10) >> 4);
       value->packetXpnEq0 = (uint8_t)((reg & 0x20) >> 5);
       value->pnThreshReached = (uint8_t)((reg & 0x40) >> 6);
       value->scExpiryPreTimeout = (uint8_t)((reg & 0x80) >> 7);
       value->scExpiryActualTimeout = (uint8_t)((reg & 0x100) >> 8);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCpmRxScExpiryPreTimeout( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScExpiryPreTimeout_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039d0) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryPreTimeoutInt) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryPreTimeoutInt = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxScExpiryPreTimeoutEnb( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScExpiryPreTimeoutEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039d4) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryPreTimeoutIntEnb) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryPreTimeoutIntEnb = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01GetRsMcsCpmRxScExpiryPreTimeoutRaw( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScExpiryPreTimeoutRaw_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039d8) + ((uintAddrBus_t)index*4);


        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryPreTimeoutInt = (uint32_t)(reg & 0xffffffff);

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmRxScExpiryPreTimeoutIntrRw( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScExpiryPreTimeoutIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039dc) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryPreTimeoutInt) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryPreTimeoutInt = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCpmRxScExpiryActualTimeout( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScExpiryActualTimeout_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039e0) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryActualTimeoutInt) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryActualTimeoutInt = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxScExpiryActualTimeoutEnb( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScExpiryActualTimeoutEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039e4) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryActualTimeoutIntEnb) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryActualTimeoutIntEnb = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01GetRsMcsCpmRxScExpiryActualTimeoutRaw( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScExpiryActualTimeoutRaw_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039e8) + ((uintAddrBus_t)index*4);


        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryActualTimeoutInt = (uint32_t)(reg & 0xffffffff);

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmRxScExpiryActualTimeoutIntrRw( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScExpiryActualTimeoutIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039ec) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryActualTimeoutInt) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryActualTimeoutInt = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCpmRxSectagRuleChkEnable( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxSectagRuleChkEnable_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039f0);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sectagVEq1Ena) & 0x1)
        | (((ra01_register_t)(value->sectagEsEq1ScEq1Ena) << 1) & 0x2)
        | (((ra01_register_t)(value->sectagScEq1ScbEq1Ena) << 2) & 0x4)
        | (((ra01_register_t)(value->sectagPnEq0Ena) << 3) & 0x8)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 4-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sectagVEq1Ena = (uint8_t)(reg & 0x1);
       value->sectagEsEq1ScEq1Ena = (uint8_t)((reg & 0x2) >> 1);
       value->sectagScEq1ScbEq1Ena = (uint8_t)((reg & 0x4) >> 2);
       value->sectagPnEq0Ena = (uint8_t)((reg & 0x8) >> 3);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxCpuPuntEnable( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxCpuPuntEnable_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039f4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->puntParseError) & 0x1)
        | (((ra01_register_t)(value->puntFlowidTcamMiss) << 1) & 0x2)
        | (((ra01_register_t)(value->puntCtrlPortDisabled) << 2) & 0x4)
        | (((ra01_register_t)(value->puntUntagged) << 3) & 0x8)
        | (((ra01_register_t)(value->puntBadTag) << 4) & 0x10)
        | (((ra01_register_t)(value->puntScCamMiss) << 5) & 0x20)
        | (((ra01_register_t)(value->puntSaNotInUse) << 6) & 0x40)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 7-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->puntParseError = (uint8_t)(reg & 0x1);
       value->puntFlowidTcamMiss = (uint8_t)((reg & 0x2) >> 1);
       value->puntCtrlPortDisabled = (uint8_t)((reg & 0x4) >> 2);
       value->puntUntagged = (uint8_t)((reg & 0x8) >> 3);
       value->puntBadTag = (uint8_t)((reg & 0x10) >> 4);
       value->puntScCamMiss = (uint8_t)((reg & 0x20) >> 5);
       value->puntSaNotInUse = (uint8_t)((reg & 0x40) >> 6);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxEnableRxmcsInsertion( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxEnableRxmcsInsertion_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039f8) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->insertOnErr) & 0x1)
        | (((ra01_register_t)(value->insertOnKayCtrl) << 1) & 0x2)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 2-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->insertOnErr = (uint8_t)(reg & 0x1);
       value->insertOnKayCtrl = (uint8_t)((reg & 0x2) >> 1);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxRxEtype( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxRxEtype_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x000039fc);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rxMcsHdrEtype) & 0xffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 16-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rxMcsHdrEtype = (uint16_t)(reg & 0xffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxRxDefaultSci( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxRxDefaultSci_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00003a00);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rxDefaultSciLsb) & 0xffffffff)
        | (((ra01_register_t)(value->rxDefaultSciMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->rxDefaultSciLsb = (uint32_t)(reg & 0xffffffff);
       value->rxDefaultSciMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxXpnThreshold( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxXpnThreshold_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00003a08);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->xpnThresholdLsb) & 0xffffffff)
        | (((ra01_register_t)(value->xpnThresholdMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->xpnThresholdLsb = (uint32_t)(reg & 0xffffffff);
       value->xpnThresholdMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxPnThreshold( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxPnThreshold_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00003a10);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->pnThreshold) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->pnThreshold = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01GetRsMcsCpmRxPnThreshReached( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxPnThreshReached_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00003a18) + ((uintAddrBus_t)index*8);


         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->sa = (uint64_t)(reg & 0xffffffffffffffff);

    return ret;
}






 int Ra01AccRsMcsCpmRxSaKeyLockout( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxSaKeyLockout_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00003a20) + ((uintAddrBus_t)index*8);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sa) & 0xffffffffffffffff)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->sa = (uint64_t)(reg & 0xffffffffffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxRxMcsBasePort( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxRxMcsBasePort_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00003a28);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rxMcsBasePort) & 0xff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 8-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rxMcsBasePort = (uint8_t)(reg & 0xff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxSaPnTableUpdate( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxSaPnTableUpdate_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00003a2c);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->maxUpdate) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->maxUpdate = (uint8_t)(reg & 0x1);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxRxFlowidTcamEnable( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxRxFlowidTcamEnable_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005340) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->enable) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->enable = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxScCamEnable( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScCamEnable_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005b60) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->enable) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->enable = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxTimeUnitTick( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxTimeUnitTick_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005d70);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->numCycles) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->numCycles = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxScTimerResetAllGo( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxScTimerResetAllGo_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005d74);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->reset) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->reset = (uint8_t)(reg & 0x1);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmRxScTimerRsvd1( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxScTimerRsvd1_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005d78);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rsvd) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rsvd = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmRxScTimerRsvd2( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxScTimerRsvd2_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005d7c);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rsvd2) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rsvd2 = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCpmRxScTimerTimeoutThresh( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxScTimerTimeoutThresh_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005d80) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->threshold) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->threshold = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}


















 int Ra01AccRsMcsCpmRxPacketSeenRwAn0( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxPacketSeenRwAn0_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005e90) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sc) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sc = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxPacketSeenRwAn1( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxPacketSeenRwAn1_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005e94) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sc) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sc = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxPacketSeenRwAn2( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxPacketSeenRwAn2_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005e98) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sc) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sc = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxPacketSeenRwAn3( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxPacketSeenRwAn3_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005e9c) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sc) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sc = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxPacketSeenW1cAn0( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxPacketSeenW1cAn0_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ea0) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sc) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sc = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxPacketSeenW1cAn1( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxPacketSeenW1cAn1_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ea4) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sc) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sc = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxPacketSeenW1cAn2( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxPacketSeenW1cAn2_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ea8) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sc) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sc = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmRxPacketSeenW1cAn3( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmRxPacketSeenW1cAn3_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005eac) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sc) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sc = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmRxDbgMuxSel( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxDbgMuxSel_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005eb0);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->seg0) & 0x3f)
        | (((ra01_register_t)(value->seg1) << 8) & 0x3f00)
        | (((ra01_register_t)(value->seg2) << 16) & 0x3f0000)
        | (((ra01_register_t)(value->seg3) << 24) & 0x3f000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 24-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->seg0 = (uint8_t)(reg & 0x3f);
       value->seg1 = (uint8_t)((reg & 0x3f00) >> 8);
       value->seg2 = (uint8_t)((reg & 0x3f0000) >> 16);
       value->seg3 = (uint8_t)((reg & 0x3f000000) >> 24);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsCpmRxDebugStatus( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmRxDebugStatus_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005eb8);


         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);

    return ret;
}


#endif // MCS_API_DEBUG

static void ra01_rs_mcs_cpm_rx_secy_map_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmRxSecyMap_t * p )
{
    p->secy = (uint8_t)((raw[0x00000000] & 0x1f) >> 0);

    p->ctrlPkt = ((((raw[0x00000000] & 0x20) >> 5) & 1) == 1);

}

int Ra01WriteRsMcsCpmRxSecyMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSecyMap_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_rx_secy_map_mem_t   raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_rx_secy_map_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00003a30,8,1, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmRxSecyMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSecyMap_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_rx_secy_map_mem_t   raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_RX_SECY_MAP_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00003a30,8,1, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_rx_secy_map_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_rx_secy_plcy_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmRxSecyPlcy_t * p )
{
    p->controlledPortEnabled = ((((raw[0x00000000] & 0x1) >> 0) & 1) == 1);

    p->validateFrames = (uint8_t)((raw[0x00000000] & 0x6) >> 1);

    p->stripSectagIcv = (uint8_t)((raw[0x00000000] & 0x18) >> 3);

    p->cipher = (uint8_t)((raw[0x00000000] & 0x1e0) >> 5);

    p->confidentialityOffset = (uint8_t)((raw[0x00000000] & 0xfe00) >> 9);

    p->preSectagAuthEnable = ((((raw[0x00000000] & 0x10000) >> 16) & 1) == 1);

    p->replayProtect = ((((raw[0x00000000] & 0x20000) >> 17) & 1) == 1);

    p->replayWindow = (uint32_t)((raw[0x00000000] & 0x3fffffffc0000) >> 18);

    p->reserved = (uint32_t)(((raw[0x00000000] >> 50) | (raw[0x00000000+1] << 14)) & 0x7fffff);

}

int Ra01WriteRsMcsCpmRxSecyPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSecyPlcy_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_rx_secy_plcy_mem_t  raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_rx_secy_plcy_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00003b30,16,4, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmRxSecyPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSecyPlcy_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_rx_secy_plcy_mem_t  raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_RX_SECY_PLCY_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00003b30,16,4, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_rx_secy_plcy_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_rx_sa_map_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmRxSaMap_t * p )
{
    p->saIndex = (uint8_t)((raw[0x00000000] & 0x3f) >> 0);

    p->saInUse = ((((raw[0x00000000] & 0x40) >> 6) & 1) == 1);

}

int Ra01WriteRsMcsCpmRxSaMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaMap_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_rx_sa_map_mem_t     raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_rx_sa_map_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00003d30,8,1, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmRxSaMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaMap_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_rx_sa_map_mem_t     raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_RX_SA_MAP_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00003d30,8,1, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_rx_sa_map_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_rx_sa_plcy_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmRxSaPlcy_t * p )
{
    ra01_extract_mem_field_by_id( raw, 0x00000000,255,0, &(p->sak), RA01_TYPE_MEMORY );

    ra01_extract_mem_field_by_id( raw, 0x00000004,127,0, &(p->hashkey), RA01_TYPE_MEMORY );

    ra01_extract_mem_field_by_id( raw, 0x00000006,95,0, &(p->salt), RA01_TYPE_MEMORY );

    p->ssci = (uint32_t)((raw[0x00000007] >> 32) & 0xffffffff);

}

int Ra01WriteRsMcsCpmRxSaPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaPlcy_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_rx_sa_plcy_mem_t    raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_rx_sa_plcy_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00004140,64,16, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmRxSaPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaPlcy_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_rx_sa_plcy_mem_t    raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_RX_SA_PLCY_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00004140,64,16, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_rx_sa_plcy_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_rx_sa_pn_table_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmRxSaPnTable_t * p )
{
    p->nextPn = (uint64_t)(raw[0x00000000]);

}

int Ra01WriteRsMcsCpmRxSaPnTable( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaPnTable_t * data )
{
    int                                  ret = 0;
    ra01_rs_mcs_cpm_rx_sa_pn_table_mem_t raw;
    ra01_register_t *                    reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_rx_sa_pn_table_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00005140,8,2, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmRxSaPnTable( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxSaPnTable_t * data )
{
    int                                   ret = 0;
    
    ra01_rs_mcs_cpm_rx_sa_pn_table_mem_t  raw;
    ra01_register_t *                     reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_RX_SA_PN_TABLE_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00005140,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_rx_sa_pn_table_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_rx_flowid_tcam_data_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmRxFlowidTcamData_t * p )
{
    p->macDa = (uint64_t)((raw[0x00000000] & 0xffffffffffff) >> 0);

    p->macSa = (((raw[0x00000000] >> 48) | (raw[0x00000000+1] << 16)) & 0xffffffffffff);

    p->etherType = (uint16_t)((raw[0x00000001] >> 32) & 0xffff);

    p->outerTagId = (uint32_t)(((raw[0x00000001] >> 48) | (raw[0x00000001+1] << 16)) & 0xfffff);

    p->outerPriority = (uint8_t)((raw[0x00000002] >> 4) & 0xf);

    p->secondOuterTagId = (uint32_t)((raw[0x00000002] >> 8) & 0xfffff);

    p->secondOuterPriority = (uint8_t)((raw[0x00000002] >> 28) & 0xf);

    p->bonusData = (uint16_t)((raw[0x00000002] >> 32) & 0xffff);

    p->tagMatchBitmap = (uint8_t)((raw[0x00000002] >> 48) & 0xff);

    p->packetType = (uint8_t)((raw[0x00000002] >> 56) & 0xf);

    p->outerVlanType = (uint8_t)((raw[0x00000002] >> 60) & 0x7);

    p->innerVlanType = (uint8_t)(((raw[0x00000002] >> 63) | (raw[0x00000002+1] << 1)) & 0x7);

    p->numTags = (uint8_t)((raw[0x00000003] >> 2) & 0x7f);

    p->express = ((((raw[0x00000003] & 0x200) >> 9) & 1) == 1);

    p->port = ((((raw[0x00000003] & 0x400) >> 10) & 1) == 1);

}

int Ra01WriteRsMcsCpmRxFlowidTcamData( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxFlowidTcamData_t * data )
{
    int                                   ret = 0;
    ra01_rs_mcs_cpm_rx_flowid_tcam_data_t raw;
    ra01_register_t *                     reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_rx_flowid_tcam_data_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00005360,32,8, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmRxFlowidTcamData( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxFlowidTcamData_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cpm_rx_flowid_tcam_data_t  raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_RX_FLOWID_TCAM_DATA_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00005360,32,8, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_rx_flowid_tcam_data_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_rx_flowid_tcam_mask_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmRxFlowidTcamMask_t * p )
{
    p->maskMacDa = (uint64_t)((raw[0x00000000] & 0xffffffffffff) >> 0);

    p->maskMacSa = (((raw[0x00000000] >> 48) | (raw[0x00000000+1] << 16)) & 0xffffffffffff);

    p->maskEtherType = (uint16_t)((raw[0x00000001] >> 32) & 0xffff);

    p->maskOuterTagId = (uint32_t)(((raw[0x00000001] >> 48) | (raw[0x00000001+1] << 16)) & 0xfffff);

    p->maskOuterPriority = (uint8_t)((raw[0x00000002] >> 4) & 0xf);

    p->maskSecondOuterTagId = (uint32_t)((raw[0x00000002] >> 8) & 0xfffff);

    p->maskSecondOuterPriority = (uint8_t)((raw[0x00000002] >> 28) & 0xf);

    p->maskBonusData = (uint16_t)((raw[0x00000002] >> 32) & 0xffff);

    p->maskTagMatchBitmap = (uint8_t)((raw[0x00000002] >> 48) & 0xff);

    p->maskPacketType = (uint8_t)((raw[0x00000002] >> 56) & 0xf);

    p->maskOuterVlanType = (uint8_t)((raw[0x00000002] >> 60) & 0x7);

    p->maskInnerVlanType = (uint8_t)(((raw[0x00000002] >> 63) | (raw[0x00000002+1] << 1)) & 0x7);

    p->maskNumTags = (uint8_t)((raw[0x00000003] >> 2) & 0x7f);

    p->maskExpress = ((((raw[0x00000003] & 0x200) >> 9) & 1) == 1);

    p->maskPort = ((((raw[0x00000003] & 0x400) >> 10) & 1) == 1);

}

int Ra01WriteRsMcsCpmRxFlowidTcamMask( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxFlowidTcamMask_t * data )
{
    int                                   ret = 0;
    ra01_rs_mcs_cpm_rx_flowid_tcam_mask_t raw;
    ra01_register_t *                     reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_rx_flowid_tcam_mask_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00005760,32,8, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmRxFlowidTcamMask( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxFlowidTcamMask_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cpm_rx_flowid_tcam_mask_t  raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_RX_FLOWID_TCAM_MASK_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00005760,32,8, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_rx_flowid_tcam_mask_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_rx_sc_cam_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmRxScCam_t * p )
{
    p->sci = (uint64_t)(raw[0x00000000]);

    p->secy = (uint8_t)((raw[0x00000001] >> 0) & 0x1f);

}

int Ra01WriteRsMcsCpmRxScCam( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScCam_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_rx_sc_cam_t         raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_rx_sc_cam_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00005b70,16,4, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmRxScCam( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScCam_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_rx_sc_cam_t         raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_RX_SC_CAM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00005b70,16,4, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_rx_sc_cam_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_rx_sc_timer_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmRxScTimer_t * p )
{
    p->timer = (uint32_t)((raw[0x00000000] & 0xffffffff) >> 0);

    p->preTimeoutThreshSelect = (uint8_t)((raw[0x00000000] & 0x300000000) >> 32);

    p->actualTimeoutThreshSelect = (uint8_t)((raw[0x00000000] & 0xc00000000) >> 34);

    p->enable = ((((raw[0x00000000] & 0x1000000000) >> 36) & 1) == 1);

}

int Ra01WriteRsMcsCpmRxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScTimer_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_rx_sc_timer_mem_t   raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_rx_sc_timer_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00005d90,8,2, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmRxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScTimer_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_rx_sc_timer_mem_t   raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_RX_SC_TIMER_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00005d90,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_rx_sc_timer_to_cooked( reg, data );
    return ret;
}

 


 int Ra01AccRsMcsCpmTxCpmTxInt( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxCpmTxInt_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ec0);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->packetXpnEq0) & 0x1)
        | (((ra01_register_t)(value->pnThreshReached) << 1) & 0x2)
        | (((ra01_register_t)(value->saNotValid) << 2) & 0x4)
        | (((ra01_register_t)(value->scExpiryPreTimeout) << 3) & 0x8)
        | (((ra01_register_t)(value->scExpiryActualTimeout) << 4) & 0x10)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->packetXpnEq0 = (uint8_t)(reg & 0x1);
       value->pnThreshReached = (uint8_t)((reg & 0x2) >> 1);
       value->saNotValid = (uint8_t)((reg & 0x4) >> 2);
       value->scExpiryPreTimeout = (uint8_t)((reg & 0x8) >> 3);
       value->scExpiryActualTimeout = (uint8_t)((reg & 0x10) >> 4);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxCpmTxIntEnb( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxCpmTxIntEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ec4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->packetXpnEq0Enb) & 0x1)
        | (((ra01_register_t)(value->pnThreshReachedEnb) << 1) & 0x2)
        | (((ra01_register_t)(value->saNotValidEnb) << 2) & 0x4)
        | (((ra01_register_t)(value->scExpiryPreTimeoutEnb) << 3) & 0x8)
        | (((ra01_register_t)(value->scExpiryActualTimeoutEnb) << 4) & 0x10)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->packetXpnEq0Enb = (uint8_t)(reg & 0x1);
       value->pnThreshReachedEnb = (uint8_t)((reg & 0x2) >> 1);
       value->saNotValidEnb = (uint8_t)((reg & 0x4) >> 2);
       value->scExpiryPreTimeoutEnb = (uint8_t)((reg & 0x8) >> 3);
       value->scExpiryActualTimeoutEnb = (uint8_t)((reg & 0x10) >> 4);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsCpmTxCpmTxIntRaw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxCpmTxIntRaw_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ec8);


        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->packetXpnEq0 = (uint8_t)(reg & 0x1);
       value->pnThreshReached = (uint8_t)((reg & 0x2) >> 1);
       value->saNotValid = (uint8_t)((reg & 0x4) >> 2);
       value->scExpiryPreTimeout = (uint8_t)((reg & 0x8) >> 3);
       value->scExpiryActualTimeout = (uint8_t)((reg & 0x10) >> 4);

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmTxCpmTxIntIntrRw( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxCpmTxIntIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ecc);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->packetXpnEq0) & 0x1)
        | (((ra01_register_t)(value->pnThreshReached) << 1) & 0x2)
        | (((ra01_register_t)(value->saNotValid) << 2) & 0x4)
        | (((ra01_register_t)(value->scExpiryPreTimeout) << 3) & 0x8)
        | (((ra01_register_t)(value->scExpiryActualTimeout) << 4) & 0x10)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->packetXpnEq0 = (uint8_t)(reg & 0x1);
       value->pnThreshReached = (uint8_t)((reg & 0x2) >> 1);
       value->saNotValid = (uint8_t)((reg & 0x4) >> 2);
       value->scExpiryPreTimeout = (uint8_t)((reg & 0x8) >> 3);
       value->scExpiryActualTimeout = (uint8_t)((reg & 0x10) >> 4);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCpmTxScExpiryPreTimeout( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxScExpiryPreTimeout_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ed0) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryPreTimeoutInt) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryPreTimeoutInt = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxScExpiryPreTimeoutEnb( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxScExpiryPreTimeoutEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ed4) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryPreTimeoutIntEnb) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryPreTimeoutIntEnb = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01GetRsMcsCpmTxScExpiryPreTimeoutRaw( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxScExpiryPreTimeoutRaw_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ed8) + ((uintAddrBus_t)index*4);


        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryPreTimeoutInt = (uint32_t)(reg & 0xffffffff);

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmTxScExpiryPreTimeoutIntrRw( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxScExpiryPreTimeoutIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005edc) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryPreTimeoutInt) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryPreTimeoutInt = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCpmTxScExpiryActualTimeout( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxScExpiryActualTimeout_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ee0) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryActualTimeoutInt) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryActualTimeoutInt = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxScExpiryActualTimeoutEnb( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxScExpiryActualTimeoutEnb_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ee4) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryActualTimeoutIntEnb) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryActualTimeoutIntEnb = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01GetRsMcsCpmTxScExpiryActualTimeoutRaw( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxScExpiryActualTimeoutRaw_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ee8) + ((uintAddrBus_t)index*4);


        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryActualTimeoutInt = (uint32_t)(reg & 0xffffffff);

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmTxScExpiryActualTimeoutIntrRw( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxScExpiryActualTimeoutIntrRw_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005eec) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->scExpiryActualTimeoutInt) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->scExpiryActualTimeoutInt = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCpmTxTxPortCfg( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxTxPortCfg_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ef0) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sectagEtype) & 0xffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 16-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->sectagEtype = (uint16_t)(reg & 0xffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxXpnThreshold( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxXpnThreshold_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005ef8);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->xpnThresholdLsb) & 0xffffffff)
        | (((ra01_register_t)(value->xpnThresholdMsb) << 32) & 0xffffffff00000000)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->xpnThresholdLsb = (uint32_t)(reg & 0xffffffff);
       value->xpnThresholdMsb = (uint32_t)((reg & 0xffffffff00000000) >> 32);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxPnThreshold( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxPnThreshold_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005f00);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->pnThreshold) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->pnThreshold = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxSaKeyLockout( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxSaKeyLockout_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005f08) + ((uintAddrBus_t)index*8);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->sa) & 0xffffffffffffffff)
      );
        // We are writing to a 64-bit register over a 32-bit bus...
       { 
           uintRegAccess_t * r = (uintRegAccess_t *)(&reg);
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, *r );
           r++;
           ret |= (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof(uintRegAccess_t)), *r );
       }
    }
    else {
         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->sa = (uint64_t)(reg & 0xffffffffffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxFixedOffsetAdjust( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxFixedOffsetAdjust_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00005f10);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->fixedOffsetAdjust) & 0x1f)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 5-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->fixedOffsetAdjust = (uint8_t)(reg & 0x1f);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxTxFlowidTcamEnable( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxTxFlowidTcamEnable_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 1) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00008540) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->enable) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->enable = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxTimeUnitTick( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxTimeUnitTick_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00008d60);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->numCycles) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->numCycles = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}






 int Ra01AccRsMcsCpmTxScTimerResetAllGo( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxScTimerResetAllGo_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00008d64);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->reset) & 0x1)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 1-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->reset = (uint8_t)(reg & 0x1);
    }

    return ret;
}




#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmTxScTimerRsvd1( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxScTimerRsvd1_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00008d68);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rsvd1) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rsvd1 = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmTxScTimerRsvd2( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxScTimerRsvd2_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00008d6c);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->rsvd2) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->rsvd2 = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}


#endif // MCS_API_DEBUG




 int Ra01AccRsMcsCpmTxScTimerTimeoutThresh( RmsDev_t * rmsDev_p, 
     unsigned index,
     Ra01RsMcsCpmTxScTimerTimeoutThresh_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;

    if (index >= 4) { return -EINVAL; }


   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00008d70) + ((uintAddrBus_t)index*4);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->threshold) & 0xffffffff)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 32-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->threshold = (uint32_t)(reg & 0xffffffff);
    }

    return ret;
}
















#ifdef MCS_API_DEBUG


 int Ra01AccRsMcsCpmTxDbgMuxSel( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxDbgMuxSel_t * value,
     ra01_reg_access_op_t op) 
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00008e80);


    if( op == RA01_WRITE_OP ) {
      reg = (0 
        | ((ra01_register_t)(value->seg0) & 0x3f)
        | (((ra01_register_t)(value->seg1) << 8) & 0x3f00)
        | (((ra01_register_t)(value->seg2) << 16) & 0x3f0000)
        | (((ra01_register_t)(value->seg3) << 24) & 0x3f000000)
      );
       ret = (*rmsDev_p->rmsHwWriteFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t)reg );
    }
    else {
        // We are reading 24-bit reg over 32-bit bus
        {
            uintRegAccess_t tempRegVal;
            ret = (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(&tempRegVal) );
            reg = (ra01_register_t)tempRegVal;
        }
       value->seg0 = (uint8_t)(reg & 0x3f);
       value->seg1 = (uint8_t)((reg & 0x3f00) >> 8);
       value->seg2 = (uint8_t)((reg & 0x3f0000) >> 16);
       value->seg3 = (uint8_t)((reg & 0x3f000000) >> 24);
    }

    return ret;
}


#endif // MCS_API_DEBUG


#ifdef MCS_API_DEBUG


 int Ra01GetRsMcsCpmTxDebugStatus( RmsDev_t * rmsDev_p, 
     Ra01RsMcsCpmTxDebugStatus_t * value)
{
    int                               ret = 0;
    ra01_register_t                   reg = 0;



   uintAddrBus_t offset;

    offset = ((uintAddrBus_t)0x00008e88);


         // We are reading to a 64-bit register over a 32-bit bus...
        {
            uint32_t volatile * d = (uint32_t volatile *)&reg;

            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset, (uintRegAccess_t*)(d++) );
            // Only read the MSB if there are some...
            ret |= (*rmsDev_p->rmsHwReadFn)( rmsDev_p->userData_p, offset + ((uintAddrBus_t)sizeof( *d )), (uintRegAccess_t*)d );
        }
       value->seg0 = (uint16_t)(reg & 0xffff);
       value->seg1 = (uint16_t)((reg & 0xffff0000) >> 16);
       value->seg2 = (uint16_t)((reg & 0xffff00000000) >> 32);
       value->seg3 = (uint16_t)((reg & 0xffff000000000000) >> 48);

    return ret;
}


#endif // MCS_API_DEBUG

static void ra01_rs_mcs_cpm_tx_secy_map_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmTxSecyMap_t * p )
{
    p->secy = (uint8_t)((raw[0x00000000] & 0x1f) >> 0);

    p->ctrlPkt = ((((raw[0x00000000] & 0x20) >> 5) & 1) == 1);

    p->sc = (uint8_t)((raw[0x00000000] & 0x7c0) >> 6);

    p->auxiliaryPlcy = ((((raw[0x00000000] & 0x800) >> 11) & 1) == 1);

}

int Ra01WriteRsMcsCpmTxSecyMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSecyMap_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_tx_secy_map_mem_t   raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_tx_secy_map_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00005f18,8,1, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmTxSecyMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSecyMap_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_tx_secy_map_mem_t   raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_TX_SECY_MAP_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00005f18,8,1, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_tx_secy_map_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_tx_secy_plcy_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmTxSecyPlcy_t * p )
{
    p->controlledPortEnabled = ((((raw[0x00000000] & 0x1) >> 0) & 1) == 1);

    p->protectFrames = ((((raw[0x00000000] & 0x2) >> 1) & 1) == 1);

    p->cipher = (uint8_t)((raw[0x00000000] & 0x3c) >> 2);

    p->confidentialityOffset = (uint8_t)((raw[0x00000000] & 0x1fc0) >> 6);

    p->preSectagAuthEnable = ((((raw[0x00000000] & 0x2000) >> 13) & 1) == 1);

    p->sectagInsertMode = ((((raw[0x00000000] & 0x4000) >> 14) & 1) == 1);

    p->sectagOffset = (uint8_t)((raw[0x00000000] & 0x3f8000) >> 15);

    p->sectagTci = (uint8_t)((raw[0x00000000] & 0xfc00000) >> 22);

    p->mtu = (uint16_t)((raw[0x00000000] & 0xffff0000000) >> 28);

    p->reserved = (uint16_t)((raw[0x00000000] & 0xffff00000000000) >> 44);

}

int Ra01WriteRsMcsCpmTxSecyPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSecyPlcy_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_tx_secy_plcy_mem_t  raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_tx_secy_plcy_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00006018,8,2, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmTxSecyPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSecyPlcy_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_tx_secy_plcy_mem_t  raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_TX_SECY_PLCY_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00006018,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_tx_secy_plcy_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_tx_sa_map_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmTxSaMap_t * p )
{
    p->saIndex0 = (uint8_t)((raw[0x00000000] & 0x3f) >> 0);

    p->saIndex1 = (uint8_t)((raw[0x00000000] & 0xfc0) >> 6);

    p->autoRekeyEnable = ((((raw[0x00000000] & 0x1000) >> 12) & 1) == 1);

    p->saIndex0Vld = ((((raw[0x00000000] & 0x2000) >> 13) & 1) == 1);

    p->saIndex1Vld = ((((raw[0x00000000] & 0x4000) >> 14) & 1) == 1);

    p->txSaActive = ((((raw[0x00000000] & 0x8000) >> 15) & 1) == 1);

    p->sectagSci = (((raw[0x00000000] >> 16) | (raw[0x00000000+1] << 48)) & 0xffffffffffffffff);

}

int Ra01WriteRsMcsCpmTxSaMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaMap_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_tx_sa_map_mem_t     raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_tx_sa_map_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00006120,16,4, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmTxSaMap( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaMap_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_tx_sa_map_mem_t     raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_TX_SA_MAP_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00006120,16,4, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_tx_sa_map_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_tx_sa_plcy_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmTxSaPlcy_t * p )
{
    ra01_extract_mem_field_by_id( raw, 0x00000000,255,0, &(p->sak), RA01_TYPE_MEMORY );

    ra01_extract_mem_field_by_id( raw, 0x00000004,127,0, &(p->hashkey), RA01_TYPE_MEMORY );

    ra01_extract_mem_field_by_id( raw, 0x00000006,95,0, &(p->salt), RA01_TYPE_MEMORY );

    p->ssci = (uint32_t)((raw[0x00000007] >> 32) & 0xffffffff);

    p->sectagAn = (uint8_t)((raw[0x00000008] >> 0) & 0x3);

}

int Ra01WriteRsMcsCpmTxSaPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaPlcy_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_tx_sa_plcy_mem_t    raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_tx_sa_plcy_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00006340,128,18, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmTxSaPlcy( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaPlcy_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_tx_sa_plcy_mem_t    raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_TX_SA_PLCY_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00006340,128,18, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_tx_sa_plcy_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_tx_sa_pn_table_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmTxSaPnTable_t * p )
{
    p->nextPn = (uint64_t)(raw[0x00000000]);

}

int Ra01WriteRsMcsCpmTxSaPnTable( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaPnTable_t * data )
{
    int                                  ret = 0;
    ra01_rs_mcs_cpm_tx_sa_pn_table_mem_t raw;
    ra01_register_t *                    reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_tx_sa_pn_table_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00008340,8,2, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmTxSaPnTable( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxSaPnTable_t * data )
{
    int                                   ret = 0;
    
    ra01_rs_mcs_cpm_tx_sa_pn_table_mem_t  raw;
    ra01_register_t *                     reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_TX_SA_PN_TABLE_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00008340,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_tx_sa_pn_table_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_tx_flowid_tcam_data_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmTxFlowidTcamData_t * p )
{
    p->macDa = (uint64_t)((raw[0x00000000] & 0xffffffffffff) >> 0);

    p->macSa = (((raw[0x00000000] >> 48) | (raw[0x00000000+1] << 16)) & 0xffffffffffff);

    p->etherType = (uint16_t)((raw[0x00000001] >> 32) & 0xffff);

    p->outerTagId = (uint32_t)(((raw[0x00000001] >> 48) | (raw[0x00000001+1] << 16)) & 0xfffff);

    p->outerPriority = (uint8_t)((raw[0x00000002] >> 4) & 0xf);

    p->secondOuterTagId = (uint32_t)((raw[0x00000002] >> 8) & 0xfffff);

    p->secondOuterPriority = (uint8_t)((raw[0x00000002] >> 28) & 0xf);

    p->bonusData = (uint16_t)((raw[0x00000002] >> 32) & 0xffff);

    p->tagMatchBitmap = (uint8_t)((raw[0x00000002] >> 48) & 0xff);

    p->packetType = (uint8_t)((raw[0x00000002] >> 56) & 0xf);

    p->outerVlanType = (uint8_t)((raw[0x00000002] >> 60) & 0x7);

    p->innerVlanType = (uint8_t)(((raw[0x00000002] >> 63) | (raw[0x00000002+1] << 1)) & 0x7);

    p->numTags = (uint8_t)((raw[0x00000003] >> 2) & 0x7f);

    p->express = ((((raw[0x00000003] & 0x200) >> 9) & 1) == 1);

    p->port = ((((raw[0x00000003] & 0x400) >> 10) & 1) == 1);

}

int Ra01WriteRsMcsCpmTxFlowidTcamData( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxFlowidTcamData_t * data )
{
    int                                   ret = 0;
    ra01_rs_mcs_cpm_tx_flowid_tcam_data_t raw;
    ra01_register_t *                     reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_tx_flowid_tcam_data_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00008560,32,8, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmTxFlowidTcamData( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxFlowidTcamData_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cpm_tx_flowid_tcam_data_t  raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_TX_FLOWID_TCAM_DATA_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00008560,32,8, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_tx_flowid_tcam_data_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_tx_flowid_tcam_mask_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmTxFlowidTcamMask_t * p )
{
    p->maskMacDa = (uint64_t)((raw[0x00000000] & 0xffffffffffff) >> 0);

    p->maskMacSa = (((raw[0x00000000] >> 48) | (raw[0x00000000+1] << 16)) & 0xffffffffffff);

    p->maskEtherType = (uint16_t)((raw[0x00000001] >> 32) & 0xffff);

    p->maskOuterTagId = (uint32_t)(((raw[0x00000001] >> 48) | (raw[0x00000001+1] << 16)) & 0xfffff);

    p->maskOuterPriority = (uint8_t)((raw[0x00000002] >> 4) & 0xf);

    p->maskSecondOuterTagId = (uint32_t)((raw[0x00000002] >> 8) & 0xfffff);

    p->maskSecondOuterPriority = (uint8_t)((raw[0x00000002] >> 28) & 0xf);

    p->maskBonusData = (uint16_t)((raw[0x00000002] >> 32) & 0xffff);

    p->maskTagMatchBitmap = (uint8_t)((raw[0x00000002] >> 48) & 0xff);

    p->maskPacketType = (uint8_t)((raw[0x00000002] >> 56) & 0xf);

    p->maskOuterVlanType = (uint8_t)((raw[0x00000002] >> 60) & 0x7);

    p->maskInnerVlanType = (uint8_t)(((raw[0x00000002] >> 63) | (raw[0x00000002+1] << 1)) & 0x7);

    p->maskNumTags = (uint8_t)((raw[0x00000003] >> 2) & 0x7f);

    p->maskExpress = ((((raw[0x00000003] & 0x200) >> 9) & 1) == 1);

    p->maskPort = ((((raw[0x00000003] & 0x400) >> 10) & 1) == 1);

}

int Ra01WriteRsMcsCpmTxFlowidTcamMask( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxFlowidTcamMask_t * data )
{
    int                                   ret = 0;
    ra01_rs_mcs_cpm_tx_flowid_tcam_mask_t raw;
    ra01_register_t *                     reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_tx_flowid_tcam_mask_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00008960,32,8, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmTxFlowidTcamMask( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxFlowidTcamMask_t * data )
{
    int                                    ret = 0;
    
    ra01_rs_mcs_cpm_tx_flowid_tcam_mask_t  raw;
    ra01_register_t *                      reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_TX_FLOWID_TCAM_MASK_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00008960,32,8, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_tx_flowid_tcam_mask_to_cooked( reg, data );
    return ret;
}

static void ra01_rs_mcs_cpm_tx_sc_timer_to_cooked( ra01_register_t const * raw, Ra01IfRsMcsCpmTxScTimer_t * p )
{
    p->timer = (uint32_t)((raw[0x00000000] & 0xffffffff) >> 0);

    p->preTimeoutThreshSelect = (uint8_t)((raw[0x00000000] & 0x300000000) >> 32);

    p->actualTimeoutThreshSelect = (uint8_t)((raw[0x00000000] & 0xc00000000) >> 34);

    p->enable = ((((raw[0x00000000] & 0x1000000000) >> 36) & 1) == 1);

    p->startMode = ((((raw[0x00000000] & 0x2000000000) >> 37) & 1) == 1);

    p->startGo = ((((raw[0x00000000] & 0x4000000000) >> 38) & 1) == 1);

}

int Ra01WriteRsMcsCpmTxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxScTimer_t * data )
{
    int                                 ret = 0;
    ra01_rs_mcs_cpm_tx_sc_timer_mem_t   raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    memset( raw, 0, sizeof(raw));

    ra01_rs_mcs_cpm_tx_sc_timer_to_raw( data, reg );

    ret = ra01_write_mem_field_by_id( rmsDev_p,0x00008d80,8,2, index, reg );
    
    return ret;
}

int Ra01ReadRsMcsCpmTxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxScTimer_t * data )
{
    int                                 ret = 0;
    
    ra01_rs_mcs_cpm_tx_sc_timer_mem_t   raw;
    ra01_register_t *                   reg = (ra01_register_t *)&(raw[0]);

    if( index >= RA01_RS_MCS_CPM_TX_SC_TIMER_MEM_SIZE ) {
        return -EINVAL; 
    }

    memset( raw, 0, sizeof(raw));

    ret = ra01_read_mem_field_by_id( rmsDev_p,0x00008d80,8,2, index, reg );
    if( ret < 0 ) { return ret; }

    ra01_rs_mcs_cpm_tx_sc_timer_to_cooked( reg, data );
    return ret;
}

 
