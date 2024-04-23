#include "command.h"
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>

/* Show version. */
DEFUN (show_version,
       show_version_cmd,
       "show version",
       SHOW_STR
       "Displays the version information\n")
{
	char info[1024];

	sprintf(info, "Product 1830 VWM IUG.  All rights reserved. v1.1");
	
	vty_out (vty, "%s\n", info);

	return CMD_SUCCESS;
}

/* Down vty node level. */
DEFUN (config_exit,
       config_exit_cmd,
       "exit",
       "Exit \n")
{
 
		exit (0);
	
  return CMD_SUCCESS;
}
       


int cmd_common_init()
{
  /* Each node's basic commands. */
 
  cmd_install_element ( &show_version_cmd);
  cmd_install_element ( &config_exit_cmd);

  
  return 0;
}
