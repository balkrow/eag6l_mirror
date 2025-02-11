--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_data_integrity_example.lua
--*
--* DESCRIPTION:
--*       test for PIPE device : Data Integrity configuration/deconfiguration.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--just run the config file
executeLocalConfig(luaTgfBuildConfigFileName("PX_data_integrity_config"))
executeLocalConfig(luaTgfBuildConfigFileName("PX_data_integrity_deconfig"))

