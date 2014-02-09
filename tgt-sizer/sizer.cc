/*
 * Copyright (c) 2014 Stephen Williams (steve@icarus.com)
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

# include "version_base.h"
# include "version_tag.h"
# include "config.h"
# include "sizer_priv.h"
# include <cstdio>
# include <cstring>
# include <cassert>

/*
 * This is a null target module. It does nothing.
 */


static const char*version_string =
"Icarus Verilog SIZER Statistics Generator " VERSION " (" VERSION_TAG ")\n\n"
"Copyright (c) 2014 Stephen Williams (steve@icarus.com)\n\n"
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

int sizer_errors = 0;

FILE*sizer_out = 0;

static int process_scan_fun(ivl_process_t net, void*raw);
static void emit_sizer_root(ivl_design_t des, ivl_scope_t model);

int target_design(ivl_design_t des)
{
      const char*sizer_path = ivl_design_flag(des, "-o");

      sizer_out = fopen(sizer_path, "wt");
      assert(sizer_out);

	// Detect processes and dispatch them.
      ivl_design_process(des, &process_scan_fun, 0);

	// Locate the root scope for the design.
      ivl_scope_t*roots;
      unsigned nroots;
      ivl_design_roots(des, &roots, &nroots);

      for (unsigned idx = 0 ; idx < nroots ; idx += 1) {
	    if (ivl_scope_type(roots[idx]) != IVL_SCT_MODULE) {
		  fprintf(stderr, "SIZER: The root scope %s must be a module.\n", ivl_scope_basename(roots[idx]));
		  sizer_errors += 1;
		  continue;
	    }

	    emit_sizer_root(des, roots[idx]);
      }

      return sizer_errors;
}


const char* target_query(const char*key)
{
      if (strcmp(key,"version") == 0)
	    return version_string;

      return 0;
}

static int process_scan_fun(ivl_process_t net, void* /*raw*/)
{
      fprintf(stderr, "%s:%u: SIZER: Processes not synthesized for statistics.\n",
	      ivl_process_file(net), ivl_process_lineno(net));
      sizer_errors += 1;
      return 0;
}

static void emit_sizer_root(ivl_design_t des, ivl_scope_t model)
{
      fprintf(sizer_out, "**** Root module: %s\n", ivl_scope_name(model));
      fprintf(sizer_out, "     Logic gates: %u (ivl_net_logic_t nodes))\n", ivl_scope_logs(model));
      fprintf(sizer_out, "     LPM nodes  : %u (ivl_lpm_t nodes)\n", ivl_scope_lpms(model));

      struct sizer_statistics stats;
      scan_logs(model, stats);
      scan_lpms(model, stats);

      fprintf(sizer_out, "     Flip-Flops   : %u\n", stats.flop_count);
      fprintf(sizer_out, "     Logic Gates  : %u\n", stats.gate_count);
      fprintf(sizer_out, "     LPM Unknown  : %u\n", stats.lpm_unknown);
      fprintf(sizer_out, "     Logic Unknown: %u\n", stats.log_unknown);
}

unsigned get_nexus_width(ivl_nexus_t nex)
{
      ivl_signal_t sig = 0;

      for (unsigned idx = 0 ; idx < ivl_nexus_ptrs(nex) ; idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex,idx);
	    sig = ivl_nexus_ptr_sig(ptr);
	    if (sig) return ivl_signal_width(sig);
      }

      fprintf(stderr, "SIZER: Unable to find width of nexus?!\n");
      sizer_errors += 1;
      return 0;
}
