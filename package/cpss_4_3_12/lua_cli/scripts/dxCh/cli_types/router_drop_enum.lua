--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* router_drop_enum.lua
--*
--* DESCRIPTION:
--*       Modes enum for router drop counter
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes


--constants


--------------------------------------------
-- type registration: router_drop_counter_mode
--------------------------------------------
CLI_type_dict["router_drop_counter_mode"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "Enter counter mode",
    enum = {
    ["COUNT_ALL"]=          {value="CPSS_DXCH_IP_DROP_CNT_COUNT_ALL_MODE_E",
                                                  help="Count All."},
    ["IP_HEADER"]=	        {value="CPSS_DXCH_IP_DROP_CNT_IP_HEADER_MODE_E",
                                                  help="Count packets dropped due to IP Header Error."},
    ["DIP_DA_MISMATCH"]=    {value="CPSS_DXCH_IP_DROP_CNT_DIP_DA_MISMATCH_MODE_E",
                                                  help="Count packets dropped due to DIP/DA Mismatch Error."},
    ["SIP_SA_MISMATCH"]=    {value="CPSS_DXCH_IP_DROP_CNT_SIP_SA_MISMATCH_MODE_E",
                                                  help="Count packets dropped due to SIP/SA Mismatch Error."},
    ["ILLEGAL_ADDRESS"]=    {value="CPSS_DXCH_IP_DROP_CNT_ILLEGAL_ADDRESS_MODE_E",
                                                  help="Count packets dropped due to Illegal Address Error."},
    ["CNT_UC_RPF"]=         {value="CPSS_DXCH_IP_DROP_CNT_UC_RPF_MODE_E",
                                                  help="Count packets dropped due to Unicast RPF Fail."},
    ["CNT_MC_RPF"]=         {value="CPSS_DXCH_IP_DROP_CNT_MC_RPF_MODE_E",
                                                  help="Count packets dropped due to Multicast RPF Fail."},
    ["TTL_HOP_LIMIT_EXCEEDED"]=  {value="CPSS_DXCH_IP_DROP_CNT_TTL_HOP_LIMIT_EXCEEDED_MODE_E",
                                                  help="Count packets dropped due to IPv4 TTL or IPv6 Hop Limit Exceeded."},
    ["MTU_EXCEEDED"]=       {value="CPSS_DXCH_IP_DROP_CNT_MTU_EXCEEDED_MODE_E",
                                                  help="Count packets dropped due to MTU Exceeded."},
    ["CNT_OPTION"]=         {value="CPSS_DXCH_IP_DROP_CNT_OPTION_MODE_E",
                                                  help="Count packets dropped due to IPv4 Options or IPv6 Hop-by-Hop Option."},
    ["IPV6_SCOPE"]=         {value="CPSS_DXCH_IP_DROP_CNT_IPV6_SCOPE_MODE_E",
                                                  help="Count packets dropped due to IPv6 Scope Exception."},
    ["CNT_UC_SIP_FILTER"]=  {value="CPSS_DXCH_IP_DROP_CNT_UC_SIP_FILTER_MODE_E",
                                                  help="Count packets dropped due to Unicast Packet SIP Filter."},
    ["CNT_NH_CMD"]=         {value="CPSS_DXCH_IP_DROP_CNT_NH_CMD_MODE_E",
                                                  help="Count packets dropped due to Packet Dropped due to next hop command SoftDrop or HardDrop."},
    ["CNT_ACCESS_MATRIX"]=  {value="CPSS_DXCH_IP_DROP_CNT_ACCESS_MATRIX_MODE_E",
                                                  help="Count packets dropped due to Access Matrix Filter."},
    ["SIP_ALL_ZEROS"]=      {value="CPSS_DXCH_IP_DROP_CNT_SIP_ALL_ZEROS_MODE_E",
                                                  help="Count packets dropped due to SIP all zeros Error."}   
    }
}



