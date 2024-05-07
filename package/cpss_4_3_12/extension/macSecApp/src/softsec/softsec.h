/**

   Software-based MACsec (802.1AE) protocol.

   File: softsec.h

   @description

   @copyright
   Copyright (c) 2012 - 2020 Rambus Inc. All rights reserved.

*/

#ifndef _SOFTSEC_H
#define _SOFTSEC_H

#include "softsec_platform.h"
#include "softsec_crypto.h"
#include "macsec_types.h"
#include "macsec_dict.h"

/*
 * Limits.
 */

/**  The maximum number of MACsec peers supported by an interface. */
#define SOFTSEC_MAX_PEERS 31

/*
 * Types.
 */

/*
 * Data key, i.e. an installed SAK.
 */
typedef struct {
  /* key identifier */
  MacsecKi ki;
#ifdef SOFTSEC_SOFTWARE_CRYPTO
  /* AES-GCM cryptographic context for software crypto */
  SoftsecCryptoContext crypto;
#endif  /* SOFTSEC_SOFTWARE_CRYPTO */
  /* AES-GCM handle for hardware crypto */
  void *crypto_handle;
  /* 32 MSBs of the salt for XPN suites in network byte order */
  uint32_t salthi;
  /* 64 LSBs of the salt for XPN suites in network byte order */
  uint64_t saltlo;
} SoftsecDkey;

/*
 * Transmit SA.
 */
typedef struct {
  /* the `enableTransmit' control */
  unsigned enable : 1;
  /* next packet number */
  uint64_t next_pn;
  /* pointer to data key */
  SoftsecDkey *dkey;
  /* SSCI for XPN suites in network byte order */
  uint32_t ssci;
  /* status information */
  MacsecTransmitSaStat stat;
} SoftsecTransmitSa;

/*
 * Transmit SC.
 */
typedef struct {
  /* current SA number */
  unsigned an;
  /* SAs */
  SoftsecTransmitSa sas[4];
  /* status information */
  MacsecTransmitScStat stat;
} SoftsecTransmitSc;

/*
 * Receive SA.
 */
typedef struct {
  /* the `enableReceive' control */
  unsigned enable : 1;
  /* lowest acceptable packet number */
  uint64_t lowest_pn;
  /* next packet number */
  uint64_t next_pn;
  /* pointer to data key */
  SoftsecDkey *dkey;
  /* SSCI for XPN suites in network byte order */
  uint32_t ssci;
  /* status information */
  MacsecReceiveSaStat stat;
} SoftsecReceiveSa;

/*
 * Receive SC.
 */
typedef struct {
  /* current SA number */
  unsigned an;
  /* previous SA number */
  unsigned oan;
  /* SAs */
  SoftsecReceiveSa sas[4];
  /* status information */
  MacsecReceiveScStat stat;
} SoftsecReceiveSc;

/*
 * A real or virtual port.
 */
typedef struct {
  /* data keys */
  SoftsecDkey dkeys[2];
  /* one-bit index of next data key to use */
  unsigned next_dkey;
  /* transmit SC */
  SoftsecTransmitSc transmit_sc;
  /* control information */
  MacsecSecyCtrl ctrl;
  /* status information */
  MacsecSecyStat stat;
  /* interface status information associated with the port */
  MacsecPortStat pstat;
  /* ControllerPortEnabled control */
  unsigned controlled_port_enabled : 1;
} SoftsecPort;

/*
 * MACsec peer, associated with a receive SC and potentially a virtual
 * port, identified by the peer MAC address.
 */
typedef struct {
  /* associated interface */
  struct SoftsecIntfS *intf;
  /* virtual port identifier; if less than 2, this peer uses the real port */
  unsigned portid;
  /* peer MAC address */
  MacsecAddress peer_address;
  /* port identifier of the receive SC */
  unsigned peer_portid;
  /* virtual port data;  unused if portid < 2 */
  SoftsecPort vport;
  /* receive SC */
  SoftsecReceiveSc receive_sc;
} SoftsecPeer;

/*
 * Dictionary of peer objects with remote MAC address as the key.
 */
MACSEC_DICT(
  SoftsecPeers, softsec_peers, SOFTSEC_MAX_PEERS,
  SoftsecPeer, peer, MacsecAddress, peer_address,
  peer->peer_address = *peer_address,
  memcmp(&peer->peer_address, peer_address, 6))

/*
 * Interface.
 */
typedef struct SoftsecIntfS {
  /* local MAC address */
  MacsecAddress address;
  /* peer dictionary */
  SoftsecPeers peers;
  /* mapping from virtual port identifier to peer (0 and 1 unused) */
  SoftsecPeer *vport_peer[2 + SOFTSEC_MAX_PEERS];
  /* real port data */
  SoftsecPort rport;
  /* MAC_Enabled of the common port */
  unsigned mac_enabled : 1;
  /* MAC_Operational of the common port */
  unsigned mac_operational : 1;
  /* operPointToPointMAC of the common port */
  unsigned oper_point_to_point_mac : 1;
  /* sysuptime in SNMP timeticks (hundredths of a second) for timestamps */
  uint32_t sysuptime;
} SoftsecIntf;

/*
 * State of an output packet descriptor.
 */
typedef enum {
  SOFTSEC_OUTPUT_PASS,
  SOFTSEC_OUTPUT_DROP,
  SOFTSEC_OUTPUT_ENCRYPT
} SoftsecOutputState;

/*
 * Output packet descriptor for softsec_output_begin() and
 * softsec_output_end().
 */
typedef struct {
  /* state after softsec_output_begin() */
  SoftsecOutputState state;
  /* destination buffer and length given to softsec_output_begin() */
  unsigned char *dstbuf;
  unsigned dstmax;
  /* source buffer given to softsec_output_begin() */
  const unsigned char *srcbuf;
  /* number of bytes of AAD in dstbuf */
  unsigned dstauth;
  /* number of bytes in srcbuf before reading data to encrypt */
  unsigned srcskip;
  /* number of bytes to encrypt */
  unsigned encrlen;
  /* crypto handle given to softsec_install_key_handle() */
  void *crypto_handle;
  /* initialization vector */
  unsigned char iv[12];
  /* buffer for ICV from crypto */
  unsigned char icv[16];
  /* final length of packet in dstbuf */
  unsigned dstlen;
  /* pointer to interface object */
  SoftsecIntf *intf;
  /* local port identifier */
  unsigned portid;
  /* pointer to transmit SC */
  SoftsecTransmitSc *tsc;
  /* pointer to transmit SA */
  SoftsecTransmitSa *tsa;
  /* nonzero if extended packet numbers in use */
  unsigned xpn : 1;
  /* nonzero if the E bit is set in SecTAG */
  unsigned ebit : 1;
} SoftsecOutputDescriptor;

/*
 * State of an input packet descriptor.
 */
typedef enum {
  SOFTSEC_INPUT_PASS,
  SOFTSEC_INPUT_DROP,
  SOFTSEC_INPUT_DECRYPT
} SoftsecInputState;

/*
 * Input packet descriptor for softsec_input_begin() and
 * softsec_input_end().
 */
typedef struct {
  /* state after softsec_input_begin() */
  SoftsecInputState state;
  /* destination buffer given to softsec_input_begin() */
  unsigned char *dstbuf;
  /* source buffer and length given to softsec_input_begin() */
  const unsigned char *srcbuf;
  unsigned srclen;
  /* number of bytes in dstbuf before storing decryption result */
  unsigned dstskip;
  /* number of bytes of AAD in srcbuf */
  unsigned srcauth;
  /* number of bytes to decrypt */
  unsigned decrlen;
  /* crypto handle given to softsec_install_key_handle() */
  void *crypto_handle;
  /* initialization vector */
  unsigned char iv[12];
  /* pointer to received ICV */
  const unsigned char *icv;
  /* nonzero if ICV was successfully verified by crypto */
  unsigned authentic;
  /* final length of packet in dstbuf */
  unsigned dstlen;
  /* pointer to interface object */
  SoftsecIntf *intf;
  /* local port identifier */
  unsigned portid;
  /* pointer to receive SC */
  SoftsecReceiveSc *rsc;
  /* pointer to receive SA */
  SoftsecReceiveSa *rsa;
  /* AN */
  unsigned an;
  /* PN */
  uint64_t pn;
  /* nonzero if extended packet numbers in use */
  unsigned xpn : 1;
  /* nonzero if validation mode is strict */
  unsigned strict : 1;
  /* nonzero if the E bit is set in SecTAG */
  unsigned ebit : 1;
  /* nonzero if the C bit is set in SecTAG */
  unsigned cbit : 1;
} SoftsecInputDescriptor;

/*
 * General functions.
 */

/** Get the capabilities of the MACsec implementation.

    @param capabilities
    Pointer to an object into which the capability variables.
*/
void
softsec_get_capabilities(
  MacsecCapabilities *capabilities);

/*
 * Interface functions.
 */

/** Initialize an interface object.

    @param intf
    Pointer to the interface object to initialize.

    @param address
    Pointer to a MAC address copied into the interface object.

    @description
    This function initializes an interface object, i.e. MACsec
    protocol state associated with a single ethernet interface. After
    initialization the control variables of the real port are set as
    follows (see IEEE 802.1AE-2006):

        validateFrames = Strict,
        replayProtect = True,
        replayWindow = 0,
        protectFrames = True,
        alwaysIncludeSCI = False,
        useES = False,
        useSCB = False.

    When a virtual port is created by softsec_create_peer(), its
    control variables are set in the same way. These values can be
    changed later using softsec_set_secy_ctrl().
*/
void
softsec_init_intf(
  SoftsecIntf *intf,
  const MacsecAddress *address);

/** Uninitialize an interface object.

    @param intf
    Pointer to the interface object to uninitialize.

    @description
    This function clears the contents of the interface object and any
    associated resources such as cryptographic keys.
 */
void
softsec_uninit_intf(
  SoftsecIntf *intf);

/** Update the MAC_Enabled, MAC_Operational and operPointToPointMAC
    parameter of the common port of an interface.

    @param intf
    Pointer to an interface object.

    @param enabled
    The new value for the MAC_Enabled parameter. A nonzero value means
    true, zero means false.

    @param operational
    The new value for the MAC_Operational parameter. A nonzero value
    means true, zero means false.

    @param point_to_point
    The new value for the operPointToPointMAC parameter. A nonzero
    value means true, zero means false.

    @description
    This function sets the MAC state flags of the common port,
    i.e. the port associated with the underlying ethernet
    interface. The flags are combined with transmit and receive SC
    state and the ControlledPortEnabled control of each controlled
    port to form the corresponding state flags of controlled ports
    associated with the interface. Note: the value of
    operPointToPointMAC should be 1 if the underlying ethernet
    interface is in full duplex mode, 0 otherwise.
*/
void
softsec_set_common_mac_state(
  SoftsecIntf *intf,
  unsigned enabled,
  unsigned operational,
  unsigned point_to_point);

/** Update the ControlledPortEnabled control variable of a real or
    virtual MACsec port.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param value
    The new value for the ControlledPortEnabled control variable. A
    nonzero value means true, zero means false.

   @description
   The ControlledPortEnabled control is combined with the MAC state of
   the common port and the transmit and receive SC state to set the
   MAC status variables of the controlled port.
*/
void
softsec_set_controlled_port_enabled(
  SoftsecIntf *intf,
  unsigned portid,
  unsigned value);

/** Update the sysuptime of an interface.

    @param intf
    Pointer to an interface object.

    @param sysuptime
    The new value for the 32-bit sysuptime.

    @description
    This function sets the sysuptime of the interface
    object. Sysuptime is used for informational purposes only, such as
    recording the time when a particular SA was created.
*/
void
softsec_set_sysuptime(
  SoftsecIntf *intf,
  uint32_t sysuptime);

/*
 * Peer functions.
 */

/** Create a peer object in an interface.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier of the local port (1 or greater).

    @param peer_address
    Pointer to a MAC address object containing the MAC address of the
    remote station, copied to the new peer object.

    @param peer_portid
    MACsec port identifier of the remote port (1 or greater).

    @return
    Pointer to the created peer object or NULL if failed.

    @description
    A peer object represents a remote ethernet station that has MACsec
    connectivity with the local station (represented by the interface
    object). Within an interface object, only one peer object may be
    associated with a particular remote station and the peer address
    must be unique.

    When a peer object is created a MACsec receive SC is also created
    and the softsec_create_receive_sa() can be subsequently used to
    set up receive SAs to process traffic sent by the remote station.

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
SoftsecPeer *
softsec_create_peer(
  SoftsecIntf *intf,
  unsigned portid,
  const MacsecAddress *peer_address,
  unsigned peer_portid);

/** Destroy a peer object.

    @param peer
    Pointer to a peer object previously returned by
    softsec_create_peer().

    @description
    This function destroys a MACsec peer object and removes the
    associated receive SA and receive SC. If a virtual port was
    associated with the peer then that port is destroyed as well.
*/
void
softsec_destroy_peer(
  SoftsecPeer *peer);

/** Get the values of the status variables of the receive SC of a
    peer.

    @param peer
    Pointer to a peer object.

    @param stat
    Pointer to an object into which the values of the status variables
    are stored.
*/
void
softsec_get_receive_sc_stat(
  SoftsecPeer *peer,
  MacsecReceiveScStat *stat);

/*
 * Real or virtual port functions.
 */

/** Set the values of the SecY control variables of a real or virtual
    MACsec port.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param ctrl
    Pointer to an object from which new values for the control
    variables are copied.
*/
void
softsec_set_secy_ctrl(
  SoftsecIntf *intf,
  unsigned portid,
  const MacsecSecyCtrl *ctrl);

/** Get the values of the SecY control variables of a real or virtual
    MACsec port.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param ctrl
    Pointer to an object into which the values of the control
    variables are copied.
*/
void
softsec_get_secy_ctrl(
  const SoftsecIntf *intf,
  unsigned portid,
  MacsecSecyCtrl *ctrl);

/** Get the values of the SecY status variables of a real or virtual
    MACsec port.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param stat
    Pointer to an object into which the values of the status variables
    are copied.
*/
void
softsec_get_secy_stat(
  const SoftsecIntf *intf,
  unsigned portid,
  MacsecSecyStat *stat);

/** Get the values of the port status variables of a real or virtual
    MACsec port.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param stat
    Pointer to an object into which the values of the status variables
    are copied.
*/
void
softsec_get_port_stat(
  const SoftsecIntf *intf,
  unsigned portid,
  MacsecPortStat *stat);

/** Get the values of the transmit SC status variables of a real or
    virtual MACsec port.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param stat
    Pointer to an object into which the values of the status variables
    are copied.
*/
void
softsec_get_transmit_sc_stat(
  const SoftsecIntf *intf,
  unsigned portid,
  MacsecTransmitScStat *stat);

/*
 * SAK functions.
 */

#ifdef SOFTSEC_SOFTWARE_CRYPTO
/** Install a new SA key (SAK) into a real or virtual MACsec port for
    software cryptography.

    @param intf
    Pointer to an interface object.

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
    softsec_create_transmit_sa() and softsec_create_receive_sa().

    This function does not fail. If the key storage is full then the
    oldest key is automatically uninstalled to accommodate the new
    key. Any SAs that were created with the uninstalled key and have
    not been disabled will erroneously reassociate with the new
    key. This is not a serious error and will not happen at all if,
    after a call to softsec_install_key(), all active SAs are replaced
    using softsec_create_transmit_sa() or softsec_create_receive_sa()
    prior to the next call to softsec_install_key(). Instead of
    replacing, receive SAs may also be disabled via
    softsec_destroy_peer().

    The salt is only used with cipher suites supporting extended
    packet numbers.
 */
SoftsecDkey *
softsec_install_key(
  SoftsecIntf *intf,
  unsigned portid,
  const MacsecKey *sak,
  const MacsecKi *ki,
  const MacsecSalt *salt);

/** Get the SoftsecDkey associated with the provided Key Identifier.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param ki
    Pointer to the key identifier (KI) of the requestedd key.

    @return
    Pointer to the data key or NULL if key was not found.
 */
SoftsecDkey *
softsec_get_dkey(
  SoftsecIntf *intf,
  unsigned portid,
  const MacsecKi *ki);

#endif /* SOFTSEC_SOFTWARE_CRYPTO */

/** Install a new SA key (SAK) into a real or virtual MACsec port for
    hardware cryptography.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param crypto_handle
    Pointer to an cryptographic resource associated with a 128-bit or
    256-bit AES key.

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
    softsec_create_transmit_sa() and softsec_create_receive_sa().

    The SAK is represented by a handle to a cryptographic resource
    that can perform AES-GCM authenticated encryption or decryption
    using the SAK. The handle can be later returned by
    softsec_get_encryption_parameters() or
    softsec_get_decryption_parameters().

    This function does not fail. If the key storage is full then the
    oldest key is automatically uninstalled to accommodate the new
    key. Any SAs that were created with the uninstalled key and have
    not been disabled will erroneously reassociate with the new
    key. This is not a serious error and will not happen at all if,
    after a call to softsec_install_key(), all active SAs are replaced
    using softsec_create_transmit_sa() or softsec_create_receive_sa()
    prior to the next call to softsec_install_key(). Instead of
    replacing, receive SAs may also be disabled via
    softsec_destroy_peer().

    The salt is only used with cipher suites supporting extended
    packet numbers.
 */
SoftsecDkey *
softsec_install_key_handle(
  SoftsecIntf *intf,
  unsigned portid,
  void *crypto_handle,
  const MacsecKi *ki,
  const MacsecSalt *salt);

/*
 * Transmit SA functions.
 */

/** Create/replace a transmit SA of a real or virtual MACsec port.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param an
    Association number (AN) of the SA to be created (0 to 3).

    @param next_pn
    The packet number (PN) of the first packet sent using the new SA
    (1 or greater).

    @param dkey
    Pointer to a data key previously returned by softsec_install_key().

    @param ssci
    Pointer to an object from which the SSCI associated with the SA is
    copied, or NULL if no SSCI is used.

    @description
    This function prepares a new transmit SA for transmission. The new
    SA is initially disabled and transmission continues using the
    previous transmit SA, if any, until softsec_enable_transmit() is
    called.

    The SSCI is only used with cipher suites supporting extended
    packet numbers.
*/
void
softsec_create_transmit_sa(
  SoftsecIntf *intf,
  unsigned portid,
  unsigned an,
  uint64_t next_pn,
  SoftsecDkey *dkey,
  const MacsecSsci *ssci);

/** Enable transmission using a transmit SA of a real or virtual
    MACsec port.

    @param intf
    Pointer to an interface object.

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
softsec_enable_transmit(
  SoftsecIntf *intf,
  unsigned portid,
  unsigned an);

/** Get the next packet number of a transmit SA of a real or virtual
    MACsec port.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param an
    Association number (AN) of the transmit SA to be read (0 to 3).

    @param next_pn
    Pointer to a variable into which the next packet number of the
    transmit SA is stored.
*/
void
softsec_get_next_pn(
  const SoftsecIntf *intf,
  unsigned portid,
  unsigned an,
  uint64_t *next_pn);

/** Get the values of the status variables of a transmit SA of a real
    or virtual MACsec port.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param an
    Association number (AN) of the transmit SA to be read (0 to 3).

    @param stat
    Pointer to an object into which the values of the status variables
    are stored.
*/
void
softsec_get_transmit_sa_stat(
  const SoftsecIntf *intf,
  unsigned portid,
  unsigned an,
  MacsecTransmitSaStat *stat);

/*
 * Receive SA functions.
 */

/** Create/replace a receive SA associated with a MACsec peer.

    @param peer
    Pointer to a MACsec peer object.

    @param an
    Association number (AN) of the SA to be created (0 to 3).

    @param lowest_pn
    The lowest packet number (PN) that the SA will initially accept (1
    or greater).

    @param dkey
    Pointer to a data key previously returned by
    softsec_install_key().

    @param ssci
    Pointer to an object from which the SSCI associated with the SA is
    copied, or NULL if no SSCI is used.

    @description
    This function prepares a new receive SA for reception. The new SA
    is initially disabled and reception continues using the previous
    receive SA, if any, until softsec_enable_receive() is called.

    The SSCI is only used with cipher suites supporting extended
    packet numbers.
*/
void
softsec_create_receive_sa(
  SoftsecPeer *peer,
  unsigned an,
  uint64_t lowest_pn,
  SoftsecDkey *dkey,
  const MacsecSsci *ssci);

/** Enable reception using a receive SA associated with a MACsec peer.

    @param peer
    Pointer to a MACsec peer object.

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
softsec_enable_receive(
  SoftsecPeer *peer,
  unsigned an);

/** Update the lowest acceptable packet number of a receive SA
    associated with a MACsec peer.

    @param peer
    Pointer to a MACsec peer object.

    @param an
    Association number (AN) of the SA to be updated (0 to 3).

    @param lowest_pn
    The lowest packet number that the SA should accept.
*/
void
softsec_set_lowest_pn(
  SoftsecPeer *peer,
  unsigned an,
  uint64_t lowest_pn);

/** Get the values of the status variables of a receive SA associated
    with a MACsec peer.

    @param peer
    Pointer to a MACsec peer object.

    @param an
    Association number (AN) of the SA to be read (0 to 3).

    @param stat
    Pointer to an object into which the values of the status variables
    are copied.
*/
void
softsec_get_receive_sa_stat(
  const SoftsecPeer *peer,
  unsigned an,
  MacsecReceiveSaStat *stat);

/*
 * Frame processing functions.
 */

#ifdef SOFTSEC_SOFTWARE_CRYPTO
/** Process an outbound MAC frame using a real or virtual MACsec port
    using software cryptography.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param dstbuf
    Pointer to a buffer into which the processed frame is stored.

    @param dstmax
    The size of the destination buffer in bytes.

    @param srcbuf
    Pointer to a buffer containing the frame to process.

    @param srclen
    Length of the frame to process in bytes.

    @return
    Length of the processed frame in bytes or 0.

    @description
    This function processes an outbound frame, applies MACsec
    protection according to the control variables of the real or
    virtual port and returns the length of the resulting frame. If the
    frame is rejected by MACsec, 0 is returned appropriate counters in
    the interface object are updated.

    @notes
    For correct reporting of too long packets, the maximum result
    length given in dstmax must equal the MTU of the common port and
    the unprotected frame length given in srclen must not exceed
    dstmax.
*/
unsigned
softsec_output(
  SoftsecIntf *intf,
  unsigned portid,
  unsigned char *dstbuf, unsigned dstmax,
  const unsigned char *srcbuf, unsigned srclen);

/** Process an inbound MAC frame using a real or virtual MACsec port
    using software cryptography.

    @param intf
    Pointer to an interface object.

    @param portid
    Pointer to a variable into which the identifier of the MACsec port
    is stored.

    @param dstbuf
    Pointer to a buffer into which the processed frame is stored.

    @param dstmax
    The size of the destination buffer in bytes.

    @param srcbuf
    Pointer to a buffer containing the frame to process.

    @param srclen
    Length of the frame to process in bytes.

    @return
    Length of the processed frame in bytes or 0.

    @description
    This function processes an inbound frame, looks up the appropriate
    real or virtual MACsec port of the interface, stores its
    identifier into the variable pointed to by portid, performs MACsec
    validation according to the control variables of the MACsec port
    and returns the length of the resulting frame. If the frame is
    rejected by MACsec, 0 is returned and appropriate counters in the
    interface object are updated.
*/
unsigned
softsec_input(
  SoftsecIntf *intf,
  unsigned *portid,
  unsigned char *dstbuf, unsigned dstmax,
  const unsigned char *srcbuf, unsigned srclen);
#endif /* SOFTSEC_SOFTWARE_CRYPTO */

/** Pre-process an outbound MAC frame using a real or virtual MACsec
    port so that it can be encrypted by hardware cryptography.

    @param outdesc
    Pointer to a descriptor object used to store frame processing
    state.

    @param intf
    Pointer to an interface object.

    @param portid
    MACsec port identifier (1 or greater).

    @param dstbuf
    Pointer to a buffer into which the processed frame is stored.

    @param dstmax
    The size of the destination buffer in bytes.

    @param srcbuf
    Pointer to a buffer containing the frame to process.

    @param srclen
    Length of the frame to process in bytes.

    @return
    One of SOFTSEC_OUTPUT_ENCRYPT, SOFTSEC_OUTPUT_PASS or
    SOFTSEC_OUTPUT_DROP.

    @description
    This function processes an outbound frame up to the point where
    either AES-GCM authenticated encryption can be performed or the
    frame can be immediately passed through or dropped. Frame
    processing state is stored in the descriptor outdesc. Memory for
    the descriptor object is managed by the caller and the object does
    not need any initialization before calling this function.

    If SOFTSEC_OUTPUT_ENCRYPT is returned, AES-GCM encryption must be
    performed on the frame before calling
    softsec_output_end(). AES-GCM encryption parameters can be
    extracted from the descriptor outdesc using
    softsec_get_encryption_parameters().

    If a value other than SOFTSEC_OUTPUT_ENCRYPT is returned,
    softsec_output_end() can be called directly.
*/
SoftsecOutputState
softsec_output_begin(
  SoftsecOutputDescriptor *outdesc,
  SoftsecIntf *intf,
  unsigned portid,
  unsigned char *dstbuf, unsigned dstmax,
  const unsigned char *srcbuf, unsigned srclen);

/** Post-process an outbound MAC frame after hardware cryptography or
    immediate pass/drop decision.

    @param outdesc
    Pointer to a descriptor object previously used with
    softsec_output_begin().

    @return
    Length of the processed frame in bytes or 0.

    @description
    This function continues processing an outbound MAC that was
    pre-processed by softsec_output_begin() and after that potentially
    encrypted using AES-GCM. The return value is the length of the
    resulting frame, or 0 if the frame is rejected by MACsec.
*/
unsigned
softsec_output_end(
  SoftsecOutputDescriptor *outdesc);

/** Extract AES-GCM encryption parameters from an outbound frame
    descriptor.

    @param outdesc
    Pointer to a descriptor object previously used with
    softsec_output_begin().

    @param crypto_handle
    Pointer to a variable that is set to contain a cryptographic
    resource handle, previously given to softsec_install_key_handle().

    @param ciphertext
    Pointer to an object that is set to contain the address and length
    of the area into which ciphertext is to be stored.

    @param tag
    Pointer to a pointer that is set to point to a buffer into which
    the AES-GCM authentication tag is to be stored.

    @param iv
    Pointer to a pointer that is set to point to the initialization
    vector.

    @param plaintext
    Pointer to an object that is set to contain the address and length
    of the area from which plaintext to encrypt is to be read.

    @param aad
    Pointer to an object that is set to contain the address and length
    of the area from which additional authenticated data (AAD) is
    read.
*/
void
softsec_get_encryption_parameters(
  SoftsecOutputDescriptor *outdesc,
  void **crypto_handle,
  SoftsecCryptoWriteBytes *ciphertext,
  SoftsecCryptoTag **tag,
  const SoftsecCryptoIv **iv,
  SoftsecCryptoReadBytes *plaintext,
  SoftsecCryptoReadBytes *aad);

/** Pre-process an inbound MAC frame using a real or virtual MACsec
    port so that it can be decrypted by hardware cryptography.

    @param indesc
    Pointer to a descriptor object used to store frame processing
    state.

    @param intf
    Pointer to an interface object.

    @param dstbuf
    Pointer to a buffer into which the processed frame is stored.

    @param dstmax
    The size of the destination buffer in bytes.

    @param srcbuf
    Pointer to a buffer containing the frame to process.

    @param srclen
    Length of the frame to process in bytes.

    @return
    One of SOFTSEC_INPUT_DECRYPT, SOFTSEC_INPUT_PASS or
    SOFTSEC_INPUT_DROP.

    @description
    This function processes an inbound frame up to the point where
    either AES-GCM authenticated decryption can be performed or the
    frame can be immediately passed through or dropped. Frame
    processing state is stored in the descriptor indesc. Memory for
    the descriptor object is managed by the caller and the object does
    not need any initialization before calling this function.

    If SOFTSEC_INPUT_DECRYPT is returned, AES-GCM decryption must be
    performed on the frame before calling softsec_input_end(). AES-GCM
    decryption parameters can be extracted from the descriptor indesc
    using softsec_get_decryption_parameters().

    If a value other than SOFTSEC_INPUT_DECRYPT is returned,
    softsec_input_end() can be called directly.
*/
SoftsecInputState
softsec_input_begin(
  SoftsecInputDescriptor *indesc,
  SoftsecIntf *intf,
  unsigned char *dstbuf, unsigned dstmax,
  const unsigned char *srcbuf, unsigned srclen);

/** Post-process an inbound MAC frame after hardware cryptography or
    immediate pass/drop decision.

    @param indesc
    Pointer to a descriptor object previously used with
    softsec_input_begin().

    @param portid
    Pointer to a variable into which the identifier of the MACsec port
    is stored.

    @return
    Length of the processed frame in bytes or 0.

    @description
    This function continues processing an inbound MAC that was
    pre-processed by softsec_input_begin() and after that potentially
    decrypted using AES-GCM. The return value is the length of the
    resulting frame, or 0 if the frame is rejected by MACsec.
*/
unsigned
softsec_input_end(
  SoftsecInputDescriptor *indesc,
  unsigned *portid);

/** Extract AES-GCM decryption parameters from an inbound frame
    descriptor.

    @param indesc
    Pointer to a descriptor object previously used with
    softsec_input_begin().

    @param crypto_handle
    Pointer to a variable that is set to contain a cryptographic
    resource handle, previously given to softsec_install_key_handle().

    @param authentic
    Pointer to a pointer that is set to point to a variable into which
    ICV verification result is to be stored, with nonzero result
    indicating successful verification.

    @param plaintext
    Pointer to an object that is set to contain the address and length
    of the area into which plaintext is to be stored.

    @param iv
    Pointer to a pointer that is set to point to the initialization
    vector.

    @param ciphertext
    Pointer to an object that is set to contain the address and length
    of the area from which ciphertext to decrypt is read.

    @param aad
    Pointer to an object that is set to contain the address and length
    of the area from which additional authenticated data (AAD) is
    read.

    @param tag
    Pointer to a pointer that is set to point to a buffer from the
    AES-GCM authentication tag to verify is to be read.
*/
void
softsec_get_decryption_parameters(
  SoftsecInputDescriptor *indesc,
  void **crypto_handle,
  unsigned **authentic,
  SoftsecCryptoWriteBytes *plaintext,
  const SoftsecCryptoIv **iv,
  SoftsecCryptoReadBytes *ciphertext,
  SoftsecCryptoReadBytes *aad,
  const SoftsecCryptoTag **tag);

void
softsec_get_latest_an(
  SoftsecIntf *intf,
  unsigned portid,
  unsigned *latest_an);

#ifdef SOFTSEC_SOFTWARE_CRYPTO
/** Perform internal test functions using software cryptography.

    @return
    Nonzero if successful, zero otherwise.
*/
unsigned
softsec_test(void);
#endif /* SOFTSEC_SOFTWARE_CRYPTO */

#endif /* _SOFTSEC_H */
