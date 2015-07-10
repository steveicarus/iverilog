/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
# include  "vvp_net_sig.h"
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
#include  "vvp_cleanup.h"
#endif
# include  <cstdio>
# include  <cassert>
# include  <cstdlib>
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

class sync_callback;

struct sync_cb  : public vvp_gen_event_s {
      sync_callback*handle;
      bool sync_flag;

      ~sync_cb () { }

      virtual void run_run();
};

inline __vpiCallback::__vpiCallback()
{
      next = 0;
}

__vpiCallback::~__vpiCallback()
{
}

int __vpiCallback::get_type_code(void) const
{ return vpiCallback; }


value_callback::value_callback(p_cb_data data)
{
      cb_data = *data;
      if (data->time) {
	    cb_time = *(data->time);
      } else {
	    cb_time.type = vpiSuppressTime;
      }
      cb_data.time = &cb_time;
      if (data->value) {
	    cb_value = *(data->value);
      } else {
	    cb_value.format = vpiSuppressVal;
      }
      cb_data.value = &cb_value;
}

/*
 * Normally, any assign to a value triggers a value change callback,
 * so return a constant true here. This is a stub.
 */
bool value_callback::test_value_callback_ready(void)
{
      return true;
}

static void vpip_real_value_change(value_callback*cbh, vpiHandle ref)
{
      struct __vpiRealVar*rfp = dynamic_cast<__vpiRealVar*>(ref);
      assert(rfp);
      vvp_vpi_callback*obj = dynamic_cast<vvp_vpi_callback*>(rfp->net->fil);
      assert(obj);

      obj->add_vpi_callback(cbh);
}

class value_part_callback : public value_callback {
    public:
      explicit value_part_callback(p_cb_data data);
      ~value_part_callback();

      bool test_value_callback_ready(void);

    private:
      char*value_bits_;
      size_t value_off_;
};

inline value_part_callback::value_part_callback(p_cb_data data)
: value_callback(data)
{
      struct __vpiPV*pobj = dynamic_cast<__vpiPV*>(data->obj);
      assert(pobj);

      vvp_vpi_callback*sig_fil;
      sig_fil = dynamic_cast<vvp_vpi_callback*>(pobj->net->fil);
      assert(sig_fil);

      sig_fil->add_vpi_callback(this);
	// Get a reference value that can be used to compare with an
	// updated value. Use the filter get_value to get the value,
	// and get it in BinStr form so that compares are easy. Note
	// that the vpiBinStr format has the MSB first, but the tbase
	// is lsb first.
      s_vpi_value tmp_value;
      tmp_value.format = vpiBinStrVal;
      sig_fil->get_value(&tmp_value);

      value_bits_ = new char[pobj->width+1];
      value_off_ = pobj->parent->vpi_get(vpiSize) - pobj->width - pobj->tbase;

      memcpy(value_bits_, tmp_value.value.str + value_off_, pobj->width);
      value_bits_[pobj->width] = 0;
}

value_part_callback::~value_part_callback()
{
      delete[]value_bits_;
}

bool value_part_callback::test_value_callback_ready(void)
{
      struct __vpiPV*pobj = dynamic_cast<__vpiPV*>(cb_data.obj);
      assert(pobj);

      vvp_vpi_callback*sig_fil;
      sig_fil = dynamic_cast<vvp_vpi_callback*>(pobj->net->fil);
      assert(sig_fil);

	// Get a reference value that can be used to compare with an
	// updated value.
      s_vpi_value tmp_value;
      tmp_value.format = vpiBinStrVal;
      sig_fil->get_value(&tmp_value);

      if (memcmp(value_bits_, tmp_value.value.str + value_off_, pobj->width) == 0)
	    return false;

      memcpy(value_bits_, tmp_value.value.str + value_off_, pobj->width);
      return true;
}

/*
 * Attach the __vpiCallback to the object that this part select
 * selects from. The part select itself is not a vvp_vpi_callback
 * object, but it refers to a net that is a vvp_vpi_callback, so
 * add the callback to that object.
 */
static value_callback*make_value_change_part(p_cb_data data)
{
	/* Attach the __vpiCallback object to the signal. */
      value_callback*cbh = new value_part_callback(data);
      return cbh;
}

/*
 * A value change callback is tripped when a bit of a signal
 * changes. This function creates that value change callback and
 * attaches it to the relevant vpiSignal object. Also, if the signal
 * does not already have them, create some callback functors to do the
 * actual value change detection.
 */
static value_callback* make_value_change(p_cb_data data)
{
      if (vpi_get(vpiAutomatic, data->obj)) {
            fprintf(stderr, "vpi error: cannot place value change "
                            "callback on automatically allocated "
                            "variable '%s'\n",
                            vpi_get_str(vpiName, data->obj));
            return 0;
      }

	// Special case: the target object is a vpiPartSelect
      if (data->obj->get_type_code() == vpiPartSelect)
	    return make_value_change_part(data);

      if (data->obj->get_type_code() == vpiMemoryWord)
	    return vpip_array_word_change(data);

      if (data->obj->get_type_code() == vpiMemory)
	    return vpip_array_change(data);

      value_callback*obj = new value_callback(data);

      assert(data->obj);
      switch (data->obj->get_type_code()) {

	  case vpiReg:
	  case vpiNet:
	  case vpiIntegerVar:
	  case vpiBitVar:
	  case vpiByteVar:
	  case vpiShortIntVar:
	  case vpiIntVar:
	  case vpiLongIntVar:
	      /* Attach the callback to the vvp_fun_signal node by
		 putting it in the vpi_callbacks list. */
	    struct __vpiSignal*sig;
	    sig = dynamic_cast<__vpiSignal*>(data->obj);

	    vvp_net_fil_t*sig_fil;
	    sig_fil = dynamic_cast<vvp_net_fil_t*>(sig->node->fil);
	    assert(sig_fil);

	      /* Attach the __vpiCallback object to the signal. */
	    sig_fil->add_vpi_callback(obj);
	    break;

	  case vpiRealVar:
	    vpip_real_value_change(obj, data->obj);
	    break;

	  case vpiNamedEvent:
	    __vpiNamedEvent*nev;
	    nev = dynamic_cast<__vpiNamedEvent*>(data->obj);
	    nev->add_vpi_callback(obj);
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
		    data->obj->get_type_code());
	    delete obj;
	    return 0;
      }

      return obj;
}

class sync_callback : public __vpiCallback {
    public:
      explicit sync_callback(p_cb_data data);
      ~sync_callback();

    public:
	// scheduled event
      struct sync_cb* cb_sync;
	// user supplied callback data
      struct t_vpi_time cb_time;

    private:
};

inline sync_callback::sync_callback(p_cb_data data)
{
      cb_sync = 0;

      cb_data = *data;
      assert(data->time);
      cb_time = *(data->time);
      cb_data.time = &cb_time;
}

sync_callback::~sync_callback()
{
      delete cb_sync;
}

void sync_cb::run_run()
{
      if (handle == 0)
	    return;

      sync_callback*cur = handle;
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

      delete cur;
}

static sync_callback* make_sync(p_cb_data data, bool readonly_flag)
{
      sync_callback*obj = new sync_callback(data);

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
      sync_callback*obj = new sync_callback(data);
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

class simulator_callback : public __vpiCallback {
    public:
      inline explicit simulator_callback(struct t_cb_data*data)
      { cb_data = *data; }

    public:
};

static simulator_callback*NextSimTime = 0;
static simulator_callback*EndOfCompile = 0;
static simulator_callback*StartOfSimulation = 0;
static simulator_callback*EndOfSimulation = 0;

#ifdef CHECK_WITH_VALGRIND
/* This is really only needed if the simulator aborts before starting the
 * main event loop. For that reason we can skip the next sim time queue. */
void simulator_cb_delete(void)
{
      simulator_callback* cur;

	/* Delete all the end of compile callbacks. */
      while (EndOfCompile) {
	    cur = EndOfCompile;
	    EndOfCompile = dynamic_cast<simulator_callback*>(cur->next);
	    delete cur;
      }

	/* Delete all the start of simulation callbacks. */
      while (StartOfSimulation) {
	    cur = StartOfSimulation;
	    StartOfSimulation = dynamic_cast<simulator_callback*>(cur->next);
	    delete cur;
      }

	/* Delete all the end of simulation callbacks. */
      while (EndOfSimulation) {
	    cur = EndOfSimulation;
	    EndOfSimulation = dynamic_cast<simulator_callback*>(cur->next);
	    delete cur;
      }
}
#endif

void vpiEndOfCompile(void) {
      simulator_callback* cur;

      /*
       * Walk the list of register callbacks, executing them and
       * freeing them when done.
       */
      assert(vpi_mode_flag == VPI_MODE_NONE);
      vpi_mode_flag = VPI_MODE_RWSYNC;

      while (EndOfCompile) {
	    cur = EndOfCompile;
	    EndOfCompile = dynamic_cast<simulator_callback*>(cur->next);
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    delete cur;
      }

      vpi_mode_flag = VPI_MODE_NONE;
}

void vpiStartOfSim(void) {
      simulator_callback* cur;

      /*
       * Walk the list of register callbacks, executing them and
       * freeing them when done.
       */
      assert(vpi_mode_flag == VPI_MODE_NONE);
      vpi_mode_flag = VPI_MODE_RWSYNC;

      while (StartOfSimulation) {
	    cur = StartOfSimulation;
	    StartOfSimulation = dynamic_cast<simulator_callback*>(cur->next);
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    delete cur;
      }

      vpi_mode_flag = VPI_MODE_NONE;
}

void vpiPostsim(void) {
      simulator_callback* cur;

      /*
       * Walk the list of register callbacks
       */
      assert(vpi_mode_flag == VPI_MODE_NONE);
      vpi_mode_flag = VPI_MODE_ROSYNC;

      while (EndOfSimulation) {
	    cur = EndOfSimulation;
	    EndOfSimulation = dynamic_cast<simulator_callback*>(cur->next);
	      /* Only set the time if it is not NULL. */
	    if (cur->cb_data.time)
	          vpip_time_to_timestruct(cur->cb_data.time, schedule_simtime());
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    delete cur;
      }

      vpi_mode_flag = VPI_MODE_NONE;
}

/*
 * The scheduler invokes this to clear out callbacks for the next
 * simulation time.
 */
void vpiNextSimTime(void)
{
      simulator_callback* cur;

      assert(vpi_mode_flag == VPI_MODE_NONE);
      vpi_mode_flag = VPI_MODE_RWSYNC;

      while (NextSimTime) {
	    cur = NextSimTime;
	    NextSimTime = dynamic_cast<simulator_callback*>(cur->next);
	    (cur->cb_data.cb_rtn)(&cur->cb_data);
	    delete cur;
      }

      vpi_mode_flag = VPI_MODE_NONE;
}

static simulator_callback* make_prepost(p_cb_data data)
{
      simulator_callback*obj = new simulator_callback(data);

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

      return obj;
}

/*
 * Removing a callback doesn't really delete it right away. Instead,
 * it clears the reference to the user callback function. This causes
 * the callback to quietly reap itself.
 */
PLI_INT32 vpi_remove_cb(vpiHandle ref)
{
      struct __vpiCallback*obj = dynamic_cast<__vpiCallback*>(ref);
      assert(obj);
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
      array_ = 0;
      array_word_ = 0;
}

vvp_vpi_callback::~vvp_vpi_callback()
{
      assert(vpi_callbacks_ == 0);
      assert(array_ == 0);
}

void vvp_vpi_callback::attach_as_word(vvp_array_t arr, unsigned long addr)
{
      assert(array_ == 0);
      array_ = arr;
      array_word_ = addr;
}

void vvp_vpi_callback::add_vpi_callback(value_callback*cb)
{
      cb->next = vpi_callbacks_;
      vpi_callbacks_ = cb;
}

#ifdef CHECK_WITH_VALGRIND
void vvp_vpi_callback::clear_all_callbacks()
{
      while (vpi_callbacks_) {
	    value_callback *tmp = dynamic_cast<value_callback*>
	                            (vpi_callbacks_->next);
	    delete vpi_callbacks_;
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
      if (array_) array_->word_change(array_word_);

      value_callback *next = vpi_callbacks_;
      value_callback *prev = 0;

      while (next) {
	    value_callback*cur = next;
	    next = dynamic_cast<value_callback*>(cur->next);

	    if (cur->cb_data.cb_rtn != 0) {
		  if (cur->test_value_callback_ready()) {
			if (cur->cb_data.value)
			      get_value(cur->cb_data.value);

			callback_execute(cur);
		  }
		  prev = cur;

	    } else if (prev == 0) {

		  vpi_callbacks_ = next;
		  cur->next = 0;
		  delete cur;

	    } else {
		  assert(prev->next == cur);
		  prev->next = next;
		  cur->next = 0;
		  delete cur;
	    }
      }
}

void vvp_signal_value::get_signal_value(struct t_vpi_value*vp)
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
	    unsigned wid = value_size();
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

static void real_signal_value(struct t_vpi_value*vp, double rval)
{
      char*rbuf = (char *) need_result_buf(64 + 1, RBUF_VAL);

      switch (vp->format) {
	  case vpiObjTypeVal:
	    vp->format = vpiRealVal;

	  case vpiRealVal:
	    vp->value.real = rval;
	    break;

	  case vpiIntVal:
	      /* NaN or +/- infinity are translated as 0. */
	    if (rval != rval || (rval && (rval == 0.5*rval))) {
		  rval = 0.0;
	    } else {
		  if (rval >= 0.0) rval = floor(rval + 0.5);
		  else rval = ceil(rval - 0.5);
	    }
	    vp->value.integer = (PLI_INT32)rval;
	    break;

	  case vpiDecStrVal:
#if !defined(__GNUC__)
		  if (isnan(rval))
			  sprintf(rbuf, "%s", "nan");
		  else
			  sprintf(rbuf, "%0.0f", rval);
#else
	    sprintf(rbuf, "%0.0f", rval);
#endif
	    vp->value.str = rbuf;
	    break;

	  case vpiHexStrVal:
	    sprintf(rbuf, "%lx", (long)rval);
	    vp->value.str = rbuf;
	    break;

	  case vpiBinStrVal: {
		unsigned long val = (unsigned long)rval;
		unsigned len = 0;

		while (val > 0) {
		      len += 1;
		      val /= 2;
		}

		val = (unsigned long)rval;
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

void vvp_fun_signal_real_aa::get_signal_value(struct t_vpi_value*vp)
{
      real_signal_value(vp, real_value());
}

void vvp_wire_real::get_signal_value(struct t_vpi_value*vp)
{
      real_signal_value(vp, real_value());
}

void vvp_fun_signal_string_aa::get_signal_value(struct t_vpi_value*)
{
      assert(0);
}
#if 0
void vvp_wire_string::get_signal_value(struct t_vpi_value*vp)
{
      assert(0);
}
#endif
void vvp_wire_vec4::get_value(struct t_vpi_value*val)
{
      get_signal_value(val);
}

void vvp_wire_vec8::get_value(struct t_vpi_value*val)
{
      get_signal_value(val);
}

void vvp_wire_real::get_value(struct t_vpi_value*val)
{
      get_signal_value(val);
}
#if 0
void vvp_wire_string::get_value(struct t_vpi_value*val)
{
      assert(0);
}
#endif
