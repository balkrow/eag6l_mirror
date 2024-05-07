--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ipv6_route_fdb.lua
--*
--* DESCRIPTION:
--*       showing of the global status of IP routing and the configured IPv6 Routes
--*       in FDB
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
require("dxCh/exec/show_ip_route_fdb")

--constants


--------------------------------------------------------------------------------
-- command registration: show ipv6 ecmp info
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show ipv6 route_fdb", "Ipv6 FDB Information")
CLI_addCommand("exec", "show ipv6 route_fdb", {
  func   = function(params)
               params.ip_protocol = "CPSS_IP_PROTOCOL_IPV6_E"
               return show_ip_route_fdb_func(params)
           end,
  help   = "Current information of the ipv6 entries in FDB table ",
  params = {
      { type = "named",
          "#all_device",
      }
  }
})