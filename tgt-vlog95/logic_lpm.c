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

# include "config.h"
# include "vlog95_priv.h"

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

/*
 * If the strength type is 2 then emit both strengths. If it is 1 then only
 * emit the 1 strength (pullup) and if it is 0 only emit the 0 strength
 * (pulldown).
 */
static void emit_strength(ivl_drive_t drive1, ivl_drive_t drive0,
                          unsigned strength_type, const char *type,
                          const char *file, unsigned lineno)
{
      assert(strength_type <= 2);
      if ((drive1 != IVL_DR_STRONG) || (drive0 != IVL_DR_STRONG)) {
	    fprintf(vlog_out, " (");
	    if (strength_type > 0) {
		  if (emit_drive(drive1)) {
			fprintf(vlog_out, "<invalid>");
			fprintf(stderr, "%s:%u: vlog95 error: Unsupported %s "
			                "1 drive (%d)\n", file, lineno,
			                type, (int)drive1);
			vlog_errors += 1;
		  }
		  fprintf(vlog_out, "1");
	    }
	    if (strength_type == 2) fprintf(vlog_out, ", ");
	    if ((strength_type & 0x01) == 0) {
		  if (emit_drive(drive0)) {
			fprintf(vlog_out, "<invalid>");
			fprintf(stderr, "%s:%u: vlog95 error: Unsupported %s "
			                "0 drive (%d)\n", file, lineno,
			                type, (int)drive1);
			vlog_errors += 1;
		  }
		  fprintf(vlog_out, "0");
	    }
	    fprintf(vlog_out, ")");
      }
}

static void emit_gate_strength(ivl_net_logic_t nlogic, unsigned strength_type)
{
      emit_strength(ivl_logic_drive1(nlogic), ivl_logic_drive0(nlogic),
                    strength_type,
                    "gate", ivl_logic_file(nlogic), ivl_logic_lineno(nlogic));
}

static void emit_lpm_strength(ivl_lpm_t lpm)
{
      emit_strength(ivl_lpm_drive1(lpm), ivl_lpm_drive0(lpm), 2,
                    "LPM", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
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

void emit_lpm(ivl_scope_t scope, ivl_lpm_t lpm)
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
      emit_gate_strength(nlogic, 2);
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

void emit_logic(ivl_scope_t scope, ivl_net_logic_t nlogic)
{
      unsigned idx, count, dly_count, strength_type = 2;
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
            strength_type = 0;
	    break;
	case IVL_LO_PULLUP:
            fprintf(vlog_out, "pullup");
            dly_count = 0;
            strength_type = 1;
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
      emit_gate_strength(nlogic, strength_type);
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

void emit_tran(ivl_scope_t scope, ivl_switch_t tran)
{
      unsigned dly_count, pins;
      fprintf(vlog_out, "%*c", indent, ' ');
      switch (ivl_switch_type(tran)) {
	case IVL_SW_RTRAN:
            fprintf(vlog_out, "rtran");
            dly_count = 0;
            pins = 2;
	    break;
	case IVL_SW_RTRANIF0:
            fprintf(vlog_out, "rtranif0");
            dly_count = 3;
            pins = 3;
	    break;
	case IVL_SW_RTRANIF1:
            fprintf(vlog_out, "rtranif1");
            dly_count = 3;
            pins = 3;
	    break;
	case IVL_SW_TRAN:
            fprintf(vlog_out, "tran");
            dly_count = 0;
            pins = 2;
	    break;
	case IVL_SW_TRANIF0:
            fprintf(vlog_out, "tranif0");
            dly_count = 3;
            pins = 3;
	    break;
	case IVL_SW_TRANIF1:
            fprintf(vlog_out, "tranif1");
            dly_count = 3;
            pins = 3;
	    break;
	case IVL_SW_TRAN_VP:
	default:
            fprintf(vlog_out, "<missing>");
	    fprintf(stderr, "%s:%u: vlog95 error: No supported for a TRAN_VP "
	                    "named: %s.\n", ivl_switch_file(tran),
	                    ivl_switch_lineno(tran),
	                    ivl_switch_basename(tran));
            dly_count = 0;
            pins = 2;
            vlog_errors += 1;
	    break;
      }
      if (dly_count) emit_delay(scope,
                                ivl_switch_delay(tran, 0),
                                ivl_switch_delay(tran, 1),
                                ivl_switch_delay(tran, 2),
                                dly_count);
      assert(pins == 2 || pins == 3);
// The same problem here as for the gates above.
//      fprintf(vlog_out, " %s(", ivl_switch_basename(tran));
      fprintf(vlog_out, " (");
      emit_name_of_nexus(scope, ivl_switch_a(tran));
      fprintf(vlog_out, ", ");
      emit_name_of_nexus(scope, ivl_switch_b(tran));
      if (pins == 3) {
	    fprintf(vlog_out, ", ");
	    emit_name_of_nexus(scope, ivl_switch_enable(tran));
      }
      fprintf(vlog_out, ");\n");
}
