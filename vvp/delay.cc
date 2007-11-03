/*
 * Copyright (c) 2005-2007 Stephen Williams <steve@icarus.com>
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

#include "delay.h"
#include "schedule.h"
#include "vpi_priv.h"
#include <iostream>
#include <assert.h>

void vvp_delay_t::calculate_min_delay_()
{
      min_delay_ = rise_;
      if (fall_ < min_delay_)
	    min_delay_ = fall_;
      if (decay_ < min_delay_)
	    min_delay_ = decay_;
}

vvp_delay_t::vvp_delay_t(vvp_time64_t rise, vvp_time64_t fall)
{
      rise_ = rise;
      fall_ = fall;
      decay_= fall < rise? fall : rise;
      min_delay_ = decay_;
}

vvp_delay_t::vvp_delay_t(vvp_time64_t rise, vvp_time64_t fall, vvp_time64_t decay)
{
      rise_ = rise;
      fall_ = fall;
      decay_= decay;

      calculate_min_delay_();
}

vvp_delay_t::~vvp_delay_t()
{
}

vvp_time64_t vvp_delay_t::get_delay(vvp_bit4_t from, vvp_bit4_t to)
{
      switch (from) {
	  case BIT4_0:
	    switch (to) {
		case BIT4_0: return 0;
		case BIT4_1: return rise_;
		case BIT4_X: return min_delay_;
		case BIT4_Z: return decay_;
	    }
	    break;
	  case BIT4_1:
	    switch (to) {
		case BIT4_0: return fall_;
		case BIT4_1: return 0;
		case BIT4_X: return min_delay_;
		case BIT4_Z: return decay_;
	    }
	    break;
	  case BIT4_X:
	    switch (to) {
		case BIT4_0: return fall_;
		case BIT4_1: return rise_;
		case BIT4_X: return 0;
		case BIT4_Z: return decay_;
	    }
	    break;
	  case BIT4_Z:
	    switch (to) {
		case BIT4_0: return fall_;
		case BIT4_1: return rise_;
		case BIT4_X: return min_delay_;
		case BIT4_Z: return 0;
	    }
	    break;
      }

      assert(0);
      return 0;
}

vvp_time64_t vvp_delay_t::get_min_delay() const
{
      return min_delay_;
}

void vvp_delay_t::set_rise(vvp_time64_t val)
{
      rise_ = val;
      if (val < min_delay_)
	    min_delay_ = val;
      else
	    calculate_min_delay_();
}

void vvp_delay_t::set_fall(vvp_time64_t val)
{
      fall_ = val;
      if (val < min_delay_)
	    min_delay_ = val;
      else
	    calculate_min_delay_();
}

void vvp_delay_t::set_decay(vvp_time64_t val)
{
      decay_ = val;
      if (val < min_delay_)
	    min_delay_ = val;
      else
	    calculate_min_delay_();
}

vvp_fun_delay::vvp_fun_delay(vvp_net_t*n, vvp_bit4_t init, const vvp_delay_t&d)
: net_(n), delay_(d), cur_vec4_(1)
{
      cur_vec4_.set_bit(0, init);
      list_ = 0;
}

vvp_fun_delay::~vvp_fun_delay()
{
      while (struct event_*cur = dequeue_())
	    delete cur;
}

void vvp_fun_delay::clean_pulse_events_(vvp_time64_t use_delay)
{
      if (list_ == 0)
	    return;

      do {
	    struct event_*cur = list_->next;
	      /* If this event is far enough from the event I'm about
	         to create, then that scheduled event is not a pulse
	         to be eliminated, so we're done. */
	    if (cur->sim_time+use_delay <= use_delay+schedule_simtime())
		  break;

	    if (list_ == cur)
		  list_ = 0;
	    else
		  list_->next = cur->next;
	    delete cur;
      } while (list_);
}

/*
 * FIXME: this implementation currently only uses the maximum delay
 * from all the bit changes in the vectors. If there are multiple
 * changes with different delays, then the results would be
 * wrong. What should happen is that if there are multiple changes,
 * multiple vectors approaching the result should be scheduled.
 */
void vvp_fun_delay::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      if (port.port() > 0) {
	      // Get the integer value of the bit vector, or 0 if
	      // there are X or Z bits.
	    unsigned long val = 0;
	    vector4_to_value(bit, val);

	    switch (port.port()) {
		case 1:
		  delay_.set_rise(val);
		  return;
		case 2:
		  delay_.set_fall(val);
		  return;
		case 3:
		  delay_.set_decay(val);
		  return;
	    }
	    return;
      }

	/* How many bits to compare? */
      unsigned use_wid = cur_vec4_.size();
      if (bit.size() < use_wid)
	    use_wid = bit.size();

	/* Scan the vectors looking for delays. Select the maximim
	   delay encountered. */
      vvp_time64_t use_delay;
      use_delay = delay_.get_delay(cur_vec4_.value(0), bit.value(0));

      for (unsigned idx = 1 ;  idx < use_wid ;  idx += 1) {
	    vvp_time64_t tmp;
	    tmp = delay_.get_delay(cur_vec4_.value(idx), bit.value(idx));
	    if (tmp > use_delay)
		  use_delay = tmp;
      }

      /* what *should* happen here is we check to see if there is a
         transaction in the queue. This would be a pulse that needs to be
         eliminated. */
      clean_pulse_events_(use_delay);

      vvp_time64_t use_simtime = schedule_simtime() + use_delay;

	/* And propagate it. */
      if (use_delay == 0) {
	    cur_vec4_ = bit;
	    vvp_send_vec4(net_->out, cur_vec4_);
      } else {
	    struct event_*cur = new struct event_(use_simtime);
	    cur->run_run_ptr = &vvp_fun_delay::run_run_vec4_;
	    cur->ptr_vec4 = bit;
	    enqueue_(cur);
	    schedule_generic(this, use_delay, false);
      }
}

void vvp_fun_delay::recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit)
{
      assert(port.port() == 0);

      if (cur_vec8_.eeq(bit))
	    return;

	/* XXXX FIXME: For now, just use the minimum delay. */
      vvp_time64_t use_delay;
      use_delay = delay_.get_min_delay();

      vvp_time64_t use_simtime = schedule_simtime() + use_delay;
      if (use_delay == 0) {
	    cur_vec8_ = bit;
	    vvp_send_vec8(net_->out, cur_vec8_);
      } else {
	    struct event_*cur = new struct event_(use_simtime);
	    cur->ptr_vec8 = bit;
	    cur->run_run_ptr = &vvp_fun_delay::run_run_vec8_;
	    enqueue_(cur);
	    schedule_generic(this, use_delay, false);
      }
}

void vvp_fun_delay::recv_real(vvp_net_ptr_t port, double bit)
{
      if (port.port() > 0) {
	    /* If the port is not 0, then this is a delay value that
	    should be rounded and converted to an integer delay. */
	    unsigned long long val = 0;
	    if (bit > 0)
		  val = (unsigned long long) (bit+0.5);

	    switch (port.port()) {
		case 1:
		  delay_.set_rise(val);
		  return;
		case 2:
		  delay_.set_fall(val);
		  return;
		case 3:
		  delay_.set_decay(val);
		  return;
	    }
	    return;
      }

      if (cur_real_ == bit)
	    return;

      vvp_time64_t use_delay;
      use_delay = delay_.get_min_delay();

      vvp_time64_t use_simtime = schedule_simtime() + use_delay;

      if (use_delay == 0) {
	    cur_real_ = bit;
	    vvp_send_real(net_->out, cur_real_);
      } else {
	    struct event_*cur = new struct event_(use_simtime);
	    cur->run_run_ptr = &vvp_fun_delay::run_run_real_;
	    cur->ptr_real = bit;
	    enqueue_(cur);

	    schedule_generic(this, use_delay, false);
      }
}

void vvp_fun_delay::run_run()
{
      vvp_time64_t sim_time = schedule_simtime();
      if (list_ == 0 || list_->next->sim_time > sim_time)
	    return;

      struct event_*cur = dequeue_();
      if (cur == 0)
	    return;

      (this->*(cur->run_run_ptr))(cur);
      delete cur;
}

void vvp_fun_delay::run_run_vec4_(struct event_*cur)
{
      cur_vec4_ = cur->ptr_vec4;
      vvp_send_vec4(net_->out, cur_vec4_);
}

void vvp_fun_delay::run_run_vec8_(struct vvp_fun_delay::event_*cur)
{
      cur_vec8_ = cur->ptr_vec8;
      vvp_send_vec8(net_->out, cur_vec8_);
}

void vvp_fun_delay::run_run_real_(struct vvp_fun_delay::event_*cur)
{
      cur_real_ = cur->ptr_real;
      vvp_send_real(net_->out, cur_real_);
}

vvp_fun_modpath::vvp_fun_modpath(vvp_net_t*net)
: net_(net), src_list_(0)
{
}

vvp_fun_modpath::~vvp_fun_modpath()
{
	// Delete the source probes.
      while (src_list_) {
	    vvp_fun_modpath_src*tmp = src_list_;
	    src_list_ = tmp->next_;
	    delete tmp;
      }
}

void vvp_fun_modpath::add_modpath_src(vvp_fun_modpath_src*that)
{
      assert(that->next_ == 0);
      that->next_ = src_list_;
      src_list_ = that;
}

static vvp_time64_t delay_from_edge(vvp_bit4_t a, vvp_bit4_t b, vvp_time64_t array[12])
{
      typedef delay_edge_t bit4_table4[4];
      const static bit4_table4 edge_table[4] = {
	    { DELAY_EDGE_01, DELAY_EDGE_01, DELAY_EDGE_0x, DELAY_EDGE_0z },
	    { DELAY_EDGE_10, DELAY_EDGE_10, DELAY_EDGE_1x, DELAY_EDGE_1z },
	    { DELAY_EDGE_x0, DELAY_EDGE_x1, DELAY_EDGE_x0, DELAY_EDGE_xz },
	    { DELAY_EDGE_z0, DELAY_EDGE_z1, DELAY_EDGE_zx, DELAY_EDGE_z0 }
      };

      return array[ edge_table[a][b] ];
}

void vvp_fun_modpath::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
	/* Only the first port is used. */
      if (port.port() > 0)
	    return;

      if (cur_vec4_.eeq(bit))
	    return;

	/* Select a time delay source that applies. */
      vvp_fun_modpath_src*src = 0;
      for (vvp_fun_modpath_src*cur = src_list_ ;  cur ;  cur=cur->next_) {
	      /* Skip paths that are disabled by conditions. */
	    if (cur->condition_flag_ == false)
		  continue;

	    if (src == 0) {
		  src = cur;
	    } else if (cur->wake_time_ > src->wake_time_) {
		  src = cur;
	    } else {
		  continue; /* Skip this entry. */
	    }
      }

      assert(src);

      vvp_time64_t out_at[12];
      vvp_time64_t now = schedule_simtime();
      for (unsigned idx = 0 ;  idx < 12 ;  idx += 1) {
	    out_at[idx] = src->wake_time_ + src->delay_[idx];
	    if (out_at[idx] <= now)
		  out_at[idx] = 0;
	    else
		  out_at[idx] -= now;
      }

	/* Given the scheduled output time, create an output event. */
      vvp_time64_t use_delay = delay_from_edge(cur_vec4_.value(0),
					       bit.value(0),
					       out_at);

	/* FIXME: This bases the edge delay on only the least
	   bit. This is WRONG! I need to find all the possible delays,
	   and schedule an event for each partial change. Hard! */
      for (unsigned idx = 1 ;  idx < bit.size() ;  idx += 1) {
	    vvp_time64_t tmp = delay_from_edge(cur_vec4_.value(idx),
					       bit.value(0),
					       out_at);
	    assert(tmp == use_delay);
      }

      cur_vec4_ = bit;
      schedule_generic(this, use_delay, false);
}

void vvp_fun_modpath::run_run()
{
      vvp_send_vec4(net_->out, cur_vec4_);
}

vvp_fun_modpath_src::vvp_fun_modpath_src(vvp_time64_t del[12])
{
      for (unsigned idx = 0 ;  idx < 12 ;  idx += 1)
	    {
	      delay_[idx] = del[idx];
	      /*
		Added By Yang.
		
		Make the delay[12] value to be Public
		make the get_delays(), put_delays() to
		be possible
	       */
	      delay [idx] = del[idx];
	    }

      next_ = 0;
      wake_time_ = 0;
      condition_flag_ = true;
}

vvp_fun_modpath_src::~vvp_fun_modpath_src()
{
}

void vvp_fun_modpath_src::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      if (port.port() == 0) {
	      // The modpath input...
	    if (test_vec4(bit))
		  wake_time_ = schedule_simtime();

      } else if (port.port() == 1) {
	      // The modpath condition input...
	    if (bit.value(0) == BIT4_1)
		  condition_flag_ = true;
	    else
		  condition_flag_ = false;
      }
}

bool vvp_fun_modpath_src::test_vec4(const vvp_vector4_t&)
{
      return true;
}

vvp_fun_modpath_edge::vvp_fun_modpath_edge(vvp_time64_t del[12],
					   bool pos, bool neg)
: vvp_fun_modpath_src(del)
{
      old_value_ = BIT4_X;
      posedge_ = pos;
      negedge_ = neg;
}

bool vvp_fun_modpath_edge::test_vec4(const vvp_vector4_t&bit)
{
      vvp_bit4_t tmp = old_value_;
      old_value_ = bit.value(0);

      int edge_flag = edge(tmp, old_value_);
      if (edge_flag > 0) return posedge_;
      if (edge_flag < 0) return negedge_;
      return false;
}


/*
 * All the below routines that begin with
 * modpath_src_* belong the internal function
 * of an vpiModPathIn object. This is used to
 * make some specific delays path operations
 *
 */
static int modpath_src_get(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code == vpiModPath));
      return 0 ;
}

/*
 * This routine will return an modpathIn input port
 * name for an modpath vpiHandle object
 *
 */
static char* modpath_src_get_str(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code == vpiModPathIn));
      struct __vpiModPathSrc *refp = (struct __vpiModPathSrc *)ref;
      char *bn   = strdup(vpi_get_str(vpiFullName, &refp->scope->base));
      
      char *nm   = (char *)refp->name ;
      
      char *rbuf = need_result_buf(strlen(bn) + strlen(nm) + 2, RBUF_STR);
      
      switch (code)
	{
	case vpiFullName:
	  sprintf(rbuf, "%s.%s", bn, nm);
	  free(bn);
	  return rbuf;
	  
	case vpiName:
	  strcpy(rbuf, nm);
	  free(bn);
	  return rbuf;
	}
      
      free(bn);
      return 0;
}

static void modpath_src_get_value(vpiHandle ref, p_vpi_value vp)
{
      assert((ref->vpi_type->type_code == vpiModPathIn));
      struct __vpiModPathSrc* modpathsrc = vpip_modpath_src_from_handle( ref) ;
      assert ( modpathsrc ) ;
      return  ;
}

static vpiHandle modpath_src_put_value(vpiHandle ref, s_vpi_value *vp )
{
      assert((ref->vpi_type->type_code == vpiModPathIn));
      struct __vpiModPathSrc* modpathsrc = vpip_modpath_src_from_handle( ref) ;
      assert ( modpathsrc ) ;
      return 0 ;
}

static vpiHandle modpath_src_get_handle(int code, vpiHandle ref)
{
      assert( (ref->vpi_type->type_code==vpiModPathIn ) );
      struct __vpiModPathSrc *rfp = (struct __vpiModPathSrc *)ref ;
      
      switch (code)
	{
	case vpiScope:
	  return &rfp->scope->base ;
	  
	case vpiModule:
	  {
	    struct __vpiScope*scope = rfp->scope;
	    while (scope && scope->base.vpi_type->type_code != vpiModule)
	      scope = scope->scope;
	    
	    assert(scope);
	    return &scope->base;
	  }
	}
      return 0;
}

static vpiHandle modpath_src_index ( vpiHandle ref, int code  )
{
      assert( (ref->vpi_type->type_code == vpiModPathIn ) );
      return 0 ;
}


static int modpath_src_free_object( vpiHandle ref )
{
      assert( (ref->vpi_type->type_code == vpiModPathIn ) );
      free ( ref ) ;
      return 1 ;
}


/*
 * This Routine will put  specific demension of delay[] values
 * into a vpiHandle. In this case, he will put an
 * specific delays values in a vpiModPathIn object
 *
 */
static void modpath_src_put_delays ( vpiHandle ref, p_vpi_delay delays )
{
      int i ;
      assert((ref->vpi_type->type_code == vpiModPathIn));
      
      struct __vpiModPathSrc * src = vpip_modpath_src_from_handle( ref) ;
      assert ( src ) ;
      vvp_fun_modpath_src *fun = dynamic_cast<vvp_fun_modpath_src*>(src->node->fun);
      assert( fun );
      
      for ( i = 0 ; i < delays->no_of_delays ; i++)
	{
	  fun->delay[i] = delays->da[ i ].real ;
	}
}

/*
 * This Routine will retrive the delay[12] values
 * of an vpiHandle. In this case, he will get an
 * specific delays values from an vpiModPathIn 
 * object
 *
 */

static void modpath_src_get_delays ( vpiHandle ref, p_vpi_delay delays )
{
      int i ;
      assert(( ref->vpi_type->type_code == vpiModPathIn ));
    
      struct __vpiModPathSrc * src = vpip_modpath_src_from_handle( ref) ;
      assert ( src ) ;
      vvp_fun_modpath_src *fun = dynamic_cast<vvp_fun_modpath_src*>(src->node->fun);
      assert( fun );
      for ( i = 0 ; i < delays->no_of_delays ; i++)
	delays->da[ i ].real = fun->delay[i];
}

/*
  This Struct will be used  by the make_vpi_modpath_src ( )
  to initializa the vpiModPathIn vpiHandle type, and assign
  method routines
  
  vpiModPathIn vpiHandle interanl operations :

  we have
  
  vpi_get         == modpath_src_get (..) ;
  vpi_get_str     == modpath_src_get_str (..) ;
  vpi_get_value   == modpath_src_get_value (..) ;
  vpi_put_value   == modpath_src_put_value (..) ;
  vpi_get_handle  == modpath_src_get_handle (..) ;
  vpi_iterate     == modpath_src_iterate (..) ;
  vpi_index       == modpath_src_index ( .. ) ;
  vpi_free_object == modpath_src_free_object ( .. ) ;
  vpi_get_delay   == modpath_src_get_delay (..) ;
  vpi_put_delay   == modpath_src_put_delay (..) ;


*/

static const struct __vpirt vpip_modpath_src = {
     vpiModPathIn,
     modpath_src_get,
     modpath_src_get_str,
     modpath_src_get_value,
     modpath_src_put_value,
     modpath_src_get_handle,
     0, /* modpath_src_iterate,*/
     modpath_src_index,
     modpath_src_free_object,
     modpath_src_get_delays,
     modpath_src_put_delays
};

/*
 * This function will Constructs a vpiModPathIn 
 * ( struct __vpiModPathSrc ) Object. will give
 * a delays[12] values, and point to the specified functor
 *
 */

vpiHandle vpip_make_modpath_src ( char *name, vvp_time64_t use_delay[12] ,  vvp_net_t *net )
{
     struct __vpiModPathSrc *obj = (struct __vpiModPathSrc *) calloc (1, sizeof ( struct __vpiModPathSrc ) ) ;
     obj->base.vpi_type          = &vpip_modpath_src;
     obj->scope                  = vpip_peek_current_scope ( );
     obj->name = (char *)calloc(strlen(name) + 1 , sizeof ( char )) ;
     strcpy ( obj->name, name ) ;
     obj->node           = net  ;
     vpip_attach_to_current_scope (&obj->base) ;
     return &obj->base ;
}


/*
  vpiModPath vpiHandle interanl operations :

  we have

  vpi_get         == modpath_get (..) ;
  vpi_get_str     == modpath_get_str (..) ;
  vpi_get_value   == modpath_get_value (..) ;
  vpi_put_value   == modpath_put_value (..) ;
  vpi_get_handle  == modpath_get_handle (..) ;
  vpi_iterate     == modpath_iterate (..) ;
  vpi_index       == modpath_index ( .. ) ;
  vpi_free_object == modpath_free_object ( .. ) ;
  vpi_get_delay   == modpath_get_delay (..) ;
  vpi_put_delay   == modpath_put_delay (..) ;

*/

static int modpath_get(int code, vpiHandle ref)
{
  assert((ref->vpi_type->type_code == vpiModPath));

  return 0 ;
}

static char* modpath_get_str(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code == vpiModPath));

      struct __vpiModPath *refp = (struct __vpiModPath *)ref;
      char *bn   = strdup(vpi_get_str(vpiFullName, &refp->scope->base));
      char *nm   = (char *)refp->name ;
      char *rbuf = need_result_buf(strlen(bn) + strlen(nm) + 2, RBUF_STR);
      
      switch (code)
	{
	case vpiFullName:
	  sprintf(rbuf, "%s.%s", bn, nm);
	  free(bn);
	  return rbuf;
	  
	case vpiName:
	  strcpy(rbuf, nm);
	  free(bn);
	  return rbuf;
	}
      
      free(bn);
      return 0;
}

static void modpath_get_value(vpiHandle ref, p_vpi_value vp)
{
      assert((ref->vpi_type->type_code == vpiModPath));
      // struct __vpiModPath* modpath = vpip_modpath_from_handle( ref) ;
      // assert ( modpath ) ;
      return  ;
}

static vpiHandle modpath_put_value(vpiHandle ref, s_vpi_value *vp )
{
     assert((ref->vpi_type->type_code == vpiModPath));
     // struct __vpiModPath* modpath = vpip_modpath_from_handle( ref) ;
     // assert ( modpath ) ;
     return 0 ;
}

static vpiHandle modpath_get_handle(int code, vpiHandle ref)
{
      assert( (ref->vpi_type->type_code==vpiModPath) );
      struct __vpiModPath *rfp = (struct __vpiModPath *)ref ;

      switch (code)
	{
	case vpiScope:
	  return &rfp->scope->base ;
	  
	case vpiModule:
	  {
	    struct __vpiScope*scope = rfp->scope;
	    while (scope && scope->base.vpi_type->type_code != vpiModule)
	      scope = scope->scope;
	    
	    assert(scope);
	    return &scope->base;
	  }
	}
      return 0;
}


static vpiHandle modpath_iterate (int code ,  vpiHandle ref )
{
      assert( (ref->vpi_type->type_code == vpiModPath) );
      return  0 ;
}

static vpiHandle modpath_index ( vpiHandle ref, int code  )
{
      assert( (ref->vpi_type->type_code == vpiModPath) );
      return 0 ;
}


static int modpath_free_object( vpiHandle ref )
{
      assert( (ref->vpi_type->type_code == vpiModPath) );
      free ( ref ) ;
      return 1 ;
}



/*
  This Struct will be used  by the make_vpi_modpath ( )
  to initializa the vpiModPath vpiHandle type, and assign
  method routines
*/
static const struct __vpirt vpip_modpath_rt = {
      vpiModPath,
      modpath_get,
      modpath_get_str,
      modpath_get_value,
      modpath_put_value,
      modpath_get_handle,
      modpath_iterate,
      modpath_index,
      modpath_free_object,
      0, // modpath_get_delays,
      0  // modpath_put_delays
};

/*
 * This function will Construct a vpiModPath Object.
 * give a respective "net", and will point to his
 * respective functor
 */

vpiHandle vpip_make_modpath ( char *name, char *input,  vvp_net_t *net )
{
  
      struct __vpiModPath   *obj = (struct __vpiModPath *) calloc (1, sizeof ( struct __vpiModPath ) ) ;
      obj->base.vpi_type         = &vpip_modpath_rt ;
      obj->scope                 = vpip_peek_current_scope ( );

      obj->name = (char *)calloc(strlen(name) + 1 , sizeof ( char )) ;
      strcpy ( obj->name, name ) ;

      obj->input = (char *)calloc(strlen(input) + 1 , sizeof ( char )) ;
      strcpy ( obj->input,input ) ;

      obj->input_net           = net ;
      vpip_attach_to_current_scope (&obj->base) ;
      return &obj->base ;
}


/*
  this Routine will safetly convert a modpath vpiHandle
  to a struct __vpiModPath { }
*/

struct __vpiModPath* vpip_modpath_from_handle(vpiHandle ref)
{
      if (ref->vpi_type->type_code != vpiModPath)
         return 0;

      return (struct __vpiModPath *) ref;
}

/*
  this Routine will safetly convert a modpathsrc vpiHandle
  to a struct __vpiModPathSrc { }, This is equivalent ao
  vpiModPathIn handle
*/

struct __vpiModPathSrc* vpip_modpath_src_from_handle(vpiHandle ref)
{
      if (ref->vpi_type->type_code != vpiModPathIn)
        return 0;
      
      return (struct __vpiModPathSrc *) ref;
}




void  vpip_add_mopdath_edge ( vpiHandle vpiobj, char  *label,
			      vvp_time64_t use_delay[12] ,
                              bool posedge , bool negedge )
{
  // printf(" In the vpip_add_mopdath_edge( ) \n") ;
  

}



void vpip_add_modpath_src ( vpiHandle modpath, vpiHandle src )
{
      assert( (src->vpi_type->type_code     == vpiModPathIn ));
      assert( (modpath->vpi_type->type_code == vpiModPath   ));

      return ;
}
