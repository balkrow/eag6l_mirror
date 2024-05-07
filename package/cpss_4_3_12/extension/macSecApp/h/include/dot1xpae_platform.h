/**

   IEEE 802.1X-2010 platform-dependent definitions.

   File: dot1xpae_platform.h

   @description

   @copyright
   Copyright (c) 2012 - 2021, Rambus Inc. All rights reserved.

*/

#ifndef _DOT1XPAE_PLATFORM_H
#define _DOT1XPAE_PLATFORM_H

/*
 * The maximum number of simultaneous MKA instances (and CAKs) per
 * ethernet interface. The absolute minimum value is 2, to support
 * seamless CAK renewal. To support a large number of virtual ports
 * (each requiring an MKA instance) this parameter might be made as
 * large as twice the maximum number of MACsec peers. This parameter
 * has a big effect on the size of the Dot1xIntf object,
 * i.e. per-interface RAM consumption.
 */
#define DOT1X_MAX_MKA_INSTANCES 8

/*
 * The maximum number of simultaneous PACP machines per ethernet
 * interface. For a supplicant, one is enough. For an authenticator a
 * larger number up to the maximum number of MACsec peers is
 * suitable. Note that a number smaller than the maximum number of
 * peers delays authentication of some peers in case all peers come up
 * at the same time.
 */
#define DOT1X_MAX_PACPS 4

/*
 * Number of key identifier / packet number pairs to keep per MKA
 * instance for guaranteeing that a SAK-nonce pair is not reused. The
 * minimum number required by 802.1X section 9.8 is four.
 */
#define DOT1X_MAX_MKA_SAK_PNS 4

/*
 * The maximum number of management-configured network objects per
 * ethernet interface. Also maximum number of networks in an
 * announcement or announcement request.
 */
#define DOT1X_MAX_NETWORKS 4

/*
 * The maximum number of cipher suites supported per network in a
 * network announcement PDU.
 */
#define DOT1X_ANPDU_MAX_CIPHER_SUITES 4


void
dot1x_platform_log(const char *fmt, ...);

#endif /* _DOT1XPAE_PLATFORM_H */
