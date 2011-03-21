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

/* This variable lets a select know it is really a >>> operator. */
static unsigned sign_extend = 0;

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
      if ((strength_type == 2) &&
          ((drive1 != IVL_DR_STRONG) || (drive0 != IVL_DR_STRONG))) {
	    fprintf(vlog_out, " (");
	    if (emit_drive(drive1)) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unsupported %s "
		                  "1 drive (%d)\n", file, lineno,
		                  type, (int)drive1);
		  vlog_errors += 1;
	    }
	    fprintf(vlog_out, "1, ");
	    if (emit_drive(drive0)) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unsupported %s "
		                  "0 drive (%d)\n", file, lineno,
		                  type, (int)drive0);
		  vlog_errors += 1;
	    }
	    fprintf(vlog_out, "0)");
      } else if ((strength_type == 1) && (drive1 != IVL_DR_PULL)) {
	    fprintf(vlog_out, " (");
	    if (emit_drive(drive1)) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unsupported %s "
		                  "1 drive (%d)\n", file, lineno,
		                  type, (int)drive1);
		  vlog_errors += 1;
	    }
	    fprintf(vlog_out, "1)");
      } else if ((strength_type == 0) && (drive0 != IVL_DR_PULL)) {
	    fprintf(vlog_out, " (");
	    if (emit_drive(drive0)) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unsupported %s "
		                  "0 drive (%d)\n", file, lineno,
		                  type, (int)drive0);
		  vlog_errors += 1;
	    }
	    fprintf(vlog_out, "0)");
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
	    emit_scaled_delayx(scope, rise, 0);
	    fprintf(vlog_out, ")");
	    return;
      }
	/* If we have a gate that only supports two delays then print them. */
      if (dly_count == 2) {
	    fprintf(vlog_out, " #(");
	    emit_scaled_delayx(scope, rise, 0);
	    fprintf(vlog_out, ", ");
	    emit_scaled_delayx(scope, fall, 0);
	    fprintf(vlog_out, ")");
	    return;
      }

	/* What's left is a gate that supports three delays. */
      fprintf(vlog_out, " #(");
      emit_scaled_delayx(scope, rise, 0);
      fprintf(vlog_out, ", ");
      emit_scaled_delayx(scope, fall, 0);
      if (decay) {
	    fprintf(vlog_out, ", ");
	    emit_scaled_delayx(scope, decay, 0);
      }
      fprintf(vlog_out, ")");
}

static unsigned is_local_nexus(ivl_scope_t scope, ivl_nexus_t nex)
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
static ivl_nexus_t get_lpm_output(ivl_scope_t scope, ivl_lpm_t lpm)
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
	case IVL_LPM_FF:
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

/* For an undriven port look for the local signal to get the nexus name. */
static void emit_nexus_port_signal(ivl_scope_t scope, ivl_nexus_t nex)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);
      ivl_signal_t sig = 0;
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    if ((ivl_nexus_ptr_drive1(nex_ptr) != IVL_DR_HiZ) ||
	        (ivl_nexus_ptr_drive0(nex_ptr) != IVL_DR_HiZ)) assert(0);
	    ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
	    if (t_sig) {
		  if (scope != ivl_signal_scope(t_sig)) continue;
		  assert(! sig);
		  sig = t_sig;
	    }
      }
	/* There will not be a signal for an empty port. */
      if (sig) emit_nexus_as_ca(scope, ivl_signal_nex(sig, 0));
      else fprintf(vlog_out, "/* Empty */");
}

void emit_nexus_port_driver_as_ca(ivl_scope_t scope, ivl_nexus_t nex)
{
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
	    emit_lpm_as_ca(scope, lpm);
      } else if (net_const) {
	    assert( !net_logic);
	    assert(! sig);
	    emit_const_nexus(scope, net_const);
      } else if (net_logic) {
	    assert(! sig);
	    emit_logic_as_ca(scope, net_logic);
      } else if (sig) {
	    emit_nexus_as_ca(scope, ivl_signal_nex(sig, 0));
	/* If there is no driver then look for a single signal that is
	 * driven by this nexus that has the correct scope. This is needed
	 * to translate top level ports. */
      } else {
	    emit_nexus_port_signal(scope, nex);
      }
}

void emit_nexus_as_ca(ivl_scope_t scope, ivl_nexus_t nex)
{
	/* If there is no nexus then there is nothing to print. */
      if (! nex) return;
	/* A local nexus only has a single driver. */
      if (is_local_nexus(scope, nex)) {
	    unsigned idx, count = ivl_nexus_ptrs(nex);
	    unsigned must_be_sig = 0;
	    unsigned out_of_scope_drive = 0;
	    ivl_lpm_t lpm = 0;
	    ivl_net_const_t net_const = 0;
	    ivl_net_logic_t net_logic = 0;
	    ivl_signal_t sig = 0;
	    for (idx = 0; idx < count; idx += 1) {
		  ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
		  if ((ivl_nexus_ptr_drive1(nex_ptr) == IVL_DR_HiZ) &&
		      (ivl_nexus_ptr_drive0(nex_ptr) == IVL_DR_HiZ)) {
			  /* If we only have a single input then we want
			   * the nexus this signal is driven by. */
			if (count == 1) {
			      must_be_sig = 1;
			} else continue;
		  }
		  ivl_lpm_t t_lpm = ivl_nexus_ptr_lpm(nex_ptr);
		  ivl_net_const_t t_net_const = ivl_nexus_ptr_con(nex_ptr);
		  ivl_net_logic_t t_net_logic = ivl_nexus_ptr_log(nex_ptr);
		  ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
		  if (t_lpm) {
			assert(! lpm);
			lpm = t_lpm;
		  }
		  if (t_net_const) {
			if (scope != ivl_const_scope(t_net_const)) {
// HERE: Need to verify that this is not a parameter
			      out_of_scope_drive = 1;
			}
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
		  assert(! must_be_sig);
		  emit_lpm_as_ca(scope, lpm);
	    } else if (net_const) {
		  assert( !net_logic);
		  assert(! sig);
		  assert(! must_be_sig);
		  if (out_of_scope_drive) {
// HERE: An out of scope const drive that is not a parameter is really a
//       port so look for and emit the local signal name (nexus_is_signal
//       may work). The is_local_nexus code also needs to be changed to
//       not emit the port expressions as a CA. Make sure this works
//       correctly if the parameter is passed as a port argument.
// For now report this as missing.
			fprintf(vlog_out, "<missing>");
		  } else emit_const_nexus(scope, net_const);
	    } else if (net_logic) {
		  assert(! sig);
		  assert(! must_be_sig);
		  emit_logic_as_ca(scope, net_logic);
	    } else if (sig) {
		  if (must_be_sig) {
			emit_nexus_as_ca(scope, ivl_signal_nex(sig, 0));
		  } else emit_name_of_nexus(scope, nex);
// HERE: The assert causes pr1703959 to fail.
//	    } else assert(0);
	    } else {
		  fprintf(stderr, "?:?: vlog95 error: Could not emit "
		                  "nexus as a CA.\n");
		  vlog_errors += 1;
		  fprintf(vlog_out, "<missing>");
	    }
      } else {
	    emit_name_of_nexus(scope, nex);
      }
}

static void emit_logic_as_ca(ivl_scope_t scope, ivl_net_logic_t nlogic)
{
      unsigned inputs = ivl_logic_pins(nlogic) - 1;
      switch (ivl_logic_type(nlogic)) {
	case IVL_LO_AND:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    fprintf(vlog_out, " & ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_BUF:
//	case IVL_LO_BUFT:
	case IVL_LO_BUFZ:
	    assert(inputs == 1);
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    break;
	case IVL_LO_NAND:
	    assert(inputs == 2);
	    fprintf(vlog_out, "~(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    fprintf(vlog_out, " & ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_NOR:
	    assert(inputs == 2);
	    fprintf(vlog_out, "~(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_NOT:
	    assert(inputs == 1);
	    fprintf(vlog_out, "(~ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_OR:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_XNOR:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    fprintf(vlog_out, " ~^ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_XOR:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
	    fprintf(vlog_out, " ^ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2));
	    fprintf(vlog_out, ")");
	    break;
	default:
	    fprintf(vlog_out, "<unknown>");
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown CA logic type "
	                    "(%d).\n",
	                    ivl_logic_file(nlogic), ivl_logic_lineno(nlogic),
	                    (int)ivl_logic_type(nlogic));
	    vlog_errors += 1;
      }
}

static void emit_lpm_array(ivl_scope_t scope, ivl_lpm_t lpm)
{
      ivl_signal_t sig = ivl_lpm_array(lpm);
      emit_scope_module_path(scope, ivl_signal_scope(sig));
      emit_id(ivl_signal_basename(sig));
      fprintf(vlog_out, "[");
// HERE: Need to remove the scale to match array base instead of adding it back.
      emit_nexus_as_ca(scope, ivl_lpm_select(lpm));
      fprintf(vlog_out, " + %d]", ivl_signal_array_base(sig));
}

static void emit_lpm_concat(ivl_scope_t scope, ivl_lpm_t lpm)
{
      unsigned idx, count= ivl_lpm_size(lpm);
      ivl_nexus_t nex;
      fprintf(vlog_out, "{");
// HERE: Need to check for a zero repeat that was dropped from the concat.
	/* Check to see if this is a repeat. */
      nex = ivl_lpm_data(lpm, 0);
      for (idx = 1; idx < count; idx += 1) {
	    if (nex != ivl_lpm_data(lpm, idx)) break;
      }
	/* If all the nexus match then we have a repeat. */
      if ((idx == count) && (count > 1)) {
	    fprintf(vlog_out, "%u{", count);
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, "}");
      } else {
	    for (idx = count-1; idx > 0; idx -= 1) {
		  emit_nexus_as_ca(scope, ivl_lpm_data(lpm, idx));
		  fprintf(vlog_out, ", ");
	    }
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
      }
      fprintf(vlog_out, "}");
}

static ivl_signal_t nexus_is_signal(ivl_scope_t scope, ivl_nexus_t nex,
                                    int*base, int*array_word)
{
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
		    /* The real signal could be hidden behind a select. */
		  if (ivl_lpm_type(t_lpm) == IVL_LPM_PART_VP) {
			t_sig = nexus_is_signal(scope, ivl_lpm_data(t_lpm, 0),
			                        base, array_word);
			if (t_sig) *base += ivl_lpm_base(t_lpm);
		  } else lpm = t_lpm;
	    }
	    if (t_net_const) {
		  assert(! net_const);
		  net_const = t_net_const;
	    }
	    if (t_net_logic) {
		  assert(! net_logic);
		    /* The real signal could be hidden behind a BUFZ gate. */
		  if (ivl_logic_type(t_net_logic) == IVL_LO_BUFZ) {
			assert(ivl_logic_pins(t_net_logic) == 2);
			t_sig = nexus_is_signal(scope,
			                        ivl_logic_pin(t_net_logic, 1),
			                        base, array_word);
		  } else net_logic = t_net_logic;
	    }
	    if (t_sig) {
		  assert(! sig);
		  sig = t_sig;
		    /* This could be an array word so save the word. */
		  *array_word = ivl_nexus_ptr_pin(nex_ptr);
	    }
      }
      if (sig) {
	    assert(! lpm);
	    assert(! net_const);
	    assert(! net_logic);
      }
      return sig;
}

static void emit_lpm_part_select(ivl_scope_t scope, ivl_lpm_t lpm)
{
      unsigned width = ivl_lpm_width(lpm);
      int array_word = 0;
      int base = ivl_lpm_base(lpm);
      int msb, lsb;
      ivl_signal_t sig = nexus_is_signal(scope, ivl_lpm_data(lpm, 0),
                                         &base, &array_word);

      if (sign_extend && !allow_signed) {
	    fprintf(stderr, "%s:%u: vlog95 error: >>> operator is not "
	                    "supported.\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
	    vlog_errors += 1;
      }

// HERE: variable parameter select needs to be rebuilt.
      if (! sig) {
	      /* Check if the compiler used a select for a shift. */
	    assert(base >= 0);
	    if (base) fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    if (base) {
		  fprintf(vlog_out, " ");
		  if (sign_extend) fprintf(vlog_out, ">");
		  fprintf(vlog_out, ">> %d)", base);
	    }
	    sign_extend = 0;
	    return;
      }

      if (sign_extend) fprintf(vlog_out, "(");
      emit_id(ivl_signal_basename(sig));
      if (ivl_signal_dimensions(sig)) {
	    array_word += ivl_signal_array_base(sig);
	    fprintf(vlog_out, "[%d]", array_word);
      }

      msb = ivl_signal_msb(sig);
      lsb = ivl_signal_lsb(sig);
      if (sign_extend) {
	    assert(base != lsb);
	    if (msb >= lsb) base += lsb;
	    else base = lsb - base;
	    fprintf(vlog_out, " >>> %d)", base);
	    sign_extend = 0;
	    return;
      }

      fprintf(vlog_out, "[");
      if (width == 1) {
	    ivl_nexus_t sel = ivl_lpm_data(lpm, 1);
	    if (sel) {
// HERE: Need to scale the select nexus.
		  if ((msb >= lsb) && (lsb == 0)) {
			emit_nexus_as_ca(scope, sel);
		  } else {
			fprintf(stderr, "%s:%u: vlog95 sorry: Non-zero based "
			                "variable part selects are not "
			                "supported.\n", ivl_lpm_file(lpm),
			                ivl_lpm_lineno(lpm));
			vlog_errors += 1;
			fprintf(vlog_out, "<missing>");
		  }
	    } else {
		  if (msb >= lsb) base += lsb;
		  else base = lsb - base;
		  fprintf(vlog_out, "%d", base);
	    }
      } else {
// HERE: No support for an indexed part select.
	    ivl_nexus_t sel = ivl_lpm_data(lpm, 1);
	    if (sel) {
		  fprintf(stderr, "%s:%u: vlog95 sorry: Variable indexed part "
		                  "selects are not supported.\n",
		                  ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
		  vlog_errors += 1;
		  fprintf(vlog_out, "<missing>:<missing>");
	    } else {
		  if (msb >= lsb) {
			base += lsb;
			fprintf(vlog_out, "%d:%d", base+(int)width-1, base);
		  } else {
			base = lsb - base;
			fprintf(vlog_out, "%d:%d", base-(int)width+1, base);
		  }
	    }
      }
      fprintf(vlog_out, "]");
}

// HERE: No support for trigger. Is this actually needed?
static void emit_lpm_func(ivl_scope_t scope, ivl_lpm_t lpm)
{
      unsigned count = ivl_lpm_size(lpm);
      if (count) {
	    unsigned idx;
	    fprintf(vlog_out, "(");
	    for (idx = count-1; idx > 0; idx -= 1) {
		  emit_nexus_as_ca(scope, ivl_lpm_data(lpm, idx));
		  fprintf(vlog_out, ", ");
	    }
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, ")");
      }
}

static void emit_lpm_as_ca(ivl_scope_t scope, ivl_lpm_t lpm)
{
      switch (ivl_lpm_type(lpm)) {
	case IVL_LPM_ADD:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " + ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
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
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " === ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_EQ:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " == ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_GE:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " >= ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_GT:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " > ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_NE:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " != ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_NEE:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " !== ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CONCAT:
	    emit_lpm_concat(scope, lpm);
	    break;
	case IVL_LPM_DIVIDE:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " / ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_MOD:
// HERE: Need to check if this LPM is IVL_VT_REAL.
	    if (0) {
		  fprintf(stderr, "%s:%u: vlog95 error: Real modulus operator "
		                  "is not supported.\n",
		                  ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
		  vlog_errors += 1;
	    }
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " %% ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_MULT:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " * ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_MUX:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_select(lpm));
	    fprintf(vlog_out, " ? ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, " : ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_PART_PV:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_PART_VP:
	    emit_lpm_part_select(scope, lpm);
	    break;
	case IVL_LPM_RE_AND:
	    fprintf(vlog_out, "(&");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_NAND:
	    fprintf(vlog_out, "(~&");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_NOR:
	    fprintf(vlog_out, "(~|");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_OR:
	    fprintf(vlog_out, "(|");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_XOR:
	    fprintf(vlog_out, "(^");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_XNOR:
	    fprintf(vlog_out, "(~^");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_REPEAT:
	    fprintf(vlog_out, "{%u{", ivl_lpm_size(lpm));
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, "}}");
	    break;
	case IVL_LPM_SFUNC:
	    fprintf(vlog_out, "%s", ivl_lpm_string(lpm));
	    emit_lpm_func(scope, lpm);
	    break;
	case IVL_LPM_SHIFTL:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " << ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_SHIFTR:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " ");
	    assert(! sign_extend);
	    fprintf(vlog_out, " >> ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_SIGN_EXT:
//	    assert(! sign_extend);
	    sign_extend = 1;
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    break;
	case IVL_LPM_SUB:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " - ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1));
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_UFUNC:
	    emit_scope_path(scope, ivl_lpm_define(lpm));
	    emit_lpm_func(scope, lpm);
	    break;
	default:
	    fprintf(vlog_out, "<unknown>");
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown LPM type (%d).\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm),
	                    (int)ivl_lpm_type(lpm));
	    vlog_errors += 1;
      }
}

static void emit_posedge_dff_prim()
{
      fprintf(vlog_out, "\n");
      fprintf(vlog_out, "/* Icarus generated UDP to represent a synthesized "
                        "positive edge D-FF. */\n");
      fprintf(vlog_out, "primitive IVL_posedge_DFF "
                        "(q, clk, en, d, clr, set);\n");
      fprintf(vlog_out, "%*coutput q;\n", indent_incr, ' ');
      fprintf(vlog_out, "%*cinput clk, en, d, clr, set;\n", indent_incr, ' ');
      fprintf(vlog_out, "%*creg q;\n", indent_incr, ' ');
      fprintf(vlog_out, "%*ctable\n", indent_incr, ' ');
      fprintf(vlog_out, "%*cr 1 0 0 0 : ? : 0 ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cr 1 1 0 0 : ? : 1 ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cp 1 0 0 0 : 0 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cp 1 1 0 0 : 1 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cp x 0 0 0 : 0 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cp x 1 0 0 : 1 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cn ? ? 0 0 : ? : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c* 0 ? 0 0 : ? : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? * ? ? ? : ? : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? * ? ? : ? : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? * ? : ? : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? ? * : ? : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? 0 1 : ? : 1 ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? 0 x : 1 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? 0 x : 0 : x ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? 1 ? : ? : 0 ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? x 0 : 0 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? x 0 : 1 : x ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? x x : ? : x ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c? ? ? x 1 : ? : x ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cendtable\n", indent_incr, ' ');
      fprintf(vlog_out, "endprimitive\n");
}

static unsigned need_posedge_dff_prim = 0;

/*
 * Synthesis creates a D-FF LPM object. To allow this to be simulated as
 * Verilog we need to generate a D-FF UDP that is used to represent this
 * LPM. Since this must be included with user derived code it must be
 * licensed using the lesser GPL to avoid the requirement that their code
 * also be licensed under the GPL. We print a note that LGPL code is
 * being included in the output so the user can remove it if desired.
 *
 * The general idea with all this is that we want the user to be able to
 * simulate a synthesized D-FF, etc., but we don't want them to take the
 * ideas behind the primitive(s) and claim them as their own.
 */
void emit_icarus_generated_udps()
{
	/* Emit the copyright information and LGPL note and then emit any
	 * needed primitives. */
      if (need_posedge_dff_prim) {
	    fprintf(vlog_out,
"\n"
"/*\n"
" * This is the copyright information for the following primitive(s)\n"
" * (library elements).\n"
" *\n"
" * Copyright (C) 2011 Cary R. (cygcary@yahoo.com)\n"
" *\n"
" * This library is free software; you can redistribute it and/or\n"
" * modify it under the terms of the GNU Lesser General Public\n"
" * License as published by the Free Software Foundation; either\n"
" * version 2.1 of the License, or (at your option) any later version.\n"
" *\n"
" * This library is distributed in the hope that it will be useful,\n"
" * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
" * Lesser General Public License for more details.\n"
" *\n"
" * You should have received a copy of the GNU Lesser General Public\n"
" * License along with this library; if not, write to the Free Software\n"
" * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA\n"
" */\n");
	    fprintf(stderr,
"NOTE: vlog95: Adding LGPL 2.1 primitive(s) at the end of the output file.\n");
      }
      if (need_posedge_dff_prim) emit_posedge_dff_prim();
}

static void emit_lpm_ff(ivl_scope_t scope, ivl_lpm_t lpm)
{
// HERE: No support for lpm attributes and hence polarity information.
//      ivl_attribute_t clock_pol = find_lpm_attr(lpm, "Clock:LPM_Polarity");
      ivl_attribute_t clock_pol = 0;
      ivl_expr_t aset_expr = ivl_lpm_aset_value(lpm);
      ivl_expr_t sset_expr = ivl_lpm_sset_value(lpm);
      ivl_nexus_t nex;
      unsigned emitted, have_data, have_sset;
      const char *aset_bits = 0;
      const char *sset_bits = 0;
	/* For now we only support a width of 1 for these bits. */
      if (aset_expr) {
	    assert(ivl_expr_width(aset_expr) == 1);
	    aset_bits = ivl_expr_bits(aset_expr);
      }
      if (sset_expr) {
	    assert(ivl_expr_width(sset_expr) == 1);
	    sset_bits = ivl_expr_bits(sset_expr);
      }

      fprintf(vlog_out, "%*c", indent, ' ');
	/* If there is a clock polarity attribute then we have a negative
	 * edge D-FF. */
      if (clock_pol) {
	    fprintf(vlog_out, "IVL_negedge_DFF");
      } else {
	    fprintf(vlog_out, "IVL_posedge_DFF");
      }
      emit_lpm_strength(lpm);
	/* The lpm FF does not support any delays. */
	/* The FF name is a temporary so we don't bother to print it unless
	 * we have a range. Then we need to use a made up name. */
      if (ivl_lpm_width(lpm) > 1) {
	    fprintf(vlog_out, " synth_%p [%u:0]", lpm, ivl_lpm_width(lpm)-1U);
      }
      fprintf(vlog_out, " (");
	/* Emit the q pin. */
      emit_name_of_nexus(scope, ivl_lpm_q(lpm));
      fprintf(vlog_out, ", ");
	/* Emit the clock pin. */
      emit_nexus_as_ca(scope, ivl_lpm_clk(lpm));
      fprintf(vlog_out, ", ");
	/* Emit the enable pin expression(s) if needed. */
      emitted = 0;
      nex = ivl_lpm_enable(lpm);
      if (nex) {
	    emit_nexus_as_ca(scope, nex);
	    emitted = 1;
      }
      nex = ivl_lpm_sync_clr(lpm);
      if (nex) {
	    if (emitted) fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, nex);
	    emitted = 1;
      }
      have_sset = 0;
      nex = ivl_lpm_sync_set(lpm);
      if (nex) {
	    if (emitted) fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, nex);
	    emitted = 1;
	    have_sset = 1;
      }
      if (!emitted) fprintf(vlog_out, "1'b1");
      fprintf(vlog_out, ", ");
	/* Emit the data pin expression(s). */
      have_data = ivl_lpm_data(lpm, 0) != 0;
      nex = ivl_lpm_sync_clr(lpm);
      if (nex) {
	    emit_nexus_as_ca(scope, nex);
	    if (have_data | have_sset) fprintf(vlog_out, " & ");
	    if (have_data & have_sset) fprintf(vlog_out, "(");
	    emitted = 1;
      }
      nex = ivl_lpm_sync_set(lpm);
      if (nex) {
	    if (! sset_bits || (sset_bits && (sset_bits[0] == '1'))) {
		  emit_nexus_as_ca(scope, nex);
		  if (have_data) fprintf(vlog_out, " | ");
	    } else {
		  fprintf(vlog_out, "~");
		  emit_nexus_as_ca(scope, nex);
		  if (have_data) fprintf(vlog_out, " & ");
	    }
      }
      nex = ivl_lpm_data(lpm, 0);
      if (nex) emit_nexus_as_ca(scope, nex);
      if (have_data & have_sset) fprintf(vlog_out, ")");
      fprintf(vlog_out, ", ");
	/* Emit the clear pin expression(s) if needed. */
      emitted = 0;
      nex = ivl_lpm_async_clr(lpm);
      if (nex) {
	    emit_nexus_as_ca(scope, nex);
	    emitted = 1;
      }
      nex = ivl_lpm_async_set(lpm);
      if (aset_bits && (aset_bits[0] != '0')) nex = 0;
      if (nex) {
	    if (emitted) fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, nex);
	    emitted = 1;
      }
      if (!emitted) fprintf(vlog_out, "1'b0");
      fprintf(vlog_out, ", ");
	/* Emit the set pin expression(s) if needed. */
      nex = ivl_lpm_async_set(lpm);
      if (aset_bits && (aset_bits[0] != '1')) nex = 0;
      if (nex) emit_nexus_as_ca(scope, nex);
      else fprintf(vlog_out, "1'b0");
      fprintf(vlog_out, ");\n");
	/* We need to emit a primitive for this instance. */
      need_posedge_dff_prim = 1;
}

static ivl_signal_t get_output_from_nexus(ivl_scope_t scope, ivl_nexus_t nex,
                                          int64_t*array_idx)
{
      ivl_signal_t use_sig = 0;
      unsigned is_array = 0;
      unsigned idx, count;
      count = ivl_nexus_ptrs(nex);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(nex_ptr);
	    if (! sig) continue;
            if (ivl_signal_local(sig)) {
		    /* If the local signal is another receiver skip it. */
		  if ((ivl_nexus_ptr_drive1(nex_ptr) == IVL_DR_HiZ) &&
		      (ivl_nexus_ptr_drive0(nex_ptr) == IVL_DR_HiZ)) {
			continue;
		  }
		  assert(0);
            }
	      /* The signal must be in the correct scope. */
	    if (scope != ivl_signal_scope(sig)) continue;
	      /* Since we are looking for the output signal it is a receiver. */
	    if ((ivl_nexus_ptr_drive1(nex_ptr) != IVL_DR_HiZ) &&
	        (ivl_nexus_ptr_drive0(nex_ptr) != IVL_DR_HiZ)) {
		  continue;
	    }
	    if (use_sig) {
// HERE: Which one should we use? For now it's the first one found.
//       I believe this needs to be solved (see the inout.v test).
		  fprintf(stderr, "%s:%u: vlog95 warning: Duplicate name (%s",
		                  ivl_signal_file(sig), ivl_signal_lineno(sig),
		                  ivl_signal_basename(sig));
		  if (ivl_signal_dimensions(sig) > 0) {
			int64_t tmp_idx = ivl_nexus_ptr_pin(nex_ptr);
			tmp_idx += ivl_signal_array_base(sig);
			fprintf(stderr, "[%"PRId64"]", tmp_idx);
		  }
		  fprintf(stderr, ") found for nexus (%s",
		                  ivl_signal_basename(use_sig));
		  if (is_array) fprintf(stderr, "[%"PRId64"]", *array_idx);
		  fprintf(stderr, ")\n");
	    } else {
		    /* We have a signal that can be used to find the name. */
		  use_sig = sig;
		  if (ivl_signal_dimensions(sig) > 0) {
			is_array = 1;
			*array_idx = ivl_nexus_ptr_pin(nex_ptr);
			*array_idx += ivl_signal_array_base(sig);
		  }
	    }
      }

      return use_sig;
}

static void emit_lpm_part_pv(ivl_scope_t scope, ivl_lpm_t lpm)
{
      unsigned width = ivl_lpm_width(lpm);
      int64_t array_word = 0;
      int base = ivl_lpm_base(lpm);
      int msb, lsb;
      ivl_signal_t sig = get_output_from_nexus(scope, ivl_lpm_q(lpm),
                                               &array_word);
      assert(sig);
      assert(ivl_lpm_data(lpm, 1) == 0);
      emit_id(ivl_signal_basename(sig));
      if (ivl_signal_dimensions(sig)) {
	    fprintf(vlog_out, "[%"PRId64"]", array_word);
      }
      msb = ivl_signal_msb(sig);
      lsb = ivl_signal_lsb(sig);
      fprintf(vlog_out, "[");
      if (width == 1) {
	    if (msb >= lsb) base += lsb;
	    else base = lsb - base;
	    fprintf(vlog_out, "%d", base);
      } else {
	    if (msb >= lsb) {
		  base += lsb;
		  fprintf(vlog_out, "%d:%d", base+(int)width-1, base);
	    } else {
		  base = lsb - base;
		  fprintf(vlog_out, "%d:%d", base-(int)width+1, base);
	    }
      }
      fprintf(vlog_out, "]");
}

void emit_lpm(ivl_scope_t scope, ivl_lpm_t lpm)
{
      ivl_nexus_t output = get_lpm_output(scope, lpm);
      ivl_lpm_type_t type = ivl_lpm_type(lpm);
	/* If the output is local then someone else will output this lpm. */
      if (! output) return;
	/* If the LPM is a D-FF then we need to emit it as a UDP. */
      if (type == IVL_LPM_FF) {
	    emit_lpm_ff(scope, lpm);
	    return;
      }
      fprintf(vlog_out, "%*cassign", indent, ' ');
      emit_lpm_strength(lpm);
      emit_delay(scope,
                 ivl_lpm_delay(lpm, 0),
                 ivl_lpm_delay(lpm, 1),
                 ivl_lpm_delay(lpm, 2),
                 3);
      fprintf(vlog_out, " ");
      if (type == IVL_LPM_PART_PV) emit_lpm_part_pv(scope, lpm);
      else emit_name_of_nexus(scope, output);
      fprintf(vlog_out, " = ");
      emit_lpm_as_ca(scope, lpm);
      fprintf(vlog_out, ";");
      if (emit_file_line) {
	    fprintf(vlog_out, " /* %s:%u */",
	                      ivl_lpm_file(lpm),
	                      ivl_lpm_lineno(lpm));
      }
      fprintf(vlog_out, "\n");
}

static void emit_logic_file_line(ivl_net_logic_t nlogic)
{
      if (emit_file_line) {
	    fprintf(vlog_out, " /* %s:%u */",
	                      ivl_logic_file(nlogic),
	                      ivl_logic_lineno(nlogic));
      }
}

/*
 * A BUFZ is a simple variable assignment possibly with strength and/or delay.
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
      emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1));
      fprintf(vlog_out, ";");
      emit_logic_file_line(nlogic);
      fprintf(vlog_out, "\n");
}

static void emit_and_save_udp_name(ivl_net_logic_t nlogic){
      ivl_udp_t udp = ivl_logic_udp(nlogic);
      assert(udp);
      emit_id(ivl_udp_name(udp));
      add_udp_to_list(udp);
}

static void emit_name_of_logic_nexus(ivl_scope_t scope, ivl_net_logic_t nlogic,
                                     ivl_nexus_t nex)
{
      if (nex) {
	    emit_name_of_nexus(scope, nex);
      } else {
	    if (ivl_logic_type(nlogic) != IVL_LO_UDP) {
		  fprintf(stderr, "%s:%u: vlog95 warning: Missing logic pin "
		                  "for (%d) named: %s.\n",
		                  ivl_logic_file(nlogic),
		                  ivl_logic_lineno(nlogic),
		                  ivl_logic_type(nlogic),
		                  ivl_logic_basename(nlogic));
	    }
	    fprintf(vlog_out, "1'bz");
      }
}

void emit_logic(ivl_scope_t scope, ivl_net_logic_t nlogic)
{
// HERE: We need to investigate if this is really the base of a CA. A real
//       gate only allows a signal or a signal bit select for the output(s)
//       and a scalar expression for the input. We also need to modify the
//       compiler to support logical 'and' and logical 'or' since they
//       short circuit. Verify input count.
      unsigned idx, count, dly_count, strength_type = 2;
      unsigned outputs = 1;
      unsigned width = ivl_logic_width(nlogic);
      const char *name;
	/* Skip gates that have a local nexus as the output since they are
	 * part of a continuous assignment. */
      if (is_local_nexus(scope, ivl_logic_pin(nlogic, 0))) return;
      fprintf(vlog_out, "%*c", indent, ' ');
      switch (ivl_logic_type(nlogic)) {
	case IVL_LO_AND:
            fprintf(vlog_out, "and");
            dly_count = 2;
	    break;
	case IVL_LO_BUF:
            fprintf(vlog_out, "buf");
            dly_count = 2;
            outputs = 0;
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
            outputs = 0;
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
            outputs = 0;
            strength_type = 0;
	    break;
	case IVL_LO_PULLUP:
            fprintf(vlog_out, "pullup");
            dly_count = 0;
            outputs = 0;
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
	case IVL_LO_UDP:
            emit_and_save_udp_name(nlogic);
            dly_count = 2;
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
//       remove this and rebuild the instance array. For now we just strip
//       this encoding and create an zero based range. Need to skip the
//       local names _s<digits>.
//       This can also be an escaped id.
      name = ivl_logic_basename(nlogic);
      if (name && *name) {
	    char *fixed_name = strdup(name);
	    unsigned lp = strlen(name) - 1;
	    if (fixed_name[lp] == '>') {
		  while (fixed_name[lp] != '<') {
			assert(lp > 0);
			lp -= 1;
		  }
		  fixed_name[lp] = 0;
	    }
	    fprintf(vlog_out, " ");
	    emit_id(fixed_name);
	    free(fixed_name);
	    if (width > 1) {
		  fprintf(vlog_out, " [%u:0]", width-1);
	    }
      }
      fprintf(vlog_out, " (");
      count = ivl_logic_pins(nlogic);
      count -= 1;
      if (outputs == 0) outputs = count;
      for (idx = 0; idx < outputs; idx += 1) {
	    emit_name_of_logic_nexus(scope, nlogic, ivl_logic_pin(nlogic, idx));
	    fprintf(vlog_out, ", ");
      }
      for (/* None */; idx < count; idx += 1) {
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, idx));
	    fprintf(vlog_out, ", ");
      }
      if (strength_type == 2) {
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, idx));
      } else {
	      /* A pull gate only has a single output connection. */
	    assert(count == 0);
	    emit_name_of_logic_nexus(scope, nlogic, ivl_logic_pin(nlogic, idx));
      }
      fprintf(vlog_out, ");");
      emit_logic_file_line(nlogic);
      fprintf(vlog_out, "\n");
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
	    emit_nexus_as_ca(scope, ivl_switch_enable(tran));
      }
      fprintf(vlog_out, ");");
      if (emit_file_line) {
	    fprintf(vlog_out, " /* %s:%u */",
	                      ivl_switch_file(tran),
	                      ivl_switch_lineno(tran));
      }
      fprintf(vlog_out, "\n");
}

void emit_signal_net_const_as_ca(ivl_scope_t scope, ivl_signal_t sig)
{
      ivl_nexus_t nex = ivl_signal_nex(sig, 0);
      unsigned idx, count = ivl_nexus_ptrs(nex);
      unsigned long emitted = (unsigned long) ivl_nexus_get_private(nex);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_net_const_t net_const = ivl_nexus_ptr_con(nex_ptr);
	    if (! net_const) continue;
	    if (scope != ivl_const_scope(net_const)) continue;
	      /* Found the constant so emit it if it has not been emitted. */
	    if (emitted) {
		  --emitted;
		  continue;
	    }
	    fprintf(vlog_out, "%*cassign", indent, ' ');
	    emit_strength(ivl_nexus_ptr_drive1(nex_ptr),
	                  ivl_nexus_ptr_drive0(nex_ptr),
	                  2, "assign",
	                  ivl_signal_file(sig), ivl_signal_lineno(sig));
	    emit_delay(scope,
	               ivl_const_delay(net_const, 0),
	               ivl_const_delay(net_const, 1),
	               ivl_const_delay(net_const, 2),
	               3);
	    fprintf(vlog_out, " ");
	    emit_id(ivl_signal_basename(sig));
	    fprintf(vlog_out, " = ");
	    emit_const_nexus(scope, net_const);
	    fprintf(vlog_out, ";");
	    emit_sig_file_line(sig);
	    fprintf(vlog_out, "\n");
	      /* Increment the emitted constant count by one. */
	    ivl_nexus_set_private(nex,
	          (void *) ((unsigned long) ivl_nexus_get_private(nex) + 1U));
	    return;
      }
	/* We must find the constant in the nexus. */
      assert(0);

}
