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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_callback.c,v 1.2 2002/08/12 01:35:05 steve Exp $"
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
      unsigned long now;
      struct __vpiCallback*rfp = (struct __vpiCallback*)cp;
      assert(rfp->base.vpi_type->type_code == vpiCallback);

      switch (rfp->cb_data.time->type) {
	  case vpiSuppressTime:
	  case vpiScaledRealTime: /* XXXX not supported */
	    break;

	  case vpiSimTime:
	    now = ((struct __vpiTimeVar*)vpip_sim_time())->time;
	    rfp->cb_data.time->low = now;
	    rfp->cb_data.time->high = 0;
	    break;
      }

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

      while (sig->mfirst) {
	    cur = sig->mfirst;
	    sig->mfirst = cur->next;
	    if (sig->mfirst == 0)
		  sig->mlast = 0;
	    cur->next = 0;
	    cur->ev = vpip_sim_insert_event(0, cur, vpip_call_callback, 0);
      }
}

/*
 * Handle read-only synch events. This causes the callback to be
 * scheduled for a moment at the end of the time period. This method
 * handles scheduling with time delays.
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
      if (sig->mfirst == 0) {
	    rfp->sig = sig;
	    rfp->next = 0;
	    sig->mfirst = rfp;
	    sig->mlast = rfp;
	    return;
      }

	/* Put me at the end of the list. Remember that the monitor
	   points to the *last* item in the list. */
      rfp->sig = sig;
      rfp->next = 0;
      sig->mlast->next = rfp;
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

      } else if (rfp->sig) {

	      /* callbacks to signals need to be removed from the
		 signal's list of monitor callbacks. */
	    struct __vpiSignal*sig = rfp->sig;

	    if (sig->mfirst == rfp) {
		  sig->mfirst = rfp->next;
		  if (sig->mfirst == 0)
			sig->mlast = 0;
		  rfp->next = 0;
		  rfp->sig = 0;
	    } else {
		  struct __vpiCallback*cur = sig->mfirst;
		  while (cur->next != rfp) {
			assert(cur->next);
			cur = cur->next;
		  }
		  cur->next = rfp->next;
		  if (cur->next == 0)
			sig->mlast = cur;
	    }
      } else {
	    assert(0);
      }

      free(rfp);
      return 0;
}

/*
 * $Log: vpi_callback.c,v $
 * Revision 1.2  2002/08/12 01:35:05  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.8  2000/08/20 17:49:05  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.7  2000/03/31 07:08:39  steve
 *  allow cancelling of cbValueChange events.
 *
 * Revision 1.6  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.5  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 * Revision 1.4  1999/11/07 20:33:30  steve
 *  Add VCD output and related system tasks.
 *
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

