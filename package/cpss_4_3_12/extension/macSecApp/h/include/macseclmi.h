/**

   IEEE 802.1AE (MACsec) layer management interface (LMI) and
   associated configuration functions.

   File: macsec_lmi.h

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#ifndef _MACSEC_LMI_H
#define _MACSEC_LMI_H

/*
 * Get platform-dependent definitions of MACSEC_MAX_PEERS,
 * MacsecConfig, MacsecIntf, MacsecPeer.
 */
#include "macseclmi_types.h"

#include "macsec_types.h"
#include <stdint.h>

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x)
#endif

/*
 * General functions.
 */

/** Get the capabilities of the MACsec implementation.

    @param capabilities
    Pointer to an object into which the capability variables.
*/

void
macsec_get_capabilities(
  MacsecCapabilities *capabilities);

/*
 * Interface functions.
 */

/** Get the attributes of an interface object.

    @param intf_id
    Idenfitier for an interface object.

    @param ifindex
    Pointer to a variable into which the index of the associated
    ethernet interface is stored.

    @param address
    Pointer to a MAC address object into which the MAC address of the
    associated ethernet interface is stored.
*/
void
macsec_get_intf_attributes(
  MacsecIntfId intf_id,
  unsigned *ifindex,
  MacsecAddress *address);


/* Implementation specific flags to interface.

   @param intf_id
   Idenfitier for an interface object.

   @param flags
   The new set of flags to set.
*/
void
macsec_set_intf_flags(
  MacsecIntfId intf_id,
  unsigned flags);

/*
 * Peer functions.
 */

/** Create a peer object in an interface.

    @param intf_id
    Idenfitier for for an interface object.

    @param portid
    MACsec port identifier of the local port (1 or greater).

    @param peer_address
    Pointer to a MAC address object containing the MAC address of the
    remote station, copied to the new peer object.

    @param peer_portid
    MACsec port identifier of the remote port (1 or greater).

    @return
    Identifier for the created peer object or 0 if failed.

    @description
    A peer object represents a remote ethernet station that has MACsec
    connectivity with the local station (represented by the interface
    object). Within an interface object, only one peer object may be
    associated with a particular remote station and the peer address
    must be unique.

    When a peer object is created a MACsec receive SC is also created
    and the macsec_create_receive_sa() can be subsequently used to set
    up receive SAs to process traffic sent by the remote station.

    If portid (the local port identifier) is 1 then the peer is
    associated with the real port of the MACsec interface. Multiple
    peers may be associated with the real port. Communication with all
    peers of the real port uses the same cryptographic parameters such
    as the SA key.

    If portid is greater than 1 then a virtual port with the given
    port identifier is created and the peer is associated with that
    virtual port. Only one peer may be associated with a virtual port
    and the port is automatically destroyed when the peer is
    destroyed. Communication with peers of different virtual ports
    uses different cryptographic parameters such as SA keys.

    The peer_portid paramer identifies the real port or a virtual port
    in the remote station and is part of the peer's SCI but has no
    other meaning in the local station.

    This function can fail if the maximum number of peers would be
    exceeded, a peer with the given peer_address already exists or a
    virtual port with the given portid already exists.
*/
MacsecPeerId
macsec_create_peer(
  MacsecIntfId intf_id,
  unsigned portid,
  const MacsecAddress *peer_address,
  unsigned peer_portid);

/** Destroy a peer object.

    @param peer_id
    Idenfitier for a peer object previously returned by
    macsec_create_peer() (or macsec_next_peer()).

    @description
    This function destroys a MACsec peer object and removes the
    associated receive SA and receive SC. If a virtual port was
    associated with the peer then that port is destroyed as well.
*/
void
macsec_destroy_peer(
  MacsecPeerId peer_id);

/** Iterate to the next peer object of an interface.

    @param intf_id
    Interface object idenfitier.

    @param prev_id
    Either 0 or idenfitier for a peer returned by a previous call to
    this function.

    @return
    Idenfitier for the next peer object or 0 if there are no objects
    left.

    @description
    This function can be used to retrieve identifier to all peers of the
    interface. In the first call, prev_id should be 0; in each
    subsequent call, prev_id should be the identifier returned by the
    preceding call. When there are no more peers in the interface the
    function returns 0.

    Other functions that might cause the set of peers to be modified
    must not be called during a chain of calls to this function, with
    the exception that destroying the previous peer (pointed to by
    prev) using macsec_destroy_peer() after a call to this function is
    permitted and does not affect retrieval of the following peers.
*/
MacsecPeerId
macsec_next_peer(
  MacsecIntfId intf_id,
  MacsecPeerId prev_id);

/** Get the attributes of a peer object.

    @param peer_id
    Identifier for a peer object previously returned by macsec_next_peer()
    (or macsec_create_peer()).

    @param portid
    Pointer to a variable into which the local MACsec port identifier
    associated with the peer is stored.

    @param peer_address
    Pointer to a MAC address object into which the MAC address of the
    remote station is stored.

    @param peer_portid
    Pointer to a variable into which the MACsec port identifier in the
    remote station is stored.
*/
void
macsec_get_peer_attributes(
  MacsecPeerId peer_id,
  unsigned *portid,
  MacsecAddress *peer_address,
  unsigned *peer_portid);

/** Get the values of the status variables of a receive SC of a peer.

    @param peer_id
    Idenfitier for a peer object.

    @param stat
    Pointer to an object into which the values of the status variables
    are stored.
*/
void
macsec_get_receive_sc_stat(
   MacsecPeerId peer_id,
   MacsecReceiveScStat *stat);

/*
 * Real or virtual port functions.
 */

/** Set the authorization data of a real or virtual MACsec port.

    @param intf_id
    Idenfitier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param auth_data
    Pointer to an object from which authorization data is copied.

    @description
    This function is used to associate a MACsec port of an interface
    with authorization information obtained e.g. from an EAP
    authentication. The real port (port identifier 1) and each virtual
    port (port identifier greater than 1) may have different
    authorization information. This function is called before enabling
    the MACsec port using macsec_set_controlled_port_enabled().

    The interpretation of the the authorization data is
    implementation-dependent.
*/
void
macsec_set_authorization(
  MacsecIntfId intf_id,
  unsigned portid,
  const MacsecAuthData *auth_data);

/** Enable or disable a real or virtual MACsec port.

    @param intf
    Idenfitier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param value
    The desired value of the ControlledPortEnabled control
    variable. Zero means false, nonzero means true.

    @description
    This function sets the state of the ControlledPortEnabled MACsec
    control variable that affects operational state of the MAC
    interface associated with the real or virtual MACsec port. The
    operational state of the MAC interface is 'up' if and only
    ControlledPortEnabled is TRUE, the underlying ethernet inteface is
    up and there is MACsec connectivity with at last one
    peer. Otherwise the operational state of the MAC interface is
    down.

    This function is used to enable bringing up the secure interface
    associated with the real or virtual MACsec port after establishing
    MACsec connectivity (as determined by the MKA protocol) and to
    bring it down after MACsec connectivity terminates.
*/
void
macsec_set_controlled_port_enabled(
  MacsecIntfId intf_id,
  unsigned portid,
  int value);

/** Set the values of the SecY control variables of a real or virtual
    MACsec port.

    @param intf_id
    Idenfitier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param ctrl
    Pointer to an object from which new values for the control
    variables are copied.
*/
void
macsec_set_secy_ctrl(
  MacsecIntfId intf_id,
  unsigned portid,
  const MacsecSecyCtrl *ctrl);

/** Get the values of the SecY control variables of a real or virtual
    MACsec port.

    @param intf_id
    Identifer for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param ctrl
    Pointer to an object into which the values of the control
    variables are copied.
*/
void
macsec_get_secy_ctrl(
  MacsecIntfId intf_id,
  unsigned portid,
  MacsecSecyCtrl *ctrl);

/** Get the values of the SecY status variables of a real or virtual
    MACsec port.

    @param intf_id
    Identifier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param stat
    Pointer to an object into which the values of the status variables
    are copied.
*/
void
macsec_get_secy_stat(
  MacsecIntfId intf_id,
  unsigned portid,
  MacsecSecyStat *stat);

/** Get the values of the transmit SC status variables of a real or
    virtual MACsec port.

    @param intf_id
    Identifier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param stat
    Pointer to an object into which the values of the status variables
    are copied.
*/
void
macsec_get_transmit_sc_stat(
  MacsecIntfId intf_id,
  unsigned portid,
  MacsecTransmitScStat *stat);

/*
 * SA key (SAK) functions.
 */

/** Install a new SA key (SAK) into a real or virtual MACsec port.

    @param intf_id
    Identifier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param sak
    Pointer to a 128-bit or 256-bit AES key, accessed during the call
    only.

    @param ki
    Pointer to an object from which the key identifier (KI) associated
    with the SAK is copied.

    @param salt
    Pointer to to an object from which a salt value associated with
    the SAK is copied, or NULL if no salt is used.

    @return
    Pointer to the installed SAK, called a data key.

    @description
    This functions installs a new SAK and returns pointer to a data
    key object to be used in subsequent calls to
    macsec_create_transmit_sa() and macsec_create_receive_sa().

    This function does not fail. If the key storage of the MACsec port
    is full then the oldest key is automatically uninstalled to
    accommodate the new key. Note that the behavior of any SAs that
    were created with the uninstalled key and have not been disabled
    is undefined. This will not happen if, after a call to
    macsec_install_key(), all active SAs of the MACsec port are
    replaced using macsec_create_transmit_sa() or
    macsec_create_receive_sa() prior to the next call to
    macsec_install_key(). Instead of replacing, receive SAs may also
    be disabled via macsec_destroy_peer().

    The key identifier is stored but not used by the MACsec layer. Its
    purpose is to identify an installed SAK and it may be used in a
    platorm-dependent way.

    The salt is only used with cipher suites supporting extended
    packet numbers.
*/
void
macsec_install_key(
  MacsecIntfId intf_id,
  unsigned portid,
  const MacsecKey *sak,
  const MacsecKi *ki,
  const MacsecSalt *salt);

/*
 * Transmit SA functions.
 */

/** Create/replace a transmit SA of a real or virtual MACsec port.

    @param intf_id
    Identifier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param an
    Association number (AN) of the SA to be created (0 to 3).

    @param next_pn
    The packet number (PN) of the first packet sent using the new SA
    (1 or greater).

    @param ki
    Pointer to the key identifier (KI) previously used when installing the
    key with macsec_install_key().

    @param ssci
    Pointer to an object from which the SSCI associated with the SA is
    copied, or NULL if no SSCI is used.

    @description
    This function prepares a new transmit SA for transmission. The new
    SA is initially disabled and transmission continues using the
    previous transmit SA, if any, until macsec_enable_transmit() is
    called.

    The SSCI is only used with cipher suites supporting extended
    packet numbers.
*/
void
macsec_create_transmit_sa(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an,
  uint64_t next_pn,
  MacsecKi *ki,
  const MacsecSsci *ssci);

/** Enable transmission using a transmit SA of a real or virtual
    MACsec port.

    @param intf_id
    Identifier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param an
    Association number (AN) of the transmit SA to be enabled (0 to 3).

    @description
    This function switches transmission from a previus transmit SA to
    the transmit SA identified by an. Transmission using the new SA is
    in effect immediately.
*/
void
macsec_enable_transmit(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an);

/** Get the next packet number of a transmit SA of a real or virtual
    MACsec port.

    @param intf_id
    Identifier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param an
    Association number (AN) of the transmit SA to be read (0 to 3).

    @param next_pn
    Pointer to a variable into which the next packet number of the
    transmit SA is stored.
*/
void
macsec_get_transmit_sa_next_pn(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an,
  uint64_t *next_pn);

/** Get the values of the status variables of a transmit SA of a real
    or virtual MACsec port.

    @param intf_id
    Identifier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param an
    Association number (AN) of the transmit SA to be read (0 to 3).

    @param stat
    Pointer to an object into which the values of the status variables
    are stored.
*/
void
macsec_get_transmit_sa_stat(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an,
  MacsecTransmitSaStat *stat);

/** Get the latest Association number (AN) of the SA of a real or
    virtual MACsec port.

    @param intf_id
    Identifier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param latest_an
    Pointer to a variable into which the latest Association number
    (AN) of the SA is stored (0 to 3).
*/
void
macsec_get_latest_an(
  MacsecIntfId intf,
  unsigned portid,
  unsigned *latest_an);

/*
 * Receive SA functions.
 */

/** Create/replace a receive SA associated with a MACsec peer.

    @param peer_id
    Identifier for a MACsec peer object.

    @param an
    Association number (AN) of the SA to be created (0 to 3).

    @param lowest_pn
    The lowest packet number (PN) that the SA will initially accept (1
    or greater).

    @param ki
    Pointer to the key identifier (KI) previously used when installing the
    key with macsec_install_key().

    @param ssci
    Pointer to an object from which the SSCI associated with the SA is
    copied, or NULL if no SSCI is used.

    @description
    This function prepares a new receive SA for reception. The new SA
    is initially disabled and reception continues using the previous
    receive SA, if any, until macsec_enable_receive() is called.

    The SSCI is only used with cipher suites supporting extended
    packet numbers.
*/
void
macsec_create_receive_sa(
  MacsecPeerId peer_id,
  unsigned an,
  uint64_t lowest_pn,
  MacsecKi *ki,
  const MacsecSsci *ssci);

/** Enable reception using a receive SA associated with a MACsec peer.

    @param peer
    Idenfitier for a MACsec peer object.

    @param an
    Association number (AN) of the receive SA to be enabled (0 to 3).

    @description
    This function switches reception from a previus receive SA to the
    receive SA identified by an. Actual reception using the new SA
    does not necessarily begin immediately; the MACsec protocol
    continues to receive using the previous SA, if any, until the
    first frame with the AN of the new SA is received.
*/
void
macsec_enable_receive(
  MacsecPeerId peer_id,
  unsigned an);

/** Update the lowest acceptable packet number of a receive SA
    associated with a MACsec peer.

    @param peer_id
    Idenfitier for a MACsec peer object.

    @param an
    Association number (AN) of the SA to be updated (0 to 3).

    @param lowest_pn
    The lowest packet number that the SA should accept.
*/
void
macsec_set_receive_sa_lowest_pn(
  MacsecPeerId peer_id,
  unsigned an,
  uint64_t lowest_pn);

/** Get the values of the status variables of a receive SA associated
    with a MACsec peer.

    @param peer_id
    Idenfitier for a MACsec peer object.

    @param an
    Association number (AN) of the SA to be read (0 to 3).

    @param stat
    Pointer to an object into which the values of the status variables
    are copied.
*/
void
macsec_get_receive_sa_stat(
  MacsecPeerId peer_id,
  unsigned an,
  MacsecReceiveSaStat *stat);

/*
 * Uncontrolled port functions.
 */

/** Set the multicast MAC address for receiving EAPOL frames on an
    interface.

    @param intf_id
    Idenfitier for an interface object.

    @param address
    Pointer to an object from which the desired multicast address is
    copied.
*/
void
macsec_set_multicast_address(
  MacsecIntfId intf_id,
  const MacsecAddress *address);

/** Get the multicast MAC address for receiving EAPOL frames of an
    interface.

    @param intf_id
    Idenfitier for an interface object.

    @param address
    Pointer to an object into which the multicast address is stored.
*/
void
macsec_get_multicast_address(
  MacsecIntfId intf_id,
  MacsecAddress *address);

/** Get the operational state of the underlying ethernet interface.

    @param intf_id
    Idenfitier for an interface object.

    @param value
    Pointer to a variable into which the operational state is
    stored. 1 means up, 0 means not up.
*/
void
macsec_get_common_port_enabled(
  MacsecIntfId intf_id,
  int *value);

/** Try sending an ethernet frame using the uncontrolled port.

    @param intf_id
    Idenfitier for an interface object.

    @param dst
    Pointer to the destination MAC address.

    @param src
    Pointer to the source MAC address.

    @param msdubuf
    Pointer to a buffer containing the rest of the frame beginning
    with ethertype (MSDU).

    @param msdulen
    The length of the MSDU in bytes.

    @return
    1 if the frame was successfully sent, 0 otherwise.
*/
int
macsec_send_uncontrolled(
  MacsecIntfId intf_id,
  const MacsecAddress *dst,
  const MacsecAddress *src,
  const unsigned char *msdubuf,
  unsigned msdulen);

/** Try receiving an ethernet frame using the uncontrolled port.

    @param intf_id
    Idenfitier for an interface object.

    @param dst
    Pointer to an object into which the destination MAC address of the
    received frame is stored.

    @param src
    Pointer to an object into which the source MAC address of the
    received frame is stored.

    @param msdubuf
    Pointer to a buffer into which the rest of the frame beginning
    with ethertype (MSDU) is stored.

    @param msdulen
    The size of the MSDU buffer in bytes.

    @return
    Length of the received frame in bytes, 0 if no frame was received.
*/
unsigned
macsec_receive_uncontrolled(
  MacsecIntfId intf_id,
  MacsecAddress *dst,
  MacsecAddress *src,
  unsigned char *msdubuf,
  unsigned msdulen);

/*
 * Port status information functions.
 */

/** Get the values of the status variables of the common port of an
    interface.

    @param intf_id
    Idenfitier for an interface object.

    @param stat
    Pointer to an object into which the values of the status variables
    are stored.
*/
void
macsec_get_common_stat(
  MacsecIntfId intf,
  MacsecPortStat *stat);

/** Get the values of the status variables of the uncontrolled port of
    an interface.

    @param intf_id
    Idenfitier for an interface object.

    @param stat
    Pointer to an object into which the values of the status variables
    are stored.
*/
void
macsec_get_uncontrolled_stat(
  MacsecIntfId intf_id,
  MacsecPortStat *stat);

/** Get the values of the status variables of a real or virtual MACsec
    port (a controlled port) of an interface.

    @param intf_id
    Idenfitier for an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param stat
    Pointer to an object into which the values of the status variables
    are stored.
*/
void
macsec_get_controlled_stat(
  MacsecIntfId intf_id,
  unsigned portid,
  MacsecPortStat *stat);

/*
 * General utility functions.
 */

/** Output a diagnostic messages associated with an interface.

    @param intf_id
    Idenfitier for an interface object.

    @param fmt
    A printf-like format string.
*/
void
macsec_message(
  MacsecIntfId intf_id,
  const char *fmt,
  ...) __attribute__ ((format (printf, 2, 3)));

/** Perform internal test functions.

    @return
    1 if successful, 0 otherwise.
*/
int
macsec_test(void);

/*
 * Configuration functions.
 */

/** Implementation-dependent initialization of an interface object.

    @param config
    Pointer to implementation-dependent interface configuration data.

    @return
    Idenfitier for implementation-dependent interface object.

    @description
    This function initializes an interface object
    implementation-dependent configuration data. It is intended to be
    called by application code and is not used by the generic 802.1X
    protocol.
*/
MacsecIntfId
macsec_init_intf(
  const MacsecConfig *config);

/** Implementation-dependent uninitialization of an interface object.

    @param intf_id
    Idenfitier for an interface object previously returned by
    macsec_init_intf().

    @description
    This function is a companion function to macsec_init_initf(). It
    is intended to be called by application code and is not used by
    the generic 802.1X protocol.
*/
void
macsec_uninit_intf(
  MacsecIntfId intf_id);

/** Implementation-dependent function which iterates to the next
    interface object of a MACsec data plane.

    @param prev_id
    Idenfitier for an interface returned by a previous call
    to this function or 0.

    @return
    Idenfitier for the next interface object or 0 if there are no
    objects left.

    @description
    This function can be used to retrieve pointers to all interaces of
    the MACsec context. In the first call, prev_id should be 0; in each
    subsequent call, prev should be the identifier returned by the
    preceding call. When there are no more interfaces in the MACsec
    data plane the function returns 0.

    Other functions that might cause the set of interfaces to be
    modified must not be called during a chain of calls to this
    function.

    Function is intended to be called by application code and is not
    used by the generic 802.1X protocol.
*/
MacsecIntfId
macsec_next_intf(
  MacsecIntfId prev_id);

/** Implementation-dependent function to open connection to a MACsec
    data plane.

    @return
    Return 1 on success and 0 on failure.

    @description
    This function should be called when connection to the MACsec data
    plane is opened. It is intended to be called by application code
    and is not used by the generic 802.1X protocol.
*/
int
macsec_open(
  void);

/** Implementation-dependent function to close connection to a MACsec
    data plane.

    @description
    This function should be called when connection to the MACsec data
    plane is closed. It is intended to be called by application code
    and is not used by the generic 802.1X protocol.
*/
void
macsec_close(
        void);

int
macsec_init(
        void);

void
macsec_uninit(
        void);

#endif /* _MACSEC_LMI_H */
