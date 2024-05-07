--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* switchport_protected_test.lua
--*
--* DESCRIPTION:
--*       The test of  switchport protected command.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

-- 60 bytes packet with VLAN tag vid=5
local packet_60b =
    "001112131415002122232425"..
    "810000053333999900000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"

-- run a configuration
local ret
local configCommands=[[
end
configure
interface range vlan device ${dev} vid 5
exit
interface range ethernet ${dev}/${port[1]},${port[2]},${port[3]}
switchport allowed vlan add 5 tagged
exit
interface ethernet ${dev}/${port[1]}
switchport protected ethernet ${dev}/${port[3]}
exit
]]
ret = pcall(executeStringCliCommands, configCommands)
if (ret==false) then
    setFailState()
end

local transmitInfo
local egressInfoTable

--send packet should be redirected to port[3]
transmitInfo = {
    devNum = devNum,
    portNum = port1 ,
    pktInfo = {fullPacket = packet_60b}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2, packetCount = 0 },
    {portNum = port3, packetCount = 1 },
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

-- run a deconfiguration part1
local deconfigCommands1=[[
end
configure
interface ethernet ${dev}/${port[1]}
no switchport protected
exit
]]
ret = pcall(executeStringCliCommands, deconfigCommands1)
if (ret==false) then
    setFailState()
end

--send packet should be fluded to port[2]
transmitInfo = {
    devNum = devNum,
    portNum = port1 ,
    pktInfo = {fullPacket = packet_60b}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2, packetCount = 1 },
    {portNum = port3, packetCount = 1 },
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end


-- run a deconfiguration part2
local deconfigCommands2=[[
end
configure
no interface range vlan device ${dev} vid 5
exit
]]
ret = pcall(executeStringCliCommands, deconfigCommands2)
if (ret==false) then
    setFailState()
end


