--********************************************************************************
--*              (c), Copyright 2023, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_patches.lua
--*
--* DESCRIPTION:
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


-- ************************************************************************
---
--  version_patches_show
--        @description  show version patches (if exist)
--
--        @param params     none
--
--        @return           none
--
local function show_patches(params)
	myGenWrapper("cpssGenShowPatches", {})
end


--------------------------------------------------------------------------------
-- command registration: show patches
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show patches",
    {
        func   = show_patches,
        help   = "Show version patches",
        params = {}
    }
)

