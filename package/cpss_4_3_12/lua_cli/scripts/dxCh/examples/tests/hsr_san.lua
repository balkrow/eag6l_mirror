--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* hsr_san.lua
--*
--* DESCRIPTION:
--*       The test for testing HSR-SAN
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


local testName = "HSR-SAN"

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local portCPU = devEnv.portCPU -- cpu port

-- Supported in IronMan but not in AAS
SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_6_30", true, "SIP_7")

--test that need 'simulation log'
local ssl_test = nil--"1-2"
local sslw = false

-- get extra PTP util functions
-- use dofile instead of require .. to allow quick load of test
dofile("dxCh/examples/common/ptp_utils.lua")

-- note : ptp_tests_extra_info[] hold extra info about the tests
local ptp_test_info = {
    [1] = {skip = false ,doc_section = "1.3(old)",ptp_step = "single step(old)" ,clockType = "BC(Border Clock),Master"   ,dsa = "FORWARD"  , note = "was removed from testing doc v3"},
    [2] = {skip = false ,doc_section = "1.3"   ,ptp_step = "single step" ,clockType = "BC(Border Clock),Master"   ,dsa = "FROM_CPU" },
    [3] = {skip = false ,doc_section = "1.5"   ,ptp_step = "single step" ,clockType = "BC(Border Clock),Ordinary" ,dsa = "FROM_CPU" },
    [4] = {skip = false ,doc_section = "1.4"   ,ptp_step = "2 steps"     ,clockType = "BC(Border Clock),Master"   ,dsa = "FROM_CPU" },
    [5] = {skip = false ,doc_section = "1.6"   ,ptp_step = "2 steps"     ,clockType = "BC(Border Clock),Ordinary" ,dsa = "FROM_CPU" },
    [6] = {skip = false ,doc_section = "not PTP??" ,ptp_step = "2 steps" ,clockType = "BC(Border Clock),Master"   ,dsa = "FROM_CPU" },
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

local hwOrEmulator = not wrlCpssIsAsicSimulation()

local clearDdeNotOk = hwOrEmulator or isGmUsed() -- GM and HW not allow clear the DDE
local fromCpuEmbeddedVlanNotOk = false
local test_ddeSameKeyExpirationTime
local inBetween_ddeSameKeyExpirationTime

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
    -- note : 1000 was not enough for case "1-7"
    test_ddeSameKeyExpirationTime = 2000
    inBetween_ddeSameKeyExpirationTime = 2
end

cmdLuaCLI_registerCfunction("wrlDxChDsaToString")
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation")

local ePortHsrBase = 0x3f0 --[[decimal 1008]]

local function hportToEPort(hport)
    return ePortHsrBase + hport
end


-- physical port of ring A
global_test_data_env.ringA      = port1
-- physical port of ring B
global_test_data_env.ringB      = port2
-- physical port of interlink
global_test_data_env.interlink  = port3

global_test_data_env.CPU = portCPU

-- eport of ring port A
global_test_data_env.ePort_ringA = hportToEPort(0)
-- eport of ring port B
global_test_data_env.ePort_ringB = hportToEPort(1)
-- eport of Interlink
global_test_data_env.ePort_interlink = hportToEPort(2)
-- base eport
global_test_data_env.ePortHsrBase = ePortHsrBase

-- physical port of ring A
local ringA = global_test_data_env.ringA
-- physical port of ring B
local ringB = global_test_data_env.ringB
-- physical port of interlink
local interlink = global_test_data_env.interlink
-- other port that should not get traffic
local otherPort = port4

local hport_map = {
    [ringA] = global_test_data_env.ePort_ringA - ePortHsrBase,
    [ringB] = global_test_data_env.ePort_ringB - ePortHsrBase,
    [interlink] = global_test_data_env.ePort_interlink - ePortHsrBase
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

local temp_C1,temp_C2 = power_2_number(portCPU) -- CPU
local temp_A1,temp_A2 = power_2_number(ringA)-- ring A
local temp_B1,temp_B2 = power_2_number(ringB)-- ring B
local temp_I1,temp_I2 = power_2_number(interlink)-- Interlink


function hsrTest1VidxToTargetPortsBmp(vidx,wordIndex)
    local relativeVidx = vidx - vidxBase
    local results_1,results_2 = 0,0

    if relativeVidx >= 8 and relativeVidx <= 15 then
        relativeVidx = relativeVidx - 8
        results_1 = results_1 + temp_C1
        results_2 = results_2 + temp_C2
    end


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

local macK1 = "00AA22334457"
local macK2 = "00AA22334458"

local macA2 = "001122334457"
local macB1 = "00FFEEDDBBAB"
local macC1 = "00987654321E"

local macCpu= "00ccc000abcd"
local macOtherCpu = "00ccc000abff"


local MC1   = "010033445566" --multicast mac

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
global_test_data_env.MC1   = makeMacAddrTestFormat(MC1)


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

local HsrEtherTypeStr = "892F" -- ethertype 16 bits

local   egress_ports_with_correction_field_changed = {
    [ringA]     = { correction_field_updated = nil },
    [ringB]     = { correction_field_updated = nil },
    [interlink] = { correction_field_updated = nil },
}



local function buildHsrTag(SeqNr,lanId,lsduSize)
    local SeqNrStr          = string.format("%4.4x",SeqNr)      --16 bits
    local HsrPathIdStr      = string.format("%1.1x",lanId)      -- 4 bits
    local HsrLsduSizeStr    = string.format("%3.3x",lsduSize)   --12 bits

    return HsrEtherTypeStr .. HsrPathIdStr .. HsrLsduSizeStr .. SeqNrStr
end

local function buildVlanTag(vlanId,cfi,vpt)
    local vidStr            = string.format("%3.3x",vlanId)     --12 bits
    local cfiVptStr         = string.format("%1.1x",vpt*2+cfi)  -- 4 bits
    return "8100" .. cfiVptStr .. vidStr
end

local function buildDsaFrwToVidx(vidx,vlanTag)
    local hwDevNum = 0x321 --[[801 decimal]] --[[ other device in the 'stack' ]]
    local edsaStc = {
        dsaType = "CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
        commonParams = { dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT",
                 vpt    = (vlanTag and vlanTag.exists) and vlanTag.vpt    or 0,
                 cfiBit = (vlanTag and vlanTag.exists) and vlanTag.cfi    or 0,
                 vid    = (vlanTag and vlanTag.exists) and vlanTag.vlanId or 1
                 },
        dsaInfo = {
            forward = {
              tag0TpidIndex        = 0,
              srcHwDev             = hwDevNum,
              srcIsTagged          = (vlanTag and vlanTag.exists) and "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E" or "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
              srcIsTrunk           = false,
              isTrgPhyPortValid    = false,
              dstInterface         ={ type="CPSS_INTERFACE_VIDX_E", vidx=vidx },
              phySrcMcFilterEnable = true,
              skipFdbSaLookup      = true,

              source = {portNum = portCPU},-- src eport
              origSrcPhy = {portNum = portCPU}   -- 12 bits in eDSA because muxed with origTrunkId
            }
        }
    }

    local rc, edsaBytes = wrlDxChDsaToString(devNum, edsaStc)
    if (rc ~=0) then
        setFailState()
        printLog(edsaBytes .. ". Error code is " .. rc)
        return ""
    end

    return edsaBytes
end

local ownDevNum , dummyPortNum = Command_Data:getHWDevicePort(devNum,0)

--[[
    devPort = {
        trgEPortNum =
        srcEPortNum =
        srcPortNum  =
    }
]]
local function buildDsaFrwToPort(vlanTag,devPort)
    local hwDevNum = ownDevNum -- as we need to use info of the srcEport
    local vlanTagExists = vlanTag and vlanTag.exists
    local edsaStc = {
      dsaType = "CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
      commonParams = { dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT",
                     vpt    = vlanTagExists and vlanTag.vpt    or 0,
                     cfiBit = vlanTagExists and vlanTag.cfi    or 0,
                     vid    = vlanTagExists and vlanTag.vlanId or 1
                     },
      dsaInfo = {
        forward = {
          tag0TpidIndex        = 0,
          srcHwDev             = hwDevNum,
          srcIsTagged          = vlanTagExists and "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E" or "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
          srcIsTrunk           = false,
          isTrgPhyPortValid    = false,
          dstInterface         ={ type="CPSS_INTERFACE_PORT_E", devPort = {devNum = hwDevNum , portNum = devPort.trgEPortNum} },
          phySrcMcFilterEnable = true,
          skipFdbSaLookup      = true,

          source = {portNum = devPort.srcEPortNum},-- src eport
          origSrcPhy = {portNum = devPort.srcPortNum}   -- 12 bits in eDSA because muxed with origTrunkId
        }
      }
    }

    local rc, edsaBytes = wrlDxChDsaToString(devNum, edsaStc)
    if (rc ~=0) then
        setFailState()
        printLog(edsaBytes .. ". Error code is " .. rc)
        return ""
    end

    return edsaBytes
end


--[[
build packet info:
     l2 = {srcMac = , dstMac = }
    ,hsrTag = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
    ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
    ,payload =
    ,dsaTag = {vidx = }

    return :
    1. the string of the packet
    2. the 'mustNotBeEqualBytesTable' if not nil , indicate what bytes must not egress as ingress
]]

local function buildPacket(info)
    local tempHsrTag = ""
    local tempVlanTag = ""
    local tempDsaTag = ""
    local ptpHeader = ""
    local my_mustNotBeEqualBytesTable = nil
    local ptpStartOffset = 0

    if  info.hsrTag and info.hsrTag.exists then
        local hsrTag = info.hsrTag
        tempHsrTag = buildHsrTag(hsrTag.SeqNr,hsrTag.lanId,hsrTag.lsduSize)
    end

    if info.dsaTag then
        if info.dsaTag.vidx then
            -- the CPU send DSA tag , that need to build
            tempDsaTag = buildDsaFrwToVidx(info.dsaTag.vidx,info.vlanTag)
        elseif info.dsaTag.note == "CPU get FRW" then
            -- the CPU get DSA FRW tag , that was removed by the CPSS
            -- we not build DSA and not vlan tag that was embedded in the FRW DSA
        elseif info.dsaTag.devPort then
            tempDsaTag = buildDsaFrwToPort(info.vlanTag,info.dsaTag.devPort)
        end
    else
        -- if we build DSA tag as FORWARD , we indicate in the DSA that it came with vlan tag (and embedded in the DSA)
        if  info.vlanTag and info.vlanTag.exists then
            local vlanTag = info.vlanTag
            tempVlanTag = buildVlanTag(vlanTag.vlanId,vlanTag.cfi,vlanTag.vpt)
        end
    end

    if info.PtpInfo then
        ptpStartOffset = string.len(info.l2.dstMac .. info.l2.srcMac .. tempDsaTag .. tempVlanTag .. tempHsrTag) / 2
        ptpHeader,my_mustNotBeEqualBytesTable = buildPtpTag(info.PtpInfo , string.len(info.payload) / 2--[[size of payload]] , ptpStartOffset)
    end

    return info.l2.dstMac .. info.l2.srcMac .. tempDsaTag .. tempVlanTag .. tempHsrTag .. ptpHeader .. info.payload
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

local function checkDdlFirstCounter(counterType,hportCounters)
    local isError = false
    local typeToCounterMap = {
        First = 1,
        Second = 2,
        Third = 3,
    }

    local index = typeToCounterMap[counterType]

    if  index ~= nil then -- first  counter
        if (hportCounters[1] ~= 0 and index ~= 1) then
            isError = true
            printLog("First Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters))
        end
        if (hportCounters[2] ~= 0 and index ~= 2) then
            isError = true
            printLog("Second Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters))
        end
        if (hportCounters[3] ~= 0 and index ~= 3) then
            isError = true
            printLog("Third Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters))
        end
        if (hportCounters[index] == 0) then
            isError = true
            printLog("Counter failed (ZERO) . hportCounters = " .. to_string(hportCounters) .. " counterType = " .. counterType)
        end
    else
        if (hportCounters[1] ~= 0) then
            printLog("First Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters) .. " counterType = " .. counterType)
            isError = true
        end
        if (hportCounters[2] ~= 0) then
            printLog("Second Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters) .. " counterType = " .. counterType)
            isError = true
        end
        if (hportCounters[3] ~= 0) then
            printLog("Third Counter failed (non-ZERO) . hportCounters = " .. to_string(hportCounters) .. " counterType = " .. counterType)
            isError = true
        end
    end

    return isError
end
local function checkDdlCounters(expected_ddlCounters,hportCounters,hport)
    if hport == hport_map[ringA] then
        return checkDdlFirstCounter(expected_ddlCounters.ringA,hportCounters)
    elseif hport == hport_map[ringB] then
        return checkDdlFirstCounter(expected_ddlCounters.ringB,hportCounters)
    elseif hport == hport_map[interlink] then
        return checkDdlFirstCounter(expected_ddlCounters.interlink,hportCounters)
    end
end

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

local function hport_DdlCountersCheck(hPort,expected_ddlCounters)
    local apiName = "cpssDxChHsrPrpHPortEntryGet"

    printLog("check discard counters for hPort = ",to_string(hPort))

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U32",    "hPort",    hPort },
        { "OUT",    "CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC",    "infoPtr"},
        { "OUT",    "GT_U64",    "firstCounterPtr"},
        { "OUT",    "GT_U64",    "secondCounterPtr"},
        { "OUT",    "GT_U64",    "thirdCounterPtr"}
    })

    --printLog("values = ",to_string(values))
    --printLog("values.firstCounterPtr = ",to_string(values["firstCounterPtr"]))
    --printLog("values.secondCounterPtr = ",to_string(values["secondCounterPtr"]))
    --printLog("values.thirdCounterPtr = ",to_string(values["thirdCounterPtr"]))
    local hportCounters = {
        [1]=values["firstCounterPtr"].l[0],
        [2]=values["secondCounterPtr"].l[0],
        [3]=values["thirdCounterPtr"].l[0]}

    --printLog("hportCounters = ",to_string(hportCounters))

    if not isError and expected_ddlCounters ~= nil then
        -- compare counters
        if(checkDdlCounters(expected_ddlCounters,hportCounters,hPort) == true) then
            isError = true
        end
    end

    return isError
end

local function globalDdlCountersCheck(outgoingPacketsDb_onPort,srcPort)
    local rc = 0
    local expected_ddlCounters = outgoingPacketsDb_onPort[1].ddlCounters

    if not expected_ddlCounters then
        -- not checking the ddlCounters
        return 0 --GT_OK
    end

    --printLog("part 1 : expected_ddlCounters",to_string(expected_ddlCounters))

    for _dummy2, egressInfo in pairs(outgoingPacketsDb_onPort) do
        local hPort = hport_map[egressInfo.trgPort]
        if true == hport_DdlCountersCheck(hPort,expected_ddlCounters) then
            rc = "failed"
        end
    end -- end of loop on egress ports

    --printLog("part 2 : expected_ddlCounters",to_string(expected_ddlCounters))
    local hPort = hport_map[srcPort]
    if true == hport_DdlCountersCheck(hPort,expected_ddlCounters) then
        rc = "failed"
    end

    return rc
end

local function clearDDE(index)
    if (isEmulatorUsed()) then
        printLog("Emulator : allow cache of DDE to clean (before CPU invalidated the entry)  --> sleep 2000 milisec")
        delay(2000)
    end

    local apiName = "cpssDxChHsrPrpDdeEntryInvalidate"

    if clearDdeNotOk then
        printLog("WARNING : DO NOT clean DDE index " .. index)
        return
    end

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
-- delete the DDE entry for MAC-A with seqNum = 1001
-- delete before changing the partition mode
local function clearDDE_8448()
    clearDDE(8448)
end
-- delete the DDE entry for MAC-A with seqNum = 1002
-- delete before changing the partition mode
local function clearDDE_17361()
    clearDDE(17361)
end
-- delete the DDE entry for MAC-B with seqNum = 1003
-- delete before changing the partition mode
local function clearDDE_5952()
    clearDDE(5952)
end
-- delete the DDE entry for MAC-C1 with seqNum = 5003
-- delete before changing the partition mode
local function clearDDE_5504()
    clearDDE(5504)
end

-- delete the DDE entry for MAC-B with seqNum = 5005
-- delete before changing the partition mode
local function clearDDE_5488()
    clearDDE(5488)
end

-- delete the DDE entry for MAC-B with seqNum = 3000
-- delete before changing the partition mode
local function clearDDE_10160()
    clearDDE(10160)
end

local function wrongLsduSizeCommandSet(allowError)
    local apiName = "cpssDxChHsrPrpExceptionCommandSet"
    local command

    if allowError then
        command = "CPSS_PACKET_CMD_FORWARD_E"
    else
        command = "CPSS_PACKET_CMD_DROP_HARD_E"
    end

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT",    "type",    "CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E" },
        { "IN",     "CPSS_PACKET_CMD_ENT",    "command",    command }
    })
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


-- info accordint to :
-- https://sp.marvell.com/sites/EBUSites/Switching/Architecture/Shared%20Documents%20-%20arch%20sharepoint/Projects/Ironman/Verification%20use%20cases/HSR%20and%20PRP%20Use%20Cases.pdf?csf=1

local incomingPacketsDb =
{
   {packetNum = "1-1" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true  , remark = "New address (SeqNr=0)"},
   {packetNum = "1-2" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = false , remark = "Existing address (SeqNr=1)"},
   {packetNum = "1-3" , srcPort = interlink , srcMac = macA , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true  , remark = "Existing address (SeqNr=2)"},
   {packetNum = "1-4" , srcPort = ringA     , srcMac = macB , dstMac = macC , SeqNr = 1000, HsrTag = true  , vlanTag = true  , remark = "First packet from Ring", inBetweenSendFunc = clearDDE_20736},
   {packetNum = "1-5" , srcPort = ringA     , srcMac = macB , dstMac = macC , SeqNr = 1000, HsrTag = true  , vlanTag = true  , remark = "Same packet from same port"},
   {packetNum = "1-6" , srcPort = ringB     , srcMac = macB , dstMac = macC , SeqNr = 1000, HsrTag = true  , vlanTag = true  , remark = "Same packet from other port"},
   {packetNum = "1-7" , srcPort = ringB     , srcMac = macB , dstMac = macC , SeqNr = 1000, HsrTag = true  , vlanTag = true  , remark = "Same packet from other port"},
   {packetNum = "1-8" , srcPort = ringA     , srcMac = macB , dstMac = macC , SeqNr = 1001, HsrTag = true  , vlanTag = false , remark = "First packet from Ring", inBetweenSendFunc = clearDDE_8448},
   {packetNum = "1-9" , srcPort = ringB     , srcMac = macB , dstMac = macC , SeqNr = 1001, HsrTag = true  , vlanTag = false , remark = "Same packet from other port"},
   {packetNum = "1-10", srcPort = ringA     , srcMac = macB , dstMac = macA , SeqNr = 1002, HsrTag = true  , vlanTag = true  , remark = "First packet 2me from Ring"},
   {packetNum = "1-11", srcPort = ringA     , srcMac = macB , dstMac = macA , SeqNr = 1002, HsrTag = true  , vlanTag = true  , remark = "Same packet 2me other port"},
   {packetNum = "1-12", srcPort = ringA     , srcMac = macA , dstMac = macB , SeqNr = 1003, HsrTag = true  , vlanTag = true  , remark = "Packet from me"},
   {packetNum = "1-13", srcPort = interlink , srcMac = macA1, dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true  , remark = "No ready entry"},
   {packetNum = "1-14-A", srcPort = ringA     , srcMac = macB , dstMac = macK1, SeqNr = 1004, HsrTag = true  , vlanTag = true  , remark = "Wrong LSDU size , defaul command (drop)" , wrongLsduSize = true},
   {packetNum = "1-14-B", srcPort = ringA     , srcMac = macB , dstMac = macK2, SeqNr = 1005, HsrTag = true  , vlanTag = true  , remark = "Wrong LSDU size , forward command (no drops)" , wrongLsduSize = true,
    preTestFunc  = wrongLsduSizeCommandSet , preTestParams = true  ,
    postTestFunc = wrongLsduSizeCommandSet , postTestParams = false},
}

local function cpssDxChStreamEntryGet(index)
    local apiName = "cpssDxChStreamEntryGet"

    printLog("Get stream IRF info for index = ",to_string(index))

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",     "devNum",   devNum },
        { "IN",     "GT_U32",    "index",    index },
        { "OUT",    "CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC"  ,     "sngIrfInfo"},
        { "OUT",    "CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC",  "irfCounters"  },
        { "OUT",    "CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC"  , "srfMappingInfo"},
    })

    printLog(to_string(values))

    if not values then
        return nil,nil,nil
    end

    return values.sngIrfInfo , values.irfCounters , values.srfMappingInfo
end

local SeqNr_base_streamId_6 = 0

if (isGmUsed()) then
    -- the GM have bug : <Reset Packet Counter> not trigger clear of : <Packet Counter>
    -- and stream_id #6 was used by the _802_1 test that runs before this test , and leave <Packet Counter> non-zero
    local sngIrfInfo , irfCounters , srfMappingInfo = cpssDxChStreamEntryGet(6)

    SeqNr_base_streamId_6 = sngIrfInfo.currentSeqNum

end

local outgoingPacketsDb =
{
    ["1-1"] = {
        {trgPort = ringA , SeqNr = SeqNr_base_streamId_6 + 0 , HsrTag = true , vlanTag = true , ddlCounters = {interlink = "-",ringB = "-" , ringA = "-" }}
       ,{trgPort = ringB , SeqNr = SeqNr_base_streamId_6 + 1 , HsrTag = true , vlanTag = true , }
    }
   ,["1-2"] = {
        {trgPort = ringA , SeqNr = SeqNr_base_streamId_6 + 2 , HsrTag = true , vlanTag = false , ddlCounters = {interlink = "-",ringB = "-" , ringA = "-" }}
       ,{trgPort = ringB , SeqNr = SeqNr_base_streamId_6 + 3 , HsrTag = true , vlanTag = false , }
    }
   ,["1-3"] = {
        {trgPort = ringA , SeqNr = SeqNr_base_streamId_6 + 4 , HsrTag = true , vlanTag = true , ddlCounters = {interlink = "-",ringB = "-" , ringA = "-" }}
       ,{trgPort = ringB , SeqNr = SeqNr_base_streamId_6 + 5 , HsrTag = true , vlanTag = true , }
    }
   ,["1-4"] = {
        {trgPort = interlink, SeqNr ="-", HsrTag = false , vlanTag = true , ddlCounters = {interlink = "First"  ,ringB = "First" , ringA = "-" }}
       ,{trgPort = ringB    , SeqNr=1000, HsrTag = true  , vlanTag = true }
    }
   ,["1-5"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "Second"  ,ringB = "Second" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true}
    }
   ,["1-6"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "Third"  ,ringB =   "NA" , ringA = "First" }}
       ,{trgPort = ringA    , SeqNr=1000, HsrTag = true  , vlanTag = true }
    }
   ,["1-7"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "-" ,ringB =  "NA" , ringA = "Second" }}
       ,{trgPort = ringA    , notEgress = true }
    }
   ,["1-8"] = {
        {trgPort = interlink, SeqNr ="-", HsrTag = false , vlanTag = false , ddlCounters = {interlink = "First"  ,ringB = "First" , ringA = "-" }}
       ,{trgPort = ringB    , SeqNr=1001, HsrTag = true  , vlanTag = false }
    }
   ,["1-9"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "Second"  ,ringB = "NA" , ringA = "First" }}
       ,{trgPort = ringA    , SeqNr=1001, HsrTag = true  , vlanTag = false }
    }
   ,["1-10"] = {
        {trgPort = interlink, SeqNr ="-", HsrTag = false , vlanTag = true , ddlCounters = {interlink = "First"  ,ringB = "-" , ringA = "-" }}
    }
   ,["1-11"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "Second" ,ringB =  "-" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true }
    }
   ,["1-12"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "-" ,ringB =  "-" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true }
    }
   ,["1-13"] = {
        {trgPort = ringA    , notEgress = true                            ,ddlCounters = {interlink = "-" ,ringB =  "-" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true }
    }


   ,["1-14-A"] = {
        {trgPort = interlink, notEgress = true                            ,ddlCounters = {interlink = "-" ,ringB =  "-" , ringA = "-" }}
       ,{trgPort = ringB    , notEgress = true }
    }
   ,["1-14-B"] = {
        {trgPort = interlink, SeqNr ="-", HsrTag = false , vlanTag = true , ddlCounters = {interlink = "First"  ,ringB = "-" , ringA = "-" }}
    }
}

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

local function showDde()
    executeStringCliCommands("do shell-execute cpssDxChHsrPrpFdbDump ${dev}")
end

local function forceExplicitDsaInfo(explicitDsa)
    local isExplicitDsa = explicitDsa and "1" or "0"
    --force/unset to packet that send by the test from the CPU to use explicit DSA tag as the test will define inside the packet
    executeStringCliCommands("do shell-execute prvTgfTrafficForceExplicitDsaInfo " .. isExplicitDsa )
--[[GT_STATUS prvTgfTrafficForceExplicitDsaInfo(
    GT_U32         force
)]]
end


local incomingPacketsDb_toFromCpu = {
    {packetNum = "5-1" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = false  , trgPort = ringA , remark = " From_CPU to Ring A"},
    {packetNum = "5-2" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = false  , trgPort = ringA , remark = " From_CPU to Ring A (vlan tag from <trgIsTagged>=1)",
    preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
    hiddenRxBytes = 4 --4 bytes of vlan tag hidden in the DSA
    },
    {packetNum = "5-3" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = 5000 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = " From_CPU to Ring B with HSR tag"},
    {packetNum = "5-4" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = 5000 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = " From_CPU to Ring A (vlan tag from <trgIsTagged>=1)",
    preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
    hiddenRxBytes = 4 --4 bytes of vlan tag hidden in the DSA
    },
    {packetNum = "5-5" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true   , trgPort = "vidx" , remark = " (from CPU) Fwd - VIDX= 1K+7",
    dsaTag = {vidx = (1024+7)},
    preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,
    postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false} ,
                    --[2] = {testFunc = showDde              , testParams = nil } ,
                    },
    hiddenRxBytes = (-16), --we build packet with 16 bytes eDSA that will be removed and embededded as 'dsa info'
    },
    {packetNum = "5-6" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = false  , trgPort = "vidx" , remark = " (from CPU) Fwd - VIDX= 1K+7",
    dsaTag = {vidx = (1024+7)},
    preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,
    postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false} ,
                    --[2] = {testFunc = showDde              , testParams = nil } ,
                    },
    hiddenRxBytes = (-16), --we build packet with 16 bytes eDSA that will be removed and embededded as 'dsa info'
    },
    {packetNum = "5-7" , srcPort = portCPU , srcMac = macA2 , dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true   , trgPort = "vidx" , remark = " (from CPU) Fwd - VIDX= 1K+7",
    dsaTag = {vidx = (1024+7)},
    preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,
    postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false} ,
                    --[2] = {testFunc = showDde              , testParams = nil } ,
                    },
    hiddenRxBytes = (-16), --we build packet with 16 bytes eDSA that will be removed and embededded as 'dsa info'
    },
   {packetNum = "5-8" , srcPort = ringA     , srcMac = macB , dstMac = macB1 , SeqNr = 5002, HsrTag = true  , vlanTag = true  , remark = "No DSA - Trapped to CPU",
   },
   {packetNum = "5-9" , srcPort = ringB     , srcMac = macB , dstMac = macB1 , SeqNr = 5002, HsrTag = true  , vlanTag = true  , remark = "No DSA - Trapped to CPU",
   },
   {packetNum = "5-10" , srcPort = ringA    , srcMac = macB , dstMac = macC1 , SeqNr = 5003, HsrTag = true  , vlanTag = false  , remark = "No DSA - Mirrored to CPU",
    inBetweenSendFunc = clearDDE_5504,
    --postTestFunc = showDde , postTestParams = nil ,
   },
   {packetNum = "5-11" , srcPort = ringB    , srcMac = macB , dstMac = macC1 , SeqNr = 5003, HsrTag = true  , vlanTag = false  , remark = "No DSA - Mirrored to CPU",
    inBetweenSendFunc = clearDDE_5504 , force_no_change_timeout = true
   },
   {packetNum = "5-12" , srcPort = ringA     , srcMac = macB , dstMac = macA2 , SeqNr = 5004, HsrTag = true  , vlanTag = true  , remark = "No DSA - Forwarded to CPU",
   },
   {packetNum = "5-13" , srcPort = ringB     , srcMac = macB , dstMac = macA2 , SeqNr = 5004, HsrTag = true  , vlanTag = true  , remark = "No DSA - Forwarded to CPU",
   },
   {packetNum = "5-14" , srcPort = ringA     , srcMac = macB , dstMac = MC1   , SeqNr = 5005, HsrTag = true  , vlanTag = false , remark = "No DSA - Forwarded to CPU",
    inBetweenSendFunc = clearDDE_5488
   },
   {packetNum = "5-15" , srcPort = ringB     , srcMac = macB , dstMac = MC1   , SeqNr = 5005, HsrTag = true  , vlanTag = false , remark = "No DSA - Forwarded to CPU",
   },
}


local outgoingPacketsDb_toFromCpu = {
    ["5-1"] = {
        {trgPort = ringA  , SeqNr = 0 , HsrTag = false , vlanTag = false},
        {trgPort = ringB  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-2"] = {
        {trgPort = ringA  , SeqNr = 0 , HsrTag = false , vlanTag = true},
        {trgPort = ringB  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-3"] = {
        {trgPort = ringB  , SeqNr = 5000 , HsrTag = true , vlanTag = false},
        {trgPort = ringA  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-4"] = {
        {trgPort = ringB  , SeqNr = 5000 , HsrTag = true , vlanTag = true},
        {trgPort = ringA  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-5"] = {
        {trgPort = ringA  , SeqNr = 0 , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , SeqNr = 1 , HsrTag = true , vlanTag = true},
        {trgPort = interlink, SeqNr = "-" , HsrTag = false , vlanTag = true}
    },
    ["5-6"] = {
        {trgPort = ringA  , SeqNr = 3 , HsrTag = true , vlanTag = false},
        {trgPort = ringB  , SeqNr = 4 , HsrTag = true , vlanTag = false},
        {trgPort = interlink, SeqNr = "-" , HsrTag = false , vlanTag = false}
    },
    ["5-7"] = {
        {trgPort = ringA  , SeqNr = 6 , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , SeqNr = 7 , HsrTag = true , vlanTag = true},
        {trgPort = interlink, SeqNr = "-" , HsrTag = false , vlanTag = true}
    },
    ["5-8"] = {
        {trgPort = portCPU , SeqNr = 5002 , HsrTag = true , vlanTag = true},
        {trgPort = ringB   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-9"] = {
        {trgPort = portCPU , SeqNr = 5002 , HsrTag = true , vlanTag = true},
        {trgPort = ringA   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-10"] = {
        {trgPort = portCPU , SeqNr = 5003 , HsrTag = true , vlanTag = false},
        {trgPort = ringB   , SeqNr = 5003 , HsrTag = true , vlanTag = false},
        {trgPort = interlink, HsrTag = false , vlanTag = false}
    },
    ["5-11"] = {
        {trgPort = portCPU , SeqNr = 5003 , HsrTag = true , vlanTag = false},
        {trgPort = ringA   , SeqNr = 5003 , HsrTag = true , vlanTag = false},
        {trgPort = interlink, notEgress = true}
    },
    ["5-12"] = {
        {trgPort = portCPU , SeqNr = 5004 , HsrTag = false , vlanTag = true ,
            dsaTag = {note = "CPU get FRW"}},
        {trgPort = ringB   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-13"] = {
        {trgPort = portCPU , notEgress = true},
        {trgPort = ringA   , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["5-14"] = {
        {trgPort = ringB    , SeqNr = 5005   , HsrTag = true  , vlanTag = false},
        {trgPort = interlink, HsrTag = false , vlanTag = false},
        {trgPort = portCPU  , HsrTag = false , vlanTag = false}
    },
    ["5-15"] = {
        {trgPort = portCPU , notEgress = true},
        {trgPort = ringA   , SeqNr = 5005   , HsrTag = true  , vlanTag = false},
        {trgPort = interlink, notEgress = true}
    },
}

if fromCpuEmbeddedVlanNotOk then
    outgoingPacketsDb_toFromCpu["5-2"][1].vlanTag =  false
    outgoingPacketsDb_toFromCpu["5-4"][1].vlanTag =  false
end

local incomingPacketsDb_ptp_with_FORWARD_DSA = {
    {packetNum = "PTP:learn macB" , skip_when_run_all = true, srcPort = ringA     , srcMac = macB  , dstMac = macC , SeqNr = 3000, HsrTag = true  , vlanTag = true  , remark = "First packet from Ring (learn macB SeqNr = 3000 on RingA)", inBetweenSendFunc = clearDDE_10160},
    {packetNum = "ptp-1" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = "-" , HsrTag = false , vlanTag = true  , trgPort = ringA , remark = "PTP 'Synch' (PTP header) from CPU (FRW eDSA) that egress Ring A,B (with HSR tag)",
         dsaTag = {devPort = {srcPort = interlink , srcEPortNum = global_test_data_env.ePort_interlink , trgEPortNum = ringA }},
        --values from TGF : prvTgfPacketSyncPtpV2Part
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sequenceId = 0xABCD},
     preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,
     postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false} ,
                    },
     hiddenRxBytes = (-6), --the device add HSR tag (6 bytes) on the way from the CPU to the 'loobcack port'(the Rings A,B)
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-2" , srcPort = portCPU , srcMac = macB , dstMac = macC , SeqNr = 3001, HsrTag = true , vlanTag = true  , trgPort = ringA , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring A that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sequenceId = 0xABCE},
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-3" , srcPort = portCPU , srcMac = macB , dstMac = macC , SeqNr = 3001, HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring B that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 ,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sequenceId = 0xABCF},
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-4" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , HsrTag = false , vlanTag = true  , trgPort = ringA , remark = "PTP 'PDelay_Response' (PTP header) from CPU (FRW eDSA) that egress Ring A,B (with HSR tag)",
         dsaTag = {devPort = {srcPort = interlink , srcEPortNum = global_test_data_env.ePort_interlink , trgEPortNum = ringA }},
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x4343 , correctionField_ms_nano = 0x4646 , correctionField_ls_nano = 0x23232323 ,
                sequenceId = 0xAB1E},
     preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,
     postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false} ,
                    },
     hiddenRxBytes = (-6), --the device add HSR tag (6 bytes) on the way from the CPU to the 'loobcack port'(the Rings A,B)
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
     },
}

local outgoingPacketsDb_ptp_with_FORWARD_DSA = {
    ["PTP:learn macB"] = {
        {trgPort = interlink, SeqNr ="-", HsrTag = false , vlanTag = true --[[, ddlCounters = {interlink = "First"  ,ringB = "First" , ringA = "-" }]]}
       ,{trgPort = ringB    , SeqNr=3000, HsrTag = true  , vlanTag = true }
    }
        ,
    ["ptp-1"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "Sync"} , SeqNr = 0   , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , PtpInfo = { messageType = "Sync"} , SeqNr = 1   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-2"] = {
        {trgPort = ringA    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = 3001   , HsrTag = true , vlanTag = true},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-3"] = {
        {trgPort = ringA    , notEgress = true},
        {trgPort = ringB    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = 3001   , HsrTag = true , vlanTag = false},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-4"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = 2   , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = 3   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
}

local incomingPacketsDb_ptp_with_FROM_CPU_DSA = {
    {packetNum = "ptp-Sync-A" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = 6666 , HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'Synch' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB2D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-Sync-B" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = 6666 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'Synch' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xAB3D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_request-A" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = 4001, HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring A that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 0,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB4E},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_request-B" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = 4001, HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring B that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 0,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xAB5E},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-A" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = 6667, HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'PDelay_Response' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB6E},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-B" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = 6668, HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'PDelay_Response' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring B (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xAB7E},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
}
local outgoingPacketsDb_ptp_with_FROM_CPU_DSA = {
    ["ptp-Sync-A"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "Sync"} , SeqNr = 6666   , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-Sync-B"] = {
        {trgPort = ringA  , notEgress = true},
        {trgPort = ringB  , PtpInfo = { messageType = "Sync"} , SeqNr = 6666   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_request-A"] = {
        {trgPort = ringA    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = 4001   , HsrTag = true , vlanTag = true},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_request-B"] = {
        {trgPort = ringA    , notEgress = true},
        {trgPort = ringB    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = 4001   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-A"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = 6667   , HsrTag = true , vlanTag = true},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-B"] = {
        {trgPort = ringA    , notEgress = true},
        {trgPort = ringB  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = 6668   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
}


local incomingPacketsDb_ptp_with_FROM_CPU_DSA_2_steps = {
    {packetNum = "ptp-Sync-A" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = 6666 , HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'Synch' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB8D},
     postTestFunc = ptpPrepareFollowUpTodInfo_2_steps ,
     postTestParams = {trgPort=ringA,domainId=2,messageType="Sync",sequenceId=0xAB8D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-Follow_up-A" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = 6666 , HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'Follow-up' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "Follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB8D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-Sync-B" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = 6666 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'Synch' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xAB9D},
     postTestFunc = ptpPrepareFollowUpTodInfo_2_steps ,
     postTestParams = {trgPort=ringB,domainId=1,messageType="Sync",sequenceId=0xAB9D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-Follow_up-B" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = 6666 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'Follow-up' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring B (with HSR tag)",
         PtpInfo = {messageType = "Follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB9D},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_request-A" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = 4001, HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring A that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xA1CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_request-B" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = 4001, HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring B that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 0,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xA2CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-A" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = 6667, HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'PDelay_Response' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xA3CE},
     postTestFunc = ptpPrepareFollowUpTodInfo_2_steps ,
     postTestParams = {trgPort=ringA,domainId=1,messageType="PDelay_Response",sequenceId=0xA3CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response_follow_up-A" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = 6667, HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'PDelay_Response_follow_up' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response_follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xA3CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-B" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = 6668, HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'PDelay_Response' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring B (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xA4CE},
     postTestFunc = ptpPrepareFollowUpTodInfo_2_steps ,
     postTestParams = {trgPort=ringB,domainId=1,messageType="PDelay_Response",sequenceId=0xA4CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response_follow_up-B" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = 6668, HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'PDelay_Response_follow_up' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring B (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response_follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 1,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xA4CE},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
}
local outgoingPacketsDb_ptp_with_FROM_CPU_DSA_2_steps = {
    ["ptp-Sync-A"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "Sync"} , SeqNr = 6666   , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-Follow_up-A"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "Follow_up"} , SeqNr = 6666   , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-Sync-B"] = {
        {trgPort = ringA  , notEgress = true},
        {trgPort = ringB  , PtpInfo = { messageType = "Sync"} , SeqNr = 6666   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-Follow_up-B"] = {
        {trgPort = ringA  , notEgress = true},
        {trgPort = ringB  , PtpInfo = { messageType = "Follow_up"} , SeqNr = 6666   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_request-A"] = {
        {trgPort = ringA    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = 4001   , HsrTag = true , vlanTag = true},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_request-B"] = {
        {trgPort = ringA    , notEgress = true},
        {trgPort = ringB    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = 4001   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-A"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = 6667   , HsrTag = true , vlanTag = true},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response_follow_up-A"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "PDelay_Response_follow_up"} , SeqNr = 6667   , HsrTag = true , vlanTag = true},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-B"] = {
        {trgPort = ringA    , notEgress = true},
        {trgPort = ringB  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = 6668   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response_follow_up-B"] = {
        {trgPort = ringA    , notEgress = true},
        {trgPort = ringB  , PtpInfo = { messageType = "PDelay_Response_follow_up"} , SeqNr = 6668   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
}

local incomingPacketsDb_ptp_BC_Ordinary = {
    {packetNum = "ptp-Sync-A-to-B-transparet-clock" , srcPort = ringA , srcMac = macB , dstMac = macC , SeqNr = 4000 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "Ring A to Ring B + update the <correction field>",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB8D},
         preTestFunc = function()
                -- we expect the packet that switched packet from RingA to RingB will update the <correction field>
                egress_ports_with_correction_field_changed[ringB].correction_field_updated = true
                -- we want the packet the packet from the CPU to RingA to set the <origin timestamp>
                -- but we send packet with PTP domain '3' that was set in the config file for RingA as update 'correction field'
                -- and not as 'origin timestamp' , so we need to change the config of RingA to 'add time' on this domain
                ptpSetPortAction(ringA,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E")--action

                setDsaFromCpuTrgVlanTagged(true)
            end,
         postTestFunc = function()
                --restore
                egress_ports_with_correction_field_changed[ringB].correction_field_updated = false
                ptpSetPortAction(ringA,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E")--action
            end,
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
    },
    {packetNum = "ptp-Sync-B-to-A-transparet-clock" , srcPort = ringB , srcMac = macB , dstMac = macC , SeqNr = 4000 , HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "Ring B to Ring A + update the <correction field>",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xAB8D},
         preTestFunc = function()
                -- we expect the packet that switched packet from RingB to RingA will update the <correction field>
                egress_ports_with_correction_field_changed[ringA].correction_field_updated = true
                -- we want the packet the packet from the CPU to RingB to set the <origin timestamp>
                -- but we send packet with PTP domain '3' that was set in the config file for RingB as update 'correction field'
                -- and not as 'origin timestamp' , so we need to change the config of RingB to 'add time' on this domain
                ptpSetPortAction(ringB,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E")--action
                setDsaFromCpuTrgVlanTagged(true)
            end,
         postTestFunc = function()
                --restore
                egress_ports_with_correction_field_changed[ringA].correction_field_updated = false
                ptpSetPortAction(ringB,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E")--action
            end,
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
    },
    {packetNum = "ptp-PDelay_request-A" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = 4002, HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring A that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xA1CF},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-A" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = 6669, HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "PTP 'PDelay_Response' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring A (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xA1CF},
         preTestFunc = function()
                -- we expect the packet that switched packet from RingA to RingB will update the <correction field>
                egress_ports_with_correction_field_changed[ringB].correction_field_updated = true
                -- we want the packet the packet from the CPU to RingA to set the <origin timestamp>
                -- but we send packet with PTP domain '3' that was set in the config file for RingA as update 'correction field'
                -- and not as 'origin timestamp' , so we need to change the config of RingA to 'add time' on this domain
                ptpSetPortAction(ringA,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E")--action
                setDsaFromCpuTrgVlanTagged(true)
            end,
         postTestFunc = function()
                --restore
                egress_ports_with_correction_field_changed[ringB].correction_field_updated = false
                ptpSetPortAction(ringA,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E")--action
            end,
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_request-B" , srcPort = portCPU , srcMac = macOtherCpu , dstMac = macCpu , SeqNr = 4002, HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'PDelay_request' (PTP header) with HSR from Ring B that capture to CPU",
         PtpInfo = {messageType = "PDelay_request" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xA2CB},
        preTestFunc  = setDsaFromCpuTrgVlanTagged , preTestParams = true   , --vlan tag embedded in the 'FROM_CPU'
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-PDelay_Response-B" , srcPort = portCPU , srcMac = macCpu, dstMac = macB , SeqNr = 6670, HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "PTP 'PDelay_Response' (PTP header) from CPU (FROM_CPU eDSA) that egress Ring B (with HSR tag)",
         PtpInfo = {messageType = "PDelay_Response" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x3434 , correctionField_ms_nano = 0x6464 , correctionField_ls_nano = 0x32323232 ,
                sourcePortIdentify = "A011B011C022D022E033",
                sequenceId = 0xA2CB},
         preTestFunc = function()
                -- we expect the packet that switched packet from RingB to RingA will update the <correction field>
                egress_ports_with_correction_field_changed[ringA].correction_field_updated = true
                -- we want the packet the packet from the CPU to RingB to set the <origin timestamp>
                -- but we send packet with PTP domain '3' that was set in the config file for RingB as update 'correction field'
                -- and not as 'origin timestamp' , so we need to change the config of RingB to 'add time' on this domain
                ptpSetPortAction(ringB,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E")--action
                setDsaFromCpuTrgVlanTagged(true)
            end,
         postTestFunc = function()
                --restore
                egress_ports_with_correction_field_changed[ringA].correction_field_updated = false
                ptpSetPortAction(ringB,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E")--action
            end,
        hiddenRxBytes = 4 ,--4 bytes of vlan tag hidden in the DSA
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },

}
local outgoingPacketsDb_ptp_BC_Ordinary = {
    ["ptp-Sync-A-to-B-transparet-clock"] = {
        {trgPort = ringB  , PtpInfo = { messageType = "Sync"} , SeqNr = 4000   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true},--filtered by 'srcId' as part of 'non-modeX'
    },
    ["ptp-Sync-B-to-A-transparet-clock"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "Sync"} , SeqNr = 4000   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true},
    },
    ["ptp-PDelay_request-A"] = {
        {trgPort = ringA    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = 4002   , HsrTag = true , vlanTag = true},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-A"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = 6669   , HsrTag = true , vlanTag = true},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_request-B"] = {
        {trgPort = ringA    , notEgress = true},
        {trgPort = ringB    , PtpInfo = { messageType = "PDelay_request"} , SeqNr = 4002   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },
    ["ptp-PDelay_Response-B"] = {
        {trgPort = ringA    , notEgress = true},
        {trgPort = ringB  , PtpInfo = { messageType = "PDelay_Response"} , SeqNr = 6670   , HsrTag = true , vlanTag = true},
        {trgPort = interlink, notEgress = true}
    },

}

local incomingPacketsDb_ptp_BC_Ordinary_2_steps = {
    {packetNum = "ptp-Sync-A-to-B-transparet-clock" , srcPort = ringA , srcMac = macB , dstMac = macC , SeqNr = 6000 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "Ring A to Ring B + update the <correction field>",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB8D},
         preTestFunc = function()
                -- we expect the packet that switched packet from RingA to RingB will update the <correction field>
                egress_ports_with_correction_field_changed[ringB].correction_field_updated = true
                -- we want the packet the packet from the CPU to RingA to set the <origin timestamp>
                -- but we send packet with PTP domain '3' that was set in the config file for RingA as update 'correction field'
                -- and not as 'origin timestamp' , so we need to change the config of RingA to 'add time' on this domain
                ptpSetPortAction(ringA,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E")--action
            end,
         postTestFunc = function()
                --restore
                egress_ports_with_correction_field_changed[ringB].correction_field_updated = false
                ptpSetPortAction(ringA,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E")--action
            end,
    },
    {packetNum = "ptp-Sync-B-to-A-transparet-clock" , srcPort = ringB , srcMac = macB , dstMac = macC , SeqNr = 6000 , HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "Ring B to Ring A + update the <correction field>",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xAB8D},
         preTestFunc = function()
                -- we expect the packet that switched packet from RingB to RingA will update the <correction field>
                egress_ports_with_correction_field_changed[ringA].correction_field_updated = true
                -- we want the packet the packet from the CPU to RingB to set the <origin timestamp>
                -- but we send packet with PTP domain '3' that was set in the config file for RingB as update 'correction field'
                -- and not as 'origin timestamp' , so we need to change the config of RingB to 'add time' on this domain
                ptpSetPortAction(ringB,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E")--action
            end,
         postTestFunc = function()
                --restore
                egress_ports_with_correction_field_changed[ringA].correction_field_updated = false
                ptpSetPortAction(ringB,domainTransparentClock,
                    "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E",--messageType
                    "CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E")--action
            end,
    },
    {packetNum = "ptp-Follow_up-from-A-capture" , srcPort = ringA , srcMac = macB , dstMac = macC , SeqNr = 6000 , HsrTag = true , vlanTag = false  , trgPort = ringB , remark = "none",
         PtpInfo = {messageType = "Follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB0D},
    },
    {packetNum = "ptp-Follow_up-from-CPU-to-B" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = 8000 , HsrTag = true , vlanTag = false  , trgPort = ringB  , remark = "none",
         PtpInfo = {messageType = "Follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xAB1D},
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
    {packetNum = "ptp-Follow_up-from-B-capture" , srcPort = ringB , srcMac = macB , dstMac = macC , SeqNr = 6000 , HsrTag = true , vlanTag = false  , trgPort = ringA , remark = "none",
         PtpInfo = {messageType = "Follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E055",
                sequenceId = 0xAB1D},
    },
    {packetNum = "ptp-Follow_up-from-CPU-to-A" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = 8000 , HsrTag = true , vlanTag = false  , trgPort = ringA  , remark = "none",
         PtpInfo = {messageType = "Follow_up" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = domainTransparentClock,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB0D},
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    },
}
local outgoingPacketsDb_ptp_BC_Ordinary_2_steps = {
    ["ptp-Sync-A-to-B-transparet-clock"] = {
        {trgPort = ringB  , PtpInfo = { messageType = "Sync"} , SeqNr = 6000   , HsrTag = true , vlanTag = false},
        {trgPort = interlink, notEgress = true},--filtered by 'srcId' as part of 'non-modeX'
    },
    ["ptp-Sync-B-to-A-transparet-clock"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "Sync"} , SeqNr = 6000   , HsrTag = true , vlanTag = false},
        {trgPort = interlink, notEgress = true},
    },
    ["ptp-Follow_up-from-A-capture"] = {
        {trgPort = portCPU  , PtpInfo = { messageType = "Follow_up"} , SeqNr = 6000   , HsrTag = true , vlanTag = false},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true},
    },
    ["ptp-Follow_up-from-CPU-to-B"] = {
        {trgPort = portCPU  , PtpInfo = { messageType = "Follow_up"} , SeqNr = 8000   , HsrTag = true , vlanTag = false},
        {trgPort = ringA    , notEgress = true},
        {trgPort = interlink, notEgress = true},
    },
    ["ptp-Follow_up-from-B-capture"] = {
        {trgPort = portCPU  , PtpInfo = { messageType = "Follow_up"} , SeqNr = 6000   , HsrTag = true , vlanTag = false},
        {trgPort = ringA    , notEgress = true},
        {trgPort = interlink, notEgress = true},
    },
    ["ptp-Follow_up-from-CPU-to-A"] = {
        {trgPort = portCPU  , PtpInfo = { messageType = "Follow_up"} , SeqNr = 8000   , HsrTag = true , vlanTag = false},
        {trgPort = ringB    , notEgress = true},
        {trgPort = interlink, notEgress = true},
    },
}

local incomingPacketsDb_ptp_with_FROM_CPU_DSA_2_steps___not_recognized_as_PTP = {
    {packetNum = "ptp-Sync-A" , srcPort = portCPU , srcMac = macCpu , dstMac = macB , SeqNr = 6666 , HsrTag = true , vlanTag = true  , trgPort = ringA , remark = "from CPU eDSA (16B) + vlan tag (4B) + hsr tag (6B) + PTP --> device fail to recognized as PTP",
         PtpInfo = {messageType = "Sync" , transportSpecific = 0xF , controlField = 0x9 , logMessageInterval = 0x77 , domainId = 2,
                correctionField_frac_nano = 0x1234 , correctionField_ms_nano = 0x1122 , correctionField_ls_nano = 0x33445566 ,
                sourcePortIdentify = "A011B011C022D022E044",
                sequenceId = 0xAB8D},
     postTestFunc = ptpPrepareFollowUpTodInfo_2_steps ,
     postTestParams = {trgPort=ringA,domainId=2,messageType="Sync",sequenceId=0xAB8D,expectNotPtp=true},
     noForceCapture = true,--we want the PTP per message type per port per domain will capture to the CPU
    }
}

local outgoingPacketsDb_ptp_with_FROM_CPU_DSA_2_steps___not_recognized_as_PTP = {
    ["ptp-Sync-A"] = {
        {trgPort = ringA  , PtpInfo = { messageType = "Sync"} , SeqNr = 6666   , HsrTag = true , vlanTag = true},
        {trgPort = ringB  , notEgress = true},
        {trgPort = interlink, notEgress = true}
    },
}


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

--[[ allow to disable mode X
the disable done by :
     1. use iPCL 1 rule to set proper 'SrcId' indications (matched on ring ports with PTP packet):
         a. bit 11 : to state : IGNORE_SOURCE_DUPLICATE_DISCARD
         b. value 15 in bits 0..3 : to define group to filter the interlink
     2. state the SrcId group of value (0x80f) not allow to egress from the interlink
the enable done by :
    1. delete the iPCL 1 rule
    2. restore SrcId group of value (0x80f) to allow the interlink
]]
local srcIdFor_IGNORE_SOURCE_DUPLICATE_DISCARD_and_for_filter_interlink = 0x80f
local restored_modeX_part1 = false
local function modeX(enable,part)

    if not enable then -- disable
        executeStringCliCommands(
        [[
            do configure

            //set IPCL to work with udb30 for 'isPtp' in metadata
            pcl-config ${dev} ipcl-metadata ptp

            interface ethernet ${dev}/${ringA}
            service-acl pcl-ID 30 lookup second
            exit

            interface ethernet ${dev}/${ringB}
            service-acl pcl-ID 31 lookup second
            exit

            access-list device ${dev} pcl-ID 30
            rule-id 42 action permit isPtp true src-id ]] .. srcIdFor_IGNORE_SOURCE_DUPLICATE_DISCARD_and_for_filter_interlink ..[[

            exit

            access-list device ${dev} pcl-ID 31
            rule-id 48 action permit isPtp true src-id ]] .. srcIdFor_IGNORE_SOURCE_DUPLICATE_DISCARD_and_for_filter_interlink ..[[

            exit

            exit

            do cpss-api call cpssDxChBrgSrcIdGroupPortDelete devNum ${dev} sourceId ]].. srcIdFor_IGNORE_SOURCE_DUPLICATE_DISCARD_and_for_filter_interlink .. " portNum ${interlink} " .. [[

        ]]
        )
    else
        enable_part = {
            [1] = [[
            do configure

            access-list device ${dev} pcl-ID 30
            delete rule-id 42
            exit

            access-list device ${dev} pcl-ID 31
            delete rule-id 48
            exit

            exit

            do cpss-api call cpssDxChBrgSrcIdGroupPortAdd devNum ${dev} sourceId ]].. srcIdFor_IGNORE_SOURCE_DUPLICATE_DISCARD_and_for_filter_interlink .. " portNum ${interlink} " .. [[

        ]],

        [2] = [[
            do configure

            interface ethernet ${dev}/${ringA}
            no service-acl pcl-ID 30
            exit

            interface ethernet ${dev}/${ringB}
            no service-acl pcl-ID 31
            exit

            delete access-list device ${dev} pcl-id 30
            delete access-list device ${dev} pcl-id 31

            //restore IPCL to work with udb30 for 'tpid-index' (default)
            no pcl-config ${dev} ipcl-metadata

            exit
        ]]

        }

        executeStringCliCommands(enable_part[part])

        restored_modeX_part1 = (part == 1) and true or false
    end

end

local ingressLanId = 0

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

local function doMainLogic_perCase(ingressInfo,OUT_Packets,catagory)
    local caseName = "Send a packet #".. ingressInfo.packetNum .. " , note : ".. ingressInfo.remark

    if catagory then
        caseName = catagory .. ": " .. caseName
    end

    printLog("========================================")
    printLog(caseName)

    ingressLanId = (ingressLanId + 1) % 16

    if(1 == (ingressLanId % 2)) then
        ingressPayload = payload_short
    else
        ingressPayload = payload_long
    end

    local ingressLsduSize = calcLsduSize(ingressPayload,ingressInfo.PtpInfo--[[only PtpInfo.messageType needed]])

    transmitInfo.portNum            = ingressInfo.srcPort

    local forceCaptureOnTransmitPort = false

    if (ingressInfo.srcPort == portCPU) then
        -- we send from the CPU to 'target port'
        if ingressInfo.trgPort ~= "vidx" then
            transmitInfo.portNum            = ingressInfo.trgPort
        end
        if not ingressInfo.noForceCapture then
            forceCaptureOnTransmitPort      = true
        end
    end


    if ingressInfo.wrongLsduSize then
        ingressLsduSize = ingressLsduSize + 0x110
    end
--[[
build packet info:
     l2 = {srcMac = , dstMac = }
    ,hsrTag = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
    ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
    ,payload =
    ,dsaTag = {vidx = }
]]
    local packetInfo = {
    l2 = {srcMac = ingressInfo.srcMac, dstMac = ingressInfo.dstMac}
    ,hsrTag = { exists = ingressInfo.HsrTag , SeqNr = ingressInfo.SeqNr , lanId = ingressLanId ,lsduSize = ingressLsduSize}
    ,vlanTag = {exists = ingressInfo.vlanTag , vlanId = 1, cfi = 0, vpt = 0}
    ,payload = ingressPayload
    ,dsaTag = ingressInfo.dsaTag
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

    if(ingressInfo.preTestFunc)then
        doFunction(ingressInfo.preTestFunc,ingressInfo.preTestParams)
    end

    --printLog("fullPacket=" ,to_string(transmitInfo.pktInfo.fullPacket));

    egressInfoTable = {}
    local index = 1
    for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do

        local egressLanId,egressLsduSize

        if ingressInfo.srcPort == ringA or  ingressInfo.srcPort == ringB then
            egressLanId = ingressLanId -- keep the same LanId
            egressLsduSize = ingressLsduSize
        else
            egressLanId = 0 -- egress with lanId = 0 if came from interlink
            egressLsduSize = calcLsduSize(ingressPayload,egressInfo.PtpInfo--[[only PtpInfo.messageType needed]])
        end


        egressInfoTable[index] = {}

        egressInfoTable[index].portNum = egressInfo.trgPort

        if egressInfo.trgPort == portCPU then
            egressInfoTable[index].portNum = "CPU"
        end

        if (ingressInfo.srcPort == portCPU and
            egressInfo.trgPort == ingressInfo.trgPort) then
            egressInfoTable[index].portNum = "CPU"
        end

        if egressInfoTable[index].portNum == "CPU" and
           ingressInfo.HsrTag and egressInfo.HsrTag and (ingressInfo.trgPort == ringA or  ingressInfo.trgPort == ringB) then
            egressLanId = ingressLanId -- keep the same LanId
            egressLsduSize = ingressLsduSize
        end
        --[[
        build packet info:
             l2 = {srcMac = , dstMac = }
            ,hsrTag = { exists = true/false , SeqNr = , lanId = ,lsduSize = }
            ,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
            payload =
            ,PtpInfo = ingressInfo.PtpInfo
        ]]
        local packetInfo = {
            l2 = {srcMac = ingressInfo.srcMac, dstMac = ingressInfo.dstMac}--[[the L2 kept]]
           ,hsrTag = { exists = egressInfo.HsrTag , SeqNr = egressInfo.SeqNr , lanId = egressLanId ,lsduSize = egressLsduSize}
           ,vlanTag = {exists = egressInfo.vlanTag , vlanId = 1, cfi = 0, vpt = 0}
           ,payload = ingressPayload--[[the payload kept]]
           ,dsaTag = egressInfo.dsaTag
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


            if egressInfo.noPacketCompare == true then
                -- we not care about the packet content and we not compare it
                -- we only want to make sure it egress the port
                egressInfoTable[index].pktInfo = nil
                egressInfoTable[index].packetCount = 1
            else
                egressInfoTable[index].pktInfo = {fullPacket = egressFullPacket , mustNotBeEqualBytesTable = my_mustNotBeEqualBytesTable}
                egressInfoTable[index].packetCount = nil
            end
        end

        index = index + 1


        end -- end of loop on egress ports

        egressInfoTable[index] = {}
        egressInfoTable[index].portNum = otherPort
        egressInfoTable[index].packetCount = 0  -- check that not getting traffic
        egressInfoTable[index].pktInfo = nil

        if (clearDdeNotOk) and ingressInfo.HsrTag and transmitInfo.inBetweenSendFunc --[[clear dde]] then
            -- the GM hold cache that is not removed by the 'clearDDE' function
            -- therefor we should get drop on the second ring port
            local numberOfCaptures = 0
            local index = 1
            for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do

                if egressInfoTable[index].pktInfo then
                    numberOfCaptures = numberOfCaptures + 1
                end

                index = index + 1
            end

            index = 1

            for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do

                if egressInfoTable[index].pktInfo and numberOfCaptures > 1 then
                    egressInfoTable[index].packetCount = 1   -- only to count once
                    egressInfoTable[index].pktInfo     = nil -- and not send second iteration

                    numberOfCaptures = numberOfCaptures - 1
                end

                index = index + 1
            end

        end


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

        --printLog("transmitInfo", to_string(transmitInfo))
        --printLog("egressInfoTable", to_string(egressInfoTable))
        if isEmulatorUsed() and transmitInfo.inBetweenSendFunc and  ingressInfo.force_no_change_timeout then
            local index = 1
            for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do

                if egressInfo.trgPort == ringA then
                    egressInfoTable[index].packetCount = 1   -- only to count once
                    egressInfoTable[index].pktInfo     = nil -- and not send second iteration
                    break--[[remove the ringA that asked for]]
                end

                index = index + 1
            end
        end

        if isEmulatorUsed() and transmitInfo.inBetweenSendFunc and not ingressInfo.force_no_change_timeout then
           ddeSameKeyExpirationTimeSet(inBetween_ddeSameKeyExpirationTime)
        end

        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           transmitInfo, egressInfoTable)
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

        local rc1 = globalDdlCountersCheck(OUT_Packets[ingressInfo.packetNum],ingressInfo.srcPort)

        if rc ~= 0 or rc1 ~= 0 then
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
end -- end of function doMainLogic

local function doMainLogic(IN_Packets,OUT_Packets,catagory)
    for _dummy1, ingressInfo in pairs(IN_Packets) do
        if ingressInfo.skip_when_run_all and -- skip if run all
           (not skip_part_1 and not skip_part_2 and not skip_part_3) -- we run all
        then
            -- this case needed only when running partial parts (or single part)
        else
            doMainLogic_perCase(ingressInfo,OUT_Packets,catagory)
        end
    end
end

-------------- do the test --------------
executeLocalConfig("dxCh/examples/configurations/hsr_san.txt")
ddeSameKeyExpirationTimeSet(test_ddeSameKeyExpirationTime)

if not skip_part_1 then
    doMainLogic(incomingPacketsDb,outgoingPacketsDb)
else
    printLog("NOTE: Skip testing of main part of HSR")
end

if not skip_part_2 then
    executeLocalConfig("dxCh/examples/configurations/hsr_san_cpu.txt")

    doMainLogic(incomingPacketsDb_toFromCpu,outgoingPacketsDb_toFromCpu)

    executeLocalConfig("dxCh/examples/configurations/hsr_san_cpu_deconfig.txt")
else
    printLog("NOTE: Skip testing of TO/FROM CPU part of HSR")
end

-- indexes match those of ptp_test_info[] and the info is extention to ptp_test_info[]
local ptp_tests_extra_info = {
    [1] = {testFunc = function(catagory)
            doMainLogic(incomingPacketsDb_ptp_with_FORWARD_DSA,outgoingPacketsDb_ptp_with_FORWARD_DSA,catagory)
        end
    },
    [2] = {testFunc = function(catagory)
            doMainLogic(incomingPacketsDb_ptp_with_FROM_CPU_DSA,outgoingPacketsDb_ptp_with_FROM_CPU_DSA,catagory)
        end
    },
    [3] = {testFunc = function(catagory)
            -- Cancel Mode X by setting DescSrcID<IgnoreSourceDuplicateDisacrd>=1
            -- Filter the Interlink port by applying one of the global filtering
            modeX(false)--disable
            doMainLogic(incomingPacketsDb_ptp_BC_Ordinary,outgoingPacketsDb_ptp_BC_Ordinary,catagory)
            modeX(true,1--[[part1]])--enable (restore)
        end
    },
    [4] = {testFunc = function(catagory)
            running_ptp_in_2_steps = true
            executeLocalConfig("dxCh/examples/configurations/hsr_san_ptp_2_steps.txt")
            doMainLogic(incomingPacketsDb_ptp_with_FROM_CPU_DSA_2_steps,outgoingPacketsDb_ptp_with_FROM_CPU_DSA_2_steps,catagory)
            executeLocalConfig("dxCh/examples/configurations/hsr_san_ptp_2_steps_deconfig.txt")
            running_ptp_in_2_steps = false
        end
    },
    [5] = {testFunc = function(catagory)
            -- Cancel Mode X by setting DescSrcID<IgnoreSourceDuplicateDisacrd>=1
            -- Filter the Interlink port by applying one of the global filtering
            modeX(false)--disable
            running_ptp_in_2_steps = true
            executeLocalConfig("dxCh/examples/configurations/hsr_san_ptp_2_steps.txt")
            doMainLogic(incomingPacketsDb_ptp_BC_Ordinary_2_steps,outgoingPacketsDb_ptp_BC_Ordinary_2_steps,catagory)
            executeLocalConfig("dxCh/examples/configurations/hsr_san_ptp_2_steps_deconfig.txt")
            running_ptp_in_2_steps = false
            modeX(true,1--[[part1]])--enable (restore)
        end
    },
    [6] = {testFunc = function(catagory)
            running_ptp_in_2_steps = true
            executeLocalConfig("dxCh/examples/configurations/hsr_san_ptp_2_steps.txt")
            doMainLogic(incomingPacketsDb_ptp_with_FROM_CPU_DSA_2_steps___not_recognized_as_PTP,outgoingPacketsDb_ptp_with_FROM_CPU_DSA_2_steps___not_recognized_as_PTP,catagory)
            executeLocalConfig("dxCh/examples/configurations/hsr_san_ptp_2_steps_deconfig.txt")
            running_ptp_in_2_steps = false
        end
    },


}

if not skip_part_3 then
    executeLocalConfig("dxCh/examples/configurations/hsr_san_ptp.txt")

    for ii = 1, #ptp_test_info do
        local ptp_info = ptp_test_info[ii]
        if not ptp_info.skip then
            local ptp_extra_info = ptp_tests_extra_info[ii]
            local catagory = "PTP:" ..ptp_info.doc_section..": "..ptp_info.ptp_step .. " " .. ptp_info.clockType .. " DSA=" .. ptp_info.dsa

            ptp_extra_info.testFunc(catagory)
        end
    end -- end of loop on ptp_test_info

    if restored_modeX_part1 then
        modeX(true,2--[[part2]])--enable (restore)
    end

    executeLocalConfig("dxCh/examples/configurations/hsr_san_ptp_deconfig.txt")
else
    printLog("NOTE: Skip testing of PTP part of HSR")
end

ddeSameKeyExpirationTimeSet(2)-- restore the default after init
executeLocalConfig("dxCh/examples/configurations/hsr_san_deconfig.txt")
testPrintResultSummary(testName)
