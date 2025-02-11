--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* acl_mirror_to_cpu.lua
--*
--* DESCRIPTION:
--*       The test for ACL Mirror to CPU feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--
-- ACL matched and not matched packets
--
local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local aclNotMatchContent =
    "0000BBCCDDEE000000003402"..
    "333300000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"

-- MAC DA updated to be unique
local aclMatchContent =
    "00A1BBCCDDEE000000003402"..
    "333300000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"

local test_scenario =
{
    no_matc =
    {
        transmitInfo =
        {
          devNum = devNum,
          portNum = port3, pktInfo = {fullPacket = aclNotMatchContent}
        },
        egressInfoTable =
        {
            {portNum = port1, pktInfo = {fullPacket = aclNotMatchContent}, packetCount = 1},
            {portNum = port2, packetCount = 1},
            {portNum = port4, packetCount = 1},
        }
    },
    matc =
    {
        transmitInfo =
        {
          devNum = devNum,
          portNum = port3, pktInfo = {fullPacket = aclMatchContent}
        },
        egressInfoTable =
        {
            {portNum = "CPU", pktInfo = {fullPacket = aclMatchContent, cpuCodeNumber = 195}},
            {portNum = port1, pktInfo = {fullPacket = aclMatchContent}, packetCount = 1},
            {portNum = port2, packetCount = 1},
            {portNum = port4, packetCount = 1},
        }
    }
}

-- local declaration
local configFile   = "dxCh/examples/configurations/acl_mirror_to_cpu.txt"
local deconfigFile = "dxCh/examples/configurations/acl_mirror_to_cpu_deconfig.txt"

local function trafficCheck(trafficScenario)
    local key, scenario;
    for key, scenario in pairs(trafficScenario) do
        print("========================================")
            print("Send a packet #".. tostring(key))
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           scenario.transmitInfo, scenario.egressInfoTable);
        assert(
            rc==0, string.format("packet transmitting error: %s",
            returnCodes[rc]))
    end
end

------------------------------ MAIN PART ---------------------------------------
-- read the packets from the file
local isOk;
isOk = luaTgfSimpleTest(configFile, deconfigFile, trafficCheck, test_scenario)
