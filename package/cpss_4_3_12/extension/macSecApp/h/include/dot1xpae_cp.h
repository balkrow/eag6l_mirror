/**

   IEEE 802.1X-2010 CP state machine.

   File: dot1xpae_cp.h

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1XPAE_CP_H
#define _DOT1XPAE_CP_H

#include "dot1xpae_types.h"

/*
 * Run the CP state machine of the interface. Return 1 if it changed
 * any conditions that might affect other state machines, 0 otherwise.
 */
int
dot1x_step_cp(Dot1xIntf *intf);

/*
 * Update CP timers.
 */
void
dot1x_tick_cp(Dot1xIntf *intf);

#endif /* _DOT1XPAE_CP_H */
