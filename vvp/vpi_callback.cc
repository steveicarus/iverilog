/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

/*
* The vpi_free_object() call to a callback doesn't actually delete
* anything, we instead allow the object to run its course and delete
* itself. The semantics of vpi_free_object for a callback is that it
* deletes the *handle*, and not the object itself, so given the vvp
* implementation, there is nothing to do here.
*/

static int free_simple_callback(vpiHandle ref)
{
      return 1;
}

const struct __vpirt callback_rt = {
      vpiCallback,

      0,
      0,
      0,
      0,

      0,
      0,
      0,

      &free_simple_callback
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

struct sync_cb  : public vvp_gen_event_s {
      struct __vpiCallback*handle;
      bool sync_flag;
};


struct __vpiCallback* new_vpi_callback()
{
      struct __vpiCallback* obj;

      obj = new __vpiCallback;

      obj->base.vpi_type = &callback_rt;
      obj->cb_sync = 0;
      obj->next    = 0;
      return obj;
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
 * attaches it to the relevant vpiSignal object. Also, if the signal
 * does not already have them, create some callback functors to do the
 * actual value change detection.
 */
static struct __vpiCallback* make_value_change(p_cb_data data)
{
      struct __vpiCallback*obj = new_vpi_callback();
      obj->cb_data = *data;
      if (data->time) {
	    obj->cb_time = *(data->time);
      } else {
	    obj->cb_time.type = vpiSuppressTime;
      }
      obj->cb_data.time = &obj->cb_time;

      assert(data->obj);
      assert(data->obj->vpi_type);

      switch (data->obj->vpi_type->type_code) {

	  case vpiReg:
	  case vpiNet:
	  case vpiIntegerVar:
	    struct __vpiSignal*sig;
	    sig = reinterpret_cast<__vpiSignal*>(data->obj);

	      /* Create callback functors, if necessary, to do the
		 value change detection and carry the callback
		 objects. */
	    if (sig->callback == 0) {
		  sig->callback = vvp_fvector_make_callback(sig->bits);
		  assert(sig->callback);
	    }

	      /* Attach the __vpiCallback object to the signals
		 callback functors. */
	    obj->next = sig->callback->cb_handle;
	    sig->callback->cb_handle = obj;
	    break;

          case vpiMemory: /* callback for change in any location of this vpiMemory */
	    vpip_memory_value_change(obj,data->obj);
	    break;

	  case vpiRealVar:
	    vpip_real_value_change(obj, data->obj);
	    break;

	  case vpiNamedEvent:
	    struct __vpiNamedEvent*nev;
	    nev = reinterpret_cast<__vpiNamedEvent*>(data->obj);
	    obj->next = nev->callbacks;
	    nev->callbacks = obj;
	    break;

	  case vpiModule:
	  case vpiConstant:
	  case vpiParameter:
	      /* These are constant, so there are no value change
		 lists to put them in. */
	    break;

	  default:
	    fprintf(stderr, "make_value_change: sorry: I cannot callback "
		    "values on type code=%d\n",
		    data->obj->vpi_type->type_code);
	    delete obj;
	    return 0;
      }

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

	/* Run the callback. If the cb_rtn function pointer is set to
	   null, then just skip the whole thing and free it. This is
	   the usual way to cancel one-time callbacks of this sort. */
      if (cur->cb_data.cb_rtn != 0) {
	    assert(vpi_mode_flag == VPI_MODE_NONE);
	    vpi_mode_flag = cb->sync_flag? VPI_MODE_ROSYNC : VPI_MODE_RWSYNC;
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    vpi_mode_flag = VPI_MODE_NONE;
      }

      vpi_free_object(&cur->base);
}

static struct __vpiCallback* make_sync(p_cb_data data, bool readonly_flag)
{
      struct __vpiCallback*obj = new_vpi_callback();
      obj->cb_data = *data;
      assert(data->time);
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
	    schedule_generic(cb, 0, 0, readonly_flag);
	    break;

	  case vpiSimTime:
	      { vvp_time64_t tv = vpip_timestruct_to_time(&obj->cb_time);
		vvp_time64_t tn = schedule_simtime();
		if (tv < tn) {
		      schedule_generic(cb, 0, 0, readonly_flag);
		} else {
		      schedule_generic(cb, 0, tv - tn, readonly_flag);
		}
		break;
	      }

	  default:
	    assert(0);
	    break;
      }
      return obj;
}

static struct __vpiCallback* make_afterdelay(p_cb_data data)
{
      struct __vpiCallback*obj = new_vpi_callback();
      obj->cb_data = *data;
      assert(data->time);
      obj->cb_time = *(data->time);
      obj->cb_data.time = &obj->cb_time;

      obj->next = 0;

      struct sync_cb*cb = new sync_cb;
      cb->sync_flag = false;
      cb->run = &make_sync_run;
      cb->handle = obj;
      obj->cb_sync = cb;

      switch (obj->cb_time.type) {
	  case vpiSimTime: {
		vvp_time64_t tv = vpip_timestruct_to_time(&obj->cb_time);
		schedule_generic(cb, 0, tv, false);
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

static struct __vpiCallback*NextSimTime = 0;
static struct __vpiCallback*EndOfCompile = NULL;
static struct __vpiCallback*StartOfSimulation = NULL;
static struct __vpiCallback*EndOfSimulation = NULL;

void vpiPresim(void) {
      struct __vpiCallback* cur;

      /*
       * Walk the list of register callbacks, executing them and
       * freeing them when done.
       */
      assert(vpi_mode_flag == VPI_MODE_NONE);
      vpi_mode_flag = VPI_MODE_RWSYNC;

      while (EndOfCompile) {
	    cur = EndOfCompile;
	    EndOfCompile = cur->next;
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    vpi_free_object(&cur->base);
      }

      while (StartOfSimulation) {
	    cur = StartOfSimulation;
	    StartOfSimulation = cur->next;
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    vpi_free_object(&cur->base);
      }

      vpi_mode_flag = VPI_MODE_NONE;
}

void vpiPostsim(void) {
      struct __vpiCallback* cur;

      /*
       * Walk the list of register callbacks
       */
      assert(vpi_mode_flag == VPI_MODE_NONE);
      vpi_mode_flag = VPI_MODE_ROSYNC;

      while (EndOfSimulation) {
	    cur = EndOfSimulation;
	    EndOfSimulation = cur->next;
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    vpi_free_object(&cur->base);
      }

      vpi_mode_flag = VPI_MODE_NONE;
}

/*
 * The scheduler invokes this to clear out callbacks for the next
 * simulation time.
 */
void vpiNextSimTime(void)
{
      struct __vpiCallback* cur;

      while (NextSimTime) {
	    cur = NextSimTime;
	    NextSimTime = cur->next;
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    vpi_free_object(&cur->base);
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
	  case cbNextSimTime:
	    obj->next = NextSimTime;
	    NextSimTime = obj;
      }

      return obj;
}

vpiHandle vpi_register_cb(p_cb_data data)
{
      struct __vpiCallback*obj = 0;

      assert(data);
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

	  case cbAfterDelay:
	    obj = make_afterdelay(data);
	    break;

	  case cbEndOfCompile:
	  case cbStartOfSimulation:
	  case cbEndOfSimulation:
	  case cbNextSimTime:
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

/*
 * Removing a callback doesn't really delete it right away. Instead,
 * it clears the reference to the user callback function. This causes
 * the callback to quietly reap itself.
 */
PLI_INT32 vpi_remove_cb(vpiHandle ref)
{
      assert(ref);
      assert(ref->vpi_type);
      assert(ref->vpi_type->type_code == vpiCallback);

      struct __vpiCallback*obj = (struct __vpiCallback*)ref;
      obj->cb_data.cb_rtn = 0;

      return 1;
}

void callback_execute(struct __vpiCallback*cur)
{
      const vpi_mode_t save_mode = vpi_mode_flag;
      vpi_mode_flag = VPI_MODE_RWSYNC;

      assert(cur->cb_data.cb_rtn);
      cur->cb_data.time->type = vpiSimTime;
      vpip_time_to_timestruct(cur->cb_data.time, schedule_simtime());
      (cur->cb_data.cb_rtn)(&cur->cb_data);

      vpi_mode_flag = save_mode;
}

/*
 * A callback_functor_s functor uses its set method to detect value
 * changes. When a value comes in, the __vpiCallback objects that are
 * associated with this callback functor are all called.
 */

void callback_functor_s::set(vvp_ipoint_t, bool, unsigned val, unsigned)
{
      struct __vpiCallback *next = cb_handle;
      struct __vpiCallback *prev = 0;

      while (next) {
	    struct __vpiCallback * cur = next;
	    next = cur->next;

	    if (cur->cb_data.cb_rtn != 0) {
		  if (cur->cb_data.value) {
			switch (cur->cb_data.value->format) {
			    case vpiScalarVal:
			      cur->cb_data.value->value.scalar = val;
			      break;
			    case vpiSuppressVal:
			      break;
			    default:
			      fprintf(stderr, "vpi_callback: value "
				      "format %d not supported\n",
				      cur->cb_data.value->format);
			}
		  }

		  callback_execute(cur);
		  prev = cur;

	    } else if (prev == 0) {

		  cb_handle = next;
		  cur->next = 0;
		  vpi_free_object(&cur->base);

	    } else {
		  assert(prev->next == cur);
		  prev->next = next;
		  cur->next = 0;
		  vpi_free_object(&cur->base);
	    }

      }
}
