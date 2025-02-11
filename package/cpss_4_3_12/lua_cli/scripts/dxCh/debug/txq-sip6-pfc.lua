--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* txq_pfc.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for PFC status
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
dofile("dxCh/debug/txq-sip6.lua")



local function dump_global_pfc_status(params)
    local res,val
    local device = tonumber(params["devID"])
    local tile =   tonumber(params["tileNum"])
    local dp = tonumber(params["dpNum"])

    if(is_sip_6(device))then
        res, val = myGenWrapper("prvCpssFalconTxqDebugQfcGlobalStatusGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "tileNum",  tile},
                { "IN", "GT_U32" , "qfcNum",  dp},
                { "OUT", "GT_U32", "pbLimitCrossedPtr"},
                { "OUT", "GT_U32", "pdxLimitCrossedPtr"},
                { "OUT", "GT_U32", "iaLimitCrossedPtr"},
                { "OUT", "GT_U32", "globalTcLimitCrossedBmpPtr"},
        })
        if(res==0)then
            print("PB limit crossed "..val["pbLimitCrossedPtr"])
            print("PDX limit crossed "..val["pdxLimitCrossedPtr"])
            print("IA limit crossed "..string.format( "0x%4.4X", val["iaLimitCrossedPtr"]))
            print("Global TC limit crossed ".. string.format( "0x%4.4X", val["globalTcLimitCrossedBmpPtr"] ))
        else
            print("Failed to get pfc status for device "..device ..  " tile "..tile.." dp "..dp)
        end
    else
        print("Device "..device ..  " is not supported since it is not SIP 6")
    end

end



local function dump_port_pfc_status(params)
    local res,val,i,j
    local devices,ports
    local tile,dp,localPort
    local phyPort

    print("")

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

     for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end
         for i=1,#ports do

            if(is_sip_6(devices[j]))then
                res, portMapShadow = myGenWrapper("cpssDxChPortPhysicalPortDetailedMapGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "CPSS_DXCH_DETAILED_PORT_MAP_STC", "portMapShadowPtr"},
                })

                if(res==0)then
                    phyPort = ports[i]
                    tile = portMapShadow["portMapShadowPtr"].extPortMap.tileId
                    dp = portMapShadow["portMapShadowPtr"].extPortMap.localDpInTile
                    localPort = portMapShadow["portMapShadowPtr"].extPortMap.localPortInDp
                    print("Mapped to tile "..tile..", dp "..dp..", local port "..localPort)
                else
                    print("Failed to get port to physical port mapping ")
                    return
                end

                res, val = myGenWrapper("prvCpssFalconTxqDebugQfcPortStatusGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "tileNum",  tile},
                        { "IN", "GT_U32" , "qfcNum",  dp},
                        { "IN", "GT_U32", "localPortNum",localPort},
                        { "OUT", "GT_U32", "portPfcStatusPtr"},
                        { "OUT", "GT_U32", "portTcPfcStatusBmpPtr"},
                })

                if(res==0)then
                    print("TXQ Port limit crossed "..val["portPfcStatusPtr"])
                    print("TXQ Port/TC limit crossed bitmap  "..string.format( "0X%2.2X", val["portTcPfcStatusBmpPtr"]))
                else
                    print("Failed to get pfc status for device "..device ..  " tile "..tile.." dp "..dp)
                    return
                end
                res, val = myGenWrapper("prvCpssFalconTxqQfcLocalPortToSourcePortGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "tileNum",  tile},
                        { "IN", "GT_U32" , "qfcNum",  dp},
                        { "IN", "GT_U32", "localPortNum",localPort},
                        { "OUT", "GT_U32", "sourcePortPtr"},
                        { "OUT", "CPSS_DXCH_PORT_FC_MODE_ENT", "fcModePtr"},
                })
                if(res==0)then
                    print("TXQ Source port mapped in QFC "..val["sourcePortPtr"])
                    print("TXQ FC mode  "..tostring(val["fcModePtr"]))
                else
                    print("Failed to get port to source port mapping ")
                    return
                end


                res, val = myGenWrapper("prvCpssDxChPortSip6XoffStatusGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  phyPort},
                        { "OUT", "GT_U32", "regAddrPtr"},
                        { "OUT", "GT_U32", "xoffStatusPtr"},
                })
                if(res==0)then
                    print("MAC TX XOFF status "..string.format( "0x%2.2X", val["xoffStatusPtr"]).." register address "..string.format( "0x%8.8X", val["regAddrPtr"]))
                else
                    print("Failed to get port TX XOFF status ")
                    return
                end

                res, val = myGenWrapper("prvCpssDxChPortSip6PfcRxPauseStatusGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  phyPort},
                        { "OUT", "GT_U32", "rxPauseStatusPtr"},
                        { "OUT", "GT_U32", "regAddrPtr"},
                })
                if(res==0)then
                    print("MAC RX XOFF status "..string.format( "0x%4.4X", val["rxPauseStatusPtr"]).." register adress "..string.format( "0x%8.8X", val["regAddrPtr"]))
                else
                    print("Failed to get port RX XOFF status ")
                    return
                end


                res, val = myGenWrapper("prvCpssDxChPortSip6PfcModeEnableGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  phyPort},
                        { "OUT", "GT_BOOL", "pfcModeEnablePtr"},
                        { "OUT", "GT_U32", "regAddrPtr"},
                })
                if(res==0)then
                    print("MAC COMMAND_CONFIG  PFC_MODE "..tostring(val["pfcModeEnablePtr"]).." register adress "..string.format( "0X%8.8X", val["regAddrPtr"]))
                else
                    print("Failed to get COMMAND_CONFIG  PFC_MODE ")
                    return
                end

                if(is_sip_6_10(device))then
                    res, val = myGenWrapper("prvCpssDxChPortPfcConfigMifEnableGet", {
                            { "IN", "GT_U8"  , "devNum", device},
                            { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum",phyPort},
                            { "OUT", "GT_BOOL", "txEnablePtr"},
                            { "OUT", "GT_BOOL", "rxEnablePtr"},
                    })
                    if(res==0)then
                        print("MIF PFC enable TX "..tostring(val["txEnablePtr"]))
                        print("MIF PFC enable RX "..tostring(val["rxEnablePtr"]))
                    else
                        print("Failed to get COMMAND_CONFIG  PFC_MODE ")
                        return
                    end
                end


            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end --if
        end --i=1,#ports do
    end --j=1,#devices do
    print("")
end



local function dump_pfcc_calendar(params)
    local res,val,pfccLastEntryIndex,ind,port,tcVector
    local device = tonumber(params["devID"])
    local tile =   tonumber(params["tileNum"])

    if(is_sip_6(device))then
        res, val = myGenWrapper("prvCpssDxChTxqDebugDumpPfccTableExt", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "tileNum",  tile},
                { "OUT", "GT_U32", "metadataPtr"},
        })
        if(res==0)then
            pfccLastEntryIndex = get_bit(val["metadataPtr"],0,9)
            print("pfccLastEntryIndex "..pfccLastEntryIndex)
            print("globalTcIndex "..get_bit(val["metadataPtr"],9,9))
            print("globalTcHrIndex "..get_bit(val["metadataPtr"],18,12))
            print("pfccEnable "..get_bit(val["metadataPtr"],30,1))
            print("isMaster "..get_bit(val["metadataPtr"],31,1))
            for ind=0,pfccLastEntryIndex do
                res, val = myGenWrapper("prvCpssFalconTxqPfccCfgTableEntryRawGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "tileNum",  tile},
                { "IN", "GT_U32" , "index",  ind},
                { "OUT", "GT_U32", "numOfBubblesPtr"},
                { "OUT", "GT_U32", "metaDataPtr"},
                })


                if(val["numOfBubblesPtr"]>0)then
                 print(ind.."  Bubble - size "..val["numOfBubblesPtr"])
                elseif(get_bit(val["metaDataPtr"],0,2)==1)then
                     print(ind.."  Global TC - "..get_bit(val["metaDataPtr"],8,4))
                elseif(get_bit(val["metaDataPtr"],0,2)==3)then
                     print(ind.."  Global TC HR - "..get_bit(val["metaDataPtr"],8,4))
                elseif(get_bit(val["metaDataPtr"],0,2)==0)then
                     port     = get_bit(val["metaDataPtr"],10,14)
                     tcVector = get_bit(val["metaDataPtr"],24,8)
                     print(ind.."  Port - "..port.." TC vector "..string.format( "0x%2.2x", tcVector))
                end
            end
        else
            print("Failed to get pfc status for device "..device ..  " tile "..tile)
        end
    else
        print("Device "..device ..  " is not supported since it is not SIP 6")
    end

end


local function dump_pool_status(params)
    local res,val
    local device = tonumber(params["devID"])
    local numberOfPools = 2
    local pool
    local mode
    local tc
    local maxDp,dpIter,pow
    local poolNum

    if(is_sip_6(device))then
        res, val = myGenWrapper("cpssDxChPortTxQPoolModeGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "OUT", "CPSS_PORT_TX_POOL_MODE_ENT", "modePtr"},
        })
        if(res==0)then
           print("Pool mode "..tostring(val["modePtr"]).."(cpssDxChPortTxQPoolModeGet)")
        else
           print("Failed to get Pool mode device."..device.."Error code "..res)
        return
        end
        if(val["modePtr"] ~="CPSS_PORT_TX_POOL_MODE_TWO_LOSSY_E") then
         numberOfPools =4
        end
        mode = val["modePtr"]
        print("\nPool occupancy (cpssDxChPortTxPoolBuffNumberGet)")
        for pool=0,numberOfPools-1 do
            if(mode =="CPSS_PORT_TX_POOL_MODE_ONE_LOSSY_TWO_LOSLESS_E") and pool==1 then
             print("Note: pool 1 is reserved  in mode CPSS_PORT_TX_POOL_MODE_ONE_LOSSY_TWO_LOSLESS_E")
            end
            res, val = myGenWrapper("cpssDxChPortTxPoolBuffNumberGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32"  , "devNum", pool},
                { "OUT", "GT_U32", "numPtr"},
            })
            if(res==0)then
             print("["..pool.."] "..val["numPtr"])
            else
             print("Failed to get Pool occupancy for device."..device..".Pool "..pool.."Error code "..res)
             return
            end
        end
        print("\nPool limit (cpssDxChPortTxSharedPoolLimitsGet)")
        for pool=0,1 do
            res, val = myGenWrapper("cpssDxChPortTxSharedPoolLimitsGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32"  , "poolNum", pool},
                { "OUT", "GT_U32", "maxBufNumPtr"},
                { "OUT", "GT_U32", "maxDescNumPtr"},
            })
            if(res==0)then
             print("["..pool.."] "..val["maxBufNumPtr"])
            else
             print("Failed to get Pool limit for device."..device..".Pool "..pool.."Error code "..res)
             return
            end
        end
        print("\nTC mapping (cpssDxChPortTxTcMapToSharedPoolGet)")
        for tc=0,7 do
            res, val = myGenWrapper("cpssDxChPortTxTcMapToSharedPoolGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U8"  , "tc", tc},
                { "OUT", "GT_U32", "poolNumPtr"}
            })
            if(res~=0)then
             print("Failed to get TC mapping for device."..device..".tc "..tc.."Error code "..res)
             return
            end

            poolNum = val["poolNumPtr"]

            res, val = myGenWrapper("cpssDxChPortPfcTcResourceModeGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U8"  , "tc", tc},
                { "OUT", "CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT", "modePtr"}
            })
            if(res==0)then
             print("["..tc.."] Pool "..poolNum.." resource mode "..tostring(val["modePtr"]))
            else
             print("Failed to get TC mapping for device."..device..".tc "..tc.."Error code "..res)
             return
            end
        end
        print("\nAvailable buffers Tail Drop(cpssDxChPortTxTailDropGlobalParamsGet)")
        res, val = myGenWrapper("cpssDxChPortTxTailDropGlobalParamsGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "OUT", "CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT"  , "resourceModePtr"},
                { "OUT", "GT_U32", "globalAvailableBuffersPtr"},
                { "OUT", "GT_U32", "pool0AvailableBuffersPtr"},
                { "OUT", "GT_U32", "pool1AvailableBuffersPtr"}
            })
        if(res==0)then
            print("PB  "..val["globalAvailableBuffersPtr"])
            print("[0] "..val["pool0AvailableBuffersPtr"])
            print("[1] "..val["pool0AvailableBuffersPtr"])
        else
           print("Failed to get Pool limit for device."..device..".Pool "..pool.."Error code "..res)
           return
        end

        res, val = myGenWrapper("prvCpssTxqSip6DebugGlobalDpNumGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "OUT", "GT_U32", "dpNumPtr"},
            })
        if(res==0)then
            print("This device contained DP number is    "..val["dpNumPtr"])
        else
           print("Failed to execute prvCpssTxqSip6DebugGlobalDpNumGet for device"..device.."Error code "..res)
           return
        end
        maxDp = val["dpNumPtr"]
        pow=1
        --go over all DPs
        print("\n PFC availeble buffers per DP mode CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_TC_E (cpssDxChPortPfcDbaTcAvailableBuffersGet)")
        for dpIter=0,maxDp-1 do
            for tc =0,7 do
                res, val = myGenWrapper("cpssDxChPortPfcDbaTcAvailableBuffersGet", {
                    { "IN", "GT_U8"  , "devNum", device},
                    { "IN", "GT_DATA_PATH_BMP"  , "dataPathBmp", pow},
                    { "IN", "CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT", "CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_TC_E"},
                    { "IN", "GT_U8", "tc",tc},
                    { "OUT", "GT_U32", "availableBuffersPtr"},
                })
                if(res==0)then
                 print("DP ["..dpIter.."] TC ["..tc.."] availeble buffers "..val["availableBuffersPtr"])
                else
                 print("Failed to get TC mapping for device."..device..".tc "..tc.."Error code "..res)
                 return
                end
            end
            pow=pow*2
            print("")
        end



    else
        print("Device "..device ..  " is not supported since it is not SIP 6")
    end

end


--------------------------------------------------------------------------------
---- command registration: pfc-status
--------------------------------------------------------------------------------

CLI_addHelp("debug", "txq-sip6-pfc", "Dump SIP6 txq pfc debug information")

CLI_addCommand("debug", "txq-sip6-pfc global-status", {
   func = dump_global_pfc_status,
   help = "Dump global flow control txq status",
   params={
        { type="named",
        { format="device %devID", name="devID",help="The device number" } ,
        { format="tile %tileID",name="tileNum",help="The tile number" },
        { format="dp %dpID",name="dpNum",help="The data path number" },
        mandatory={"devID","tileNum","dpNum"},
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: txq-sip6-pfc port-status
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-pfc port-status", {
   func = dump_port_pfc_status,
   help = "Dump port flow control txq status",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: txq-sip6-pfc pfcc-calendar
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-pfc pfcc-calendar", {
   func = dump_pfcc_calendar,
   help = "Dump PFCC table",
   params={
        { type="named",
        { format="device %devID",name="devID",help="The device number" } ,
        { format="tile %tileID",name="tileNum",help="The tile number" },
         mandatory={"devID","tileNum"},
        }
    }
})


--------------------------------------------------------------------------------
---- command registration: txq-sip6-pfc pfcc-calendar
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-pfc pool-status", {
   func = dump_pool_status,
   help = "Show pools status",
   params={
        { type="named",
        { format="device %devID",name="devID",help="The device number" } ,
         mandatory={"devID"}
        }
    }
})


