--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ptp_utils.lua
--*
--* DESCRIPTION:
--*       functions tests that using PTP v2 header.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
local devNum  = devEnv.dev

local PtpEtherTypeStr = "88f7" -- ethertype 16 bits
local ptpVersion = "2"                --4 bits

local ptpMessageHeaderLength = 34 -- without additional body
local ptpOriginTimeStampLength = 10

--[[local]] defaultDomainNumber = 188        --8 bits

--[[local]] domainTransparentClock = 3
--[[local]] running_ptp_in_2_steps = false
-- when running_ptp_in_2_steps == true
-- for the 'follow up' the 10 bytes of TOD need to be read from the switch from the egress mac/CTSU that captured the 'Synch'/'PDelay_Response'
local ptp_10_bytes_TOD_for_follow_up = nil--must be set in runtime after the synch sent out

--indication that the MAC knows to put the correction time in the queue
local isMacCaptureSupported = not isGmUsed() -- WM and Emulator support it

local domainId_to_high_seconds = {
    -- was set by cpssDxChPtpManagerTodValueSet(...)
    [0] = {time32MSBits = "EEEEEEEE",purpose="BC(boundary-clock)"},
    [1] = {time32MSBits = "DDDDDDDD",purpose="BC(boundary-clock)"},
    [2] = {time32MSBits = "CCCCCCCC",purpose="BC(boundary-clock)"},
    [domainTransparentClock] = {time32MSBits = "BBBBBBBB",purpose="TC(transparet-clock)"},
}

local function ptp_Sync(PtpInfo,ptpStartOffset)
    if running_ptp_in_2_steps and PtpInfo.domainId ~= domainTransparentClock then
        -- the 'Synch' Packet goes 'AS IS' without modifications
        -- so no mustNotBeEqualBytesTable
        -- lets reset the 10 bytes of Origin TimeStamp
        return "00000000000000000000" --[[10 bytes of Origin TimeStamp]]
            ,nil
    end

    local startByte = ptpStartOffset + ptpMessageHeaderLength
    --[[
        the bytes that we set on the ingress packet are 'dummy' and need to be changed
        on the egress by the TOD of the proper TAI

        the upper bytes we use highSeconds
    ]]
    local mustNotBeEqualBytesTable = {
        {startByte = startByte+4, endByte = startByte+9 ,reason = "6 LSBytes Origin TimeStamp (set by egress MAC)"}
    }

    if isGmUsed() then 
        -- the GM not have DP/MAC units to update the PTP related fields
        mustNotBeEqualBytesTable = nil 
    end

    local highSeconds = domainId_to_high_seconds[PtpInfo.domainId].time32MSBits or "00000000"

    return highSeconds .. "bbaa99887766" --[[10 bytes of Origin TimeStamp]]
           ,mustNotBeEqualBytesTable
end
local function ptp_Pdelay_request(PtpInfo,ptpStartOffset)
-- no mask needed as the packet trapped to CPU without PTP actions that modify the packet
    return "56789abcef0123456789" .. --[[10 bytes of Origin TimeStamp]]
           "00000000000000000000"         -- Reserved
           ,nil
end
local function ptp_Pdelay_Response(PtpInfo,ptpStartOffset)
    if running_ptp_in_2_steps then
        -- the 'Pdelay_Response' Packet goes 'AS IS' without modifications
        -- so no mustNotBeEqualBytesTable
        -- lets reset the 10 bytes of Receive Receipt TimeStamp
        return "00000000000000000000" .. --[[10 bytes of Receive Receipt TimeStamp]]
               "56789abcef0123456789"    --[[10 bytes of Pequesting Port Identity]]
               ,nil
    end
    local startByte = ptpStartOffset + ptpMessageHeaderLength
    --[[
        the lower bytes that we set on the ingress packet are 'dummy' and need to be changed
        on the egress by the TOD of the proper TAI

        the upper bytes we use highSeconds
    ]]
    local mustNotBeEqualBytesTable = {
        {startByte = startByte+4, endByte = startByte+9 ,reason = "6 LSBytes Origin TimeStamp (set by egress MAC)"}
    }

    if isGmUsed() then 
        -- the GM not have DP/MAC units to update the PTP related fields
        mustNotBeEqualBytesTable = nil 
    end

    local highSeconds = domainId_to_high_seconds[PtpInfo.domainId].time32MSBits or "00000000"

    return  highSeconds .. "bbaa99887766" .. --[[10 bytes of Receive Receipt TimeStamp]]
            "56789abcef0123456789"    --[[10 bytes of Pequesting Port Identity]]
            ,mustNotBeEqualBytesTable
end

--expected only in running_ptp_in_2_steps == true
local function ptp_Follow_up(PtpInfo,ptpStartOffset)
    if ptp_10_bytes_TOD_for_follow_up == nil then
        -- we emulate Follow_up that we get from remote CPU , on TC domain
        return "334455667788991122aa"
    end
    
    -- need to return the 10 bytes of TOD that was captured by the MAC/CTSU , during the send of the preious 'Sync'
    return ptp_10_bytes_TOD_for_follow_up
end
--expected only in running_ptp_in_2_steps == true
local function ptp_PDelay_Response_follow_up(PtpInfo,ptpStartOffset)
    -- need to return the 10 bytes of TOD that was captured by the MAC/CTSU , during the send of the preious 'PDelay_Response'
    return ptp_10_bytes_TOD_for_follow_up ..
           "56789abcef0123456789"    --[[10 bytes of Pequesting Port Identity]]
end

--[[local]] ptp_messageTypes = {
    ["Sync"]                       = {hex="0", length = ptpMessageHeaderLength + ptpOriginTimeStampLength , func = ptp_Sync},
    ["Delay_request"]              = {hex="1", length = "NA"},
    ["PDelay_request"]             = {hex="2", length = ptpMessageHeaderLength + ptpOriginTimeStampLength + 10 --[[reserved]], func = ptp_Pdelay_request},
    ["PDelay_Response"]            = {hex="3", length = ptpMessageHeaderLength + ptpOriginTimeStampLength + 10 --[[requestingPortIdentity]], func = ptp_Pdelay_Response},
    ["Reserved4"]                  = {hex="4", length = "NA"},
    ["Reserved5"]                  = {hex="5", length = "NA"},
    ["Reserved6"]                  = {hex="6", length = "NA"},
    ["Reserved7"]                  = {hex="7", length = "NA"},
    ["Follow_up"]                  = {hex="8", length = ptpMessageHeaderLength + ptpOriginTimeStampLength, func = ptp_Follow_up},
    ["Delay_response"]             = {hex="9", length = "NA"},
    ["PDelay_Response_follow_up"]  = {hex="A", length = ptpMessageHeaderLength + ptpOriginTimeStampLength + 10 --[[requestingPortIdentity]], func = ptp_PDelay_Response_follow_up},
    ["Announce"]                   = {hex="B", length = "NA"},
    ["Signaling"]                  = {hex="C", length = "NA"},
    ["Management"]                 = {hex="D", length = "NA"},
    ["Reserved14"]                 = {hex="E", length = "NA"},
    ["Reserved15"]                 = {hex="F", length = "NA"},
}

local function setZeroIfNil(param)
    if not param then return 0 end
    return param
end
--[[
    PTP fields:
    transportSpecific           --4 bits
    messageType                 --4 bits
    messageLength               --16 bits --value is 34 + payloadLength
    flagField                   --16 bits
    correctionField_frac_nano   --16 bits
    correctionField_ms_nano     --16 bits
    correctionField_ls_nano     --32 bits
    sourcePortIdentify          -- if exists : must be string(20 nibbles)
    sequenceId                  --16 bits
    controlField                --8 bits
    logMessageInterval          --8 bits

    domainId -- used as 'offset' from default of value 188 (0xBC)
]]
function buildPtpTag(PtpInfo,payloadLength,ptpStartOffset) -- 2 bytes ethertype + 34 bytes header
    local transportSpecific = string.format("%1.1x",setZeroIfNil(PtpInfo.transportSpecific))--4 bits
    local messageType = setZeroIfNil(PtpInfo.messageType)
    local messageLength
    local flagField  = string.format("%4.4x",setZeroIfNil(PtpInfo.flagField))--16 bits
    local correctionField_frac_nano  = string.format("%4.4x",setZeroIfNil(PtpInfo.correctionField_frac_nano)) --16 bits
    local correctionField_ms_nano  = string.format("%4.4x",setZeroIfNil(PtpInfo.correctionField_ms_nano)) --16 bits
    local correctionField_ls_nano  = string.format("%8.8x",setZeroIfNil(PtpInfo.correctionField_ls_nano)) --32 bits
    local sourcePortIdentify = PtpInfo.sourcePortIdentify-- must be string(20 nibbles)
    local sequenceId = string.format("%4.4x",setZeroIfNil(PtpInfo.sequenceId)) --16 bits
    local controlField  = string.format("%2.2x",setZeroIfNil(PtpInfo.controlField))--8 bits
    local logMessageInterval = string.format("%2.2x",setZeroIfNil(PtpInfo.logMessageInterval))--8 bits
    local postHeaderFunc = function() return "x",nil end 
    local domainNumber = string.format("%2.2x",defaultDomainNumber + setZeroIfNil(PtpInfo.domainId))--8 bits

    if ptp_messageTypes[messageType] then -- support string
        --printLog("ptp_messageTypes[messageType]=",to_string(ptp_messageTypes[messageType]))
        messageLength = string.format("%4.4x",ptp_messageTypes[messageType].length + payloadLength)--16 bits --[[value is 34 + payloadLength]]
        postHeaderFunc = ptp_messageTypes[messageType].func
        messageType = ptp_messageTypes[messageType].hex
    else -- support number 0..15
        printLog("ERROR : unknown messageType :",to_string(messageType))
        messageLength = string.format("%4.4x",34 + payloadLength)--16 bits --[[value is 34 + payloadLength]]
        messageType = string.format("%1.1x",messageType)--4 bits
    end

    if not sourcePortIdentify then sourcePortIdentify = "00000000000000000000" end --10 bytes (20 nibbles)

    ptpStartOffset = ptpStartOffset + 2--[[the caller did not skip the ethertype of the PTP header]]
    local postHeader,my_mustNotBeEqualBytesTable = postHeaderFunc(PtpInfo,ptpStartOffset)

    -- each line here aligned to 2 bytes
    return
        PtpEtherTypeStr ..
        transportSpecific .. messageType .. "0" --[[reserved]].. ptpVersion ..  --Bytes 0,1
        messageLength ..                            --Bytes 2,3
        domainNumber .. "00"--[[reserved]] ..       --Bytes 4,5
        flagField ..                                --Bytes 6,7
        correctionField_ms_nano ..                  --Bytes 8,9
        correctionField_ls_nano ..                  --Bytes 10,11,12,13
        correctionField_frac_nano ..                --Bytes 14,15
        "00000000" --[[reserved]] ..                --Bytes 16,17,18,19
        sourcePortIdentify ..                       --Bytes 20..29
        sequenceId ..                               --Bytes 30,31
        controlField .. logMessageInterval ..       --Bytes 32,33
        postHeader                                  -- more bytes of the specific message type

        ,my_mustNotBeEqualBytesTable

end

local command_data = Command_Data()

local billion = 1000000000
local half_billion = billion / 2

local function printError(errorStr)
    printLog(errorStr)
    testAddErrorString(errorStr)
end

local function checkCapturedTime(timeInfo,name)
    local errorStr = nil

    if timeInfo.seconds > 3 then
        errorStr = name .. " ERROR : captured seconds > 3 , got " .. to_string(timeInfo.seconds)
        printError(errorStr)
    end
    if timeInfo.nanoSeconds >= billion then
        errorStr = name .. " ERROR : captured nanoSeconds >= 1000000000 , got " .. to_string(timeInfo.nanoSeconds)
        printError(errorStr)
    end
end

--the fucntion is called in '2 steps' mode , after the 'Synch' / 'PDelay_Response'
--the function sets the 10 bytes of ptp_10_bytes_TOD_for_follow_up
--info with the info : {trgPort=ringA,domainId=2,messageType="Sync",sequenceId=0xAB8D}
--the function checks that the TOD info is in the queues of : ERMRK,CTSU,MAC
function ptpPrepareFollowUpTodInfo_2_steps(info)
    local timeInfo = {seconds = 0 , nanoSeconds = 0}
    local queueNum = 0 --???
    local erep_queueEntryId = 0
    local domainNumber = defaultDomainNumber + info.domainId --8 bits
    --read from ERMRK
    local apiName = "cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead"
    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U32",    "queueNum",    queueNum },
        { "OUT",    "CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC", "ermrkInfo"},
    })
    if not isError and values and values.ermrkInfo and values.ermrkInfo.entryValid == true then
        local ermrkInfo = values.ermrkInfo

        if (info.expectNotPtp == true) then
            isError = true
            printLog("ERMRK info : ERROR : mismatch ",to_string(ermrkInfo) , " NOT expected packet to be recognized as PTP , therfore not to capture")
        end
        if ermrkInfo.portNum ~= info.trgPort then
            isError = true
            printLog("ERMRK info : ERROR : mismatch ",to_string(ermrkInfo) , "expected portNum = " .. to_string(info.trgPort))
        end
        if ermrkInfo.domainNum ~= domainNumber then
            isError = true
            printLog("ERMRK info : ERROR : mismatch ",to_string(ermrkInfo) , "expected domainId = " .. to_string(domainNumber))
        end
        if ermrkInfo.messageType ~= tonumber("0x"..ptp_messageTypes[info.messageType].hex,16) then
            isError = true
            printLog("ERMRK info : ERROR : mismatch ",to_string(ermrkInfo) , "expected messageType = " .. to_string(info.messageType))
        end
        if ermrkInfo.sequenceId ~= info.sequenceId then
            isError = true
            printLog("ERMRK info : ERROR : mismatch ",to_string(ermrkInfo) , "expected sequenceId = " .. to_string(info.sequenceId))
        end
        if ermrkInfo.isPtpException ~= false then
            isError = true
            printLog("ERMRK info : ERROR : mismatch ",to_string(ermrkInfo) , "expected isPtpException = false")
        end

        if ermrkInfo.packetFormat ~= "CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E" then
            printLog("ERMRK info : ERROR : mismatch ",to_string(ermrkInfo) , "expected packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E")
            isError = true
        end

        erep_queueEntryId = ermrkInfo.queueEntryId
        if isError then
            printError("ERMRK capture mismatch on port [" .. tostring(info.trgPort) .. "],messageType= [" .. info.messageType .. "]")
        else
            printLog("ERMRK info: as expected values :",to_string(ermrkInfo))
        end

    elseif (info.expectNotPtp == true) then
        printLog("ERMRK info : GOOD : NOT expected packet to be recognized as PTP , therfore not to capture on ERMRK ")
    else
        printError("ERMRK not captured on port [" .. tostring(info.trgPort) .. "],messageType= [" .. info.messageType .. "]")
        isError = true
    end


    if isGmUsed() then
        -- dummy value as GM not have the CTSU
        ptp_10_bytes_TOD_for_follow_up = "00112233445566778899"
        return
    end

    -- Read from the CTSU
    local apiName = "cpssDxChPtpManagerEgressTimestampPortQueueEntryRead"
    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",  info.trgPort},
        { "IN",     "GT_U32",    "queueNum",    queueNum },
        { "OUT",    "CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC", "ctsuInfo"},
    })

    if not isError and values and values.ctsuInfo and values.ctsuInfo.entryValid then
        local ctsuInfo = values.ctsuInfo

        printLog("CTSU Info:",to_string(ctsuInfo))

        if (info.expectNotPtp == true) then
            isError = true
            printLog("CTSU info : ERROR : mismatch ",to_string(ctsuInfo) , " NOT expected packet to be recognized as PTP , therfore not to capture")
        end

        if ctsuInfo.queueEntryId ~= erep_queueEntryId then
            isError = true
            printLog("CTSU info : mismatch with ERMRK",to_string(ctsuInfo) , "expected queueEntryId = " .. to_string(erep_queueEntryId))
        end

        if ctsuInfo.taiSelect ~= info.domainId then
            isError = true
            printLog("CTSU info : ERROR : mismatch ",to_string(ctsuInfo) , "expected taiSelect = " .. to_string(info.domainId))
        end

        if isError == true then 
            printError("CTSU capture mismatch on port [" .. tostring(info.trgPort) .. "],messageType= [" .. info.messageType .. "]")
        end

        -- the capture hold the 'global time' and not only the 'correction field' if it was in single step
        timeInfo.seconds     = ctsuInfo.seconds
        timeInfo.nanoSeconds = ctsuInfo.nanoSeconds
        checkCapturedTime(timeInfo,"CTSU")

        --[[
        GT_U32 todUpdateFlag;
        GT_U32 seconds;
        GT_U32 nanoSeconds;
        ]]
    elseif (info.expectNotPtp == true) then
        printLog("CTSU info : GOOD : NOT expected packet to be recognized as PTP , therfore not to capture on CTSU ")
    else
        printError("CTSU not captured on port [" .. tostring(info.trgPort) .. "],messageType= [" .. info.messageType .. "]")
        isError = true
    end

    printLog("current time (after CTSU)",to_string(timeInfo))

    --Read from the Egress MAC
    local apiName = "cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead"
    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",  info.trgPort},
        { "OUT",    "CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC", "macInfo"},
    })

    if not isError and values and values.macInfo and values.macInfo.entryValid then
        local macInfo = values.macInfo

        printLog("MAC info : valid entry ",to_string(macInfo))

        if (info.expectNotPtp == true) then
            isError = true
            printLog("MAC info : ERROR : mismatch ",to_string(macInfo) , " NOT expected packet to be recognized as PTP , therfore not to capture on the MAC")
        end

        -- the capture hold the 'global time' and not only the 'correction field' if it was in single step
        timeInfo.seconds     = macInfo.seconds
        timeInfo.nanoSeconds = macInfo.nanoSeconds
        
        checkCapturedTime(timeInfo,"MAC")

        if isError == true then 
            printError("MAC capture mismatch on port [" .. tostring(info.trgPort) .. "],messageType= [" .. info.messageType .. "]")
        end        
        --[[
            CPSS_DXCH_PTP_MANAGER_SIGNATURE_UNT signature;
            GT_U32 seconds;
            GT_U32 nanoSeconds;
        ]]        
        
    elseif (info.expectNotPtp == true) then
        printLog("MAC info : GOOD : NOT expected packet to be recognized as PTP , therfore not to capture on the MAC ")
    else
        if isMacCaptureSupported then
            isError = true 
            printError("MAC not captured on port [" .. tostring(info.trgPort) .. "],messageType= [" .. info.messageType .. "]")
        else
            printLog("MAC info : WARNING : the MAC capture of timestamp not implemented yet ")
        end
    end

    if (info.expectNotPtp == true) then
        -- dummy value as PTP was not recognized
        ptp_10_bytes_TOD_for_follow_up = "00112233445566778899"
        return
    end

    printLog("current time (after CTSU and MAC)",to_string(timeInfo))

    local _16BitsSecMs , _32BitsSecMs , _32BitsNanoSeconds = 0x0011 , 0x22334455 , half_billion
    local saved_32BitsSecMs = _32BitsSecMs
    local tempNano = _32BitsNanoSeconds + timeInfo.nanoSeconds
    
    printLog("tempNano = "..to_string(tempNano))
    _32BitsSecMs = _32BitsSecMs + timeInfo.seconds
    
    if tempNano > billion then
        _32BitsSecMs = _32BitsSecMs + 1
        _32BitsNanoSeconds = tempNano - billion
    else
        _32BitsNanoSeconds = tempNano
    end
    
    if _32BitsSecMs < saved_32BitsSecMs then
        _16BitsSecMs = _16BitsSecMs + 1
    end
    
    -- Nano's  are in the 32 LSbits (4 bytes)
    -- seconds are in the 48 MSBits (6 bytes) , but we get only 2 bits from CTSU,MAC
    local nanoSeconds_STR   = string.format(     "%8.8x",_32BitsNanoSeconds )
    local seconds_STR       = string.format("%4.4x%8.8x",_16BitsSecMs,_32BitsSecMs  )

    ptp_10_bytes_TOD_for_follow_up = seconds_STR .. nanoSeconds_STR
    
    printLog("Calculated ptp_10_bytes_TOD_for_follow_up : "..ptp_10_bytes_TOD_for_follow_up)
    
    if string.len(ptp_10_bytes_TOD_for_follow_up) ~= 20 then
        printError("Error : calculating the 10 Bytes of ptp_10_bytes_TOD_for_follow_up on port [" .. tostring(info.trgPort) .. "],messageType= [" .. info.messageType .. "]")
    end
    
end

function ptpSetPortAction(portNum,domainIndex,messageType,action)
    local domainEntry =
    {
      ptpOverUdpIpv4TsEnable={ },
      ptpOverEhernetTsEnable=true,
      transportSpecificTsEnableBmp=65535,
      messageTypeTsEnableBmp=65535,
      ptpOverUdpIpv6TsEnable={ }
    }

    local apiName = "cpssDxChPtpManagerPortPacketActionsConfig"
    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8",    "devNum",  devNum },
        { "IN", "GT_U32",   "portNum", portNum },
        { "IN", "CPSS_DXCH_PTP_TAI_NUMBER_ENT",    "taiNumber",    "CPSS_DXCH_PTP_TAI_NUMBER_0_E"},
        { "IN", "GT_U32",   "domainIndex"                               , domainIndex },
        { "IN", "CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC",    "domainEntry",domainEntry},
        { "IN", "CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT",   "messageType",messageType},
        {"IN",  "CPSS_DXCH_PTP_TS_ACTION_ENT",              "action"     ,action}
    })
end
