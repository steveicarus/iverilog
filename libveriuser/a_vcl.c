/*
 * Copyright (c) 2003-2014 Stephen Williams (steve@icarus.com)
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

#include  <vpi_user.h>
#include  <acc_user.h>
#include  <stdlib.h>
#include  "priv.h"
#include  <assert.h>
#include  "ivl_alloc.h"

/*
 * This is the structure of a record that I use locally to hold the
 * information about a VCL. This record includes a pointer to the vpi
 * callback that is actually watching the value, and that callback has
 * a pointer to this record in its user_data so that I can get to it
 * when the value changes.
 *
 * Keep all these records in a vcl_list so that I can get access to
 * them for the vcl_delete.
 */
struct vcl_record {
	/* Object who's value I'm watching. */
      vpiHandle obj;
	/* User's callback routine. */
      PLI_INT32(*consumer)(p_vc_record);
      void*user_data;

      PLI_INT32 vcl_flag;

      vpiHandle callback;
      struct vcl_record*next;
};

static struct vcl_record*vcl_list = 0;

static int vpi_strength_to_vcl(int vs)
{
      switch (vs) {
	  case vpiSupplyDrive:
	    return vclSupply;
	  case vpiStrongDrive:
	    return vclStrong;
	  case vpiPullDrive:
	    return vclPull;
	  case vpiLargeCharge:
	    return vclLarge;
	  case vpiWeakDrive:
	    return vclWeak;
	  case vpiMediumCharge:
	    return vclMedium;
	  case vpiSmallCharge:
	    return vclSmall;
	  case vpiHiZ:
	    return vclHighZ;
	  default:
	    return -1;
      }
}

/*
 * This is a VPI callback that notices the value change. This function
 * further dispatches the information about the callback to the
 * consumer function.
 */
static PLI_INT32 vcl_value_callback(struct t_cb_data*cb)
{
      s_vpi_time sim_time;
      s_vpi_value obj_value;
      struct t_vc_record vcr;
      struct vcl_record*cur = (struct vcl_record*)cb->user_data;

      sim_time.type = vpiSimTime;
      vpi_get_time(cur->obj, &sim_time);

      switch (cur->vcl_flag) {
	  case VCL_VERILOG_LOGIC:
	    vpi_printf("XXXX vcl_value_callback(%s=%d);\n",
		       vpi_get_str(vpiName, cur->obj), -1);

	    vcr.vc_reason = logic_value_change;
	    break;

	  case VCL_VERILOG_STRENGTH:
	    vcr.vc_reason = strength_value_change;
	    obj_value.format = vpiStrengthVal;
	    vpi_get_value(cur->obj, &obj_value);
	    assert(obj_value.format == vpiStrengthVal);
	    switch (obj_value.value.strength[0].logic) {
		case vpi0:
		  vcr.out_value.strengths_s.logic_value = acc0;
		  vcr.out_value.strengths_s.strength1 =
			vpi_strength_to_vcl(obj_value.value.strength[0].s0);
		  vcr.out_value.strengths_s.strength2 =
			vpi_strength_to_vcl(obj_value.value.strength[0].s0);
		  break;
		case vpi1:
		  vcr.out_value.strengths_s.logic_value = acc1;
		  vcr.out_value.strengths_s.strength1 =
			vpi_strength_to_vcl(obj_value.value.strength[0].s1);
		  vcr.out_value.strengths_s.strength2 =
			vpi_strength_to_vcl(obj_value.value.strength[0].s1);
		  break;
		case vpiX:
		  vcr.out_value.strengths_s.logic_value = accX;
		  vcr.out_value.strengths_s.strength1 =
			vpi_strength_to_vcl(obj_value.value.strength[0].s1);
		  vcr.out_value.strengths_s.strength2 =
			vpi_strength_to_vcl(obj_value.value.strength[0].s0);
		  break;
		case vpiZ:
		  vcr.out_value.strengths_s.logic_value = accZ;
		  vcr.out_value.strengths_s.strength1 = vclHighZ;
		  vcr.out_value.strengths_s.strength2 = vclHighZ;
		  break;
		default:
		  assert(0);
	    }

	    if (pli_trace) {
		  fprintf(pli_trace,
			  "Call vcl_value_callback(%s=%d <s1=%d,s2=%d>)\n",
			  vpi_get_str(vpiFullName, cur->obj),
			  vcr.out_value.strengths_s.logic_value,
			  vcr.out_value.strengths_s.strength1,
			  vcr.out_value.strengths_s.strength2);
	    }
	    break;

	  default:
	    assert(0);
      }

      vcr.vc_hightime = sim_time.high;
      vcr.vc_lowtime  = sim_time.low;
      vcr.user_data = cur->user_data;

      (cur->consumer) (&vcr);
      return 0;
}

void acc_vcl_add(handle obj, PLI_INT32(*consumer)(p_vc_record),
		 void*data, PLI_INT32 vcl_flag)
{
      struct vcl_record*cur;
      struct t_cb_data cb;

      switch (vpi_get(vpiType, obj)) {
	  case vpiNet:
	  case vpiReg:
	    cur = malloc(sizeof (struct vcl_record));
	    cur->obj = obj;
	    cur->consumer = consumer;
	    cur->user_data = data;
	    cur->vcl_flag = vcl_flag;
	    cur->next = vcl_list;
	    vcl_list = cur;

	    cb.reason = cbValueChange;
	    cb.cb_rtn = vcl_value_callback;
	    cb.obj = obj;
	    cb.time = 0;
	    cb.value = 0;
	    cb.user_data = (void*)cur;
	    cur->callback = vpi_register_cb(&cb);

	    if (pli_trace) {
		  fprintf(pli_trace, "acc_vcl_add(<%s>, ..., %p, %d)\n",
			  vpi_get_str(vpiFullName, obj), data, (int)vcl_flag);
	    }
	    break;

	  default:
	    vpi_printf("XXXX acc_vcl_add(<type=%d>, ..., %d);\n",
		       (int)vpi_get(vpiType, obj), (int)vcl_flag);
	    break;
      }

}

void acc_vcl_delete(handle obj, PLI_INT32(*consumer)(p_vc_record),
		    void*data, PLI_INT32 vcl_flag)
{
      (void)obj; /* Parameter is not used. */
      (void)consumer; /* Parameter is not used. */
      (void)data; /* Parameter is not used. */
      (void)vcl_flag; /* Parameter is not used. */
      vpi_printf("XXXX acc_vcl_delete(...)\n");
}
