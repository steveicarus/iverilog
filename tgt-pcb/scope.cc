/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
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

# include "pcb_priv.h"
# include <map>
# include <string>
# include <cassert>
# include <cstdio>

using namespace std;

list<struct nexus_data*> nexus_list;

map <string, element_data_t*> element_list;

struct attr_value {
      ivl_attribute_type_t type;
      string str;
      long num;
};

static void sheet_box(ivl_scope_t scope, const map<string,attr_value>&attrs);
static void black_box(ivl_scope_t scope, const map<string,attr_value>&attrs);

static string wire_name(ivl_signal_t sig);

int scan_scope(ivl_scope_t scope)
{
      int black_box_flag = 0;

      map<string,attr_value> attrs;

	// Scan the attributes, looking in particular for the
	// black_box attribute. While we are at it, save the collected
	// attributes into a map that we can pass on to the processing
	// functions.
      for (unsigned idx = 0 ; idx < ivl_scope_attr_cnt(scope) ; idx += 1) {
	    ivl_attribute_t attr = ivl_scope_attr_val(scope, idx);
	    string attr_key = attr->key;

	    if (attr_key == "ivl_black_box") {
		    // Ah hah, this is a black box.
		  black_box_flag = 1;
	    } else {
		  struct attr_value val;
		  val.type = attr->type;
		  switch (val.type) {
		      case IVL_ATT_VOID:
			break;
		      case IVL_ATT_STR:
			val.str = attr->val.str;
			break;
		      case IVL_ATT_NUM:
			val.num = attr->val.num;
			break;
		  }
		  attrs[attr_key] = val;
	    }
      }

	// If this scope is a black box, then process it
	// so. Otherwise, process it as a sheet, which will recurse.
      if (black_box_flag) {
	    black_box(scope, attrs);
      } else {
	    sheet_box(scope, attrs);
      }

      return 0;
}

extern "C" int child_scan_fun(ivl_scope_t scope, void*)
{
      scan_scope(scope);
      return 0;
}

void sheet_box(ivl_scope_t scope, const map<string,attr_value>&)
{
      printf("Sheet %s...\n", ivl_scope_name(scope));
      unsigned sigs = ivl_scope_sigs(scope);
      for (unsigned idx = 0 ; idx < sigs ; idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, idx);
	    printf("   Wire %s\n", ivl_signal_basename(sig));

	    assert(ivl_signal_array_count(sig) == 1);
	    ivl_nexus_t nex = ivl_signal_nex(sig, 0);

	    struct nexus_data*nex_data = reinterpret_cast<nexus_data*>
		  (ivl_nexus_get_private(nex));
	    if (nex_data == 0) {
		  nex_data = new nexus_data;
		  nex_data->name = wire_name(sig);
		  nexus_list.push_back(nex_data);
		  ivl_nexus_set_private(nex, nex_data);
	    }
      }

      ivl_scope_children(scope, child_scan_fun, 0);
}

/*
 * A black box is a component. Do not process the contents, other than
 * to get at the ports that we'll attach to the netlist.
 */
static void black_box(ivl_scope_t scope, const map<string,attr_value>&attrs)
{
      assert(ivl_scope_type(scope) == IVL_SCT_MODULE);
      printf("   Component %s is %s\n", ivl_scope_name(scope), ivl_scope_tname(scope));

	// The refdes for the object is by default the name of
	// the instance. If the user attaches a refdes
	// attribute, then use that instead.
      string refdes = ivl_scope_basename(scope);
      map<string,attr_value>::const_iterator aptr = attrs.find("refdes");
      if (aptr != attrs.end()) {
	    assert(aptr->second.type == IVL_ATT_STR);
	    refdes = aptr->second.str;
      }

      element_data_t*elem_data = new element_data_t;

	// Scan the parameters of the module for any values that may
	// be of interest to the PCB element.
      unsigned params = ivl_scope_params(scope);
      for (unsigned idx = 0 ; idx < params ; idx += 1) {
	    ivl_parameter_t par = ivl_scope_param(scope, idx);
	    string name = ivl_parameter_basename(par);

	    if (name == "description") {
		  ivl_expr_t exp = ivl_parameter_expr(par);
		  switch (ivl_expr_type(exp)) {
		      case IVL_EX_STRING:
			elem_data->description = ivl_expr_string(exp);
			break;
		      default:
			assert(0);
		  }

	    } else if (name == "value") {
		  ivl_expr_t exp = ivl_parameter_expr(par);
		  switch (ivl_expr_type(exp)) {
		      case IVL_EX_STRING:
			elem_data->value = ivl_expr_string(exp);
			break;
		      default:
			assert(0);
		  }

	    } else if (name == "footprint") {
		  ivl_expr_t exp = ivl_parameter_expr(par);
		  switch (ivl_expr_type(exp)) {
		      case IVL_EX_STRING:
			elem_data->footprint = ivl_expr_string(exp);
			break;
		      default:
			assert(0);
		  }
	    }
      }

	// If there is a "description" attribute for the device, then
	// use that in place of the parameter.
      if ( (aptr = attrs.find("description")) != attrs.end() ) {
	    assert(aptr->second.type == IVL_ATT_STR);
	    elem_data->description = aptr->second.str;
      }

	// Get the "value" attribute for the device.
      if ( (aptr = attrs.find("value")) != attrs.end() ) {
	    switch (aptr->second.type) {
		case IVL_ATT_VOID:
		  break;
		case IVL_ATT_STR:
		  elem_data->value = aptr->second.str;
		  break;
		case IVL_ATT_NUM:
		  assert(0);
		  break;
	    }
      }

	// Look for the ports of the black box and make sure they are
	// attached to signals.  Attach the port as a pin wired to a net.
      unsigned sigs = ivl_scope_sigs(scope);
      for (unsigned idx = 0 ; idx < sigs ; idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, idx);
	    ivl_signal_port_t sip = ivl_signal_port(sig);
	      // Skip signals that are not ports.
	    if (sip == IVL_SIP_NONE)
		  continue;

	    assert(ivl_signal_array_count(sig) == 1);
	    ivl_nexus_t nex = ivl_signal_nex(sig, 0);
	    struct nexus_data*nex_data = reinterpret_cast<struct nexus_data*>(ivl_nexus_get_private(nex));
	    assert(nex_data);

	    string pindes = ivl_signal_basename(sig);
	    string pin = refdes + "-" + pindes;
	    nex_data->pins.insert(pin);
	    printf("     port %s\n", ivl_signal_basename(sig));
      }

      element_data_t*&eptr = element_list[refdes];
      assert(eptr == 0);
      eptr = elem_data;
}

static string wire_name(ivl_signal_t sig)
{
      string res = ivl_signal_basename(sig);
      return res;
}
