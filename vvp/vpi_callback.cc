/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_callback.cc,v 1.2 2001/06/21 23:05:08 steve Exp $"
#endif

/*
 * Callbacks are objects that carry a function to be called when some
 * event in the simulation occurs. The VPI code create a __vpiCallback
 * object, and that object is put in some location that the simulation
 * can look when the event in question is tripped.
 */

# include  <vpi_user.h>
# include  "vpi_priv.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <assert.h>


const struct __vpirt callback_rt = {
      vpiCallback,

      0,
      0,
      0,
      0,

      0,
      0,
      0
};

/*
 * A value change callback is tripped when a bit of a signal
 * changes. This function creates that value change callback and
 * attaches it to the relevent vpiSignal object. Also flag the
 * functors associated with the signal so that they know to trip.
 */
static struct __vpiCallback* make_value_change(p_cb_data data)
{
      struct __vpiCallback*obj = new __vpiCallback;
      obj->base.vpi_type = &callback_rt;
      obj->cb_data = *data;

      assert((data->obj->vpi_type->type_code == vpiReg)
	     || (data->obj->vpi_type->type_code == vpiNet));
      struct __vpiSignal*sig = reinterpret_cast<__vpiSignal*>(data->obj);

	/* Attach the callback to the signal who's value I'm waiting for. */
      obj->next = sig->callbacks;
      sig->callbacks = obj;

      unsigned wid = (sig->msb >= sig->lsb)
	    ? sig->msb - sig->lsb + 1
	    : sig->lsb - sig->msb + 1;

	/* Make sure the functors are tickled to trigger a callback. */
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(sig->bits, idx);
	    functor_t fun = functor_index(ptr);
	    fun->callback |= 1;
      }

      return obj;
}

vpiHandle vpi_register_cb(p_cb_data data)
{
      struct __vpiCallback*obj = 0;

      switch (data->reason) {

	  case cbValueChange:
	    obj = make_value_change(data);
	    break;

	  default:
	    fprintf(stderr, "vpi error: vpi_register_cb invalid or "
		    "unsupported callback reason: %d\n",
		    data->reason);
	    break;
      }

      return obj? &obj->base : 0;
}


int vpi_remove_cb(vpiHandle ref)
{
      assert(ref->vpi_type->type_code == vpiCallback);

      struct __vpiCallback*obj = reinterpret_cast<__vpiCallback*>(ref);

      fprintf(stderr, "vpi error: vpi_remove_cb not supported\n");

      return 0;
}

/*
 * A functor callback trips when a functor is set by the functor_set
 * function. This only happens when a propagated value passes
 * through. This causes a callback only if the callback flag in the
 * functor is set.
 *
 * When I get to this point, I locate the signal is associated with
 * this functor and call all the callbacks on its callback list. The
 * callbacks are deleted as I go.
 */
void vpip_trip_functor_callbacks(vvp_ipoint_t ptr)
{
      struct __vpiSignal*sig = vpip_sig_from_ptr(ptr);
      assert(sig);

      struct __vpiCallback*callbacks = sig->callbacks;
      sig->callbacks = 0;

      while (callbacks) {
	    struct __vpiCallback*cur = callbacks;
	    callbacks = cur->next;

	    cur->cb_data.time->type = vpiSimTime;
	    cur->cb_data.time->low = schedule_simtime();
	    cur->cb_data.time->high = 0;
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    delete cur;
      }
}

void vpip_trip_monitor_callbacks(void)
{
}


/*
 * $Log: vpi_callback.cc,v $
 * Revision 1.2  2001/06/21 23:05:08  steve
 *  Some documentation of callback behavior.
 *
 * Revision 1.1  2001/06/21 22:54:12  steve
 *  Support cbValueChange callbacks.
 *
 */

