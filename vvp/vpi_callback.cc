/*
 * Copyright (c) 2001-2009 Stephen Williams (steve@icarus.com)
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

# include  "vpi_user.h"
# include  "vpi_priv.h"
# include  "vvp_net.h"
# include  "schedule.h"
# include  "event.h"
# include  "config.h"
# include  <cstdio>
# include  <cassert>
# include  <cstdlib>

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

      ~sync_cb () { }

      virtual void run_run();
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

void delete_vpi_callback(struct __vpiCallback* ref)
{
      assert(ref);
      assert(ref->base.vpi_type);
      assert(ref->base.vpi_type->type_code == vpiCallback);
      if (ref->cb_sync != 0) delete ref->cb_sync;
      delete ref;
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
      if (vpi_get(vpiAutomatic, data->obj)) {
            fprintf(stderr, "vpi error: cannot place value change "
                            "callback on automatically allocated "
                            "variable '%s'\n",
                            vpi_get_str(vpiName, data->obj));
            return 0;
      }

      struct __vpiCallback*obj = new_vpi_callback();
      obj->cb_data = *data;
      if (data->time) {
	    obj->cb_time = *(data->time);
      } else {
	    obj->cb_time.type = vpiSuppressTime;
      }
      obj->cb_data.time = &obj->cb_time;
      if (data->value) {
	    obj->cb_value = *(data->value);
      } else {
	    obj->cb_value.format = vpiSuppressVal;
      }
      obj->cb_data.value = &obj->cb_value;

      assert(data->obj);
      assert(data->obj->vpi_type);

      switch (data->obj->vpi_type->type_code) {

	  case vpiReg:
	  case vpiNet:
	  case vpiIntegerVar:
	      /* Attach the callback to the vvp_fun_signal node by
		 putting it in the vpi_callbacks list. */
	    struct __vpiSignal*sig;
	    sig = reinterpret_cast<__vpiSignal*>(data->obj);

	    vvp_fun_signal_base*sig_fun;
	    sig_fun = dynamic_cast<vvp_fun_signal_base*>(sig->node->fun);
	    assert(sig_fun);

	      /* Attach the __vpiCallback object to the signal. */
	    sig_fun->add_vpi_callback(obj);
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

	  case vpiMemoryWord:
	    vpip_array_word_change(obj, data->obj);
	    break;

	  case vpiMemory:
	    vpip_array_change(obj, data->obj);
	    break;

	  case vpiPartSelect:
	    vpip_part_select_value_change(obj, data->obj);
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

void sync_cb::run_run()
{
      if (handle == 0)
	    return;

      struct __vpiCallback*cur = handle;
      cur->cb_data.time->type = vpiSimTime;
      vpip_time_to_timestruct(cur->cb_data.time, schedule_simtime());

	/* Run the callback. If the cb_rtn function pointer is set to
	   null, then just skip the whole thing and free it. This is
	   the usual way to cancel one-time callbacks of this sort. */
      if (cur->cb_data.cb_rtn != 0) {
	    assert(vpi_mode_flag == VPI_MODE_NONE);
	    vpi_mode_flag = sync_flag? VPI_MODE_ROSYNC : VPI_MODE_RWSYNC;
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    vpi_mode_flag = VPI_MODE_NONE;
      }

      delete_vpi_callback(cur);
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
      cb->handle = obj;
      obj->cb_sync = cb;

      switch (obj->cb_time.type) {
	  case vpiSuppressTime:
	    schedule_generic(cb, 0, true, readonly_flag);
	    break;

	  case vpiSimTime:
	      { vvp_time64_t tv = vpip_timestruct_to_time(&obj->cb_time);
		vvp_time64_t tn = schedule_simtime();
		if (tv < tn) {
		      schedule_generic(cb, 0, true, readonly_flag);
		} else {
		      schedule_generic(cb, tv - tn, true, readonly_flag);
		}
		break;
	      }

	  default:
	    fprintf(stderr, "Unsupported time type %d.\n",
	            (int)obj->cb_time.type);
	    assert(0);
	    break;
      }
      return obj;
}

static struct __vpiCallback* make_afterdelay(p_cb_data data, bool simtime_flag)
{
      struct __vpiCallback*obj = new_vpi_callback();
      obj->cb_data = *data;
      assert(data->time);
      obj->cb_time = *(data->time);
      obj->cb_data.time = &obj->cb_time;

      obj->next = 0;

      struct sync_cb*cb = new sync_cb;
      cb->sync_flag = false;
      cb->handle = obj;
      obj->cb_sync = cb;

      vvp_time64_t tv;
      switch (obj->cb_time.type) {
	  case vpiSimTime:
	    tv = vpip_timestruct_to_time(&obj->cb_time);
	    break;

	  default:
	    fprintf(stderr, "Unsupported time type %d.\n",
	            (int)obj->cb_time.type);
	    assert(0);
	    tv = 0;
	    break;
      }

      if (simtime_flag) {
	    vvp_time64_t cur = schedule_simtime();
	    if (cur > tv) {
		  tv = 0;
		  assert(0);
	    } else if (cur == tv) {
		  tv = 0;
	    } else {
		  tv -= cur;
	    }
	    schedule_at_start_of_simtime(cb, tv);
      } else {
	    schedule_generic(cb, tv, false);
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

void vpiEndOfCompile(void) {
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
	    delete_vpi_callback(cur);
      }

      vpi_mode_flag = VPI_MODE_NONE;
}

void vpiStartOfSim(void) {
      struct __vpiCallback* cur;

      /*
       * Walk the list of register callbacks, executing them and
       * freeing them when done.
       */
      assert(vpi_mode_flag == VPI_MODE_NONE);
      vpi_mode_flag = VPI_MODE_RWSYNC;

      while (StartOfSimulation) {
	    cur = StartOfSimulation;
	    StartOfSimulation = cur->next;
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    delete_vpi_callback(cur);
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
	      /* Only set the time if it is not NULL. */
	    if (cur->cb_data.time)
	          vpip_time_to_timestruct(cur->cb_data.time, schedule_simtime());
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    delete_vpi_callback(cur);
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
	    delete_vpi_callback(cur);
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

	  case cbAtStartOfSimTime:
	    obj = make_afterdelay(data, true);
	    break;

	  case cbAfterDelay:
	    obj = make_afterdelay(data, false);
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
		    (int)data->reason);
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
      switch (cur->cb_data.time->type) {
	  case vpiSimTime:
	    vpip_time_to_timestruct(cur->cb_data.time, schedule_simtime());
	    break;
	  case vpiScaledRealTime: {
	    cur->cb_data.time->real =
	         vpip_time_to_scaled_real(schedule_simtime(),
	             (struct __vpiScope *) vpi_handle(vpiScope,
	                                              cur->cb_data.obj));
	    break;
	  }
	  case vpiSuppressTime:
	    break;
	  default:
	    fprintf(stderr, "Unsupported time format %d.\n",
	            (int)cur->cb_data.time->type);
	    assert(0);
	    break;
      }
      (cur->cb_data.cb_rtn)(&cur->cb_data);

      vpi_mode_flag = save_mode;
}

vvp_vpi_callback::vvp_vpi_callback()
{
      vpi_callbacks_ = 0;
}

vvp_vpi_callback::~vvp_vpi_callback()
{
      assert(vpi_callbacks_ == 0);
}

void vvp_vpi_callback::add_vpi_callback(__vpiCallback*cb)
{
      cb->next = vpi_callbacks_;
      vpi_callbacks_ = cb;
}

#ifdef CHECK_WITH_VALGRIND
void vvp_vpi_callback::clear_all_callbacks()
{
      while (vpi_callbacks_) {
	    struct __vpiCallback*tmp = vpi_callbacks_->next;
	    delete_vpi_callback(vpi_callbacks_);
	    vpi_callbacks_ = tmp;
      }
}
#endif

/*
 * A vvp_fun_signal uses this method to run its callbacks whenever it
 * has a value change. If the cb_rtn is non-nil, then call the
 * callback function. If the cb_rtn pointer is nil, then the object
 * has been marked for deletion. Free it.
 */
void vvp_vpi_callback::run_vpi_callbacks()
{
      struct __vpiCallback *next = vpi_callbacks_;
      struct __vpiCallback *prev = 0;

      while (next) {
	    struct __vpiCallback*cur = next;
	    next = cur->next;

	    if (cur->cb_data.cb_rtn != 0) {
		  if (cur->cb_data.value)
			get_value(cur->cb_data.value);

		  callback_execute(cur);
		  prev = cur;

	    } else if (prev == 0) {

		  vpi_callbacks_ = next;
		  cur->next = 0;
		  delete_vpi_callback(cur);

	    } else {
		  assert(prev->next == cur);
		  prev->next = next;
		  cur->next = 0;
		  delete_vpi_callback(cur);
	    }
      }
}

vvp_vpi_callback_wordable::vvp_vpi_callback_wordable()
{
      array_ = 0;
      array_word_ = 0;
}

vvp_vpi_callback_wordable::~vvp_vpi_callback_wordable()
{
      assert(array_ == 0);
}

void vvp_vpi_callback_wordable::run_vpi_callbacks()
{
      if (array_) array_word_change(array_, array_word_);

      vvp_vpi_callback::run_vpi_callbacks();
}

void vvp_vpi_callback_wordable::attach_as_word(vvp_array_t arr, unsigned long addr)
{
      assert(array_ == 0);
      array_ = arr;
      array_word_ = addr;
}

void vvp_fun_signal4::get_value(struct t_vpi_value*vp)
{
      switch (vp->format) {
	  case vpiScalarVal:
	    // This works because vvp_bit4_t has the same encoding
	    // as a scalar value! See vpip_vec4_get_value() for a
	    // more robust method.
	    vp->value.scalar = value(0);
	    break;

	  case vpiBinStrVal:
	  case vpiOctStrVal:
	  case vpiDecStrVal:
	  case vpiHexStrVal:
	  case vpiIntVal:
	  case vpiVectorVal:
	  case vpiStringVal:
	  case vpiRealVal: {
	    unsigned wid = size();
	    vvp_vector4_t vec4(wid);
	    for (unsigned idx = 0; idx < wid; idx += 1) {
		  vec4.set_bit(idx, value(idx));
	    }
	    vpip_vec4_get_value(vec4, wid, false, vp);
	    break;
	  }

	  case vpiSuppressVal:
	    break;

	  default:
	    fprintf(stderr, "vpi_callback: value "
		    "format %d not supported (fun_signal)\n",
		    (int)vp->format);
      }
}

void vvp_fun_signal8::get_value(struct t_vpi_value*vp)
{
      switch (vp->format) {
	  case vpiScalarVal:
	    vp->value.scalar = value(0);
	    break;
	  case vpiSuppressVal:
	    break;
	  default:
	    fprintf(stderr, "vpi_callback: value "
		    "format %d not supported (fun_signal8)\n",
		    (int)vp->format);
      }
}

void vvp_fun_signal_real::get_value(struct t_vpi_value*vp)
{
      char*rbuf = need_result_buf(64 + 1, RBUF_VAL);

      switch (vp->format) {
	  case vpiObjTypeVal:
	    vp->format = vpiRealVal;

	  case vpiRealVal:
	    vp->value.real = real_value();
	    break;

	  case vpiIntVal:
	    vp->value.integer = (int)(real_value() + 0.5);
	    break;

	  case vpiDecStrVal:
	    sprintf(rbuf, "%0.0f", real_value());
	    vp->value.str = rbuf;
	    break;

	  case vpiHexStrVal:
	    sprintf(rbuf, "%lx", (long)real_value());
	    vp->value.str = rbuf;
	    break;

	  case vpiBinStrVal: {
		unsigned long val = (unsigned long)real_value();
		unsigned len = 0;

		while (val > 0) {
		      len += 1;
		      val /= 2;
		}

		val = (unsigned long)real_value();
		for (unsigned idx = 0 ;  idx < len ;  idx += 1) {
		      rbuf[len-idx-1] = (val & 1)? '1' : '0';
		      val /= 2;
		}

		rbuf[len] = 0;
		if (len == 0) {
		      rbuf[0] = '0';
		      rbuf[1] = 0;
		}
		vp->value.str = rbuf;
		break;
	  }

	  case vpiSuppressVal:
	    break;

	  default:
	    fprintf(stderr, "vpi_callback: value "
		    "format %d not supported (fun_signal_real)\n",
		    (int)vp->format);
      }
}
