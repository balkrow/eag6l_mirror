/**

   QuickSec for MACsec example application, platform specific part for
   UNIX.

   File: macsecapp_platform.c

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#include "macsecapp_platform.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#if 0 /* MACSEC_APP_DRIVER */
/*
 * Configure signal handler.
 */
static void handlesig(int sig, void (*func)(void));
#endif

/*
 * Misc variables.
 */
static const char *program;
static int ttyfd = -1;
static struct termios oldttycfg;
static struct termios newttycfg;
static int ttymod;

#if 0 /* MACSEC_APP_DRIVER */
/*
 * Entry.
 */
int
main(int argc, char *argv[])
{
  if ((program = strrchr(argv[0], '/')))
    program++;
  else
    program = argv[0];

  errno = 0;

  handlesig(SIGHUP, ma_resume);
  handlesig(SIGQUIT, ma_quit);
  handlesig(SIGTERM, ma_quit);
  handlesig(SIGINT, ma_quit);
  handlesig(SIGUSR1, ma_print);
  handlesig(SIGUSR2, ma_configure);
  return ma_main(argc, argv);
}
#endif

/*
 * Public functions.
 */

int
ma_open_terminal()
{
  if (ttyfd >= 0)
    {
      ma_errorf("terminal already open");
      return -1;
    }

  if (!isatty(STDIN_FILENO))
    {
      errno = 0;
      return -1;
    }

  if ((ttyfd = dup(STDIN_FILENO)) < 0)
    {
      ma_errorf("dup(%d)", STDIN_FILENO);
      goto fail;
    }

  if (tcgetattr(ttyfd, &oldttycfg))
    {
      ma_errorf("tcgetattr");
      goto fail;
    }
  newttycfg = oldttycfg;
  cfmakeraw(&newttycfg);
  newttycfg.c_oflag = oldttycfg.c_oflag;
  if (tcsetattr(ttyfd, TCSANOW, &newttycfg))
    {
      ma_errorf("tcsetattr");
      goto fail;
    }
  ttymod = 1;

  return ttyfd;

 fail:
  if (ttyfd >= 0)
    ma_close_terminal(ttyfd);
  return -1;
}

void
ma_close_terminal(int fd)
{
  if (fd != ttyfd)
    {
      ma_errorf("bad terminal handle");
      return;
    }

  if (ttyfd < 0)
    {
      ma_errorf("terminal not open");
      return;
    }

  if (ttymod)
    {
      if (tcsetattr(ttyfd, TCSANOW, &oldttycfg))
        ma_errorf("tcsetattr");
      ttymod = 0;
    }

  if (close(ttyfd))
    ma_errorf("close");
  ttyfd = -1;
}

int
ma_read_terminal(int fd)
{
  unsigned char b;
  int n;

  if ((n = read(fd, &b, 1)) < 0)
    {
      if (errno != EAGAIN)
        ma_errorf("read");
      else
        errno = 0;
      return -1;
    }

  if (n <= 0)
    return -1;

  return (int)b;
}

void
ma_errorf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  ma_verrorf(fmt, ap);
  va_end(ap);
}

void
ma_verrorf(const char *fmt, va_list ap)
{
  ma_errorf_begin("%s", "");
  ma_verrorf_end(fmt, ap);
}

void
ma_errorf_begin(const char *fmt, ...)
{
  int errno_save;
  va_list ap;

  errno_save = errno;

  fprintf(stderr, "%s: ", program);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  if (ferror(stderr))
    clearerr(stderr);

  errno = errno_save;
}

void
ma_verrorf_end(const char *fmt, va_list ap)
{
  int errno_save;

  errno_save = errno;

  vfprintf(stderr, fmt, ap);
  if (errno_save)
    {
      fprintf(stderr, ": %s", strerror(errno_save));
      errno_save = 0;
    }
  fputc('\n', stderr);

  if (ferror(stderr))
    clearerr(stderr);

  errno = errno_save;
}

/*
 * Static functions.
 */

#if 0 /* MACSEC_APP_DRIVER */
static void
handlesig(int sig, void (*func)(void))
{
  struct sigaction act;

  memset(&act, 0, sizeof act);
  act.sa_handler = (void *)func;

  if (sigaction(sig, &act, NULL))
    ma_errorf("sigaction");
}
#endif

static void
log_timestamp_prefix(void)
{
    struct timespec ts = {0, 0};
    unsigned long s, h, m, ms;
    int errno_save;

    errno_save = errno;

    (void) clock_gettime(CLOCK_REALTIME, &ts);

    s = ts.tv_sec % 86400;
    h = s / 3600;
    s -= h * 3600;
    m = s / 60;
    s -= m * 60;
    ms = ts.tv_nsec / 1000000;

    fprintf(stderr, "%02lu:%02lu:%02lu.%03lu ", h, m, s, ms);

    if (ferror(stderr))
        clearerr(stderr);

    errno = errno_save;
}

void
dot1x_platform_log(const char *fmt, ...)
{
    va_list ap;

    flockfile(stderr);

    log_timestamp_prefix();

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    funlockfile(stderr);
}
