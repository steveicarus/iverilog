/*
 * Copyright (C) 2010-2011 Cary R. (cygcary@yahoo.com)
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

# include <inttypes.h>
# include <string.h>
# include "config.h"
# include "vlog95_priv.h"
# include "ivl_alloc.h"

static char*get_time_const(int time_value)
{
      switch (time_value) {
	case   2: return "100s";
	case   1: return "10s";
	case   0: return "1s";
	case  -1: return "100ms";
	case  -2: return "10ms";
	case  -3: return "1ms";
	case  -4: return "100us";
	case  -5: return "10us";
	case  -6: return "1us";
	case  -7: return "100ns";
	case  -8: return "10ns";
	case  -9: return "1ns";
	case -10: return "100ps";
	case -11: return "10ps";
	case -12: return "1ps";
	case -13: return "100fs";
	case -14: return "10fs";
	case -15: return "1fs";
	default:
	    fprintf(stderr, "Invalid time constant value %d.\n", time_value);
	    return "N/A";
      }
}

void emit_func_return(ivl_signal_t sig)
{
      if (ivl_signal_dimensions(sig) > 0) {
	    fprintf(stderr, "%s:%u: vlog95 error: A function cannot return "
	                    "an array.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig));
	    vlog_errors += 1;
      } else if (ivl_signal_integer(sig)) {
	    fprintf(vlog_out, " integer");
      } else if (ivl_signal_data_type(sig) == IVL_VT_REAL) {
	    fprintf(vlog_out, " real");
      } else {
	    int msb = ivl_signal_msb(sig);
	    int lsb = ivl_signal_lsb(sig);
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
      }
}

void emit_var_def(ivl_signal_t sig)
{
      if (ivl_signal_local(sig)) return;
      fprintf(vlog_out, "%*c", indent, ' ');
      if (ivl_signal_integer(sig)) {
	    fprintf(vlog_out, "integer %s;\n", ivl_signal_basename(sig));
	    if (ivl_signal_dimensions(sig) > 0) {
		  fprintf(stderr, "%s:%u: vlog95 error: Integer arrays (%s) "
		                  "are not supported.\n", ivl_signal_file(sig),
		                  ivl_signal_lineno(sig),
		                  ivl_signal_basename(sig));
		  vlog_errors += 1;
	    }
      } else if (ivl_signal_data_type(sig) == IVL_VT_REAL) {
	    fprintf(vlog_out, "real %s;\n", ivl_signal_basename(sig));
	    if (ivl_signal_dimensions(sig) > 0) {
		  fprintf(stderr, "%s:%u: vlog95 error: Real arrays (%s) "
		                  "are not supported.\n", ivl_signal_file(sig),
		                  ivl_signal_lineno(sig),
		                  ivl_signal_basename(sig));
		  vlog_errors += 1;
	    }
      } else {
	    int msb = ivl_signal_msb(sig);
	    int lsb = ivl_signal_lsb(sig);
	    fprintf(vlog_out, "reg");
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
	    fprintf(vlog_out, " %s", ivl_signal_basename(sig));
	    if (ivl_signal_dimensions(sig) > 0) {
		  unsigned wd_count = ivl_signal_array_count(sig);
		  int first = ivl_signal_array_base(sig);
		  int last = first + wd_count - 1;
		  if (ivl_signal_array_addr_swapped(sig)) {
			fprintf(vlog_out, " [%d:%d]", last, first);
		  } else {
			fprintf(vlog_out, " [%d:%d]", first, last);
		  }
	    }
	    fprintf(vlog_out, ";\n");
	    if (ivl_signal_signed(sig)) {
		  fprintf(stderr, "%s:%u: vlog95 error: Signed registers (%s) "
		                  "are not supported.\n", ivl_signal_file(sig),
		                  ivl_signal_lineno(sig),
		                  ivl_signal_basename(sig));
		  vlog_errors += 1;
	    }
      }
}

void emit_net_def(ivl_signal_t sig)
{
      int msb = ivl_signal_msb(sig);
      int lsb = ivl_signal_lsb(sig);
      if (ivl_signal_local(sig)) return;
      fprintf(vlog_out, "%*c", indent, ' ');
      if (ivl_signal_data_type(sig) == IVL_VT_REAL){
	    fprintf(vlog_out, "wire %s;\n", ivl_signal_basename(sig));
	    fprintf(stderr, "%s:%u: vlog95 error: Real nets (%s) are "
	                    "not supported.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), ivl_signal_basename(sig));
	    vlog_errors += 1;
      } else if (ivl_signal_signed(sig)) {
	    fprintf(vlog_out, "wire");
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
	    fprintf(vlog_out, " %s;\n", ivl_signal_basename(sig));
	    fprintf(stderr, "%s:%u: vlog95 error: Signed nets (%s) are "
	                    "not supported.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), ivl_signal_basename(sig));
	    vlog_errors += 1;
      } else if (ivl_signal_dimensions(sig) > 0) {
	    fprintf(vlog_out, "wire");
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
	    fprintf(vlog_out, " %s;\n", ivl_signal_basename(sig));
	    fprintf(stderr, "%s:%u: vlog95 error: Array nets (%s) are "
	                    "not supported.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), ivl_signal_basename(sig));
	    vlog_errors += 1;
      } else {
	    switch(ivl_signal_type(sig)) {
	      case IVL_SIT_TRI:
	      case IVL_SIT_UWIRE:
// HERE: Need to add support for supply nets. Probably supply strength
//       with a constant 0/1 driver for all the bits.
		  fprintf(vlog_out, "wire");
		  break;
	      case IVL_SIT_TRI0:
		  fprintf(vlog_out, "tri0");
		  break;
	      case IVL_SIT_TRI1:
		  fprintf(vlog_out, "tri1");
		  break;
	      case IVL_SIT_TRIAND:
		  fprintf(vlog_out, "wand");
		  break;
	      case IVL_SIT_TRIOR:
		  fprintf(vlog_out, "wor");
		  break;
	      default:
		  fprintf(vlog_out, "<unknown>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unknown net type "
	                    "(%d).\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), (int)ivl_signal_type(sig));
		  vlog_errors += 1;
		  break;
	    }
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
	    fprintf(vlog_out, " %s;\n", ivl_signal_basename(sig));
      }
}

static char *get_mangled_name(ivl_scope_t scope, unsigned root)
{
      char *name;
	/* If the module has parameters and it's not a root module than it
	 * may not be unique so we create a mangled name version instead. */
      if (ivl_scope_params(scope) && ! root) {
	    unsigned idx;
	    size_t len = strlen(ivl_scope_name(scope)) +
	                 strlen(ivl_scope_tname(scope)) + 2;
	    name = (char *)malloc(len);
	    (void) strcpy(name, ivl_scope_tname(scope));
	    (void) strcat(name, "_");
	    (void) strcat(name, ivl_scope_name(scope));
	    assert(name[len-1] == 0);
	    for (idx = 0; idx < len; idx += 1) {
		  if (name[idx] == '.') name[idx] = '_';
	    }
      } else {
	    name = strdup(ivl_scope_tname(scope));
      }
      return name;
}

static unsigned emit_drive(ivl_drive_t drive)
{
      switch (drive) {
	case IVL_DR_HiZ:
	    fprintf(vlog_out, "highz");
	    break;
	case IVL_DR_WEAK:
	    fprintf(vlog_out, "weak");
	    break;
	case IVL_DR_PULL:
	    fprintf(vlog_out, "pull");
	    break;
	case IVL_DR_STRONG:
	    fprintf(vlog_out, "strong");
	    break;
	case IVL_DR_SUPPLY:
	    fprintf(vlog_out, "supply");
	    break;
	default:
	    return 1;
	    break;
      }
      return 0;
}

static void emit_gate_strength(ivl_net_logic_t nlogic)
{
      ivl_drive_t drive1 = ivl_logic_drive1(nlogic);
      ivl_drive_t drive0 = ivl_logic_drive0(nlogic);
      if ((drive1 != IVL_DR_STRONG) || (drive0 != IVL_DR_STRONG)) {
	    fprintf(vlog_out, " (");
	    if (emit_drive(drive1)) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unsupported gate "
		                  "1 drive (%d)\n", ivl_logic_file(nlogic),
		                  ivl_logic_lineno(nlogic), (int)drive1);
		  vlog_errors += 1;
	    }
	    fprintf(vlog_out, "1, ");
	    if (emit_drive(drive0)) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unsupported gate "
		                  "0 drive (%d)\n", ivl_logic_file(nlogic),
		                  ivl_logic_lineno(nlogic), (int)drive0);
		  vlog_errors += 1;
	    }
	    fprintf(vlog_out, "0)");
      }
}

static void emit_lpm_strength(ivl_lpm_t lpm)
{
      ivl_drive_t drive1 = ivl_lpm_drive1(lpm);
      ivl_drive_t drive0 = ivl_lpm_drive0(lpm);
      if (drive0 != IVL_DR_STRONG || drive1 != IVL_DR_STRONG) {
	    fprintf(vlog_out, " (");
	    if (emit_drive(drive1)) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unsupported LPM "
		                  "1 drive (%d)\n", ivl_lpm_file(lpm),
		                  ivl_lpm_lineno(lpm), (int)drive1);
		  vlog_errors += 1;
	    }
	    fprintf(vlog_out, "1, ");
	    emit_drive(drive0);
	    if (emit_drive(drive0)) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unsupported LPM "
		                  "0 drive (%d)\n", ivl_lpm_file(lpm),
		                  ivl_lpm_lineno(lpm), (int)drive0);
		  vlog_errors += 1;
	    }
	    fprintf(vlog_out, "0)");
      }
}

static void emit_delay(ivl_scope_t scope, ivl_expr_t rise, ivl_expr_t fall,
                       ivl_expr_t decay, unsigned dly_count)
{
      assert((dly_count >= 2) && (dly_count <= 3));

	/* No delays. */
      if (! rise) {
	    assert(! fall);
	    assert(! decay);
	    return;
      }
	/* If all three delays match then we only have a single delay. */
      if ((rise == fall) && (rise == decay)) {
	    fprintf(vlog_out, " #(");
	    emit_scaled_delayx(scope, rise);
	    fprintf(vlog_out, ")");
	    return;
      }
	/* If we have a gate that only supports two delays then print them. */
      if (dly_count == 2) {
	    fprintf(vlog_out, " #(");
	    emit_scaled_delayx(scope, rise);
	    fprintf(vlog_out, ", ");
	    emit_scaled_delayx(scope, fall);
	    fprintf(vlog_out, ")");
	    return;
      }

	/* What's left is a gate that supports three delays. */
      fprintf(vlog_out, " #(");
      emit_scaled_delayx(scope, rise);
      fprintf(vlog_out, ", ");
      emit_scaled_delayx(scope, fall);
      if (decay) {
	    fprintf(vlog_out, ", ");
	    emit_scaled_delayx(scope, decay);
      }
      fprintf(vlog_out, ")");
}

unsigned is_local_nexus(ivl_scope_t scope, ivl_nexus_t nex)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);
      unsigned is_local = 0;
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(nex_ptr);
	    if (! sig) continue;
	    if (scope != ivl_signal_scope(sig)) continue;
	    if ((ivl_nexus_ptr_drive1(nex_ptr) != IVL_DR_HiZ) ||
	        (ivl_nexus_ptr_drive0(nex_ptr) != IVL_DR_HiZ)) continue;
	    if (ivl_signal_local(sig)) {
//		  assert(! is_local);
		  is_local = 1;
	    } else {
		  is_local = 0;
		  break;
	    }
      }
      return is_local;
}

/*
 * This returns the nexus of the LPM if it is not a local signal.
 */
ivl_nexus_t get_lpm_output(ivl_scope_t scope, ivl_lpm_t lpm)
{
      ivl_nexus_t output = 0;
      switch (ivl_lpm_type(lpm)) {
	case IVL_LPM_ADD:
	case IVL_LPM_ARRAY:
	case IVL_LPM_CAST_INT:
	case IVL_LPM_CAST_INT2:
	case IVL_LPM_CAST_REAL:
	case IVL_LPM_CMP_EEQ:
	case IVL_LPM_CMP_EQ:
	case IVL_LPM_CMP_GE:
	case IVL_LPM_CMP_GT:
	case IVL_LPM_CMP_NE:
	case IVL_LPM_CMP_NEE:
	case IVL_LPM_CONCAT:
	case IVL_LPM_DIVIDE:
	case IVL_LPM_MOD:
	case IVL_LPM_MULT:
	case IVL_LPM_MUX:
	case IVL_LPM_PART_PV:
	case IVL_LPM_PART_VP:
	case IVL_LPM_RE_AND:
	case IVL_LPM_RE_NAND:
	case IVL_LPM_RE_NOR:
	case IVL_LPM_RE_OR:
	case IVL_LPM_RE_XOR:
	case IVL_LPM_RE_XNOR:
	case IVL_LPM_REPEAT:
	case IVL_LPM_SFUNC:
	case IVL_LPM_SHIFTL:
	case IVL_LPM_SHIFTR:
	case IVL_LPM_SIGN_EXT:
	case IVL_LPM_SUB:
	case IVL_LPM_UFUNC:
	      /* If the output of this LPM is a local signal then something
	       * else will request that this be emitted. */
	    output = ivl_lpm_q(lpm);
	    if (is_local_nexus(scope, output)) return 0;
	    break;
	default:
// HERE: Can this be a simple assert at some point in time?
	    fprintf(vlog_out, "<unknown>");
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown LPM type (%d).\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm),
	                    (int)ivl_lpm_type(lpm));
	    vlog_errors += 1;
	    return 0;
      }
      return output;
}

static void emit_logic_as_ca(ivl_scope_t scope, ivl_net_logic_t nlogic);
static void emit_lpm_as_ca(ivl_scope_t scope, ivl_lpm_t lpm);

static void emit_nexus_as_ca(ivl_scope_t scope, ivl_nexus_t nex)
{
	/* A local nexus only has a single driver. */
      if (is_local_nexus(scope, nex)) {
	    unsigned idx, count = ivl_nexus_ptrs(nex);
	    ivl_lpm_t lpm = 0;
	    ivl_net_const_t net_const = 0;
	    ivl_net_logic_t net_logic = 0;
	    ivl_signal_t sig = 0;
	    for (idx = 0; idx < count; idx += 1) {
		  ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
		  if ((ivl_nexus_ptr_drive1(nex_ptr) == IVL_DR_HiZ) &&
		      (ivl_nexus_ptr_drive0(nex_ptr) == IVL_DR_HiZ)) continue;
		  ivl_lpm_t t_lpm = ivl_nexus_ptr_lpm(nex_ptr);
		  ivl_net_const_t t_net_const = ivl_nexus_ptr_con(nex_ptr);
		  ivl_net_logic_t t_net_logic = ivl_nexus_ptr_log(nex_ptr);
		  ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
		  if (t_lpm) {
			assert(! lpm);
			lpm = t_lpm;
		  }
		  if (t_net_const) {
			assert(! net_const);
			net_const = t_net_const;
		  }
		  if (t_net_logic) {
			assert(! net_logic);
			net_logic = t_net_logic;
		  }
		  if (t_sig) {
			assert(! sig);
			sig = t_sig;
		  }
	    }
	    if (lpm) {
		  assert(! net_const);
		  assert(! net_logic);
		  assert(! sig);
		  fprintf(vlog_out, "(");
		  emit_lpm_as_ca(scope, lpm);
		  fprintf(vlog_out, ")");
	    } else if (net_const) {
		  assert( !net_logic);
		  assert(! sig);
		  emit_const_nexus(scope, net_const);
	    } else if (net_logic) {
		  assert(! sig);
		  fprintf(vlog_out, "(");
		  emit_logic_as_ca(scope, net_logic);
		  fprintf(vlog_out, ")");
	    } else if (sig) {
		  emit_name_of_nexus(scope, nex);
// HERE: The assert causes pr1703959 to fail.
//	    } else assert(0);
	    } else fprintf(vlog_out, "<missing>");
      } else {
	    emit_name_of_nexus(scope, nex);
      }
}

static void emit_logic_as_ca(ivl_scope_t scope, ivl_net_logic_t nlogic)
{
// HERE: Do we need to check that the pin count is correct for these?
      switch (ivl_logic_type(nlogic)) {
	case IVL_LO_AND:
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 0));
	    fprintf(vlog_out, " & ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    break;
	case IVL_LO_BUF:
//	case IVL_LO_BUFT:
	case IVL_LO_BUFZ:
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 0));
	    break;
	case IVL_LO_NAND:
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 0));
	    fprintf(vlog_out, " ~& ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    break;
	case IVL_LO_NOR:
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 0));
	    fprintf(vlog_out, " ~| ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    break;
	case IVL_LO_NOT:
	    fprintf(vlog_out, "~ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 0));
	case IVL_LO_OR:
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 0));
	    fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    break;
	case IVL_LO_XNOR:
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 0));
	    fprintf(vlog_out, " ~^ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    break;
	case IVL_LO_XOR:
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 0));
	    fprintf(vlog_out, " ^ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    break;
	default:
	    fprintf(vlog_out, "<unknown>");
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown logic type (%d).\n",
	                    ivl_logic_file(nlogic), ivl_logic_lineno(nlogic),
	                    (int)ivl_logic_type(nlogic));
	    vlog_errors += 1;
      }
}

static void emit_lpm_array(ivl_scope_t scope, ivl_lpm_t lpm)
{
      ivl_signal_t sig = ivl_lpm_array(lpm);
      emit_scope_module_path(scope, ivl_signal_scope(sig));
      fprintf(vlog_out, "%s[", ivl_signal_basename(sig));
// HERE : Need to scale this to match array base.
      emit_nexus_as_ca(scope, ivl_lpm_select(lpm));
      fprintf(vlog_out, "]");
}

static void emit_lpm_concat(ivl_scope_t scope, ivl_lpm_t lpm)
{
      unsigned idx, count= ivl_lpm_size(lpm);
      fprintf(vlog_out, "{");
      for (idx = count-1; idx > 0; idx -= 1) {
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, idx));
	    fprintf(vlog_out, ", ");
      }
      emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
      fprintf(vlog_out, "}");
}

ivl_signal_t nexus_is_signal(ivl_scope_t scope, ivl_nexus_t nex)
{
      return 0;
}

static void emit_lpm_part_select(ivl_scope_t scope, ivl_lpm_t lpm)
{
      unsigned width = ivl_lpm_width(lpm);
      unsigned base = ivl_lpm_base(lpm);
      ivl_signal_t sig = nexus_is_signal(scope, ivl_lpm_data(lpm, 0));
      emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
      if (! sig) return;
// HERE: We need the signal MSB/LSB to scale the select correctly.
      fprintf(vlog_out, "[");
      if (width == 1) {
	    ivl_nexus_t sel = ivl_lpm_data(lpm, 1);
	    if (sel) emit_nexus_as_ca(scope, sel);
	    else fprintf(vlog_out, "%u", base);
      } else {
// HERE: No support for a variable select.
	    fprintf(vlog_out, "%u", base+width-1);
	    fprintf(vlog_out, ":");
	    fprintf(vlog_out, "%u", base);
      }
      fprintf(vlog_out, "]");
}

// HERE: No support for trigger. Is this actually needed?
static void emit_lpm_func(ivl_scope_t scope, ivl_lpm_t lpm, const char *name)
{
      unsigned idx, count= ivl_lpm_size(lpm);
      fprintf(vlog_out, "%s(", name);;
      for (idx = count-1; idx > 0; idx -= 1) {
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, idx));
	    fprintf(vlog_out, ", ");
      }
      emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
      fprintf(vlog_out, ")");
}

static void emit_lpm_as_ca(ivl_scope_t scope, ivl_lpm_t lpm)
{
      switch (ivl_lpm_type(lpm)) {
	case IVL_LPM_ADD:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " + ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_ARRAY:
	    emit_lpm_array(scope, lpm);
	    break;
	case IVL_LPM_CAST_INT:
	case IVL_LPM_CAST_INT2:
	case IVL_LPM_CAST_REAL:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_CMP_EEQ:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " === ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_CMP_EQ:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " == ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_CMP_GE:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " >= ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_CMP_GT:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " > ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_CMP_NE:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " != ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_CMP_NEE:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " !== ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_CONCAT:
	    emit_lpm_concat(scope, lpm);
	    break;
	case IVL_LPM_DIVIDE:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " / ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_MOD:
// HERE: Need to check if this LPM is IVL_VT_REAL.
	    if (0) {
		  fprintf(stderr, "%s:%u: vlog95 error: Real modulus operator "
		                  "is not supported.\n",
		                  ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
		  vlog_errors += 1;
	    }
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " %% ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_MULT:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " * ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_MUX:
	    emit_nexus_as_ca(scope, ivl_lpm_select(lpm));
	    fprintf(vlog_out, " ? ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, " : ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_PART_PV:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_PART_VP:
	    emit_lpm_part_select(scope, lpm);
	    break;
	case IVL_LPM_RE_AND:
	    fprintf(vlog_out, " & ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_RE_NAND:
	    fprintf(vlog_out, " ~& ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_RE_NOR:
	    fprintf(vlog_out, " ~| ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_RE_OR:
	    fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_RE_XOR:
	    fprintf(vlog_out, " ^ ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_RE_XNOR:
	    fprintf(vlog_out, " ~^ ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_REPEAT:
	    fprintf(vlog_out, "{%u{", ivl_lpm_size(lpm));
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, "}}");
	    break;
	case IVL_LPM_SFUNC:
	    emit_lpm_func(scope, lpm, ivl_lpm_string(lpm));
	    break;
	case IVL_LPM_SHIFTL:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " << ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_SHIFTR:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " >> ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_SIGN_EXT:
// HERE: Do we need to extend here?
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_SUB:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " - ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    break;
	case IVL_LPM_UFUNC:
	    emit_lpm_func(scope, lpm, ivl_scope_name(ivl_lpm_define(lpm)));
	    break;
	default:
	    fprintf(vlog_out, "<unknown>");
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown LPM type (%d).\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm),
	                    (int)ivl_lpm_type(lpm));
	    vlog_errors += 1;
      }
}

static void emit_lpm(ivl_scope_t scope, ivl_lpm_t lpm)
{
      ivl_nexus_t output = get_lpm_output(scope, lpm);
	/* If the output is local then someone else will output this lpm. */
      if (! output) return;
      fprintf(vlog_out, "%*cassign", indent, ' ');
      emit_lpm_strength(lpm);
      emit_delay(scope,
                 ivl_lpm_delay(lpm, 0),
                 ivl_lpm_delay(lpm, 1),
                 ivl_lpm_delay(lpm, 2),
                 3);
      fprintf(vlog_out, " ");
      emit_name_of_nexus(scope, output);
      fprintf(vlog_out, " = ");
      emit_lpm_as_ca(scope, lpm);
      fprintf(vlog_out, ";\n");
}

/*
 * A BUFZ is a simple variable assignment possible with strength and/or delay.
 */
static void emit_bufz(ivl_scope_t scope, ivl_net_logic_t nlogic)
{
      assert(ivl_logic_pins(nlogic) == 2);
      fprintf(vlog_out, "assign");
      emit_gate_strength(nlogic);
      emit_delay(scope,
                 ivl_logic_delay(nlogic, 0),
                 ivl_logic_delay(nlogic, 1),
                 ivl_logic_delay(nlogic, 2),
                 3);
      fprintf(vlog_out, " ");
      emit_name_of_nexus(scope, ivl_logic_pin(nlogic, 0));
      fprintf(vlog_out, " = ");
      emit_name_of_nexus(scope, ivl_logic_pin(nlogic, 1));
      fprintf(vlog_out, ";\n");
}

static void emit_logic(ivl_scope_t scope, ivl_net_logic_t nlogic)
{
      unsigned idx, count, dly_count;
      fprintf(vlog_out, "%*c", indent, ' ');
      switch (ivl_logic_type(nlogic)) {
	case IVL_LO_AND:
            fprintf(vlog_out, "and");
            dly_count = 2;
	    break;
	case IVL_LO_BUF:
            fprintf(vlog_out, "buf");
            dly_count = 2;
	    break;
	case IVL_LO_BUFIF0:
            fprintf(vlog_out, "bufif0");
            dly_count = 3;
	    break;
	case IVL_LO_BUFIF1:
            fprintf(vlog_out, "bufif1");
            dly_count = 3;
	    break;
//	case IVL_LO_BUFT:
	case IVL_LO_BUFZ:
	    emit_bufz(scope, nlogic);
	    return;
	case IVL_LO_CMOS:
            fprintf(vlog_out, "cmos");
            dly_count = 3;
	    break;
	case IVL_LO_NAND:
            fprintf(vlog_out, "nand");
            dly_count = 2;
	    break;
	case IVL_LO_NMOS:
            fprintf(vlog_out, "nmos");
            dly_count = 3;
	    break;
	case IVL_LO_NOR:
            fprintf(vlog_out, "nor");
            dly_count = 2;
	    break;
	case IVL_LO_NOT:
            fprintf(vlog_out, "not");
            dly_count = 2;
	    break;
	case IVL_LO_NOTIF0:
            fprintf(vlog_out, "notif0");
            dly_count = 3;
	    break;
	case IVL_LO_NOTIF1:
            fprintf(vlog_out, "notif1");
            dly_count = 3;
	    break;
	case IVL_LO_OR:
            fprintf(vlog_out, "or");
            dly_count = 2;
	    break;
	case IVL_LO_PMOS:
            fprintf(vlog_out, "pmos");
            dly_count = 3;
	    break;
	case IVL_LO_PULLDOWN:
            fprintf(vlog_out, "pulldown");
            dly_count = 0;
	    break;
	case IVL_LO_PULLUP:
            fprintf(vlog_out, "pullup");
            dly_count = 0;
	    break;
	case IVL_LO_RCMOS:
            fprintf(vlog_out, "rcmos");
            dly_count = 3;
	    break;
	case IVL_LO_RNMOS:
            fprintf(vlog_out, "rnmos");
            dly_count = 3;
	    break;
	case IVL_LO_RPMOS:
            fprintf(vlog_out, "rpmos");
            dly_count = 3;
	    break;
	case IVL_LO_XNOR:
            fprintf(vlog_out, "xnor");
            dly_count = 2;
	    break;
	case IVL_LO_XOR:
            fprintf(vlog_out, "xor");
            dly_count = 2;
	    break;
	default:
// HERE: Missing support for UDP.
            fprintf(vlog_out, "<unknown>(");
	    fprintf(stderr, "%s:%u: vlog95 error: Unsupported logic type "
	                    "(%d) named: %s.\n", ivl_logic_file(nlogic),
	                    ivl_logic_lineno(nlogic), ivl_logic_type(nlogic),
	                    ivl_logic_basename(nlogic));
            vlog_errors += 1;
            dly_count = 0;
	    break;
      }
      emit_gate_strength(nlogic);
      if (dly_count) emit_delay(scope,
                                ivl_logic_delay(nlogic, 0),
                                ivl_logic_delay(nlogic, 1),
                                ivl_logic_delay(nlogic, 2),
                                dly_count);
// HERE: The name has the location information encoded in it. We need to
//       remove this and rebuild the instance array. For now skip the name.
//      fprintf(vlog_out, " %s(", ivl_logic_basename(nlogic));
      fprintf(vlog_out, " (");
      count = ivl_logic_pins(nlogic);
      count -= 1;
      for (idx = 0; idx < count; idx += 1) {
	    emit_name_of_nexus(scope, ivl_logic_pin(nlogic, idx));
	    fprintf(vlog_out, ", ");
      }
      emit_name_of_nexus(scope, ivl_logic_pin(nlogic, count));
      fprintf(vlog_out, ");\n");
}

/*
 * This function is called for each process in the design so that we
 * can extract the processes for the given scope.
 */
static int find_process(ivl_process_t proc, ivl_scope_t scope)
{
      if (scope == ivl_process_scope(proc)) emit_process(scope, proc);
      return 0;
}

void emit_scope_variables(ivl_scope_t scope)
{
      unsigned idx, count;
	/* Output the parameters for this scope. */
      count = ivl_scope_params(scope);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_parameter_t par = ivl_scope_param(scope, idx);
	    ivl_expr_t pex = ivl_parameter_expr(par);
	    fprintf(vlog_out, "%*cparameter %s = ", indent, ' ',
	                      ivl_parameter_basename(par));
	    emit_expr(scope, pex, 0);
	    fprintf(vlog_out, ";\n");
      }
      if (count) fprintf(vlog_out, "\n");

	/* Output the signals for this scope. */
      count = ivl_scope_sigs(scope);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, idx);
	    if (ivl_signal_type(sig) == IVL_SIT_REG) {
		    /* Do not output the implicit function return register. */
		  if (ivl_scope_type(scope) == IVL_SCT_FUNCTION &&
                      strcmp(ivl_signal_basename(sig),
		              ivl_scope_tname(scope)) == 0) continue;
		  emit_var_def(sig);
	    } else {
		  emit_net_def(sig);
	    }
      }
      if (count) fprintf(vlog_out, "\n");

	/* Output the named events for this scope. */
      count = ivl_scope_events(scope);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_event_t event = ivl_scope_event(scope, idx);
	      /* If this event has any type of edge sensitivity then it is
	       * not a named event. */
	    if (ivl_event_nany(event)) continue;
	    if (ivl_event_npos(event)) continue;
	    if (ivl_event_nneg(event)) continue;
	    fprintf(vlog_out, "%*cevent %s;\n", indent, ' ',
	                      ivl_event_basename(event));
      }
      if (count) fprintf(vlog_out, "\n");
}

/*
 * This search method may be slow for a large structural design with a
 * large number of gate types. That's not what this converter was built
 * for so this is probably OK. If this becomes an issue then we need a
 * better method/data structure.
*/
static const char **scopes_emitted = 0;
static unsigned num_scopes_emitted = 0;

static unsigned scope_has_been_emitted(ivl_scope_t scope)
{
      unsigned idx;
      for (idx = 0; idx < num_scopes_emitted; idx += 1) {
	    if (! strcmp(ivl_scope_tname(scope), scopes_emitted[idx])) return 1;
      }
      return 0;
}

static void add_scope_to_list(ivl_scope_t scope)
{
      num_scopes_emitted += 1;
      scopes_emitted = realloc(scopes_emitted, num_scopes_emitted *
                                               sizeof(char *));
      scopes_emitted[num_scopes_emitted-1] = ivl_scope_tname(scope);
}

void free_emitted_scope_list()
{
      free(scopes_emitted);
      scopes_emitted = 0;
      num_scopes_emitted = 0;
}

/*
 * A list of module scopes that need to have their definition emitted when
 * the current root scope (module) is finished is kept here.
 */
static ivl_scope_t *scopes_to_emit = 0;
static unsigned num_scopes_to_emit = 0;
static unsigned emitting_scopes = 0;

int emit_scope(ivl_scope_t scope, ivl_scope_t parent)
{
      ivl_scope_type_t sc_type = ivl_scope_type(scope);
      unsigned is_auto = ivl_scope_is_auto(scope);
      unsigned idx, count, start = 0;
      char *name;

	/* Output the scope definition. */
      switch (sc_type) {
	case IVL_SCT_MODULE:
	    assert(!is_auto);
	    name = get_mangled_name(scope, !parent && !emitting_scopes);
	      /* This is an instantiation. */
	    if (parent) {
		  assert(indent != 0);
		    /* If the module has parameters than it may not be unique
		     * so we create a mangled name version instead. */
		  fprintf(vlog_out, "\n%*c%s %s(", indent, ' ', name,
		                    ivl_scope_basename(scope));
// HERE: Still need to add port information.
		  fprintf(vlog_out, ");\n");
		  free(name);
		  num_scopes_to_emit += 1;
		  scopes_to_emit = realloc(scopes_to_emit, num_scopes_to_emit *
		                                           sizeof(ivl_scope_t));
		  scopes_to_emit[num_scopes_to_emit-1] = scope;
		  return 0;
	    }
	    assert(indent == 0);
	      /* Set the time scale for this scope. */
	    fprintf(vlog_out, "\n`timescale %s/%s\n",
	                      get_time_const(ivl_scope_time_units(scope)),
	                      get_time_const(ivl_scope_time_precision(scope)));
	    if (ivl_scope_is_cell(scope)) {
		  fprintf(vlog_out, "`celldefine\n");
	    }
	    fprintf(vlog_out, "/* This module was originally defined in "
	                      "file %s at line %u. */\n",
	                      ivl_scope_def_file(scope),
	                      ivl_scope_def_lineno(scope));
	    fprintf(vlog_out, "module %s", name);
	    free(name);
// HERE: Still need to add port information.
	    break;
	case IVL_SCT_FUNCTION:
	    assert(indent != 0);
	    fprintf(vlog_out, "\n%*cfunction", indent, ' ');
	    assert(ivl_scope_ports(scope) >= 2);
	      /* The function return information is the zero port. */
	    emit_func_return(ivl_scope_port(scope, 0));
	    start = 1;
	    fprintf(vlog_out, " %s", ivl_scope_tname(scope));
	    if (is_auto) {
		  fprintf(stderr, "%s:%u: vlog95 error: Automatic functions "
	                    "(%s) are not supported.\n", ivl_scope_file(scope),
	                    ivl_scope_lineno(scope), ivl_scope_tname(scope));
		  vlog_errors += 1;
	    }
	    break;
	case IVL_SCT_TASK:
	    assert(indent != 0);
	    fprintf(vlog_out, "\n%*ctask %s", indent, ' ',
	                      ivl_scope_tname(scope));
	    if (is_auto) {
		  fprintf(stderr, "%s:%u: vlog95 error: Automatic tasks "
	                    "(%s) are not supported.\n", ivl_scope_file(scope),
	                    ivl_scope_lineno(scope), ivl_scope_tname(scope));
		  vlog_errors += 1;
	    }
	    break;
	case IVL_SCT_BEGIN:
	case IVL_SCT_FORK:
	    assert(indent != 0);
	    return 0; /* A named begin/fork is handled in line. */
	default:
	    fprintf(stderr, "%s:%u: vlog95 error: Unsupported scope type "
	                    "(%d) named: %s.\n", ivl_scope_file(scope),
	                    ivl_scope_lineno(scope), sc_type,
	                    ivl_scope_tname(scope));
	    vlog_errors += 1;
	    return 0;
      }
      fprintf(vlog_out, ";\n");
      indent += indent_incr;

	/* Output the scope ports for this scope. */
      count = ivl_scope_ports(scope);
      for (idx = start; idx < count; idx += 1) {
	    fprintf(vlog_out, "%*c", indent, ' ');
	    ivl_signal_t port = ivl_scope_port(scope, idx);
	    switch (ivl_signal_port(port)) {
	      case IVL_SIP_INPUT:
		  fprintf(vlog_out, "input");
		  break;
	      case IVL_SIP_OUTPUT:
		  fprintf(vlog_out, "output");
		  break;
	      case IVL_SIP_INOUT:
		  fprintf(vlog_out, "inout");
		  break;
	      default:
		  fprintf(vlog_out, "<unknown>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unknown port "
	                    "direction (%d) for signal %s.\n",
	                    ivl_signal_file(port), ivl_signal_lineno(port),
	                    (int)ivl_signal_port(port),
		            ivl_signal_basename(port));
		  vlog_errors += 1;
		  break;
	    }
	    fprintf(vlog_out, " %s;\n", ivl_signal_basename(port));
      }
      if (count) fprintf(vlog_out, "\n");

      emit_scope_variables(scope);

      if (sc_type == IVL_SCT_MODULE) {
	      /* Output the LPM devices. */
	    count = ivl_scope_lpms(scope);
	    for (idx = 0; idx < count; idx += 1) {
		  emit_lpm(scope, ivl_scope_lpm(scope, idx));
	    }

	      /* Output any logic devices. */
	    count = ivl_scope_logs(scope);
	    for (idx = 0; idx < count; idx += 1) {
		  emit_logic(scope, ivl_scope_log(scope, idx));
	    }

	      /* Output the initial/always blocks for this module. */
	    ivl_design_process(design, (ivl_process_f)find_process, scope);
      }

	/* Output the function/task body. */
      if (sc_type == IVL_SCT_TASK || sc_type == IVL_SCT_FUNCTION) {
	    emit_stmt(scope, ivl_scope_def(scope));
      }

	/* Now print out any sub-scopes. */
      ivl_scope_children(scope, (ivl_scope_f*) emit_scope, scope);

	/* Output the scope ending. */
      assert(indent >= indent_incr);
      indent -= indent_incr;
      switch (sc_type) {
	case IVL_SCT_MODULE:
	    assert(indent == 0);
	    fprintf(vlog_out, "endmodule  /* %s */\n", ivl_scope_tname(scope));
	    if (ivl_scope_is_cell(scope)) {
		  fprintf(vlog_out, "`endcelldefine\n");
	    }
	      /* If this is a root scope then emit any saved instance scopes.
	       * Save any scope that does not have parameters/a mangled name
	       * to a list so we don't print duplicate module definitions. */
	    if (!emitting_scopes) {
		  emitting_scopes = 1;
		  for (idx =0; idx < num_scopes_to_emit; idx += 1) {
			ivl_scope_t scope_to_emit = scopes_to_emit[idx];
			if (scope_has_been_emitted(scope_to_emit)) continue;
			(void) emit_scope(scope_to_emit, 0);
			  /* If we used a mangled name then the instance is
			   * unique so don't add it to the list. */
			if (ivl_scope_params(scope_to_emit)) continue;
			add_scope_to_list(scope_to_emit);
		  }
		  free(scopes_to_emit);
		  scopes_to_emit = 0;
		  num_scopes_to_emit = 0;
		  emitting_scopes = 0;
	    }
	    break;
	case IVL_SCT_FUNCTION:
	    fprintf(vlog_out, "%*cendfunction  /* %s */\n", indent, ' ',
	                      ivl_scope_tname(scope));
	    break;
	case IVL_SCT_TASK:
	    fprintf(vlog_out, "%*cendtask  /* %s */\n", indent, ' ',
	                      ivl_scope_tname(scope));
	    break;
	default:
	    assert(0);
	    break;
      }
      return 0;
}
