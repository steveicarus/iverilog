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

# include  "sizer_priv.h"

using namespace std;

/*
 * Count each bit of flip-flops. It is clear and obvious how these
 * come out, so no need to make alternate counts as well.
 */
static void scan_lpms_ff(ivl_scope_t, ivl_lpm_t lpm, struct sizer_statistics&stats)
{
      ivl_nexus_t out = ivl_lpm_q(lpm);
      unsigned wid = get_nexus_width(out);

      stats.flop_count += wid;
}

/*
 * Count adders as 2m gates.
 * Also keep a count of adders by width, just out of curiosity.
 */
static void scan_lpms_add(ivl_scope_t, ivl_lpm_t lpm, struct sizer_statistics&stats)
{
      unsigned wid = ivl_lpm_width(lpm);

      stats.adder_count[wid] += 1;

      stats.gate_count += 2*wid;
}

/*
 * Count equality comparator as 2m gates.
 * Also keep a count of comparators by width, just out of curiosity.
 */
static void scan_lpms_equality(ivl_scope_t, ivl_lpm_t lpm, struct sizer_statistics&stats)
{
      unsigned wid = ivl_lpm_width(lpm);

      stats.equality_count[wid] += 1;

      stats.gate_count += 2*wid;
}

static void scan_lpms_equality_wild(ivl_scope_t, ivl_lpm_t lpm, struct sizer_statistics&stats)
{
      unsigned wid = ivl_lpm_width(lpm);

      stats.equality_wc_count[wid] += 1;

      stats.gate_count += 2*wid;
}

/*
 * Count magnitude comparators as 2m gates.
 * Also keep a count of comparators by width, just out of curiosity.
 */
static void scan_lpms_magnitude(ivl_scope_t, ivl_lpm_t lpm, struct sizer_statistics&stats)
{
      unsigned wid = ivl_lpm_width(lpm);

      stats.magnitude_count[wid] += 1;

      stats.gate_count += 2*wid;
}

/*
 * Count mux devices as 2m gates.
 * Also count the mux slices of various select sizes.
 */
static void scan_lpms_mux(ivl_scope_t, ivl_lpm_t lpm, struct sizer_statistics&stats)
{
	// For now, don't generate statistics for wide mux devices.
      if (ivl_lpm_size(lpm) > 2) {
	    stats.lpm_bytype[ivl_lpm_type(lpm)] += 1;
	    return;
      }

	// The "width" of a mux is the number of 1-bit slices.
      unsigned wid = ivl_lpm_width(lpm);

	// Count the slices of the various width of muxes.
      stats.mux_count[2] += wid;

      stats.gate_count += 2*wid;
}

/*
 * Count reduction gates (wide input gates) as 1m gates.
 */
static void scan_lpms_reduction(ivl_scope_t, ivl_lpm_t lpm, struct sizer_statistics&stats)
{
      unsigned wid = ivl_lpm_width(lpm);

      stats.gate_count += wid;
}

void scan_lpms(ivl_scope_t scope, struct sizer_statistics&stats)
{
      for (unsigned idx = 0 ; idx < ivl_scope_lpms(scope) ; idx += 1) {
	    ivl_lpm_t lpm = ivl_scope_lpm(scope,idx);
	    switch (ivl_lpm_type(lpm)) {
		    // Part select nodes don't actually take up
		    // hardware. These represent things like bundle
		    // manipulations, which are done in routing.
		case IVL_LPM_PART_VP:
		case IVL_LPM_PART_PV:
		case IVL_LPM_CONCAT:
		case IVL_LPM_CONCATZ:
		case IVL_LPM_REPEAT:
		case IVL_LPM_SUBSTITUTE:
		  break;

		case IVL_LPM_ADD:
		  scan_lpms_add(scope, lpm, stats);
		  break;

		case IVL_LPM_CMP_EQ:
		case IVL_LPM_CMP_NE:
		case IVL_LPM_CMP_EEQ:
		case IVL_LPM_CMP_NEE:
		  scan_lpms_equality(scope, lpm, stats);
		  break;

		case IVL_LPM_CMP_EQX:
		case IVL_LPM_CMP_EQZ:
		  scan_lpms_equality_wild(scope, lpm, stats);
		  break;

		case IVL_LPM_CMP_GE:
		case IVL_LPM_CMP_GT:
		  scan_lpms_magnitude(scope, lpm, stats);
		  break;

		    // D-Type flip-flops.
		case IVL_LPM_FF:
		  scan_lpms_ff(scope, lpm, stats);
		  break;

		case IVL_LPM_MUX:
		  scan_lpms_mux(scope, lpm, stats);
		  break;

		case IVL_LPM_RE_AND:
		case IVL_LPM_RE_NAND:
		case IVL_LPM_RE_OR:
		case IVL_LPM_RE_NOR:
		case IVL_LPM_RE_XOR:
		case IVL_LPM_RE_XNOR:
		  scan_lpms_reduction(scope, lpm, stats);
		  break;

		default:
		  stats.lpm_bytype[ivl_lpm_type(lpm)] += 1;
		  break;
	    }
      }
}
