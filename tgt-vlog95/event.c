/*
 * Copyright (C) 2011 Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

# include <stdlib.h>
# include <string.h>
# include "config.h"
# include "vlog95_priv.h"

void emit_event(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned eidx, nevents, first = 1;

      nevents = ivl_stmt_nevent(stmt);
      for (eidx = 0; eidx < nevents; eidx += 1) {
	    unsigned idx, count, had_edge = 0;
	    ivl_event_t event = ivl_stmt_events(stmt, eidx);

	      /* Check for any edge events. */
	    count = ivl_event_nany(event);
	    if (count) had_edge = 1;
	    for (idx = 0; idx < count; idx += 1) {
		  if (first) first = 0;
		  else fprintf(vlog_out, " or ");
		  emit_name_of_nexus(scope, ivl_event_any(event, idx));
	    }

	      /* Check for positive edge events. */
	    count = ivl_event_npos(event);
	    if (count) had_edge = 1;
	    for (idx = 0; idx < count; idx += 1) {
		  if (first) first = 0;
		  else fprintf(vlog_out, " or ");
		  fprintf(vlog_out, "posedge ");
		  emit_name_of_nexus(scope, ivl_event_pos(event, idx));
	    }

	      /* Check for negative edge events. */
	    count = ivl_event_nneg(event);
	    if (count) had_edge = 1;
	    for (idx = 0; idx < count; idx += 1) {
		  if (first) first = 0;
		  else fprintf(vlog_out, " or ");
		  fprintf(vlog_out, "negedge ");
		  emit_name_of_nexus(scope, ivl_event_neg(event, idx));
	    }

	      /* We have a named event if there were no edge events. */
	    if (!had_edge) {
		  ivl_scope_t ev_scope = ivl_event_scope(event);
		  emit_scope_module_path(scope, ev_scope);
		  fprintf(vlog_out, "%s", ivl_event_basename(event));
	    }
      }
}
