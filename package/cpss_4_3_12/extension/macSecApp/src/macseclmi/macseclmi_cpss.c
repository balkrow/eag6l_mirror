#include <macseclmi_types.h>
#include <cpss/dxCh/dxChxGen/macSec/cpssDxChMacSec.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <cpssCommon/cpssPresteraDefs.h>

#define MACSEC_MKA_MAX_INTERFACE_TEST        10
#define MACSEC_MKA_MAX_PEERS_TEST            1

/*
 * Transmit SA.
 */
typedef struct {
	/* the `enableTransmit' control */
	unsigned                enable : 1;

	/* next packet number */
	uint64_t                next_pn;

	/* pointer to data key */
	MacsecKi                ki;

	/* SSCI for XPN suites in network byte order */
	uint32_t                ssci;

    MacsecTransmitSaStat    stat;
} lmiTransmitSa;

/*
 * Transmit SC.
 */
typedef struct {
	/* current SA number */
	unsigned                an;

	/* SAs */
	lmiTransmitSa           sas[4];

    MacsecTransmitScStat    stat;
} lmiTransmitSc;

/*
 * Receive SA.
 */
typedef struct {
    /* the `enableReceive' control */
    unsigned                enable : 1;

    /* lowest acceptable packet number */
    uint64_t                lowest_pn;

    /* next packet number */
    uint64_t                next_pn;

    /* pointer to data key */
    MacsecKi                ki;

    /* SSCI for XPN suites in network byte order */
    uint32_t                ssci;

    MacsecReceiveSaStat     stat;
} lmiReceiveSa;

/*
 * Receive SC.
 */
typedef struct {
    /* current SA number */
    unsigned                an;

    /* previous SA number */
    unsigned                oan;

    /* SAs */
    lmiReceiveSa            sas[4];

    MacsecReceiveScStat     stat;
} lmiReceiveSc;

/*
 * MACsec peer object.
 */
typedef struct MacsecPeer_s {

  /* associated interface */
  struct lmiMacsecIntf_cpss     *intf;

  /* port identifier; greater than 1 indicates a virtual port */
  unsigned                      portid;

  /* MAC address of peer */
  MacsecAddress                 peer_address;

  /* port identifier of peer */
  unsigned                      peer_portid;

  /* 1 if object completely initialized */
  int                           valid;

  /* controlled port port object of a virtual port; unused if portid <= 1 */
  MlPort                        port;

  /* peer id */
  MacsecPeerId                  peer_id;
} lmiMacsecPeer;

typedef struct {
    /* sysuptime in SNMP timeticks (hundredths of a second) for timestamps */
    GT_U32                              sysuptime;

	/* interface index */
	int                                 portId;

	/* EAPOL multicast address */
	MacsecAddress                       multicast_address;

	MacsecKey                           sak;
	MacsecKi                            ki;
	MacsecSalt                          salt;

	lmiTransmitSc                       transmit_sc;
    lmiReceiveSc                        receive_sc;

	lmiMacsecPeer                       peers[MACSEC_MKA_MAX_PEERS_TEST];
	lmiMacsecPeer                       vport_peers[MACSEC_MKA_MAX_PEERS_TEST];

    MacsecAuthData                      auth_data;

    /* Statistics data */
    MacsecSecyStat                      secy_stat;
    MacsecPortStat                      port_stat;
    MacsecPortStat                      ctrl_port_stat;
    MacsecPortStat                      un_ctrl_port_stat;

	/**** Output ****/
	CPSS_DXCH_MACSEC_SECY_SA_HANDLE     tran_saHandle;
	CPSS_DXCH_MACSEC_SECY_SA_HANDLE     recv_saHandle;
}lmiMacsecIntf_cpss;


/* DDK Related global configuration */
typedef struct {

    /**** Input ****/
    /* SA Add */
    CPSS_DXCH_MACSEC_SECY_SA_STC        saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC trRecParams;

}ddkGlobalConfig;


/* MKA Related global configuration */
typedef struct {

    MacsecCapabilities capabilities;

}mkaGlobalConfig;

lmiMacsecIntf_cpss     mkaDb[MACSEC_MKA_MAX_INTERFACE_TEST];       /* MKA DB    - MKA related config */
ddkGlobalConfig     ddkConfigDb;                                /* Global DB - DDK configuration */
mkaGlobalConfig     mkaConfigDb;

void macsec_mka_init()
{

    /* MKA related global config */
    mkaConfigDb.capabilities.max_peer_scs                   = 1;
    mkaConfigDb.capabilities.max_receive_keys               = 1;
    mkaConfigDb.capabilities.max_transmit_keys              = 1;
    mkaConfigDb.capabilities.cipher_suites[0].implemented   = 1;
    mkaConfigDb.capabilities.cipher_suites[0].capability    = MACSEC_CAPABILITY_INTEGRITY;
    return;
}

void macsec_ddk_init()
{
    /* CPSS_DXCH_MACSEC_SECY_SA_STC */
    ddkConfigDb.saParams.params.ingress.saInUse                 = GT_TRUE;
    ddkConfigDb.saParams.params.ingress.confidentialityOffset   = 0;

    /* CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC */
    ddkConfigDb.trRecParams.an              = 0;
    memset(&ddkConfigDb.trRecParams.keyArr, 0, sizeof(ddkConfigDb.trRecParams.keyArr));
    ddkConfigDb.trRecParams.keyByteCount    = 16;
    memset(&ddkConfigDb.trRecParams.sciArr, 0, sizeof(ddkConfigDb.trRecParams.sciArr));
    return;
}

static lmiMacsecIntf_cpss *
get_port(MacsecIntfId   intf_id,
        unsigned        portid)
{
    if(portid > MACSEC_MKA_MAX_INTERFACE_TEST)
    {
        return NULL;
    }
    return &mkaDb[portid];
}

/* Get macsec interface from peer_id */
static void
macsec_get_interface(MacsecPeerId peer_id,
        lmiMacsecIntf_cpss        *portPtr)
{
    GT_U32                  portId;
    GT_U32                  peerId;

    for(portId=0; portId < MACSEC_MKA_MAX_INTERFACE_TEST; portId++)
    {
        portPtr = get_port(0, portId);
        for(peerId=0; peerId < MACSEC_MKA_MAX_PEERS_TEST; peerId++)
        {
            if(portPtr->peers[peerId].peer_id == peer_id)
            {
                return;
            }
        }
    }
    portPtr = NULL;
    return;
}

void
macsec_install_key(
        MacsecIntfId     intf_id,
        unsigned         portid,
        const MacsecKey  *sak,
        const MacsecKi   *ki,
        const MacsecSalt *salt)
{
    lmiMacsecIntf_cpss       *portPtr;

    assert(sak->len == 16 || sak->len == 32);

    portPtr = get_port(intf_id, portid);

    portPtr->ki = *ki;
    if (salt)
    {
        memcpy(&portPtr->salt, salt->buf, sizeof(portPtr->salt));
    }
    if (sak)
    {
        memcpy(&portPtr->sak, sak->buf, sak->len);
    }
    return;
}


void
macsec_create_transmit_sa(
        MacsecIntfId     intf_id,
        unsigned         portid,
        unsigned         an,
        uint64_t         next_pn,
        MacsecKi         *ki,
        const MacsecSsci *ssci)
{
    lmiMacsecIntf_cpss              *portPtr;
    lmiTransmitSc                   *tsc;
    lmiTransmitSa                   *tsa;
    GT_U8                           devNum, devIndex;
    GT_MACSEC_UNIT_BMP              unitBmp;
    CPSS_DXCH_MACSEC_DIRECTION_ENT  direction;
    GT_U32                          vPortId;

    portPtr = get_port(intf_id, portid);
    tsc = &portPtr->transmit_sc;

    assert(an <= 3);

    tsa = &tsc->sas[an];

    tsa->enable = 0;
    tsa->next_pn = next_pn;
    tsa->ki = *ki;
    if (ssci)
    {
        assert(sizeof tsa->ssci == sizeof ssci->buf);
        memcpy(&tsa->ssci, ssci->buf, sizeof ssci->buf);
    }

    memset(&tsa->stat, 0, sizeof tsa->stat);
    tsa->stat.created_time = portPtr->sysuptime;
    tsa->stat.started_time = portPtr->sysuptime;

    devIndex = SYSTEM_DEV_NUM_MAC(0);
    devNum   = appDemoPpConfigList[devIndex].devNum;

    /* smemMultiDpUnitIndexGet(devObjPtr, portNum) TODO - Is there a way to convert without devObjPtr ?? */
    unitBmp     = BIT_0;
    direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
    vPortId     = portid;
    cpssDxChMacSecSecySaAdd(devNum,
            unitBmp,
            direction,
            vPortId,
            &ddkConfigDb.saParams,
            &ddkConfigDb.trRecParams,
            &portPtr->tran_saHandle);
    return;
}


void
macsec_enable_transmit(
        MacsecIntfId    intf_id,
        unsigned        portid,
        unsigned        an)
{
    lmiMacsecIntf_cpss              *portPtr;
    lmiTransmitSc                   *tsc;
    lmiTransmitSa                   *tsa, *oldtsa = NULL;

    portPtr                 = get_port(intf_id, portid);
    tsc                     = &portPtr->transmit_sc;

    tsa                     = &tsc->sas[an];
    tsa->enable             = 1;
    tsa->stat.in_use        = 1;
    tsa->stat.started_time  = portPtr->sysuptime;
    tsc->an                 = an;

    if (tsc->an != an)
    {
        oldtsa = &tsc->sas[tsc->an];
        if (oldtsa && oldtsa->enable)
        {
            oldtsa->enable              = 0;
            oldtsa->stat.in_use         = 0;
            oldtsa->stat.stopped_time   = portPtr->sysuptime;
        }
    }

    if (!tsc->stat.transmitting)
    {
        tsc->stat.transmitting      = 1;
        tsc->stat.started_time      = portPtr->sysuptime;
        /* TODO - cpss mapping
         * update_controlled_mac(intf, portid);
         */
    }
    tsc->stat.encoding_sa       = tsc->an;
    tsc->stat.enciphering_sa    = tsc->an;
}

MacsecPeerId
macsec_create_peer(
  MacsecIntfId          intf_id,
  unsigned              portid,
  const MacsecAddress   *peer_address,
  unsigned              peer_portid)
{
    lmiMacsecIntf_cpss            *portPtr;
    lmiMacsecPeer                 *peerPtr;

    portPtr = get_port(intf_id, portid);

    peerPtr                = &portPtr->peers[0];
    peerPtr->portid        = portid;
    peerPtr->peer_portid   = peer_portid;
    peerPtr->peer_address  = *peer_address;

    /* assign the interface associated */
    peerPtr->intf          = (struct lmiMacsecIntf_cpss *)portPtr;

    /* Unique number to be identified in later APIs */
    peerPtr->peer_id       = (portid << 2);

    /* TODO - Same need to be copied to vport_peers too ?? */
    portPtr->vport_peers[0]= *peerPtr;

    /* stat update */
    /* LINUX_SPECIFIC_DATA
     * portPtr->transmit_sc.stat.sci.address    = portPtr->address;
     */
    portPtr->transmit_sc.stat.sci.portid     = portid;
    portPtr->transmit_sc.stat.created_time   = portPtr->sysuptime;
    portPtr->transmit_sc.stat.started_time   = portPtr->sysuptime;
    portPtr->transmit_sc.stat.stopped_time   = portPtr->sysuptime;

    portPtr->receive_sc.stat.sci.address    = *peer_address;
    portPtr->receive_sc.stat.sci.portid     = peer_portid;
    portPtr->receive_sc.stat.created_time   = portPtr->sysuptime;
    portPtr->receive_sc.stat.started_time   = portPtr->sysuptime;
    portPtr->receive_sc.stat.stopped_time   = portPtr->sysuptime;

    /* Current implimentation single peer */
    return 0;
}

void
macsec_destroy_peer(
  MacsecPeerId peer_id)
{
    lmiMacsecIntf_cpss *portPtr = NULL;

    macsec_get_interface(peer_id, portPtr);

    if(portPtr)
    {
        if(portPtr->peers[0].peer_id == peer_id)
        {
            memset(&portPtr->peers[0], 0, sizeof(portPtr->peers[0]));
            memset(&portPtr->vport_peers[0], 0, sizeof(portPtr->vport_peers[0]));
        }
        return;
    }
}

MacsecPeerId
macsec_next_peer(
        MacsecIntfId intf_id,
        MacsecPeerId prev_id)
{
    /* Current implimentation for - 1 peer ID per interface */
    lmiMacsecIntf_cpss  *portPtr;
    GT_U32              peerIndex;

    macsec_get_interface(prev_id, portPtr);

    for(peerIndex=0; peerIndex < MACSEC_MKA_MAX_PEERS_TEST; peerIndex++)
    {
        if(portPtr->peers[peerIndex].peer_id == prev_id)
        {
            /* return portPtr->peers[peerId + 1].peer_id; */
            return portPtr->peers[0].peer_id;
        }
    }
    return 0;
}



void
macsec_get_capabilities(
        MacsecCapabilities *capabilities)
{
    GT_U32      i;

    for (i = 0; i < MACSEC_CIPHER_SUITE_COUNT; i++)
    {
        capabilities->cipher_suites[i].implemented =
            mkaConfigDb.capabilities.cipher_suites[i].implemented;
        capabilities->cipher_suites[i].capability =
            mkaConfigDb.capabilities.cipher_suites[i].capability;
    }
    capabilities->max_peer_scs =
        mkaConfigDb.capabilities.max_peer_scs;
    capabilities->max_receive_keys =
        mkaConfigDb.capabilities.max_receive_keys;
    capabilities->max_transmit_keys =
        mkaConfigDb.capabilities.max_transmit_keys;
}

void
macsec_get_intf_attributes(
  MacsecIntfId      intf_id,
  unsigned          *ifindex,
  MacsecAddress     *address)
{
    /* TODO - LINUX_SPECIFIC_API */
    return;
}

void
macsec_get_peer_attributes(
        MacsecPeerId            peer_id,
        unsigned                *portid,
        MacsecAddress           *peer_address,
        unsigned                *peer_portid)
{
    lmiMacsecIntf_cpss  *portPtr = NULL;

    macsec_get_interface(peer_id, portPtr);;

    if(peer_id == portPtr->peers[0].peer_id)
    {
        *portid        = portPtr->peers[0].portid;
        *peer_portid   = portPtr->peers[0].peer_portid;
        *peer_address  = portPtr->peers[0].peer_address;
    }
}

void
macsec_get_receive_sc_stat(
        MacsecPeerId        peer_id,
        MacsecReceiveScStat *stat)
{
    lmiMacsecIntf_cpss  *portPtr = NULL;

    macsec_get_interface(peer_id, portPtr);;

    *stat = portPtr->receive_sc.stat;
    return;
}

void
macsec_set_authorization(
        MacsecIntfId            intf_id,
        unsigned                portid,
        const MacsecAuthData    *auth_data)
{
    lmiMacsecIntf_cpss *port = get_port(intf_id, portid);

    port->auth_data.len = auth_data->len;
    memcpy(&port->auth_data.buf, &auth_data->buf, auth_data->len);
    return;
}

void
macsec_set_controlled_port_enabled(
        MacsecIntfId            intf_id,
        unsigned                portid,
        int                     value)
{
}

void
macsec_set_secy_ctrl(
        MacsecIntfId            intf_id,
        unsigned                portid,
        const MacsecSecyCtrl    *ctrl)
{
}

void
macsec_get_secy_ctrl(
        MacsecIntfId    intf_id,
        unsigned        portid,
        MacsecSecyCtrl  *ctrl)
{
}

void
macsec_get_secy_stat(
        MacsecIntfId    intf_id,
        unsigned        portid,
        MacsecSecyStat  *stat)
{
    lmiMacsecIntf_cpss *portPtr = get_port(intf_id, portid);
    *stat = portPtr->secy_stat;
    return;
}

void
macsec_get_transmit_sc_stat(
        MacsecIntfId            intf_id,
        unsigned                portid,
        MacsecTransmitScStat    *stat)
{
    lmiMacsecIntf_cpss *portPtr = get_port(intf_id, portid);;
    *stat = portPtr->transmit_sc.stat;
    return;
}



void
macsec_get_transmit_sa_next_pn(
  MacsecIntfId      intf_id,
  unsigned          portid,
  unsigned          an,
  uint64_t          *next_pn)
{
    lmiMacsecIntf_cpss *portPtr = get_port(intf_id, portid);
    *next_pn = portPtr->transmit_sc.sas[an].next_pn;
    return;
}

void
macsec_get_transmit_sa_stat(
        MacsecIntfId            intf_id,
        unsigned                portid,
        unsigned                an,
        MacsecTransmitSaStat    *stat)
{
    lmiMacsecIntf_cpss *portPtr = get_port(intf_id, portid);;
    *stat = portPtr->transmit_sc.sas[an].stat;
    return;
}

void
macsec_get_latest_an(
        MacsecIntfId    intf_id,
        unsigned        portid,
        unsigned        *latest_an)
{
    lmiMacsecIntf_cpss *portPtr = get_port(intf_id, portid);
    *latest_an = portPtr->transmit_sc.an;
    return;
}

void
macsec_create_receive_sa(
        MacsecPeerId     peer_id,
        unsigned         an,
        uint64_t         lowest_pn,
        MacsecKi         *ki,
        const MacsecSsci *ssci)
{
    lmiMacsecIntf_cpss              *portPtr = NULL;
    lmiReceiveSc                    *rsc;
    lmiReceiveSa                    *rsa;
    GT_U8                           devNum, devIndex;
    GT_MACSEC_UNIT_BMP              unitBmp;
    CPSS_DXCH_MACSEC_DIRECTION_ENT  direction;
    GT_U32                          vPortId;

    macsec_get_interface(peer_id, portPtr);
    rsc         = &portPtr->receive_sc;

    assert(an <= 3);
    rsa = &rsc->sas[an];

    rsa->enable     = 0;
    rsa->lowest_pn  = lowest_pn;
    rsa->next_pn    = lowest_pn;
    rsa->ki         = *ki;
    if (ssci)
    {
        assert(sizeof(rsa->ssci) == sizeof(ssci->buf));
        memcpy(&rsa->ssci, ssci->buf, sizeof ssci->buf);
    }

    /*
    memset(&tsa->stat, 0, sizeof tsa->stat);
    tsa->stat.created_time = intf->sysuptime;
    tsa->stat.started_time = intf->sysuptime;
    */

    devIndex = SYSTEM_DEV_NUM_MAC(0);
    devNum   = appDemoPpConfigList[devIndex].devNum;

    /* smemMultiDpUnitIndexGet(devObjPtr, portNum) TODO - Is there a way to convert without devObjPtr ?? */
    unitBmp     = BIT_0;
    direction   = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
    vPortId     = portPtr->portId;
    cpssDxChMacSecSecySaAdd(devNum,
            unitBmp,
            direction,
            vPortId,
            &ddkConfigDb.saParams,
            &ddkConfigDb.trRecParams,
            &portPtr->recv_saHandle);
    return;
}

void
macsec_enable_receive(
        MacsecPeerId    peer_id,
        unsigned        an)
{
    lmiMacsecIntf_cpss              *portPtr = NULL;
    lmiReceiveSc                    *rsc;
    lmiReceiveSa                    *rsa, *oldrsa = NULL;

    macsec_get_interface(peer_id, portPtr);
    rsc         = &portPtr->receive_sc;

    rsa         = &rsc->sas[an];
    rsa->enable = 1;
    rsc->an     = an;

    if (rsc->an != an)
    {
        oldrsa = &rsc->sas[rsc->an];
        if (oldrsa && oldrsa->enable)
        {
            oldrsa->enable = 0;
        }
    }
}

void
macsec_set_receive_sa_lowest_pn(
        MacsecPeerId    peer_id,
        unsigned        an,
        uint64_t        lowest_pn)
{
    lmiMacsecIntf_cpss              *portPtr = NULL;
    lmiReceiveSa                    *rsa;

    macsec_get_interface(peer_id, portPtr);

    rsa = &portPtr->receive_sc.sas[an];
    rsa->lowest_pn = lowest_pn;
    if (rsa->next_pn < lowest_pn)
    {
        rsa->next_pn = lowest_pn;
    }
}

void
macsec_get_receive_sa_stat(
        MacsecPeerId            peer_id,
        unsigned                an,
        MacsecReceiveSaStat     *stat)
{
    lmiMacsecIntf_cpss *portPtr = NULL;

    macsec_get_interface(peer_id, portPtr);

    *stat = portPtr->receive_sc.sas[an].stat;
    return;
}

void
macsec_set_multicast_address(
        MacsecIntfId intf_id,
        const MacsecAddress *address)
{
    lmiMacsecIntf_cpss *portPtr = get_port(0, intf_id);

    portPtr->multicast_address = *address;
    return;
}

void
macsec_get_multicast_address(
        MacsecIntfId        intf_id,
        MacsecAddress       *address)
{
    lmiMacsecIntf_cpss *portPtr = get_port(0, intf_id);

    *address = portPtr->multicast_address;
    return;
}

void
macsec_get_common_port_enabled(
        MacsecIntfId    intf_id,
        int             *value)
{
    lmiMacsecIntf_cpss *portPtr = get_port(0, intf_id);
    *value = (portPtr)? 1 : 0;
    return;
}

int
macsec_send_uncontrolled(
        MacsecIntfId            intf_id,
        const MacsecAddress     *dst,
        const MacsecAddress     *src,
        const unsigned char     *msdubuf,
        unsigned                msdulen)
{
    return 0;
}

unsigned
macsec_receive_uncontrolled(
        MacsecIntfId            intf_id,
        MacsecAddress           *dst,
        MacsecAddress           *src,
        unsigned char           *msdubuf,
        unsigned                msdulen)
{
    return 0;
}

void
macsec_get_common_stat(
        MacsecIntfId        intf_id,
        MacsecPortStat      *stat)
{
    lmiMacsecIntf_cpss *portPtr = get_port(0, intf_id);
    *stat = portPtr->port_stat;
    return;
}

void
macsec_get_uncontrolled_stat(
        MacsecIntfId        intf_id,
        MacsecPortStat      *stat)
{
    lmiMacsecIntf_cpss *portPtr = get_port(0, intf_id);
    *stat = portPtr->un_ctrl_port_stat;
    return;
}

void
macsec_get_controlled_stat(
        MacsecIntfId        intf_id,
        unsigned            portid,
        MacsecPortStat      *stat)
{
    lmiMacsecIntf_cpss *portPtr = get_port(intf_id, portid);
    *stat = portPtr->ctrl_port_stat;
    return;
}

void
macsec_uninit_intf(
        MacsecIntfId intf_id)
{
    /* TODO - LINUX_SPECIFIC_API */
    return;
}

MacsecIntfId
macsec_init_intf(
        const MacsecConfig *config)
{
    /* TODO - LINUX_SPECIFIC_API */
    return 0;
}

void
macsec_set_intf_flags(
        MacsecIntfId    intf_id,
        unsigned        flags)
{
    /* TODO - LINUX_SPECIFIC_API */
    return;
}

MacsecIntfId
macsec_next_intf(
        MacsecIntfId prev)
{
    /* TODO - LINUX_SPECIFIC_API */
    return 0;
}
