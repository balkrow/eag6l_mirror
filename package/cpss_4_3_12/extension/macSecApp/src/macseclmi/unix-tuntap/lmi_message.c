/**
   @copyright
   Copyright (c) 2020, Rambus Inc. All rights reserved.
*/

#define _POSIX_SOURCE

#include "lmi_message.h"
#include "macseclmi_sysdep.h"
#include "macseclmi_types.h"
#include <stdarg.h>
#include <stdio.h>

void
macsec_message_add(
        MacsecMessage *message,
        const void *base,
        size_t len)
{
  unsigned io_index = message->io_index;

  if (!message->error)
    {
      if (io_index < MACSEC_MESSAGE_IOVEC_SIZE)
        {
          message->io_vector[io_index].iov_base = (void *) base;
          message->io_vector[io_index].iov_len = len;
          message->io_index++;
        }
      else
        {
          message->error = 1;
        }
    }
}

void
macsec_message_set_last_len(
        MacsecMessage *message,
        size_t len)
{
  if (!message->error)
  {
    if (message->io_index > 0)
    {
      message->io_vector[message->io_index - 1].iov_len = len;
    }
    else
    {
      message->error = 1;
    }
  }
}


void *
macsec_message_add_space(
        MacsecMessage *message,
        size_t len)
{
  void *p = NULL;

  if (!message->error)
  {
    if (len <= message->buffer_size)
    {
      p = message->buffer;

      macsec_message_add(message, p, len);

      message->buffer += len;
      message->buffer_size -= len;
    }
    else
    {
      message->error = 1;
    }
  }

  return p;
}

void
macsec_message_add_copy(
        MacsecMessage *message,
        const void *base,
        size_t len)
{
  void *p = macsec_message_add_space(message, len);

  if (p)
    {
      memcpy(p, base, len);
    }
}

void *
macsec_message_get(
        MacsecMessage *message,
        size_t size)
{
  unsigned char *p = NULL;

  if (!message->error)
    {
      if (message->io_index < message->io_count)
        {
          struct iovec *iov = &message->io_vector[message->io_index];

          if (size <= iov->iov_len)
            {
              p = iov->iov_base;
              iov->iov_base = p + size;
              iov->iov_len -= size;

              if (iov->iov_len == 0)
                {
                  message->io_index++;
                }
            }
        }
      else
        {
          message->error = 1;
        }
    }

  return p;
}

void
macsec_message_get_copy(
        MacsecMessage *message,
        void *p,
        size_t size)
{
  void *d = macsec_message_get(message, size);

  if (d)
    memcpy(p, d, size);
}

void
macsec_message_init(
        MacsecMessage *message,
        MacsecDataplaneReq req,
        MacsecIntfId intf_id,
        MacsecPeerId peer_id)
{
  message->req = req;
  message->intf_id = intf_id;
  message->peer_id = peer_id;
  message->io_index = 0;
  message->offset = 0;
  message->size = 0;
  message->error = 0;
  message->buffer = NULL;
  message->buffer_size = 0;

  MACSEC_MESSAGE_ADD(message, message->req);
  MACSEC_MESSAGE_ADD(message, message->intf_id);
  MACSEC_MESSAGE_ADD(message, message->peer_id);
}

void
macsec_message_set_buffer(
        MacsecMessage *message,
        char *buffer,
        size_t size)
{
    message->buffer = buffer;
    message->buffer_size = size;
}

void
macsec_message_clear(
        MacsecMessage *message)
{
  macsec_message_init(message, 0, 0, 0);
}

void
macsec_message_send(
        int fd,
        MacsecMessage *message)
{
  if (!message->error)
    {
      ssize_t status = writev(fd, message->io_vector, message->io_index);
      if (status < 0)
        {
          ml_error("send() fd %d, req %d, error", fd, message->req);
          message->error = 1;
        }
    }
}

ssize_t
macsec_message_receive(
        int fd,
        MacsecMessage *message)
{
  ssize_t size = -1;

  while (!message->error)
    {
      size = readv(fd, message->io_vector, message->io_index);

      if (size < 0)
        {
          if (errno == EINTR) /* interrupted, try again */
            continue;

          ml_error("receive() fd %d, error", fd);
          message->error = 1;
        }

      break;
    }

  if (size >= 0)
    {
      message->size = size;

      message->io_count = message->io_index;
      message->io_index = 0;
    }

  if (size > 0)
    {
      MACSEC_MESSAGE_GET_COPY(message, message->req);
      MACSEC_MESSAGE_GET_COPY(message, message->intf_id);
      MACSEC_MESSAGE_GET_COPY(message, message->peer_id);
    }

  return size;
}

int
macsec_message_error(
        MacsecMessage *message)
{
  return message->error != 0;
}

static void
error_begin(const char *fmt, ...)
{
  struct timespec ts = {0, 0};
  unsigned long s, h, m, ms;
  int errno_save;
  va_list ap;

  errno_save = errno;

  ml_realtime(&ts);
  s = ts.tv_sec % 86400;
  h = s / 3600;
  s -= h * 3600;
  m = s / 60;
  s -= m * 60;
  ms = ts.tv_nsec / 1000000;

  fprintf(stderr, "%02lu:%02lu:%02lu.%03lu ", h, m, s, ms);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  if (ferror(stderr))
    clearerr(stderr);

  errno = errno_save;
}

static void
error_end_arg(int error, const char *fmt, va_list ap)
{
  char error_str[64];

  vfprintf(stderr, fmt, ap);
  if (error)
    {
      /* TODO strerror_r(error, error_str, sizeof error_str); */
      fprintf(stderr, ": %s", error_str);
    }
  fputc('\n', stderr);

  if (ferror(stderr))
    clearerr(stderr);

  errno = 0;
}

static void
error_end(const char *fmt, va_list ap)
{
  error_end_arg(errno, fmt, ap);

  errno = 0;
}

/*
 * Functions to be used by the system-dependend part.
 */
void
ml_error(const char *fmt, ...)
{
    va_list ap;

    flockfile(stderr);

    error_begin("%s", "");
    va_start(ap, fmt);
    error_end(fmt, ap);
    va_end(ap);

    funlockfile(stderr);
}

void
ml_error_arg(int error, const char *fmt, ...)
{
    va_list ap;

    flockfile(stderr);

    error_begin("%s", "");
    va_start(ap, fmt);
    error_end_arg(error, fmt, ap);
    va_end(ap);

    funlockfile(stderr);
}
