/*
 * Copyright (c) 2016 Stephen Williams (steve@icarus.com)
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
# include  <assert.h>
# include  <stdlib.h>
# include  <string.h>


/*
 * This function draws a BUFT to drive a constant delay value.
 */
static char* draw_const_net(void*ptr, char*suffix, uint64_t value)
{
      char tmp[64];
      char c4_value[69];
      unsigned idx;
      c4_value[0] = 'C';
      c4_value[1] = '4';
      c4_value[2] = '<';
      for (idx = 0; idx < 64; idx += 1) {
	    c4_value[66-idx] = (value & 1) ? '1' : '0';
	    value >>= 1;
      }
      c4_value[67] = '>';
      c4_value[68] = 0;

	/* Make the constant an argument to a BUFT, which is
	   what we use to drive the value. */
      fprintf(vvp_out, "L_%p/%s .functor BUFT 1, %s, C4<0>, C4<0>, C4<0>;\n",
	      ptr, suffix, c4_value);
      snprintf(tmp, sizeof tmp, "L_%p/%s", ptr, suffix);
      return strdup(tmp);
}

/*
 * Draw the appropriate delay statement.
 */
void draw_delay(void*ptr, unsigned wid, const char*input, ivl_expr_t rise_exp,
		ivl_expr_t fall_exp, ivl_expr_t decay_exp)
{
      char tmp[64];
      if (input == 0) {
	    snprintf(tmp, sizeof tmp, "L_%p/d", ptr);
	    input = tmp;
      }

	/* If the delays are all constants then process them here. */
      if (number_is_immediate(rise_exp, 64, 0) &&
	  number_is_immediate(fall_exp, 64, 0) &&
	  number_is_immediate(decay_exp, 64, 0)) {

	    assert(! number_is_unknown(rise_exp));
	    assert(! number_is_unknown(fall_exp));
	    assert(! number_is_unknown(decay_exp));

	    fprintf(vvp_out, "L_%p .delay %u "
			     "(%" PRIu64 ",%" PRIu64 ",%" PRIu64 ") %s;\n",
			     ptr, wid,
			     get_number_immediate64(rise_exp),
			     get_number_immediate64(fall_exp),
			     get_number_immediate64(decay_exp),
			     input);

	/* For a variable delay we indicate only two delays by setting the
	 * decay time to zero. */
      } else {
	    char*rise_const = 0;
	    char*fall_const = 0;
	    char*decay_const = 0;
	    const char*rise_str;
	    const char*fall_str;
	    const char*decay_str;

	    if (number_is_immediate(rise_exp, 64, 0)) {
		  uint64_t value = get_number_immediate64(rise_exp);
		  rise_str = rise_const = draw_const_net(ptr, "tr", value);
	    } else {
		  ivl_signal_t sig = ivl_expr_signal(rise_exp);
		  assert(sig && ivl_signal_dimensions(sig) == 0);
		  rise_str = draw_net_input(ivl_signal_nex(sig,0));
	    }

	    if (number_is_immediate(fall_exp, 64, 0)) {
		  uint64_t value = get_number_immediate64(fall_exp);
		  fall_str = fall_const = draw_const_net(ptr, "tf", value);
	    } else {
		  ivl_signal_t sig = ivl_expr_signal(fall_exp);
		  assert(sig && ivl_signal_dimensions(sig) == 0);
		  fall_str = draw_net_input(ivl_signal_nex(sig,0));
	    }

	    if (decay_exp == 0) {
		  decay_str = "0";
	    } else if (number_is_immediate(decay_exp, 64, 0)) {
		  uint64_t value = get_number_immediate64(decay_exp);
		  decay_str = decay_const = draw_const_net(ptr, "td", value);
	    } else {
		  ivl_signal_t sig = ivl_expr_signal(decay_exp);
		  assert(sig && ivl_signal_dimensions(sig) == 0);
		  decay_str = draw_net_input(ivl_signal_nex(sig,0));
	    }

	    fprintf(vvp_out, "L_%p .delay %u %s, %s, %s, %s;\n",
		    ptr, wid, input, rise_str, fall_str, decay_str);

	    free(rise_const);
	    free(fall_const);
	    free(decay_const);
      }
}
