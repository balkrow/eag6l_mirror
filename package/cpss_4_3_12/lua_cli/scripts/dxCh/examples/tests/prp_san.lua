--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* prp_san.lua
--*
--* DESCRIPTION:
--*       The test for testing PRP-SAN
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local testName = "PRP-SAN"

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local portCPU = devEnv.portCPU -- cpu port

-- Supported in IronMan but not in AAS
SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_6_30", true, "SIP_7")


--test that need 'simulation log'
local ssl_test = nil--"ptp-PDelay_request-A"
local sslw = true

-- get extra PTP util functions
-- use dofile instead of require .. to allow quick load of test
dofile("dxCh/examples/common/ptp_utils.lua")

-- note : ptp_tests_extra_info[] hold extra info about the tests
local ptp_test_info = {
    [1] = {skip = false ,doc_section = ""   ,ptp_step = "single step" ,clockType = ""},
    [2] = {skip = false ,doc_section = ""   ,ptp_step = "2 steps"     ,clockType = ""},
}

local skip_part_ptp = true

for ii = 1, #ptp_test_info do
    if not ptp_test_info[ii].skip then
        -- if any of the ptp tests not skipped , we not skip this part
        skip_part_ptp = false
    end
end -- end of loop on ptp_test_info


local devFamily = wrlCpssDeviceFamilyGet(devNum)
if devFamily == "CPSS_PP_FAMILY_DXCH_AAS_E" then
    -- the PTP manager call USX/CPU mac config but not support (yet) 100G/400G mac
    skip_part_ptp = true
end

local skip_part_1 = false
local skip_part_2 = false
local skip_part_3 = skip_part_ptp           -- The GM fixed the PCH issue


local test_ddeSameKeyExpirationTime
local inBetween_ddeSameKeyExpirationTime

local fromCpuEmbeddedVlanNotOk = false
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation")

if wrlCpssIsAsicSimulation() or isGmUsed() then
    -- allow to run ssl (simulation LOG)
    test_ddeSameKeyExpirationTime = 5000
    inBetween_ddeSameKeyExpirationTime = test_ddeSameKeyExpirationTime
elseif isEmulatorUsed() then
    test_ddeSameKeyExpirationTime = 100
    inBetween_ddeSameKeyExpirationTime = 2
else
    -- allow the entry to not be deleted too soon , so the next packet from same port or other port
    -- will still 'see' this entry as valid
    test_ddeSameKeyExpirationTime = 1000
    inBetween_ddeSameKeyExpirationTime = 2
end



local ePortPrpBase = 0x3f0 --[[decimal 1008]]

local function hportToEPort(hport)
    return ePortPrpBase + hport
end


-- physical port of PRP port A
global_test_data_env.PRP_port_A = port1
-- physical port of PRP port B
global_test_data_env.PRP_port_B = port2
-- physical port of interlink
global_test_data_env.interlink  = port3

global_test_data_env.CPU = portCPU

-- eport of PRP port A
global_test_data_env.ePort_PRP_port_A = hportToEPort(0)
-- eport of PRP port B
global_test_data_env.ePort_PRP_port_B = hportToEPort(1)
-- eport of Interlink
global_test_data_env.ePort_interlink = hportToEPort(2)
-- base eport
global_test_data_env.ePortPrpBase = ePortPrpBase

local remove_rct_bit = 7
local function srcIdForEport(eport,remove_rct)
    local srcId = eport - ePortPrpBase
    if(remove_rct and get_bit(srcId,remove_rct_bit,1) == 0) then
        -- need to remove rct
        srcId = srcId + 2^remove_rct_bit
    end

    return srcId
end

-- SRC-ID with indication of 'remove RCT' for ingress from PRP port A
global_test_data_env.remove_rct_src_id_PRP_port_A = srcIdForEport(global_test_data_env.ePort_PRP_port_A,true)
-- SRC-ID with indication of 'remove RCT' for ingress from PRP port B
global_test_data_env.remove_rct_src_id_PRP_port_B = srcIdForEport(global_test_data_env.ePort_PRP_port_B,true)


-- physical port of ring A
local PRP_port_A = global_test_data_env.PRP_port_A
-- physical port of ring B
local PRP_port_B = global_test_data_env.PRP_port_B
-- physical port of interlink
local interlink = global_test_data_env.interlink
-- other port that should not get traffic
local otherPort = port4

local hport_map = {
    [PRP_port_A] = global_test_data_env.ePort_PRP_port_A - ePortPrpBase,
    [PRP_port_B] = global_test_data_env.ePort_PRP_port_B - ePortPrpBase,
    [interlink] = global_test_data_env.ePort_interlink - ePortPrpBase
}


local vidxBase = 1024

-- the function return 4 numbers : lowest 32 bits , higher 32 bits , higher 32 bits , highest 32 bits
-- as lua round numbers when using greater than 100,000,000,000,000
-- https://www.lua.org/pil/2.3.html
local function power_2_number(number)
    if number < 32 then
        return 2^number , 0,0,0
    end
    if number < 64 then
        return 0 , 2^(number-32) , 0,0
    end
    if number < 96 then
        return 0 , 0 , 2^(number-64) , 0
    end
    if number < 128 then
        return 0 , 0 , 0, 2^(number-96)
    end
    -- not implemented for number >= 128
    return nil,nil,nil,nil
end

--local temp_C1,temp_C2 = power_2_number(portCPU) -- CPU
local temp_A1,temp_A2 = power_2_number(PRP_port_A)-- PRP_port_A
local temp_B1,temp_B2 = power_2_number(PRP_port_B)-- PRP_port_A
local temp_I1,temp_I2 = power_2_number(interlink)-- Interlink

function prpTest1VidxToTargetPortsBmp(vidx,wordIndex)
    local relativeVidx = vidx - vidxBase
    local results_1,results_2 = 0,0

    --[[ no CPU port support needed
    if relativeVidx >= 8 and relativeVidx <= 15 then
        relativeVidx = relativeVidx - 8
        results_1 = results_1 + temp_C1
        results_2 = results_2 + temp_C2
    end
    ]]

    if     relativeVidx == 0 then
        --
    elseif relativeVidx == 1 then
        results_1 = results_1 + temp_A1
        results_2 = results_2 + temp_A2
    elseif relativeVidx == 2 then
        results_1 = results_1 + temp_B1
        results_2 = results_2 + temp_B2
    elseif relativeVidx == 3 then
        results_1 = results_1 + temp_A1
        results_2 = results_2 + temp_A2
        results_1 = results_1 + temp_B1
        results_2 = results_2 + temp_B2
    elseif relativeVidx == 4 then
        results_1 = results_1 + temp_I1
        results_2 = results_2 + temp_I2
    elseif relativeVidx == 5 then
        results_1 = results_1 + temp_A1
        results_2 = results_2 + temp_A2
        results_1 = results_1 + temp_I1
        results_2 = results_2 + temp_I2
    elseif relativeVidx == 6 then
        results_1 = results_1 + temp_B1
        results_2 = results_2 + temp_B2
        results_1 = results_1 + temp_I1
        results_2 = results_2 + temp_I2
    elseif relativeVidx == 7 then
        results_1 = results_1 + temp_A1
        results_2 = results_2 + temp_A2
        results_1 = results_1 + temp_B1
        results_2 = results_2 + temp_B2
        results_1 = results_1 + temp_I1
        results_2 = results_2 + temp_I2
    else
        --
    end

    if wordIndex == 1 then
        return results_2
    end

    return results_1

end

local command_data = Command_Data()

local macA  = "001122334455"
local macB  = "00FFEEDDBBAA"
local macC  = "00987654321F"
local macA1 = "001122334456"

local macA2 = "001122334457"
local macB1 = "00FFEEDDBBAB"
local macC1 = "00987654321E"

local macCpu= "00ccc000abcd"
local macOtherCpu = "00ccc000abff"

local function makeMacAddrTestFormat (macAddr)
    local str = ""
    local newPart,offset,ii

    for ii = 1 , 6 do
        offset = (ii-1)*2
        newPart = string.sub(macAddr, 1 + offset, 2 + offset)
        if(str == "")then
            str = newPart
        else
            str = str .. ":" .. newPart
        end
    end

    return str
end


global_test_data_env.macB1 = makeMacAddrTestFormat(macB1)
global_test_data_env.macA2 = makeMacAddrTestFormat(macA2)
global_test_data_env.macC1 = makeMacAddrTestFormat(macC1)

local transmitInfo =
{
  devNum = devNum,
  pktInfo = {},
  inBetweenSendFunc = nil
}
local egressInfoTable


local payload_short =
"aaaabbbbccccddddeeeeffff00001111"..
"22223333444455556666777788889999"..
"a1a2a3a4b1b2b3b4c1c2c3c4d1d2d3d4"..
"e1e2e3e4f1f2f3f49192939481828384"


local payload_long =
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short

local prpSuffix = "88FB"

local   egress_ports_with_correction_field_changed = {
    [PRP_port_A] = { correction_field_updated = nil },
    [PRP_port_B] = { correction_field_updated = nil },
    [interlink]  = { correction_field_updated = nil },
}

local function buildPrpRct(SeqNr,lanId,lsduSize)
    if lanId == nil then lanId = 0 end
    local SeqNrStr          = string.format("%4.4x",SeqNr)      --16 bits
    local lanIdStr      = string.format("%1.1x",lanId)      -- 4 bits
    local lsduSizeStr    = string.format("%3.3x",lsduSize)   --12 bits
    return SeqNrStr .. lanIdStr .. lsduSizeStr .. prpSuffix
end

local function buildVlanTag(vlanId,cfi,vpt)
    local vidStr            = string.format("%3.3x",vlanId)     --12 bits
    local cfiVptStr         = string.format("%1.1x",vpt*2+cfi)  -- 4 bits
    return "8100" .. cfiVptStr .. vidStr
end


--[[
build packet info:
     l2 = {srcMac = , dstMac = }
    ,prpRct = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
    ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
    payload =
    totalPacketSize =
    padding =

    return :
    1. the string of the packet
    2. the 'mustNotBeEqualBytesTable' if not nil , indicate what bytes must not egress as ingress
    3. the ptpStartOffset is offset from start of packet where the PTP header starts
]]
local function buildPacket(info)
    local tempPrpTrailer = ""
    local tempVlanTag = ""
    local ptpHeader = ""
    local my_mustNotBeEqualBytesTable = nil
    local ptpStartOffset = 0

    if  info.prpRct and info.prpRct.exists then
        local prpRct = info.prpRct
        tempPrpTrailer = buildPrpRct(prpRct.SeqNr,prpRct.lanId,prpRct.lsduSize)
    end

    if  info.vlanTag and info.vlanTag.exists then
        local vlanTag = info.vlanTag
        tempVlanTag = buildVlanTag(vlanTag.vlanId,vlanTag.cfi,vlanTag.vpt)
    end

    if info.PtpInfo then
        ptpStartOffset = string.len(info.l2.dstMac .. info.l2.srcMac .. tempVlanTag) / 2
        ptpHeader,my_mustNotBeEqualBytesTable = buildPtpTag(info.PtpInfo , string.len(info.payload) / 2--[[size of payload]] , ptpStartOffset)
    end

    local totalPacket =  info.l2.dstMac .. info.l2.srcMac .. tempVlanTag .. ptpHeader .. info.payload .. tempPrpTrailer

    if (info.totalPacketSize ~= nil) then
        local currTotalLen   = string.len(totalPacket)  / 2
        local currPayloadLen = string.len(info.payload) / 2

        local numBytesRemovedFromPayload = currTotalLen - info.totalPacketSize

        local paddingBytes = ""
        if info.padding ~= nil and info.padding ~= 0 then
            for index = 1, info.padding do
                paddingBytes = paddingBytes .. "00"
            end
        end

        --update the lsduSize if relevant
        if  info.prpRct and info.prpRct.exists then
            local prpRct = info.prpRct
            local lsduSize = prpRct.lsduSize - numBytesRemovedFromPayload
            if info.padding ~= nil then
                lsduSize = lsduSize + info.padding
            end

            tempPrpTrailer = buildPrpRct(prpRct.SeqNr,prpRct.lanId,lsduSize)
        end

        -- we need tp trim the packet
        totalPacket = info.l2.dstMac .. info.l2.srcMac .. tempVlanTag ..
            string.sub(info.payload, 1 , (currPayloadLen - numBytesRemovedFromPayload) * 2) .. paddingBytes .. tempPrpTrailer
    end

    return totalPacket
           ,my_mustNotBeEqualBytesTable , ptpStartOffset
end

local function calcLsduSize(payload,PtpInfo)
    local ptpSize = 0
    if(PtpInfo and ptp_messageTypes[PtpInfo.messageType]) then
        ptpSize = 2 --[[ethertype of PTP]] + ptp_messageTypes[PtpInfo.messageType].length
    end

    -- 4 and not 6 because without the 2 bytes of the ethertype
    local LsduSize = 4 + (string.len(payload) / 2)
    return LsduSize + ptpSize
end

local function clearDDE(index)
    if (isEmulatorUsed()) then
        printLog("Emulator : allow cache of DDE to clean (before CPU invalidated the entry)  --> sleep 2000 milisec")
        delay(2000)
    end

    local apiName = "cpssDxChHsrPrpDdeEntryInvalidate"

    printLog("clean DDE index " .. index)

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U32",    "index",    index }
    })

end

-- delete the DDE entry for MAC-A with seqNum = 1000
-- delete before changing the partition mode
local function clearDDE_20736()
    clearDDE(20736)
end


local function treatWrongLanIdAsRctExistsSet(portNum,enable)
    local apiName = "cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet"

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",    "GT_U8",         "devNum"    ,    devNum },
        { "IN",    "GT_PORT_NUM",   "portNum"   ,    portNum },
        { "IN",    "GT_BOOL",       "enable"    ,    enable},
    })
end

local function onPortA_B_treatWrongLanIdAsRctExistsSet(enable)
    treatWrongLanIdAsRctExistsSet(PRP_port_A,enable)
    treatWrongLanIdAsRctExistsSet(PRP_port_B,enable)
end

local function setDsaFromCpuTrgVlanTagged(valid)
    local isValid = valid and "1" or "0"
    --set/unset to egress with vlan tag as indication from the DSA tag and not as 'payload' after the DSA
    executeStringCliCommands("do shell-execute prvTgfTrafficForceVlanTagInfo " .. isValid .. ",0,0,1")
--[[GT_STATUS prvTgfTrafficForceVlanTagInfo(
    GT_U32         valid,
    GT_U32         vpt,
    GT_U32         cfiBit,
    GT_U32         vid
);]]
end

-- delete the DDE entry for MAC-C1 with seqNum = 5003
-- delete before changing the partition mode
local function clearDDE_5504()
    clearDDE(5504)
end

-- we need to capture the egress packet that loopback from the port to the CPU
-- use iPCL rule to trap it
local function forceCaptureOnTransmitPortSet(force)

    if force then
        executeStringCliCommands(
        [[
            do configure

            access-list device ${dev} pcl-ID 10
            rule-id 0 action trap-to-cpu
            exit

            access-list device ${dev} pcl-ID 11
            rule-id 6 action trap-to-cpu
            exit

            access-list device ${dev} pcl-ID 12
            rule-id 12 action trap-to-cpu
            exit

            exit
        ]]
        )
    else
        executeStringCliCommands(
        [[
            do configure

            access-list device ${dev} pcl-ID 10
            delete rule-id 0
            exit

            access-list device ${dev} pcl-ID 11
            delete rule-id 6
            exit

            access-list device ${dev} pcl-ID 12
            delete rule-id 12
            exit

            exit
        ]]
        )
    end

end

--

-- info accordint to :
-- https://sp.marvell.com/sites/EBUSites/Switching/Architecture/Shared%20Documents%20-%20arch%20sharepoint/Projects/Ironman/Verification%20use%20cases/HSR%20and%20PRP%20Use%20Cases.pdf?csf=1

local incomingPacketsDb =
{
    {packetNum = "1-1" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = true  , remark = "New address (SeqNr=0)"}
   ,{packetNum = "1-2" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false , remark = "Existing address (SeqNr=1)"}
   ,{packetNum = "1-3" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = true  , remark = "Existing address (SeqNr=2)"}
   ,{packetNum = "1-4" , srcPort = PRP_port_A, srcMac = macB , dstMac = macC , SeqNr = 1000, prpRct = true  , vlanTag = true  , remark = "First packet from Ring", inBetweenSendFunc = clearDDE_20736}
   ,{packetNum = "1-5" , srcPort = PRP_port_A, srcMac = macB , dstMac = macC , SeqNr = 1000, prpRct = true  , vlanTag = true  , remark = "Same packet from same port"}
   ,{packetNum = "1-6" , srcPort = PRP_port_B, srcMac = macB , dstMac = macC , SeqNr = 1000, prpRct = true  , vlanTag = true  , remark = "Same packet from other port"}
   ,{packetNum = "1-7" , srcPort = interlink , srcMac = macA1, dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = true  , remark = "No ready entry"}
   ,{packetNum = "1-8-A1" , srcPort = PRP_port_A , srcMac = macB, dstMac = macC , SeqNr = 2000 , prpRct = true , vlanTag = true  , remark = "wrong LAN-ID treat as RCT (first from port A)",
    wrongLanId = true,
    preTestFunc  = onPortA_B_treatWrongLanIdAsRctExistsSet , preTestParams = true  ,
    postTestFunc = onPortA_B_treatWrongLanIdAsRctExistsSet , postTestParams = false}
   ,{packetNum = "1-8-A2" , srcPort = PRP_port_B , srcMac = macB, dstMac = macC , SeqNr = 2000 , prpRct = true , vlanTag = true  , remark = "wrong LAN-ID treat as RCT (second from port B so dropped)",
    wrongLanId = true,
    preTestFunc  = onPortA_B_treatWrongLanIdAsRctExistsSet , preTestParams = true  ,
    postTestFunc = onPortA_B_treatWrongLanIdAsRctExistsSet , postTestParams = false}
   ,{packetNum = "1-8-B1" , srcPort = PRP_port_A , srcMac = macB, dstMac = macC , SeqNr = 2001 , prpRct = true , vlanTag = true  , remark = "wrong LAN-ID not treat as RCT (first from port A)",
    wrongLanId = true}
   ,{packetNum = "1-8-B2" , srcPort = PRP_port_B , srcMac = macB, dstMac = macC , SeqNr = 2001 , prpRct = true , vlanTag = true  , remark = "wrong LAN-ID not treat as RCT (second from port B allowed and not dropped)",
    wrongLanId = true}
   ,{packetNum = "1-9-1" , srcPort = PRP_port_A , srcMac = macB, dstMac = macC , SeqNr = 2002 , prpRct = true , vlanTag = true  , remark = "wrong LSDU size not treat as RCT (first from port A)",
    wrongLsduSize = true
    }
   ,{packetNum = "1-9-2" , srcPort = PRP_port_B , srcMac = macB, dstMac = macC , SeqNr = 2002 , prpRct = true , vlanTag = true  , remark = "wrong LSDU size not treat as RCT (second from port B allowed and not dropped)",
    wrongLsduSize = true
    }
   ,{packetNum = "1-10-1" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = false , remark = "padding test",
    totalPacketSize = 64 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-2" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 64 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-3" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 65 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-4" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 66 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-5" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 67 - 4 --[[CRC]]
    }
   ,{packetNum = "1-10-6" , srcPort = interlink , srcMac = macA, dstMac = macC , SeqNr = "-"  , prpRct = false , vlanTag = true  , remark = "padding test",
    totalPacketSize = 68 - 4 --[[CRC]]
    }


}

local outgoingPacketsDb =
{
    ["1-1"] = {
        {trgPort = PRP_port_A , SeqNr = 0 , prpRct = true , vlanTag = true }
       ,{trgPort = PRP_port_B , SeqNr = 1 , prpRct = true , vlanTag = true }
    }
   ,["1-2"] = {
        {trgPort = PRP_port_A , SeqNr = 2 , prpRct = true , vlanTag = false}
       ,{trgPort = PRP_port_B , SeqNr = 3 , prpRct = true , vlanTag = false}
    }
   ,["1-3"] = {
        {trgPort = PRP_port_A , SeqNr = 4 , prpRct = true , vlanTag = true}
       ,{trgPort = PRP_port_B , SeqNr = 5 , prpRct = true , vlanTag = true}
    }
   ,["1-4"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true}
    }
   ,["1-5"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true}
    }
   ,["1-6"] = {
        {trgPort = interlink  , notEgress = true}
       ,{trgPort = PRP_port_A , notEgress = true}
    }
   ,["1-7"] = {
        {trgPort = PRP_port_A , notEgress = true }
       ,{trgPort = PRP_port_B , notEgress = true }
    }
   ,["1-8-A1"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true }
    }
   ,["1-8-A2"] = {
        {trgPort = interlink  , notEgress = true}
       ,{trgPort = PRP_port_A , notEgress = true}
    }
   ,["1-8-B1"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true }
    }
   ,["1-8-B2"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_A , notEgress = true }
    }
   ,["1-9-1"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_B , notEgress = true }
    }
   ,["1-9-2"] = {
        {trgPort = interlink  , SeqNr ="-", prpRct = false , vlanTag = true}
       ,{trgPort = PRP_port_A , notEgress = true }
    }
   ,["1-10-1"] = {
        {trgPort = PRP_port_A , SeqNr = 6 , prpRct = true , vlanTag = false , padding = 0 , totalPacketSize = 64 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-2"] = {
        {trgPort = PRP_port_A , SeqNr = 7 , prpRct = true , vlanTag = true  , padding = 4 , totalPacketSize = 64 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-3"] = {
        {trgPort = PRP_port_A , SeqNr = 8 , prpRct = true , vlanTag = true  , padding = 3 , totalPacketSize = 65 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-4"] = {
        {trgPort = PRP_port_A , SeqNr = 9 , prpRct = true , vlanTag = true  , padding = 2 , totalPacketSize = 66 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-5"] = {
        {trgPort = PRP_port_A , SeqNr = 10, prpRct = true , vlanTag = true  , padding = 1 , totalPacketSize = 67 + 6--[[RCT]] - 4 --[[CRC]]}}
   ,["1-10-6"] = {
        {trgPort = PRP_port_A , SeqNr = 11, prpRct = true , vlanTag = true  , padding = 0 , totalPacketSize = 68 + 6--[[RCT]] - 4 --[[CRC]]}}


}


local incomingPacketsDb_toFromCpu = {
    {packetNum = "5-1" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = " From_CPU to PRP network"},
    {packetNum = "5-2" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = " From_CPU to PRP network (vlan tag from <trgIsTagged>=1)",
    preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   ,
    hiddenRxBytes = 4
    },
    {packetNum = "5-3" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = 5000 , prpRct = true , vlanTag = false  , trgPort = PRP_port_B , remark = " From_CPU to Ring B with PRP RCT"},
    {packetNum = "5-4" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = 5000 , prpRct = true , vlanTag = false  , trgPort = PRP_port_B , remark = " From_CPU to Ring A (vlan tag from <trgIsTagged>=1)",
    preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   ,
    hiddenRxBytes = 4
    },
   {packetNum = "5-5" , srcPort = PRP_port_A     , srcMac = macB , dstMac = macB1 , SeqNr = 5002, prpRct = true  , vlanTag = true  , remark = "No DSA - Trapped to CPU",
   },
   {packetNum = "5-6" , srcPort = PRP_port_B     , srcMac = macB , dstMac = macB1 , SeqNr = 5002, prpRct = true  , vlanTag = true  , remark = "No DSA - Trapped to CPU",
   },
   {packetNum = "5-7" , srcPort = PRP_port_A    , srcMac = macB , dstMac = macC1 , SeqNr = 5003, prpRct = true  , vlanTag = false  , remark = "No DSA - Mirrored to CPU",
    inBetweenSendFunc = clearDDE_5504,
    --postTestFunc = showDde , postTestParams = nil ,
   },
   {packetNum = "5-8" , srcPort = PRP_port_B    , srcMac = macB , dstMac = macC1 , SeqNr = 5003, prpRct = true  , vlanTag = false  , remark = "No DSA - Mirrored to CPU",
    inBetweenSendFunc = clearDDE_5504 , force_no_change_timeout = true
   },
}

local outgoingPacketsDb_toFromCpu = {
    ["5-1"] = {
        {trgPort = PRP_port_A  , SeqNr = 0 , prpRct = false , vlanTag = false},
        {trgPort = PRP_port_B  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-2"] = {
        {trgPort = PRP_port_A  , SeqNr = 0 , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-3"] = {
        {trgPort = PRP_port_B  , SeqNr = 5000 , prpRct = true , vlanTag = false},
        {trgPort = PRP_port_A  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-4"] = {
        {trgPort = PRP_port_B  , SeqNr = 5000 , prpRct = true , vlanTag = true},
        {trgPort = PRP_port_A  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-5"] = {
        {trgPort = portCPU , SeqNr = 5002 , prpRct = true , vlanTag = true},
        {trgPort = PRP_port_B   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-6"] = {
        {trgPort = portCPU , SeqNr = 5002 , prpRct = true , vlanTag = true},
        {trgPort = PRP_port_A   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-7"] = {
        {trgPort = portCPU , SeqNr = 5003 , prpRct = true , vlanTag = false},
        {trgPort = PRP_port_B   , notEgress = true},
        {trgPort = interlink, prpRct = false , vlanTag = false}
    },
    ["5-8"] = {
        {trgPort = portCPU , SeqNr = 5003 , prpRct = true , vlanTag = false},
        {trgPort = PRP_port_A   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
}

if fromCpuEmbeddedVlanNotOk then
    outgoingPacketsDb_toFromCpu["5-2"][1].vlanTag =  false
    outgoingPacketsDb_toFromCpu["5-4"][1].vlanTag =  false
end

local function preTestFromCpuWithVlanTagAndCorrectionTime(trgPort)
    -- we expect the packet that switched from CPU to Port A will update the <correction field> (in addition to <origin timestamp>)
    -- to hold the extra time from the CSTU till the MAC
    egress_ports_with_correction_field_changed[trgPort].correction_field_updated = true
    setDsaFromCpuTrgVlanTagged(true) --vlan tag embedded in the 'FROM_CPU'
end

-- the PTP packets in the PRP system are NOT using the PRP identification (RCT)
local incomingPacketsDb_ptp_no_RCT = {
    {packetNum = "ptp-Sync-A" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = "PTP 'Synch' (PTP header) from CPU to Port A (without RCT)",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB2D},
        preTestFunc = preTestFromCpuWithVlanTagAndCorrectionTime , preTestParams = PRP_port_A,
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-Sync-B" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_B , remark = "PTP 'Synch' (PTP header) from CPU to Port B (without RCT)",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xAB3D},
        preTestFunc = preTestFromCpuWithVlanTagAndCorrectionTime , preTestParams = PRP_port_B,
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_request-A" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = "PTP 'PDelay_request' (PTP header) from Port A (without RCT) that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 0,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB4E},
        preTestFunc = setDsaFromCpuTrgVlanTagged , preTestParams = true,--vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_request-B" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_B , remark = "PTP 'PDelay_request' (PTP header) from Port B (without RCT) that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 0,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xAB5E},
        preTestFunc = setDsaFromCpuTrgVlanTagged , preTestParams = true,--vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-A" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = "PTP 'PDelay_Response' (PTP header) from CPU that egress Port A (without RCT)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB6E},
        preTestFunc = preTestFromCpuWithVlanTagAndCorrectionTime , preTestParams = PRP_port_A,
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-B" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_B , remark = "PTP 'PDelay_Response' (PTP header) from CPU that egress Port B (without RCT)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xAB7E},
        preTestFunc = preTestFromCpuWithVlanTagAndCorrectionTime , preTestParams = PRP_port_B,
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
}
local outgoingPacketsDb_ptp_no_RCT = {
    ["ptp-Sync-A"] = {
        {trgPort = PRP_port_A  , PtpInfo = { messageType = "Sync"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-Sync-B"] = {
        {trgPort = PRP_port_A  , notEgress = true},
        {trgPort = PRP_port_B  , PtpInfo = { messageType = "Sync"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_request-A"] = {
        {trgPort = PRP_port_A    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_request-B"] = {
        {trgPort = PRP_port_A    , notEgress = true},
        {trgPort = PRP_port_B    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-A"] = {
        {trgPort = PRP_port_A  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-B"] = {
        {trgPort = PRP_port_A    , notEgress = true},
        {trgPort = PRP_port_B  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
}

local incomingPacketsDb_ptp_no_RCT_2_steps = {
    {packetNum = "ptp-Sync-A" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = "PTP 'Synch' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB8D},
     postTestFunc = ptpPrepareFollowUpTodInfo_2_steps ,
     postTestParams = {trgPort=PRP_port_A,domainId=2,messageType="Sync",sequenceId=0xAB8D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-Follow_up-A" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = "PTP 'Follow-up' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "Follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB8D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-Sync-B" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_B , remark = "PTP 'Synch' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xAB9D},
     postTestFunc = ptpPrepareFollowUpTodInfo_2_steps ,
     postTestParams = {trgPort=PRP_port_B,domainId=1,messageType="Sync",sequenceId=0xAB9D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-Follow_up-B" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = "-" , prpRct = false , vlanTag = false  , trgPort = PRP_port_B , remark = "PTP 'Follow-up' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring B (with HSR tag)",
         PtpInfo = {messageType = "Follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB9D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_request-A" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring A that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xA1CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_request-B" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_B , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring B that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 0,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xA2CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-A" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = "PTP 'PDelay_Response' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xA3CE},
     postTestFunc = ptpPrepareFollowUpTodInfo_2_steps ,
     postTestParams = {trgPort=PRP_port_A,domainId=1,messageType="PDelay_Response",sequenceId=0xA3CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response_follow_up-A" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_A , remark = "PTP 'PDelay_Response_follow_up' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response_follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xA3CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-B" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_B , remark = "PTP 'PDelay_Response' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring B (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xA4CE},
     postTestFunc = ptpPrepareFollowUpTodInfo_2_steps ,
     postTestParams = {trgPort=PRP_port_B,domainId=1,messageType="PDelay_Response",sequenceId=0xA4CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response_follow_up-B" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = "-", prpRct = false , vlanTag = false  , trgPort = PRP_port_B , remark = "PTP 'PDelay_Response_follow_up' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring B (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response_follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xA4CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
}
local outgoingPacketsDb_ptp_no_RCT_2_steps = {
    ["ptp-Sync-A"] = {
        {trgPort = PRP_port_A  , PtpInfo = { messageType = "Sync"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-Follow_up-A"] = {
        {trgPort = PRP_port_A  , PtpInfo = { messageType = "Follow_up"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-Sync-B"] = {
        {trgPort = PRP_port_A  , notEgress = true},
        {trgPort = PRP_port_B  , PtpInfo = { messageType = "Sync"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-Follow_up-B"] = {
        {trgPort = PRP_port_A  , notEgress = true},
        {trgPort = PRP_port_B  , PtpInfo = { messageType = "Follow_up"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_request-A"] = {
        {trgPort = PRP_port_A    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_request-B"] = {
        {trgPort = PRP_port_A    , notEgress = true},
        {trgPort = PRP_port_B    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-A"] = {
        {trgPort = PRP_port_A  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response_follow_up-A"] = {
        {trgPort = PRP_port_A  , PtpInfo = { messageType = "PDelay_Response_follow_up"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = PRP_port_B    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-B"] = {
        {trgPort = PRP_port_A    , notEgress = true},
        {trgPort = PRP_port_B  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response_follow_up-B"] = {
        {trgPort = PRP_port_A    , notEgress = true},
        {trgPort = PRP_port_B  , PtpInfo = { messageType = "PDelay_Response_follow_up"} , SeqNr = "-"   , prpRct = false , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
}


local lanIdMap = {[PRP_port_A] = 0xA , [PRP_port_B] = 0xB}
local functionalityMap = {[PRP_port_A] = "PRP_port_A" , [PRP_port_B] = "PRP_port_B" , [interlink] = "interlink" , ["CPU"] = portCPU }
local packetCounter = 0

local function doFunction (func , params)

    if(not func)then return end

    if( type(func) == "table") then
        for _dummy, entry in pairs(func) do
            entry.testFunc(entry.testParams)
        end
    else
        func(params)
    end
end

local function doMainLogic(IN_Packets,OUT_Packets , catagory)
    for _dummy1, ingressInfo in pairs(IN_Packets) do
        local caseName = "Send a packet #".. ingressInfo.packetNum .. " , note : ".. ingressInfo.remark

        if catagory then
            caseName = catagory .. ": " .. caseName
        end
        printLog("========================================")
        printLog(caseName)

        ingressLanId = lanIdMap[ingressInfo.srcPort]

        if(1 == (packetCounter % 2)) then
            ingressPayload = payload_short
        else
            ingressPayload = payload_long
        end
        packetCounter = packetCounter + 1

        local ingressLsduSize = calcLsduSize(ingressPayload,ingressInfo.PtpInfo--[[only PtpInfo.messageType needed]])

        if ingressInfo.wrongLsduSize then
            ingressLsduSize = ingressLsduSize + 0x330
        end

        if ingressInfo.wrongLanId then
            ingressLanId = ingressLanId + 2 --{0xA --> 0xC} , {0xB --> 0xD}
        end

        transmitInfo.portNum            = ingressInfo.srcPort

        local forceCaptureOnTransmitPort = false

        if (ingressInfo.srcPort == portCPU) then
            -- we send from the CPU to 'target port'
            transmitInfo.portNum            = ingressInfo.trgPort
            if not ingressInfo.noForceCapture then
                forceCaptureOnTransmitPort      = true
            end
        end



    --[[
    build packet info:
         l2 = {srcMac = , dstMac = }
        ,prpRct = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
        ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
        payload =
        totalPacketSize =
    ]]
        local packetInfo = {
            l2 = {srcMac = ingressInfo.srcMac, dstMac = ingressInfo.dstMac}
           ,prpRct = { exists = ingressInfo.prpRct , SeqNr = ingressInfo.SeqNr , lanId = ingressLanId ,lsduSize = ingressLsduSize}
           ,vlanTag = {exists = ingressInfo.vlanTag , vlanId = 1, cfi = 0, vpt = 0}
           ,payload = ingressPayload
           ,totalPacketSize = ingressInfo.totalPacketSize
           ,PtpInfo = ingressInfo.PtpInfo
        }

        --printLog("packetInfo=" ,to_string(packetInfo));

        transmitInfo.pktInfo.fullPacket = buildPacket(packetInfo)
        transmitInfo.inBetweenSendFunc  = ingressInfo.inBetweenSendFunc
        if(fromCpuEmbeddedVlanNotOk and (ingressInfo.packetNum == "5-2" or ingressInfo.packetNum == "5-4")) then
            -- there is no vlan tag that is hidden in the DSA
            transmitInfo.hiddenRxBytes      = nil
        else
            transmitInfo.hiddenRxBytes      = ingressInfo.hiddenRxBytes
        end

        --printLog("fullPacket=" ,to_string(transmitInfo.pktInfo.fullPacket));

        if(ingressInfo.preTestFunc)then
            doFunction(ingressInfo.preTestFunc,ingressInfo.preTestParams)
        end

        egressInfoTable = {}
        local index = 1
        for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do

            local egressLanId,egressLsduSize

            egressLanId = lanIdMap[egressInfo.trgPort] -- egress with lanId according to egress port

            if ingressInfo.srcPort == PRP_port_A or  ingressInfo.srcPort == PRP_port_B then
                egressLsduSize = ingressLsduSize
            else
                egressLsduSize = calcLsduSize(ingressPayload,egressInfo.PtpInfo--[[only PtpInfo.messageType needed]])
            end

            --[[
            build packet info:
                 l2 = {srcMac = , dstMac = }
                ,prpRct = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
                ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
                payload =
                totalPacketSize =
                padding =
            ]]

            local egress_prpRct

            egressInfoTable[index] = {}
            egressInfoTable[index].portNum = egressInfo.trgPort

            if egressInfo.trgPort == portCPU then
                egressInfoTable[index].portNum = "CPU"
                egressLanId = ingressLanId -- keep the same LanId
            end

            if (ingressInfo.srcPort == portCPU and
                 egressInfo.trgPort == ingressInfo.trgPort) then
                egressInfoTable[index].portNum = "CPU"
                if egressInfo.prpRct and (ingressInfo.trgPort == PRP_port_A or  ingressInfo.trgPort == PRP_port_B) then
                    egressLanId = ingressLanId -- keep the same LanId
                    egressLsduSize = ingressLsduSize
                end
            end

            local packetInfo = {
                l2 = {srcMac = ingressInfo.srcMac, dstMac = ingressInfo.dstMac}--[[the L2 kept]]
               ,prpRct = { exists = egressInfo.prpRct , SeqNr = egressInfo.SeqNr , lanId = egressLanId ,lsduSize = egressLsduSize}
               ,vlanTag = {exists = egressInfo.vlanTag , vlanId = 1, cfi = 0, vpt = 0}
               ,payload = ingressPayload--[[the payload kept]]
               ,totalPacketSize = egressInfo.totalPacketSize
               ,padding = egressInfo.padding
               ,PtpInfo = ingressInfo.PtpInfo
            }

            if egressInfo.notEgress == true then -- indication of drop
                egressInfoTable[index].pktInfo = nil
                egressInfoTable[index].packetCount = 0 -- check that not getting traffic
            else
                local egressFullPacket,my_mustNotBeEqualBytesTable,ptpStartOffset = buildPacket(packetInfo)

                if ptpStartOffset ~= 0 and
                   egress_ports_with_correction_field_changed[egressInfo.trgPort] and
                   egress_ports_with_correction_field_changed[egressInfo.trgPort].correction_field_updated
                then
                    -- we need to ignore 6 bytes of <correction field>
                    if not my_mustNotBeEqualBytesTable then
                        my_mustNotBeEqualBytesTable = {}
                    end

                    local startByte = ptpStartOffset + 2--[[the caller did not skip the ethertype of the PTP header]]
                    local newItem = {startByte = startByte+8, endByte = startByte+13 ,reason = "6 bytes nanoSeconds in <correction field>"}

                    if isGmUsed() then
                        -- the GM not have DP/MAC units to update the PTP related fields
                    else
                        tableAppend(my_mustNotBeEqualBytesTable,newItem)
                    end
                end

                egressInfoTable[index].pktInfo = {fullPacket = egressFullPacket , mustNotBeEqualBytesTable = my_mustNotBeEqualBytesTable}
                egressInfoTable[index].packetCount = nil
            end

            index = index + 1


        end -- end of loop on egress ports

        egressInfoTable[index] = {}
        egressInfoTable[index].portNum = otherPort
        egressInfoTable[index].packetCount = 0  -- check that not getting traffic
        egressInfoTable[index].pktInfo = nil

        if ingressInfo.packetNum == ssl_test then
            if sslw == true then
                shell_execute("sslw")
            else
                shell_execute("ssl")
            end
        end

        if forceCaptureOnTransmitPort then
            -- we need to capture the egress packet that loopback from the port to the CPU
            -- use iPCL rule to trap it
            forceCaptureOnTransmitPortSet(true)

        end

        printLog("Ingress from : " .. functionalityMap[transmitInfo.portNum] .. "(port : " .. transmitInfo.portNum ..")")
        for _dummy3, egressInfo in pairs(egressInfoTable) do
            local egressPortString
            if functionalityMap[egressInfo.portNum] then
                egressPortString = "Egress from : " .. functionalityMap[egressInfo.portNum] .. "(port : " .. egressInfo.portNum ..")"
            else
                egressPortString =  "Egress from : port " .. egressInfo.portNum .. "(non-functional port)"
            end

            if(egressInfo.packetCount == 0) then
                egressPortString = "! Not ! " .. egressPortString
            end

            printLog(egressPortString)
        end

        --printLog("transmitInfo", to_string(transmitInfo))
        --printLog("egressInfoTable", to_string(egressInfoTable))
        if isEmulatorUsed() and transmitInfo.inBetweenSendFunc and  ingressInfo.force_no_change_timeout then
            local index = 1
            for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do
                local needToEgress = not egressInfo.notEgress
                if egressInfo.trgPort == PRP_port_A and needToEgress then
                    egressInfoTable[index].packetCount = 1   -- only to count once
                    egressInfoTable[index].pktInfo     = nil -- and not send second iteration
                    break--[[remove the PRP_port_A that asked for]]
                end

                index = index + 1
            end
        end

        if isEmulatorUsed() and transmitInfo.inBetweenSendFunc and not ingressInfo.force_no_change_timeout then
            ddeSameKeyExpirationTimeSet(inBetween_ddeSameKeyExpirationTime)
        end

        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           transmitInfo, egressInfoTable);
        local result_string = caseName

        if isEmulatorUsed() and transmitInfo.inBetweenSendFunc and not ingressInfo.force_no_change_timeout then
            --restore
            ddeSameKeyExpirationTimeSet(test_ddeSameKeyExpirationTime)
        end

        if forceCaptureOnTransmitPort then
            -- restore the default
            forceCaptureOnTransmitPortSet(false)
        end

        if ingressInfo.packetNum == ssl_test then
            shell_execute("nssl")
        end

        if rc ~= 0 then
            result_string = result_string .. " FAILED"
            printLog ("ENDED : " .. result_string .. "\n")
            testAddErrorString(result_string)
        else
            printLog ("ENDED : " .. result_string .. "\n")
            testAddPassString(result_string)
        end

        if(ingressInfo.postTestFunc)then
            doFunction(ingressInfo.postTestFunc,ingressInfo.postTestParams)
        end

        setDsaFromCpuTrgVlanTagged(false) -- remove the need for dedicated post fucntion to do this
        egress_ports_with_correction_field_changed[PRP_port_A].correction_field_updated = false
        egress_ports_with_correction_field_changed[PRP_port_B].correction_field_updated = false
        egress_ports_with_correction_field_changed[interlink].correction_field_updated  = false


    end -- loop on incomingPacketsDb[]
end -- end of function doMainLogic

-- indexes match those of ptp_test_info[] and the info is extention to ptp_test_info[]
local ptp_tests_extra_info = {
    [1] = {testFunc = function(catagory)
            doMainLogic(incomingPacketsDb_ptp_no_RCT,outgoingPacketsDb_ptp_no_RCT,catagory)
        end
    },
    [2] = {testFunc = function(catagory)
            running_ptp_in_2_steps = true
            executeLocalConfig("dxCh/examples/configurations/prp_san_ptp_2_steps.txt")
            doMainLogic(incomingPacketsDb_ptp_no_RCT_2_steps,outgoingPacketsDb_ptp_no_RCT_2_steps,catagory)
            executeLocalConfig("dxCh/examples/configurations/prp_san_ptp_2_steps_deconfig.txt")
            running_ptp_in_2_steps = false
        end
    },
}

function ddeSameKeyExpirationTimeSet(timeInMilisec)
    local apiName = "cpssDxChHsrPrpDdeTimeFieldSet"

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT",    "field",    CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E },
        { "IN",     "GT_U32",    "timeInMicroSec",    timeInMilisec * 1000 }-- time in the API is in micro sec
    })
end

-------------- do the test --------------
-- IM-M previous preemption tests reconfigures ports.
-- This is conflicts with IM-M internal PHY ports with PM
-- Disable PM task to avoid conflicts.
if (isPortManagerMode() == true) and is_IronMan_M_in_system() then
    luaShellExecute("prvWrAppPortManagerTaskEnableSet 0")
end

executeLocalConfig("dxCh/examples/configurations/prp_san.txt")
ddeSameKeyExpirationTimeSet(test_ddeSameKeyExpirationTime)

if not skip_part_1 then
    doMainLogic(incomingPacketsDb,outgoingPacketsDb)
end

if not skip_part_2 then
    executeLocalConfig("dxCh/examples/configurations/prp_san_cpu.txt")

    doMainLogic(incomingPacketsDb_toFromCpu,outgoingPacketsDb_toFromCpu)

    executeLocalConfig("dxCh/examples/configurations/prp_san_cpu_deconfig.txt")
end

if not skip_part_3 then
    executeLocalConfig("dxCh/examples/configurations/prp_san_ptp.txt")

    for ii = 1, #ptp_test_info do
        local ptp_info = ptp_test_info[ii]
        if not ptp_info.skip then
            local ptp_extra_info = ptp_tests_extra_info[ii]
            local catagory = "PTP:" ..ptp_info.doc_section..": "..ptp_info.ptp_step .. " " .. ptp_info.clockType

            ptp_extra_info.testFunc(catagory)
        end
    end -- end of loop on ptp_test_info

    executeLocalConfig("dxCh/examples/configurations/prp_san_ptp_deconfig.txt")
else
    printLog("NOTE: Skip testing of PTP part of PRP")
end


ddeSameKeyExpirationTimeSet(2)-- restore the default after init
executeLocalConfig("dxCh/examples/configurations/prp_san_deconfig.txt")
testPrintResultSummary(testName)

if (isPortManagerMode() == true) and is_IronMan_M_in_system() then
    luaShellExecute("prvWrAppPortManagerTaskEnableSet 1")
end
