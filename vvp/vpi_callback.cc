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
#ident "$Id: vpi_callback.cc,v 1.7 2001/10/12 03:00:09 steve Exp $"
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

static struct __vpiCallback* free_callback_root = 0x0;

inline static struct __vpiCallback* new_vpi_callback()
{
      struct __vpiCallback* obj;
      if (!free_callback_root) {
	    obj = new __vpiCallback;
	    obj->base.vpi_type = &callback_rt;
      } else {
	    obj = free_callback_root;
	    free_callback_root = obj->next;
      }
      obj->next = 0x0;
      return obj;
}

inline static void free_vpi_callback(struct __vpiCallback* obj)
{
      obj->next = free_callback_root;
      free_callback_root = obj;
}

/*
 * A signal get equipped with an M42 event functor to trigger 
 * callbacks.
 */

struct vvp_cb_fobj_s *vvp_fvector_make_callback(vvp_fvector_t vec,
						const unsigned char *edge)
{
      struct vvp_cb_fobj_s *obj = new vvp_cb_fobj_s;
      unsigned nvec = vvp_fvector_size(vec);
      unsigned nfun = (nvec+3)/4;

      if (edge)
	    nfun ++ ;
      else if (nfun > 1  &&  4*nfun == nvec)
	    nfun ++;
      
      vvp_ipoint_t fdx = functor_allocate(nfun);
      unsigned vi = 0;
      for (unsigned i = 0; i<nfun; i++) {
	    vvp_ipoint_t ipt = ipoint_index(fdx, i);
	    functor_t fu = functor_index(ipt);
	    fu->ival = 0xaa;
	    fu->oval = 2;
	    fu->odrive0 = 6;
	    fu->odrive0 = 6;
	    fu->out  = 0;
#if defined(WITH_DEBUG)
	    fu->breakpoint = 0;
#endif
	    if (i == nfun-1) {
		  fu->mode = M42;
		  fu->obj = obj;
	    } else {
		  fu->mode = 1;
		  fu->event = (struct vvp_event_s*) 
			malloc(sizeof (struct vvp_event_s));
		  fu->event->threads = 0;
		  fu->old_ival = fu->ival;
		  fu->event->vvp_edge_tab = edge;
		  fu->out = ipoint_input_index(ipoint_index(fdx, nfun-1), 3);
	    }
	    for (unsigned j=0; j<4 && vi < nvec; j++, vi++) {
		  vvp_ipoint_t vipt = vvp_fvector_get(vec, vi);
		  functor_t vfu = functor_index(vipt);
		  fu->port[j] = vfu->out;
		  vfu->out = ipoint_input_index(ipt, j);
	    }
      }
      obj->permanent = 0;
      obj->cb_handle = 0;
      return obj;
}

/*
 * A value change callback is tripped when a bit of a signal
 * changes. This function creates that value change callback and
 * attaches it to the relevent vpiSignal object. Also flag the
 * functors associated with the signal so that they know to trip.
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

      if (!sig->callback) {
	    sig->callback = vvp_fvector_make_callback(sig->bits);
	    assert(sig->callback);
      }
      
      obj->next = sig->callback->cb_handle;
      sig->callback->cb_handle = obj;

      return obj;
}

struct sync_cb  : public vvp_gen_event_s {
      struct __vpiCallback*handle;
};

static void make_sync_run(vvp_gen_event_t obj, unsigned char)
{
      struct sync_cb*cb = (struct sync_cb*)obj;
      if (cb->handle == 0)
	    return;

      struct __vpiCallback*cur = cb->handle;
      cur->cb_data.time->type = vpiSimTime;
      cur->cb_data.time->low = schedule_simtime();
      cur->cb_data.time->high = 0;
      (cur->cb_data.cb_rtn)(&cur->cb_data);

      free_vpi_callback(cur);
}

static struct __vpiCallback* make_sync(p_cb_data data)
{
      struct __vpiCallback*obj = new_vpi_callback();
      obj->cb_data = *data;
      obj->cb_time = *(data->time);
      obj->cb_data.time = &obj->cb_time;

      obj->next = 0;

      struct sync_cb*cb = new sync_cb;
      cb->sync_flag = true;
      cb->run = &make_sync_run;
      cb->handle = obj;
      obj->cb_sync = cb;

      schedule_generic(cb, 0, 0);
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
	    obj = make_sync(data);
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
 * A callback event happened.
 */

void vvp_cb_fobj_s::set(vvp_ipoint_t i, functor_t f, bool push)
{
      struct __vpiCallback *next = cb_handle;
      if (!permanent)
	    cb_handle = 0;
      while (next) {
	    struct __vpiCallback * cur = next;
	    next = cur->next;
	    cur->cb_data.time->type = vpiSimTime;
	    cur->cb_data.time->low = schedule_simtime();
	    cur->cb_data.time->high = 0;
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    if (!permanent)
		  free_vpi_callback(cur);
      }
}


void vpip_trip_monitor_callbacks(void)
{
}


/*
 * $Log: vpi_callback.cc,v $
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

