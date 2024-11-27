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

/*
 * Data type used to signify if a $signed or $unsigned should be emitted.
 */
typedef enum lpm_sign_e {
      NO_SIGN = 0,
      NEED_SIGNED = 1,
      NEED_UNSIGNED = 2
} lpm_sign_t;

static ivl_signal_t nexus_is_signal(ivl_scope_t scope, ivl_nexus_t nex,
                                    int*msb, int*lsb, unsigned*array_word,
				    ivl_net_const_t*accept_const);

/*
 * Look to see if the nexus driver is signed.
 */
static int nexus_driver_is_signed(ivl_scope_t scope, ivl_nexus_t nex)
{
      int is_signed = 0;
      unsigned sign_set = 0;
      unsigned idx, count = ivl_nexus_ptrs(nex);

      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_lpm_t t_lpm = ivl_nexus_ptr_lpm(nex_ptr);
	    ivl_net_const_t t_net_const = ivl_nexus_ptr_con(nex_ptr);
	    ivl_net_logic_t t_nlogic = ivl_nexus_ptr_log(nex_ptr);
	    ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
	    ivl_drive_t cur_drive1 = ivl_nexus_ptr_drive1(nex_ptr);
	    ivl_drive_t cur_drive0 = ivl_nexus_ptr_drive0(nex_ptr);
	    if ((cur_drive1 == IVL_DR_HiZ) &&
	        (cur_drive0 == IVL_DR_HiZ)) continue;
	      /* Only one driver can set the sign information. */
	    assert( ! sign_set);
	    if (t_lpm) {
		  sign_set = 1;
		  is_signed = ivl_lpm_signed(t_lpm);
	    }
	    if (t_net_const) {
		  sign_set = 1;
		  is_signed = ivl_const_signed(t_net_const);
	    }
	    if (t_nlogic) {
		  sign_set = 1;
		    /* A BUFZ is used to drive a local signal so look for the
		     * local signal to get the sign information. */
		  if (ivl_logic_type(t_nlogic) == IVL_LO_BUFZ) {
			unsigned array_word = 0;
			int msb = 0, lsb = 0;
			ivl_signal_t sig;
			assert(ivl_logic_pins(t_nlogic) == 2);
			sig = nexus_is_signal(scope,
			                      ivl_logic_pin(t_nlogic, 0),
			                      &msb, &lsb, &array_word, 0);
			assert(sig);
			is_signed = ivl_signal_signed(sig);
		  }
		  /* The rest of the logic type are always unsigned. */
	    }
	    if (t_sig) {
		  sign_set = 1;
		  is_signed = ivl_signal_signed(t_sig);
	    }
      }

      return is_signed;
}

static unsigned get_nexus_width(ivl_nexus_t nex)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);

      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(nex_ptr);
	    if (sig) return ivl_signal_width(sig);
      }
      assert(0);
      return 0;
}

static lpm_sign_t lpm_get_sign_type(ivl_lpm_t lpm,
                                    unsigned can_skip_unsigned)
{
      lpm_sign_t rtn = NO_SIGN;
      int opr_sign = 0;
      int lpm_sign = ivl_lpm_signed(lpm);

      switch (ivl_lpm_type(lpm)) {
	case IVL_LPM_SIGN_EXT:
	    opr_sign = nexus_driver_is_signed(ivl_lpm_scope(lpm),
	                                      ivl_lpm_data(lpm, 0));
	    break;
	default:
	    opr_sign = lpm_sign;
	    break;
      }

	/* Check to see if a $signed() or $unsigned() is needed. */
      if (lpm_sign && ! opr_sign) rtn = NEED_SIGNED;
      if (! lpm_sign && opr_sign && ! can_skip_unsigned) rtn = NEED_UNSIGNED;

      return rtn;
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

static void emit_part_selector(int msb, int lsb)
{
      if (msb != lsb)
	    fprintf(vlog_out, "[%d:%d]", msb, lsb);
      else
	    fprintf(vlog_out, "[%d]", lsb);
}

/*
 * Look for a single driver behind an LPM that passes strength information
 * and get the real drive information from it.
 */
static void get_unique_lpm_drive(ivl_lpm_t lpm, ivl_drive_t *drive1,
                                 ivl_drive_t *drive0)
{
      ivl_nexus_t nex = ivl_lpm_data(lpm, 0);
      unsigned idx, count = ivl_nexus_ptrs(nex);
      unsigned have_driver = 0;

      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_drive_t cur_drive1 = ivl_nexus_ptr_drive1(nex_ptr);
	    ivl_drive_t cur_drive0 = ivl_nexus_ptr_drive0(nex_ptr);
	    if ((cur_drive1 == IVL_DR_HiZ) &&
	        (cur_drive0 == IVL_DR_HiZ)) continue;
	    assert(! have_driver);
	    *drive1 = cur_drive1;
	    *drive0 = cur_drive0;
	    have_driver = 1;
      }

	/* This should never happen. */
      if (! have_driver) {
	    fprintf(stderr, "%s:%u: vlog95 error: Unable to find drive "
	                    "information for strength transparent LPM.\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
	    vlog_errors += 1;
      }
}

static void emit_lpm_strength(ivl_lpm_t lpm)
{
      ivl_lpm_type_t type = ivl_lpm_type(lpm);
      ivl_drive_t drive1 = IVL_DR_STRONG;
      ivl_drive_t drive0 = IVL_DR_STRONG;
	/* This LPM object passes strength information so we need to look
	 * for the strength information at the real driver. */
      if (type == IVL_LPM_PART_PV) {
	    get_unique_lpm_drive(lpm, &drive1, &drive0);
      } else {
	    drive1 = ivl_lpm_drive1(lpm);
	    drive0 = ivl_lpm_drive0(lpm);
      }
      emit_strength(drive1, drive0, 2, "LPM",
                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
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

static void emit_driver_delay(ivl_scope_t scope, ivl_nexus_t nex)
{
      ivl_expr_t rise = 0;
      ivl_expr_t fall = 0;
      ivl_expr_t decay = 0;
      unsigned idx, count = ivl_nexus_ptrs(nex);

      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_lpm_t t_lpm = ivl_nexus_ptr_lpm(nex_ptr);
	    ivl_net_const_t t_net_const = ivl_nexus_ptr_con(nex_ptr);
	    ivl_net_logic_t t_nlogic = ivl_nexus_ptr_log(nex_ptr);
	    ivl_drive_t cur_drive1 = ivl_nexus_ptr_drive1(nex_ptr);
	    ivl_drive_t cur_drive0 = ivl_nexus_ptr_drive0(nex_ptr);
	    if ((cur_drive1 == IVL_DR_HiZ) &&
	        (cur_drive0 == IVL_DR_HiZ)) continue;
	      /* Only one driver can set the delay. */
	    assert( ! rise);
	    if (t_lpm) {
		  rise  = ivl_lpm_delay(t_lpm, 0);
		  fall  = ivl_lpm_delay(t_lpm, 1);
		  decay = ivl_lpm_delay(t_lpm, 2);
	    }
	    if (t_net_const) {
		  rise  = ivl_const_delay(t_net_const, 0);
		  fall  = ivl_const_delay(t_net_const, 1);
		  decay = ivl_const_delay(t_net_const, 2);
	    }
	    if (t_nlogic) {
		  rise  = ivl_logic_delay(t_nlogic, 0);
		  fall  = ivl_logic_delay(t_nlogic, 1);
		  decay = ivl_logic_delay(t_nlogic, 2);
	    }
      }

      emit_delay(scope, rise, fall, decay, 3);
}

static unsigned is_local_nexus(const ivl_scope_t scope, ivl_nexus_t nex)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);
      unsigned is_local = 1;
      unsigned has_output_driver = 0;
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(nex_ptr);
	    if (! sig) continue;
	      /* Check to see if there is an output or inout port
	       * driving into the local scope. */
	    if ((scope == ivl_scope_parent(ivl_signal_scope(sig))) &&
		((ivl_signal_port(sig) == IVL_SIP_OUTPUT) ||
		 (ivl_signal_port(sig) == IVL_SIP_INOUT))) {
		  has_output_driver = 1;
		  continue;
	    }
	    if (scope != ivl_signal_scope(sig)) continue;
	    if ((ivl_nexus_ptr_drive1(nex_ptr) != IVL_DR_HiZ) ||
		(ivl_nexus_ptr_drive0(nex_ptr) != IVL_DR_HiZ)) continue;
	    if (ivl_signal_local(sig)) {
		  is_local = 1;
	    } else {
		  is_local = 0;
		  break;
	    }
      }
	/* We return is_local=true only if there is not an output or inout
	 * driving into this scope. This is needed since some module outputs
	 * are combined with a concatenation and some inouts are connected
	 * with a tran_VP. */
      return is_local && !has_output_driver;
}

/*
 * This returns the nexus of the LPM if it is not a local signal.
 */
static ivl_nexus_t get_lpm_output(ivl_scope_t scope, ivl_lpm_t lpm)
{
      ivl_nexus_t output = 0;
      switch (ivl_lpm_type(lpm)) {
	case IVL_LPM_ABS:
	case IVL_LPM_ADD:
	case IVL_LPM_ARRAY:
	case IVL_LPM_CAST_INT:
	case IVL_LPM_CAST_INT2:
	case IVL_LPM_CAST_REAL:
	case IVL_LPM_CMP_EEQ:
	case IVL_LPM_CMP_EQ:
	case IVL_LPM_CMP_EQX:
	case IVL_LPM_CMP_EQZ:
	case IVL_LPM_CMP_GE:
	case IVL_LPM_CMP_GT:
	case IVL_LPM_CMP_NE:
	case IVL_LPM_CMP_NEE:
	case IVL_LPM_CMP_WEQ:
	case IVL_LPM_CMP_WNE:
	case IVL_LPM_CONCAT:
	case IVL_LPM_CONCATZ:
	case IVL_LPM_DIVIDE:
	case IVL_LPM_FF:
	case IVL_LPM_LATCH:
	case IVL_LPM_MOD:
	case IVL_LPM_MULT:
	case IVL_LPM_MUX:
	case IVL_LPM_PART_PV:
	case IVL_LPM_PART_VP:
	case IVL_LPM_POW:
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
	case IVL_LPM_SUBSTITUTE:
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
static void emit_lpm_as_ca(ivl_scope_t scope, ivl_lpm_t lpm,
                           unsigned sign_extend);

/* For an undriven port look for the local signal to get the nexus name. */
static void emit_nexus_port_signal(ivl_scope_t scope, ivl_nexus_t nex)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);
      ivl_signal_t sig = 0;
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
	    if ((ivl_nexus_ptr_drive1(nex_ptr) != IVL_DR_HiZ) ||
	        (ivl_nexus_ptr_drive0(nex_ptr) != IVL_DR_HiZ)) assert(0);
	    if (t_sig) {
		  if (scope != ivl_signal_scope(t_sig)) continue;
		  assert(! sig);
		  sig = t_sig;
	    }
      }
	/* There will not be a signal for an empty port. */
      if (sig) emit_nexus_as_ca(scope, ivl_signal_nex(sig, 0), 0, 0);
      else fprintf(vlog_out, "/* Empty */");
}

static ivl_signal_t find_local_signal(const ivl_scope_t scope, ivl_nexus_t nex,
                                      unsigned *word)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);
      ivl_signal_t sig = 0;
      *word = 0;
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
	    if (!t_sig) continue;
	    if (ivl_signal_local(t_sig) &&
	        (ivl_signal_port(t_sig) != IVL_SIP_INPUT)) continue;
	    if (ivl_signal_scope(t_sig) != scope) continue;
	    assert(! sig);
	    sig = t_sig;
	    *word = ivl_nexus_ptr_pin(nex_ptr);
      }
      return sig;
}

/*
 * Emit the input port driving expression.
 */
void emit_nexus_port_driver_as_ca(ivl_scope_t scope, ivl_nexus_t nex)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);
      ivl_lpm_t lpm = 0;
      ivl_net_const_t net_const = 0;
      ivl_net_logic_t nlogic = 0;
      ivl_signal_t sig = 0;
      unsigned word = 0;
	/* Look for the nexus driver. */
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_lpm_t t_lpm = ivl_nexus_ptr_lpm(nex_ptr);
	    ivl_net_const_t t_net_const = ivl_nexus_ptr_con(nex_ptr);
	    ivl_net_logic_t t_nlogic = ivl_nexus_ptr_log(nex_ptr);
	    ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
	    if ((ivl_nexus_ptr_drive1(nex_ptr) == IVL_DR_HiZ) &&
	        (ivl_nexus_ptr_drive0(nex_ptr) == IVL_DR_HiZ)) continue;
	    if (t_lpm) {
		  assert(! lpm);
		  lpm = t_lpm;
	    }
	    if (t_net_const) {
		  assert(! net_const);
		  net_const = t_net_const;
	    }
	    if (t_nlogic) {
		  assert(! nlogic);
		  nlogic = t_nlogic;
	    }
	    if (t_sig) {
		  assert(! sig);
		  sig = t_sig;
		  word = ivl_nexus_ptr_pin(nex_ptr);
	    }
      }
	/* An LPM is driving the nexus. */
      if (lpm) {
	    assert(! net_const);
	    assert(! nlogic);
	    assert(! sig);
	      /* If there is a signal in this scope that is also driven by
	       * the LPM then use the signal instead. */
	    sig = find_local_signal(scope, ivl_lpm_q(lpm), &word);
	    if (sig) emit_nexus_as_ca(scope, ivl_signal_nex(sig, word), 0, 0);
	    else emit_lpm_as_ca(scope, lpm, 0);
	/* A constant is driving the nexus. */
      } else if (net_const) {
	    assert( !nlogic);
	    assert(! sig);
	      /* If there is a signal in this scope that is also driven by
	       * the constant then use the signal instead. */
	    sig = find_local_signal(scope, ivl_const_nex(net_const), &word);
	    if (sig) emit_nexus_as_ca(scope, ivl_signal_nex(sig, word), 0, 0);
	    else emit_const_nexus(scope, net_const);
	/* A logic gate is driving the nexus. */
      } else if (nlogic) {
	    assert(! sig);
	      /* If there is a signal in this scope that is also driven by
	       * the logic then use the signal instead. */
	    sig = find_local_signal(scope, ivl_logic_pin(nlogic, 0), &word);
	    if (sig) emit_nexus_as_ca(scope, ivl_signal_nex(sig, word), 0, 0);
	    else emit_logic_as_ca(scope, nlogic);
	/* A signal is driving the nexus. */
      } else if (sig) {
	    emit_nexus_as_ca(scope, ivl_signal_nex(sig, word), 0, 0);
	/* If there is no driver then look for a single signal that is
	 * driven by this nexus that has the correct scope. This is needed
	 * to translate top level ports. */
      } else {
	    emit_nexus_port_signal(scope, nex);
      }
}

void emit_nexus_as_ca(ivl_scope_t scope, ivl_nexus_t nex, unsigned allow_UD,
                      unsigned sign_extend)
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
	    ivl_net_logic_t nlogic = 0;
	    ivl_signal_t sig = 0;
	    unsigned word = 0;
	    for (idx = 0; idx < count; idx += 1) {
		  ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
		  ivl_lpm_t t_lpm = ivl_nexus_ptr_lpm(nex_ptr);
		  ivl_net_const_t t_net_const = ivl_nexus_ptr_con(nex_ptr);
		  ivl_net_logic_t t_nlogic = ivl_nexus_ptr_log(nex_ptr);
		  ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
		  if ((ivl_nexus_ptr_drive1(nex_ptr) == IVL_DR_HiZ) &&
		      (ivl_nexus_ptr_drive0(nex_ptr) == IVL_DR_HiZ)) {
			  /* If we only have a single input then we want
			   * the nexus this signal is driven by. */
			if (count == 1) {
			      must_be_sig = 1;
			} else continue;
		  }
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
		  if (t_nlogic) {
			assert(! nlogic);
			nlogic = t_nlogic;
		  }
		  if (t_sig) {
			assert(! sig);
			sig = t_sig;
			word = ivl_nexus_ptr_pin(nex_ptr);
		  }
	    }
	    if (lpm) {
		  assert(! net_const);
		  assert(! nlogic);
		  assert(! sig);
		  assert(! must_be_sig);
// HERE: I think we need special input code like the following.
#if 0
	      /* If there is a signal in this scope that is also driven by
	       * the LPM then use the signal instead. */
	    sig = find_local_signal(scope, ivl_lpm_q(lpm), &word);
	    if (sig) emit_nexus_as_ca(scope, ivl_signal_nex(sig, word), 0, 0);
	    else emit_lpm_as_ca(scope, lpm, sign_extend);
#endif
		  emit_lpm_as_ca(scope, lpm, sign_extend);
	    } else if (net_const) {
		  assert( !nlogic);
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
	    } else if (nlogic) {
		  assert(! sig);
		  assert(! must_be_sig);
		  emit_logic_as_ca(scope, nlogic);
	    } else if (sig) {
// HERE: should these be allow_UD?
		  if (must_be_sig) {
			emit_nexus_as_ca(ivl_signal_scope(sig),
					 ivl_signal_nex(sig, word),
			                 0, 0);
		  } else emit_name_of_nexus(scope, nex, 0);
// HERE: The assert causes pr1703959 to fail.
//	    } else assert(0);
	    } else {
		  fprintf(stderr, "?:?: vlog95 error: Could not emit "
		                  "nexus as a CA.\n");
		  vlog_errors += 1;
		  fprintf(vlog_out, "<missing>");
	    }
      } else {
	    emit_name_of_nexus(scope, nex, allow_UD);
      }
}

static void emit_pull_const(char value, unsigned width)
{
      unsigned idx;
      fprintf(vlog_out, "%u'b", width);
      for (idx = 0; idx < width; idx += 1) {
	    fprintf(vlog_out, "%c", value);
      }
}

static void emit_logic_as_ca(ivl_scope_t scope, ivl_net_logic_t nlogic)
{
      unsigned inputs = ivl_logic_pins(nlogic) - 1;
      switch (ivl_logic_type(nlogic)) {
	case IVL_LO_AND:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    fprintf(vlog_out, " & ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_BUF:
	case IVL_LO_BUFT:
	case IVL_LO_BUFZ:
	    assert(inputs == 1);
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    break;
	case IVL_LO_EQUIV:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    fprintf(vlog_out, " ~^ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_IMPL:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(~");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_NAND:
	    assert(inputs == 2);
	    fprintf(vlog_out, "~(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    fprintf(vlog_out, " & ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_NOR:
	    assert(inputs == 2);
	    fprintf(vlog_out, "~(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_NOT:
	    assert(inputs == 1);
	    fprintf(vlog_out, "(~ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_OR:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_XNOR:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    fprintf(vlog_out, " ~^ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LO_XOR:
	    assert(inputs == 2);
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
	    fprintf(vlog_out, " ^ ");
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 2), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	  /* A pull up/down at this point has been turned into an assignment
	   * with strength so just emit the appropriate constant. */
	case IVL_LO_PULLDOWN:
	    emit_pull_const('0', ivl_logic_width(nlogic));
	    break;
	case IVL_LO_PULLUP:
	    emit_pull_const('1', ivl_logic_width(nlogic));
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
      emit_nexus_as_ca(scope, ivl_lpm_select(lpm), 0, 0);
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
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, "}");
	/* Icarus uses a concat to combine the output from multiple devices
	 * into a single vector, because of this we need to also look for
	 * the nexus driver outside the scope. emit_nexus_as_ca( , , 1, ) */
      } else {
	    for (idx = count-1; idx > 0; idx -= 1) {
		  emit_nexus_as_ca(scope, ivl_lpm_data(lpm, idx), 1, 0);
		  fprintf(vlog_out, ", ");
	    }
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 1, 0);
      }
      fprintf(vlog_out, "}");
}

/*
 * Look for an output signal in the nexus that is driving into this scope.
 */
static ivl_signal_t find_output_signal(const ivl_scope_t scope, ivl_nexus_t nex,
                                       unsigned*array_word)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);
      (void)array_word;  /* Parameter is not used. */
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
	    if (! t_sig) continue;
	      /* The signal must not be a driver. */
	    if ((ivl_nexus_ptr_drive1(nex_ptr) != IVL_DR_HiZ) ||
	        (ivl_nexus_ptr_drive0(nex_ptr) != IVL_DR_HiZ)) continue;
	      /* The signal must be an output. */
	    if (ivl_signal_port(t_sig) != IVL_SIP_OUTPUT) continue;
	      /* The signal must be driving into this scope. */
	    if (ivl_scope_parent(ivl_signal_scope(t_sig)) == scope) {
		  return t_sig;
	    }
      }
      return 0;
}

static ivl_signal_t nexus_is_signal(ivl_scope_t scope, ivl_nexus_t nex,
                                    int*msb, int*lsb, unsigned*array_word,
				    ivl_net_const_t*accept_const)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);
      ivl_lpm_t lpm = 0;
      ivl_net_const_t net_const = 0;
      ivl_net_logic_t nlogic = 0;
      ivl_signal_t sig;
	/* Look for a signal in the local scope first. */
      sig = find_local_signal(scope, nex, array_word);
      if (sig) {
	    get_sig_msb_lsb(sig, msb, lsb);
	    return sig;
      }
	/* Now look for an output signal driving into the local scope. */
      sig = find_output_signal(scope, nex, array_word);
      if (sig) {
	    get_sig_msb_lsb(sig, msb, lsb);
	    return sig;
      }
	/* Now scan the nexus looking for a driver. */
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_lpm_t t_lpm = ivl_nexus_ptr_lpm(nex_ptr);
	    ivl_net_const_t t_net_const = ivl_nexus_ptr_con(nex_ptr);
	    ivl_net_logic_t t_nlogic = ivl_nexus_ptr_log(nex_ptr);
	    ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
	    if ((ivl_nexus_ptr_drive1(nex_ptr) == IVL_DR_HiZ) &&
	        (ivl_nexus_ptr_drive0(nex_ptr) == IVL_DR_HiZ)) continue;
	    if (t_lpm) {
		  assert(! lpm);
		    /* The real signal could be hidden behind a select. */
		  if (ivl_lpm_type(t_lpm) == IVL_LPM_PART_VP) {
			t_sig = nexus_is_signal(scope, ivl_lpm_data(t_lpm, 0),
			                        msb, lsb, array_word, 0);
		  }

		  if (t_sig) {
			if (*msb >= *lsb) {
			      *lsb += ivl_lpm_base(t_lpm);
			      *msb = *lsb + (int)ivl_lpm_width(t_lpm) - 1;
			} else {
			      *lsb -= ivl_lpm_base(t_lpm);
			      *msb = *lsb - (int)ivl_lpm_width(t_lpm) + 1;
			}
		  } else lpm = t_lpm;
	    }
	    if (t_net_const) {
		  assert(! net_const);
		  net_const = t_net_const;
	    }
	    if (t_nlogic) {
		  assert(! nlogic);
		    /* The real signal could be hidden behind a BUFZ gate. */
		  if (ivl_logic_type(t_nlogic) == IVL_LO_BUFZ) {
			assert(ivl_logic_pins(t_nlogic) == 2);
			t_sig = nexus_is_signal(scope,
			                        ivl_logic_pin(t_nlogic, 1),
			                        msb, lsb, array_word, 0);
		  } else nlogic = t_nlogic;
	    }
	    if (t_sig) {
		  assert(! sig);
		  sig = t_sig;
		    /* This could be an array word so save the word. */
		  *array_word = ivl_nexus_ptr_pin(nex_ptr);
	    }
      }
      if (sig) return sig;

      if (accept_const && net_const) {
	    *lsb = 0;
	    *msb = ivl_const_width(net_const) - 1;
	    *accept_const = net_const;
      }
      return 0;
}

static void emit_part_name(ivl_scope_t scope, ivl_signal_t sig,
                           unsigned array_word)
{
      emit_scope_call_path(scope, ivl_signal_scope(sig));
      emit_id(ivl_signal_basename(sig));
      if (ivl_signal_dimensions(sig)) {
	    int array_idx = (int) array_word + ivl_signal_array_base(sig);
	    fprintf(vlog_out, "[%d]", array_idx);
      }

}

static void emit_lpm_part_select(ivl_scope_t scope, ivl_lpm_t lpm,
                                 unsigned sign_extend)
{
      unsigned width = ivl_lpm_width(lpm);
      unsigned array_word = 0;
      int base = ivl_lpm_base(lpm);
      int msb = 0, lsb = 0;
      ivl_signal_t sig = nexus_is_signal(scope, ivl_lpm_data(lpm, 0),
                                         &msb, &lsb, &array_word, 0);

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
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    if (base) {
		  fprintf(vlog_out, " ");
		  if (sign_extend) fprintf(vlog_out, ">");
		  fprintf(vlog_out, ">> %d)", base);
	    }
	    return;
      }

      if (sign_extend) {
	    fprintf(vlog_out, "(");
	    assert(base != lsb);
// HERE: This looks wrong.
	    if (msb >= lsb) base += lsb;
	    else base = lsb - base;
	    emit_part_name(scope, sig, array_word);
	    fprintf(vlog_out, " >>> %d)", base);
	    return;
      }

      if (width == 1) {
	    emit_part_name(scope, sig, array_word);
	    ivl_nexus_t sel = ivl_lpm_data(lpm, 1);
	    if (sel) {
		  fprintf(vlog_out, "[");
		  if ((msb >= lsb) && (lsb == 0)) {
			emit_nexus_as_ca(scope, sel, 0, 0);
// HERE: Need to scale the select nexus.
		  } else {
			fprintf(stderr, "%s:%u: vlog95 sorry: Non-zero based "
			                "variable part selects are not "
			                "supported.\n", ivl_lpm_file(lpm),
			                ivl_lpm_lineno(lpm));
			vlog_errors += 1;
			fprintf(vlog_out, "<missing>");
		  }
	    } else {
		    /* Skip a select of the entire bit. */
		  if ((msb == lsb) && (base == 0)) return;
		  fprintf(vlog_out, "[");
		  if (msb >= lsb) base += lsb;
		  else base = lsb - base;
		  fprintf(vlog_out, "%d", base);
	    }
	    fprintf(vlog_out, "]");
      } else {
	    ivl_nexus_t sel = ivl_lpm_data(lpm, 1);
	    if (sel) {
		  if ((msb >= lsb) && (lsb == 0)) {
			unsigned idx;
			fprintf(vlog_out, "{");
			for (idx = width-1; idx > 0; idx -= 1) {
			      emit_part_name(scope, sig, array_word);
			      fprintf(vlog_out, "[");
			      emit_nexus_as_ca(scope, sel, 0, 0);
			      fprintf(vlog_out, "+%u],", idx);
			}
			emit_part_name(scope, sig, array_word);
			fprintf(vlog_out, "[");
			emit_nexus_as_ca(scope, sel, 0, 0);
			fprintf(vlog_out, "]}");
// HERE: Need to scale the select nexus.
		  } else {
			fprintf(stderr, "%s:%u: vlog95 sorry: Non-zero based "
			                "variable part selects are not "
			                "supported.\n", ivl_lpm_file(lpm),
			                ivl_lpm_lineno(lpm));
			vlog_errors += 1;
			emit_part_name(scope, sig, array_word);
			fprintf(vlog_out, "[<missing>]");
		  }
	    } else {
		  emit_part_name(scope, sig, array_word);
		  fprintf(vlog_out, "[");
		  if (msb >= lsb) {
			base += lsb;
			fprintf(vlog_out, "%d:%d", base+(int)width-1, base);
		  } else {
			base = lsb - base;
			fprintf(vlog_out, "%d:%d", base-(int)width+1, base);
		  }
		  fprintf(vlog_out, "]");
	    }
      }
}

// HERE: No support for trigger. Is this actually needed?
static void emit_lpm_func(ivl_scope_t scope, ivl_lpm_t lpm)
{
      unsigned count = ivl_lpm_size(lpm);
      if (count) {
	    unsigned idx;
	    count -= 1;
	    fprintf(vlog_out, "(");
	    for (idx = 0; idx < count; idx += 1) {
		  emit_nexus_as_ca(scope, ivl_lpm_data(lpm, idx), 0, 0);
		  fprintf(vlog_out, ", ");
	    }
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, count), 0, 0);
	    fprintf(vlog_out, ")");
      }
}

static void emit_mux_select_bit(ivl_scope_t scope, ivl_lpm_t lpm, unsigned bit)
{
      unsigned array_word = 0;
      int msb = 0, lsb = 0;

      ivl_signal_t sig = nexus_is_signal(scope, ivl_lpm_select(lpm),
					 &msb, &lsb, &array_word, 0);
      assert(sig);
      emit_scope_call_path(scope, ivl_signal_scope(sig));
      emit_id(ivl_signal_basename(sig));
      if (ivl_signal_dimensions(sig)) {
	    int array_idx = (int) array_word + ivl_signal_array_base(sig);
	    fprintf(vlog_out, "[%d]", array_idx);
      }
      if (msb >= lsb) {
	    fprintf(vlog_out, "[%d]", lsb + (int)bit);
      } else {
	    fprintf(vlog_out, "[%d]", lsb - (int)bit);
      }
}

static void emit_lpm_mux(ivl_scope_t scope, ivl_lpm_t lpm,
			 unsigned sel_bit, unsigned offset)
{
      assert(sel_bit < sizeof(unsigned)*8);
      fprintf(vlog_out, "(");
      emit_mux_select_bit(scope, lpm, sel_bit);
      fprintf(vlog_out, " ? ");
      if (sel_bit > 0) {
	    emit_lpm_mux(scope, lpm, sel_bit - 1, offset + (1U << sel_bit));
	    fprintf(vlog_out, " : ");
	    emit_lpm_mux(scope, lpm, sel_bit - 1, offset);
      } else {
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, offset + 1), 0, 0);
	    fprintf(vlog_out, " : ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, offset + 0), 0, 0);
      }
      fprintf(vlog_out, ")");
}

static void emit_lpm_substitute(ivl_scope_t scope, ivl_lpm_t lpm)
{
      unsigned array_word = 0;
      int msb = 0, lsb = 0;
      unsigned base = ivl_lpm_base(lpm);
      unsigned width;
      int psb;
      int wid;

	/* Find the wider signal. Accept a constant if there's no signal. */
      ivl_net_const_t net_const = 0;
      ivl_signal_t sig = nexus_is_signal(scope, ivl_lpm_data(lpm, 0),
					 &msb, &lsb, &array_word,
					 &net_const);
      assert(sig || net_const);

	// Get the width of the part being substituted.
      width = get_nexus_width(ivl_lpm_data(lpm, 1));

      fprintf(vlog_out, "{");

      if (msb >= lsb) {
	   psb = lsb + base + width; wid = 1 + msb - psb;
      } else {
	   psb = lsb - base - width; wid = 1 + psb - msb;
      }
      if (wid > 0) {
	    if (sig) {
		  emit_scope_call_path(scope, ivl_signal_scope(sig));
		  emit_id(ivl_signal_basename(sig));
		  if (ivl_signal_dimensions(sig)) {
			int array_idx = (int) array_word + ivl_signal_array_base(sig);
			fprintf(vlog_out, "[%d]", array_idx);
		  }
		  emit_part_selector(msb, psb);
	    } else {
		  assert (msb >= psb);
		  emit_number(ivl_const_bits(net_const) + psb, wid, 0,
			      ivl_const_file(net_const),
			      ivl_const_lineno(net_const));
	    }

	    fprintf(vlog_out, ", ");
      }

      emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);

      if (msb >= lsb) {
	   psb = lsb + base - 1; wid = 1 + psb - lsb;
      } else {
	   psb = lsb - base + 1; wid = 1 + lsb - psb;
      }
      if (wid > 0) {
	    fprintf(vlog_out, ", ");

	    if (sig) {
		  emit_scope_call_path(scope, ivl_signal_scope(sig));
		  emit_id(ivl_signal_basename(sig));
		  if (ivl_signal_dimensions(sig)) {
			int array_idx = (int) array_word + ivl_signal_array_base(sig);
			fprintf(vlog_out, "[%d]", array_idx);
		  }
		  emit_part_selector(psb, lsb);
	    } else {
		  assert (psb >= lsb);
		  emit_number(ivl_const_bits(net_const), wid, 0,
			      ivl_const_file(net_const),
			      ivl_const_lineno(net_const));
	    }
      }

      fprintf(vlog_out, "}");
}

static void emit_lpm_as_ca(ivl_scope_t scope, ivl_lpm_t lpm,
                           unsigned sign_extend)
{
      lpm_sign_t sign_type;
	/* Check to see if a $signed() or $unsigned() needs to be emitted
	 * before the expression. */
      sign_type = lpm_get_sign_type(lpm, 0);
      if (sign_type == NEED_SIGNED) {
	    fprintf(vlog_out, "$signed(");
	    if (! allow_signed) {
		  fprintf(stderr, "%s:%u: vlog95 error: $signed() is not "
		                  "supported.\n",
		                  ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
		  vlog_errors += 1;
	    }
      }
      if (sign_type == NEED_UNSIGNED) {
	    fprintf(vlog_out, "$unsigned(");
	    if (! allow_signed) {
		  fprintf(stderr, "%s:%u: vlog95 error: $unsigned() is not "
		                  "supported.\n",
		                  ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
		  vlog_errors += 1;
	    }
      }

      switch (ivl_lpm_type(lpm)) {
	  /* Convert the Verilog-A abs() function. This only works when the
	   * argument has no side effect. */
	case IVL_LPM_ABS:
// HERE: If this is a real net then use the $abs() function to get NaN to
//       work correctly. See the expr code.
	    fprintf(vlog_out, "((");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, ") > ");
	    fprintf(vlog_out, "0 ? (");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, ") : -(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, "))");
	    break;
	case IVL_LPM_ADD:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " + ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_ARRAY:
	    emit_lpm_array(scope, lpm);
	    break;
	case IVL_LPM_CAST_INT:
	case IVL_LPM_CAST_INT2:
	case IVL_LPM_CAST_REAL:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 1, 0);
	    break;
	case IVL_LPM_CMP_EEQ:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " === ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_EQ:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " == ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_EQX:
// HERE: Need to check that this is not a real nexus.
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " ==? ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    fprintf(stderr, "%s:%u: vlog95 error: Compare wildcard equal (caseX) "
	                    "operator is not supported.\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
	    vlog_errors += 1;
	    break;
	case IVL_LPM_CMP_EQZ:
// HERE: Need to check that this is not a real nexus.
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " == ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    fprintf(stderr, "%s:%u: vlog95 error: Compare equal Z (caseZ) "
	                    "operator is not supported.\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
	    vlog_errors += 1;
	    break;
	case IVL_LPM_CMP_GE:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " >= ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_GT:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " > ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_NE:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " != ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_NEE:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " !== ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_CMP_WEQ:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " ==? ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    fprintf(stderr, "%s:%u: vlog95 error: Wild equality "
	                    "operator is not supported.\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
	    vlog_errors += 1;
	    break;
	case IVL_LPM_CMP_WNE:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " !=? ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    fprintf(stderr, "%s:%u: vlog95 error: Wild inequality "
	                    "operator is not supported.\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
	    vlog_errors += 1;
	    break;
	  /* A concat-Z should never be generated, but report it as an
	   * error if one is generated. */
	case IVL_LPM_CONCATZ:
	    fprintf(stderr, "%s:%u: vlog95 error: Transparent concatenations "
	                    "should not be generated.\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
	    vlog_errors += 1;
	    // fallthrough
	case IVL_LPM_CONCAT:
	    emit_lpm_concat(scope, lpm);
	    break;
	case IVL_LPM_DIVIDE:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " / ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
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
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " %% ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_MULT:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " * ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_MUX:
            if (ivl_lpm_selects(lpm) > 1) {
                  emit_lpm_mux(scope, lpm, ivl_lpm_selects(lpm) - 1, 0);
            } else {
                  fprintf(vlog_out, "(");
                  emit_nexus_as_ca(scope, ivl_lpm_select(lpm), 0, 0);
                  fprintf(vlog_out, " ? ");
                  emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
                  fprintf(vlog_out, " : ");
                  emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
                  fprintf(vlog_out, ")");
            }
	    break;
	case IVL_LPM_PART_PV:
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 1, 0);
	    break;
	case IVL_LPM_PART_VP:
	    emit_lpm_part_select(scope, lpm, sign_extend);
	    break;
	case IVL_LPM_POW:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " ** ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    fprintf(stderr, "%s:%u: vlog95 error: Power operator is not "
	                    "supported.\n",
	                    ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
	    vlog_errors += 1;
	    break;
	case IVL_LPM_RE_AND:
	    fprintf(vlog_out, "(&");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_NAND:
	    fprintf(vlog_out, "(~&");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_NOR:
	    fprintf(vlog_out, "(~|");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_OR:
	    fprintf(vlog_out, "(|");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_XOR:
	    fprintf(vlog_out, "(^");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_RE_XNOR:
	    fprintf(vlog_out, "(~^");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_REPEAT:
	    fprintf(vlog_out, "{%u{", ivl_lpm_size(lpm));
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, "}}");
	    break;
	case IVL_LPM_SFUNC:
	    fprintf(vlog_out, "%s", ivl_lpm_string(lpm));
	    emit_lpm_func(scope, lpm);
	    break;
	case IVL_LPM_SHIFTL:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " << ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_SHIFTR:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " ");
	    if (ivl_lpm_signed(lpm)) {
		  if (! allow_signed) {
			fprintf(stderr, "%s:%u: vlog95 error: >>> operator "
			                "is not supported.\n",
			                ivl_lpm_file(lpm),
			                ivl_lpm_lineno(lpm));
			vlog_errors += 1;
		  }
		  fprintf(vlog_out, ">");
	    }
	    fprintf(vlog_out, ">> ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_SIGN_EXT:
	    assert(! sign_extend);
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 1, 1);
	    break;
	case IVL_LPM_SUB:
	    fprintf(vlog_out, "(");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 0), 0, 0);
	    fprintf(vlog_out, " - ");
	    emit_nexus_as_ca(scope, ivl_lpm_data(lpm, 1), 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case IVL_LPM_SUBSTITUTE:
            emit_lpm_substitute(scope, lpm);
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

	/* Close the $signed() or $unsigned() if needed. */
      if (sign_type != NO_SIGN) fprintf(vlog_out, ")");
}

static void emit_posedge_dff_prim(void)
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

static void emit_negedge_dff_prim(void)
{
      fprintf(vlog_out, "\n");
      fprintf(vlog_out, "/* Icarus generated UDP to represent a synthesized "
                        "negative edge D-FF. */\n");
      fprintf(vlog_out, "primitive IVL_negedge_DFF "
                        "(q, clk, en, d, clr, set);\n");
      fprintf(vlog_out, "%*coutput q;\n", indent_incr, ' ');
      fprintf(vlog_out, "%*cinput clk, en, d, clr, set;\n", indent_incr, ' ');
      fprintf(vlog_out, "%*creg q;\n", indent_incr, ' ');
      fprintf(vlog_out, "%*ctable\n", indent_incr, ' ');
      fprintf(vlog_out, "%*cf 1 0 0 0 : ? : 0 ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cf 1 1 0 0 : ? : 1 ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cn 1 0 0 0 : 0 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cn 1 1 0 0 : 1 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cn x 0 0 0 : 0 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cn x 1 0 0 : 1 : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cp ? ? 0 0 : ? : - ;\n", 2*indent_incr, ' ');
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

static void emit_latch_prim(void)
{
      fprintf(vlog_out, "\n");
      fprintf(vlog_out, "/* Icarus generated UDP to represent a synthesized "
                        "LATCH. */\n");
      fprintf(vlog_out, "primitive IVL_LATCH "
                        "(q, en, d);\n");
      fprintf(vlog_out, "%*coutput q;\n", indent_incr, ' ');
      fprintf(vlog_out, "%*cinput en, d;\n", indent_incr, ' ');
      fprintf(vlog_out, "%*creg q;\n", indent_incr, ' ');
      fprintf(vlog_out, "%*ctable\n", indent_incr, ' ');
      fprintf(vlog_out, "%*c 1 0 : ? : 0 ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c 1 1 : ? : 1 ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*c 0 ? : ? : - ;\n", 2*indent_incr, ' ');
      fprintf(vlog_out, "%*cendtable\n", indent_incr, ' ');
      fprintf(vlog_out, "endprimitive\n");
}

static unsigned need_posedge_dff_prim = 0;
static unsigned need_negedge_dff_prim = 0;
static unsigned need_latch_prim = 0;

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
void emit_icarus_generated_udps(void)
{
	/* Emit the copyright information and LGPL note and then emit any
	 * needed primitives. */
      if (need_posedge_dff_prim || need_negedge_dff_prim || need_latch_prim) {
	    fprintf(vlog_out,
"\n"
"/*\n"
" * This is the copyright information for the following primitive(s)\n"
" * (library elements).\n"
" *\n"
" * Copyright (C) 2011-2016 Cary R. (cygcary@yahoo.com)\n"
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
      if (need_negedge_dff_prim) emit_negedge_dff_prim();
      if (need_latch_prim) emit_latch_prim();
}

static void emit_lpm_ff(ivl_scope_t scope, ivl_lpm_t lpm)
{
      unsigned negedge = ivl_lpm_negedge(lpm);
      ivl_expr_t aset_expr = ivl_lpm_aset_value(lpm);
      ivl_expr_t sset_expr = ivl_lpm_sset_value(lpm);
      ivl_nexus_t nex;
      unsigned emitted, have_data, have_sset;
      const char *aset_bits = 0;
      const char *sset_bits = 0;
	/* For now we only support a width of 1 for these bits. */
      if (aset_expr) {
	    if (ivl_expr_width(aset_expr) != 1) {
		  fprintf(stderr, "%s:%u: vlog95 sorry: FF LPMs with "
			  "multi-bit asynchronous set values are not "
			  "currently translated.\n",
			  ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
		  vlog_errors += 1;
	    }
	    aset_bits = ivl_expr_bits(aset_expr);
      }
      if (sset_expr) {
	    assert(ivl_expr_width(sset_expr) == 1);
	    sset_bits = ivl_expr_bits(sset_expr);
      }

      fprintf(vlog_out, "%*c", indent, ' ');
      if (negedge) {
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
      emit_name_of_nexus(scope, ivl_lpm_q(lpm), 0);
      fprintf(vlog_out, ", ");
	/* Emit the clock pin. */
      emit_nexus_as_ca(scope, ivl_lpm_clk(lpm), 0, 0);
      fprintf(vlog_out, ", ");
	/* Emit the enable pin expression(s) if needed. */
      emitted = 0;
      nex = ivl_lpm_enable(lpm);
      if (nex) {
	    emit_nexus_as_ca(scope, nex, 0, 0);
	    emitted = 1;
      }
      nex = ivl_lpm_sync_clr(lpm);
      if (nex) {
	    if (emitted) fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, nex, 0, 0);
	    emitted = 1;
      }
      have_sset = 0;
      nex = ivl_lpm_sync_set(lpm);
      if (nex) {
	    if (emitted) fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, nex, 0, 0);
	    emitted = 1;
	    have_sset = 1;
      }
      if (!emitted) fprintf(vlog_out, "1'b1");
      fprintf(vlog_out, ", ");
	/* Emit the data pin expression(s). */
      have_data = ivl_lpm_data(lpm, 0) != 0;
      nex = ivl_lpm_sync_clr(lpm);
      if (nex) {
	    emit_nexus_as_ca(scope, nex, 0, 0);
	    if (have_data | have_sset) fprintf(vlog_out, " & ");
	    if (have_data & have_sset) fprintf(vlog_out, "(");
      }
      nex = ivl_lpm_sync_set(lpm);
      if (nex) {
	    if (! sset_bits || (sset_bits[0] == '1')) {
		  emit_nexus_as_ca(scope, nex, 0, 0);
		  if (have_data) fprintf(vlog_out, " | ");
	    } else {
		  fprintf(vlog_out, "~");
		  emit_nexus_as_ca(scope, nex, 0, 0);
		  if (have_data) fprintf(vlog_out, " & ");
	    }
      }
      nex = ivl_lpm_data(lpm, 0);
      if (nex) emit_nexus_as_ca(scope, nex, 0, 0);
      if (have_data & have_sset) fprintf(vlog_out, ")");
      fprintf(vlog_out, ", ");
	/* Emit the clear pin expression(s) if needed. */
      emitted = 0;
      nex = ivl_lpm_async_clr(lpm);
      if (nex) {
	    emit_nexus_as_ca(scope, nex, 0, 0);
	    emitted = 1;
      }
      nex = ivl_lpm_async_set(lpm);
      if (!aset_bits || (aset_bits[0] != '0')) nex = 0;
      if (nex) {
	    if (emitted) fprintf(vlog_out, " | ");
	    emit_nexus_as_ca(scope, nex, 0, 0);
	    emitted = 1;
      }
      if (!emitted) fprintf(vlog_out, "1'b0");
      fprintf(vlog_out, ", ");
	/* Emit the set pin expression(s) if needed. */
      nex = ivl_lpm_async_set(lpm);
      if (aset_bits && (aset_bits[0] != '1')) nex = 0;
      if (nex) emit_nexus_as_ca(scope, nex, 0, 0);
      else fprintf(vlog_out, "1'b0");
      fprintf(vlog_out, ");\n");
	/* We need to emit a primitive for this instance. */
      if (negedge)
	    need_negedge_dff_prim = 1;
      else
	    need_posedge_dff_prim = 1;
}

static void emit_lpm_latch(ivl_scope_t scope, ivl_lpm_t lpm)
{
      ivl_nexus_t nex;
      unsigned emitted;

      fprintf(vlog_out, "%*c", indent, ' ');
      fprintf(vlog_out, "IVL_LATCH");
      emit_lpm_strength(lpm);
	/* The lpm LATCH does not support any delays. */
	/* The LATCH name is a temporary so we don't bother to print it unless
	 * we have a range. Then we need to use a made up name. */
      if (ivl_lpm_width(lpm) > 1) {
	    fprintf(vlog_out, " synth_%p [%u:0]", lpm, ivl_lpm_width(lpm)-1U);
      }
      fprintf(vlog_out, " (");
	/* Emit the q pin. */
      emit_name_of_nexus(scope, ivl_lpm_q(lpm), 0);
      fprintf(vlog_out, ", ");
	/* Emit the enable pin expression(s) if needed. */
      emitted = 0;
      nex = ivl_lpm_enable(lpm);
      if (nex) {
	    emit_nexus_as_ca(scope, nex, 0, 0);
	    emitted = 1;
      }
      if (!emitted) fprintf(vlog_out, "1'b1");
      fprintf(vlog_out, ", ");
	/* Emit the data pin expression(s). */
      nex = ivl_lpm_data(lpm, 0);
      assert (nex);
      emit_nexus_as_ca(scope, nex, 0, 0);
      fprintf(vlog_out, ");\n");
      need_latch_prim = 1;
}

static ivl_signal_t get_output_from_nexus(const ivl_scope_t scope, ivl_nexus_t nex,
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
      get_sig_msb_lsb(sig, &msb, &lsb);
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

static unsigned output_is_module_instantiation_input(const ivl_scope_t scope,
                                                     ivl_nexus_t nex)
{
      unsigned idx, count = ivl_nexus_ptrs(nex);
      unsigned rtn = 0;
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	      /* Skip drivers. */
	    if ((ivl_nexus_ptr_drive1(nex_ptr) != IVL_DR_HiZ) ||
	        (ivl_nexus_ptr_drive0(nex_ptr) != IVL_DR_HiZ)) continue;
	    ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
	      /* If the nexus is driving other things or signals that are
	       * not a module instantiation input then return false. */
// HERE: debug this to see if the output can drive other things local to the
//       module that is being called.
//	    if (! t_sig) return 0;
	    if (! t_sig) continue;
	    if (ivl_signal_port(t_sig) != IVL_SIP_INPUT) return 0;
	    if (ivl_scope_parent(ivl_signal_scope(t_sig)) != scope) return 0;
	    if (rtn) return 0;
	    rtn = 1;
      }
      return rtn;
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
      if (type == IVL_LPM_LATCH) {
	    emit_lpm_latch(scope, lpm);
	    return;
      }
// HERE: Look for a select passed to a pull device (pr2019553).
	/* Skip assignments to a module instantiation input. */
      if (output_is_module_instantiation_input(scope, output)) return;
      fprintf(vlog_out, "%*cassign", indent, ' ');
      emit_lpm_strength(lpm);
      if (type == IVL_LPM_PART_PV) {
	    emit_driver_delay(scope, ivl_lpm_data(lpm, 0));
	    fprintf(vlog_out, " ");
	    emit_lpm_part_pv(scope, lpm);
      } else {
	    emit_delay(scope,
                       ivl_lpm_delay(lpm, 0),
                       ivl_lpm_delay(lpm, 1),
                       ivl_lpm_delay(lpm, 2),
                       3);
	    fprintf(vlog_out, " ");
	    emit_name_of_nexus(scope, output, 0);
      }
      fprintf(vlog_out, " = ");
      emit_lpm_as_ca(scope, lpm, 0);
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
      emit_name_of_nexus(scope, ivl_logic_pin(nlogic, 0), 0);
      fprintf(vlog_out, " = ");
      emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, 1), 0, 0);
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
	    emit_name_of_nexus(scope, nex, 0);
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
      const char *name;
	/* Skip gates that have a local nexus as the output since they are
	 * part of a continuous assignment. */
      if (is_local_nexus(scope, ivl_logic_pin(nlogic, 0))) return;
      fprintf(vlog_out, "%*c", indent, ' ');
	/* Check to see if this logical should really be emitted as/was
	 * generated from a continuous assignment. */
      if (ivl_logic_is_cassign(nlogic)) {
	    unsigned pin_count = 2;
	    if (ivl_logic_type(nlogic) != IVL_LO_NOT) pin_count += 1;
	    assert(ivl_logic_pins(nlogic) == pin_count);
	    fprintf(vlog_out, "assign");
	    emit_gate_strength(nlogic, strength_type);
	    emit_delay(scope,
	               ivl_logic_delay(nlogic, 0),
	               ivl_logic_delay(nlogic, 1),
	               ivl_logic_delay(nlogic, 2),
	               3);
	    fprintf(vlog_out, " ");
	    emit_name_of_nexus(scope, ivl_logic_pin(nlogic, 0), 0);
	    fprintf(vlog_out, " = ");
	    emit_logic_as_ca(scope, nlogic);
	    fprintf(vlog_out, ";");
	    emit_logic_file_line(nlogic);
	    fprintf(vlog_out, "\n");
	    return;
      }
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
	case IVL_LO_BUFT:
	    return;
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
//       remove this and rebuild the instance array. For now we just make
//       this encoding a real name and create a zero based range. Need to
//       skip the local names _s<digits>.
//       This can also be an escaped id.
      name = ivl_logic_basename(nlogic);
      if (name && *name) {
	    char *fixed_name = strdup(name);
	    unsigned lp = strlen(name) - 1;
	    unsigned width = ivl_logic_width(nlogic);
	    if (fixed_name[lp] == '>') {
		  fixed_name[lp] = 0;
		  while (fixed_name[lp] != '.') {
			assert(lp > 0);
			lp -= 1;
		  }
		  fixed_name[lp] = '_';
		  while (fixed_name[lp] != '<') {
			assert(lp > 0);
			lp -= 1;
		  }
		  fixed_name[lp] = '_';
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
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, idx), 0, 0);
	    fprintf(vlog_out, ", ");
      }
      if (strength_type == 2) {
	    emit_nexus_as_ca(scope, ivl_logic_pin(nlogic, idx), 0, 0);
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
      const char *name;
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
      name = ivl_switch_basename(tran);
      if (name && *name) {
	    char *fixed_name = strdup(name);
	    unsigned lp = strlen(name) - 1;
	    unsigned width = ivl_switch_width(tran);
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
      emit_name_of_nexus(scope, ivl_switch_a(tran), 0);
      fprintf(vlog_out, ", ");
      emit_name_of_nexus(scope, ivl_switch_b(tran), 0);
      if (pins == 3) {
	    fprintf(vlog_out, ", ");
	    emit_nexus_as_ca(scope, ivl_switch_enable(tran), 0, 0);
      }
      fprintf(vlog_out, ");");
      if (emit_file_line) {
	    fprintf(vlog_out, " /* %s:%u */",
	                      ivl_switch_file(tran),
	                      ivl_switch_lineno(tran));
      }
      fprintf(vlog_out, "\n");
}

void emit_signal_net_const_as_ca(ivl_scope_t scope, ivl_signal_t sig,
                                 ivl_nexus_ptr_t nex_ptr)
{
      ivl_net_const_t net_const = ivl_nexus_ptr_con(nex_ptr);
      assert(net_const);
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
}

static void dump_drive(ivl_drive_t drive)
{
      switch (drive) {
	case IVL_DR_HiZ:    fprintf(stderr, "highz"); break;
	case IVL_DR_SMALL:  fprintf(stderr, "small"); break;
	case IVL_DR_MEDIUM: fprintf(stderr, "medium"); break;
	case IVL_DR_WEAK:   fprintf(stderr, "weak"); break;
	case IVL_DR_LARGE:  fprintf(stderr, "large"); break;
	case IVL_DR_PULL:   fprintf(stderr, "pull"); break;
	case IVL_DR_STRONG: fprintf(stderr, "strong"); break;
	case IVL_DR_SUPPLY: fprintf(stderr, "supply"); break;
      }
}

/*
 * Routine to dump the nexus information.
 */
void dump_nexus_information(ivl_scope_t scope, ivl_nexus_t nex)
{
      unsigned idx, count;
      if ((scope == 0) || (nex == 0)) return;
      count = ivl_nexus_ptrs(nex);
      fprintf(stderr, "Dumping nexus %p from scope: %s\n", nex,
              ivl_scope_name(scope));
      for (idx = 0; idx < count; idx += 1) {
            ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
            ivl_lpm_t lpm = ivl_nexus_ptr_lpm(nex_ptr);
            ivl_net_const_t net_const = ivl_nexus_ptr_con(nex_ptr);
            ivl_net_logic_t nlogic = ivl_nexus_ptr_log(nex_ptr);
            ivl_signal_t sig = ivl_nexus_ptr_sig(nex_ptr);
            fprintf(stderr, "  %u (", idx);
	    dump_drive(ivl_nexus_ptr_drive1(nex_ptr));
            fprintf(stderr, "1 ,");
	    dump_drive(ivl_nexus_ptr_drive0(nex_ptr));
            fprintf(stderr, "0) ");
	    if (lpm) {
		  ivl_scope_t lpm_scope = ivl_lpm_scope(lpm);
		  assert(! net_const);
		  assert(! nlogic);
		  assert(! sig);
		  fprintf(stderr, "LPM: ");
		  fprintf(stderr, "{%s:%u} ", ivl_lpm_file(lpm),
		          ivl_lpm_lineno(lpm));
		  if (scope != lpm_scope) fprintf(stderr, "(%s) ",
		                                  ivl_scope_name(lpm_scope));
		  switch (ivl_lpm_type(lpm)) {
		      case IVL_LPM_ABS:       fprintf(stderr, "abs"); break;
		      case IVL_LPM_ADD:       fprintf(stderr, "add"); break;
		      case IVL_LPM_ARRAY:     fprintf(stderr, "array"); break;
		      case IVL_LPM_CAST_INT:  fprintf(stderr, "<int>"); break;
		      case IVL_LPM_CAST_INT2: fprintf(stderr, "<int2>"); break;
		      case IVL_LPM_CAST_REAL: fprintf(stderr, "<real>"); break;
		      case IVL_LPM_CONCAT:    fprintf(stderr, "concat"); break;
		      case IVL_LPM_CONCATZ:   fprintf(stderr, "concatz"); break;
		      case IVL_LPM_CMP_EEQ:   fprintf(stderr, "eeq"); break;
		      case IVL_LPM_CMP_EQ:    fprintf(stderr, "eq"); break;
		      case IVL_LPM_CMP_GE:    fprintf(stderr, "ge"); break;
		      case IVL_LPM_CMP_GT:    fprintf(stderr, "gt"); break;
		      case IVL_LPM_CMP_NE:    fprintf(stderr, "ne"); break;
		      case IVL_LPM_CMP_NEE:   fprintf(stderr, "nee"); break;
		      case IVL_LPM_DIVIDE:    fprintf(stderr, "divide"); break;
		      case IVL_LPM_FF:        fprintf(stderr, "dff"); break;
		      case IVL_LPM_LATCH:     fprintf(stderr, "latch"); break;
		      case IVL_LPM_MOD:       fprintf(stderr, "mod"); break;
		      case IVL_LPM_MULT:      fprintf(stderr, "mult"); break;
		      case IVL_LPM_MUX:       fprintf(stderr, "mux"); break;
		      case IVL_LPM_PART_VP:   fprintf(stderr, "part-VP");
			fprintf(stderr, "(%u+%u)", ivl_lpm_base(lpm),
			        ivl_lpm_width(lpm)); break;
		      case IVL_LPM_PART_PV:   fprintf(stderr, "part-PV"); break;
		      case IVL_LPM_POW:       fprintf(stderr, "pow"); break;
		      case IVL_LPM_RE_AND:    fprintf(stderr, "R-AND"); break;
		      case IVL_LPM_RE_NAND:   fprintf(stderr, "R-NAND"); break;
		      case IVL_LPM_RE_OR:     fprintf(stderr, "R-OR"); break;
		      case IVL_LPM_RE_NOR:    fprintf(stderr, "R-NOR"); break;
		      case IVL_LPM_RE_XNOR:   fprintf(stderr, "R-XNOR"); break;
		      case IVL_LPM_RE_XOR:    fprintf(stderr, "R-XOR"); break;
		      case IVL_LPM_REPEAT:    fprintf(stderr, "repeat"); break;
		      case IVL_LPM_SFUNC:     fprintf(stderr, "S-func"); break;
		      case IVL_LPM_SHIFTL:    fprintf(stderr, "shiftl"); break;
		      case IVL_LPM_SHIFTR:    fprintf(stderr, "shiftr"); break;
		      case IVL_LPM_SIGN_EXT:  fprintf(stderr, "sign"); break;
		      case IVL_LPM_SUB:       fprintf(stderr, "sub"); break;
		      case IVL_LPM_UFUNC:     fprintf(stderr, "U-func"); break;
		      default: fprintf(stderr, "<%d>", ivl_lpm_type(lpm));
		  }
		  if (ivl_lpm_signed(lpm)) fprintf(stderr, " <signed>");
	    } else if (net_const) {
		  ivl_scope_t const_scope = ivl_const_scope(net_const);
		  assert(! nlogic);
		  assert(! sig);
		  fprintf(stderr, "Const:");
		  if (scope != const_scope) {
			fprintf(stderr, " (%s)", ivl_scope_name(const_scope));
		  }
		  if (ivl_const_signed(net_const)) fprintf(stderr, " <signed>");
	    } else if (nlogic) {
		  ivl_scope_t logic_scope = ivl_logic_scope(nlogic);
		  ivl_logic_t logic_type = ivl_logic_type(nlogic);
		  assert(! sig);
		  fprintf(stderr, "Logic: ");
		  fprintf(stderr, "{%s:%u} ", ivl_logic_file(nlogic),
		          ivl_logic_lineno(nlogic));
		  if (scope != logic_scope) {
			fprintf(stderr, "(%s) ", ivl_scope_name(logic_scope));
		  }
		  switch (logic_type) {
		      case IVL_LO_AND:      fprintf(stderr, "and"); break;
		      case IVL_LO_BUF:      fprintf(stderr, "buf"); break;
		      case IVL_LO_BUFIF0:   fprintf(stderr, "bufif0"); break;
		      case IVL_LO_BUFIF1:   fprintf(stderr, "bufif1"); break;
		      case IVL_LO_BUFT:     fprintf(stderr, "buft"); break;
		      case IVL_LO_BUFZ:     fprintf(stderr, "bufz"); break;
		      case IVL_LO_CMOS:     fprintf(stderr, "cmos"); break;
		      case IVL_LO_NAND:     fprintf(stderr, "nand"); break;
		      case IVL_LO_NMOS:     fprintf(stderr, "nmos"); break;
		      case IVL_LO_NOR:      fprintf(stderr, "nor"); break;
		      case IVL_LO_NOT:      fprintf(stderr, "not"); break;
		      case IVL_LO_NOTIF0:   fprintf(stderr, "notif0"); break;
		      case IVL_LO_NOTIF1:   fprintf(stderr, "notif1"); break;
		      case IVL_LO_OR:       fprintf(stderr, "or"); break;
		      case IVL_LO_PMOS:     fprintf(stderr, "pmos"); break;
		      case IVL_LO_PULLDOWN: fprintf(stderr, "pulldown"); break;
		      case IVL_LO_PULLUP:   fprintf(stderr, "pullup"); break;
		      case IVL_LO_RCMOS:    fprintf(stderr, "rcmos"); break;
		      case IVL_LO_RNMOS:    fprintf(stderr, "rnmos"); break;
		      case IVL_LO_RPMOS:    fprintf(stderr, "rpmos"); break;
		      case IVL_LO_UDP: {
			ivl_udp_t udp = ivl_logic_udp(nlogic);
			assert(udp);
			fprintf(stderr, "UDP %s", ivl_udp_name(udp));
			break;
		      }
		      case IVL_LO_XNOR:     fprintf(stderr, "xnor"); break;
		      case IVL_LO_XOR:      fprintf(stderr, "xor"); break;
		      default: fprintf(stderr, "<%d>", ivl_logic_type(nlogic));
		  }
		    /* The BUF and NOT gates can have multiple outputs and a
		     * single input. . */
		  if ((logic_type == IVL_LO_BUF) ||
		      (logic_type == IVL_LO_NOT)) {
			unsigned outputs = ivl_logic_pins(nlogic) - 1;
			if (outputs == 1) fprintf(stderr, "(1 output)");
			else fprintf(stderr, "(%u outputs)", outputs);
		    /* The rest of the gates have a single output and can
		     * have zero or more inputs. */
		  } else {
			unsigned inputs = ivl_logic_pins(nlogic) - 1;
			if (inputs == 1) fprintf(stderr, "(1 input)");
			else fprintf(stderr, "(%u inputs)", inputs);
		  }
	    } else if (sig) {
		  ivl_scope_t sig_scope = ivl_signal_scope(sig);
		  fprintf(stderr, "Signal: \"");
		  if (scope != sig_scope) fprintf(stderr, "%s.",
		                                  ivl_scope_name(sig_scope));
		  fprintf(stderr, "%s", ivl_signal_basename(sig));
		  if (ivl_signal_dimensions(sig) > 0) {
			fprintf(stderr, "[]");
		  }
		  fprintf(stderr, "\"");
// HERE: Do we need to add support for an array word or is that an LPM.
		  if (ivl_signal_local(sig)) fprintf(stderr, " {local}");
		  else fprintf(stderr, " {%s:%u}", ivl_signal_file(sig),
		               ivl_signal_lineno(sig));
		  switch (ivl_signal_port(sig)) {
		      case IVL_SIP_INPUT:  fprintf(stderr, " input"); break;
		      case IVL_SIP_OUTPUT: fprintf(stderr, " output"); break;
		      case IVL_SIP_INOUT:  fprintf(stderr, " inout"); break;
		      case IVL_SIP_NONE: break;
		  }
		  switch (ivl_signal_type(sig)) {
		      case IVL_SIT_NONE:   fprintf(stderr, " <no type>"); break;
		      case IVL_SIT_REG:    fprintf(stderr, " reg"); break;
		      case IVL_SIT_TRI:    fprintf(stderr, " tri"); break;
		      case IVL_SIT_TRI0:   fprintf(stderr, " tri0"); break;
		      case IVL_SIT_TRI1:   fprintf(stderr, " tri1"); break;
		      case IVL_SIT_TRIAND: fprintf(stderr, " triand"); break;
		      case IVL_SIT_TRIOR:  fprintf(stderr, " trior"); break;
		      case IVL_SIT_UWIRE:  fprintf(stderr, " uwire"); break;
		  }
		  switch (ivl_signal_data_type(sig)) {
		      case IVL_VT_VOID:    fprintf(stderr, " <void>"); break;
		      case IVL_VT_NO_TYPE: fprintf(stderr, " <no type>"); break;
		      case IVL_VT_REAL:    fprintf(stderr, " real"); break;
		      case IVL_VT_BOOL:    fprintf(stderr, " bool"); break;
		      case IVL_VT_LOGIC:   fprintf(stderr, " logic"); break;
		      case IVL_VT_STRING:  fprintf(stderr, " string"); break;
		      case IVL_VT_DARRAY:  fprintf(stderr, " dynamic array"); break;
		      case IVL_VT_CLASS:   fprintf(stderr, " class"); break;
		      case IVL_VT_QUEUE:   fprintf(stderr, " queue"); break;
		  }
		  if (ivl_signal_signed(sig)) fprintf(stderr, " <signed>");
	    } else fprintf(stderr, "Error: No/missing information!");
	    fprintf(stderr, " (%u)\n", ivl_nexus_ptr_pin(nex_ptr));
      }
}
