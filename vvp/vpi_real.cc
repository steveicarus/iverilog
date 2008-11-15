/*
 * Copyright (c) 2003-2008 Stephen Williams (steve@icarus.com)
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

# include  "compile.h"
# include  "vpi_priv.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>

struct __vpiRealVar* vpip_realvar_from_handle(vpiHandle obj)
{
      assert(obj);
      if (obj->vpi_type->type_code == vpiRealVar)
	    return (struct __vpiRealVar*)obj;
      else
	    return 0;
}

static int real_var_get(int code, vpiHandle ref)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp = vpip_realvar_from_handle(ref);

      switch (code) {
	case vpiArray:
	    return rfp->is_netarray != 0;

	case vpiSize:
	    return 1;

	case vpiLineNo:
	    return 0; // Not implemented for now!

	case vpiAutomatic:
	    return (int) vpip_scope(rfp)->is_automatic;
      }

      return 0;
}

static char* real_var_get_str(int code, vpiHandle ref)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;

      if (code == vpiFile) {  // Not implemented for now!
	    return simple_set_rbuf_str(file_names[0]);
      }

      char *nm, *ixs;
      if (rfp->is_netarray) {
	    nm = strdup(vpi_get_str(vpiName, rfp->within.parent));
	    s_vpi_value vp;
	    vp.format = vpiDecStrVal;
	    vpi_get_value(rfp->id.index, &vp);
	    ixs = vp.value.str;  /* do I need to strdup() this? */
      } else {
	    nm = strdup(rfp->id.name);
	    ixs = NULL;
      }

      char *rbuf = generic_get_str(code, &(vpip_scope(rfp)->base), nm, ixs);
      free(nm);
      return rbuf;
}

static vpiHandle real_var_get_handle(int code, vpiHandle ref)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;

      switch (code) {

	  case vpiParent:
	    return rfp->is_netarray ? rfp->within.parent : 0;

	  case vpiIndex:
	    return rfp->is_netarray ? rfp->id.index : 0;

	  case vpiScope:
	    return &(vpip_scope(rfp)->base);
      }

      return 0;
}

static vpiHandle real_var_iterate(int code, vpiHandle ref)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;

      if (code == vpiIndex) {
	    return rfp->is_netarray ? (rfp->id.index->vpi_type->iterate_)
	                              (code, rfp->id.index) : 0;
      }

      return 0;
}

static void real_var_get_value(vpiHandle ref, s_vpi_value*vp)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp
	    = (struct __vpiRealVar*)ref;
      vvp_fun_signal_real*fun
	    = dynamic_cast<vvp_fun_signal_real*>(rfp->net->fun);

      fun->get_value(vp);
}

static vpiHandle real_var_put_value(vpiHandle ref, p_vpi_value vp, int)
{
      vvp_vector4_t vec4(1024);
      double result;
      bool is_signed = false;
      assert(ref->vpi_type->type_code == vpiRealVar);

      switch (vp->format) {
	  case vpiRealVal:
	    result = vp->value.real;
	    break;
	  case vpiIntVal:
	    result = (double)vp->value.integer;
	    break;
	  case vpiBinStrVal:
	    vpip_bin_str_to_vec4(vec4, vp->value.str);
	    if (vp->value.str[0] == '-') is_signed = true;
	    vector4_to_value(vec4, result, is_signed);
	    break;
	  case vpiOctStrVal:
	    vpip_oct_str_to_vec4(vec4, vp->value.str);
	    if (vp->value.str[0] == '-') is_signed = true;
	    vector4_to_value(vec4, result, is_signed);
	    break;
	  case vpiDecStrVal:
	    vpip_dec_str_to_vec4(vec4, vp->value.str);
	    if (vp->value.str[0] == '-') is_signed = true;
	    vector4_to_value(vec4, result, is_signed);
	    break;
	  case vpiHexStrVal:
	    vpip_hex_str_to_vec4(vec4, vp->value.str);
	    if (vp->value.str[0] == '-') is_signed = true;
	    vector4_to_value(vec4, result, is_signed);
	    break;

	  default:
	    fprintf(stderr, "Cannot convert type %d to a real value.",
	            vp->format);
	    assert(0);
	    break;
      }

      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;
      assert(rfp);
      vvp_net_ptr_t destination (rfp->net, 0);
      vvp_send_real(destination, result, vthread_get_wt_context());
      return 0;
}

static const struct __vpirt vpip_real_var_rt = {
      vpiRealVar,

      real_var_get,
      real_var_get_str,
      real_var_get_value,
      real_var_put_value,

      real_var_get_handle,
      real_var_iterate,
      0,

      0
};

void vpip_real_value_change(struct __vpiCallback*cbh,
			     vpiHandle ref)
{
      struct __vpiRealVar*rfp
	    = (struct __vpiRealVar*)ref;
      vvp_fun_signal_real*fun
	    = dynamic_cast<vvp_fun_signal_real*>(rfp->net->fun);

      fun->add_vpi_callback(cbh);
}

vpiHandle vpip_make_real_var(const char*name, vvp_net_t*net)
{
      struct __vpiRealVar*obj = (struct __vpiRealVar*)
	    malloc(sizeof(struct __vpiRealVar));

      obj->base.vpi_type = &vpip_real_var_rt;
      obj->id.name = name ? vpip_name_string(name) : 0;
      obj->is_netarray = 0;
      obj->net = net;

      obj->within.scope = vpip_peek_current_scope();

      return &obj->base;
}
