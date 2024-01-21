/*
 * Copyright (c) 2014-2024 Stephen Williams (steve@icarus.com)
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

using namespace std;

/*
 * This is a null target module. It does nothing.
 */


static const char*version_string =
"Icarus Verilog SIZER Statistics Generator " VERSION " (" VERSION_TAG ")\n\n"
"Copyright (c) 2014-2024 Stephen Williams (steve@icarus.com)\n\n"
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
static void emit_sizer_scope(ivl_design_t des, ivl_scope_t model, struct sizer_statistics&stats);
static void show_stats(struct sizer_statistics&stats);

/*
 * This is called by the ivl core to get version information from the
 * loadable code generator.
 */
const char* target_query(const char*key)
{
      if (strcmp(key,"version") == 0)
	    return version_string;

      return 0;
}

/*
 * This is the main entry point from the IVL core.
 */
int target_design(ivl_design_t des)
{
      const char*sizer_path = ivl_design_flag(des, "-o");

      sizer_out = fopen(sizer_path, "wt");
      assert(sizer_out);

	// Detect processes and dispatch them.
      ivl_design_process(des, &process_scan_fun, 0);

	// Locate the root scopes for the design.
      ivl_scope_t*roots;
      unsigned nroots;
      ivl_design_roots(des, &roots, &nroots);

	// Process all the root scopes. It is possible that there are
	// multiple root scopes, we will give isolated numbers for
	// each and keep then separate.
      for (unsigned idx = 0 ; idx < nroots ; idx += 1) {
	    if (ivl_scope_type(roots[idx]) != IVL_SCT_MODULE) {
		  fprintf(stderr, "SIZER: The root scope %s must be a module.\n", ivl_scope_basename(roots[idx]));
		  sizer_errors += 1;
		  continue;
	    }

	    struct sizer_statistics stats;
	    emit_sizer_scope(des, roots[idx], stats);

	    fprintf(sizer_out, "**** TOTALS\n");
	    show_stats(stats);
      }

      return sizer_errors;
}


/*
 * Processes are not collected into scopes, but we should not have any
 * left anyhow. Give error messages for all the processes that we find
 * to be remaining.
 */
static int process_scan_fun(ivl_process_t net, void* /*raw*/)
{
      for (unsigned idx = 0 ; idx < ivl_process_attr_cnt(net) ; idx += 1) {
	    ivl_attribute_t att = ivl_process_attr_val(net, idx);

	      // If synthesis is explicitly turned off for this
	      // process, then we just ignore it.
	    if (strcmp(att->key, "ivl_synthesis_off") == 0)
		  return 0;
      }

      fprintf(stderr, "%s:%u: SIZER: Processes not synthesized for statistics.\n",
	      ivl_process_file(net), ivl_process_lineno(net));
      sizer_errors += 1;
      return 0;
}

static void emit_sizer_scope(ivl_design_t des, ivl_scope_t scope, struct sizer_statistics&stats)
{

      fprintf(sizer_out, "**** module/scope: %s\n", ivl_scope_name(scope));

      scan_logs(scope, stats);
      scan_lpms(scope, stats);

      show_stats(stats);

      for (size_t idx = 0 ; idx < ivl_scope_childs(scope) ; idx += 1) {
	    ivl_scope_t child = ivl_scope_child(scope,idx);
	    struct sizer_statistics child_stats;
	    emit_sizer_scope(des, child, child_stats);
	    stats += child_stats;
      }
}

static void show_stats(struct sizer_statistics&stats)
{
      fprintf(sizer_out, "     Flip-Flops   : %u\n", stats.flop_count);
      fprintf(sizer_out, "     Logic Gates  : %u\n", stats.gate_count);

      for (map<unsigned,unsigned>::const_iterator cur = stats.adder_count.begin()
		 ; cur != stats.adder_count.end() ; ++ cur) {
	    fprintf(sizer_out, "     ADDER[%u]: %u units\n", cur->first, cur->second);
      }

      for (map<unsigned,unsigned>::const_iterator cur = stats.equality_count.begin()
		 ; cur != stats.equality_count.end() ; ++ cur) {
	    fprintf(sizer_out, "     EQUALITY[%u]: %u units\n", cur->first, cur->second);
      }

      for (map<unsigned,unsigned>::const_iterator cur = stats.equality_wc_count.begin()
		 ; cur != stats.equality_wc_count.end() ; ++ cur) {
	    fprintf(sizer_out, "     EQUALITY_WC[%u]: %u units\n", cur->first, cur->second);
      }

      for (map<unsigned,unsigned>::const_iterator cur = stats.magnitude_count.begin()
		 ; cur != stats.magnitude_count.end() ; ++ cur) {
	    fprintf(sizer_out, "     MAGNITUDE[%u]: %u units\n", cur->first, cur->second);
      }

      for (map<unsigned,unsigned>::const_iterator cur = stats.mux_count.begin()
		 ; cur != stats.mux_count.end() ; ++ cur) {
	    fprintf(sizer_out, "     MUX[%u]: %u slices\n", cur->first, cur->second);
      }

	// These are diagnostic outputs for when more detail is needed.
      for (map<ivl_lpm_type_t,unsigned>::const_iterator cur = stats.lpm_bytype.begin()
		 ; cur != stats.lpm_bytype.end() ; ++ cur) {
	    fprintf(sizer_out, "     LPM[%d]: %u unaccounted\n", cur->first, cur->second);
      }

      for (map<ivl_logic_t,unsigned>::const_iterator cur = stats.log_bytype.begin()
		 ; cur != stats.log_bytype.end() ; ++ cur) {
	    fprintf(sizer_out, "     LOG[%d]: %u unaccounted\n", cur->first, cur->second);
      }
}

unsigned get_nexus_width(ivl_nexus_t nex)
{
      for (unsigned idx = 0 ; idx < ivl_nexus_ptrs(nex) ; idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex,idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig) return ivl_signal_width(sig);
      }

      fprintf(stderr, "SIZER: Unable to find width of nexus?!\n");
      sizer_errors += 1;
      return 0;
}

struct sizer_statistics& sizer_statistics::operator += (const sizer_statistics&that)
{
      flop_count += that.flop_count;
      gate_count += that.gate_count;

      for (map<unsigned,unsigned>::const_iterator cur = that.adder_count.begin()
		 ; cur != that.adder_count.end() ; ++ cur)
	    adder_count[cur->first] += cur->second;

      for (map<unsigned,unsigned>::const_iterator cur = that.equality_count.begin()
		 ; cur != that.equality_count.end() ; ++ cur)
	    equality_count[cur->first] += cur->second;

      for (map<unsigned,unsigned>::const_iterator cur = that.equality_wc_count.begin()
		 ; cur != that.equality_wc_count.end() ; ++ cur)
	    equality_wc_count[cur->first] += cur->second;

      for (map<unsigned,unsigned>::const_iterator cur = that.magnitude_count.begin()
		 ; cur != that.magnitude_count.end() ; ++ cur)
	    magnitude_count[cur->first] += cur->second;


      for (map<unsigned,unsigned>::const_iterator cur = that.mux_count.begin()
		 ; cur != that.mux_count.end() ; ++ cur)
	    mux_count[cur->first] += cur->second;


      for (map<ivl_lpm_type_t,unsigned>::const_iterator cur = that.lpm_bytype.begin()
		 ; cur != that.lpm_bytype.end() ; ++ cur)
	    lpm_bytype[cur->first] += cur->second;


      for (map<ivl_logic_t,unsigned>::const_iterator cur = that.log_bytype.begin()
		 ; cur != that.log_bytype.end() ; ++ cur)
	    log_bytype[cur->first] += cur->second;

      return *this;
}
