/*
 * Copyright (c) 2013-2024 Stephen Williams (steve@icarus.com)
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

# include  "version_base.h"
# include  "version_tag.h"
# include  "priv.h"
# include  "ivl_target.h"
# include  "nex_data.h"
# include  <vector>
# include  <cstdio>
# include  <cstring>
# include  <cassert>

using namespace std;

/*
 * This is a BLIF target module.
 */

static const char*version_string =
"Icarus Verilog BLIF Code Generator " VERSION " (" VERSION_TAG ")\n\n"
"Copyright (c) 2013-2024 Stephen Williams (steve@icarus.com)\n\n"
"  This program is free software; you can redistribute it and/or modify\n"
"  it under the terms of the GNU General Public License as published by\n"
"  the Free Software Foundation; either version 2 of the License, or\n"
"  (at your option) any later version.\n"
"\n"
"  This program is distributed in the hope that it will be useful,\n"
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"  GNU General Public License for more details.\n"
"\n"
"  You should have received a copy of the GNU General Public License along\n"
"  with this program; if not, write to the Free Software Foundation, Inc.,\n"
"  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n"
;

int blif_errors = 0;

static void emit_blif(const char*blif_path, ivl_design_t des, ivl_scope_t model);

static int process_scan_fun(ivl_process_t net, void* /*raw*/)
{
      fprintf(stderr, "%s:%u: sorry: BLIF: Processes not supported yet.\n",
	      ivl_process_file(net), ivl_process_lineno(net));
      blif_errors += 1;
      return 0;
}

int target_design(ivl_design_t des)
{
      const char*blif_path = ivl_design_flag(des, "-o");

      vector<ivl_scope_t> root_modules;

	// Locate the root scope for the design. Note that the BLIF
	// format implies that there is a single root of the model.
      ivl_scope_t*roots;
      unsigned nroots;
      ivl_design_roots(des, &roots, &nroots);

      for (unsigned idx = 0 ; idx < nroots ; idx += 1) {
	    if (ivl_scope_type(roots[idx]) == IVL_SCT_MODULE) {
		  root_modules.push_back(roots[idx]);
		  continue;
	    }
	    if (ivl_scope_type(roots[idx]) == IVL_SCT_PACKAGE) {
		  fprintf(stderr, "Skipping package scope %s (%s:%u)\n",
			  ivl_scope_name(roots[idx]), ivl_scope_file(roots[idx]),
			  ivl_scope_lineno(roots[idx]));
		  continue;
	    }

	    fprintf(stderr, "BLIF: Don't know how to handle root scope %s (%s:%u)\n",
		    ivl_scope_name(roots[idx]), ivl_scope_file(roots[idx]),
		    ivl_scope_lineno(roots[idx]));
      }

      if (root_modules.size() != 1) {
	    fprintf(stderr, "BLIF: The BLIF code generator requires that there be only one root module scope. Found these root modules:\n");
	    for (size_t idx = 0 ; idx < root_modules.size() ; idx += 1) {
		  fprintf(stderr, "    %s  (%s:%u)\n",
			  ivl_scope_name(root_modules[idx]),
			  ivl_scope_file(root_modules[idx]),
			  ivl_scope_lineno(root_modules[idx]));
	    }
	    return 1;
      }


	// Detect processes and dispatch them.
      ivl_design_process(des, &process_scan_fun, 0);

	// Emit to the destination file.
      assert(blif_path);
      emit_blif(blif_path, des, root_modules[0]);

      return blif_errors;
}


const char* target_query(const char*key)
{
      if (strcmp(key,"version") == 0)
	    return version_string;

      return 0;
}

/*
 * Print all the bits of a signal. This is for the .input or .output
 * lines of a .model. All the bits need to be exploded, so print each
 * bit of a vector as its own name.
 */
static void print_signal_bits(FILE*fd, ivl_signal_t sig)
{
      ivl_nexus_t nex = ivl_signal_nex(sig, 0);
      blif_nex_data_t* ned = blif_nex_data_t::get_nex_data(nex);
      ned->set_name(sig);

      for (unsigned idx = 0 ; idx < ned->get_width() ; idx += 1) {
	    fprintf(fd, " %s%s", ned->get_name(), ned->get_name_index(idx));
      }
}


static void emit_scope(FILE*fd, ivl_scope_t scope)
{
      for (unsigned idx = 0 ; idx < ivl_scope_logs(scope) ; idx += 1) {
	    ivl_net_logic_t net = ivl_scope_log(scope, idx);
	    assert(net);
	    blif_errors += print_logic_gate(fd, net);
      }

      for (unsigned idx = 0 ; idx < ivl_scope_lpms(scope) ; idx += 1) {
	    ivl_lpm_t net = ivl_scope_lpm(scope, idx);
	    blif_errors += print_lpm(fd, net);
      }

      for (size_t idx = 0 ; idx < ivl_scope_childs(scope) ; idx += 1) {
	    ivl_scope_t child = ivl_scope_child(scope, idx);
	    emit_scope(fd, child);
      }
}

static void emit_blif(const char*blif_path, ivl_design_t des, ivl_scope_t model)
{
      FILE*fd = fopen(blif_path, "wt");
      if (fd == 0) {
	    perror(blif_path);
	    blif_errors += 1;
	    return;
      }

      fprintf(fd, ".model %s\n", ivl_scope_basename(model));

	// The root scope
      vector<ivl_signal_t> ports_in;
      vector<ivl_signal_t> ports_out;
      for (unsigned idx = 0 ; idx < ivl_scope_sigs(model) ; idx += 1) {
	    ivl_signal_t prt = ivl_scope_sig(model, idx);
	    ivl_signal_port_t dir = ivl_signal_port(prt);

	    switch (dir) {
		case IVL_SIP_NONE:
		  break;
		case IVL_SIP_INPUT:
		  ports_in.push_back(prt);
		  break;
		case IVL_SIP_OUTPUT:
		  ports_out.push_back(prt);
		  break;
		case IVL_SIP_INOUT:
		  fprintf(stderr, "BLIF: error: "
			  "Model port %s is bi-directional.\n",
			  ivl_signal_basename(prt));
		  blif_errors += 1;
		  ports_in.push_back(prt);
		  ports_out.push_back(prt);
		  break;
	    }
      }

      if (ports_in.size() > 0) {
	    fprintf(fd, ".inputs");
	    for (size_t idx = 0 ; idx < ports_in.size() ; idx += 1) {
		  ivl_signal_t prt = ports_in[idx];
		  print_signal_bits(fd, prt);
	    }
	    fprintf(fd, "\n");
      }
      if (ports_out.size() > 0) {
	    fprintf(fd, ".outputs");
	    for (size_t idx = 0 ; idx < ports_out.size() ; idx += 1) {
		  ivl_signal_t prt = ports_out[idx];
		  print_signal_bits(fd, prt);
	    }
	    fprintf(fd, "\n");
      }

      emit_scope(fd, model);

      emit_constants(fd, des, model);

      fprintf(fd, ".end\n");
      fclose(fd);
}

bool scope_is_in_model(const ivl_scope_t model, ivl_scope_t scope)
{
      while (scope) {
	    if (scope==model)
		  return true;

	    scope = ivl_scope_parent(scope);
      }

      return false;
}
