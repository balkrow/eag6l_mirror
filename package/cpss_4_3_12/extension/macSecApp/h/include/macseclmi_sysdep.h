/**

   @copyright
   Copyright (c) 2012 - 2021, Rambus Inc. All rights reserved.

   File: macseclmi_sysdep.h

   @description

   Interface to services dependent on UNIX system type, needed by
   MACsec LMI and data plane based on UNIX TUN/TAP devices.

*/


#ifndef _MACSECLMI_SYSDEP_H
#define _MACSECLMI_SYSDEP_H

/*
 * Get system-dependent definitions of MlEthif and MlTuntap.
 */
#include "macseclmi_sysdep_types.h"
#include <sys/time.h>
#include <time.h>

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x)
#endif

/*
 * Initialize/uninitialize an ethernet interface object associated
 * with the ethernet interface identified by `ifindex'. Return 1 if
 * initialization succeeded, 0 otherwise.
 */
int ml_init_ethif(MlEthif *e, const char *ifname);
void ml_uninit_ethif(MlEthif *e);

/*
 * Get the interface index and MAC address of a an ethernet interface.
 */
void ml_get_ethif_attributes(
  MlEthif *e, int *ifindex, unsigned char (*address)[6]);

/*
 * Get the link up and full duplex status of an ethernet
 * interface. Store 1 to *up if link is up, otherwise store 0. Store 1
 * to *fdx if full duplex is on, otherwise store 0. Return 1 if
 * successful, 0 otherwise.
 */
int ml_get_ethif_status(MlEthif *e, int *up, int *fdx);

/*
 * Initialize/uninitialize a TUN/TAP object, associated with an
 * ethernet interface object for the purposes of copying the MAC
 * address, MTU etc. settings from the ethernet interface to the
 * TUN/TAP device. Multiple TUN/TAP objects may be associated with a
 * single ethernet interface in which case each TUN/TAP object is
 * initialized using a different value of the `portid'
 * parameter. Return 1 if initialization succeeded, 0 otherwise.
 */
int ml_init_tuntap(MlTuntap *t, const MlEthif *e, unsigned portid);
void ml_uninit_tuntap(MlTuntap *t);

/*
 * Bring a TUN/TAP interface up or down.
 */
void ml_enable_tuntap(MlTuntap *t);
void ml_disable_tuntap(MlTuntap *t);

/*
 * Return a pollable file handle associated with an ethernet interface
 * or TUN/TAP device.
 */
int ml_ethif_fd(const MlEthif *e);
int ml_tuntap_fd(const MlTuntap *t);

/*
 * Read/write a packet from/to an ethernet interface or TUN/TAP
 * device. Return number of bytes transferred (possibly zero), or -1
 * on error.
 */
int ml_read_ethif(MlEthif *e, unsigned char *buf, unsigned len);
int ml_write_ethif(MlEthif *e,const unsigned char *buf,unsigned len);
int ml_read_tuntap(MlTuntap *t, unsigned char *buf, unsigned len);
int ml_write_tuntap(MlTuntap *t,const unsigned char *buf,unsigned len);

/*
 * Return the least significant bits of system uptime in hundreths of
 * a second.
 */
void ml_realtime(struct timespec *tp);

/*
 * Return the least significant bits of system uptime in hundreths of
 * a second.
 */
unsigned ml_uptime(void);

/*
 * Error output for the system-dependent part (not implemented by it).
 */
void ml_error(const char *fmt, ...)
     __attribute__ ((format (printf, 1, 2)));


/*
 * Error output for the system-dependent part (not implemented by it).
 */
void ml_error_arg(int error, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));


#endif /* _MACSECLMI_SYSDEP_H */
