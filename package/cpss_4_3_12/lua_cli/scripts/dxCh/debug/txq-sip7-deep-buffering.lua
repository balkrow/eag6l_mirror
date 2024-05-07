--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* txq-sip7-deep-buffering.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for deep-buffering
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


function dump_port_epb_params(params)
    local res,val,devices,j,ports,i,enableBit

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

        for i=1,#ports do
            if(is_supported_feature(devices[j], "EPB_SPILLOVER"))then
                res, val = myGenWrapper("prvCpssTxqSip7DebugEpbProfileGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "GT_U32", "pdsProfileIndex"},
                        { "OUT", "GT_BOOL", "epbSpilloverEnablePtr"},
                        { "OUT", "GT_U32", "mailBox0IdPtr"},
                        { "OUT", "GT_U32", "mailBox1IdPtr"},
                        { "OUT", "GT_U32", "typePtr"},
                        { "OUT", "GT_U32", "tilePtr"},
                        { "OUT", "GT_U32", "pdsPtr"}
                })

                if val["epbSpilloverEnablePtr"]==true then
                 enableBit=1
                else
                 enableBit=0
                end

                  if (res==0)then
                    print(string.format("Dev:%3d Port:%5d Tile:%2d PDS:%2d Profile:%2d Spill Enable %d MB0 %d MB1 %d TX(bit1)/RX(bit0) %d",devices[j],ports[i],val["tilePtr"],val["pdsPtr"],val["pdsProfileIndex"],enableBit,val["mailBox0IdPtr"],val["mailBox1IdPtr"],val["typePtr"]))
                else
                    print("Could not get device "..devices[j] ..  " Port "..ports[i].." data")
                end
            else
                print("Device "..devices[j] ..  "  - EPB_SPILLOVER feature is not supported.")
            end
        end
    end
end


local function show_pds_configuration(params)
 local device = tonumber(params["devID"])
 local tileInd = tonumber(params["tile"])
 local maxDp,dpIter,id

 if(is_supported_feature(device, "EPB_SPILLOVER"))then
   print("EPB_SPILLOVER configurations:")
   res, val = myGenWrapper("prvCpssTxqSip6DebugLocalDpNumGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "OUT", "GT_U32", "dpNumPtr"},
            })
   if(res~=0)then
     print("Failed to execute prvCpssTxqSip6DebugLocalDpNumGet for device"..device.."Error code "..res)
     return
   end

   maxDp = val["dpNumPtr"]
   for dpIter=0,maxDp-1 do
    res, val = myGenWrapper("prvCpssSip7TxqPdsIdGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32", "tileNum",tileInd},
                { "IN", "GT_U32", "pdsNum",dpIter},
                { "OUT", "GT_U32", "idPtr"},
            })
    if(res~=0)then
     print("Failed to execute prvCpssSip7TxqPdsIdGet for device"..device.."Error code "..res)
    return
    end


    id = val["idPtr"]

    res, val = myGenWrapper("prvCpssSip7TxqDpdsModeGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32", "tileNum",tileInd},
                { "IN", "GT_U32", "pdsNum",dpIter},
                { "OUT", "GT_BOOL", "enablePtr"},
            })
    if(res~=0)then
     print("Failed to execute prvCpssSip7TxqDpdsModeGet for device"..device.."Error code "..res)
     return
    end


    print("DP "..dpIter.." id "..id.." DPDS "..tostring(val["enablePtr"]))
   end
 else
   print("EPB_SPILLOVER feature is not supported.")
 end
end


local function show_mailbox(params)
 local device = tonumber(params["devID"])
 local tileInd = tonumber(params["tile"])
 local pds = tonumber(params["pds"])
 local maxDp,i,id,dramQ
 local copyBackCnt,copyBackLimit

 if(is_supported_feature(device, "EPB_SPILLOVER"))then
   print("Mailbox configurations and status:")

   for i=0,31 do

    res, val = myGenWrapper("prvCpssSip7TxqPdsMailBoxCopyBackDataGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32", "tileNum",tileInd},
                { "IN", "GT_U32", "pdsNum",pds},
                { "IN", "GT_U32", "mailBoxId",i},
                { "OUT", "GT_U32", "reqCntPtr"},
                { "OUT", "GT_U32", "reqLimitPtr"},
            })
    if(res~=0)then
     print("Failed to execute prvCpssSip7TxqPdsMailBoxCopyBackDataGet for device"..device.."Error code "..res)
    return
    end

    copyBackCnt = val["reqCntPtr"]
    copyBackLimit = val["reqLimitPtr"]

    res, val = myGenWrapper("prvCpssSip7TxqDpdsMailBoxDramQueueGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32", "tileNum",tileInd},
                { "IN", "GT_U32", "pdsNum",pds},
                { "IN", "GT_U32", "mailBoxId",i},
                { "OUT", "GT_U32", "dramQueuePtr"}
            })
    if(res~=0)then
     print("Failed to execute prvCpssSip7TxqDpdsMailBoxDramQueueGet for device"..device.."Error code "..res)
    return
    end

    dramQ = val["dramQueuePtr"]

    res, val = myGenWrapper("prvCpssSip7TxqPdsMailBoxGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32", "tileNum",tileInd},
                { "IN", "GT_U32", "pdsNum",pds},
                { "IN", "GT_U32", "mailBoxId",i},
                { "OUT", "GT_U32", "pdsIdOutPtr"},
                { "OUT", "GT_U32", "mailBoxIdOutPtr"},
                { "OUT", "GT_BOOL", "hiPriorityPtr"}
            })
    if(res~=0)then
     print("Failed to execute prvCpssSip7TxqPdsMailBoxGet for device"..device.."Error code "..res)
    return
    end

    print("["..i.."] Target PDS id "..val["pdsIdOutPtr"].." Target mailbox id "..val["mailBoxIdOutPtr"].." HP "..tostring(val["hiPriorityPtr"]).." DRAM Q "..dramQ.." CopyBack Cnt "..copyBackCnt.." CopyBack Lmt "..copyBackLimit)
   end
 else
   print("EPB_SPILLOVER feature is not supported.")
 end
end


--------------------------------------------------------------------------------
---- command registration: "txq-sip7-deep-buffering  show-pds-configuration"
--------------------------------------------------------------------------------


CLI_addCommand("debug", "txq-sip7-deep-buffering  show-pds-configuration", {
   func = show_pds_configuration,
   help = "Show pds deep buffering related configurations",
   params={
        { type="named",
        { format="device %devID",name="devID",help="The device number" } ,
        { format="tile %tileID",name="tile",help="Tile number" },
         mandatory={"devID","tile"},
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: "txq-sip7-deep-buffering  show-mailbox"
--------------------------------------------------------------------------------


CLI_addCommand("debug", "txq-sip7-deep-buffering  show-mailbox", {
   func = show_mailbox,
   help = "Show pds mailbox related configurations",
   params={
        { type="named",
        { format="device %devID",name="devID",help="The device number" } ,
        { format="tile %tileID",name="tile",help="Tile number" },
        { format="pds %pdsID",name="pds",help="Pds number" },
         mandatory={"devID","tile","pds"},
        }
    }
})
--------------------------------------------------------------------------------
---- command registration: "txq-sip7-deep-buffering  port-epb-profile"
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip7-deep-buffering port-epb-profile", {
   func = dump_port_epb_params,
   help = "Dump port PDS profile",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})


