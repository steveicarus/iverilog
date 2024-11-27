/*
 * Copyright (C) 2011-2024 Cary R. (cygcary@yahoo.com)
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
# include "ivl_alloc.h"

static void emit_entry(ivl_udp_t udp, char entry, unsigned *rerun)
{
      const char *value = 0;
      switch (entry) {
	case '0':
	    value = "  0 ";
	    break;
	case '1':
	    value = "  1 ";
	    break;
	case 'x':
	    value = "  x ";
	    break;
	case '?':  /* 0, 1 or x */
	    value = "  ? ";
	    break;
	case 'b':  /* 0 or 1 */
	    value = "  b ";
	    break;
	case '-':  /* Current value. */
	    value = "  - ";
	    break;
	case '*':  /* (??) */
	    value = "  * ";
	    break;
	case 'r':  /* (01) */
	    value = "  r ";
	    break;
	case 'f':  /* (10) */
	    value = "  f ";
	    break;
	case 'p':  /* (01), (0x) or (x1) */
	    value = "  p ";
	    break;
	case 'n':  /* (10), (1x) or (x0) */
	    value = "  n ";
	    break;
	  /* Icarus encodings/extensions. */
	case 'h':
	    if (*rerun) {
		  value = "  x ";
	    } else {
		  value = "  1 ";
		  *rerun = 1;
	    }
	    break;
	case 'l':
	    if (*rerun) {
		  value = "  x ";
	    } else {
		  value = "  0 ";
		  *rerun = 1;
	    }
	    break;
	case 'q':
	    value = "(bx)";
	    break;
	case 'B':
	    value = "(x?)";
	    break;
	case 'F':
	    value = "(x0)";
	    break;
	case 'M':
	    value = "(1x)";
	    break;
	case 'N':
	    value = "(1?)";
	    break;
	case 'P':
	    value = "(0?)";
	    break;
	case 'Q':
	    value = "(0x)";
	    break;
	case 'R':
	    value = "(x1)";
	    break;
	case '%':  /* This is effectively the same as q. */
	    value = "(?x)";
	    break;
	case '+':
	    value = "(?1)";
	    break;
	case '_':
	    value = "(?0)";
	    break;
	default:
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown primitive table "
	                    "entry %c in primitive %s.\n", ivl_udp_file(udp),
	                    ivl_udp_lineno(udp), entry, ivl_udp_name(udp));
	    vlog_errors += 1;
	    value = "<?> ";
	    break;
      }
      fprintf(vlog_out, "%s ", value);
}

static void emit_sequ_table(ivl_udp_t udp)
{
      unsigned idx, count = ivl_udp_rows(udp);
      for (idx = 0; idx < count; idx += 1) {
	    const char *row = ivl_udp_row(udp, idx);
            unsigned need_to_rerun = 0;
	    do {
		  unsigned entry, inputs = ivl_udp_nin(udp);
		  unsigned rerun, rerunning = need_to_rerun;
		  need_to_rerun = 0;
		  fprintf(vlog_out, "%*c", 2*indent_incr, ' ');
		  for (entry = 1; entry <= inputs; entry += 1) {
			rerun = rerunning;
			emit_entry(udp, row[entry], &rerun);
			if (rerun && ! rerunning) need_to_rerun = rerun;
		  }
		  fprintf(vlog_out, ": ");
		    /* The first entry is the current state. */
		  rerun = rerunning;
		  emit_entry(udp, row[0], &rerun);
		  if (rerun && ! rerunning) need_to_rerun = rerun;
		  fprintf(vlog_out, ": ");
		    /* The new value is after the inputs. */
		  rerun = rerunning;
		  emit_entry(udp, row[inputs+1], &rerun);
		  if (rerun && ! rerunning) need_to_rerun = rerun;
		  fprintf(vlog_out, ";\n");
	    } while (need_to_rerun);
      }
}

static void emit_comb_table(ivl_udp_t udp)
{
      unsigned idx, count = ivl_udp_rows(udp);
      for (idx = 0; idx < count; idx += 1) {
	    const char *row = ivl_udp_row(udp, idx);
            unsigned need_to_rerun = 0;
	    do {
		  unsigned entry, inputs = ivl_udp_nin(udp);
		  unsigned rerun, rerunning = need_to_rerun;
		  need_to_rerun = 0;
		  fprintf(vlog_out, "%*c", 2*indent_incr, ' ');
		  for (entry = 0; entry < inputs; entry += 1) {
			rerun = rerunning;
			emit_entry(udp, row[entry], &rerun);
			if (rerun && ! rerunning) need_to_rerun = rerun;
		  }
		    /* The new value is after the inputs. */
		  fprintf(vlog_out, ": ");
		  rerun = rerunning;
		  emit_entry(udp, row[inputs], &rerun);
		  if (rerun && ! rerunning) need_to_rerun = rerun;
		  fprintf(vlog_out, ";\n");
	    } while (need_to_rerun);
      }
}

static void emit_udp(ivl_udp_t udp)
{
      unsigned idx, count;
      fprintf(vlog_out, "\n");
      fprintf(vlog_out, "/* This primitive was originally defined in "
                        "file %s at line %u. */\n",
                        ivl_udp_file(udp), ivl_udp_lineno(udp));
      fprintf(vlog_out, "primitive ");
      emit_id(ivl_udp_name(udp));
      fprintf(vlog_out, " (");
      emit_id(ivl_udp_port(udp, 0));
      count = ivl_udp_nin(udp);
      for (idx = 1; idx <= count; idx += 1) {
	    fprintf(vlog_out, ", ");
	    emit_id(ivl_udp_port(udp, idx));
      }
      fprintf(vlog_out, ");\n");
      fprintf(vlog_out, "%*coutput ", indent_incr, ' ');
      emit_id(ivl_udp_port(udp, 0));
      fprintf(vlog_out, ";\n");
      for (idx = 1; idx <= count; idx += 1) {
	    fprintf(vlog_out, "%*cinput ", indent_incr, ' ');
	    emit_id(ivl_udp_port(udp, idx));
	    fprintf(vlog_out, ";\n");
      }
      if (ivl_udp_sequ(udp)) {
	    char init = ivl_udp_init(udp);
	    fprintf(vlog_out, "\n");
	    fprintf(vlog_out, "%*creg ", indent_incr, ' ');
	    emit_id(ivl_udp_port(udp, 0));
	    fprintf(vlog_out, ";\n");
	    switch (init) {
	      case '0':
	      case '1':
		  break;
	      default:
		  init = 'x';
		  break;
	    }
	    fprintf(vlog_out, "%*cinitial ", indent_incr, ' ');
	    emit_id(ivl_udp_port(udp, 0));
	    fprintf(vlog_out, " = 1'b%c;\n", init);
      }
      fprintf(vlog_out, "\n");
      fprintf(vlog_out, "%*ctable\n", indent_incr, ' ');
      if (ivl_udp_sequ(udp)) emit_sequ_table(udp);
      else emit_comb_table(udp);
      fprintf(vlog_out, "%*cendtable\n", indent_incr, ' ');
      fprintf(vlog_out, "endprimitive /* %s */\n", ivl_udp_name(udp));
}

static ivl_udp_t *udps = 0;
static unsigned num_udps = 0;

void add_udp_to_list(ivl_udp_t udp)
{
      unsigned idx;
	/* Look to see if the UDP is already in the list. */
      for (idx = 0; idx < num_udps; idx += 1) {
	    if ((ivl_udp_lineno(udp) == ivl_udp_lineno(udps[idx])) &&
                (strcmp(ivl_udp_name(udp), ivl_udp_name(udps[idx])) == 0)) {
		  return;
	    }
      }
      num_udps += 1;
      udps = realloc(udps, num_udps * sizeof(ivl_udp_t));
      udps[num_udps-1] = udp;
}

void emit_udp_list(void)
{
      unsigned idx;
      for (idx = 0; idx < num_udps; idx += 1) {
	    emit_udp(udps[idx]);
      }
      free(udps);
      udps = 0;
      num_udps = 0;
}
