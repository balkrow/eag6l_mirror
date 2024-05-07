/**
   
   @copyright
   Copyright (c) 2004 - 2013, Rambus Inc. All rights reserved.
   
*/

/*
 *        Program: sshdns
 *        $Source: /srv/cvs/src/lib/sshutil/sshnet/sshnameserver.c,v $
 *        $Author: eaula $
 *
 *        Creation          : 11:45 Apr 22 2004 kivinen
 *        Last Modification : 17:13 Oct 24 2006 kivinen
 *        Last check in     : $Date: 2011-03-01 16:00:53 $
 *        Revision number   : $Revision: 1.39.2.1 $
 *        State             : $State: Exp $
 *        Version           : 1.525
 *        Edit time         : 207 min
 *
 *        Description       : This is offers the dns name server functions.
 *			      It uses either internal dns resolver
 *			      or system resolver depending on the
 *			      configuration. It can also support
 *			      other databases (/etc/hosts etc).
 *
 *        $Log: sshnameserver.c,v $
 *        Revision 1.39.2.1  2011-03-01 16:00:53  eaula
 *        Updated the header comments to comply with documentation scripts
 *        and with the QuickSec source commenting conventions.
 *
 *        Revision 1.39  2010/10/14 16:32:28  eaula
 *        Updated copyright notices to Authentec 2010 and into unified format.
 *        Removed authors and comments about creation and last modification.
 *
 *        Revision 1.38  2010/01/07 06:25:55  jpylkkanen
 *                Fix for case #17798. A memory leak when using windows
 *                and call to get_adapters_addresses fails.
 *
 *        Revision 1.37  2009/10/01 12:56:31  ssaarinen
 *        	Merging quicksec_4_4_ziic_branch to trunk. It is quite likely
 *        	that the trunk is unstable for some time after this merge...
 *
 *        Revision 1.36  2009/05/06 12:26:54  sponkanen
 *        	Merged quicksec_5_0_release to trunk.
 *
 *        Revision 1.35.4.2  2009/04/22 08:03:08  ken
 *        	Compiler warning fix from Tunski.
 *
 *        Revision 1.35.4.1  2009/04/02 14:25:52  jpylkkanen
 *        	Put all XXX comments inside SSHDIST_INPLACE in src/lib and
 *        	src/interceptor directories.
 *
 *        Revision 1.35  2008/08/05 08:53:06  jpylkkanen
 *        	Removed win2k support from net library for now. Actually placed it
 *        	into if 0's and SSHDIST_INPLACE.
 *
 *        Revision 1.34  2008/07/14 12:11:58  tunskitj
 *        	Fixed Windows CE compilation.
 *
 *        Revision 1.33  2008/07/04 05:41:59  rsinha
 *        	Added IPv6 DNS server support in Windows.
 *
 *        Revision 1.32  2008/04/02 12:45:53  mnippula
 *        	Committing fixes from quicksec_4_4_branch to TRUNK.
 *
 *        Revision 1.31.2.1  2008/03/14 07:49:05  ken
 *        	Fixed long lines to keep autodist.pl silent.
 *
 *        Revision 1.31  2008/03/03 13:40:57  sponkanen
 *        	Merged quicksec_4_3_branch_vs changes to trunk.
 *
 *        Revision 1.30.10.1  2008/01/24 13:07:16  sponkanen
 *        	Moved vxworks virtual stack changes from quicksec_4_3_branch
 *        	to quicksec_4_3_branch_vs.
 *
 *        Revision 1.30.8.1  2008/01/24 10:21:57  mnippula
 *        	Virtual stacks support for VxWorks. VxWorks interceptor 
 *              load/unloading and again loading/unloading works. Global 
 *              variables now use ssh_global almost globally.
 *
 *        Revision 1.30  2006/10/31 13:14:16  kivinen
 *        	Changed ssh_*printf and renderers to take unsigned char *.
 *
 *        Revision 1.29  2006/10/17 06:13:23  pmikkila
 *        	Added a couple of missing #ifdef SSHDIST_UTIL_DNS_RESOLVER's.
 *
 *        Revision 1.28  2006/09/14 10:19:07  kivinen
 *        	Fixed ifdefs.
 *
 *        Revision 1.27  2006/09/14 10:10:42  kivinen
 *        	Fixed ifdefs.
 *
 *        Revision 1.26  2006/05/02 14:53:59  sponkanen
 *        	Fixed a large number of compiler warnings
 *        	on win64.
 *
 *        Revision 1.25  2005/09/23 11:41:49  sponkanen
 *        	Moved declarations of ssh_tcp_get_host_addrs_by_name_sync()
 *        	and ssh_tcp_get_host_by_addr_sync() to sshnameserver.h
 *        	to reflect the documentation.
 *
 *        Revision 1.24  2005/08/19 13:45:24  tunskitj
 *        	Merged Windows specific implementation from client branch.
 *
 *        Revision 1.23  2005/06/27 08:31:22  irwin
 *        	Fixed numerous inconsistencies in the usage of signed vs.
 *        	unsigned character strings with regard to usernames, passwords
 *        	and URL's.
 *
 *        	Added alternative versions of C library string functions that
 *        	can be used for unsigned character strings.
 *
 *        	Fixed compiler warnings related to incorrect usage of
 *        	signed/unsigned parameters in functions.
 *
 *        Revision 1.22  2005/01/26 16:51:31  kivinen
 *        	Group commit from the 1st IKEv2 coding camp.
 *
 *        Revision 1.21  2005/01/05 11:07:59  kivinen
 *        	Allow ssh_name_server_init to be called multiple times. Later
 *        	calls simply reconfigure the name server, and also reread the
 *        	resolv.conf.
 *
 *        Revision 1.20  2004/12/30 12:52:48  kivinen
 *        	Allow name to be "[ipaddr]" and return "ipaddr" in that case.
 *
 *        Revision 1.19  2004/10/25 10:07:19  shaapanen
 *        	VxWorks: resolvlib.h -> resolvLib.h.
 *
 *        Revision 1.18  2004/10/07 07:27:13  kivinen
 *        	Added check for config being allocated.
 *
 *        Revision 1.17  2004/10/05 11:18:12  kivinen
 *        	Added ignore_default_safety_belt option.
 *
 *        Revision 1.16  2004/09/30 10:24:32  shaapanen
 *        	VxWorks: Read server addresses from VxWorks name resolver.
 *
 *        Revision 1.15  2004/07/28 13:10:37  kivinen
 *        	Fixed authentic to authorative.
 *
 *        Revision 1.14  2004/07/28 13:01:03  kivinen
 *        	Added allow_non_authentic_data and forward_dns_queries
 *        	options.
 *
 *        Revision 1.13  2004/06/24 12:17:27  tmo
 *        	Memory leak on nameserver operation abort.
 *
 *        Revision 1.12  2004/06/16 10:08:11  kivinen
 *        	If the request times out return SSH_TCP_TIMEOUT instead of
 *        	SSH_TCP_NO_ADDRESS.
 *
 *        Revision 1.11  2004/04/29 14:35:04  kivinen
 *        	Added ssh_name_server_resolver function.
 *
 *        Revision 1.10  2004/04/28 10:06:51  kivinen
 *        	Fixed bug when forward lookup fails completely.
 *
 *        Revision 1.9  2004/04/27 12:28:47  kivinen
 *        	Fixed typo.
 *
 *        Revision 1.8  2004/04/27 11:17:40  kivinen
 *        	Changed to use sshglobals.
 *
 *        Revision 1.7  2004/04/27 08:56:00  kivinen
 *        	Added code that will internally resolve 127.0.0.1 and ::1 to
 *        	localhost.
 *
 *        Revision 1.6  2004/04/26 15:25:31  kivinen
 *        	Enabled dns resolver again. Made sure that if
 *        	ENABLE_SYSTEM_DNS_RESOLVER is defined then this file does not
 *        	refer to the dns resolver, thus it is not linked to final
 *        	binary. Remove the final dot in the reverse name lookups.
 *        	Fixed couple of bugs. Added code that will resolve the
 *        	localhost locally without using dns.
 *
 *        Revision 1.5  2004/04/26 07:10:16  tmo
 *        	Extra tokens at end of ifdef.
 *
 *        Revision 1.4  2004/04/23 15:17:31  kivinen
 *        	Define ENABLE_SYSTEM_DNS_RESOLVER to force it to use system
 *        	resolver until we get bugs out from the dns library.
 *
 *        Revision 1.3  2004/04/23 15:12:43  kivinen
 *        	Fixed resolv conf parsing.
 *
 *        Revision 1.2  2004/04/23 14:28:48  kivinen
 *        	Added freeing of the resolv conf file buffer.
 *
 *        Revision 1.1  2004/04/23 08:33:17  kivinen
 *        	Created.
 *
 *        $EndLog$
 */

#include "strings.h"
#include "sshincludes.h"
#include "sshoperation.h"
#include "sshadt.h"
#include "sshadt_bag.h"
#include "sshadt_list.h"
#include "sshobstack.h"
#include "sshinet.h"




#include "sshnameserver.h"
#include "sshglobals.h"

#ifdef VXWORKS
#include "resolv/nameser.h"
#include "resolv/resolv.h"
#include "resolvLib.h"
#endif





#if 0
#ifndef ENABLE_SYSTEM_DNS_RESOLVER
#define ENABLE_SYSTEM_DNS_RESOLVER 1
#endif /* ENABLE_SYSTEM_DNS_RESOLVER */
#endif

#define SSH_DEBUG_MODULE "SshNameServer"












/* Internal global data. */
typedef struct SshNameServerDataRec {
  SshUInt32 timeout;		/* Timeout in useconds.  */
  Boolean use_system;






} *SshNameServerData, SshNameServerDataStruct;

/* Global data. This is allocated in the ssh_name_server_init and freed by the
   ssh_name_server_uninit. */
SSH_GLOBAL_DECLARE(SshNameServerData, ssh_name_server_data);
#define ssh_name_server_data SSH_GLOBAL_USE(ssh_name_server_data)
SSH_GLOBAL_DEFINE(SshNameServerData, ssh_name_server_data);

/* Looks up all ip-addresses of the host, returning them as a
   comma-separated list when calling the callback.  The host name may
   already be an ip address, in which case it is returned directly. This uses
   the system resolver.  */
SshOperationHandle
ssh_tcp_get_host_addrs_by_name_system(const unsigned char *name,
				      SshLookupCallback callback,
				      void *context)
{
  unsigned char *addrs;

  addrs = ssh_tcp_get_host_addrs_by_name_sync(name);
  if (addrs)
    {
      callback(SSH_TCP_OK, addrs, context);
      ssh_free(addrs);
    }
  else
    callback(SSH_TCP_NO_ADDRESS, NULL, context);
  return NULL;
}

/* Looks up the name of the host by its ip-address.  Verifies that the
   address returned by the name servers also has the original ip address.
   Calls the callback with either error or success.  The callback should
   copy the returned name. This uses the system resolver. */
SshOperationHandle
ssh_tcp_get_host_by_addr_system(const unsigned char *addr,
				SshLookupCallback callback,
				void *context)
{
  unsigned char *name;

  name = ssh_tcp_get_host_by_addr_sync(addr);
  if (name)
    {
      callback(SSH_TCP_OK, name, context);
      ssh_free(name);
    }
  else
    callback(SSH_TCP_NO_ADDRESS, NULL, context);
  return NULL;
}








































































































/* Initialize and configure the name server. */
Boolean ssh_name_server_init(SshNameServerConfig config)
{
#ifdef SSH_GLOBALS_EMULATION
  if (!SSH_GLOBAL_CHECK(ssh_name_server_data))
  {
    SSH_GLOBAL_INIT(ssh_name_server_data, NULL); 
  }
#endif /* SSH_GLOBALS_EMULATION */

  if (ssh_name_server_data == NULL)
    {
      ssh_name_server_data = ssh_calloc(1, sizeof(*ssh_name_server_data));
      if (ssh_name_server_data == NULL)
	return FALSE;
    }

#ifdef ENABLE_SYSTEM_DNS_RESOLVER
  /* Force to use system resolver. */
  ssh_name_server_data->use_system = TRUE;
  return TRUE;
#else /* ENABLE_SYSTEM_DNS_RESOLVER */
  if (config && config->use_system == TRUE)
    ssh_name_server_data->use_system = TRUE;































































  ssh_name_server_data->use_system = TRUE;

  return TRUE;
#endif /* ENABLE_SYSTEM_DNS_RESOLVER */
}

/* Unitialize the name server. */
void ssh_name_server_uninit(void)
{
  if (!ssh_name_server_data)
    return;







  ssh_free(ssh_name_server_data);
  ssh_name_server_data = NULL;
}


































































































































































































































































































































































































































































































































/* Looks up all ip-addresses of the host, returning them as a
   comma-separated list when calling the callback.  The host name may
   already be an ip address, in which case it is returned directly. */
SshOperationHandle
ssh_tcp_get_host_addrs_by_name(const unsigned char *name,
			       SshLookupCallback callback,
			       void *context)
{
  SshIpAddrStruct address[1];

  if (ssh_name_server_data == NULL)
    ssh_name_server_init(NULL);

  /* Check if it is IPv6 address in [addr] format. */
  if (*name == '[')
    {
      unsigned char *ret;
      size_t len;

      ret = ssh_strdup(name + 1);
      if (ret == NULL)
	{
	  callback(SSH_TCP_FAILURE, NULL, context);
	  return NULL;
	}
      len = strlen((char *)ret);
      if (len > 0)
	{
	  if (ret[len - 1] == ']')
	    {
	      ret[len - 1] = '\0';
	      if (ssh_ipaddr_parse(address, ret))
		{
		  callback(SSH_TCP_OK, ret, context);
		  ssh_free(ret);
		  return NULL;
		}
	    }
	}
      ssh_free(ret);
    }
  /* First check if it is already an ip address. */
  if (ssh_ipaddr_parse(address, name))
    {
      callback(SSH_TCP_OK, name, context);
      return NULL;
    }

  if (strcasecmp((char *)name, "localhost") == 0 ||
      strcasecmp((char *)name, "localhost.") == 0)
    {
#ifdef WITH_IPV6
      callback(SSH_TCP_OK, (unsigned char *)"127.0.0.1,::1", context);
#else /* WITH_IPV6 */
      callback(SSH_TCP_OK, (unsigned char*)"127.0.0.1", context);
#endif /* WITH_IPV6 */
      return NULL;
    }







  return ssh_tcp_get_host_addrs_by_name_system(name, callback, context);
}

/* Looks up the name of the host by its ip-address.  Verifies that the
   address returned by the name servers also has the original ip address.
   Calls the callback with either error or success.  The callback should
   copy the returned name. */
SshOperationHandle
ssh_tcp_get_host_by_addr(const unsigned char *addr,
			 SshLookupCallback callback,
			 void *context)
{
  unsigned char buffer[16];
  size_t len;

  if (ssh_name_server_data == NULL)
    ssh_name_server_init(NULL);

  len = 16;
  if (!ssh_inet_strtobin(addr, buffer, &len))
    {
      callback(SSH_TCP_FAILURE, NULL, context);
      return NULL;
    }
  if ((len == 4 && memcmp(buffer, "\x7f\x00\x00\x01", 4) == 0) ||
      (len == 16 &&
       memcmp(buffer, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1", 16) == 0))
    {
      callback(SSH_TCP_OK, (unsigned char *)"localhost", context);
      return NULL;
    }







  return ssh_tcp_get_host_by_addr_system(addr, callback, context);
}




































































































































































































































































































































