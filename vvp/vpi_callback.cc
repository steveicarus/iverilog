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
#ident "$Id: vpi_callback.cc,v 1.13 2002/05/04 03:03:17 steve Exp $"
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
# include  "functor.h"
# include  "event.h"
# include  <stdio.h>
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>

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
 * Callback handles are created when the VPI function registers a
 * callback. The handle is stored by the run time, and it triggered
 * when the run-time thing that it is waiting for happens.
 *
 * This is the thing that the VPI code references by the vpiHandle. It
 * also points to callback data that the caller attached to the event,
 * as well as the time structure to receive data.
 *
 * The cb_sync is a private member that points to the schedulable
 * event that is triggered when the event happens. The sync_cb class
 * represents the action to execute when the scheduler gets to this
 * event. This member is only used for things like cbReadOnlySync.
 */

struct __vpiCallback {
      struct __vpiHandle base;

	// user supplied callback data
      struct t_cb_data cb_data;
      struct t_vpi_time cb_time;

	// scheduled event
      struct sync_cb* cb_sync;

	// Used for listing callbacks.
      struct __vpiCallback*next;
};

struct sync_cb  : public vvp_gen_event_s {
      struct __vpiCallback*handle;
};


inline static struct __vpiCallback* new_vpi_callback()
{
      struct __vpiCallback* obj;

      obj = new __vpiCallback;

      obj->base.vpi_type = &callback_rt;
      obj->cb_sync = 0;
      obj->next    = 0;
      return obj;
}

inline static void free_vpi_callback(struct __vpiCallback* obj)
{
      delete obj;
}

/*
 * The callback_functor_s functor is used to support cbValueChange
 * callbacks. When a value change callback is created, instances of
 * this functor are created to receive values and detect changes in
 * the functor web at the right spot.
 */

struct callback_functor_s: public functor_s {
      callback_functor_s();
      virtual ~callback_functor_s();
      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      struct __vpiCallback *cb_handle;
};

callback_functor_s::callback_functor_s() 
{}

callback_functor_s::~callback_functor_s()
{}

/*
 * Make the functors necessary to support an edge callback. This is a
 * single event functor that is in turn fed by edge detector functors
 * attached to all the input bit functors. (Just like an event-or.)
 * This function creates the callback_functor and the necessary event
 * functors, and attaches the event functors to the net.
 */
struct callback_functor_s *vvp_fvector_make_callback(vvp_fvector_t vec,
						     event_functor_s::edge_t edge)
{
      // Let's make a callback functor.
      struct callback_functor_s *obj = new callback_functor_s;

      // We want to connect nvec inputs
      unsigned nvec = vvp_fvector_size(vec);

      // We require a total of nfun functors.  If there's an edge to
      // look for, then all inputs must go to extra event functors.
      // Otherwise we just make sure we have one input left on the base
      // functor to connect the extras to, if there are any.
      unsigned nfun = (nvec+3)/4;
      if (edge != vvp_edge_none)
	    nfun ++ ;
      else if (nfun > 1  &&  4*nfun == nvec)
	    nfun ++;
      
      vvp_ipoint_t fdx = functor_allocate(nfun);
      functor_define(fdx, obj);

      for (unsigned i=1; i<nfun; i++) {
	functor_t fu = new event_functor_s(edge);
	functor_define(ipoint_index(fdx, i), fu);
	fu->out = fdx;
      }

      unsigned i;

      if (edge != vvp_edge_none)
	    i = 4;
      else if (nvec > 4)
	    i = 1;
      else 
	    i = 0;

      for (unsigned vi = 0;  vi < nvec;  vi++, i++) {
	    vvp_ipoint_t vipt = vvp_fvector_get(vec, vi);
	    functor_t vfu = functor_index(vipt);

	    vvp_ipoint_t ipt = ipoint_input_index(fdx, i);
	    functor_t fu = functor_index(ipt);
	    unsigned pp = ipoint_port(ipt);

	    fu->port[pp] = vfu->out;
	    vfu->out = ipt;
      }

      obj->cb_handle = 0;
      return obj;
}

/*
 * A value change callback is tripped when a bit of a signal
 * changes. This function creates that value change callback and
 * attaches it to the relevent vpiSignal object. Also, if the signal
 * does not already have them, create some callback functors to do the
 * actual value change detection.
 */
static struct __vpiCallback* make_value_change(p_cb_data data)
{
      struct __vpiCallback*obj = new_vpi_callback();
      obj->cb_data = *data;
      obj->cb_time = *(data->time);
      obj->cb_data.time = &obj->cb_time;

      assert(data->obj);
      assert(data->obj->vpi_type);
      assert((data->obj->vpi_type->type_code == vpiReg)
	     || (data->obj->vpi_type->type_code == vpiNet));
      struct __vpiSignal*sig = reinterpret_cast<__vpiSignal*>(data->obj);

	/* Create callback functors, if necessary, to do the value
	   change detection and carry the callback objects. */
      if (sig->callback == 0) {
	    sig->callback = vvp_fvector_make_callback(sig->bits);
	    assert(sig->callback);
      }

	/* Attach the __vpiCallback object to the signals callback
	   functors. */
      obj->next = sig->callback->cb_handle;
      sig->callback->cb_handle = obj;

      return obj;
}

static void make_sync_run(vvp_gen_event_t obj, unsigned char)
{
      struct sync_cb*cb = (struct sync_cb*)obj;
      if (cb->handle == 0)
	    return;

      struct __vpiCallback*cur = cb->handle;
      cur->cb_data.time->type = vpiSimTime;
      vpip_time_to_timestruct(cur->cb_data.time, schedule_simtime());
      (cur->cb_data.cb_rtn)(&cur->cb_data);

      free_vpi_callback(cur);
}

static struct __vpiCallback* make_sync(p_cb_data data, bool readonly_flag)
{
      struct __vpiCallback*obj = new_vpi_callback();
      obj->cb_data = *data;
      obj->cb_time = *(data->time);
      obj->cb_data.time = &obj->cb_time;

      obj->next = 0;

      struct sync_cb*cb = new sync_cb;
      cb->sync_flag = readonly_flag? true : false;
      cb->run = &make_sync_run;
      cb->handle = obj;
      obj->cb_sync = cb;

      switch (obj->cb_time.type) {
	  case vpiSuppressTime:
	    schedule_generic(cb, 0, 0);
	    break;

	  case vpiSimTime:
	      { vvp_time64_t tv = vpip_timestruct_to_time(&obj->cb_time);
		vvp_time64_t tn = schedule_simtime();
		if (tv < tn) {
		      schedule_generic(cb, 0, 0);
		} else {
		      schedule_generic(cb, 0, tv - tn);
		}
		break;
	      }

	  default:
	    assert(0);
	    break;
      }
      return obj;
}

/*
 * The following functions are the used for pre and post simulation
 * callbacks.
 */

static struct __vpiCallback*EndOfCompile = NULL;
static struct __vpiCallback*StartOfSimulation = NULL;
static struct __vpiCallback*EndOfSimulation = NULL;

void vpiPresim() {
      struct __vpiCallback* cur;

      /*
       * Walk the list of register callbacks
       */
      for (cur = EndOfCompile; cur != NULL; cur = cur->next) {
	  (cur->cb_data.cb_rtn)(&cur->cb_data);
      }
      for (cur = StartOfSimulation; cur != NULL; cur = cur->next) {
	  (cur->cb_data.cb_rtn)(&cur->cb_data);
      }
}

void vpiPostsim() {
      struct __vpiCallback* cur;

      /*
       * Walk the list of register callbacks
       */
      for (cur = EndOfSimulation; cur != NULL; cur = cur->next) {
	  (cur->cb_data.cb_rtn)(&cur->cb_data);
      }
}

static struct __vpiCallback* make_prepost(p_cb_data data)
{
      struct __vpiCallback*obj = new_vpi_callback();
      obj->cb_data = *data;

      /* Insert at head of list */
      switch (data->reason) {
	  case cbEndOfCompile:
	      obj->next = EndOfCompile;
	      EndOfCompile = obj;
	  break;
	  case cbStartOfSimulation:
	      obj->next = StartOfSimulation;
	      StartOfSimulation = obj;
	  break;
	  case cbEndOfSimulation:
	      obj->next = EndOfSimulation;
	      EndOfSimulation = obj;
	  break;
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

	  case cbReadOnlySynch:
	    obj = make_sync(data, true);
	    break;

	  case cbReadWriteSynch:
	    obj = make_sync(data, false);
	    break;

	  case cbEndOfCompile:
	  case cbStartOfSimulation:
	  case cbEndOfSimulation:
	    obj = make_prepost(data);
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
      assert(ref);
      assert(ref->vpi_type);
      assert(ref->vpi_type->type_code == vpiCallback);

      fprintf(stderr, "vpi error: vpi_remove_cb not supported\n");

      return 0;
}

/*
 * A callback_functor_s functor uses its set method to detect value
 * changes. When a value comes in, the __vpiCallback objects that are
 * associated with this callback functor are all called.
 */

void callback_functor_s::set(vvp_ipoint_t, bool, unsigned val, unsigned)
{
      struct __vpiCallback *next = cb_handle;

      while (next) {
	    struct __vpiCallback * cur = next;
	    next = cur->next;
	    if (cur->cb_data.value) {
		  switch (cur->cb_data.value->format) {
		  case vpiScalarVal:
			cur->cb_data.value->value.scalar = val;
			break;
		  case vpiSuppressVal:
			break;
		  default:
			fprintf(stderr, "vpi_callback: value format %d not supported\n", cur->cb_data.value->format);
		  }
	    }
	    cur->cb_data.time->type = vpiSimTime;
	    vpip_time_to_timestruct(cur->cb_data.time, schedule_simtime());
	    (cur->cb_data.cb_rtn)(&cur->cb_data);

      }
}



/*
 * $Log: vpi_callback.cc,v $
 * Revision 1.13  2002/05/04 03:03:17  steve
 *  Add simulator event callbacks.
 *
 * Revision 1.12  2002/04/20 04:33:23  steve
 *  Support specified times in cbReadOnlySync, and
 *  add support for cbReadWriteSync.
 *  Keep simulation time in a 64bit number.
 *
 * Revision 1.11  2002/04/06 20:25:45  steve
 *  cbValueChange automatically replays.
 *
 * Revision 1.10  2001/11/06 03:07:22  steve
 *  Code rearrange. (Stephan Boettcher)
 *
 * Revision 1.9  2001/10/31 04:27:47  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.8  2001/10/25 04:19:53  steve
 *  VPI support for callback to return values.
 *
 * Revision 1.7  2001/10/12 03:00:09  steve
 *  M42 implementation of mode 2 (Stephan Boettcher)
 *
 * Revision 1.6  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.5  2001/08/08 01:05:06  steve
 *  Initial implementation of vvp_fvectors.
 *  (Stephan Boettcher)
 *
 * Revision 1.4  2001/07/13 03:02:34  steve
 *  Rewire signal callback support for fast lookup. (Stephan Boettcher)
 *
 * Revision 1.3  2001/07/11 02:27:21  steve
 *  Add support for REadOnlySync and monitors.
 *
 * Revision 1.2  2001/06/21 23:05:08  steve
 *  Some documentation of callback behavior.
 *
 * Revision 1.1  2001/06/21 22:54:12  steve
 *  Support cbValueChange callbacks.
 *
 */

