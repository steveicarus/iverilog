/*
 * Copyright (c) 2001-2024 Stephen Williams (steve@icarus.com)
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
# include  <stdlib.h>
# include  <math.h>
# include  <string.h>
# include  <inttypes.h>
# include  <assert.h>
# include  "ivl_alloc.h"

/*
 *  Escape non-symbol characters in ids, and quotes in strings.
 */

__inline__ static char hex_digit(unsigned i)
{
      i &= 0xf;
      return i>=10 ? i-10+'A' : i+'0';
}

const char *vvp_mangle_id(const char *id)
{
      static char *out = 0x0;
      static size_t out_len;

      int nesc = 0;
      int iout = 0;
      const char *inp = id;

      const char nosym[] = "!\"#%&'()*+,-/:;<=>?@[\\]^`{|}~";

      char *se = strpbrk(inp, nosym);
      if (!se)
	    return id;

      do {
	    int n = se - inp;
	    unsigned int nlen = strlen(id) + 4*(++nesc) + 1;
	    if (out_len < nlen) {
		  out = realloc(out, nlen);
		  out_len = nlen;
	    }
	    if (n) {
		  strncpy(out+iout, inp, n);
		  iout += n;
	    }
	    inp += n+1;
	    out[iout++] = '\\';
	    switch (*se) {
		case '\\':
		case '/':
		case '<':
		case '>':
		  out[iout++] = *se;
		  break;
		default:
		  out[iout++] = 'x';
		  out[iout++] = hex_digit(*se >> 4);
		  out[iout++] = hex_digit(*se);
		  break;
	    }

	    se = strpbrk(inp, nosym);
      } while (se);

      strcpy(out+iout, inp);
      return out;
}

const char *vvp_mangle_name(const char *id)
{
      static char *out = 0x0;
      static size_t out_len;

      int nesc = 0;
      int iout = 0;
      const char *inp = id;

      const char nosym[] = "\"\\";

      char *se = strpbrk(inp, nosym);
      if (!se)
	    return id;

      do {
	    int n = se - inp;
	    unsigned int nlen = strlen(id) + 2*(++nesc) + 1;
	    if (out_len < nlen) {
		  out = realloc(out, nlen);
		  out_len = nlen;
	    }
	    if (n) {
		  strncpy(out+iout, inp, n);
		  iout += n;
	    }
	    inp += n+1;
	    out[iout++] = '\\';
	    out[iout++] = *se;

	    se = strpbrk(inp, nosym);
      } while (se);

      strcpy(out+iout, inp);
      return out;
}

/* REMOVE ME: vvp_signal_label should not be used. DEAD CODE
 * Given a signal, generate a string name that is suitable for use as
 * a label. The only rule is that the same signal will always have the
 * same label. The result is stored in static memory, so remember to
 * copy it out.
 */
const char* vvp_signal_label(ivl_signal_t sig)
{
      static char buf[32];
      sprintf(buf, "%p", sig);
      return buf;
}

static ivl_signal_t signal_of_nexus(ivl_nexus_t nex, unsigned*word)
{
      unsigned idx;
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0)
		  continue;
	    if (ivl_signal_local(sig))
		  continue;
	    *word = ivl_nexus_ptr_pin(ptr);
	    return sig;
      }

      return 0;
}

unsigned width_of_nexus(ivl_nexus_t nex)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig != 0)
		  return ivl_signal_width(sig);
      }

      return 0;
}

ivl_variable_type_t data_type_of_nexus(ivl_nexus_t nex)
{
      unsigned idx;
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig != 0)
		  return ivl_signal_data_type(sig);
      }

	/* shouldn't happen! */
      return IVL_VT_NO_TYPE;
}


static ivl_nexus_ptr_t ivl_logic_pin_ptr(ivl_net_logic_t net, unsigned pin)
{
      ivl_nexus_t nex = ivl_logic_pin(net, pin);
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_net_logic_t tmp = ivl_nexus_ptr_log(ptr);
	    if (tmp == 0)
		  continue;
	    if (tmp != net)
		  continue;
	    if (ivl_nexus_ptr_pin(ptr) != pin)
		  continue;

	    return ptr;
      }
      assert(0);
      return 0;
}

#if 0
const char*drive_string(ivl_drive_t drive)
{
      switch (drive) {
	  case IVL_DR_HiZ:
	    return "";
	  case IVL_DR_SMALL:
	    return "sm";
	  case IVL_DR_MEDIUM:
	    return "me";
	  case IVL_DR_WEAK:
	    return "we";
	  case IVL_DR_LARGE:
	    return "la";
	  case IVL_DR_PULL:
	    return "pu";
	  case IVL_DR_STRONG:
	    return "";
	  case IVL_DR_SUPPLY:
	    return "su";
      }

      return "";
}
#endif


/*
 * The draw_scope function draws the major functional items within a
 * scope. This includes the scopes themselves, of course. All the
 * other functions in this file are in support of that task.
 */


/*
 * NEXUS
 * ivl builds up the netlist into objects connected together by
 * ivl_nexus_t objects. The nexus receives all the drivers of the
 * point in the net and resolves the value. The result is then sent to
 * all the nets that are connected to the nexus. The nets, then, are
 * read to get the value of the nexus.
 *
 * NETS
 * Nets are interesting and special, because a nexus may be connected
 * to several of them at once. This can happen, for example, as an
 * artifact of module port connects, where the inside and the outside
 * of the module are connected through an in-out port. (In fact, ivl
 * will simply connect signals that are bound through a port, because
 * the input/output/inout properties are enforced as compile time.)
 *
 * This case is handled by choosing one to receive the value of the
 * nexus. This one then feeds to another net at the nexus, and so
 * on. The last net is selected as the output of the nexus.
 */

/*
 * When checking if we can elide a buffer we need to keep the buffer
 * if both the input and output for the buffer are connected only
 * to netlist signals. This routine performs this check on the
 * given nexus.
 */
static unsigned is_netlist_signal(const ivl_net_logic_t net, ivl_nexus_t nex)
{
      unsigned idx, rtn;

	/* Assume that this is a netlist signal. */
      rtn = 1;

      for (idx = 0; idx < ivl_nexus_ptrs(nex); idx += 1) {
	    ivl_nexus_ptr_t nptr;
	    ivl_signal_t sptr;

	    nptr = ivl_nexus_ptr(nex, idx);

	      /* Skip a pointer to the buffer we're checking. */
            if (ivl_nexus_ptr_log(nptr) == net) continue;

	      /* Check to see if this is a netlist signal. */
	    sptr = ivl_nexus_ptr_sig(nptr);
	    if (sptr && !ivl_signal_local(sptr)) continue;

	      /* If we get here then this is not just a netlist signal. */
	    rtn = 0;
	    break;
      }

      return rtn;
}

int signal_is_return_value(ivl_signal_t sig)
{
      ivl_scope_t sig_scope = ivl_signal_scope(sig);
      if (ivl_scope_type(sig_scope) != IVL_SCT_FUNCTION)
	    return 0;
      if (strcmp(ivl_signal_basename(sig), ivl_scope_basename(sig_scope)) == 0)
	    return 1;
      return 0;
}

/*
 * This tests a bufz device against an output receiver, and determines
 * if the device can be skipped. If this function returns false, then a
 * gate will be generated for this node. Otherwise, the code generator
 * will connect its input to its output and skip the gate.
 */
int can_elide_bufz(ivl_net_logic_t net, ivl_nexus_ptr_t nptr)
{
	// If bufz is a module input/output buffer do not elide
      if (ivl_logic_port_buffer(net)) return 0;

      ivl_nexus_t in_n, out_n;
      unsigned idx;

	/* These are the drives we expect. */
      ivl_drive_t dr0 = ivl_nexus_ptr_drive0(nptr);
      ivl_drive_t dr1 = ivl_nexus_ptr_drive1(nptr);
      int drive_count = 0;

	/* If the gate carries a delay, it must remain. */
      if (ivl_logic_delay(net, 0) != 0)
	    return 0;

	/* If the input is connected to the output, then do not elide
	   the gate. This is some sort of cycle. */
      if (ivl_logic_pin(net, 0) == ivl_logic_pin(net, 1))
	    return 0;

      in_n = ivl_logic_pin(net, 1);
      for (idx = 0 ;  idx < ivl_nexus_ptrs(in_n) ;  idx += 1) {
	    ivl_nexus_ptr_t in_np = ivl_nexus_ptr(in_n, idx);
	    if (ivl_nexus_ptr_log(in_np) == net)
		  continue;

	      /* If the driver for the source does not match the
		 expected drive, then we need to keep the bufz. This
		 test also catches the case that the input device is
		 really also an input, as that device will have a
		 drive of HiZ. We need to keep BUFZ devices in that
		 case in order to prevent back-flow of data. */
	    if (ivl_nexus_ptr_drive0(in_np) != dr0)
		  return 0;
	    if (ivl_nexus_ptr_drive1(in_np) != dr1)
		  return 0;

	    drive_count += 1;
      }

	/* If the BUFZ input has multiple drivers on its input, then
	   we need to keep this device in order to hide the
	   resolution. */
      if (drive_count != 1)
	    return 0;

	/* If the BUFZ output is connected to a net that is subject
	   to a force statement, we need to keep the BUFZ to prevent
	   back-flow of the forced value. */
      out_n = ivl_logic_pin(net, 0);
      for (idx = 0 ;  idx < ivl_nexus_ptrs(out_n) ;  idx += 1) {
	    ivl_nexus_ptr_t out_np = ivl_nexus_ptr(out_n, idx);
	    ivl_signal_t out_sig = ivl_nexus_ptr_sig(out_np);
	    if (out_sig && ivl_signal_forced_net(out_sig))
		  return 0;
      }

	/* If both the input and output are netlist signal then we cannot
	   elide a BUFZ since it represents a continuous assignment. */
      if (is_netlist_signal(net, ivl_logic_pin(net, 0)) &&
          is_netlist_signal(net, ivl_logic_pin(net, 1)) &&
          (ivl_logic_type(net) == IVL_LO_BUFZ)) {
	     return 0;
      }

      return 1;
}

char* draw_Cr_to_string(double value)
{
      char tmp[256];

      uint64_t mant = 0;
      int sign, expo, vexp;
      double fract;

      if (isinf(value)) {
	    if (value > 0)
		  snprintf(tmp, sizeof(tmp), "Cr<m0g3fff>");
	    else
		  snprintf(tmp, sizeof(tmp), "Cr<m0g7fff>");
	    return strdup(tmp);
      }
      if (value != value) {
	    snprintf(tmp, sizeof(tmp), "Cr<m1g3fff>");
	    return strdup(tmp);
      }

      sign = 0;
      if (value < 0.0 || (value == 0.0 && 1.0/value < 0.0)) {
	    sign = 0x4000;
	    value *= -1;
      }

      fract = frexp(value, &expo);
      fract = ldexp(fract, 63);
      mant = fract;
      expo -= 63;

      vexp = expo + 0x1000;
      assert(vexp >= 0);
      assert(vexp < 0x2000);
      vexp += sign;

      snprintf(tmp, sizeof(tmp), "Cr<m%" PRIx64 "g%x>", mant, vexp);
      return strdup(tmp);
}

const char*draw_input_from_net(ivl_nexus_t nex)
{
      static char result[32];
      unsigned word;

      ivl_signal_t sig = signal_of_nexus(nex, &word);
      if (sig == 0)
	    return draw_net_input(nex);
      if (ivl_signal_type(sig)==IVL_SIT_REG && ivl_signal_dimensions(sig)>0)
	    return draw_net_input(nex);

      snprintf(result, sizeof result, "v%p_%u", sig, word);
      return result;
}


static const char *local_flag_str( ivl_signal_t sig )
{
    return ivl_signal_local(sig)? "*" : "";
}

/*
 * This function draws a reg/int/variable in the scope. This is a very
 * simple device to draw as there are no inputs to connect so no need
 * to scan the nexus. We do have to account for the possibility that
 * the device is arrayed, though, by making a node for each array element.
 */
static void draw_reg_in_scope(ivl_signal_t sig)
{
      int msb;
      int lsb;
      switch (ivl_signal_packed_dimensions(sig)) {
	  case 0:
	    msb = 0;
	    lsb = 0;
	    break;
	  case 1:
	    msb = ivl_signal_packed_msb(sig, 0);
	    lsb = ivl_signal_packed_lsb(sig, 0);
	    break;
	  default:
	    msb = ivl_signal_width(sig) - 1;
	    lsb = 0;
	    break;
      }

	/* Special Case: If this variable is the return value of a function,
	   then it need to exist as an actual variable. */
      if ((ivl_signal_data_type(sig)==IVL_VT_REAL)
	  && signal_is_return_value(sig)) {
	    fprintf(vvp_out, "; Variable %s is REAL return value of scope S_%p\n",
		    ivl_signal_basename(sig), ivl_signal_scope(sig));
	    return;
      }
      if ((ivl_signal_data_type(sig)==IVL_VT_STRING)
	  && signal_is_return_value(sig)) {
	    fprintf(vvp_out, "; Variable %s is string return value of scope S_%p\n",
		    ivl_signal_basename(sig), ivl_signal_scope(sig));
	    return;
      }
      if ((ivl_signal_data_type(sig)==IVL_VT_LOGIC)
	  && signal_is_return_value(sig)) {
	    fprintf(vvp_out, "; Variable %s is vec4 return value of scope S_%p\n",
		    ivl_signal_basename(sig), ivl_signal_scope(sig));
	    return;
      }
      if ((ivl_signal_data_type(sig)==IVL_VT_BOOL)
	  && signal_is_return_value(sig)) {
	    fprintf(vvp_out, "; Variable %s is bool return value of scope S_%p\n",
		    ivl_signal_basename(sig), ivl_signal_scope(sig));
	    return;
      }

      const char *datatype_flag = ivl_signal_integer(sig) ? "/i" :
			       ivl_signal_signed(sig)? "/s" : "";
      const char *local_flag = local_flag_str(sig);
      int vector_dims = 1;

      switch (ivl_signal_data_type(sig)) {
	  case IVL_VT_BOOL:
	    if (ivl_signal_signed(sig))
		  datatype_flag = "/2s";
	    else
		  datatype_flag = "/2u";
	    break;
	  case IVL_VT_REAL:
	    datatype_flag = "/real";
	    vector_dims = 0;
	    break;
	  case IVL_VT_STRING:
	    datatype_flag = "/str";
	    vector_dims = 0;
	    break;
	  case IVL_VT_CLASS:
	    datatype_flag = "/obj";
	    vector_dims = 0;
	    break;
	  default:
	    break;
      }

	/* If the reg objects are collected into an array, then first
	   write out the .array record to declare the array indices. */
      if (ivl_signal_dimensions(sig) > 0 && vector_dims==0) {

              /* Some types cannot be placed in packed dimensions, so
		 do not include packed dimensions. */
	    unsigned word_count = ivl_signal_array_count(sig);
	    unsigned swapped = ivl_signal_array_addr_swapped(sig);
	    int last = ivl_signal_array_base(sig)+word_count-1;
	    int first = ivl_signal_array_base(sig);
	    fprintf(vvp_out, "v%p .array%s \"%s\", %d %d;\n",
		    sig, datatype_flag,
		    vvp_mangle_name(ivl_signal_basename(sig)),
		    swapped ? first: last, swapped ? last : first);

      } else if (ivl_signal_dimensions(sig) > 0) {
	    unsigned word_count = ivl_signal_array_count(sig);
	    unsigned swapped = ivl_signal_array_addr_swapped(sig);
	    int last = ivl_signal_array_base(sig)+word_count-1;
	    int first = ivl_signal_array_base(sig);
	    fprintf(vvp_out, "v%p .array%s \"%s\", %d %d, %d %d;\n",
		    sig, datatype_flag,
		    vvp_mangle_name(ivl_signal_basename(sig)),
		    swapped ? first: last, swapped ? last : first, msb, lsb);

      } else if (ivl_signal_data_type(sig) == IVL_VT_DARRAY) {
	    ivl_type_t var_type = ivl_signal_net_type(sig);
	    ivl_type_t element_type = ivl_type_element(var_type);

	    fprintf(vvp_out, "v%p_0 .var/darray \"%s\", %u;%s\n", sig,
		    vvp_mangle_name(ivl_signal_basename(sig)),
		    ivl_type_packed_width(element_type),
		    ivl_signal_local(sig)? " Local signal" : "");

      } else if (ivl_signal_data_type(sig) == IVL_VT_QUEUE) {
	    ivl_type_t var_type = ivl_signal_net_type(sig);
	    ivl_type_t element_type = ivl_type_element(var_type);

	    fprintf(vvp_out, "v%p_0 .var/queue \"%s\", %u;%s\n", sig,
		    vvp_mangle_name(ivl_signal_basename(sig)),
		    ivl_type_packed_width(element_type),
		    ivl_signal_local(sig)? " Local signal" : "");

      } else if (ivl_signal_data_type(sig) == IVL_VT_STRING) {
	    fprintf(vvp_out, "v%p_0 .var/str \"%s\";%s\n", sig,
		    vvp_mangle_name(ivl_signal_basename(sig)),
		    ivl_signal_local(sig)? " Local signal" : "");

      } else if (ivl_signal_data_type(sig) == IVL_VT_CLASS) {
	    fprintf(vvp_out, "v%p_0 .var/cobj \"%s\";%s\n", sig,
		    vvp_mangle_name(ivl_signal_basename(sig)),
		    ivl_signal_local(sig)? " Local signal" : "");

      } else {

	    fprintf(vvp_out, "v%p_0 .var%s %s\"%s\", %d %d;%s\n",
		    sig, datatype_flag, local_flag,
		    vvp_mangle_name(ivl_signal_basename(sig)), msb, lsb,
		    ivl_signal_local(sig)? " Local signal" : "" );
      }
}


/*
 * This function draws a net. This is a bit more complicated as we
 * have to find an appropriate functor to connect to the input.
 */
static void draw_net_in_scope(ivl_signal_t sig)
{
      int msb;
      int lsb;
      switch (ivl_signal_packed_dimensions(sig)) {
	  case 0:
	    msb = 0;
	    lsb = 0;
	    break;
	  case 1:
	    msb = ivl_signal_packed_msb(sig, 0);
	    lsb = ivl_signal_packed_lsb(sig, 0);
	    break;
	  default:
	    msb = ivl_signal_width(sig) - 1;
	    lsb = 0;
	    break;
      }

      const char*datatype_flag = ivl_signal_signed(sig)? "/s" : "";
      const char *local_flag = local_flag_str(sig);

      unsigned iword;

      switch (ivl_signal_data_type(sig)) {
	  case IVL_VT_BOOL:
	    if (ivl_signal_signed(sig))
		  datatype_flag = "/2s";
	    else
		  datatype_flag = "/2u";
	    break;
	  case IVL_VT_REAL:
	    datatype_flag = "/real";
	    break;
	  default:
	    break;
      }

      for (iword = 0 ;  iword < ivl_signal_array_count(sig); iword += 1) {

	    unsigned word_count = ivl_signal_array_count(sig);
	    unsigned dimensions = ivl_signal_dimensions(sig);
	    struct vvp_nexus_data*nex_data;

	      /* Connect the pin of the signal to something. */
	    ivl_nexus_t nex = ivl_signal_nex(sig, iword);
	    const char*driver = draw_net_input(nex);

	    nex_data = (struct vvp_nexus_data*)ivl_nexus_get_private(nex);
	    assert(nex_data);

	    if (nex_data->net == 0) {
		  int strength_aware_flag = 0;
		  const char*vec8 = "";
		  if (nex_data->flags&VVP_NEXUS_DATA_STR)
			strength_aware_flag = 1;
		  if (nex_data->drivers_count > 1)
			vec8 = "8";
		  if (strength_aware_flag)
			vec8 = "8";

		  if (iword == 0 && dimensions > 0) {
			unsigned swapped = ivl_signal_array_addr_swapped(sig);
			int last = ivl_signal_array_base(sig) + word_count-1;
			int first = ivl_signal_array_base(sig);
			fprintf(vvp_out, "v%p .array \"%s\", %d %d;\n",
				sig, vvp_mangle_name(ivl_signal_basename(sig)),
				swapped ? first : last, swapped ? last : first);
		  }
		  if (dimensions > 0) {
			/* If this is a word of an array, then use an
			   array reference in place of the net name. */
			fprintf(vvp_out, "v%p_%u .net%s%s v%p %u, %d %d, %s;"
				" %u drivers%s\n",
				sig, iword, vec8, datatype_flag, sig,
				iword, msb, lsb, driver,
				nex_data->drivers_count,
				strength_aware_flag?", strength-aware":"" );

		  } else if (ivl_signal_local(sig) && ivl_scope_is_auto(ivl_signal_scope(sig))) {
			assert(word_count == 1);
			fprintf(vvp_out, "; Elide local/automatic net v%p_%u name=%s\n",
				sig, iword, ivl_signal_basename(sig));

		  } else if (ivl_signal_local(sig) && nex_data->drivers_count==0) {
			assert(word_count == 1);
			fprintf(vvp_out, "; Elide local net with no drivers, v%p_%u name=%s\n",
				sig, iword, ivl_signal_basename(sig));

		  } else {
			/* If this is an isolated word, it uses its
			   own name. */
			assert(word_count == 1);
			fprintf(vvp_out, "v%p_%u .net%s%s %s\"%s\", %d %d, %s; "
				" %u drivers%s\n",
				sig, iword, vec8, datatype_flag, local_flag,
				vvp_mangle_name(ivl_signal_basename(sig)),
				msb, lsb, driver,
				nex_data->drivers_count,
				strength_aware_flag?", strength-aware":"" );
		  }
		  nex_data->net = sig;
		  nex_data->net_word = iword;

	    } else if (dimensions > 0) {

		    /* In this case, we have an alias to an existing
		       signal array. this typically is an instance of
		       port collapsing that the elaborator combined to
		       discover that the entire array can be collapsed,
		       so the word count for the signal and the alias
		       *must* match. */
		  if (iword == 0 && ivl_signal_dimensions(nex_data->net) > 0 &&
                      word_count == ivl_signal_array_count(nex_data->net)) {

			fprintf(vvp_out, "v%p .array \"%s\", v%p; Alias to %s \n",
			        sig, vvp_mangle_name(ivl_signal_basename(sig)),
			        nex_data->net,
			        ivl_signal_basename(nex_data->net));
			break;

		    /* An alias for an individual word. */
		  } else {
			if (iword == 0) {
			      unsigned swapped = ivl_signal_array_addr_swapped(sig);
			      int first = ivl_signal_array_base(sig);
			      int last = first + word_count-1;
			      fprintf(vvp_out, "v%p .array \"%s\", %d %d;\n",
				      sig,
				      vvp_mangle_name(ivl_signal_basename(sig)),
				      swapped ? first : last,
				      swapped ? last : first );
			}

			fprintf(vvp_out, "v%p_%u .net%s v%p %u, %d %d, "
			        "v%p_%u; Alias to %s[%u]\n", sig, iword,
			        datatype_flag, sig, iword, msb, lsb,
			        nex_data->net, nex_data->net_word,
			        ivl_signal_basename(nex_data->net),
				nex_data->net_word);
		  }
	    } else {
		    /* Finally, we may have an alias that is a word
		       connected to another word. Again, this is a
		       case of port collapsing. */
		  int strength_aware_flag = 0;
		  const char*vec8 = "";
		  if (nex_data->flags&VVP_NEXUS_DATA_STR)
			strength_aware_flag = 1;
		  if (nex_data->drivers_count > 1)
			vec8 = "8";
		  if (strength_aware_flag)
			vec8 = "8";

		  fprintf(vvp_out, "v%p_%u .net%s%s %s\"%s\", %d %d, %s; "
				" alias, %u drivers%s\n",
				sig, iword, vec8, datatype_flag, local_flag,
				vvp_mangle_name(ivl_signal_basename(sig)),
				msb, lsb, driver,
				nex_data->drivers_count,
				strength_aware_flag?", strength-aware":"");
	    }
      }
}

/*
 * Check to see if we need a delay.
 */
static unsigned need_delay(ivl_net_logic_t lptr)
{
	/* If we have no rising delay then we do not have any delays. */
      if (ivl_logic_delay(lptr, 0) == 0) {
	    assert(ivl_logic_delay(lptr, 1) == 0);
	    assert(ivl_logic_delay(lptr, 2) == 0);
	    return 0;
      }

      return 1;
}

/*
 * Draw the appropriate delay statement. Returns zero if there is not a delay.
 */
static void draw_logic_delay(ivl_net_logic_t lptr)
{
      ivl_expr_t rise_exp = ivl_logic_delay(lptr, 0);
      ivl_expr_t fall_exp = ivl_logic_delay(lptr, 1);
      ivl_expr_t decay_exp = ivl_logic_delay(lptr, 2);

	/* Calculate the width of the delay. We also use a BUFZ for real
	 * values so we need to resize if the first input is real. */
      unsigned delay_wid = width_of_nexus(ivl_logic_pin(lptr, 0));
      if (data_type_of_nexus(ivl_logic_pin(lptr, 0)) == IVL_VT_REAL) {
	    delay_wid = 0;
      }

      draw_delay(lptr, delay_wid, 0, rise_exp, fall_exp, decay_exp);
}

static void draw_udp_def(ivl_udp_t udp)
{
  unsigned init;
  unsigned i;

  switch (ivl_udp_init(udp))
    {
    case '0':
      init = 0;
      break;
    case '1':
      init = 1;
      break;
    default:
      init = 2;
      break;
    }

  if (ivl_udp_sequ(udp))
	fprintf(vvp_out,
		"UDP_%s .udp/sequ \"%s\", %u, %u",
		vvp_mangle_id(ivl_udp_name(udp)),
		vvp_mangle_name(ivl_udp_name(udp)),
		ivl_udp_nin(udp),
		init );
  else
	fprintf(vvp_out,
		"UDP_%s .udp/comb \"%s\", %u",
		vvp_mangle_id(ivl_udp_name(udp)),
		vvp_mangle_name(ivl_udp_name(udp)),
		ivl_udp_nin(udp));

  for (i=0; i<ivl_udp_rows(udp); i++)
    fprintf(vvp_out, "\n ,\"%s\"", ivl_udp_row(udp, i) );

  fprintf(vvp_out, ";\n");
}

/* Check to see if the output of the UDP is connected with a modpath. */
static int udp_has_modpath_output(ivl_net_logic_t lptr)
{
	/* The first pin is the output connection (nexus). */
      ivl_nexus_t nex = ivl_logic_pin(lptr, 0);
      ivl_scope_t scope = ivl_logic_scope(lptr);
      unsigned idx;

	/* Check to see if there is a signal connected to the output. */
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0) continue;
	      /* The modpath will be connected to the UDP output. */
	    if ((ivl_signal_scope(sig) == scope) &&
	        (ivl_signal_port(sig) == IVL_SIP_OUTPUT) &&
	        (ivl_signal_npath(sig))) return 1;
      }

      return 0;
}

static void draw_udp_in_scope(ivl_net_logic_t lptr)
{
      unsigned pdx;

      ivl_udp_t udp = ivl_logic_udp(lptr);

      static ivl_udp_t *udps = 0x0;
      static int nudps = 0;
      int i;
      unsigned ninp;
      const char **input_strings;

	/* Do we need a delay? */
      unsigned need_delay_flag = need_delay(lptr);

      for (i=0; i<nudps; i++) {
	    if (udps[i] == udp) break;
      }

      if (i >= nudps) {
	    udps = realloc(udps, (nudps+1)*sizeof(ivl_udp_t));
	    udps[nudps++] = udp;
	    draw_udp_def(udp);
      }

	/*
	 * We need to process the arguments first so any evaluation code
	 * (.resolv, etc.) can be built before we build the .udp call.
	 * This matches what is done for the other primitives.
	 */
      assert(ivl_logic_pins(lptr) > 0);
      ninp = ivl_logic_pins(lptr) - 1;
      input_strings = calloc(ninp, sizeof(char*));
      for (pdx = 0 ;  pdx < ninp ;  pdx += 1) {
	    ivl_nexus_t nex = ivl_logic_pin(lptr, pdx+1);

	      /* Unlike other logic gates, primitives may have unconnected
	       * inputs. The proper behavior is to attach a HiZ to the
	       * port. */
	    if (nex == 0) {
		  assert(ivl_logic_width(lptr) == 1);
		  input_strings[pdx] = "C4<z>";
	    } else {
		  input_strings[pdx] = draw_net_input(nex);
	    }
      }

	/* Because vvp uses a wide functor for the output of a UDP we need
	 * to define the output delay net when needed, otherwise it will
	 * not be cleaned up correctly (gives a valgrind warning). */
      if (need_delay_flag) {
	    fprintf(vvp_out, "v%p_0 .net *\"_d%p\", 0 0, L_%p/d;\n", lptr,
	            lptr, lptr);
      }
	/* The same situation exists if a modpath is used to connect the UDP
	 * output to the true output signal. For this case the modpath is
	 * the only thing connected to the UDP output. */
      if (udp_has_modpath_output(lptr)) {
	    fprintf(vvp_out, "v%p_0 .net *\"_m%p\", 0 0, L_%p;\n", lptr,
	            lptr, lptr);
      }

	/* Generate the UDP call. */
      fprintf(vvp_out, "L_%p%s .udp UDP_%s", lptr, need_delay_flag ? "/d" : "",
                       vvp_mangle_id(ivl_udp_name(udp)));

      for (pdx = 0 ;  pdx < ninp ;  pdx += 1) {
	    fprintf(vvp_out, ", %s", input_strings[pdx]);
      }
      free(input_strings);

      fprintf(vvp_out, ";\n");

	/* Generate a delay when needed. */
      if (need_delay_flag) draw_logic_delay(lptr);
}

static void draw_equiv_impl_in_scope(ivl_net_logic_t lptr)
{
      unsigned ninp;
      const char *lval;
      const char *rval;
      const char*ltype = "?";

      assert(width_of_nexus(ivl_logic_pin(lptr, 0)) == 1);

      assert(ivl_logic_pins(lptr) > 0);
      ninp = ivl_logic_pins(lptr) - 1;
      assert(ninp == 2);

      lval = draw_net_input(ivl_logic_pin(lptr, 1));
      rval = draw_net_input(ivl_logic_pin(lptr, 2));

      if (ivl_logic_type(lptr) == IVL_LO_EQUIV) {
	    ltype = "EQUIV";
      } else {
	    assert(ivl_logic_type(lptr) == IVL_LO_IMPL);
	    ltype = "IMPL";
      }

      fprintf(vvp_out, "L_%p .functor %s 1, %s, %s, C4<0>, C4<0>;\n", lptr, ltype, lval, rval);
}

static void draw_logic_in_scope(ivl_net_logic_t lptr)
{
      unsigned pdx;
      const char*ltype = "?";
      const char*lcasc = 0;
      char identity_val = '0';

	/* Do we need a delay? */
      unsigned need_delay_flag = need_delay(lptr);

      unsigned vector_width = width_of_nexus(ivl_logic_pin(lptr, 0));

      ivl_drive_t str0 = ivl_logic_drive0(lptr);
      ivl_drive_t str1 = ivl_logic_drive1(lptr);

      int level;
      unsigned ninp;
      const char **input_strings;

      switch (ivl_logic_type(lptr)) {

          case IVL_LO_UDP:
	    draw_udp_in_scope(lptr);
	    return;

          case IVL_LO_BUFZ: {
		  /* Draw bufz objects, but only if the gate cannot
		     be elided. If I can elide it, then the
		     draw_nex_input will take care of it for me. */
		ivl_nexus_ptr_t nptr = ivl_logic_pin_ptr(lptr,0);

		ltype = "BUFZ";

		if (can_elide_bufz(lptr, nptr))
		      return;

		break;
	  }

	  case IVL_LO_BUFT: {
		  /* Draw bufz objects, but only if the gate cannot
		     be elided. If I can elide it, then the
		     draw_nex_input will take care of it for me. */
		ivl_nexus_ptr_t nptr = ivl_logic_pin_ptr(lptr,0);

		ltype = "BUFT";

		if (can_elide_bufz(lptr, nptr))
		      return;

		break;
	  }

	  case IVL_LO_PULLDOWN:
	  case IVL_LO_PULLUP:
	      /* Skip pullup and pulldown objects. Things that have
		 pull objects as inputs will instead generate the
		 appropriate C<?> symbol. */
	    return;

	  case IVL_LO_AND:
	    ltype = "AND";
	    identity_val = '1';
	    break;

	  case IVL_LO_BUF:
	    ltype = "BUF";
	    break;

	  case IVL_LO_BUFIF0:
	    ltype = "BUFIF0";
	    break;

	  case IVL_LO_BUFIF1:
	    ltype = "BUFIF1";
	    break;

	  case IVL_LO_EQUIV:
	  case IVL_LO_IMPL:
	    draw_equiv_impl_in_scope(lptr);
	    return;

	  case IVL_LO_NAND:
	    ltype = "NAND";
	    lcasc = "AND";
	    identity_val = '1';
	    break;

	  case IVL_LO_NOR:
	    ltype = "NOR";
	    lcasc = "OR";
	    break;

	  case IVL_LO_NOT:
	    ltype = "NOT";
	    break;

	  case IVL_LO_OR:
	    ltype = "OR";
	    break;

	  case IVL_LO_XNOR:
	    ltype = "XNOR";
	    lcasc = "XOR";
	    break;

	  case IVL_LO_XOR:
	    ltype = "XOR";
	    break;

	  case IVL_LO_CMOS:
	    ltype = "CMOS";
	    break;

	  case IVL_LO_PMOS:
	    ltype = "PMOS";
	    break;

	  case IVL_LO_NMOS:
	    ltype = "NMOS";
	    break;

	  case IVL_LO_RCMOS:
	    ltype = "RCMOS";
	    break;

	  case IVL_LO_RPMOS:
	    ltype = "RPMOS";
	    break;

	  case IVL_LO_RNMOS:
	    ltype = "RNMOS";
	    break;

	  case IVL_LO_NOTIF0:
	    ltype = "NOTIF0";
	    break;

	  case IVL_LO_NOTIF1:
	    ltype = "NOTIF1";
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt: error: Unhandled logic type: %d\n",
		    ivl_logic_type(lptr));
	    ltype = "?";
	    break;
      }

      if (!lcasc)
	lcasc = ltype;

	/* Get all the input label that I will use for parameters to
	   the functor that I create later. */
      assert(ivl_logic_pins(lptr) > 0);
      ninp = ivl_logic_pins(lptr) - 1;
      input_strings = calloc(ninp, sizeof(char*));
      for (pdx = 0 ;  pdx < (unsigned)ninp ;  pdx += 1)
	    input_strings[pdx] = draw_net_input(ivl_logic_pin(lptr, pdx+1));

      level = 0;
      while (ninp) {
	    unsigned inst;
	    for (inst = 0; inst < (unsigned)ninp; inst += 4) {
		  if (ninp > 4)
			fprintf(vvp_out, "L_%p/%d/%u .functor %s %u",
				lptr, level, inst, lcasc, vector_width);
		  else {
			fprintf(vvp_out, "L_%p%s .functor %s %u",
				lptr, need_delay_flag? "/d" : "",
				ltype, vector_width);

			if (str0 != IVL_DR_STRONG || str1 != IVL_DR_STRONG)
			      fprintf(vvp_out, " [%d %d]", str0, str1);

		  }
		  for (pdx = inst; pdx < (unsigned)ninp && pdx < inst+4 ; pdx += 1) {
			if (level) {
			      fprintf(vvp_out, ", L_%p/%d/%u",
				      lptr, level - 1, pdx*4);
			} else {
			      fprintf(vvp_out, ", %s", input_strings[pdx]);
			}
		  }
		  for ( ;  pdx < inst+4 ;  pdx += 1) {
			unsigned wdx;
			fprintf(vvp_out, ", C4<");
			for (wdx = 0 ; wdx < vector_width ;  wdx += 1)
			      fprintf(vvp_out, "%c", identity_val);
			fprintf(vvp_out, ">");
		  }

		  fprintf(vvp_out, ";\n");
	    }
	    if (ninp > 4)
		  ninp = (ninp+3) / 4;
	    else
		  ninp = 0;
	    level += 1;
      }

	/* Free the array of char*. The strings themselves are
	   persistent, held by the ivl_nexus_t objects. */
      free(input_strings);

	/* Generate a delay when needed. */
      if (need_delay_flag) draw_logic_delay(lptr);
}

static void draw_event_in_scope(ivl_event_t obj)
{
      char tmp[4][32];

      const unsigned ntmp = sizeof(tmp) / sizeof(tmp[0]);

      unsigned nany = ivl_event_nany(obj);
      unsigned nneg = ivl_event_nneg(obj);
      unsigned npos = ivl_event_npos(obj);
      unsigned nedg = ivl_event_nedg(obj);

      unsigned cnt = 0;

	/* Figure out how many probe functors are needed. */
      if (nany > 0)
	    cnt += (nany+ntmp-1) / ntmp;

      if (nneg > 0)
	    cnt += (nneg+ntmp-1) / ntmp;

      if (npos > 0)
	    cnt += (npos+ntmp-1) / ntmp;

      if (nedg > 0)
	    cnt += (nedg+ntmp-1) / ntmp;

      if (cnt == 0) {
	      /* If none are needed, then this is a named event. The
		 code needed is easy. */
	    fprintf(vvp_out, "E_%p .event \"%s\";\n", obj,
		    vvp_mangle_name(ivl_event_basename(obj)));

      } else if (cnt > 1) {
	      /* There are a bunch of events that need to be event/or
		 combined. */
	    unsigned idx;
	    unsigned ecnt = 0;

	    for (idx = 0 ;  idx < nany ;  idx += ntmp, ecnt += 1) {
		  unsigned sub, top;

		  top = idx + ntmp;
		  if (nany < top)
			top = nany;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, sub);
			strncpy(tmp[sub-idx], draw_input_from_net(nex), sizeof(tmp[0]));
		  }

		  fprintf(vvp_out, "E_%p/%u .event anyedge", obj, ecnt);
		  for (sub = idx ;  sub < top ;  sub += 1)
			fprintf(vvp_out, ", %s", tmp[sub-idx]);

		  fprintf(vvp_out, ";\n");
	    }

	    for (idx = 0 ;  idx < nneg ;  idx += ntmp, ecnt += 1) {
		  unsigned sub, top;

		  top = idx + ntmp;
		  if (nneg < top)
			top = nneg;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, sub);
			strncpy(tmp[sub-idx], draw_input_from_net(nex), sizeof(tmp[0]));
		  }

		  fprintf(vvp_out, "E_%p/%u .event negedge", obj, ecnt);
		  for (sub = idx ;  sub < top ;  sub += 1)
			fprintf(vvp_out, ", %s", tmp[sub-idx]);

		  fprintf(vvp_out, ";\n");
	    }

	    for (idx = 0 ;  idx < npos ;  idx += ntmp, ecnt += 1) {
		  unsigned sub, top;

		  top = idx + ntmp;
		  if (npos < top)
			top = npos;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, sub);
			strncpy(tmp[sub-idx], draw_input_from_net(nex), sizeof(tmp[0]));
		  }

		  fprintf(vvp_out, "E_%p/%u .event posedge", obj, ecnt);
		  for (sub = idx ;  sub < top ;  sub += 1)
			fprintf(vvp_out, ", %s", tmp[sub-idx]);

		  fprintf(vvp_out, ";\n");
	    }

	    for (idx = 0 ;  idx < nedg ;  idx += ntmp, ecnt += 1) {
		  unsigned sub, top;

		  top = idx + ntmp;
		  if (nedg < top)
			top = nedg;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_edg(obj, sub);
			strncpy(tmp[sub-idx], draw_input_from_net(nex), sizeof(tmp[0]));
		  }

		  fprintf(vvp_out, "E_%p/%u .event edge", obj, ecnt);
		  for (sub = idx ;  sub < top ;  sub += 1)
			fprintf(vvp_out, ", %s", tmp[sub-idx]);

		  fprintf(vvp_out, ";\n");
	    }

	    assert(ecnt == cnt);

	    fprintf(vvp_out, "E_%p .event/or", obj);
	    fprintf(vvp_out, " E_%p/0",  obj);

	    for (idx = 1 ;  idx < cnt ;  idx += 1)
		  fprintf(vvp_out, ", E_%p/%u", obj, idx);

	    fprintf(vvp_out, ";\n");

      } else {
	    unsigned num_input_strings = nany + nneg + npos + nedg;
	    unsigned idx;
	    const char*edge = 0;

	    assert(num_input_strings <= ntmp);

	    if (nany > 0) {
		  assert((nneg + npos + nedg) == 0);
		  edge = "anyedge";

		  for (idx = 0 ;  idx < nany ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, idx);
			strncpy(tmp[idx], draw_input_from_net(nex), sizeof(tmp[0]));
		  }

	    } else if (nneg > 0) {
		  assert((nany + npos + nedg) == 0);
		  edge = "negedge";

		  for (idx = 0 ;  idx < nneg ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, idx);
			strncpy(tmp[idx], draw_input_from_net(nex), sizeof(tmp[0]));
		  }

	    } else if (npos > 0) {
		  assert((nany + nneg + nedg) == 0);
		  edge = "posedge";

		  for (idx = 0 ;  idx < npos ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, idx);
			strncpy(tmp[idx], draw_input_from_net(nex), sizeof(tmp[0]));
		  }
	    } else {
		  assert((nany + nneg + npos) == 0);
		  edge = "edge";

		  for (idx = 0 ;  idx < nedg ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_edg(obj, idx);
			strncpy(tmp[idx], draw_input_from_net(nex), sizeof(tmp[0]));
		  }
	    }

	    fprintf(vvp_out, "E_%p .event %s", obj, edge);
	    for (idx = 0 ;  idx < num_input_strings ;  idx += 1) {
		  fprintf(vvp_out, ", %s", tmp[idx]);
	    }
	    fprintf(vvp_out, ";\n");
      }
}

/*
 * This function draws any functors needed to calculate the input to
 * this nexus, and leaves in the data array strings that can be used
 * as functor arguments. The strings are from the draw_net_input
 * function, which in turn returns nexus names, so the strings are
 * safe to pass around.
 */
static void draw_lpm_data_inputs(ivl_lpm_t net, unsigned base,
				 unsigned ndata, const char**src_table)
{
      unsigned idx;
      for (idx = 0 ;  idx < ndata ;  idx += 1) {
	    ivl_nexus_t nex = ivl_lpm_data(net, base+idx);
	    src_table[idx] = draw_net_input(nex);
      }
}

/*
 * If needed, draw a .delay node to delay the output from the LPM
 * device. Return the "/d" string if we drew this .delay node, or the
 * "" string if the node was not needed. The caller uses that string
 * to modify labels that are generated.
 */
static const char* draw_lpm_output_delay(ivl_lpm_t net, ivl_variable_type_t dt)
{
      ivl_expr_t d_rise = ivl_lpm_delay(net, 0);
      ivl_expr_t d_fall = ivl_lpm_delay(net, 1);
      ivl_expr_t d_decay = ivl_lpm_delay(net, 2);
      unsigned width = ivl_lpm_width(net);

	/* The comparison and reduction operators only have a single output
	 * bit to delay. */
      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_CMP_EEQ:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_EQX:
	  case IVL_LPM_CMP_EQZ:
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_CMP_NEE:
	  case IVL_LPM_RE_AND:
	  case IVL_LPM_RE_OR:
	  case IVL_LPM_RE_XOR:
	  case IVL_LPM_RE_NAND:
	  case IVL_LPM_RE_NOR:
	  case IVL_LPM_RE_XNOR:
            width = 1;
	  default:
            break;
      }

      if (dt == IVL_VT_REAL) width = 0;

      const char*dly = "";
      if (d_rise != 0) {
	    draw_delay(net, width, 0, d_rise, d_fall, d_decay);
	    dly = "/d";
      }

      return dly;
}

static void draw_lpm_abs(ivl_lpm_t net)
{
      const char*src_table[1];
      ivl_variable_type_t dt = data_type_of_nexus(ivl_lpm_data(net,0));
      const char*dly;

      draw_lpm_data_inputs(net, 0, 1, src_table);

      dly = draw_lpm_output_delay(net, dt);

      fprintf(vvp_out, "L_%p%s .abs %s;\n",
	      net, dly, src_table[0]);
}

static void draw_lpm_cast_int2(ivl_lpm_t net)
{
      const char*src_table[1];
      const char*dly;

      draw_lpm_data_inputs(net, 0, 1, src_table);

      dly = draw_lpm_output_delay(net, IVL_VT_BOOL);

      fprintf(vvp_out, "L_%p%s .cast/2 %u, %s;\n",
	      net, dly, ivl_lpm_width(net), src_table[0]);
}

static void draw_lpm_cast_int(ivl_lpm_t net)
{
      const char*src_table[1];
      const char*dly;

      draw_lpm_data_inputs(net, 0, 1, src_table);

      dly = draw_lpm_output_delay(net, IVL_VT_LOGIC);

      fprintf(vvp_out, "L_%p%s .cast/int %u, %s;\n",
	      net, dly, ivl_lpm_width(net), src_table[0]);
}

static void draw_lpm_cast_real(ivl_lpm_t net)
{
      const char*src_table[1];
      const char*dly;
      const char*is_signed = "";

      draw_lpm_data_inputs(net, 0, 1, src_table);

      dly = draw_lpm_output_delay(net, IVL_VT_REAL);

      if (ivl_lpm_signed(net)) is_signed = ".s";

      fprintf(vvp_out, "L_%p%s .cast/real%s %s;\n",
	      net, dly, is_signed, src_table[0]);
}

static void draw_lpm_add(ivl_lpm_t net)
{
      const char*src_table[2];
      unsigned width;
      const char*type = "";
      ivl_variable_type_t dta = data_type_of_nexus(ivl_lpm_data(net,0));
      ivl_variable_type_t dtb = data_type_of_nexus(ivl_lpm_data(net,1));
      ivl_variable_type_t dto = IVL_VT_LOGIC;
      const char*dly;

      if (dta == IVL_VT_REAL || dtb == IVL_VT_REAL)
	    dto = IVL_VT_REAL;

      width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_ADD:
	    if (dto == IVL_VT_REAL)
		  type = "sum.r";
	    else
		  type = "sum";
	    break;
	  case IVL_LPM_SUB:
	    if (dto == IVL_VT_REAL)
		  type = "sub.r";
	    else
		  type = "sub";
	    break;
	  case IVL_LPM_MULT:
	    if (dto == IVL_VT_REAL)
		  type = "mult.r";
	    else
		  type = "mult";
	    break;
	  case IVL_LPM_DIVIDE:
	    if (dto == IVL_VT_REAL)
		  type = "div.r";
	    else if (ivl_lpm_signed(net))
		  type = "div.s";
	    else
		  type = "div";
	    break;
	  case IVL_LPM_MOD:
	    if (dto == IVL_VT_REAL)
		  type = "mod.r";
	    else if (ivl_lpm_signed(net))
		  type = "mod.s";
	    else
		  type = "mod";
	    break;
	  case IVL_LPM_POW:
	    if (dto == IVL_VT_REAL)
		  type = "pow.r";
	    else if (ivl_lpm_signed(net))
		  type = "pow.s";
	    else
		  type = "pow";
	    break;
	  default:
	    assert(0);
      }

      draw_lpm_data_inputs(net, 0, 2, src_table);

      dly = draw_lpm_output_delay(net, dto);

      fprintf(vvp_out, "L_%p%s .arith/%s %u, %s, %s;\n",
	      net, dly, type, width, src_table[0], src_table[1]);
}

/*
* The read port to an array is generated as a single record that takes
* the address as an input.
*/
static void draw_lpm_array(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      ivl_signal_t mem = ivl_lpm_array(net);
      const char*tmp;

      nex = ivl_lpm_select(net);
      tmp = draw_net_input(nex);

      fprintf(vvp_out, "L_%p .array/port v%p, %s;\n", net, mem, tmp);
}

static void draw_lpm_cmp(ivl_lpm_t net)
{
      const char*src_table[2];
      unsigned width;
      const char*type = "";
      const char*signed_string = ivl_lpm_signed(net)? ".s" : "";
      ivl_variable_type_t dta = data_type_of_nexus(ivl_lpm_data(net,0));
      ivl_variable_type_t dtb = data_type_of_nexus(ivl_lpm_data(net,1));
      ivl_variable_type_t dtc = IVL_VT_LOGIC;
      const char*dly;

      if (dta == IVL_VT_REAL || dtb == IVL_VT_REAL)
	    dtc = IVL_VT_REAL;

      width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_CMP_EEQ:
	    assert(dtc != IVL_VT_REAL); /* Should never get here! */
	    type = "eeq";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_EQ:
	    if (dtc == IVL_VT_REAL)
		  type = "eq.r";
	    else
		  type = "eq";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_EQX:
	    assert(dtc != IVL_VT_REAL);
	    type = "eqx";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_EQZ:
	    assert(dtc != IVL_VT_REAL);
	    type = "eqz";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_GE:
	    if (dtc == IVL_VT_REAL) {
		  type = "ge.r";
		  signed_string = "";
	    } else
		  type = "ge";
	    break;
	  case IVL_LPM_CMP_GT:
	    if (dtc == IVL_VT_REAL) {
		  type = "gt.r";
		  signed_string = "";
	    } else
		  type = "gt";
	    break;
	  case IVL_LPM_CMP_NE:
	    if (dtc == IVL_VT_REAL)
		  type = "ne.r";
	    else
		  type = "ne";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_NEE:
	    assert(dtc != IVL_VT_REAL); /* Should never get here! */
	    type = "nee";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_WEQ:
	    assert(dtc != IVL_VT_REAL); /* Should never get here! */
	    type = "weq";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_WNE:
	    assert(dtc != IVL_VT_REAL); /* Should never get here! */
	    type = "wne";
	    signed_string = "";
	    break;
	  default:
	    assert(0);
      }

      draw_lpm_data_inputs(net, 0, 2, src_table);

	/* The output of a compare is always logical. */
      dly = draw_lpm_output_delay(net, IVL_VT_LOGIC);

      fprintf(vvp_out, "L_%p%s .cmp/%s%s %u, %s, %s;\n",
	      net, dly, type, signed_string, width,
	      src_table[0], src_table[1]);
}

/*
 * This function draws the arguments to a .const node using the
 * lpm inputs starting at "start" and for "cnt" inputs. This input
 * count must be <= 4. It is up to the caller to write the header part
 * of the statement, and to organize the data into multiple
 * statements.
 *
 * Return the width of the final concatenation.
 */
static unsigned lpm_concat_inputs(ivl_lpm_t net, unsigned start,
				  unsigned cnt, const char*src_table[])
{
      unsigned idx;
      unsigned wid = 0;

      assert(cnt <= 4);

	/* First, draw the [L M N O] part of the statement, the list
	   of widths for the .concat statement. */
      fprintf(vvp_out, "[");

      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    ivl_nexus_t nex = ivl_lpm_data(net, start+idx);
	    unsigned nexus_width = width_of_nexus(nex);
	    fprintf(vvp_out, " %u", nexus_width);
	    wid += nexus_width;
      }

      for ( ; idx < 4 ;  idx += 1)
	    fprintf(vvp_out, " 0");

      fprintf(vvp_out, "]");


      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    fprintf(vvp_out, ", %s", src_table[idx]);
      }

      fprintf(vvp_out, ";\n");
      return wid;
}

/*
 * Implement the general IVL_LPM_CONCAT using .concat nodes. Use as
 * many nested nodes as necessary to support the desired number of
 * input vectors.
 */
static void draw_lpm_concat(ivl_lpm_t net)
{
      const char*src_table[4];
      unsigned icnt = ivl_lpm_size(net);
      const char*dly = draw_lpm_output_delay(net, IVL_VT_LOGIC);
      const char*z = ivl_lpm_type(net)==IVL_LPM_CONCATZ? "8" : "";

      if (icnt <= 4) {
	      /* This is the easiest case. There are 4 or fewer input
		 vectors, so the entire IVL_LPM_CONCAT can be
		 implemented with a single .concat node. */
	    draw_lpm_data_inputs(net, 0, icnt, src_table);
	    fprintf(vvp_out, "L_%p%s .concat%s ", net, dly, z);
	    lpm_concat_inputs(net, 0, icnt, src_table);

      } else {
	      /* If there are more than 4 inputs, things get more
		 complicated. We need to generate a balanced tree of
		 .concat nodes to blend the inputs down to a single
		 root node, that becomes the output from the
		 concatenation. */
	    unsigned idx, depth;
	    struct concat_tree {
		  unsigned base;
		  unsigned wid;
	    } *tree;

	    tree = malloc((icnt + 3)/4 * sizeof(struct concat_tree));

	      /* First, fill in all the leaves with the initial inputs
		 to the tree. After this loop, there are (icnt+3)/4
		 .concat nodes drawn, that together take all the
		 inputs. */
	    for (idx = 0 ;  idx < icnt ;  idx += 4) {
		  unsigned wid = 0;
		  unsigned trans = 4;
		  if ((idx + trans) > icnt)
			trans = icnt - idx;

		  draw_lpm_data_inputs(net, idx, trans, src_table);
		  fprintf(vvp_out, "LS_%p_0_%u .concat%s ", net, idx, z);
		  wid = lpm_concat_inputs(net, idx, trans, src_table);

		  tree[idx/4].base = idx;
		  tree[idx/4].wid  = wid;
	    }

	      /* icnt is the input count for the level. It is the
		 number of .concats of the previous level that have to
		 be concatenated at the current level. (This is not
		 the same as the bit width.) */
	    icnt = (icnt + 3)/4;

	      /* Tree now has icnt nodes that are depth=0 concat nodes
		 which take in the leaf inputs. The while loop below
		 starts and ends with a tree of icnt nodes. Each time
		 through, there are 1/4 the nodes we started
		 with. Thus, we eventually get down to <=4 nodes, and
		 that is when we fall out of the loop. */

	    depth = 1;
	    while (icnt > 4) {
		  for (idx = 0 ;  idx < icnt ;  idx += 4) {
			unsigned tdx;
			unsigned wid = 0;
			unsigned trans = 4;
			if ((idx+trans) > icnt)
			      trans = icnt - idx;

			fprintf(vvp_out, "LS_%p_%u_%u .concat%s [",
				net, depth, idx, z);

			for (tdx = 0 ;  tdx < trans ;  tdx += 1) {
			      fprintf(vvp_out, " %u", tree[idx+tdx].wid);
			      wid += tree[idx+tdx].wid;
			}

			for ( ;  tdx < 4 ;  tdx += 1)
			      fprintf(vvp_out, " 0");

			fprintf(vvp_out, "]");

			for (tdx = 0; tdx < trans ;  tdx += 1) {
			      fprintf(vvp_out, ", LS_%p_%u_%u", net,
				      depth-1, tree[idx+tdx].base);
			}

			fprintf(vvp_out, ";\n");
			tree[idx/4].base = idx;
			tree[idx/4].wid = wid;
		  }

		  depth += 1;
		  icnt = (icnt + 3)/4;
	    }

	      /* Finally, draw the root node that takes in the final
		 row of tree nodes and generates a single output. */
	    fprintf(vvp_out, "L_%p%s .concat%s [", net, dly, z);
	    for (idx = 0 ;  idx < icnt ;  idx += 1)
		  fprintf(vvp_out, " %u", tree[idx].wid);
	    for ( ;  idx < 4 ;  idx += 1)
		  fprintf(vvp_out, " 0");
	    fprintf(vvp_out, "]");

	    for (idx = 0 ;  idx < icnt ;  idx += 1)
		  fprintf(vvp_out, ", LS_%p_%u_%u",
			  net, depth-1, tree[idx].base);

	    fprintf(vvp_out, ";\n");
	    free(tree);
      }
}

/*
 * Emit a DFF primitive. This uses the following syntax:
 *
 * .dff<variant> <width> <data>, <clock>, <enable>[, <async>[, <async-value>]];
 */
static void draw_lpm_ff(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      const char*clk_in;
      const char*d_in;
      const char*e_in;
      const char*clr_in;
      const char*set_in;

	/* Sync set/clear control is not currently supported. This is not
	 * a problem, as synthesis can incorporate this in the D input
	 * expression. All modern synthesis tools do this as a matter of
	 * course, as most cell libraries don't contain flip-flops with
	 * sync set/clear.
	 */
      assert(ivl_lpm_sync_clr(net) == 0);
      assert(ivl_lpm_sync_set(net) == 0);

      unsigned width = ivl_lpm_width(net);
      char*edge = ivl_lpm_negedge(net) ? "n" : "p";

      nex = ivl_lpm_clk(net);
      assert(nex);
      assert(width_of_nexus(nex) == 1);
      clk_in = draw_net_input(nex);

      nex = ivl_lpm_data(net,0);
      assert(nex);
      assert(width_of_nexus(nex) == width);
      d_in = draw_net_input(nex);

      nex = ivl_lpm_enable(net);
      if (nex) {
	    assert(width_of_nexus(nex) == 1);
	    e_in = draw_net_input(nex);
      } else {
	    e_in = ", C4<1>";
      }

      nex = ivl_lpm_async_clr(net);
      if (nex) {
	    assert(width_of_nexus(nex) == 1);
	    clr_in = draw_net_input(nex);
      } else {
	    clr_in = 0;
      }

      nex = ivl_lpm_async_set(net);
      if (nex) {
	    assert(width_of_nexus(nex) == 1);
	    set_in = draw_net_input(nex);
      } else {
	    set_in = 0;
      }

      if (clr_in) {
	      /* Synthesis doesn't currently support both set and clear.
		 If it ever does, it might be better to implement the
		 flip-flop as a UDP. See tgt-vlog95 for an example of
		 how to do this. */
	    if (set_in) {
		  fprintf(stderr, "%s:%u:vvp.tgt: sorry: No support for a DFF "
		                  "with both an async. set and clear.\n",
		                  ivl_lpm_file(net), ivl_lpm_lineno(net));
		  vvp_errors += 1;
	    }
	    fprintf(vvp_out, "L_%p .dff/%s/aclr %u ", net, edge, width);
      } else if (ivl_lpm_async_set(net)) {
	    fprintf(vvp_out, "L_%p .dff/%s/aset %u ", net, edge, width);
      } else {
	    fprintf(vvp_out, "L_%p .dff/%s %u ", net, edge, width);
      }

      fprintf(vvp_out, "%s, %s, %s", d_in, clk_in, e_in);

      if (clr_in) {
	    fprintf(vvp_out, ", %s", clr_in);
      }

      if (set_in) {
	    ivl_expr_t val = ivl_lpm_aset_value(net);
	    fprintf(vvp_out, ", %s", set_in);
	    if (val) {
		  unsigned nbits = ivl_expr_width(val);
		  const char*bits = ivl_expr_bits(val);
		  unsigned bb;
		  assert(nbits == width);
		  fprintf(vvp_out, ", C4<");
		  for (bb = 0 ;  bb < nbits;  bb += 1)
			fprintf(vvp_out, "%c", bits[nbits-bb-1]);
		  fprintf(vvp_out, ">");
	    }
      }

      fprintf(vvp_out, ";\n");
}

/*
 * Emit a LATCH primitive. This uses the following syntax:
 *
 * .latch <width> <data>, <enable>;
 */
static void draw_lpm_latch(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      const char*d_in;
      const char*e_in;

      unsigned width = ivl_lpm_width(net);

      nex = ivl_lpm_data(net,0);
      assert(nex);
      assert(width_of_nexus(nex) == width);
      d_in = draw_net_input(nex);

      nex = ivl_lpm_enable(net);
      assert(nex);
      assert(width_of_nexus(nex) == 1);
      e_in = draw_net_input(nex);

      fprintf(vvp_out, "L_%p .latch %u %s, %s;\n", net, width, d_in, e_in);
}

static void draw_lpm_shiftl(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      const char* signed_flag = ivl_lpm_signed(net)? "s" : "";
      const char*dly = draw_lpm_output_delay(net, IVL_VT_LOGIC);

      const char *lval = draw_net_input(ivl_lpm_data(net, 0));
      const char *rval = draw_net_input(ivl_lpm_data(net, 1));

      if (ivl_lpm_type(net) == IVL_LPM_SHIFTR)
	    fprintf(vvp_out, "L_%p%s .shift/r%s %u", net, dly, signed_flag,
	            width);
      else
	    fprintf(vvp_out, "L_%p%s .shift/l %u", net, dly, width);

      fprintf(vvp_out, ", %s, %s;\n", lval, rval);
}

static void draw_type_string_of_nex(ivl_nexus_t nex)
{
      switch (data_type_of_nexus(nex)) {
	  case IVL_VT_REAL:
	    fprintf(vvp_out, "r");
	    break;
	  case IVL_VT_LOGIC:
          case IVL_VT_BOOL:
	    fprintf(vvp_out, "v%u", width_of_nexus(nex));
	    break;
	  default:
	    assert(0);
	    break;
      }
}

/* Check to see if the output of the system function is connected with
 * a modpath. */
static int sfunc_has_modpath_output(ivl_lpm_t lptr)
{
	/* The q port is the output connection (nexus). */
      ivl_nexus_t nex = ivl_lpm_q(lptr);
      ivl_scope_t scope = ivl_lpm_scope(lptr);
      unsigned idx;

	/* Check to see if there is a signal connected to the output. */
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0) continue;
	      /* The modpath will be connected to the system function
	       * output. */
	    if ((ivl_signal_scope(sig) == scope) &&
	        (ivl_signal_port(sig) == IVL_SIP_OUTPUT) &&
	        (ivl_signal_npath(sig))) return 1;
      }

      return 0;
}

/* Emit a definition for a net that has a delay or modpath. */
static void draw_sfunc_output_def(ivl_lpm_t net, char type)
{
      ivl_nexus_t nex = ivl_lpm_q(net);
      const char *suf = (type == 'd') ? "/d" : "";

      switch (data_type_of_nexus(nex)) {
	case IVL_VT_REAL:
	    fprintf(vvp_out, "v%p_0 .net/real *\"_%c%p\", 0 0, L_%p%s;\n",
	            net, type, net, net, suf);
	    break;
	case IVL_VT_LOGIC:
	case IVL_VT_BOOL:
	    fprintf(vvp_out, "v%p_0 .net *\"_%c%p\", %u 0, L_%p%s;\n",
	            net, type, net, width_of_nexus(nex)-1, net, suf);
	    break;
	default:
	    assert(0);
	    break;
      }
}

static void draw_lpm_sfunc(ivl_lpm_t net)
{
      unsigned idx;

      ivl_variable_type_t dt = data_type_of_nexus(ivl_lpm_q(net));
      const char*dly = draw_lpm_output_delay(net, dt);

	/* Get all the input labels that I will use for net signals that
	   connect to the inputs of the function. */
      unsigned ninp = ivl_lpm_size(net);
      const char**input_strings = calloc(ninp, sizeof(char*));
      for (idx = 0 ;  idx < ninp ;  idx += 1)
	    input_strings[idx] = draw_net_input(ivl_lpm_data(net, idx));

	/* Because vvp uses a wide functor for the output of a system
	 * function we need to define the output delay net when needed,
	 * otherwise it will not be cleaned up correctly (gives a
	 * valgrind warning). */
      if (*dly != 0) {
	    draw_sfunc_output_def(net, 'd');
      }
	/* The same situation exists if a modpath is used to connect the
	 * system function output to the true output signal. For this case
	 * the modpath is the only thing connected to the UDP output. */
      if (sfunc_has_modpath_output(net)) {
	    draw_sfunc_output_def(net, 'm');
      }

      if (ivl_lpm_trigger(net))
            fprintf(vvp_out, "L_%p%s .sfunc/e %u %u \"%s\", E_%p", net, dly,
                    ivl_file_table_index(ivl_lpm_file(net)),
                    ivl_lpm_lineno(net), ivl_lpm_string(net),
                    ivl_lpm_trigger(net));
      else
            fprintf(vvp_out, "L_%p%s .sfunc %u %u \"%s\"", net, dly,
                    ivl_file_table_index(ivl_lpm_file(net)),
                    ivl_lpm_lineno(net), ivl_lpm_string(net));

	/* Print the function type descriptor string. */
      fprintf(vvp_out, ", \"");

      draw_type_string_of_nex(ivl_lpm_q(net));

      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 1)
	    draw_type_string_of_nex(ivl_lpm_data(net,idx));

      fprintf(vvp_out, "\"");

      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 1) {
	    fprintf(vvp_out, ", %s", input_strings[idx]);
      }

      fprintf(vvp_out, ";\n");
}

static void draw_lpm_ufunc(ivl_lpm_t net)
{
      unsigned idx;
      unsigned ninp;
      const char**input_strings;
      ivl_scope_t def = ivl_lpm_define(net);

      ivl_variable_type_t dt = data_type_of_nexus(ivl_lpm_q(net));
      const char*dly = draw_lpm_output_delay(net, dt);

      const char*type_string = "";
      switch (dt) {
	  case IVL_VT_REAL:
		type_string = "/real";
		break;
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
		type_string = "/vec4";
		break;
	  default:
		break;
      }

	/* Get all the input labels that I will use for net signals that
	   connect to the inputs of the function. */
      ninp = ivl_lpm_size(net);
      input_strings = calloc(ninp, sizeof(char*));
      for (idx = 0 ;  idx < ninp ;  idx += 1)
	    input_strings[idx] = draw_net_input(ivl_lpm_data(net, idx));

      if (ivl_lpm_trigger(net)) {
	    assert(ninp > 0);
	    fprintf(vvp_out, "L_%p%s .ufunc/e TD_%s, %u, E_%p", net, dly,
	            vvp_mangle_id(ivl_scope_name(def)),
	            ivl_lpm_width(net), ivl_lpm_trigger(net));
      } else
	    fprintf(vvp_out, "L_%p%s .ufunc%s TD_%s, %u", net, dly, type_string,
	            vvp_mangle_id(ivl_scope_name(def)),
	            ivl_lpm_width(net));

	/* Print all the net signals that connect to the input of the
	   function. */
      for (idx = 0 ;  idx < ninp ;  idx += 1) {
	    fprintf(vvp_out, ", %s", input_strings[idx]);
      }
      free(input_strings);

      assert((ninp+1) == ivl_scope_ports(def));

	/* Now print all the variables in the function scope that
	   receive the input values given in the previous list. */
      for (idx = 0 ;  idx < ninp ;  idx += 1) {
	    ivl_signal_t psig = ivl_scope_port(def, idx+1);

	    if (idx == 0)
		  fprintf(vvp_out, " (");
	    else
		  fprintf(vvp_out, ", ");

	    assert(ivl_signal_dimensions(psig) == 0);
	    fprintf(vvp_out, "v%p_0", psig);
      }

      if (ninp == 0)
	    fprintf(vvp_out, ",");
      else
	    fprintf(vvp_out, ")");
#if 0
	/* Now print the reference to the signal from which the
	   result is collected. */
      { ivl_signal_t psig = ivl_scope_port(def, 0);
        assert(ivl_lpm_width(net) == ivl_signal_width(psig));
	assert(ivl_signal_dimensions(psig) == 0);

	fprintf(vvp_out, " v%p_0", psig);
      }
#endif
        /* Finally, print the scope identifier. */
      fprintf(vvp_out, " S_%p;\n", def);
}

/*
 * Handle a PART SELECT device. This has a single input and output,
 * plus an optional extra input that is a non-constant base.
 */
static void draw_lpm_part(ivl_lpm_t net)
{
      unsigned width, base;
      ivl_nexus_t sel;

      const char*input = draw_net_input(ivl_lpm_data(net, 0));
      const char*dly = draw_lpm_output_delay(net, IVL_VT_LOGIC);

      width = ivl_lpm_width(net);
      base = ivl_lpm_base(net);
      sel = ivl_lpm_data(net,1);

      if (sel == 0) {
	    fprintf(vvp_out, "L_%p%s .part %s", net, dly, input);
	    fprintf(vvp_out, ", %u, %u;\n", base, width);
      } else {
	    const char*sel_symbol = draw_net_input(sel);
	    fprintf(vvp_out, "L_%p%s .part/v%s %s", net, dly,
	                     (ivl_lpm_signed(net) ? ".s" : ""),
	                     input);
	    fprintf(vvp_out, ", %s", sel_symbol);
	    fprintf(vvp_out, ", %u;\n", width);
      }
}

/*
 * Handle a PART SELECT PV device. Generate a .part/pv node that
 * includes the part input, and the geometry of the part.
 */
static void draw_lpm_part_pv(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned base  = ivl_lpm_base(net);
      unsigned signal_width = width_of_nexus(ivl_lpm_q(net));

      const char*input = draw_net_input(ivl_lpm_data(net, 0));

      fprintf(vvp_out, "L_%p .part/pv %s", net, input);
      fprintf(vvp_out, ", %u, %u, %u;\n", base, width, signal_width);
}

/*
 * Draw unary reduction devices.
 */
static void draw_lpm_re(ivl_lpm_t net, const char*type)
{
      const char*input = draw_net_input(ivl_lpm_data(net, 0));
      const char*dly = draw_lpm_output_delay(net, IVL_VT_LOGIC);

      fprintf(vvp_out, "L_%p%s .reduce/%s %s;\n", net, dly, type, input);
}

static void draw_lpm_repeat(ivl_lpm_t net)
{
      const char*input = draw_net_input(ivl_lpm_data(net, 0));
      const char*dly = draw_lpm_output_delay(net, IVL_VT_LOGIC);

      fprintf(vvp_out, "L_%p%s .repeat %u, %u, %s;\n", net, dly,
	      ivl_lpm_width(net), ivl_lpm_size(net), input);
}

static void draw_lpm_sign_ext(ivl_lpm_t net)
{
      const char*input = draw_net_input(ivl_lpm_data(net, 0));
      const char*dly = draw_lpm_output_delay(net, IVL_VT_LOGIC);

      fprintf(vvp_out, "L_%p%s .extend/s %u, %s;\n",
	      net, dly, ivl_lpm_width(net), input);
}

static void draw_lpm_in_scope(ivl_lpm_t net)
{
      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_ABS:
	    draw_lpm_abs(net);
	    return;

	  case IVL_LPM_CAST_INT:
	    draw_lpm_cast_int(net);
	    return;

	  case IVL_LPM_CAST_INT2:
	    draw_lpm_cast_int2(net);
	    return;

	  case IVL_LPM_CAST_REAL:
	    draw_lpm_cast_real(net);
	    return;

	  case IVL_LPM_ADD:
	  case IVL_LPM_SUB:
	  case IVL_LPM_MULT:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_POW:
	    draw_lpm_add(net);
	    return;

	  case IVL_LPM_ARRAY:
	    draw_lpm_array(net);
	    return;

	  case IVL_LPM_PART_VP:
	    draw_lpm_part(net);
	    return;

	  case IVL_LPM_PART_PV:
	    draw_lpm_part_pv(net);
	    return;

	  case IVL_LPM_CONCAT:
	  case IVL_LPM_CONCATZ:
	    draw_lpm_concat(net);
	    return;

	  case IVL_LPM_FF:
	    draw_lpm_ff(net);
	    return;

	  case IVL_LPM_LATCH:
	    draw_lpm_latch(net);
	    return;

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
	    draw_lpm_cmp(net);
	    return;

	  case IVL_LPM_MUX:
	    draw_lpm_mux(net);
	    return;

	  case IVL_LPM_RE_AND:
	    draw_lpm_re(net, "and");
	    return;
	  case IVL_LPM_RE_OR:
	    draw_lpm_re(net, "or");
	    return;
	  case IVL_LPM_RE_XOR:
	    draw_lpm_re(net, "xor");
	    return;
	  case IVL_LPM_RE_NAND:
	    draw_lpm_re(net, "nand");
	    return;
	  case IVL_LPM_RE_NOR:
	    draw_lpm_re(net, "nor");
	    return;
	  case IVL_LPM_RE_XNOR:
	    draw_lpm_re(net, "xnor");
	    return;

	  case IVL_LPM_REPEAT:
	    draw_lpm_repeat(net);
	    return;

	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    draw_lpm_shiftl(net);
	    return;

	  case IVL_LPM_SIGN_EXT:
	    draw_lpm_sign_ext(net);
	    return;

	  case IVL_LPM_SFUNC:
	    draw_lpm_sfunc(net);
	    return;

	  case IVL_LPM_SUBSTITUTE:
	    draw_lpm_substitute(net);
	    return;

	  case IVL_LPM_UFUNC:
	    draw_lpm_ufunc(net);
	    return;

	  default:
	    fprintf(stderr, "XXXX LPM not supported: %s.%s\n",
		    ivl_scope_name(ivl_lpm_scope(net)), ivl_lpm_basename(net));
      }
}


static const char *vvp_port_info_type_str(ivl_signal_port_t ptype)
{
      switch( ptype )
      {
      case IVL_SIP_INPUT :  return "/INPUT";
      case IVL_SIP_OUTPUT : return "/OUTPUT";
      case IVL_SIP_INOUT :  return "/INOUT";
      case IVL_SIP_NONE :   return "/NODIR";
      default :
        abort(); // NO SUPPORT FOR ANYTHING ELSE YET...
      }
}

int draw_scope(ivl_scope_t net, ivl_scope_t parent)
{
      unsigned idx;
      const char *type;

      const char*prefix = ivl_scope_is_auto(net) ? "auto" : "";
      char suffix[32];

      suffix[0] = 0;

      switch (ivl_scope_type(net)) {
      case IVL_SCT_MODULE:   type = "module";   break;
      case IVL_SCT_FUNCTION: type = "function"; break;
      case IVL_SCT_TASK:     type = "task";     break;
      case IVL_SCT_BEGIN:    type = "begin";    break;
      case IVL_SCT_FORK:     type = "fork";     break;
      case IVL_SCT_GENERATE: type = "generate"; break;
      case IVL_SCT_PACKAGE:  type = "package";  break;
      case IVL_SCT_CLASS:    type = "class";    break;
      default:               type = "?";        assert(0);
      }

      if (ivl_scope_type(net)==IVL_SCT_FUNCTION) {
	    switch (ivl_scope_func_type(net)) {
		case IVL_VT_LOGIC:
		  snprintf(suffix, sizeof suffix, ".vec4.%c%u",
			   ivl_scope_func_signed(net)? 'u' : 's',
			   ivl_scope_func_width(net));
		  break;
		case IVL_VT_BOOL:
		  snprintf(suffix, sizeof suffix, ".vec2.%c%u",
			   ivl_scope_func_signed(net)? 'u' : 's',
			   ivl_scope_func_width(net));
		  break;
		case IVL_VT_REAL:
		  snprintf(suffix, sizeof suffix, ".real");
		  break;
		case IVL_VT_STRING:
		  snprintf(suffix, sizeof suffix, ".str");
		  break;
		case IVL_VT_CLASS:
		case IVL_VT_DARRAY:
		case IVL_VT_QUEUE:
		  snprintf(suffix, sizeof suffix, ".obj");
		  break;
		case IVL_VT_VOID:
		  snprintf(suffix, sizeof suffix, ".void");
		  break;
		default:
		  assert(0);
		  break;
	    }
      }

      fprintf(vvp_out, "S_%p .scope %s%s%s, \"%s\" \"%s\" %u %u",
	      net, prefix, type, suffix,
	      vvp_mangle_name(ivl_scope_basename(net)),
              vvp_mangle_name(ivl_scope_tname(net)),
	      ivl_file_table_index(ivl_scope_file(net)),
              ivl_scope_lineno(net));

      if (parent) {
	    fprintf(vvp_out, ", %u %u %u, S_%p;\n",
	            ivl_file_table_index(ivl_scope_def_file(net)),
	            ivl_scope_def_lineno(net), ivl_scope_is_cell(net), parent);
      } else {

	    fprintf(vvp_out, ";\n");
      }

      fprintf(vvp_out, " .timescale %d %d;\n", ivl_scope_time_units(net),
                                               ivl_scope_time_precision(net));

      if( ivl_scope_type(net) == IVL_SCT_MODULE ) {

	      // Port data for VPI: needed for vpiPorts property of vpiModule
	    for( idx = 0; idx < ivl_scope_mod_module_ports(net); ++idx ) {
		  const char *name =  ivl_scope_mod_module_port_name(net,idx);
		  ivl_signal_port_t ptype = ivl_scope_mod_module_port_type(net,idx);
		  unsigned width = ivl_scope_mod_module_port_width(net,idx);
		  ivl_net_logic_t buffer = ivl_scope_mod_module_port_buffer(net,idx);
		  if( name == 0 )
			name = "";
		  fprintf( vvp_out, "    .port_info %u %s %u \"%s\"",
		           idx, vvp_port_info_type_str(ptype), width,
		           vvp_mangle_name(name) );
		  if (buffer) fprintf( vvp_out, " L_%p;\n", buffer);
		  else fprintf( vvp_out, ";\n");
	    }
      }

      for (idx = 0 ;  idx < ivl_scope_params(net) ;  idx += 1) {
	    ivl_parameter_t par = ivl_scope_param(net, idx);

	      // Skip type parameters for now. Support for type parameters
	      // should be added together with support for quering types through
	      // VPI.
	    if (ivl_parameter_is_type(par))
		  continue;

	    ivl_expr_t pex = ivl_parameter_expr(par);
	    switch (ivl_expr_type(pex)) {
		case IVL_EX_STRING:
		  fprintf(vvp_out, "P_%p .param/str \"%s\" %d %u %u, \"%s\";\n",
			  par, vvp_mangle_name(ivl_parameter_basename(par)),
			  ivl_parameter_local(par),
			  ivl_file_table_index(ivl_parameter_file(par)),
			  ivl_parameter_lineno(par),
			  ivl_expr_string(pex));
		  break;
		case IVL_EX_NUMBER:
		  fprintf(vvp_out, "P_%p .param/l \"%s\" %d %u %u, %sC4<",
			  par, vvp_mangle_name(ivl_parameter_basename(par)),
                          ivl_parameter_local(par),
			  ivl_file_table_index(ivl_parameter_file(par)),
			  ivl_parameter_lineno(par),
			  ivl_expr_signed(pex)? "+":"");
		  { const char*bits = ivl_expr_bits(pex);
		    unsigned nbits = ivl_expr_width(pex);
		    unsigned bb;
		    for (bb = 0 ;  bb < nbits;  bb += 1)
			  fprintf(vvp_out, "%c", bits[nbits-bb-1]);
		  }
		  fprintf(vvp_out, ">;\n");
		  break;
		case IVL_EX_REALNUM:
		  { char *res = draw_Cr_to_string(ivl_expr_dvalue(pex));
		    fprintf(vvp_out, "P_%p .param/real \"%s\" %d %u %u, %s; "
		            "value=%#g\n", par,
			    vvp_mangle_name(ivl_parameter_basename(par)),
	                    ivl_parameter_local(par),
			    ivl_file_table_index(ivl_parameter_file(par)),
			    ivl_parameter_lineno(par), res,
			    ivl_expr_dvalue(pex));
		    free(res);
		  }
		  break;
		default:
		  fprintf(vvp_out, "; parameter type %d unsupported\n",
			  ivl_expr_type(pex));
		  break;
	    }
      }

      for (idx = 0 ; idx < ivl_scope_classes(net) ; idx += 1) {
	    ivl_type_t class_type = ivl_scope_class(net,idx);
	    draw_class_in_scope(class_type);
      }

	/* Scan the scope for enumeration types, and write out
	   enumeration typespecs. */

      for (idx = 0 ; idx < ivl_scope_enumerates(net) ; idx += 1) {
	    ivl_enumtype_t enumtype = ivl_scope_enumerate(net, idx);
	    draw_enumeration_in_scope(enumtype);
      }

	/* Scan the scope for logic devices. For each device, draw out
	   a functor that connects pin 0 to the output, and the
	   remaining pins to inputs. */

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1) {
	    ivl_net_logic_t lptr = ivl_scope_log(net, idx);
	    draw_logic_in_scope(lptr);
      }


	/* Scan the signals (reg and net) and draw the appropriate
	   statements to make the signal function. */

      for (idx = 0 ;  idx < ivl_scope_sigs(net) ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(net, idx);

	    switch (ivl_signal_type(sig)) {
		case IVL_SIT_REG:
		  draw_reg_in_scope(sig);
		  break;
		default:
		  draw_net_in_scope(sig);
		  break;
	    }
      }

      for (idx = 0 ;  idx < ivl_scope_events(net) ;  idx += 1) {
	    ivl_event_t event = ivl_scope_event(net, idx);
	    draw_event_in_scope(event);
      }

      for (idx = 0 ;  idx < ivl_scope_lpms(net) ;  idx += 1) {
	    ivl_lpm_t lpm = ivl_scope_lpm(net, idx);
	    draw_lpm_in_scope(lpm);
      }

      for (idx = 0 ; idx < ivl_scope_switches(net) ; idx += 1) {
	    ivl_switch_t sw = ivl_scope_switch(net, idx);
	    draw_switch_in_scope(sw);
      }

      if (ivl_scope_type(net) == IVL_SCT_TASK)
	    draw_task_definition(net);

      if (ivl_scope_type(net) == IVL_SCT_FUNCTION)
	    vvp_errors += draw_func_definition(net);

      ivl_scope_children(net, (ivl_scope_f*) draw_scope, net);
      return 0;
}
