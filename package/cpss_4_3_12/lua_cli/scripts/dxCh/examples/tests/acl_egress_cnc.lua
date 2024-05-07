--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* acl_egress_cnc.lua
--*
--* DESCRIPTION:
--*       The test of CNC counters for EPCL client.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devNum, "SIP_5")
--EPCL in BOBCAT3 mapped to group not served by TCAM floor0
--CNC indexes used in PCL rules are equal to rule indexes in TCAM
NOT_SUPPORTED_DEV_DECLARE(devNum, "CPSS_PP_FAMILY_DXCH_BOBCAT3_E")

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

-- 60 bytes packet with 2 VLAN tags vid=5
-- the VLAN tags will be removed to 52 byte packet by HA
-- the packet will be zero-padded by HA to 60 byte again
-- the CNC counters must show 60 bytes
local packet_60b =
    "001112131415002122232425"..
    "810000058100000533339999"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"

local transmitInfo
local egressInfoTable

-- run a configuration
local ret
local confidFile = "dxCh/examples/configurations/acl_egress_cnc.txt"
ret = pcall(executeLocalConfig, confidFile)
if (ret==false) then
    setFailState()
end

--clean CNC value - assume clear on read
local epclBlocksList = getCNCBlock("EPCL")
if type(epclBlocksList) ~= "table" then
    printLog ("Wrong client name EPCL for selected device")
    setFailState()
end
if type(epclBlocksList) == "table" then
    local blockNum = epclBlocksList[1]
    local ret, val = myGenWrapper("cpssDxChCncCounterGet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "blockNum", blockNum },
        { "IN", "GT_U32", "index", 18 },
        { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
        { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr"}
    })

    if ret ~= 0 then
        printLog("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
        setFailState()
    end
end

--send packet 60+4(crc) bytes - should pass

transmitInfo = {
    devNum = devNum,
    portNum = port1 ,
    pktInfo = {fullPacket = (packet_60b)}
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

--check CNC value
local epclBlocksList = getCNCBlock("EPCL")
if type(epclBlocksList) ~= "table" then
	printLog ("Wrong client name EPCL for selected device")
    setFailState()
end
if type(epclBlocksList) == "table" then
	local blockNum = epclBlocksList[1]
	local ret, val = myGenWrapper("cpssDxChCncCounterGet", {
		{ "IN", "GT_U8", "devNum", devNum },
		{ "IN", "GT_U32", "blockNum", blockNum },
		{ "IN", "GT_U32", "index", 18 },
		{ "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
		{ "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
	})

	if ret == 0 then
		local counterPtr = val["counterPtr"]
		local byteCounterLSB = counterPtr["byteCount"]["l"][0]
		printLog("EPCL CNC byte counter: " .. byteCounterLSB)
		if byteCounterLSB < 60 then
			printLog("ERROR - zero padding not included in CNC byte counter: " .. byteCounterLSB)
			setFailState()
		end
	else
		printLog("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
		setFailState()
	end
end


-- run a deconfiguration
local confidFile = "dxCh/examples/configurations/acl_egress_cnc_deconfig.txt"
ret = pcall(executeLocalConfig, confidFile)
if (ret==false) then
    setFailState()
end

