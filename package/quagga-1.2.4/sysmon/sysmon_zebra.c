/* RIPd and zebra interface.
 * Copyright (C) 1997, 1999 Kunihiro Ishiguro <kunihiro@zebra.org>
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

#include <zebra.h>

#include "command.h"
#include "prefix.h"
#include "table.h"
#include "stream.h"
#include "memory.h"
#include "routemap.h"
#include "zclient.h"
#include "log.h"
#include "vrf.h"

/* All information about zebra. */
struct zclient *zclient = NULL;

void
rip_zclient_reset (void)
{
  zclient_reset (zclient);
}

/* RIP configuration write function. */
static int
config_write_zebra (struct vty *vty)
{
  if (! zclient->enable)
    {
      return 1;
    }
  else if (! vrf_bitmap_check (zclient->redist[ZEBRA_ROUTE_SYSMON], VRF_DEFAULT))
    {
      return 1;
    }
  return 0;
}

static void
sysmon_zebra_connected (struct zclient *zclient)
{
  zclient_send_requests (zclient, VRF_DEFAULT);
}

/* Inteface link up message processing */
int
sysmon_interface_up (int command, struct zclient *zclient, zebra_size_t length,
    vrf_id_t vrf_id)
{
return 0;
}

/* Inteface link down message processing. */
int
sysmon_interface_down (int command, struct zclient *zclient, zebra_size_t length,
    vrf_id_t vrf_id)
{
return 0;
}

/* Zebra node structure. */
static struct cmd_node zebra_node =
{
  ZEBRA_NODE,
  "%s(config-router)# ",
};

void
sysmon_zclient_init (struct thread_master *master)
{
  /* Set default value to the zebra client structure. */
  zclient = zclient_new (master);
  zclient_init (zclient, ZEBRA_ROUTE_SYSMON);
  zclient->zebra_connected = sysmon_zebra_connected;
#if 0
  zclient->interface_add = rip_interface_add;
  zclient->interface_delete = rip_interface_delete;
  zclient->interface_address_add = rip_interface_address_add;
  zclient->interface_address_delete = rip_interface_address_delete;
  zclient->ipv4_route_add = rip_zebra_read_ipv4;
  zclient->ipv4_route_delete = rip_zebra_read_ipv4;
#endif
  zclient->interface_up = sysmon_interface_up;
  zclient->interface_down = sysmon_interface_down;
  
  /* Install zebra node. */
  install_node (&zebra_node, config_write_zebra);

  /* Install command elements to zebra node. */ 
  //install_element (CONFIG_NODE, &router_zebra_cmd);
  //install_element (VIEW_NODE, &show_sysmon_cmd);
}
