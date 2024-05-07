/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfIpv4Ipv6UcRoutingRandomMultipleRuns.c
*
* @brief IPV4 & IPV6 UC Routing when filling the Lpm and using many Add,
* Delete and flush operations.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>
#include <ip/prvTgfIpv4Ipv6UcRoutingRandomMultipleRuns.h>

/* the Virtual Router index */
static GT_U32        prvUtfVrfId                = 0;
/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/**
* @internal prvTgfIpLpmIpv4Ipv6UcPrefixAddMaxPrefix function
* @endinternal
*
* @brief  Flush all UC prefixes according to protocol
*
* @param[in] defragmentationEnable    - whether to enable
*                                      performance costing
*                                      de-fragmentation process
*                                      in the case that there is
*                                      no place to insert the
*                                      prefix. To point of the
*                                      process is just to make
*                                      space for this prefix.
*                                      (For DEVICES: xCat3; AC5;
*                                      Lion2: relevant only if
*                                      the LPM DB was created
*                                      with partitionEnable =
*                                      GT_FALSE)
* @param[out] addedNumIpv4Ptr  - (pointer to) the max number of ipv4 UC prefixes added
* @param[out] addedNumIpv6Ptr  - (pointer to) the max number of ipv6 UC prefixes added

*/
GT_STATUS prvTgfIpLpmIpv4Ipv6UcPrefixAddMaxPrefix
(
    GT_BOOL defragmentationEnable,
    GT_U32 *maxAddedNumIpv4Ptr,
    GT_U32 *maxAddedNumIpv6Ptr
)
{
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT              nextHopInfo;
    GT_IPADDR                                           ipv4Addr;
    GT_IPV6ADDR                                         ipv6Addr;
    GT_U32                                              idx, total_entrys, prefix;
    GT_U32                                              data, lfsr_val, lfsr_prefix;
    GT_BOOL                                             found_prefix, find_err;
    GT_U32                                              prefix_len_db[66] = { 0 };
    GT_U32                                              pref_cnt[66] = {0};
    GT_U8                                               msb;
    GT_STATUS                                           rc;

    /* Next hop */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;

    /* Random seed */
    cpssOsSrand(cpssOsTime());
    do
    {
        lfsr_val = cpssOsRand() & 0x3ff; /* 10bit */
    } while (lfsr_val == 0);

    /* For prefix database */
    do
    {
        lfsr_prefix = cpssOsRand() & 0x7ffff; /* 19bit */
    } while (lfsr_prefix == 0);

    /* Build the prefix length database */
    cpssOsMemSet(prefix_len_db,0,sizeof(prefix_len_db));
    prvTgfIpLpmUtilRibPrefixLenDbBuild(GT_TRUE, prefix_len_db);

    find_err = GT_FALSE;
    total_entrys=0;
    for (idx = 0; (find_err != GT_TRUE); idx++)
    {
        /* Add an IPv6 prefix entry */
        cpssOsMemSet(&ipv6Addr, 0, sizeof(ipv6Addr));

        ipv6Addr.arIP[0] = cpssOsRand() & 0xFE; /* Not multicast */
        ipv6Addr.arIP[1] = (lfsr_prefix >> 16) & 0xff;
        ipv6Addr.arIP[2] = (lfsr_prefix >>  8) & 0xff;
        ipv6Addr.arIP[3] = (lfsr_prefix >>  0) & 0xff;
        ipv6Addr.arIP[4] = (lfsr_prefix >> 16) & 0xff;
        ipv6Addr.arIP[5] = (lfsr_prefix >>  8) & 0xff;
        ipv6Addr.arIP[6] = (lfsr_prefix >>  0) & 0xff;
        ipv6Addr.arIP[7] = (lfsr_prefix >> 16) & 0xff;

        /* set prefix mask */
        found_prefix = GT_FALSE;
        for (prefix = 1; (prefix <= 64) && (found_prefix != GT_TRUE); )
        {
            if (lfsr_val < prefix_len_db[prefix])
            {
                found_prefix = GT_TRUE; /* Found */
            }
            else
            {
                prefix++; /* to next prefix length */
            }
        }

        rc = cpssDxChIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, &ipv6Addr, prefix, &nextHopInfo, GT_FALSE, defragmentationEnable);
        if ( rc && (rc != GT_ALREADY_EXIST) )
        {
            find_err = GT_TRUE;
        }

        /* Add done, do count */
        if (rc == GT_OK)
        {
            total_entrys++; pref_cnt[prefix]++;

            /* Next LFSR */
            lfsr_prefix = prvTgfIpLpmUtilShiftLFSR19b(lfsr_prefix);
            lfsr_val = prvTgfIpLpmUtilShiftLFSR10b(lfsr_val);
        }
    }

    /* find max ipv6 */
    *maxAddedNumIpv6Ptr = total_entrys;
    /* flush UC ipv6*/
    rc = cpssDxChIpLpmIpv6UcPrefixesFlush(prvTgfLpmDBId, prvUtfVrfId);
    if (rc!=GT_OK)
    {
        return rc;
    }
    /* Build the prefix length database */
    cpssOsMemSet(prefix_len_db,0,sizeof(prefix_len_db));
    prvTgfIpLpmUtilRibPrefixLenDbBuild(GT_FALSE, prefix_len_db);

    find_err=GT_FALSE;
    total_entrys=0;
    for (idx = 0; (find_err != GT_TRUE); idx++)
    {
        cpssOsMemSet(&ipv4Addr, 0, sizeof(ipv4Addr));

        /* Not multicast, 4b+4b */
        do
        {
            msb = ((cpssOsRand()&0xf)<<4) | (lfsr_prefix&0xf);
        } while ((msb >= 224) && (msb <= 239));

        data  = msb << 24;               /*  8b */
        data |= (lfsr_prefix>>4)<<9;     /* 15b */
        data |= cpssOsRand() & 0x1ff;    /*  9b */

        ipv4Addr.arIP[0] = (data >> 24) & 0xff;
        ipv4Addr.arIP[1] = (data >> 16) & 0xff;
        ipv4Addr.arIP[2] = (data >>  8) & 0xff;
        ipv4Addr.arIP[3] = (data >>  0) & 0xff;

        /* set prefix mask */
        found_prefix = GT_FALSE;
        for (prefix = 1; (prefix <= 32) && (found_prefix != GT_TRUE); )
        {
            if (lfsr_val < prefix_len_db[prefix])
            {
                found_prefix = GT_TRUE; /* Found */
            }
            else
            {
                prefix++; /* to next prefix length */
            }
        }

        rc = cpssDxChIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, &ipv4Addr, prefix, &nextHopInfo, GT_FALSE, defragmentationEnable);
        if ( rc && (rc != GT_ALREADY_EXIST) )
        {
            find_err = GT_TRUE;
        }

        /* Add done, do count */
        if (rc == GT_OK)
        {
            total_entrys++; pref_cnt[prefix]++;

            /* Next LFSR */
            lfsr_prefix = prvTgfIpLpmUtilShiftLFSR19b(lfsr_prefix);
            lfsr_val = prvTgfIpLpmUtilShiftLFSR10b(lfsr_val);
        }
    }

    /* find max ipv4 */
    *maxAddedNumIpv4Ptr = total_entrys;
    /* flush UC ipv4 */
    rc = cpssDxChIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId, prvUtfVrfId);
    return rc;
}


/**
* @internal prvTgfIpLpmFlushUcPrefixes function
* @endinternal
*
* @brief  Flush all UC prefixes according to protocol
*
* @param[in] isIpv6   - GT_TRUE: ipv6 protocol
*                       GT_FALSE: ipv4 protocol
*/
GT_STATUS prvTgfIpLpmFlushUcPrefixes
(
    GT_BOOL isIpv6
)
{
    GT_STATUS       rc;

    if(isIpv6 == GT_TRUE)
    {
        rc = cpssDxChIpLpmIpv6UcPrefixesFlush(prvTgfLpmDBId, prvUtfVrfId);

    }
    else
    {
        rc = cpssDxChIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId, prvUtfVrfId);
    }
    return rc;
}

/**
* @internal prvTgfIpLpmAddUcPrefixes function
* @endinternal
*
* @brief  Add UC prefixes according to parameters
*
* @param[in] isIpv6   - GT_TRUE: ipv6 protocol
*                       GT_FALSE: ipv4 protocol
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*                                      (For DEVICES: xCat3; AC5; Lion2: relevant only if
*                                      the LPM DB was created with partitionEnable = GT_FALSE)
* @param[in] numOfPrefixesToAdd - number of prefixes to add
*
*/
GT_STATUS prvTgfIpLpmAddUcPrefixes
(
    GT_BOOL is_ipv6,
    GT_BOOL defragmentationEnable,
    GT_U32 numOfPrefixesToAdd
)
{
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT              nextHopInfo;
    GT_IPADDR                                           ipv4Addr;
    GT_IPV6ADDR                                         ipv6Addr;
    GT_U32                                              idx, total_entrys, prefix;
    GT_U32                                              data, lfsr_val, lfsr_prefix;
    GT_BOOL                                             found_prefix, find_err;
    GT_U32                                              prefix_len_db[66] = { 0 };
    GT_U32                                              pref_cnt[66] = {0};
    GT_U8                                               msb;
    GT_STATUS                                           rc=GT_OK;

    /* Next hop */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;

    /* Random seed */
    cpssOsSrand(cpssOsTime());
    do
    {
        lfsr_val = cpssOsRand() & 0x3ff; /* 10bit */
    } while (lfsr_val == 0);

    /* For prefix database */
    do
    {
        lfsr_prefix = cpssOsRand() & 0x7ffff; /* 19bit */
    } while (lfsr_prefix == 0);

    /* Build the prefix length database */
    prvTgfIpLpmUtilRibPrefixLenDbBuild(is_ipv6, prefix_len_db);

    find_err = GT_FALSE;
    for (total_entrys = 0, idx = 0; (total_entrys < numOfPrefixesToAdd) && (find_err != GT_TRUE); idx++)
    {
        /* Add an IPv6 prefix entry */
        if (is_ipv6 == GT_TRUE)
        {
            cpssOsMemSet(&ipv6Addr, 0, sizeof(ipv6Addr));

            ipv6Addr.arIP[0] = cpssOsRand() & 0xFE; /* Not multicast */
            ipv6Addr.arIP[1] = (lfsr_prefix >> 16) & 0xff;
            ipv6Addr.arIP[2] = (lfsr_prefix >>  8) & 0xff;
            ipv6Addr.arIP[3] = (lfsr_prefix >>  0) & 0xff;
            ipv6Addr.arIP[4] = (lfsr_prefix >> 16) & 0xff;
            ipv6Addr.arIP[5] = (lfsr_prefix >>  8) & 0xff;
            ipv6Addr.arIP[6] = (lfsr_prefix >>  0) & 0xff;
            ipv6Addr.arIP[7] = (lfsr_prefix >> 16) & 0xff;

            /* set prefix mask */
            found_prefix = GT_FALSE;
            for (prefix = 1; (prefix <= 64) && (found_prefix != GT_TRUE); )
            {
                if (lfsr_val < prefix_len_db[prefix])
                {
                    found_prefix = GT_TRUE; /* Found */
                }
                else
                {
                    prefix++; /* to next prefix length */
                }
            }

            rc = cpssDxChIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, &ipv6Addr, prefix, &nextHopInfo, GT_FALSE, defragmentationEnable);
            if ( rc && (rc != GT_ALREADY_EXIST) )
            {
                find_err = GT_TRUE;
            }
        }
        else /* Add an IPv4 prefix entry */
        {
            cpssOsMemSet(&ipv4Addr, 0, sizeof(ipv4Addr));

            /* Not multicast, 4b+4b */
            do
            {
                msb = ((cpssOsRand()&0xf)<<4) | (lfsr_prefix&0xf);
            } while ((msb >= 224) && (msb <= 239));

            data  = msb << 24;               /*  8b */
            data |= (lfsr_prefix>>4)<<9;     /* 15b */
            data |= cpssOsRand() & 0x1ff;    /*  9b */

            ipv4Addr.arIP[0] = (data >> 24) & 0xff;
            ipv4Addr.arIP[1] = (data >> 16) & 0xff;
            ipv4Addr.arIP[2] = (data >>  8) & 0xff;
            ipv4Addr.arIP[3] = (data >>  0) & 0xff;

            /* set prefix mask */
            found_prefix = GT_FALSE;
            for (prefix = 1; (prefix <= 32) && (found_prefix != GT_TRUE); )
            {
                if (lfsr_val < prefix_len_db[prefix])
                {
                    found_prefix = GT_TRUE; /* Found */
                }
                else
                {
                    prefix++; /* to next prefix length */
                }
            }

            rc = cpssDxChIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, &ipv4Addr, prefix, &nextHopInfo, GT_FALSE, defragmentationEnable);
            if ( rc && (rc != GT_ALREADY_EXIST) )
            {

                find_err = GT_TRUE;
            }
        }
        /* Add done, do count */
        if (rc == GT_OK)
        {
            total_entrys++; pref_cnt[prefix]++;

            /* Next LFSR */
            lfsr_prefix = prvTgfIpLpmUtilShiftLFSR19b(lfsr_prefix);
            lfsr_val = prvTgfIpLpmUtilShiftLFSR10b(lfsr_val);
        }
    }

    return rc;
}

/**
 */
GT_U32  prvTgfIpLpmUtilShiftLFSR19b(GT_U32 v)
{
    /* config          : galois
     * length          : 20
     * taps            : (19, 18, 9, 7, 6, 5, 4, 2)
     * shift-amount    : 3
     * shift-direction : right
    */
    GT_U32 tap_0  = 19;
    GT_U32 tap_1  = 18;
    GT_U32 tap_2  =  9;
    GT_U32 tap_3  =  7;
    GT_U32 tap_4  =  6;
    GT_U32 tap_5  =  5;
    GT_U32 tap_6  =  4;
    GT_U32 tap_7  =  2;

    GT_U32 zero = 0;
    GT_U32 lsb = zero + 1;

    GT_U32 feedback = (
        (lsb << (tap_0 - 1)) ^
        (lsb << (tap_1 - 1)) ^
        (lsb << (tap_2 - 1)) ^
        (lsb << (tap_3 - 1)) ^
        (lsb << (tap_4 - 1)) ^
        (lsb << (tap_5 - 1)) ^
        (lsb << (tap_6 - 1)) ^
        (lsb << (tap_7 - 1))
    );
    v = (v >> 1) ^ ((zero - (v & lsb)) & feedback);
    v = (v >> 1) ^ ((zero - (v & lsb)) & feedback);
    v = (v >> 1) ^ ((zero - (v & lsb)) & feedback);
    return v;
}

/**
 */
GT_U32  prvTgfIpLpmUtilShiftLFSR10b(GT_U32 v)
{
    /* config          : galois
     * length          : 10
     * taps            : (10, 9, 4, 2)
     * shift-amount    : 2
     * shift-direction : right
    */
    GT_U32 tap_0  = 10;
    GT_U32 tap_1  =  9;
    GT_U32 tap_2  =  4;
    GT_U32 tap_3  =  2;

    GT_U32 zero = 0;
    GT_U32 lsb = zero + 1;

    GT_U32 feedback = (
        (lsb << (tap_0 - 1)) ^
        (lsb << (tap_1 - 1)) ^
        (lsb << (tap_2 - 1)) ^
        (lsb << (tap_3 - 1))
    );
    v = (v >> 1) ^ ((zero - (v & lsb)) & feedback);
    v = (v >> 1) ^ ((zero - (v & lsb)) & feedback);
    return v;
}

/**
 */
GT_VOID prvTgfIpLpmUtilRibPrefixLenDbBuild(GT_BOOL is_ipv6, GT_U32 *prefix_len_db)
{
    GT_U32  v4_rib_prefix[66] = {
                    8,1,       11,3,       12,12,     13,27,     14,45,    15,80,
                    16,719,    17,77,      18,125,    19,229,    20,314,   21,13281,
                    22,17541,  23, 16917,  24,96144,  25,495,    26,511,   27,349,
                    28,200,    29,186,     30,200,    0,0 /*END*/
                    };
    GT_U32  v6_rib_prefix[80] = {
                    32,1356,   33,80,      34,70,      35,71,    36,71,    37,70,
                    38,70,     39,64,      40,2033,    41,58,    42,58,    43,57,
                    44,62,     45,68,      46,57,      47,64,    48,3422,  49,58,
                    50,52,     51,48,      52,49,      53,63,    54,41,    55,64,
                    56,6806,   57,47,      58,46,      59,50,    60,48,    61,49,
                    62,44,     63,37,      64,33919,   0,0 /*END*/
                    };
    GT_U32  *rib_prefix_ptr = NULL;
    GT_U32  idx, ofs, total, diff;
    GT_U32  calc_cnt, curr_cnt;

    if (is_ipv6 == GT_TRUE)
    {
        rib_prefix_ptr = &(v6_rib_prefix[0]); /* Build IPv6 prefix length db */
    }
    else
    {
        rib_prefix_ptr = &(v4_rib_prefix[0]); /* Build IPv4 prefix length db */
    }

    /* Get total counters */
    for (total = 0, idx = 0; rib_prefix_ptr[idx] != 0; idx += 2)
    {
        total += rib_prefix_ptr[idx+1];
    }

    /* Calculate the new value in range [1:1024] */
    for (curr_cnt = 0, idx = 0; rib_prefix_ptr[idx] != 0; idx += 2)
    {
        calc_cnt = ( (rib_prefix_ptr[idx+1]*1024)/total );
        if (calc_cnt == 0) calc_cnt = 1;
        curr_cnt += calc_cnt;
        if (curr_cnt > 1024) curr_cnt = 1024;
        prefix_len_db[rib_prefix_ptr[idx+0]] = curr_cnt;
    }
    if ( curr_cnt < 1024 )
    {
        diff = 1024 - curr_cnt;
        if ( is_ipv6 == GT_TRUE )
        {
            ofs = idx = 64; /* IPv6 /64 is the major */
        }
        else
        {
            ofs = idx = 24; /* IPv4 /24 is the major */
        }
        for ( ; rib_prefix_ptr[idx] != 0; idx += 2, ofs++)
        {
            prefix_len_db[ofs] += diff;
        }
    }
    /* Last prefix set to 1024 */
    prefix_len_db[rib_prefix_ptr[idx-2]] = 1024;
}


/**
* @internal prvTgfIpLpmAddIpv6UcEntriesbyPrefixStreamIncrBulk function
* @endinternal
*
* @brief  Add ipv6 UC prefixes according to parameters using bulk
*
* @param[in] ulPrefixBitmap - flag for different bulk parameters
* @param[in] numOfPrefixesToAdd - number of prefixes to add
*
*/
GT_STATUS prvTgfIpLpmAddIpv6UcEntriesbyPrefixStreamIncrBulk(
   IN  GT_U32 ulPrefixBitmap,
   IN  GT_U32 numOfPrefixesToAdd
)
{
	GT_STATUS rc = GT_OK;
    GT_U8 arIP[16];
    int i, j, k;

	GT_U32                                  ulRnd = 250, x = 200, y = 500;
    GT_U32                                  ipv6PrefixArrayLen = 0;
    GT_U32                                  ulIPv6PrefixNum = 0;
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC     *ipv6PrefixArrayPtr = NULL;

    GT_U32                                  numOfPrefixesAdded = 0, numAddedInFdb = 0, numAddedInLpm = 0, numBadState = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
	GT_U32                                  prefixLength = 0;

    ipv6PrefixArrayPtr = cpssOsMalloc(_1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));

	cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
	nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
	nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
	nextHopInfo.ipLttEntry.numOfPaths          = 0;
	nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

	numOfPrefixesAdded = 0;

    cpssOsSrand(cpssOsTime());

    if (0 != (ulPrefixBitmap & 1))
    {
        /* 2402:5ec0:14::1/128 51200 */
        cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x24;
        arIP[1] = 0x02;
        arIP[2] = 0x5e;
        arIP[3] = 0xc0;
        arIP[4] = 0x00;
        arIP[5] = 0x14;

    	prefixLength = 128;
        ulIPv6PrefixNum = numOfPrefixesToAdd;
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[13] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[14] = j;
                for (k = 1; k <= 255; k++)
                {
                    arIP[15] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixAddBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr, GT_FALSE);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesAdded += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesAdded >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were added, 2402:5ec0:14::1/128 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesAdded, numAddedInFdb, numAddedInLpm, numBadState);
    }

    if (0 != (ulPrefixBitmap & 2))
    {
        /* 2402:6ec0:11::1/48 51200 */
       cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x24;
        arIP[1] = 0x02;
        arIP[2] = 0x6e;
        arIP[3] = 0xc0;
        arIP[4] = 0x00;
        arIP[5] = 0x11;

    	prefixLength = 48;
        ulIPv6PrefixNum = numOfPrefixesToAdd;
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0xc0; i <= 255; i++)
        {
            arIP[3] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[4] = j;
                for (k = 0x11; k <= 255; k++)
                {
                    arIP[5] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixAddBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr, GT_FALSE);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesAdded += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesAdded >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were added, 2402:6ec0:11::1/48 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesAdded, numAddedInFdb, numAddedInLpm, numBadState);
    }

    if (0 != (ulPrefixBitmap & 4))
    {
        /* 2402:5ec0:13::1/64 51200 */
       cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x24;
        arIP[1] = 0x02;
        arIP[2] = 0x5e;
        arIP[3] = 0xc0;
        arIP[4] = 0x00;
        arIP[5] = 0x13;

    	prefixLength = 64;
        ulIPv6PrefixNum = numOfPrefixesToAdd;
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0x13; i <= 255; i++)
        {
            arIP[5] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[6] = j;
                for (k = 0; k <= 255; k++)
                {
                    arIP[7] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixAddBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr, GT_FALSE);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesAdded += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesAdded >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were added, 2402:5ec0:13::1/64 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesAdded, numAddedInFdb, numAddedInLpm, numBadState);
    }

    if (0 != (ulPrefixBitmap & 8))
    {
        /* 2402:8ec1:12::1/56 51200 */
       cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x24;
        arIP[1] = 0x02;
        arIP[2] = 0x8e;
        arIP[3] = 0xc1;
        arIP[4] = 0x00;
        arIP[5] = 0x12;

    	prefixLength = 56;
        ulIPv6PrefixNum = numOfPrefixesToAdd;
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[4] = i;

            for (j = 0x12; j <= 255; j++)
            {
                arIP[5] = j;
                for (k = 0; k <= 255; k++)
                {
                    arIP[6] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixAddBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr, GT_FALSE);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesAdded += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesAdded >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were added, 2402:8ec1:12::1/56 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesAdded, numAddedInFdb, numAddedInLpm, numBadState);
    }

    if (0 != (ulPrefixBitmap & 0x10))
    {
        /* 1900:0:1::1/128 16384 */
       cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x19;
        arIP[1] = 0x0;
        arIP[2] = 0x0;
        arIP[3] = 0x0;
        arIP[4] = 0x00;
        arIP[5] = 0x1;

    	prefixLength = 128;
        ulIPv6PrefixNum = numOfPrefixesToAdd;
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[13] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[14] = j;
                for (k = 1; k <= 255; k++)
                {
                    arIP[15] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixAddBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr, GT_FALSE);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesAdded += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesAdded >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }

    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were added, 1900:0:1::1/128 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesAdded, numAddedInFdb, numAddedInLpm, numBadState);
    }

    cpssOsFree(ipv6PrefixArrayPtr);
	return rc;
}

/**
* @internal prvTgfIpLpmAddIpv6UcEntriesbyPrefixStreamIncrBulkDel function
* @endinternal
*
* @brief  Delete ipv6 UC prefixes according to parameters using bulk
*
* @param[in] ulPrefixBitmap - flag for different bulk parameters
* @param[in] numOfPrefixesToDel - number of prefixes to deleted
*
*/
GT_STATUS prvTgfIpLpmAddIpv6UcEntriesbyPrefixStreamIncrBulkDel(
   IN GT_U32 ulPrefixBitmap,
   IN GT_U32 numOfPrefixesToDel
)
{
	GT_STATUS rc = GT_OK;
    GT_U8 arIP[16];
    int i, j, k;

	GT_U32                                  ulRnd = 250, x = 200, y = 500;
    GT_U32                                  ipv6PrefixArrayLen = 0;
    GT_U32                                  ulIPv6PrefixNum = 0;
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC     *ipv6PrefixArrayPtr = NULL;

    /*GT_IPADDR                               currentIpv4Addr;*/
    GT_U32                                  numOfPrefixesDeleted = 0, numDeletedInFdb = 0, numDeletedInLpm = 0, numBadState = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
	GT_U32                                  prefixLength = 0;

    ipv6PrefixArrayPtr = cpssOsMalloc(_1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));

	cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
	nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
	nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
	nextHopInfo.ipLttEntry.numOfPaths          = 0;
	nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

	numOfPrefixesDeleted = 0;

	cpssOsSrand(cpssOsTime());

    if (0 != (ulPrefixBitmap & 1))
    {
        /* 2402:5ec0:14::1/128 51200 */
       cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x24;
        arIP[1] = 0x02;
        arIP[2] = 0x5e;
        arIP[3] = 0xc0;
        arIP[4] = 0x00;
        arIP[5] = 0x14;

    	prefixLength = 128;
        ulIPv6PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[13] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[14] = j;
                for (k = 1; k <= 255; k++)
                {
                    arIP[15] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  del the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixDelBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesDeleted += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesDeleted >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were deleted, 2402:5ec0:14::1/128 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesDeleted, numDeletedInFdb, numDeletedInLpm, numBadState);

    }

    if (0 != (ulPrefixBitmap & 2))
    {
        /* 2402:6ec0:11::1/48 51200 */
       cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x24;
        arIP[1] = 0x02;
        arIP[2] = 0x6e;
        arIP[3] = 0xc0;
        arIP[4] = 0x00;
        arIP[5] = 0x11;

    	prefixLength = 48;
        ulIPv6PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0xc0; i <= 255; i++)
        {
            arIP[3] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[4] = j;
                for (k = 0x11; k <= 255; k++)
                {
                    arIP[5] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  del the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixDelBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesDeleted += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesDeleted >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were deleted, 2402:6ec0:11::1/48 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesDeleted, numDeletedInFdb, numDeletedInLpm, numBadState);
    }

    if (0 != (ulPrefixBitmap & 4))
    {
        /* 2402:5ec0:13::1/64 51200 */
       cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x24;
        arIP[1] = 0x02;
        arIP[2] = 0x5e;
        arIP[3] = 0xc0;
        arIP[4] = 0x00;
        arIP[5] = 0x13;

    	prefixLength = 64;
        ulIPv6PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0x13; i <= 255; i++)
        {
            arIP[5] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[6] = j;
                for (k = 0; k <= 255; k++)
                {
                    arIP[7] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixDelBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesDeleted += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesDeleted >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were deleted, 2402:5ec0:13::1/64 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesDeleted, numDeletedInFdb, numDeletedInLpm, numBadState);

    }

    if (0 != (ulPrefixBitmap & 8))
    {
        /* 2402:8ec1:12::1/56 51200 */
       cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x24;
        arIP[1] = 0x02;
        arIP[2] = 0x8e;
        arIP[3] = 0xc1;
        arIP[4] = 0x00;
        arIP[5] = 0x12;

    	prefixLength = 56;
        ulIPv6PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[4] = i;

            for (j = 0x12; j <= 255; j++)
            {
                arIP[5] = j;
                for (k = 0; k <= 255; k++)
                {
                    arIP[6] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  del the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixDelBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesDeleted += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesDeleted >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were deleted, 2402:8ec1:12::1/56 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesDeleted, numDeletedInFdb, numDeletedInLpm, numBadState);
    }

    if (0 != (ulPrefixBitmap & 0x10))
    {
        /* 1900:0:1::1/128 16384 */
       cpssOsMemSet(arIP, 0, 16);
        arIP[0] = 0x19;
        arIP[1] = 0x0;
        arIP[2] = 0x0;
        arIP[3] = 0x0;
        arIP[4] = 0x00;
        arIP[5] = 0x1;

    	prefixLength = 128;
        ulIPv6PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv6PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
        ipv6PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[13] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[14] = j;
                for (k = 1; k <= 255; k++)
                {
                    arIP[15] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv6PrefixArrayPtr[ipv6PrefixArrayLen].ipAddr.arIP, arIP, 16);

                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].prefixLen = prefixLength;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].vrId = 0;
                	ipv6PrefixArrayPtr[ipv6PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv6PrefixArrayPtr[ipv6PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv6PrefixArrayLen++;

                	if ((ipv6PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv6PrefixArrayLen) >= ulIPv6PrefixNum))
                	{
            			/*  del the current prefix */
            			rc = cpssDxChIpLpmIpv6UcPrefixDelBulk(0, ipv6PrefixArrayLen, ipv6PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesDeleted += ipv6PrefixArrayLen;
                        }

            	        if (numOfPrefixesDeleted >= ulIPv6PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv6PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }

    	cpssOsPrintf("\n (%d) IPv6 UC Prefixes were deleted, 1900:0:1::1/128 : %d was added in FDB and %d was added in LPM, numBadState %d\n",numOfPrefixesDeleted, numDeletedInFdb, numDeletedInLpm, numBadState);
    }
    cpssOsFree(ipv6PrefixArrayPtr);
	return rc;
}

/**
* @internal prvTgfIpLpmAddIpv4UcEntriesbyPrefixStreamIncrBulk function
* @endinternal
*
* @brief  Add ipv4 UC prefixes according to parameters using bulk
*
* @param[in] ulPrefixBitmap - flag for different bulk parameters
* @param[in] numOfPrefixesToAdd - number of prefixes to add
*
*/
GT_STATUS prvTgfIpLpmAddIpv4UcEntriesbyPrefixStreamIncrBulk(
   IN  GT_U32 ulPrefixBitmap,
   IN  GT_U32 numOfPrefixesToAdd
)
{
	GT_STATUS rc = GT_OK;
	GT_U8 arIP[4] = {0};
    GT_U32 ulRnd = 250, x = 200, y = 500;
    int i, j, k;
	GT_U32                                  prefixLength = 0;
    GT_U32                                  ulIPv4PrefixNum = 0;
    GT_U32                                  ipv4PrefixArrayLen = 0;
    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC     *ipv4PrefixArrayPtr = NULL;
    GT_U32                                  numOfPrefixesAdded = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;

	ipv4PrefixArrayPtr = cpssOsMalloc(_1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));

	cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
	nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
	nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
	nextHopInfo.ipLttEntry.numOfPaths          = 0;
	nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

	numOfPrefixesAdded = 0;
    cpssOsSrand(cpssOsTime());

    if (0 != (ulPrefixBitmap & 1))
    {
        /* 101.0.0.0/24 51200 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 101;
    	prefixLength = 24;
        ulIPv4PrefixNum = numOfPrefixesToAdd;
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[2] = j;

            	/* add route entry here*/
            	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

            	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
            	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
            	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
            	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
            	ipv4PrefixArrayLen++;

            	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
            	{
        			/*  add the current prefix */
        			rc = cpssDxChIpLpmIpv4UcPrefixAddBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

        			if (rc == GT_OUT_OF_PP_MEM)
        			{
        				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
        				i = j = k = 300;
        				break;
        			}

        			if (rc != GT_OK)
        			{
        				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
        				i = j = k = 300;
        				break;
        			}

                    if (rc == GT_OK)
                    {
                    	numOfPrefixesAdded += ipv4PrefixArrayLen;
                    }

        	        if (numOfPrefixesAdded >= ulIPv4PrefixNum)
        	        {
        	            i = j = k = 300;
        	            break;
        	        }

                    ipv4PrefixArrayLen = 0;
                    ulRnd = x + cpssOsRand()%y;
                }
            }
        }

        cpssOsPrintf("\r\addIpv4UcEntriesbyPrefixStream_Incr 101.0.0.0/24 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesAdded);
    }

    if (0 != (ulPrefixBitmap & 2))
    {
        /* 14.0.0.0/32 51200 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 14;
    	prefixLength = 32;
        ulIPv4PrefixNum = numOfPrefixesToAdd;
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[2] = j;
                for (k = 0; k <= 255; k++)
                {
                    arIP[3] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv4PrefixArrayLen++;

                	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv4UcPrefixAddBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesAdded += ipv4PrefixArrayLen;
                        }

            	        if (numOfPrefixesAdded >= ulIPv4PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv4PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }

        cpssOsPrintf("\r\addIpv4UcEntriesbyPrefixStream_Incr 14.0.0.0/32 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesAdded);
    }

    if (0 != (ulPrefixBitmap & 4))
    {
        /* 13.0.0.0/28 51200 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 13;
    	prefixLength = 28;
        ulIPv4PrefixNum = numOfPrefixesToAdd;
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[2] = j;

                for (k = 0x10; k <= 255; k += 0x10)
                {
                    arIP[3] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv4PrefixArrayLen++;

                	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv4UcPrefixAddBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesAdded += ipv4PrefixArrayLen;
                        }

            	        if (numOfPrefixesAdded >= ulIPv4PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv4PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
        cpssOsPrintf("\r\addIpv4UcEntriesbyPrefixStream_Incr 13.0.0.0/28 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesAdded);
    }

    if (0 != (ulPrefixBitmap & 8))
    {
        /* 100.0.0.0/26 51200 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 100;
    	prefixLength = 26;
        ulIPv4PrefixNum = numOfPrefixesToAdd;
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[2] = j;

                for (k = 0x40; k <= 255; k += 0x40)
                {
                    arIP[3] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv4PrefixArrayLen++;

                	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv4UcPrefixAddBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesAdded += ipv4PrefixArrayLen;
                        }

            	        if (numOfPrefixesAdded >= ulIPv4PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv4PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
        cpssOsPrintf("\r\addIpv4UcEntriesbyPrefixStream_Incr 100.0.0.0/26 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesAdded);
    }

    if (0 != (ulPrefixBitmap & 0x10))
    {
        /* 190.0.1.0/32 2048 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 190;
    	prefixLength = 32;
        ulIPv4PrefixNum = numOfPrefixesToAdd;/* 2048*/
        numOfPrefixesAdded = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 1; j <= 255; j++)
            {
                arIP[2] = j;
                for (k = 0; k <= 255; k++)
                {
                    arIP[3] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv4PrefixArrayLen++;

                	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesAdded + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
                	{
            			/*  add the current prefix */
            			rc = cpssDxChIpLpmIpv4UcPrefixAddBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesAdded += ipv4PrefixArrayLen;
                        }

            	        if (numOfPrefixesAdded >= ulIPv4PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv4PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }

        cpssOsPrintf("\r\addIpv4UcEntriesbyPrefixStream_Incr 190.0.1.0/32 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesAdded);
    }

    cpssOsFree(ipv4PrefixArrayPtr);
    return GT_OK;
}

/**
* @internal prvTgfIpLpmAddIpv4UcEntriesbyPrefixStreamIncrBulkDel function
* @endinternal
*
* @brief  Delete ipv4 UC prefixes according to parameters using bulk
*
* @param[in] ulPrefixBitmap - flag for different bulk parameters
* @param[in] numOfPrefixesToDel - number of prefixes to deleted
*
*/
GT_STATUS prvTgfIpLpmAddIpv4UcEntriesbyPrefixStreamIncrBulkDel(
   IN GT_U32 ulPrefixBitmap,
   IN GT_U32 numOfPrefixesToDel
)
{
	GT_STATUS rc = GT_OK;
	GT_U8 arIP[4] = {0};
    GT_U32 ulRnd = 250, x = 200, y = 500;
    int i, j, k;
	GT_U32                                  prefixLength = 0;
    GT_U32 ulIPv4PrefixNum = 0;
    GT_U32                               ipv4PrefixArrayLen = 0;
    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr = NULL;
    GT_U32                                  numOfPrefixesDeleted = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;

	ipv4PrefixArrayPtr = cpssOsMalloc(_1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));

	cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
	nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
	nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
	nextHopInfo.ipLttEntry.numOfPaths          = 0;
	nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

	numOfPrefixesDeleted = 0;
    cpssOsSrand(cpssOsTime());

    if (0 != (ulPrefixBitmap & 1))
    {
        /* 101.0.0.0/24 51200 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 101;
    	prefixLength = 24;
        ulIPv4PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[2] = j;

            	/* add route entry here*/
            	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

            	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
            	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
            	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
            	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
            	ipv4PrefixArrayLen++;

            	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
            	{
        			/*  del the current prefix */
        			rc = cpssDxChIpLpmIpv4UcPrefixDelBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

        			if (rc == GT_OUT_OF_PP_MEM)
        			{
        				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
        				i = j = k = 300;
        				break;
        			}

        			if (rc != GT_OK)
        			{
        				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
        				i = j = k = 300;
        				break;
        			}

                    if (rc == GT_OK)
                    {
                    	numOfPrefixesDeleted += ipv4PrefixArrayLen;
                    }

        	        if (numOfPrefixesDeleted >= ulIPv4PrefixNum)
        	        {
        	            i = j = k = 300;
        	            break;
        	        }

                    ipv4PrefixArrayLen = 0;
                    ulRnd = x + cpssOsRand()%y;
                }
            }
        }

        cpssOsPrintf("\r\naddIpv4UcEntriesbyPrefixStream_Incr_BulkDel 101.0.0.0/24 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesDeleted);
    }

    if (0 != (ulPrefixBitmap & 2))
    {
        /* 14.0.0.0/32 51200 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 14;
    	prefixLength = 32;
        ulIPv4PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[2] = j;
                for (k = 0; k <= 255; k++)
                {
                    arIP[3] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv4PrefixArrayLen++;

                	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
                	{
            			/*  del the current prefix */
            			rc = cpssDxChIpLpmIpv4UcPrefixDelBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesDeleted += ipv4PrefixArrayLen;
                        }

            	        if (numOfPrefixesDeleted >= ulIPv4PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv4PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }

        cpssOsPrintf("\r\naddIpv4UcEntriesbyPrefixStream_Incr_BulkDel 14.0.0.0/32 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesDeleted);
    }

    if (0 != (ulPrefixBitmap & 4))
    {
        /* 13.0.0.0/28 51200 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 13;
    	prefixLength = 28;
        ulIPv4PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[2] = j;

                for (k = 0x10; k <= 255; k += 0x10)
                {
                    arIP[3] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv4PrefixArrayLen++;

                	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
                	{
            			/*  del the current prefix */
            			rc = cpssDxChIpLpmIpv4UcPrefixDelBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesDeleted += ipv4PrefixArrayLen;
                        }

            	        if (numOfPrefixesDeleted >= ulIPv4PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv4PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
        cpssOsPrintf("\r\naddIpv4UcEntriesbyPrefixStream_Incr_BulkDel 13.0.0.0/28 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesDeleted);

    }

    if (0 != (ulPrefixBitmap & 8))
    {
        /* 100.0.0.0/26 51200 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 100;
    	prefixLength = 26;
        ulIPv4PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 0; j <= 255; j++)
            {
                arIP[2] = j;

                for (k = 0x40; k <= 255; k += 0x40)
                {
                    arIP[3] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv4PrefixArrayLen++;

                	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
                	{
            			/*  del the current prefix */
            			rc = cpssDxChIpLpmIpv4UcPrefixDelBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesDeleted += ipv4PrefixArrayLen;
                        }

            	        if (numOfPrefixesDeleted >= ulIPv4PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv4PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
        cpssOsPrintf("\r\naddIpv4UcEntriesbyPrefixStream_Incr_BulkDel 100.0.0.0/26 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesDeleted);

    }

    if (0 != (ulPrefixBitmap & 0x10))
    {
        /* 190.0.1.0/32 2048 */
       cpssOsMemSet(arIP, 0, 4);
        arIP[0] = 190;
    	prefixLength = 32;
        ulIPv4PrefixNum = numOfPrefixesToDel;
        numOfPrefixesDeleted = 0;
    	cpssOsMemSet(ipv4PrefixArrayPtr, 0, _1K * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
        ipv4PrefixArrayLen = 0;

        for (i = 0; i <= 255; i++)
        {
            arIP[1] = i;

            for (j = 1; j <= 255; j++)
            {
                arIP[2] = j;
                for (k = 0; k <= 255; k++)
                {
                    arIP[3] = k;

                	/* add route entry here*/
                	cpssOsMemCpy(ipv4PrefixArrayPtr[ipv4PrefixArrayLen].ipAddr.arIP, arIP, 4);

                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].prefixLen = prefixLength;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].vrId = 0;
                	ipv4PrefixArrayPtr[ipv4PrefixArrayLen].override = GT_FALSE;
                	cpssOsMemCpy(&ipv4PrefixArrayPtr[ipv4PrefixArrayLen].nextHopInfo, &nextHopInfo, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
                	ipv4PrefixArrayLen++;

                	if ((ipv4PrefixArrayLen >= ulRnd) || ((numOfPrefixesDeleted + ipv4PrefixArrayLen) >= ulIPv4PrefixNum))
                	{
            			/*  del the current prefix */
            			rc = cpssDxChIpLpmIpv4UcPrefixDelBulk(0, ipv4PrefixArrayLen, ipv4PrefixArrayPtr);

            			if (rc == GT_OUT_OF_PP_MEM)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d\n", prefixLength);
            				i = j = k = 300;
            				break;
            			}

            			if (rc != GT_OK)
            			{
            				cpssOsPrintf("\n IPv6 UC Prefixes failed after adding LPM, prefix is %d, rc = %d\n", prefixLength, rc);
            				i = j = k = 300;
            				break;
            			}

                        if (rc == GT_OK)
                        {
                        	numOfPrefixesDeleted += ipv4PrefixArrayLen;
                        }

            	        if (numOfPrefixesDeleted >= ulIPv4PrefixNum)
            	        {
            	            i = j = k = 300;
            	            break;
            	        }

                        ipv4PrefixArrayLen = 0;
                        ulRnd = x + cpssOsRand()%y;
                    }
                }
            }
        }
        cpssOsPrintf("\r\naddIpv4UcEntriesbyPrefixStream_Incr_BulkDel 190.0.1.0/32 Successed, ulTotalEntrys = %d\r\n", numOfPrefixesDeleted);
    }

    cpssOsFree(ipv4PrefixArrayPtr);
    return GT_OK;
}

/**
* @internal prvTgfIpv4Ipv6UcMcValidityCheck function
* @endinternal
*
* @brief   Check validity for LTT Route Configuration
*
*/
GT_VOID prvTgfIpv4Ipv6UcMcValidityCheck
(
    GT_VOID
)
{
    GT_STATUS rc=GT_OK;

    /* enable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_TRUE);

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId, prvUtfVrfId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck IPV4 UC: %d", prvTgfDevNum);

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId, prvUtfVrfId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck IPV6 UC: %d", prvTgfDevNum);

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId, prvUtfVrfId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck IPV4 MC: %d", prvTgfDevNum);

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId, prvUtfVrfId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck IPV6 MC: %d", prvTgfDevNum);

    /* disable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_FALSE);
}

/**
* @internal prvTgfIpLpmFlushMcPrefixes function
* @endinternal
*
* @brief  Flush all MC prefixes according to protocol
*
* @param[in] isIpv6   - GT_TRUE: ipv6 protocol
*                       GT_FALSE: ipv4 protocol
*/
GT_STATUS prvTgfIpLpmFlushMcPrefixes
(
    GT_BOOL isIpv6
)
{
    GT_STATUS       rc;

    if(isIpv6 == GT_TRUE)
    {
        rc = cpssDxChIpLpmIpv6McEntriesFlush(prvTgfLpmDBId, prvUtfVrfId);

    }
    else
    {
        rc = cpssDxChIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, prvUtfVrfId);
    }
    return rc;
}

/**
* @internal prvTgfIpLpmAddMcPrefixes function
* @endinternal
*
* @brief  Add MC prefixes according to parameters
*
* @param[in] isIpv6   - GT_TRUE: ipv6 protocol
*                       GT_FALSE: ipv4 protocol
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*                                      (For DEVICES: xCat3; AC5; Lion2: relevant only if
*                                      the LPM DB was created with partitionEnable = GT_FALSE)
* @param[in] numOfPrefixesToAdd - number of prefixes to add
*
*/
GT_STATUS prvTgfIpLpmAddMcPrefixes
(
    GT_BOOL is_ipv6,
    GT_BOOL defragmentationEnable,
    GT_U32 numOfPrefixesToAdd
)
{
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT              nextHopInfo;
    GT_IPADDR                                           ipv4Addr;
    GT_IPV6ADDR                                         ipv6Addr;
    GT_U32                                              idx, total_entrys, prefix;
    GT_U32                                              data, lfsr_val, lfsr_prefix;
    GT_BOOL                                             found_prefix, find_err;
    GT_U32                                              prefix_len_db[66] = { 0 };
    GT_U32                                              pref_cnt[66] = {0};
    GT_U8                                               msb;
    GT_STATUS                                           rc=GT_OK;

    GT_IPADDR                    ipGroup;
    GT_IPV6ADDR                  ipv6Group;
    CPSS_DXCH_IP_LTT_ENTRY_STC   mcRouteLttEntry;

    /* Next hop */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;

    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
    cpssOsMemSet(&ipv6Group, 0, sizeof(GT_IPV6ADDR));

    mcRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E;
    mcRouteLttEntry.numOfPaths = 2;

    ipGroup.arIP[0] = 225;
    ipGroup.arIP[1] = 34;
    ipGroup.arIP[2] = 211;
    ipGroup.arIP[3] = 7;

    /* Random seed */
    cpssOsSrand(cpssOsTime());
    do {
        lfsr_val = cpssOsRand() & 0x3ff; /* 10bit */
    } while (lfsr_val == 0);

    /* For prefix database */
    do {
        lfsr_prefix = cpssOsRand() & 0x7ffff; /* 19bit */
    } while (lfsr_prefix == 0);

    /* Build the prefix length database */
    prvTgfIpLpmUtilRibPrefixLenDbBuild (is_ipv6, prefix_len_db);

    find_err = GT_FALSE;
    for (total_entrys = 0, idx = 0; (total_entrys < numOfPrefixesToAdd) && (find_err != GT_TRUE); idx++)
    {
        /* Add an IPv6 prefix entry */
        if (is_ipv6 == GT_TRUE)
        {
            cpssOsMemSet(&ipv6Addr, 0, sizeof(ipv6Addr));

            ipv6Addr.arIP[0] = cpssOsRand() & 0xFE; /* Not multicast */
            ipv6Addr.arIP[1] = (lfsr_prefix >> 16) & 0xff;
            ipv6Addr.arIP[2] = (lfsr_prefix >>  8) & 0xff;
            ipv6Addr.arIP[3] = (lfsr_prefix >>  0) & 0xff;
            ipv6Addr.arIP[4] = (lfsr_prefix >> 16) & 0xff;
            ipv6Addr.arIP[5] = (lfsr_prefix >>  8) & 0xff;
            ipv6Addr.arIP[6] = (lfsr_prefix >>  0) & 0xff;
            ipv6Addr.arIP[7] = (lfsr_prefix >> 16) & 0xff;

            /* set prefix mask */
            found_prefix = GT_FALSE;
            for (prefix = 1; (prefix <= 64) && (found_prefix != GT_TRUE); ) {
                if (lfsr_val < prefix_len_db[prefix]) {
                    found_prefix = GT_TRUE; /* Found */
                } else {
                    prefix++; /* to next prefix length */
                }
            }

            ipv6Group.arIP[0]= 0xff;
            ipv6Group.arIP[1]= 0x00;
            ipv6Group.arIP[15]= ipv6Addr.arIP[6];

            rc = cpssDxChIpLpmIpv6McEntryAdd(prvTgfLpmDBId,
                                             prvUtfVrfId,
                                             &ipv6Group,
                                             128,
                                             &ipv6Addr,
                                             prefix,
                                             &mcRouteLttEntry,
                                             GT_FALSE,
                                             defragmentationEnable);
            if ( rc && (rc != GT_ALREADY_EXIST) ) {
                find_err = GT_TRUE;
            }
        }
        else /* Add an IPv4 prefix entry */
        {
            cpssOsMemSet(&ipv4Addr, 0, sizeof(ipv4Addr));

            /* Not multicast, 4b+4b */
            do {
                msb = ((cpssOsRand()&0xf)<<4) | (lfsr_prefix&0xf);
            } while ((msb >= 224) && (msb <= 239));

            data  = msb << 24;               /*  8b */
            data |= (lfsr_prefix>>4)<<9;     /* 15b */
            data |= cpssOsRand() & 0x1ff;    /*  9b */

            ipv4Addr.arIP[0] = (data >> 24) & 0xff;
            ipv4Addr.arIP[1] = (data >> 16) & 0xff;
            ipv4Addr.arIP[2] = (data >>  8) & 0xff;
            ipv4Addr.arIP[3] = (data >>  0) & 0xff;

            ipGroup.arIP[1] = 34;
            ipGroup.arIP[2] = ipv4Addr.arIP[1];
            ipGroup.arIP[3] = ipv4Addr.arIP[3];
            /* set prefix mask */
            found_prefix = GT_FALSE;
            for (prefix = 1; (prefix <= 32) && (found_prefix != GT_TRUE); ) {
                if (lfsr_val < prefix_len_db[prefix]) {
                    found_prefix = GT_TRUE; /* Found */
                } else {
                    prefix++; /* to next prefix length */
                }
            }

            rc = cpssDxChIpLpmIpv4McEntryAdd(prvTgfLpmDBId,
                                             prvUtfVrfId,
                                             &ipGroup,
                                             32,
                                             &ipv4Addr,
                                             prefix,
                                             &mcRouteLttEntry,
                                             GT_FALSE,
                                             defragmentationEnable);


            if ( rc && (rc != GT_ALREADY_EXIST) ) {
                find_err = GT_TRUE;
            }
        }
        /* Add done, do count */
        if (rc == GT_OK) {
            total_entrys++; pref_cnt[prefix]++;

            /* Next LFSR */
            lfsr_prefix = prvTgfIpLpmUtilShiftLFSR19b(lfsr_prefix);
            lfsr_val = prvTgfIpLpmUtilShiftLFSR10b(lfsr_val);
        }
    }
    return rc;
}


/**
* @internal prvTgfIpLpmIpv4Ipv6McPrefixAddMaxPrefix function
* @endinternal
*
* @brief  Flush all MC prefixes according to protocol
*
* @param[in] defragmentationEnable    - whether to enable
*                                      performance costing
*                                      de-fragmentation process
*                                      in the case that there is
*                                      no place to insert the
*                                      prefix. To point of the
*                                      process is just to make
*                                      space for this prefix.
*                                      (For DEVICES: xCat3; AC5;
*                                      Lion2: relevant only if
*                                      the LPM DB was created
*                                      with partitionEnable =
*                                      GT_FALSE)
* @param[out] addedNumIpv4Ptr  - (pointer to) the max number of ipv4 MC prefixes added
* @param[out] addedNumIpv6Ptr  - (pointer to) the max number of ipv6 MC prefixes added

*/
GT_STATUS prvTgfIpLpmIpv4Ipv6McPrefixAddMaxPrefix
(
    GT_BOOL defragmentationEnable,
    GT_U32 *maxAddedNumIpv4Ptr,
    GT_U32 *maxAddedNumIpv6Ptr
)
{
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT              nextHopInfo;
    GT_IPADDR                                           ipv4Addr;
    GT_IPV6ADDR                                         ipv6Addr;
    GT_U32                                              idx, total_entrys, prefix;
    GT_U32                                              data, lfsr_val, lfsr_prefix;
    GT_BOOL                                             found_prefix, find_err;
    GT_U32                                              prefix_len_db[66] = { 0 };
    GT_U32                                              pref_cnt[66] = {0};
    GT_U8                                               msb;
    GT_STATUS                                           rc;

    GT_IPADDR                    ipGroup;
    GT_IPV6ADDR                  ipv6Group;
    CPSS_DXCH_IP_LTT_ENTRY_STC   mcRouteLttEntry;

    /* Next hop */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;

    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
    cpssOsMemSet(&ipv6Group, 0, sizeof(GT_IPV6ADDR));

    mcRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E;
    mcRouteLttEntry.numOfPaths = 2;

    ipGroup.arIP[0] = 225;
    ipGroup.arIP[1] = 34;
    ipGroup.arIP[2] = 211;
    ipGroup.arIP[3] = 7;

    /* Random seed */
    cpssOsSrand(cpssOsTime());
    do {
        lfsr_val = cpssOsRand() & 0x3ff; /* 10bit */
    } while (lfsr_val == 0);

    /* For prefix database */
    do {
        lfsr_prefix = cpssOsRand() & 0x7ffff; /* 19bit */
    } while (lfsr_prefix == 0);

    /* Build the prefix length database */
    prvTgfIpLpmUtilRibPrefixLenDbBuild (GT_TRUE, prefix_len_db);

    find_err = GT_FALSE;
    total_entrys=0;
    for (idx = 0; (find_err != GT_TRUE); idx++)
    {
        /* Add an IPv6 prefix entry */
        cpssOsMemSet(&ipv6Addr, 0, sizeof(ipv6Addr));

        ipv6Addr.arIP[0] = cpssOsRand() & 0xFE; /* Not multicast */
        ipv6Addr.arIP[1] = (lfsr_prefix >> 16) & 0xff;
        ipv6Addr.arIP[2] = (lfsr_prefix >>  8) & 0xff;
        ipv6Addr.arIP[3] = (lfsr_prefix >>  0) & 0xff;
        ipv6Addr.arIP[4] = (lfsr_prefix >> 16) & 0xff;
        ipv6Addr.arIP[5] = (lfsr_prefix >>  8) & 0xff;
        ipv6Addr.arIP[6] = (lfsr_prefix >>  0) & 0xff;
        ipv6Addr.arIP[7] = (lfsr_prefix >> 16) & 0xff;

        /* set prefix mask */
        found_prefix = GT_FALSE;
        for (prefix = 1; (prefix <= 64) && (found_prefix != GT_TRUE); ) {
            if (lfsr_val < prefix_len_db[prefix]) {
                found_prefix = GT_TRUE; /* Found */
            } else {
                prefix++; /* to next prefix length */
            }
        }

        ipv6Group.arIP[0]= 0xff;
        ipv6Group.arIP[1]= 0x00;
        ipv6Group.arIP[15]= ipv6Addr.arIP[6];

        rc = cpssDxChIpLpmIpv6McEntryAdd(prvTgfLpmDBId,
                                         prvUtfVrfId,
                                         &ipv6Group,
                                         128,
                                         &ipv6Addr,
                                         prefix,
                                         &mcRouteLttEntry,
                                         GT_FALSE,
                                         defragmentationEnable);
        if ( rc && (rc != GT_ALREADY_EXIST) ) {
            find_err = GT_TRUE;
        }
          /* Add done, do count */
        if (rc == GT_OK) {
            total_entrys++; pref_cnt[prefix]++;

            /* Next LFSR */
            lfsr_prefix = prvTgfIpLpmUtilShiftLFSR19b(lfsr_prefix);
            lfsr_val = prvTgfIpLpmUtilShiftLFSR10b(lfsr_val);
        }
    }

    /* find max ipv6 */
    *maxAddedNumIpv6Ptr = total_entrys;
    /* flush UC ipv6*/
    rc = cpssDxChIpLpmIpv6McEntriesFlush(prvTgfLpmDBId, prvUtfVrfId);
    if (rc!=GT_OK)
    {
        return rc;
    }
    /* Build the prefix length database */
    cpssOsMemSet(prefix_len_db,0,sizeof(prefix_len_db));
    prvTgfIpLpmUtilRibPrefixLenDbBuild(GT_FALSE, prefix_len_db);

    find_err=GT_FALSE;
    total_entrys=0;
    for (idx = 0; (find_err != GT_TRUE); idx++)
    {
        /* Add an IPv4 prefix entry */
        cpssOsMemSet(&ipv4Addr, 0, sizeof(ipv4Addr));

        /* Not multicast, 4b+4b */
        do {
            msb = ((cpssOsRand()&0xf)<<4) | (lfsr_prefix&0xf);
        } while ((msb >= 224) && (msb <= 239));

        data  = msb << 24;               /*  8b */
        data |= (lfsr_prefix>>4)<<9;     /* 15b */
        data |= cpssOsRand() & 0x1ff;    /*  9b */

        ipv4Addr.arIP[0] = (data >> 24) & 0xff;
        ipv4Addr.arIP[1] = (data >> 16) & 0xff;
        ipv4Addr.arIP[2] = (data >>  8) & 0xff;
        ipv4Addr.arIP[3] = (data >>  0) & 0xff;

        ipGroup.arIP[1] = 34;
        ipGroup.arIP[2] = ipv4Addr.arIP[1];
        ipGroup.arIP[3] = ipv4Addr.arIP[3];
        /* set prefix mask */
        found_prefix = GT_FALSE;
        for (prefix = 1; (prefix <= 32) && (found_prefix != GT_TRUE); ) {
            if (lfsr_val < prefix_len_db[prefix]) {
                found_prefix = GT_TRUE; /* Found */
            } else {
                prefix++; /* to next prefix length */
            }
        }
        rc = cpssDxChIpLpmIpv4McEntryAdd(prvTgfLpmDBId,
                                         prvUtfVrfId,
                                         &ipGroup,
                                         32,
                                         &ipv4Addr,
                                         prefix,
                                         &mcRouteLttEntry,
                                         GT_FALSE,
                                         defragmentationEnable);

        if ( rc && (rc != GT_ALREADY_EXIST) ) {
            find_err = GT_TRUE;
        }
        /* Add done, do count */
        if (rc == GT_OK) {
            total_entrys++; pref_cnt[prefix]++;

            /* Next LFSR */
            lfsr_prefix = prvTgfIpLpmUtilShiftLFSR19b(lfsr_prefix);
            lfsr_val = prvTgfIpLpmUtilShiftLFSR10b(lfsr_val);
        }
    }

    /* find max ipv4 */
    *maxAddedNumIpv4Ptr = total_entrys;
    /* flush MC ipv4 */
    rc = cpssDxChIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, prvUtfVrfId);
    return rc;
}


