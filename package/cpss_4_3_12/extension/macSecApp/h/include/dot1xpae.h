/**

   Public 802.1X functions.

   File: dot1xpae.h

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_H
#define _DOT1XPAE_H

#include "dot1xpae_types.h"
#include "eapauth.h"
#include "macseclmi.h"

/** Initialize an interface object.

    @param intf
    Pointer to the object to be initialized.

    @param eap_context
    Pointer to an initialized EAP context object, accessed using the
    functions defined in eapauth.h.

    @param macsec_intf
    Pointer to an initialized MACsec interface object, accessed using
    the functions defined in macseclmi.h.

    @param ifname
    Interface name to show in logs.
*/
void
dot1x_init_intf(
  Dot1xIntf *intf,
  EapContext *eap_context,
  MacsecIntfId macsec_intf,
  const char *ifname);

/** Uninitialize an interface object. Clear any EAP or MACsec
    resources created in the associated EAP context or MACsec
    interface objects. Clear any cryptographic information.

    @param intf
    Pointer to the object to be uninitialized.
*/
void
dot1x_uninit_intf(
  Dot1xIntf *intf);

/** Set the values of the control variables of an interface object and
    then recalculate interface state based on the new values.

    @param intf
    Pointer to the object to be changed.

    @param ctrl
    Pointer to an object from which new values for the control
    variables are copied.
 */
void
dot1x_set_pae_ctrl(
  Dot1xIntf *intf,
  const Dot1xPaeCtrl *ctrl);

/** Get the values of the control variables of an interface object.

    @param intf
    Pointer to the object to be changed.

    @param ctrl
    Pointer to an object into which values of the control variables
    are copied.
 */
void
dot1x_get_pae_ctrl(
  const Dot1xIntf *intf,
  Dot1xPaeCtrl *ctrl);

/** Get the values of the status variables of an interface object.

    @param intf
    Pointer to the object to be changed.

    @param stat
    Pointer to an object into which values of the status variables are
    copied.
 */
void
dot1x_get_pae_stat(
  const Dot1xIntf *intf,
  Dot1xPaeStat *stat);

/** Set the values of the control variables of the default network
    (network with null NID) of an interface.

    @param intf
    Pointer to an interface object.

    @param ctrl
    Pointer to an object from which new values for the control
    variables are copied.
*/
void
dot1x_set_default_network_ctrl(
  Dot1xIntf *intf,
  Dot1xNetworkCtrl *ctrl);

/** Get the values of the control variables of the default network
    (network with null NID) of an interface.

    @param intf
    Pointer to an interface object.

    @param ctrl
    Pointer to an object into which values of the control variables
    are copied.
*/
void
dot1x_get_default_network_ctrl(
  const Dot1xIntf *intf,
  Dot1xNetworkCtrl *ctrl);

/** Get the values of the status variables of the default network
    (network with null NID) of an interface.

    @param intf
    Pointer to an interface object.

    @param stat
    Pointer to an object into which values of the status variables
    are copied.
*/
void
dot1x_get_default_network_stat(
  const Dot1xIntf *intf,
  Dot1xNetworkStat *stat);

/** Create a network object in an interface.

    @param intf
    Pointer to an interface object.

    @param nid
    Pointer to a network identity in UTF-8 format to be copied into
    the new network object. The identity must be unique within an
    interface object.

    @return
    Pointer to the created network object, or NULL if the network
    could not be created, e.g. because of the maximum number of
    networks would be exceeded or duplicate network identity.

    @description
    This function creates a network object for setting per-network
    authentication and protection policies, choosing suitable EAP
    credentials based on network identity and for network announcement
    transmission and reception purposes.

    If transmission of network announcements is enabled by interface
    control variables the network is included in transmitted
    announcements. If reception of network announcements is enabled
    then the network is included in transmitted annoucement requests.

    In addition to this function, a network object may be
    automatically created based on received network announcements. If
    this function is called with a NID for which an automatically
    created network already exists then that network becomes
    'managed', i.e. it will not expire and will behave just like it
    was originally created by this function.
 */
Dot1xNetwork *
dot1x_create_network(
  Dot1xIntf *intf,
  const Dot1xNid *nid);

/** Destroy a network object in an interface.

    @param network
    Pointer to a network object previously returned by
    dot1x_create_network() or dot1x_next_network().

    @description
    This function can be used to destroy any network object, including
    networks created automatically from received network
    announcements.
 */
void
dot1x_destroy_network(
  Dot1xNetwork *network);

/** Iterate to the next network object of an interface.

    @param intf
    Pointer to an interface object.

    @param prev
    Either NULL or pointer to a network returned by a previous call to
    this function.

    @return
    Pointer to the next network object or NULL if there are no objects
    left.

    @description
    This function can be used to retrieve pointers to all networks
    in the interface. In the first call, prev should be NULL; in
    each subsequent call, prev should be the pointer returned by the
    preceding call. When there are no more networks in the
    interface the function returns NULL.

    Other functions that might cause the set of networks to be
    modified must not be called during a chain of calls to this
    function, with the exception that destroying the previous network
    (pointed to by prev) using dot1x_destroy_network() after a call to
    this function is permitted and does not affect retrieval of the
    following networks.

    @notes
    This function iterates all network objects, including those
    created by received announcements.
*/
Dot1xNetwork *
dot1x_next_network(
  const Dot1xIntf *intf,
  const Dot1xNetwork *prev);

/** Get the permanent attributes of a network object.

    @param network
    Pointer to a network object previously returned by
    dot1x_next_network() (or dot1x_create_network()).

    @param nid
    Pointer to a network identity object into which the identity of
    the network is copied in UTF-8 format.
*/
void
dot1x_get_network_attributes(
  const Dot1xNetwork *network,
  Dot1xNid *nid);

/** Set the values of the control variables of a network object.

    @param network
    Pointer to a network object previously returned by
    dot1x_create_network() of dot1x_next_network().

    @param ctrl
    Pointer to an object from which new values for the control
    variables are copied.
*/
void
dot1x_set_network_ctrl(
  Dot1xNetwork *network,
  Dot1xNetworkCtrl *ctrl);

/** Get the values of the control variables of a network object.

    @param network
    Pointer to a network object previously returned by
    dot1x_create_network() of dot1x_next_network().

    @param ctrl
    Pointer to an object into which values for the control variables
    are copied.
*/
void
dot1x_get_network_ctrl(
  const Dot1xNetwork *network,
  Dot1xNetworkCtrl *ctrl);

/** Get the values of the status variables of a network object.

    @param network
    Pointer to a network object previously returned by
    dot1x_create_network() of dot1x_next_network().

    @param stat
    Pointer to an object into which values of the status variables are
    copied.
*/
void
dot1x_get_network_stat(
  const Dot1xNetwork *network,
  Dot1xNetworkStat *stat);

/** Create a pre-shared CA key (CAK) in an interface object.

    @param intf
    Pointer to an interface object.

    @param ckn
    Pointer to a CA key name (CKN) to be copied into the new
    pre-shared CAK object. The CKN must be unique within a an
    interface object.

    @param key
    Pointer to a CA key (CAK) value to be copied into the new
    pre-shared CAK object.

    @param type
    Either DOT1X_CAK_GROUP or DOT1X_CAK_PAIRWISE.

    @param auth_data
    Pointer to an authorization data object to be copied into the new
    pre-shared CAK object.

    @param nid
    Pointer to a network identity (NID) object to be copied into the
    new pre-shared CAK object.

    @param cak_activated_callback
    Optional callback function (NULL if not needed) for receiving
    signal when a CAK has either sent distributed CAK or SAK or has 
    received either one which means that it has been activated. This 
    could be used for example as a trigger when other older CAKs could 
    be deleted.

    @return
    Pointer to the created CAK object, or NULL if the CAK could not be
    created, e.g. because of duplicate CKN or the maximum number of
    CAKs would be exceeded.

    @description

    Normally, an MKA protocol instance using the CAK will be active
    whenever MKA is enabled by control variables, the uncontrolled
    port is operational and there is a network object corresponding
    the the network identity of the CAK (either configured or created
    by received announcements).

    The type (pairwise or group) of the CAK has effect when the number
    of CA members becomes greater than two. In this case, a pairwise
    CAK will be used to distribute a generated group CAK instead of
    SAKs when the MKA instance associated with the CAK is acting as
    the MKA key server.

    If the CAK results in successful authentication and bringing up a
    controlled port then the contents of the authorization data object
    are passed unchanged to macsec_set_authorization().

    If the network identity has non-zero length then the CAK is
    associated with the network with that NID, otherwise it is
    associated with the default network.
 */
Dot1xCak *
dot1x_create_cak(
  Dot1xIntf *intf,
  const Dot1xCkn *ckn,
  const MacsecKey *key,
  Dot1xCakType type,
  const MacsecAuthData *auth_data,
  const Dot1xNid *nid,
  void (*cak_activated_callback)(Dot1xCak*));

/** Destroy a CAK.

    @param cak
    Pointer to a CAK object previously returned by dot1x_create_cak()
    (or dot1x_next_cak()).

    @description
    This function destroys a pre-shared CAK previously created using
    dot1x_create_cak(), or any CAK returned by dot1x_next_cak(),
    including EAP-based and distributed CAKs. Any MKA instance
    associated with the CAK is immediately terminated.
 */
void
dot1x_destroy_cak(
  Dot1xCak *cak);

/** Iterate to the next CAK object of an interface.

    @param intf
    Pointer to an interface object.

    @param prev
    Either NULL or pointer to a CAK returned by a previous call to
    this function.

    @return
    Pointer to the next CAK object or NULL if there are no objects
    left.

    @description
    This function can be used to retrieve pointers to all CAKs in the
    interface. In the first call, prev should be NULL; in each
    subsequent call, prev should be the pointer returned by the
    preceding call. When there are no more CAKs in the interface the
    function returns NULL.

    Other functions that might cause the set of CAKs to be modified
    must not be called during a chain of calls to this function, with
    the exception that destroying the previous CAK (pointed to by
    prev) using dot1x_destroy_cak() after a call to this function is
    permitted and does not affect retrieval of the following CAKs.

    @notes
    This function iterates all CAK objects, not just pre-shared CAKs.
*/
Dot1xCak *
dot1x_next_cak(
  Dot1xIntf *intf,
  const Dot1xCak *prev);

/** Get the permanent attributes of a CAK object.

    @param cak
    Pointer to a CAK object previously returned by dot1x_next_cak()
    (or dot1x_create_cak()).

    @param ckn
    Pointer to an object into which the CKN of the CAK is copied.

    @param type
    Pointer to a variable into which the type (pairwise of group) of
    the CAK is copied.

    @param auth_data
    Pointer to an object into which the authorization data associated
    with the CAK is copied.

    @param nid
    Pointer to an object into which the network identity (NID) of the
    CAK is copied.

    @notes
    The actual CA key (CAK) value can not be retrieved for security
    reasons.
*/
void
dot1x_get_cak_attributes(
  const Dot1xCak *cak,
  Dot1xCkn *ckn,
  Dot1xCakType *type,
  MacsecAuthData *auth_data,
  Dot1xNid *nid);

/** Get the values of the status variables of a CAK.

    @param cak
    Pointer to a CAK object previously returned by dot1x_create_cak()
    of dot1x_next_cak().

    @param stat
    Pointer to an object into which values of the status variables are
    copied.
*/
void
dot1x_get_cak_stat(
  const Dot1xCak *cak,
  Dot1xCakStat *stat);

/** Iterate to the next virtual port object of an interface.

    @param intf
    Pointer to an interface object.

    @param prev
    Either NULL or pointer to a port returned by a previous call to
    this function.

    @return
    Pointer to the next port object or NULL if there are no objects
    left.

    @description

    This function can be used to retrieve pointers to all virtual ports
    in the interface. In the first call, prev should be NULL; in
    each subsequent call, prev should be the pointer returned by the
    preceding call. When there are no more virtual ports in the
    interface the function returns NULL.

    Other functions that might cause the set of virtual ports to be
    modified must not be called during a chain of calls to this
    function.
*/
Dot1xPort *
dot1x_next_virtual_port(
  Dot1xIntf *intf,
  const Dot1xPort *prev);

/** Get the permanent attributes of a virtual port.

    @param port
    Pointer to a port object previously returned by
    dot1x_next_virtual_port().

    @param portid
    Pointer to a variable into which the port identifier of the
    virtual port is copied.

    @param peer_address
    Pointer to an object into which the MAC address of the MACsec peer
    associated with the virtual port is copied.
*/
void
dot1x_get_virtual_port_attributes(
  const Dot1xPort *port,
  unsigned *portid,
  MacsecAddress *peer_address);

/** Get the values of the status variables of the real port of an
    interface.

    @param intf
    Pointer to an interface object.

    @param stat
    Pointer to an object into which values of the status variables are
    copied.
*/
void
dot1x_get_real_port_stat(
  const Dot1xIntf *intf,
  Dot1xPortStat *stat);

/** Get the values of the status variables of a virtual port.

    @param port
    Pointer to a port object previously returned by
    dot1x_next_virtual_port().

    @param stat
    Pointer to an object into which values of the status variables are
    copied.
*/
void
dot1x_get_virtual_port_stat(
  const Dot1xPort *port,
  Dot1xPortStat *stat);

/** Run the 802.1X protocol of an interface after external events have
    occurred.

    @param intf
    Pointer to an interface object.

    @param events
    Pointer to a set of bits indicating which events should be
    processed.

    @description
    This function retrieves any data associated with the events using
    the MACsec interface and EAP context objects associated with the
    interface and the functions in macseclmi.h and eapauth.h. During
    processing the events and data it calls other functions in these
    APIs.

    This function may be called at any time, regardless whether or not
    there are new events. However the time_tick event is special: this
    function must be called with the time_tick event bit set twice per
    second, with approximately 0.5 second interval.
 */
void
dot1x_handle_events(
  Dot1xIntf *intf,
  const Dot1xEvents *events);

/** Enable a diagnostic message category.

    @param intf
    Pointer to an interface object.

    @param msg
    A message category.

    @notes
    Diagnostic messages are output using dot1x_platform_log() and the
    MACsec interface object associated with the interface.
 */
void
dot1x_enable_message(
  Dot1xIntf *intf,
  Dot1xMessage msg);

/** Disable a diagnostic message category.

    @param intf
    Pointer to an interface object.

    @param msg
    A message category.
 */
void
dot1x_disable_message(
  Dot1xIntf *intf,
  Dot1xMessage msg);

/** Perform internal test functions.

    @return
    1 if successful, 0 otherwise.

    @description

    This function verifies CAK, CKN, KEK, ICK and SAK derivation using
    the test vectors specified in 802.1X-2010.
*/
int
dot1x_test(void);


/** Set CAK cache callback function for interface.

    @param intf
    Pointer to an interface object.

    @param cak_cache_callback
    Callback function.
*/
void
dot1x_set_cak_cache_callback(
        Dot1xIntf *intf,
        void (*cak_cache_callback)(Dot1xCakCacheOp op, const Dot1xCak *cak));


/** Add CAK to cache when resuming suspension.
 */
Dot1xCak *
dot1x_insert_cak_cache(
        Dot1xIntf *intf,
        const Dot1xCak *cache_cak);

#endif /* _DOT1XPAE_H */
