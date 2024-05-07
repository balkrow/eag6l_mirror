/**

   @copyright
   Copyright (c) 2012 - 2021, Rambus Inc. All rights reserved.

   File: macseclmi.c

   @description
   MACsec LMI and data plane based on UNIX TUN/TAP devices.

*/

#include "macseclmi.h"
#include "macseclmi_sysdep.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/un.h>

#include "lmi_message.h"

typedef struct LmiConnectionS
{
    int fd;
} LmiConnection;

LmiConnection dp_connection;

static int
receive_fd(int s);

static void
macsec_message_receive_dp_data(
        MacsecMessage *message,
        void *ret_data,
        size_t ret_data_size)
{
    LmiConnection *dp_conn = &dp_connection;

    if (! macsec_message_error(message))
    {
        macsec_message_clear(message);

        if (ret_data)
            macsec_message_add(message, ret_data, ret_data_size);

        macsec_message_receive(dp_conn->fd, message);
    }
}

static void
macsec_get_dp_data(
        MacsecDataplaneReq req,
        MacsecIntfId intf_id,
        MacsecPeerId peer_id,
        unsigned portid,
        void *ret_data,
        size_t ret_data_size)
{
  LmiConnection *dp_conn = &dp_connection;
  MacsecMessage message;

  macsec_message_init(&message, req, intf_id, peer_id);

  if (portid != 0)
    MACSEC_MESSAGE_ADD(&message, portid);

  macsec_message_send(dp_conn->fd, &message);

  macsec_message_receive_dp_data(&message, ret_data, ret_data_size);
}

static void
macsec_set_dp_data(
        MacsecDataplaneReq req,
        MacsecIntfId intf_id,
        MacsecPeerId peer_id,
        unsigned portid,
        const void *data,
        size_t data_size)
{
  LmiConnection *dp_conn = &dp_connection;
  MacsecMessage message;

  macsec_message_init(&message, req, intf_id, peer_id);

  if (portid != 0)
    MACSEC_MESSAGE_ADD(&message, portid);

  macsec_message_add(&message, data, data_size);

  macsec_message_send(dp_conn->fd, &message);

  macsec_message_receive_dp_data(&message, NULL, 0);
}

void
macsec_get_capabilities(
        MacsecCapabilities *capabilities)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_CAPABILITIES,
            0, 0, 0,
            capabilities, sizeof *capabilities);
}

void
macsec_get_intf_attributes(
  MacsecIntfId intf_id,
  unsigned *ifindex,
  MacsecAddress *address)
{
  LmiConnection *dp_conn = &dp_connection;
  MacsecMessage message;

  macsec_message_init(&message, MACSEC_DP_GET_INTF_ATTRIBUTES, intf_id, 0);
  macsec_message_send(dp_conn->fd, &message);

  if (!macsec_message_error(&message))
    {
      macsec_message_clear(&message);
      MACSEC_MESSAGE_ADD(&message, *ifindex);
      MACSEC_MESSAGE_ADD(&message, *address);

      macsec_message_receive(dp_conn->fd, &message);
    }
}

MacsecPeerId
macsec_create_peer(
  MacsecIntfId intf_id,
  unsigned portid,
  const MacsecAddress *peer_address,
  unsigned peer_portid)
{
  MacsecPeerId peer_id = 0;
  LmiConnection *dp_conn = &dp_connection;
  MacsecMessage message;

  macsec_message_init(&message, MACSEC_DP_CREATE_PEER, intf_id, 0);
  MACSEC_MESSAGE_ADD(&message, portid);
  MACSEC_MESSAGE_ADD(&message, *peer_address);
  MACSEC_MESSAGE_ADD(&message, peer_portid);

  macsec_message_send(dp_conn->fd, &message);

  macsec_message_receive_dp_data(&message, &peer_id, sizeof peer_id);

  return peer_id;
}

void
macsec_destroy_peer(
  MacsecPeerId peer_id)
{
  LmiConnection *dp_conn = &dp_connection;
  MacsecMessage message;

  macsec_message_init(&message, MACSEC_DP_DESTROY_PEER, 0, peer_id);

  macsec_message_send(dp_conn->fd, &message);

  macsec_message_receive_dp_data(&message, NULL, 0);
}

MacsecPeerId
macsec_next_peer(
        MacsecIntfId intf_id,
        MacsecPeerId prev_id)
{
    MacsecPeerId peer_id = 0;
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;

    macsec_message_init(&message, MACSEC_DP_NEXT_PEER, intf_id, prev_id);

    macsec_message_send(dp_conn->fd, &message);

    macsec_message_receive_dp_data(&message, &peer_id, sizeof peer_id);

    return peer_id;
}

void
macsec_get_peer_attributes(
        MacsecPeerId peer_id,
        unsigned *portid,
        MacsecAddress *peer_address,
        unsigned *peer_portid)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;

    macsec_message_init(&message, MACSEC_DP_GET_PEER_ATTRIBUTES, 0, peer_id);

    macsec_message_send(dp_conn->fd, &message);

    if (!macsec_message_error(&message))
    {
        macsec_message_clear(&message);

        MACSEC_MESSAGE_ADD(&message, *portid);
        MACSEC_MESSAGE_ADD(&message, *peer_address);
        MACSEC_MESSAGE_ADD(&message, *peer_portid);

        macsec_message_receive(dp_conn->fd, &message);
    }
}

void
macsec_get_receive_sc_stat(
        MacsecPeerId peer_id,
        MacsecReceiveScStat *stat)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_RECEIVE_SC_STAT,
            0, peer_id, 0,
            stat, sizeof *stat);
}

void
macsec_set_authorization(
        MacsecIntfId intf_id,
        unsigned portid,
        const MacsecAuthData *auth_data)
{
    macsec_set_dp_data(
            MACSEC_DP_SET_AUTHORIZATION,
            intf_id, 0, portid,
            auth_data, sizeof *auth_data);
}

void
macsec_set_controlled_port_enabled(
        MacsecIntfId intf_id,
        unsigned portid,
        int value)
{
    macsec_set_dp_data(
            MACSEC_DP_SET_CONTROLLED_PORT_ENABLED,
            intf_id, 0, portid,
            &value, sizeof value);
}

void
macsec_set_secy_ctrl(
        MacsecIntfId intf_id,
        unsigned portid,
        const MacsecSecyCtrl *ctrl)
{
    macsec_set_dp_data(
            MACSEC_DP_SET_SECYCTRL,
            intf_id, 0, portid,
            ctrl, sizeof *ctrl);
}

void
macsec_get_secy_ctrl(
        MacsecIntfId intf_id,
        unsigned portid,
        MacsecSecyCtrl *ctrl)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_SECYCTRL,
            intf_id, 0, portid,
            ctrl, sizeof *ctrl);
}

void
macsec_get_secy_stat(
        MacsecIntfId intf_id,
        unsigned portid,
        MacsecSecyStat *stat)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_SECYSTAT,
            intf_id, 0, portid,
            stat, sizeof *stat);
}

void
macsec_get_transmit_sc_stat(
        MacsecIntfId intf_id,
        unsigned portid,
        MacsecTransmitScStat *stat)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_TRANSMIT_SC_STAT,
            intf_id, 0, portid,
            stat, sizeof *stat);
}

void
macsec_install_key(
        MacsecIntfId intf_id,
        unsigned portid,
        const MacsecKey *sak,
        const MacsecKi *ki,
        const MacsecSalt *salt)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;
    unsigned has_key = 0;
    MacsecSalt salt_data = {{0}};

    if (salt)
    {
        has_key = 1;
        salt_data = *salt;
    }

    macsec_message_init(&message, MACSEC_DP_INSTALL_KEY, intf_id, 0);
    MACSEC_MESSAGE_ADD(&message, portid);
    MACSEC_MESSAGE_ADD(&message, *sak);
    MACSEC_MESSAGE_ADD(&message, *ki);
    MACSEC_MESSAGE_ADD(&message, has_key);
    MACSEC_MESSAGE_ADD(&message, salt_data);

    macsec_message_send(dp_conn->fd, &message);

    macsec_message_receive_dp_data(&message, NULL, 0);
}

void
macsec_create_transmit_sa(
        MacsecIntfId intf_id,
        unsigned portid,
        unsigned an,
        uint64_t next_pn,
        MacsecKi *ki,
        const MacsecSsci *ssci)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;

    macsec_message_init(&message, MACSEC_DP_CREATE_TRANSMIT_SA, intf_id, 0);
    MACSEC_MESSAGE_ADD(&message, portid);
    MACSEC_MESSAGE_ADD(&message, an);
    MACSEC_MESSAGE_ADD(&message, next_pn);
    MACSEC_MESSAGE_ADD(&message, *ki);
    MACSEC_MESSAGE_ADD(&message, *ssci);

    macsec_message_send(dp_conn->fd, &message);

    macsec_message_receive_dp_data(&message, NULL, 0);
}

void
macsec_enable_transmit(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an)
{
  LmiConnection *dp_conn = &dp_connection;
  MacsecMessage message;

  macsec_message_init(&message, MACSEC_DP_ENABLE_TRANSMIT, intf_id, 0);
  MACSEC_MESSAGE_ADD(&message, portid);
  MACSEC_MESSAGE_ADD(&message, an);

  macsec_message_send(dp_conn->fd, &message);

  macsec_message_receive_dp_data(&message, NULL, 0);
}

void
macsec_get_transmit_sa_next_pn(
  MacsecIntfId intf_id,
  unsigned portid,
  unsigned an,
  uint64_t *next_pn)
{
  LmiConnection *dp_conn = &dp_connection;
  MacsecMessage message;

  macsec_message_init(&message, MACSEC_DP_GET_TRANSMIT_SA_NEXT_PN, intf_id, 0);
  MACSEC_MESSAGE_ADD(&message, portid);
  MACSEC_MESSAGE_ADD(&message, an);
  macsec_message_send(dp_conn->fd, &message);

  if (!macsec_message_error(&message))
    {
      macsec_message_clear(&message);
      MACSEC_MESSAGE_ADD(&message, *next_pn);
      macsec_message_receive(dp_conn->fd, &message);
    }
}

void
macsec_get_transmit_sa_stat(
        MacsecIntfId intf_id,
        unsigned portid,
        unsigned an,
        MacsecTransmitSaStat *stat)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;

    macsec_message_init(&message, MACSEC_DP_GET_TRANSMIT_SA_STAT, intf_id, 0);
    MACSEC_MESSAGE_ADD(&message, portid);
    MACSEC_MESSAGE_ADD(&message, an);

    macsec_message_send(dp_conn->fd, &message);

    macsec_message_receive_dp_data(&message, stat, sizeof *stat);
}

void
macsec_get_latest_an(
        MacsecIntfId intf_id,
        unsigned portid,
        unsigned *latest_an)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_LATEST_AN,
            intf_id, 0, portid,
            latest_an, sizeof *latest_an);
}

void
macsec_create_receive_sa(
        MacsecPeerId peer_id,
        unsigned an,
        uint64_t lowest_pn,
        MacsecKi *ki,
        const MacsecSsci *ssci)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;

    macsec_message_init(&message, MACSEC_DP_CREATE_RECEIVE_SA, 0, peer_id);
    MACSEC_MESSAGE_ADD(&message, an);
    MACSEC_MESSAGE_ADD(&message, lowest_pn);
    MACSEC_MESSAGE_ADD(&message, *ki);
    MACSEC_MESSAGE_ADD(&message, *ssci);

    macsec_message_send(dp_conn->fd, &message);

    macsec_message_receive_dp_data(&message, NULL, 0);
}

void
macsec_enable_receive(
        MacsecPeerId peer_id,
        unsigned an)
{
    macsec_set_dp_data(
            MACSEC_DP_ENABLE_RECEIVE,
            0, peer_id, 0,
            &an, sizeof an);
}

void
macsec_set_receive_sa_lowest_pn(
        MacsecPeerId peer_id,
        unsigned an,
        uint64_t lowest_pn)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;

    macsec_message_init(&message, MACSEC_DP_SET_RECEIVE_LOWEST_PN, 0, peer_id);
    MACSEC_MESSAGE_ADD(&message, an);
    MACSEC_MESSAGE_ADD(&message, lowest_pn);

    macsec_message_send(dp_conn->fd, &message);

    macsec_message_receive_dp_data(&message, NULL, 0);
}

void
macsec_get_receive_sa_stat(
        MacsecPeerId peer_id,
        unsigned an,
        MacsecReceiveSaStat *stat)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;

    macsec_message_init(&message, MACSEC_DP_GET_RECEIVE_SA_STAT, 0, peer_id);
    MACSEC_MESSAGE_ADD(&message, an);

    macsec_message_send(dp_conn->fd, &message);
    macsec_message_receive_dp_data(&message, stat, sizeof *stat);
}

void
macsec_set_multicast_address(
        MacsecIntfId intf_id,
        const MacsecAddress *address)
{
    macsec_set_dp_data(
            MACSEC_DP_SET_MULTICAST_ADDRESS,
            intf_id, 0, 0,
            address, sizeof *address);
}

void
macsec_get_multicast_address(
        MacsecIntfId intf_id,
        MacsecAddress *address)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_MULTICAST_ADDRESS,
            intf_id, 0, 0,
            address, sizeof *address);
}

void
macsec_get_common_port_enabled(
        MacsecIntfId intf_id,
        int *value)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_COMMON_PORT_ENABLED,
            intf_id, 0, 0,
            value, sizeof *value);
}

int
macsec_send_uncontrolled(
        MacsecIntfId intf_id,
        const MacsecAddress *dst,
        const MacsecAddress *src,
        const unsigned char *msdubuf,
        unsigned msdulen)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;
    int result = 0;

    macsec_message_init(&message, MACSEC_DP_SEND_UNCONTROLLED, intf_id, 0);

    MACSEC_MESSAGE_ADD(&message, *dst);
    MACSEC_MESSAGE_ADD(&message, *src);
    MACSEC_MESSAGE_ADD(&message, msdulen);
    macsec_message_add(&message, msdubuf, msdulen);

    macsec_message_send(dp_conn->fd, &message);

    macsec_message_receive_dp_data(&message, &result, sizeof result);

    return result;
}

unsigned
macsec_receive_uncontrolled(
        MacsecIntfId intf_id,
        MacsecAddress *dst,
        MacsecAddress *src,
        unsigned char *msdubuf,
        unsigned msdulen)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;
    unsigned rlen = 0;

    macsec_message_init(&message, MACSEC_DP_RECEIVE_UNCONTROLLED, intf_id, 0);
    MACSEC_MESSAGE_ADD(&message, msdulen);

    macsec_message_send(dp_conn->fd, &message);

    if (!macsec_message_error(&message))
    {
        macsec_message_clear(&message);

        MACSEC_MESSAGE_ADD(&message, *dst);
        MACSEC_MESSAGE_ADD(&message, *src);
        MACSEC_MESSAGE_ADD(&message, rlen);
        macsec_message_add(&message, msdubuf, msdulen);

        macsec_message_receive(dp_conn->fd, &message);

        if (rlen > msdulen)
            rlen = 0;
    }

    return rlen;
}

void
macsec_get_common_stat(
        MacsecIntfId intf_id,
        MacsecPortStat *stat)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_COMMON_STAT,
            intf_id, 0, 0,
            stat, sizeof *stat);
}

void
macsec_get_uncontrolled_stat(
        MacsecIntfId intf_id,
        MacsecPortStat *stat)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_UNCONTROLLED_STAT,
            intf_id, 0, 0,
            stat, sizeof *stat);
}

void
macsec_get_controlled_stat(
        MacsecIntfId intf_id,
        unsigned portid,
        MacsecPortStat *stat)
{
    macsec_get_dp_data(
            MACSEC_DP_GET_CONTROLLED_STAT,
            intf_id, 0, portid,
            stat, sizeof *stat);
}

int
macsec_test(void)
{
    int result;
    macsec_get_dp_data(
            MACSEC_DP_TEST,
            0, 0, 0,
            &result, sizeof result);
    return result;
}

void
macsec_uninit_intf(
        MacsecIntfId intf_id)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;

    macsec_message_init(&message, MACSEC_DP_UNINIT_INTF, intf_id, 0);
    macsec_message_send(dp_conn->fd, &message);

    macsec_message_receive_dp_data(&message, NULL, 0);
}

MacsecIntfId
macsec_init_intf(
        const MacsecConfig *config)
{
    LmiConnection *dp_conn = &dp_connection;
    MacsecMessage message;
    MacsecIntfId ID;

    macsec_message_init(&message, MACSEC_DP_INIT_INTF, 0, 0);
    MACSEC_MESSAGE_ADD(&message, *config);
    macsec_message_send(dp_conn->fd, &message);

    if (!macsec_message_error(&message))
    {
        macsec_message_clear(&message);
        MACSEC_MESSAGE_ADD(&message, ID);

        macsec_message_receive(dp_conn->fd, &message);
        *config->fdescp = receive_fd(dp_conn->fd);
    }

    return ID;
}

void
macsec_set_intf_flags(
        MacsecIntfId intf_id,
        unsigned flags)
{
  macsec_set_dp_data(
          MACSEC_DP_SET_INTF_FLAGS,
          intf_id, 0, 0,
          &flags, sizeof flags);
}

MacsecIntfId
macsec_next_intf(
        MacsecIntfId prev)
{
  MacsecIntfId ID;

  macsec_get_dp_data(
          MACSEC_DP_NEXT_INTF,
          prev, 0, 0,
          &ID, sizeof ID);

  return ID;
}

int
macsec_open(
        void)
{
  int ok = 1;
  LmiConnection *dp_conn = &dp_connection;

  struct sockaddr_un addr;
  socklen_t addr_len;

  memset(&addr, 0, sizeof addr);
  memcpy(addr.sun_path, LMI_SOCKET_PATH, strlen(LMI_SOCKET_PATH));
  addr.sun_family = AF_UNIX;
  addr_len = sizeof(addr.sun_family) + strlen(addr.sun_path) + 1;

  dp_conn->fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (dp_conn->fd < 0)
    {
      ml_error("Failed to open socket");
      ok = 0;
    }

  if (ok)
    {
      if (connect(dp_conn->fd, (void *) &addr, addr_len) < 0)
        {
          ml_error("Failed to connect");
          ok = 0;
        }
    }

  return ok;
}

void
macsec_close(
        void)
{
  LmiConnection *dp_conn = &dp_connection;

  close(dp_conn->fd);

  dp_conn->fd = -1;
}

static int
receive_fd(int s)
{
  int n;
  struct msghdr msg = {0};
  struct cmsghdr *cmsg;
  int fd = -1;
  char buf[CMSG_SPACE(sizeof fd)];
  int data;

  struct iovec msg_iov;

  memset(buf, 0, sizeof buf);

  msg.msg_control = buf;
  msg.msg_controllen = sizeof buf;

  msg_iov.iov_base = &data;
  msg_iov.iov_len = sizeof data;

  msg.msg_iov = &msg_iov;
  msg.msg_iovlen = 1;

  n = recvmsg(s, &msg, 0);
  if (n < 0)
    {
      ml_error("recvmsg()");
    }
  else
  if (n > 0)
    {
      if ((msg.msg_flags & MSG_CTRUNC) != 0)
        {
          ml_error("control messages truncated.");
        }

      for (cmsg = CMSG_FIRSTHDR(&msg);
           cmsg != NULL;
           cmsg = CMSG_NXTHDR(&msg, cmsg))
        {
          if (cmsg->cmsg_level == SOL_SOCKET &&
                cmsg->cmsg_type == SCM_RIGHTS)
            {
              break;
            }
        }

      if (cmsg != NULL)
        {
          const int *fd_p = (int *) CMSG_DATA(cmsg);

          if (CMSG_LEN(sizeof *fd_p) <= cmsg->cmsg_len)
            {
              fd = *fd_p;
            }
        }
    }

  return fd;
}
