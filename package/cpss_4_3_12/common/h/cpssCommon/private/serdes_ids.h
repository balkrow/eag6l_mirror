#if 0
#*******************************************************************************
#              (c), Copyright 2015, Marvell International Ltd.                 *
# THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
# NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
# OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
# DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
# THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
# IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
#*******************************************************************************
#
#*******************************************************************************
# @file serdes_ids.h
#
# @brief Defines the SERDES_IDS enumeration, which contains the IDs of various SERDES devices.
#        This file is used by both C code and bash script, hence its unique syntax
#
# @version   1
#*******************************************************************************
#
#endif

#if 0
    shopt -s expand_aliases
    alias ENUM='true'
    alias COMMA=
#else
#   define ENUM  enum
#   define COMMA ,
#endif


ENUM SERDES_IDS
{
    COMPHYC28GP4X1CMN25=0x0           COMMA
    COMPHYC28GP4X1156=0x1             COMMA
    COMPHYC28GP4X1LANE=0x2            COMMA
    COMPHYC28GP4X1MAIN=0x3            COMMA
    COMPHYC28GP4X4MAIN=0x4            COMMA
    COMPHYC28GP4X4CMN25=0x5           COMMA  
    COMPHYC28GP4X4156=0x6             COMMA
    COMPHYC28GP4X4LANE=0x7            COMMA
    COMPHYC112GX4MAIN=0x8             COMMA
    COMPHYN5XC56GP5X4MAIN=0x9         COMMA
    COMPHYC12GP41P2VMAIN=0xa          COMMA
}


#if 0
    srds_arr[$COMPHYC28GP4X1CMN25]="COMPHYC28GP4X1CMN25"
    srds_arr[$COMPHYC28GP4X1156]="COMPHYC28GP4X1156"
    srds_arr[$COMPHYC28GP4X1LANE]="COMPHYC28GP4X1LANE"
    srds_arr[$COMPHYC28GP4X1MAIN]="COMPHYC28GP4X1MAIN"
    srds_arr[$COMPHYC28GP4X4MAIN]="COMPHYC28GP4X4MAIN"
    srds_arr[$COMPHYC28GP4X4CMN25]="COMPHYC28GP4X4CMN25"
    srds_arr[$COMPHYC28GP4X4156]="COMPHYC28GP4X4156"
    srds_arr[$COMPHYC28GP4X4LANE]="COMPHYC28GP4X4LANE"
    srds_arr[$COMPHYC112GX4MAIN]="COMPHYC112GX4MAIN"
    srds_arr[$COMPHYN5XC56GP5X4MAIN]="COMPHYN5XC56GP5X4MAIN"
    srds_arr[$COMPHYC12GP41P2VMAIN]="COMPHYC12GP41P2VMAIN"
#endif
