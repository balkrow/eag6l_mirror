#include <zebra.h>

#include "command.h"
#include "prefix.h"
#include "plist.h"
#include "buffer.h"
#include "linklist.h"
#include "stream.h"
#include "thread.h"
#include "log.h"
#include "memory.h"
#include "hash.h"
#include "filter.h"
#include "sys_fifo.h"

extern int8_t sysmon_llcf_set
(
 int8_t enable
);

DEFUN (show_sysmon,
       show_sysmon_cmd,
       "show sysmon",
       SHOW_STR
       "sysmon info\n")
{
  vty_out (vty, "show sysmon test%s", VTY_NEWLINE);
  return CMD_SUCCESS;
}

DEFUN (llcf_conf,
       llcf_conf_cmd,
       "llcf (enable|disable)",
       "LLCF"
       "enable\n"
       "disable\n")
{

  if (strncmp (argv[0], "e", 1) == 0)
	sysmon_llcf_set(IPC_FUNC_ON);
  else
	sysmon_llcf_set(IPC_FUNC_OFF);

  return CMD_SUCCESS;
}


void
sysmon_vty_init (void)
{
  install_element (VIEW_NODE, &show_sysmon_cmd);
  install_element (VIEW_NODE, &llcf_conf_cmd);
}
