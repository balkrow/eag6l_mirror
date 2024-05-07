--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ipv6_ecmp_info.lua
--*
--* DESCRIPTION:
--*       showing of the ECMP configuration for configured IP v6 Routes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
require("dxCh/exec/show_ip_ecmp_info")

--constants


--------------------------------------------------------------------------------
-- command registration: show ipv6 ecmp info
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show ipv6 ecmp_info", "Ipv6 ECMP Information")
CLI_addCommand("exec", "show ipv6 ecmp_info", {
  func   = function(params)
               params.ip_protocol = "CPSS_IP_PROTOCOL_IPV6_E"
               return show_ip_ecmp_info_func(params)
           end,
  help   = "Current information of the ipv6 ECMP entries in LPM and FDB prefixes ",
  params = {
        {
            type = "named",
             "#all_device",
			mandatory = { "all_device" },
			{ format = "ecmp_id  %GT_U32", name = "ecmp_id", help = "Show all the LPM entries and FDB entries with ecmp_id" },
            { format = "distinct", name = "distinct", help = "Filter ECMP NH duplication" },          
        }
    }
})