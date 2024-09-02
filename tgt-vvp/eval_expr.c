/*
 * Copyright (c) 2001-2020 Stephen Williams (steve@icarus.com)
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

# include  "vvp_priv.h"
# include  <string.h>
# include  <stdlib.h>
# include  <assert.h>
# include  <stdbool.h>
# include  "ivl_alloc.h"


int number_is_unknown(ivl_expr_t expr)
{
      const char*bits;
      unsigned idx;

      if (ivl_expr_type(expr) == IVL_EX_ULONG)
	    return 0;

      assert(ivl_expr_type(expr) == IVL_EX_NUMBER);

      bits = ivl_expr_bits(expr);
      for (idx = 0 ;  idx < ivl_expr_width(expr) ;  idx += 1)
	    if ((bits[idx] != '0') && (bits[idx] != '1'))
		  return 1;

      return 0;
}

/*
 * This function returns TRUE if the number can be represented as a
 * lim_wid immediate value. This amounts to verifying that any upper
 * bits are the same. For a negative value we do not support the most
 * negative twos-complement value since it can not be negated. This
 * code generator always emits positive values, hence the negation
 * requirement.
 */
int number_is_immediate(ivl_expr_t expr, unsigned lim_wid, int negative_ok_flag)
{
      const char *bits;
      unsigned nbits = ivl_expr_width(expr);
      char pad_bit = '0';
      unsigned idx;

	/* We can only convert numbers to an immediate value. */
      if (ivl_expr_type(expr) != IVL_EX_NUMBER
	  && ivl_expr_type(expr) != IVL_EX_ULONG
	  && ivl_expr_type(expr) != IVL_EX_DELAY)
	    return 0;

	/* If a negative value is OK, then we really have one less
	 * significant bit because of the sign bit. */
      if (negative_ok_flag) lim_wid -= 1;

	/* This is an unsigned value so it can not have the -2**N problem. */
      if (ivl_expr_type(expr) == IVL_EX_ULONG) {
	    unsigned long imm;
	    if (lim_wid >= 8*sizeof(unsigned long)) return 1;
	      /* At this point we know that lim_wid is smaller than an
	       * unsigned long variable. */
	    imm = ivl_expr_uvalue(expr);
	    if (imm < (1UL << lim_wid)) return 1;
	    else return 0;
      }

	/* This is an unsigned value so it can not have the -2**N problem. */
      if (ivl_expr_type(expr) == IVL_EX_DELAY) {
	    uint64_t imm;
	    if (lim_wid >= 8*sizeof(uint64_t)) return 1;
	      /* At this point we know that lim_wid is smaller than a
	       * uint64_t variable. */
	    imm = ivl_expr_delay_val(expr);
	    if (imm < ((uint64_t)1 << lim_wid)) return 1;
	    else return 0;
      }

      bits = ivl_expr_bits(expr);

      if (ivl_expr_signed(expr) && bits[nbits-1]=='1') pad_bit = '1';

      if (pad_bit == '1' && !negative_ok_flag) return 0;

	/* Check if all the bits are either x or z. */
      if ((bits[0] ==  'x') || (bits[0] == 'z')) {
	    char first_bit = bits[0];
	    unsigned bits_match = 1;
	    for (idx = 1 ;  idx < nbits ;  idx += 1)
		  if (bits[idx] != first_bit) {
			bits_match = 0;
			break;
		  }
	    if (bits_match) return 1;
      }
      for (idx = lim_wid ;  idx < nbits ;  idx += 1)
	    if (bits[idx] != pad_bit) return 0;

	/* If we have a negative number make sure it is not too big. */
      if (pad_bit == '1') {
	    for (idx = 0; idx < lim_wid; idx += 1)
		  if (bits[idx] == '1') return 1;
	    return 0;
      }

      return 1;
}

/*
 * We can return positive or negative values. You must verify that the
 * number is not unknown (number_is_unknown) and is small enough
 * (number_is_immediate).
 */
long get_number_immediate(ivl_expr_t expr)
{
      long imm = 0;

      switch (ivl_expr_type(expr)) {
	  case IVL_EX_ULONG:
	    imm = ivl_expr_uvalue(expr);
	    break;

	  case IVL_EX_NUMBER: {
		const char*bits = ivl_expr_bits(expr);
		unsigned nbits = ivl_expr_width(expr);
		unsigned idx;
		  /* We can not copy more bits than fit into a long. */
		if (nbits > 8*sizeof(long)) nbits = 8*sizeof(long);
		for (idx = 0 ; idx < nbits ; idx += 1) switch (bits[idx]){
		    case '0':
		      break;
		    case '1':
		      imm |= 1L << idx;
		      break;
		    default:
		      assert(0);
		}
		if (ivl_expr_signed(expr) && bits[nbits-1]=='1' &&
		    nbits < 8*sizeof(long)) imm |= -1UL << nbits;
		break;
	  }

	  default:
	    assert(0);
      }

      return imm;
}

uint64_t get_number_immediate64(ivl_expr_t expr)
{
      uint64_t imm = 0;

      switch (ivl_expr_type(expr)) {
	  case IVL_EX_ULONG:
	    imm = ivl_expr_uvalue(expr);
	    break;

	  case IVL_EX_NUMBER: {
		const char*bits = ivl_expr_bits(expr);
		unsigned nbits = ivl_expr_width(expr);
		unsigned idx;
		for (idx = 0 ; idx < nbits ; idx += 1) switch (bits[idx]){
		    case '0':
		      break;
		    case '1':
		      assert(idx < 64);
		      imm |= UINT64_C(1) << idx;
		      break;
		    default:
		      assert(0);
		}
		if (ivl_expr_signed(expr) && bits[nbits-1]=='1' && nbits < 64)
		      imm |= (-UINT64_C(1)) << nbits;
		break;
	  }

	  default:
	    assert(0);
      }

      return imm;
}

void eval_logic_into_integer(ivl_expr_t expr, unsigned ix)
{
      switch (ivl_expr_type(expr)) {

	  case IVL_EX_NUMBER:
	  case IVL_EX_ULONG:
	      {
		    assert(number_is_immediate(expr, IMM_WID, 1));
		    if (number_is_unknown(expr)) {
			    /* We are loading a 'bx so mimic %ix/get. */
			  fprintf(vvp_out, "    %%ix/load %u, 0, 0;\n", ix);
			  fprintf(vvp_out, "    %%flag_set/imm 4, 1;\n");
			  break;
		    }
		    long imm = get_number_immediate(expr);
		    if (imm >= 0) {
			  fprintf(vvp_out, "    %%ix/load %u, %ld, 0;\n", ix, imm);
		    } else {
			  fprintf(vvp_out, "    %%ix/load %u, 0, 0; loading %ld\n", ix, imm);
			  fprintf(vvp_out, "    %%ix/sub %u, %ld, 0;\n", ix, -imm);
		    }
		      /* This can not have have a X/Z value so clear flag 4. */
		    fprintf(vvp_out, "    %%flag_set/imm 4, 0;\n");
	      }
	      break;

		/* Special case: There is an %ix instruction for
		   reading index values directly from variables. In
		   this case, try to use that special instruction. */
	  case IVL_EX_SIGNAL: {
		const char*type = ivl_expr_signed(expr) ? "/s" : "";
		ivl_signal_t sig = ivl_expr_signal(expr);

		unsigned word = 0;
		if (ivl_signal_dimensions(sig) > 0) {
		      ivl_expr_t ixe;

			/* Detect the special case that this is a
			   variable array. In this case, the ix/getv
			   will not work, so do it the hard way. */
		      if (ivl_signal_type(sig) == IVL_SIT_REG) {
			    draw_eval_vec4(expr);
			    fprintf(vvp_out, "    %%ix/vec4%s %u;\n", type, ix);
			    break;
		      }

		      ixe = ivl_expr_oper1(expr);
		      if (number_is_immediate(ixe, IMM_WID, 0)) {
		            assert(! number_is_unknown(ixe));
		            word = get_number_immediate(ixe);
		      } else {
		            draw_eval_vec4(expr);
		            fprintf(vvp_out, "    %%ix/vec4%s %u;\n", type, ix);
		            break;
		      }
		}
		fprintf(vvp_out, "    %%ix/getv%s %u, v%p_%u;\n", type, ix,
		                 sig, word);
		break;
	  }

	  default:
	    draw_eval_vec4(expr);
	      /* Is this a signed expression? */
	    if (ivl_expr_signed(expr)) {
		  fprintf(vvp_out, "    %%ix/vec4/s %u;\n", ix);
	    } else {
		  fprintf(vvp_out, "    %%ix/vec4 %u;\n", ix);
	    }
	    break;
      }
}

/*
 * This function, in addition to setting the value into index 0, sets
 * bit 4 to 1 if the value is unknown.
 */
void draw_eval_expr_into_integer(ivl_expr_t expr, unsigned ix)
{
      switch (ivl_expr_value(expr)) {

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    eval_logic_into_integer(expr, ix);
	    break;

	  case IVL_VT_REAL:
	    draw_eval_real(expr);
	    fprintf(vvp_out, "    %%cvt/sr %u;\n", ix);
	    break;

	  default:
	    fprintf(stderr, "XXXX ivl_expr_value == %d\n",
		    ivl_expr_value(expr));
	    assert(0);
      }
}

char *process_octal_codes(const char *in, unsigned width)
{
      unsigned idx = 0;
      unsigned ridx = 0;
      unsigned str_len = strlen(in);
      char *out = (char *)malloc(str_len+1);

        /* If we do not have any octal codes just return the input. */
      if (width/8 == str_len) {
	    strcpy(out, in);
	    return out;
      }

      while (ridx < str_len) {
	      /* An octal constant always has three digits. */
	    if (in[ridx] == '\\') {
		  out[idx] = (in[ridx+1]-'0')*64 + (in[ridx+2]-'0')*8 +
		            (in[ridx+3]-'0');
		  idx += 1;
		  ridx += 4;
	    } else {
		  out[idx] = in[ridx];
		  idx += 1;
		  ridx += 1;
	    }
      }
      out[idx] = '\0';

      return out;
}
