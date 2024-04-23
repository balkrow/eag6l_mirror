/* Virtual terminal interface shell.
 * Copyright (C) 2000 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include "vtysh.h"
#include "command.h"

#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>


int execute_flag = 0;

/* Help information display. */
static void usage (char *progname, int status)
{
	
	exit (status);
}


void
sigint (int sig)
{
  /* Check this process is not child process. */
  if (! execute_flag)
    {
      rl_initialize ();
      printf ("\n");
      rl_forced_update_display ();
    }
}



/* Initialization of signal handles. */
static void signal_init ()
{
  signal (SIGINT, sigint);
  signal (SIGTSTP, SIG_IGN);
  signal (SIGPIPE, SIG_IGN);
}


/* VTY shell main routine. */
int main (int argc, char **argv, char **env)
{
	char *line;
		
	/* Signal and others. */
	signal_init ();

	/* Init the cmd */
	cmd_init();

	/* Init the vtysh */
	vtysh_init_vty ();

	/* Install command and node view */
	cmd_parse_init();

	//TODO load the dynamic so

	/* sort the node */
	cmd_sort_node();
		
	/* Main command loop. */
	while ((line = vtysh_readline()) != NULL)
		vtysh_execute (line);
	printf ("\n");

	exit (0);
}
