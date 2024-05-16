## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I.. -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_builddir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600

AM_CFLAGS = $(WERROR)

noinst_LIBRARIES = libsysmon.a
sbin_PROGRAMS = sysmon

libsysmon_a_SOURCES = \
	sysmon.c sysmon_zebra.c

noinst_HEADERS = \
	sysmon.h

sysmon_SOURCES = \
	sysmon_main.c $(libsysmon_a_SOURCES)

sysmon_LDADD = ../lib/libzebra.la @LIBCAP@

examplesdir = $(exampledir)
#dist_examples_DATA = ripd.conf.sample

#EXTRA_DIST = RIPv2-MIB.txt

