--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_mti_test.lua
--*
--* DESCRIPTION:
--*       The test of  VLAN MTI.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devNum,"SIP_5")

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

local transmitInfo
local egressInfoTable

-- run a configuration
local ret
local configCommands=[[
end
configure
interface range vlan device ${dev} vid 5
exit
interface range ethernet ${dev}/${port[1]},${port[2]}
switchport allowed vlan add 5 tagged
exit
interface range vlan device ${dev} vid 5
vlan-mtu 64
exit
]]
ret = pcall(executeStringCliCommands, configCommands)
if (ret==false) then
    setFailState()
end

--send packet 60+4(crc) bytes - should pass

transmitInfo = {
    devNum = devNum,
    portNum = port1 ,
    pktInfo = {fullPacket = (packet_60b .. "")}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port2, packetCount = 1 },
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

--send packet 68+4(crc) bytes - should drop

transmitInfo = {
    devNum = devNum,
    portNum = port1 ,
    pktInfo = {fullPacket = (packet_60b .. "1122334455667788")}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port2, packetCount = 0 },
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

-- run a deconfiguration
local deconfigCommands=[[
end
configure
interface range vlan device ${dev} vid 5
no vlan-mtu
exit
no interface range vlan device ${dev} vid 5
exit
]]
ret = pcall(executeStringCliCommands, deconfigCommands)
if (ret==false) then
    setFailState()
end


