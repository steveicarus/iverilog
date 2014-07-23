#ifndef IVL_sizer_priv_H
#define IVL_sizer_priv_H
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

# include  "config.h"
# include  "ivl_target.h"

# include  <map>
# include  <cstdio>

struct sizer_statistics {
	// These are the accumulated global statistics
      unsigned flop_count;
      unsigned gate_count;
	// Count adders of various dimension
      std::map<unsigned,unsigned> adder_count;
	// count equality comparators
      std::map<unsigned,unsigned> equality_count;
	// count equality (with wildcard) comparators
      std::map<unsigned,unsigned> equality_wc_count;
	// Count magnitude comparators
      std::map<unsigned,unsigned> magnitude_count;
	// Count mux's of various dimension
      std::map<unsigned,unsigned> mux_count;
	// Different kinds of nodes that we have not accounted for
      std::map<ivl_lpm_type_t,unsigned> lpm_bytype;
      std::map<ivl_logic_t,unsigned>    log_bytype;

      inline sizer_statistics()
      {
	    flop_count = 0;
	    gate_count = 0;
      }

      struct sizer_statistics& operator += (const struct sizer_statistics&that);
};

extern int sizer_errors;
extern FILE*sizer_out;

extern void scan_logs(ivl_scope_t scope, struct sizer_statistics&stats);
extern void scan_lpms(ivl_scope_t scope, struct sizer_statistics&stats);


extern unsigned get_nexus_width(ivl_nexus_t nex);

#endif /* IVL_sizer_priv_H */
