/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT)
#ident "$Id: vpi_callback.c,v 1.3 1999/10/29 03:37:22 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static struct __vpirt vpip_callback_rt = {
      vpiCallback,
      0,
      0,
      0,
      0,
      0
};

/*
 * This function is called by the scheduler to execute an event of
 * some sort. The parameter is a vpiHandle of the callback that is to
 * be executed.
 */
static void vpip_call_callback(void*cp)
{
      struct __vpiCallback*rfp = (struct __vpiCallback*)cp;
      assert(rfp->base.vpi_type->type_code == vpiCallback);
      rfp->cb_data.cb_rtn(&rfp->cb_data);
      free(rfp);
}

/*
 * This function is called by the product when it changes the value of
 * a signal. It arranges for all the value chance callbacks to be
 * called.
 */
void vpip_run_value_changes(struct __vpiSignal*sig)
{
      struct __vpiCallback*cur;
      if (sig->monitor == 0) return;

      while (sig->monitor->next != sig->monitor) {
	    cur = sig->monitor->next;
	    sig->monitor->next = cur->next;

	    cur->next = 0;
	    cur->ev = vpip_sim_insert_event(0, cur, vpip_call_callback, 0);
      }

      cur = sig->monitor;
      sig->monitor = 0;
      cur->next = 0;
      cur->ev = vpip_sim_insert_event(0, cur, vpip_call_callback, 0);
}

/*
 * Handle read-only synch events. This causes the callback to be
 * scheduled for a moment at the end of the time period. This method
 * handles scheduling with itme delays.
 */
static void go_readonly_synch(struct __vpiCallback*rfp)
{
      unsigned long tim;
      assert(rfp->cb_data.time);
      assert(rfp->cb_data.time->type == vpiSimTime);
      assert(rfp->cb_data.time->high == 0);
      tim = rfp->cb_data.time->low;
      rfp->ev = vpip_sim_insert_event(tim, rfp, vpip_call_callback, 1);
}

/*
 * To schedule a value change, attach the callback to the signal to me
 * monitored. I'll be inserted as an event later.
 */
static void go_value_change(struct __vpiCallback*rfp)
{
      struct __vpiSignal*sig = (struct __vpiSignal*)rfp->cb_data.obj;
      assert((sig->base.vpi_type->type_code == vpiReg)
	     || (sig->base.vpi_type->type_code == vpiNet));

	/* If there are no monitor events, start the list. */
      if (sig->monitor == 0) {
	    rfp->next = rfp;
	    sig->monitor = rfp;
	    return;
      }

	/* Put me at the end of the list. Remember that the monitor
	   points to the *last* item in the list. */
      rfp->next = sig->monitor->next;
      sig->monitor->next = rfp;
      sig->monitor = rfp;
}

/*
 * Register callbacks. This supports a variety of callback reasons,
 * mostly by dispatching them to a type specific handler.
 */
vpiHandle vpi_register_cb(p_cb_data data)
{
      struct __vpiCallback*rfp = calloc(1, sizeof(struct  __vpiCallback));
      rfp->base.vpi_type = &vpip_callback_rt;
      rfp->cb_data = *data;

      switch (data->reason) {
	  case cbReadOnlySynch:
	    go_readonly_synch(rfp);
	    break;

	  case cbValueChange:
	    go_value_change(rfp);
	    break;

	  default:
	    assert(0);
      }

      return &(rfp->base);
}


int vpi_remove_cb(vpiHandle ref)
{
      struct __vpiCallback*rfp = (struct __vpiCallback*)ref;
      assert(ref->vpi_type->type_code == vpiCallback);

      if (rfp->ev) {
	      /* callbacks attached to events are easy. */
	    vpip_sim_cancel_event(rfp->ev);

      } else {
	    assert(0);
      }

      free(rfp);
      return 0;
}

/*
 * $Log: vpi_callback.c,v $
 * Revision 1.3  1999/10/29 03:37:22  steve
 *  Support vpiValueChance callbacks.
 *
 * Revision 1.2  1999/10/28 04:47:57  steve
 *  Support delay in constSync callback.
 *
 * Revision 1.1  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 */

