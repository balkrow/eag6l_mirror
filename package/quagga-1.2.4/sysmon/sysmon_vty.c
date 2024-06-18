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

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
extern GLOBAL_DB gDB;

uint8_t * gSvcFsmStateStr[SVC_ST_MAX] = {
	"INIT",
	"INIT Fail",
	"APPDemo Shutdown",
	"Check DPRAM access",
	"Check FPGA access",
	"Check DPRAM access",
	"SDK Init",
	"Get Inventory information",
	"INIT Done",
	"----"
};

uint8_t * gSvcFsmEvtStr[SVC_EVT_MAX] = {
	"None",
	"Init",
	"IPC Comm Successfully",
	"Wait IPC Comm",
	"IPC Comm Failure",
	"PLL Lock",
	"PLL Unlock",
	"DPRAM Access Successfully",
	"DPRAM Access Failed",
	"FPGA Access Successfully",
	"FPGA Access Failed",
	"CPLD Access Successfully",
	"CPLD Access Failed",
	"SDK Init Successfully",
	"SDK Init Failed",
	"Get Inventory Successfully",
	"Get Inventory Failed",
	"Port Link Up",
	"Port Link Down",
	"INIT Done",
	"AppDemo Shutdown",
	"----"
};
#endif

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

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
extern synce_if_pri_select(int8_t port, int8_t pri);
DEFUN (synce_if_conf,
       synce_if_conf_cmd,
       "sync-e (primary|secondary) IFNAME",
       "sync ethernet"
       "primary\n"
       "secondary\n"
       "interface number(etc 0,8,16..50)\n")
{
  uint8_t pri = 0;
  uint8_t port = 0;

  if(argv != 2)
	vty_out (vty, "parameter error%s", VTY_NEWLINE);

  if (strncmp (argv[0], "p", 1) == 0)
	  pri = 1;

  port = atoi(argv[1]); 
  synce_if_pri_select(port, pri);

  return CMD_SUCCESS;
}
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
DEFUN (show_sysmon_system,
       show_sysmon_system_cmd,
       "show system",
       SHOW_STR
       "sysmon info\n")
{
	/*fsm status*/
	vty_out(vty, "-------------------------------------------%s", VTY_NEWLINE);
	vty_out(vty, "Init FSM state %s (%s)%s", gSvcFsmStateStr[gDB.svc_fsm.state],
		gSvcFsmEvtStr[gDB.svc_fsm.evt], VTY_NEWLINE);
	vty_out(vty, "Init FSM state %x (%x)%s", gDB.svc_fsm.state,
		gDB.svc_fsm.evt, VTY_NEWLINE);
	return CMD_SUCCESS;
}
#endif

void
sysmon_vty_init (void)
{
  install_element (VIEW_NODE, &show_sysmon_cmd);
  install_element (VIEW_NODE, &llcf_conf_cmd);
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
  install_element (VIEW_NODE, &synce_if_conf_cmd);
  install_element (VIEW_NODE, &show_sysmon_system_cmd);
#endif
}
