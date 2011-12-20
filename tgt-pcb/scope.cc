/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include "pcb_priv.h"
# include <cstring>
# include <cassert>

using namespace std;

list<struct nexus_data*> nexus_list;

static void sheet_box(ivl_scope_t scope);
static void black_box(ivl_scope_t scope);

static string wire_name(ivl_signal_t sig);

int scan_scope(ivl_scope_t scope)
{
      int black_box_flag = 0;
      int idx;

	// Scan the attributes, looking in particular for the
	// black_box attribute.
      for (idx = 0 ; idx < ivl_scope_attr_cnt(scope) ; idx += 1) {
	    ivl_attribute_t attr = ivl_scope_attr_val(scope, idx);
	    if (strcmp(attr->key, "ivl_black_box") == 0)
		  black_box_flag = 1;
      }

	// If this scope is a black box, then process it
	// so. Otherwise, process it as a sheet, which will recurse.
      if (black_box_flag) {
	    black_box(scope);
      } else {
	    sheet_box(scope);
      }

      return 0;
}

static int child_scan_fun(ivl_scope_t scope, void*)
{
      int rc = scan_scope(scope);
      return 0;
}

void sheet_box(ivl_scope_t scope)
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
 * A black box is a component. Do not process the contents, other then
 * to get at the ports that we'll attach to the netlist.
 */
static void black_box(ivl_scope_t scope)
{
      assert(ivl_scope_type(scope) == IVL_SCT_MODULE);
      printf("   Component %s is %s\n", ivl_scope_name(scope), ivl_scope_tname(scope));
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

	    string refdes = ivl_scope_basename(scope);
	    string pindes = ivl_signal_basename(sig);
	    string pin = refdes + "-" + pindes;
	    nex_data->pins.insert(pin);
	    printf("     port %s\n", ivl_signal_basename(sig));
      }
}

static string wire_name(ivl_signal_t sig)
{
      string res = ivl_signal_basename(sig);
      return res;
}
