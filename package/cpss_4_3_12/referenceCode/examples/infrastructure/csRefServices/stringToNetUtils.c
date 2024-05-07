/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* stringToNetUtils.c
*
* DESCRIPTION:
*  Convert presentation-level(string) Mac/IPv6/IPv4 address to network order
*  binary form.
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include "../../prvCsrefLog.h"
#include "stringToNetUtils.h"

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsStr.h>
/* get the device info and common info */

#include <cpss/generic/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include "../../../../cpssEnabler/mainOs/h/gtOs/cpssOs.h"


/*****************************************************/
/**************** Static Varaibles       ********************/
/*****************************************************/



/********************************************************************************/
/********************** String to IP binary address convert *********************/
/********************************************************************************/

static GT_32 prvDecToValue (GT_CHAR ch)
{
  if ('0' <= ch && ch <= '9')
    return ch - '0';
  return -1;
}

/* Convert IPv6 address from string to network order binary
   form.
 */
#define IPV4_ADDR_SIZE  4

static GT_STATUS prvInetStrToIpv4Arr
(
  GT_CHAR   *src,
  GT_U8     *dst
)
{
  GT_32   nibleVal, digitsCnt, charCntr, delimiterFound, current;
  GT_U8   ipv4Byte, tmp[IPV4_ADDR_SIZE];
  GT_CHAR ch;

  osMemSet (tmp, 0, IPV4_ADDR_SIZE);
  current = 0;
  delimiterFound = 0;
  digitsCnt = 0;
  ipv4Byte=0;
  ch = *src;
  charCntr = 0;
  while(ch != 0)
  {
    charCntr++;
    if(current == IPV4_ADDR_SIZE)
       CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)

    if(ch != '.')
    {
      if(digitsCnt == 3)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM);

      /* Must be 2 nibles, so read 2 */
      nibleVal = prvDecToValue(ch);
      if(nibleVal < 0)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)
      digitsCnt++;
      ipv4Byte = ipv4Byte*10 + nibleVal;

      delimiterFound = 0;
    }
    else
    {
      if(delimiterFound == 1)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)

      tmp[current++] = ipv4Byte;
      delimiterFound++;
      digitsCnt = 0;
      ipv4Byte = 0;
    }

    src++;
    ch = *src;
  }

  if(charCntr == 0)
     CSREF_LOG_RETURN_MAC(GT_BAD_SIZE)

  tmp[current++] = ipv4Byte;/* copy last ipv4 byte*/
  if(current < IPV4_ADDR_SIZE)
      CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)

  osMemCpy (dst, tmp, IPV4_ADDR_SIZE);

  return GT_OK;
}

static GT_32 prvHexToValue (GT_CHAR ch)
{
  if ('0' <= ch && ch <= '9')
    return ch - '0';
  if ('A' <= ch && ch <= 'F')
    return ch - 'A' + 10;
  if ('a' <= ch && ch <= 'f')
    return ch - 'a' + 10;
  return -1;
}

/* Convert IPv6 address from string to network order binary
   form.
 */
#define IPV6_ADDR_SIZE  16
static GT_STATUS prvInetStrToIpv6Arr
(
  IN  GT_CHAR *src,
  OUT GT_U8 *dst
)
{
  GT_32   nibleVal, nibleCnt, charCntr, delimiterFound, doubleFoundInd;
  GT_U16  ipv6Bytes;
  GT_U8   tmp[IPV6_ADDR_SIZE], current;
  GT_CHAR ch;


  osMemSet (tmp, 0, IPV6_ADDR_SIZE);
  current = 0;
  delimiterFound = 0;
  doubleFoundInd = -1;
  nibleCnt = 0;
  ipv6Bytes=0;
  ch = *src;
  charCntr = 0;
  while(ch != 0)
  {
    charCntr++;
    if(current == IPV6_ADDR_SIZE)
       CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)

    if(ch != ':')
    {
      if(nibleCnt == 4)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)

      nibleVal = prvHexToValue(ch);
      if(nibleVal < 0)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)

      nibleCnt++;
      ipv6Bytes = (ipv6Bytes<<4)| nibleVal;
      delimiterFound=0;
    }
    else
    {
      if(delimiterFound == 2)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)
      delimiterFound++;
      if(nibleCnt > 0) /* it is smaller than 5. */
      {
        tmp[current++] = (ipv6Bytes >> 8) & 0xFF;
        tmp[current++] = ipv6Bytes & 0xFF;
        nibleCnt = 0;
        ipv6Bytes=0;
      }
      else if(delimiterFound == 2)
      {
        if(doubleFoundInd >= 0)
           CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)
        doubleFoundInd = current;
      }
    }
    src++;
    ch = *src;
  }
  /* Last 2 bytes */
  if(nibleCnt > 0) /* samller than 4. */
  {
    tmp[current++] = (ipv6Bytes >> 8) & 0xFF;
    tmp[current++] = ipv6Bytes & 0xFF;
  }

  if(charCntr == 0)
     CSREF_LOG_RETURN_MAC(GT_BAD_SIZE)

  if((doubleFoundInd == -1) && (current < 16))
      CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)
  if(current == IPV6_ADDR_SIZE)
  {
     osMemCpy (dst, tmp, 16);
  }
  else
  {
    osMemSet (dst, 0, IPV6_ADDR_SIZE);
    osMemCpy (dst, tmp, doubleFoundInd);
    osMemCpy (dst + doubleFoundInd + (IPV6_ADDR_SIZE-current), tmp+doubleFoundInd, current - doubleFoundInd);
  }
  return GT_OK;
}

/* Convert MAC address from string to network order binary
   form.
 */
#define MAC_ADDR_SIZE   6

static GT_STATUS prvInetStrToMacArr
(
  IN  GT_CHAR *src,
  OUT GT_U8 *dst
)
{
  GT_32   nibleVal, nibleCnt, charCntr, delimiterFound, current;
  GT_U8   macByte, tmp[MAC_ADDR_SIZE];
  GT_CHAR ch;


  osMemSet (tmp, 0, MAC_ADDR_SIZE);
  current = 0;
  delimiterFound = 0;
  nibleCnt = 0;
  macByte=0;
  ch = *src;
  charCntr = 0;
  while(ch != 0)
  {
    charCntr++;
    if(current == MAC_ADDR_SIZE)
       CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)

    if(ch != ':')
    {
      if(nibleCnt == 2)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM);

      /* Must be 2 nibles, so read 2 */

      nibleVal = prvHexToValue(src[0]);
      if(nibleVal < 0)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)
      nibleCnt++;
      macByte = nibleVal<<4;

      nibleVal = prvHexToValue(src[1]);
      if(nibleVal < 0)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)
      nibleCnt++;
      macByte = macByte | (nibleVal&0xF);

      tmp[current++] = macByte;

      delimiterFound = 0;
      src+=2;
    }
    else
    {
      if(delimiterFound == 1)
         CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)
      delimiterFound++;
      nibleCnt = 0;
      macByte = 0;
      src++;
    }
    ch = *src;
  }

  if(charCntr == 0)
     CSREF_LOG_RETURN_MAC(GT_BAD_SIZE)

  tmp[current++] = macByte; /* last MAC address byte. */
  if(current < MAC_ADDR_SIZE)
      CSREF_LOG_RETURN_MAC(GT_BAD_PARAM)

  osMemCpy (dst, tmp, MAC_ADDR_SIZE);

  return GT_OK;
}



/**
* @internal csRefInetStringToNet function
* @endinternal
*
* @brief   This function convert presentation-level(string)
*          Mac/IPv6/IPv4 address to network order binary form.
*
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  addressTypeaf - type of address to convert MAC, IPV4 or IPv6.
* @param[in] src            - String that contains the address to convert.
                              Address format should be according to standard
* @param[out] dst           - Array to store the address's binary format.
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS csRefInetStringToNet
(
  IN  INET_ADDRESS_TYPE addressType,
  IN  char             *src,
  OUT GT_U8            *dst
)
{
  GT_STATUS rc = GT_OK;
  switch (addressType)
  {
    case INET_ADDRESS_IPV4:
      rc = prvInetStrToIpv4Arr(src, dst);
      break;
    case INET_ADDRESS_IPV6:
      rc = prvInetStrToIpv6Arr(src, dst);
      break;
    case INET_ADDRESS_MAC:
      rc = prvInetStrToMacArr(src, dst);
      break;
    default:
      CSREF_LOG_RETURN_NOT_OK_MAC(GT_BAD_PARAM);;
  }
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  return GT_OK;
}




/* Incremenmt */
static GT_BOOL prvCsrefIncOnecell(GT_U8 *val, GT_U32 incVal)
{
    incVal = incVal + (GT_U32)(*val);
    (*val) =  (GT_U8)(incVal & 0xFF);

    /* Check for incrementing higher order cell. */
    if (incVal >= 256) {
        return GT_TRUE;
    }
    else {
        return GT_FALSE;
    }
}

static GT_STATUS prvCsrefIncOneArray
(
   GT_U8  *array,
   GT_U32  numOfCells,
   GT_U32  incVal
)
{
    GT_U32 place=0;

    while( (place < numOfCells) &&
            prvCsrefIncOnecell(&(array[numOfCells-1-place]), incVal))
    {
        place++;
        incVal = 1;
    }

    return GT_OK;
}


/**
* @internal csRefIncAddress function
* @endinternal
*
* @brief   This function increment Mac/IPv6/IPv4 addrress by one.
*
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  addressType - type of address to convert MAC, IPV4 or IPv6.
* @param[in]  netAddress  - Array of bytes contains the address to convert.
                            Address format is network order.
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS csRefIncAddress
(
  IN     INET_ADDRESS_TYPE addressType,
  INOUT  GT_U8            *netAddress,
  IN     GT_U32            prefix
)
{
  GT_U32 numOfCells, incVal;


  switch (addressType)
  {
    case INET_ADDRESS_IPV4:
      numOfCells = 4;
      incVal = 1 << (32-prefix);
      break;
    case INET_ADDRESS_IPV6:
      numOfCells = 16;
      incVal = 1; /* Always */
      break;
    case INET_ADDRESS_MAC:
      numOfCells = 6;
      incVal = 1; /* Always */
      break;
    default:
      return GT_BAD_PARAM;
  }

  return prvCsrefIncOneArray(netAddress, numOfCells, incVal);
}


GT_STATUS csRefInetPrint
(
  IN  INET_ADDRESS_TYPE addressType,
  IN  char             *src
)
{
  GT_U8  tmp[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  osPrintf("\nAddresss before %s\n", src);


  csRefInetStringToNet(addressType, src, tmp);
  switch (addressType)
  {
    case INET_ADDRESS_IPV4:
      osPrintf("\n IPv4 addresss after %d.%d.%d.%d\n",
                              tmp[0],tmp[1],tmp[2],tmp[3]);
      break;
    case INET_ADDRESS_IPV6:
      osPrintf("\nIPv6 addresss after %X %X:%X %X:%X %X:%X %X:%X %X:%X %X:%X %X:%X %X\n",
          tmp[0],tmp[1],tmp[2],tmp[3],
          tmp[4],tmp[5],tmp[6],tmp[7],
          tmp[8],tmp[9],tmp[10],tmp[11],
          tmp[12],tmp[13],tmp[14],tmp[15]);

      break;
    case INET_ADDRESS_MAC:
        osPrintf("\nMAC addresss after %X:%X:%X:%X:%X:%X\n",
                              tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);

      break;
    default:
      return GT_BAD_PARAM;
  }

  return 0;
}


