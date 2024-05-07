--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* txq-sip7-profiles.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for SIP7 profiles
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

dofile("dxCh/debug/txq-sip6.lua")
dofile("dxCh/debug/txq-sip7-deep-buffering.lua")

local function dump_sip_7_port_sdq_params(params)
local res,val,devices,j,ports,i,q
local portMapShadow,attr,profile
local attrNames = {"TH_AGING","TH_HIGH","TH_LOW", "TH_NEG", "TC", "SEMIELIG","QCN_DIV_FACTOR", "PRIO","QBV_GATE", "AVBN"}

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

        for i=1,#ports do
            if(is_sip_7(devices[j]))then

               res, portMapShadow = myGenWrapper("cpssDxChPortPhysicalPortDetailedMapGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "CPSS_DXCH_DETAILED_PORT_MAP_STC", "portMapShadowPtr"},
                })

                if(res==0 and portMapShadow["portMapShadowPtr"].valid==true)then
                  print("Physical port number :  "..ports[i])
                  print("  Tile               :  "..portMapShadow["portMapShadowPtr"].extPortMap.tileId)
                  print("  Local dp (aNode sw):  "..portMapShadow["portMapShadowPtr"].extPortMap.localDpInTile)
                  print("  Local port in dp   :  "..portMapShadow["portMapShadowPtr"].extPortMap.localPortInDp)
                  print("  SDQ first Q        :  "..portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.secondarySchFirstQueueIndex)

                  for q=0,portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.numberOfQueues-1 do

                       res,val = myGenWrapper("prvCpssSip7TxqSdqMapQueueToProfileGet", {
                          { "IN", "GT_U8"  , "devNum", devices[j]},
                          { "IN", "GT_U32" , "tileNum",  portMapShadow["portMapShadowPtr"].extPortMap.tileId},
                          { "IN", "GT_U32" , "sdqNum",  portMapShadow["portMapShadowPtr"].extPortMap.localDpInTile},
                          { "IN", "GT_U32" , "queueNumber",  portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.secondarySchFirstQueueIndex+q},
                          { "OUT", "GT_U32", "profilePtr"},
                         })

                         if(res==0)then
                            print("  SDQ profile local q "..q.." :  "..val.profilePtr)
                            profile = val.profilePtr
                            for attr=1,10 do
                                res,val = myGenWrapper("prvCpssSip7TxqSdqProfileGet", {
                                  { "IN", "GT_U8"  , "devNum", devices[j]},
                                  { "IN", "GT_U32" , "tileNum",  portMapShadow["portMapShadowPtr"].extPortMap.tileId},
                                  { "IN", "GT_U32" , "sdqNum",  portMapShadow["portMapShadowPtr"].extPortMap.localDpInTile},
                                  { "IN", "GT_U32" , "attribute", attr-1},
                                  { "IN", "GT_U32" , "profile", profile},
                                  { "OUT", "GT_U32", "value"},
                                 })
                                 if(res==0) then
                                  print("   Attribute "..attrNames[attr].." :  "..val.value)
                                 else
                                  print("Failed to fetch attribute "..attr.." for queue "..q)
                                 end
                            end --for attr=0,10 do
                         else
                            print("Failed to fetch SDQ profile for queue")
                         end

                  end --for q=0,portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.numberOfQueues-1 do


                else
                  print("Device "..devices[j] ..  "port "..ports[i]..  " is not mapped")
                end
            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end
        end
    end
end

local function dump_sip_7_port_pds_params(params)
    print("Length adjust")
    dump_port_length_adjust_params(params)
    print("PDS long Q")
    dump_port_pds_params(params)
    print("EPB params")
    dump_port_epb_params(params)
end
--------------------------------------------------------------------------------
---- command registration: "txq-sip7-profiles  sdq"
--------------------------------------------------------------------------------


CLI_addCommand("debug", "txq-sip7-profile  sdq", {
   func = dump_sip_7_port_sdq_params,
   help = "SIP7 Dump port SDQ profile",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: "txq-sip7-profiles  pds"
--------------------------------------------------------------------------------


CLI_addCommand("debug", "txq-sip7-profile  pds", {
   func = dump_sip_7_port_pds_params,
   help = "SIP7 Dump port PDS profile",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})


