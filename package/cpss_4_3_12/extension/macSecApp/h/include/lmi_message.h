/**

   @copyright
   Copyright (c) 2020, Rambus Inc. All rights reserved.
*/

#ifndef LMI_MESSAGE_H
#define LMI_MESSAGE_H

#include <sys/uio.h>
#include <errno.h>

#include "macseclmi_types.h"

#define MACSEC_MESSAGE_IOVEC_SIZE 8

#define LMI_SOCKET_PATH "./lmi.sock"

typedef enum
{
  MACSEC_DP_NONE,
  MACSEC_DP_GET_CAPABILITIES,
  MACSEC_DP_SET_AUTHORIZATION,
  MACSEC_DP_SET_SECYCTRL,
  MACSEC_DP_GET_SECYCTRL,
  MACSEC_DP_GET_SECYSTAT,
  MACSEC_DP_SEND_UNCONTROLLED,
  MACSEC_DP_RECEIVE_UNCONTROLLED,
  MACSEC_DP_CREATE_PEER,
  MACSEC_DP_DESTROY_PEER,
  MACSEC_DP_GET_PEER_ATTRIBUTES,
  MACSEC_DP_NEXT_PEER,
  MACSEC_DP_GET_COMMON_STAT,
  MACSEC_DP_GET_UNCONTROLLED_STAT,
  MACSEC_DP_GET_CONTROLLED_STAT,
  MACSEC_DP_GET_TRANSMIT_SA_STAT,
  MACSEC_DP_GET_RECEIVE_SA_STAT,
  MACSEC_DP_GET_TRANSMIT_SC_STAT,
  MACSEC_DP_GET_RECEIVE_SC_STAT,
  MACSEC_DP_SET_CONTROLLED_PORT_ENABLED,
  MACSEC_DP_INSTALL_KEY,
  MACSEC_DP_CREATE_TRANSMIT_SA,
  MACSEC_DP_CREATE_RECEIVE_SA,
  MACSEC_DP_ENABLE_TRANSMIT,
  MACSEC_DP_ENABLE_RECEIVE,
  MACSEC_DP_GET_COMMON_PORT_ENABLED,
  MACSEC_DP_SET_RECEIVE_LOWEST_PN,
  MACSEC_DP_GET_MULTICAST_ADDRESS,
  MACSEC_DP_SET_MULTICAST_ADDRESS,
  MACSEC_DP_GET_LATEST_AN,
  MACSEC_DP_GET_INTF_ATTRIBUTES,
  MACSEC_DP_GET_TRANSMIT_SA_NEXT_PN,
  MACSEC_DP_INIT_INTF,
  MACSEC_DP_UNINIT_INTF,
  MACSEC_DP_NEXT_INTF,
  MACSEC_DP_TEST,
  MACSEC_DP_SET_INTF_FLAGS,
} MacsecDataplaneReq;

typedef struct MacsecMessageS
{
  MacsecDataplaneReq req;
  MacsecIntfId intf_id;
  MacsecPeerId peer_id;
  struct iovec io_vector[MACSEC_MESSAGE_IOVEC_SIZE];
  unsigned io_index;
  unsigned io_count;
  size_t size;
  size_t offset;
  int error;
  char *buffer;
  size_t buffer_size;
} MacsecMessage;

#define MACSEC_MESSAGE_ADD(message, item) \
  macsec_message_add(message, &(item), sizeof item)

#define MACSEC_MESSAGE_ADD_COPY(message, item)               \
    macsec_message_add_copy(message, &(item), sizeof item)

#define MACSEC_MESSAGE_GET_COPY(message, item)                  \
  macsec_message_get_copy(message, &(item), sizeof item)

void
macsec_message_add(
        MacsecMessage *message,
        const void *base,
        size_t len);

void
macsec_message_set_last_len(
        MacsecMessage *message,
        size_t len);

void *
macsec_message_add_space(
        MacsecMessage *message,
        size_t len);

void
macsec_message_add_copy(
        MacsecMessage *message,
        const void *base,
        size_t len);

void *
macsec_message_get(
        MacsecMessage *message,
        size_t size);

void
macsec_message_get_copy(
        MacsecMessage *message,
        void *p,
        size_t size);

void
macsec_message_init(
        MacsecMessage *message,
        MacsecDataplaneReq req,
        MacsecIntfId intf_id,
        MacsecPeerId peer_id);

void
macsec_message_set_buffer(
        MacsecMessage *message,
        char *buffer,
        size_t size);

void
macsec_message_clear(
        MacsecMessage *message);

void
macsec_message_send(
        int fd,
        MacsecMessage *message);

ssize_t
macsec_message_receive(
        int fd,
        MacsecMessage *message);

int
macsec_message_error(
        MacsecMessage *message);



int
macsec_connection_init(
        void);

void
macsec_connection_uninit(
        int fd);

#endif /* LMI_MESSAGE_H */
