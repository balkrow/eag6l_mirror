--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* types_eee.lua
--*
--* DESCRIPTION:
--*       define types for Serdes.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--********************************************************************************
--      types
--********************************************************************************
CLI_type_dict["type_sqlch"]={
	checker = CLI_check_param_number,
	min=0,
	max=310,
	complete = CLI_complete_param_number,
	help = "Threshold that trips the Squelch detector"
}

CLI_type_dict["type_ffeRes"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "Mainly controls the low frequency gain"
}

CLI_type_dict["type_ffeCap"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "Mainly controls the high frequency gain"
}

CLI_type_dict["type_dcGain"]={
	checker = CLI_check_param_number,
	min=0,
	max=255,
	complete = CLI_complete_param_number,
	help = "DC Gain"
}

CLI_type_dict["type_bandWidth"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE Band-width"
}

CLI_type_dict["short_channel"]={
    checker = CLI_check_param_number,
    min=0,
    max=1,
    complete = CLI_complete_param_number,
    help = "CTLE Short-channel"
}

CLI_type_dict["type_gainShape1"]={
	checker = CLI_check_param_number,
	min=0,
	max=3,
	complete = CLI_complete_param_number,
	help = "CTLE gain shape 1"
}

CLI_type_dict["type_gainShape2"]={
	checker = CLI_check_param_number,
	min=0,
	max=3,
	complete = CLI_complete_param_number,
	help = "CTLE gain shape 2"
}

CLI_type_dict["type_minLf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE minLf"
}

CLI_type_dict["type_maxLf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE maxLf"
}

CLI_type_dict["type_minHf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE minLf"
}

CLI_type_dict["type_maxHf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE maxHf"
}

CLI_type_dict["type_BfLf"]={
	checker = CLI_check_param_number,
	min=0,
	max=8,
	complete = CLI_complete_param_number,
	help = "DFE BfLf"
}

CLI_type_dict["type_BfHf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE BfHf"
}

CLI_type_dict["type_minPre1"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE min Pre1"
}

CLI_type_dict["type_maxPre1"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE max Pre1"
}

CLI_type_dict["type_minPre2"]={
	checker = CLI_check_param_number,
	min=-10,
	max=10,
	complete = CLI_complete_param_number,
	help = "DFE min Pre2"
}

CLI_type_dict["type_maxPre2"]={
	checker = CLI_check_param_number,
	min=-10,
	max=10,
	complete = CLI_complete_param_number,
	help = "DFE max Pre2"
}

CLI_type_dict["type_minPost"]={
	checker = CLI_check_param_number,
	min=-15,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE min Post"
}
CLI_type_dict["type_maxPost"]={
	checker = CLI_check_param_number,
	min=-15,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE max Post"
}

CLI_type_dict["type_gain1"]={
	checker = CLI_check_param_number,
	min=0,
	max=255,
	complete = CLI_complete_param_number,
	help = "DFE gain Tap1 strength"
}

CLI_type_dict["type_gain2"]={
	checker = CLI_check_param_number,
	min=0,
	max=255,
	complete = CLI_complete_param_number,
	help = "DFE gain Tap2 strength"
}

CLI_type_dict["type_laneNum"]={
	checker = CLI_check_param_number,
	min=0,
	max=7,
	complete = CLI_complete_param_number,
	help = "serdes lane number"
}

CLI_type_dict["type_post1"]={
	checker = CLI_check_param_number,
	min=-15,
	max=15,
	complete = CLI_complete_param_number,
	help = "Serdes Post-cursor"
}
CLI_type_dict["type_pre1"]={
	checker = CLI_check_param_number,
	complete = CLI_complete_param_number,
	help = "Serdes Pre-cursor"
}

CLI_type_dict["type_pre2"]={
	checker = CLI_check_param_number,
	min=-15,
	max=15,
	complete = CLI_complete_param_number,
	help = "Serdes Pre-cursor2"
}

-- serdes tx params--

function CLI_check_param_tx(param, name, desc)
    param = tonumber(param)
    if param == nil then
        return false, name .. " not a number"
    end
    if  desc.min ~= nil and desc.max ~= nil and ( param < desc.min or (param > desc.max and param ~= 0xFFFF)) then
        return false, name.." is out of range "..tostring(desc.min)..".."..tostring(desc.max)
    end
    if desc.min ~= nil and param < desc.min then
        return false, name.." less than a minimal value "..tostring(desc.min)
    end
    if desc.max ~= nil and (param > desc.max and param ~= 0xFFFF) then
        return false, name.." greater than a maximum value "..tostring(desc.max)
    end

    if tostring(name) == "post" or tostring(name) == "pre" then
       if is_sip_6() and not is_sip_6_10() and param % 2 ~= 0 then
            return false, " Must be even value "
        end
    end

    return true, param
end
function CLI_complete_param_sd_tx(str, name, desc)
    if is_sip_6_20() then
        if tostring(name) == "post" then
            desc.min = -16
            desc.max = 16
        elseif tostring(name) == "pre" then
            desc.min = -22
            desc.max = 22
        elseif tostring(name) == "pre2" then
            desc.min = -9
            desc.max = 9
        elseif tostring(name) == "pre3" then
            desc.min = nil
            desc.max = nil
        elseif tostring(name) == "amp" then
            desc.min = 35
            desc.max = 63
        end
    elseif is_sip_6_15() then
        if tostring(name) == "post" then
            desc.min = 0
            desc.max = 15
        elseif tostring(name) == "pre" then
            desc.min = 0
            desc.max = 15
        elseif tostring(name) == "pre2" then
            desc.min = nil
            desc.max = nil
        elseif tostring(name) == "pre3" then
            desc.min = nil
            desc.max = nil
        elseif tostring(name) == "amp" then
            desc.min = 0
            desc.max = 15
        end
    elseif is_sip_6_10() then
        if tostring(name) == "post" then
            desc.min = -35
            desc.max = 35
        elseif tostring(name) == "pre" then
            desc.min = -22
            desc.max = 22
        elseif tostring(name) == "pre2" then
            desc.min = -9
            desc.max = 9
        elseif tostring(name) == "pre3" then
            desc.min = nil
            desc.max = nil
        elseif tostring(name) == "amp" then
            desc.min = 0
            desc.max = 63
        end
    elseif is_sip_6() then
        if tostring(name) == "post" then
            desc.min = -18
            desc.max = 18
        elseif tostring(name) == "pre" then
            desc.min = -10
            desc.max = 10
        elseif tostring(name) == "pre2" then
            desc.min = -15
            desc.max = 15
        elseif tostring(name) == "pre3" then
            desc.min = -1
            desc.max = 1
        elseif tostring(name) == "amp" then
            desc.min = 0
            desc.max = 31
        end
    end
    if desc.min == nil and desc.max == nil then
    end

    help = "<"
    if    (nil ~= desc.min) and (nil ~= desc.max)   then
        help = help .. tostring(desc.min) .. "-" .. tostring(desc.max)
    elseif nil ~= desc.min                          then
        help = help .. tostring(desc.min) .. " and above"
    elseif nil ~= desc.max                          then
        help = help .. tostring(desc.max) .. " and below"
    else
        help = help .. " 0 not valid for device "
    end
    help = help .. ">"

    if type(desc.help) == "string" then
        help = help .. "   " .. desc.help
    end
    return {}, {def=help}
end

CLI_type_dict["type_amp"]={
	checker = CLI_check_param_number,
        name="amp",
	complete = CLI_complete_param_sd_tx,
	help = "Tx Amplitude/Attenuation of the signal"
}
CLI_type_dict["type_txpost"]={
	checker = CLI_check_param_tx,
        name="post",
	complete = CLI_complete_param_sd_tx,
	help = "Serdes Tx Post-cursor"
}

CLI_type_dict["type_txpre"]={
	checker = CLI_check_param_tx,
        name="pre",
	complete = CLI_complete_param_sd_tx,
	help = "Serdes Tx Pre-cursor1"
}

CLI_type_dict["type_txpre2"]={
	checker = CLI_check_param_tx,
        name="pre2",
	complete = CLI_complete_param_sd_tx,
	help = "Serdes Tx Pre-cursor2"
}

CLI_type_dict["type_txpre3"]={
	checker = CLI_check_param_tx,
        name="pre3",
	complete = CLI_complete_param_sd_tx,
	help = "Serdes Tx Pre-cursor3"
}

CLI_type_dict["type_rxtermination"]={
	checker = CLI_check_param_number,
	min=0,
	max=2,
	complete = CLI_complete_param_number,
	help = "Set Rx Termination"
}

CLI_type_dict["type_rxenvelope"]={
	checker = CLI_check_param_number,
	min=0,
	max=255,
	complete = CLI_complete_param_number,
	help = "Set Rx Envelope settings"
}
