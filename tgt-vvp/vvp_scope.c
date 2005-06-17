/*
 * Copyright (c) 2001-2005 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: vvp_scope.c,v 1.129 2005/06/17 03:46:52 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>

/*
 *  Escape non-symbol chararacters in ids, and quotes in strings.
 */

inline static char hex_digit(unsigned i)
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
		  assert(out);
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
		  assert(out);
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

static void draw_C4_repeated_constant(char bit_char, unsigned width)
{
      unsigned idx;

      fprintf(vvp_out, "C4<");
      for (idx = 0 ;  idx < width ;  idx += 1)
	    fprintf(vvp_out, "%c", bit_char);

      fprintf(vvp_out, ">");
}

static void str_repeat(char*buf, const char*str, unsigned rpt)
{
      unsigned idx;
      size_t len = strlen(str);
      for (idx = 0 ;  idx < rpt ;  idx += 1) {
	    strcpy(buf, str);
	    buf += len;
      }
}

/*
 * Given a signal, generate a string name that is suitable for use as
 * a label. The only rule is that the same signal will always have the
 * same label. The result is stored in static memory, so remember to
 * copy it out.
 */
const char* vvp_signal_label(ivl_signal_t sig)
{
      static char buf[32];
      sprintf(buf, "$%p", sig);
      return buf;
}

const char* vvp_word_label(ivl_variable_t sig)
{
      static char buf[32];
      sprintf(buf, "$%p", sig);
      return buf;
}

/*
 * This makes a string suitable for use as a label for memories.
 */
const char* vvp_memory_label(ivl_memory_t mem)
{
      static char buf[32];
      sprintf(buf, "$%p", mem);
      return buf;
}

ivl_signal_type_t signal_type_of_nexus(ivl_nexus_t nex)
{
      unsigned idx;
      ivl_signal_type_t out = IVL_SIT_TRI;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_signal_type_t stype;
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0)
		  continue;

	    stype = ivl_signal_type(sig);
	    if (stype == IVL_SIT_REG)
		  continue;
	    if (stype == IVL_SIT_TRI)
		  continue;
	    if (stype == IVL_SIT_NONE)
		  continue;
	    out = stype;
      }

      return out;
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


ivl_nexus_ptr_t ivl_logic_pin_ptr(ivl_net_logic_t net, unsigned pin)
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
 * This tests a bufz device against an output receiver, and determines
 * if the device can be skipped. If this function returns true, then a
 * gate will be generated for this node. Otherwise, the code generator
 * will connect its input to its output and skip the gate.
 */
static int can_elide_bufz(ivl_net_logic_t net, ivl_nexus_ptr_t nptr)
{
      ivl_nexus_t in_n;
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

      return 1;
}

static void draw_C4_to_string(char*result, size_t nresult,
			      ivl_net_const_t cptr)
{
      const char*bits = ivl_const_bits(cptr);
      unsigned idx;

      char*dp = result;
      strcpy(dp, "C4<");
      dp += strlen(dp);

      for (idx = 0 ;  idx < ivl_const_width(cptr) ;  idx += 1) {
	    switch (bits[ivl_const_width(cptr)-idx-1]) {
		case '0':
		  *dp++ = '0';
		  break;
		case '1':
		  *dp++ = '1';
		  break;
		default:
		  *dp++ = bits[idx];
		  break;
	    }
	    assert(dp - result < nresult);
      }

      strcpy(dp, ">");
}

static void draw_C8_to_string(char*result, size_t nresult,
			      ivl_net_const_t cptr,
			      ivl_drive_t dr0, ivl_drive_t dr1)
{
      const char*bits = ivl_const_bits(cptr);
      unsigned idx;

      char dr0c = "01234567"[dr0];
      char dr1c = "01234567"[dr1];
      char*dp = result;

      strcpy(dp, "C8<");
      dp += strlen(dp);

      for (idx = 0 ;  idx < ivl_const_width(cptr) ;  idx += 1) {
	    switch (bits[ivl_const_width(cptr)-idx-1]) {
		case '0':
		  *dp++ = dr0c;
		  *dp++ = dr0c;
		  *dp++ = '0';
		  break;
		case '1':
		  *dp++ = dr1c;
		  *dp++ = dr1c;
		  *dp++ = '1';
		  break;
		case 'x':
		case 'X':
		  *dp++ = dr0c;
		  *dp++ = dr1c;
		  *dp++ = 'x';
		  break;
		case 'z':
		case 'Z':
		  *dp++ = '0';
		  *dp++ = '0';
		  *dp++ = 'z';
		  break;
		default:
		  assert(0);
		  break;
	    }
	    assert(dp - result < nresult);
      }

      strcpy(dp, ">");
}

/*
 * This function takes a nexus and looks for an input functor. It then
 * draws to the output a string that represents that functor. What we
 * are trying to do here is find the input to the net that is attached
 * to this nexus.
 */

static const char* draw_net_input_drive(ivl_nexus_t nex, ivl_nexus_ptr_t nptr)
{
      static char result[2048];
      unsigned nptr_pin = ivl_nexus_ptr_pin(nptr);
      ivl_net_const_t cptr;
      ivl_net_logic_t lptr;
      ivl_signal_t sptr;
      ivl_lpm_t lpm;

      lptr = ivl_nexus_ptr_log(nptr);
      if (lptr && (ivl_logic_type(lptr) == IVL_LO_BUFZ) && (nptr_pin == 0))
	    do {
		  if (! can_elide_bufz(lptr, nptr))
			break;

		  return draw_net_input(ivl_logic_pin(lptr, 1));
	    } while(0);

	/* If this is a pulldown device, then there is a single pin
	   that drives a constant value to the entire width of the
	   vector. The driver normally drives a pull0 value, so a C8<>
	   constant is appropriate, but if the drive is really strong,
	   then we can draw a C4<> constant instead. */
      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLDOWN)) {
	    if (ivl_nexus_ptr_drive0(nptr) == IVL_DR_STRONG) {
		  char*dp = result;
		  strcpy(dp, "C4<");
		  dp += strlen(dp);
		  str_repeat(dp, "0", ivl_logic_width(lptr));
		  dp += ivl_logic_width(lptr);
		  *dp++ = '>';
		  *dp = 0;
		  assert((dp-result) <= sizeof result);
		  return result;
	    } else {
		  char val[4];
		  char*dp = result;

		  val[0] = "01234567"[ivl_nexus_ptr_drive0(nptr)];
		  val[1] = val[0];
		  val[2] = '0';
		  val[3] = 0;

		  strcpy(dp, "C8<");
		  dp += strlen(dp);
		  str_repeat(dp, val, ivl_logic_width(lptr));
		  dp += 3*ivl_logic_width(lptr);
		  *dp++ = '>';
		  *dp = 0;
		  assert((dp-result) <= sizeof result);
		  return result;
	    }
      }

      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLUP)) {
	    if (ivl_nexus_ptr_drive1(nptr) == IVL_DR_STRONG) {
		  char*dp = result;
		  strcpy(dp, "C4<");
		  dp += strlen(dp);
		  str_repeat(dp, "1", ivl_logic_width(lptr));
		  dp += ivl_logic_width(lptr);
		  *dp++ = '>';
		  *dp = 0;
		  assert((dp-result) <= sizeof result);
		  return result;
	    } else {
		  char val[4];
		  char*dp = result;

		  val[0] = "01234567"[ivl_nexus_ptr_drive0(nptr)];
		  val[1] = val[0];
		  val[2] = '1';
		  val[3] = 0;

		  strcpy(dp, "C8<");
		  dp += strlen(dp);
		  str_repeat(dp, val, ivl_logic_width(lptr));
		  dp += 3*ivl_logic_width(lptr);
		  *dp++ = '>';
		  *dp = 0;
		  assert((dp-result) <= sizeof result);
		  return result;
	    }
      }

      if (lptr && (nptr_pin == 0)) {
	    sprintf(result, "L_%p", lptr);
	    return result;
      }

      sptr = ivl_nexus_ptr_sig(nptr);
      if (sptr && (ivl_signal_type(sptr) == IVL_SIT_REG)) {
	      /* Input is a .var. Note that these devices have only
		 exactly one pin (that carries a vector) so nptr_pin
		 must be 0. */
	    assert(nptr_pin == 0);
	    sprintf(result, "V_%s", vvp_signal_label(sptr));
	    return result;
      }

      cptr = ivl_nexus_ptr_con(nptr);
      if (cptr) {
	      /* Constants should have exactly 1 pin, with a vector value. */
	    assert(nptr_pin == 0);

	    if ((ivl_nexus_ptr_drive0(nptr) == IVL_DR_STRONG)
		&& (ivl_nexus_ptr_drive1(nptr) == IVL_DR_STRONG)) {

		  draw_C4_to_string(result, sizeof(result), cptr);

	    } else {
		  draw_C8_to_string(result, sizeof(result), cptr,
				    ivl_nexus_ptr_drive0(nptr),
				    ivl_nexus_ptr_drive1(nptr));
	    }
	    return result;
      }

      lpm = ivl_nexus_ptr_lpm(nptr);
      if (lpm) switch (ivl_lpm_type(lpm)) {

	  case IVL_LPM_FF:
	  case IVL_LPM_RAM:
	  case IVL_LPM_ADD:
	  case IVL_LPM_CONCAT:
	  case IVL_LPM_CMP_EEQ:
	  case IVL_LPM_CMP_EQ:
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
	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	  case IVL_LPM_SIGN_EXT:
	  case IVL_LPM_SUB:
	  case IVL_LPM_MULT:
	  case IVL_LPM_MUX:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_UFUNC:
	  case IVL_LPM_PART_VP:
	  case IVL_LPM_PART_PV: /* NOTE: This is only a partial driver. */
	  case IVL_LPM_REPEAT:
	    if (ivl_lpm_q(lpm, 0) == nex) {
		  sprintf(result, "L_%p", lpm);
		  return result;
	    }

	    break;

      }

      fprintf(stderr, "internal error: no input to nexus %s\n",
	      ivl_nexus_name(nex));
      assert(0);
      return "C<z>";
}

/*
 * This function draws the input to a net. What that means is that it
 * returns a static string that can be used to represent a resolved
 * driver to a nexus. If there are multiple drivers to the nexus, then
 * it writes out the resolver declarations needed to perform strength
 * resolution.
 *
 * The string that this returns is bound to the nexus, so the pointer
 * remains valid.
 */
const char* draw_net_input(ivl_nexus_t nex)
{
      ivl_signal_type_t res;
      char result[512];
      unsigned idx;
      int level;
      unsigned ndrivers = 0;
      static ivl_nexus_ptr_t *drivers = 0x0;
      static unsigned adrivers = 0;

      const char*resolv_type;

	/* If this nexus already has a label, then its input is
	   already figured out. Just return the existing label. */
      char*nex_private = (char*)ivl_nexus_get_private(nex);
      if (nex_private)
	    return nex_private;

      res = signal_type_of_nexus(nex);
      switch (res) {
	  case IVL_SIT_TRI:
	    resolv_type = "tri";
	    break;
	  case IVL_SIT_TRI0:
	    resolv_type = "tri0";
	    break;
	  case IVL_SIT_TRI1:
	    resolv_type = "tri1";
	    break;
	  case IVL_SIT_TRIAND:
	    resolv_type = "triand";
	    break;
	  case IVL_SIT_TRIOR:
	    resolv_type = "trior";
	    break;
	  default:
	    fprintf(stderr, "vvp.tgt: Unsupported signal type: %u\n", res);
	    assert(0);
	    resolv_type = "tri";
	    break;
      }


      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t nptr = ivl_nexus_ptr(nex, idx);

	      /* Skip input only pins. */
	    if ((ivl_nexus_ptr_drive0(nptr) == IVL_DR_HiZ)
		&& (ivl_nexus_ptr_drive1(nptr) == IVL_DR_HiZ))
		  continue;

	      /* Save this driver. */
	    if (ndrivers >= adrivers) {
		  adrivers += 4;
		  drivers = realloc(drivers, adrivers*sizeof(ivl_nexus_ptr_t));
		  assert(drivers);
	    }
	    drivers[ndrivers] = nptr;
	    ndrivers += 1;
      }

	/* If the nexus has no drivers, then send a constant HiZ into
	   the net. */
      if (ndrivers == 0) {
	    unsigned idx, wid = width_of_nexus(nex);
	    char*tmp = malloc(wid + 5);
	    nex_private = tmp;
	    strcpy(tmp, "C4<");
	    tmp += strlen(tmp);
	    switch (res) {
		case IVL_SIT_TRI:
		  for (idx = 0 ;  idx < wid ;  idx += 1)
			*tmp++ = 'z';
		  break;
		case IVL_SIT_TRI0:
		  for (idx = 0 ;  idx < wid ;  idx += 1)
			*tmp++ = '0';
		  break;
		case IVL_SIT_TRI1:
		  for (idx = 0 ;  idx < wid ;  idx += 1)
			*tmp++ = '1';
		  break;
		default:
		  assert(0);
	    }
	    *tmp++ = '>';
	    *tmp = 0;
	    ivl_nexus_set_private(nex, nex_private);
	    return nex_private;
      }


	/* If the nexus has exactly one driver, then simply draw
	   it. Note that this will *not* work if the nexus is not a
	   TRI type nexus. */
      if (ndrivers == 1 && res == IVL_SIT_TRI) {
	    nex_private = strdup(draw_net_input_drive(nex, drivers[0]));
	    ivl_nexus_set_private(nex, nex_private);
	    return nex_private;
      }

      level = 0;
      while (ndrivers) {
	    unsigned int inst;
	    for (inst = 0; inst < ndrivers; inst += 4) {
		  if (ndrivers > 4)
			fprintf(vvp_out, "RS_%p/%d/%d .resolv tri",
				nex, level, inst);
		  else
			fprintf(vvp_out, "RS_%p .resolv %s",
				nex, resolv_type);

		  for (idx = inst; idx < ndrivers && idx < inst+4; idx += 1) {
			if (level) {
			      fprintf(vvp_out, ", RS_%p/%d/%d",
				      nex, level - 1, idx*4);
			} else {
			      fprintf(vvp_out, ", %s",
				      draw_net_input_drive(nex, drivers[idx]));
			}
		  }
		  for ( ;  idx < inst+4 ;  idx += 1) {
			fprintf(vvp_out, ", ");
			draw_C4_repeated_constant('z',width_of_nexus(nex));
		  }

		  fprintf(vvp_out, ";\n");
	    }
	    if (ndrivers > 4)
		  ndrivers = (ndrivers+3) / 4;
	    else
		  ndrivers = 0;
	    level += 1;
      }

      sprintf(result, "RS_%p", nex);
      nex_private = strdup(result);
      ivl_nexus_set_private(nex, nex_private);
      return nex_private;
}



/*
 * This function looks at the nexus in search of the net to attach
 * functor inputs to. Sort the signals in the nexus by name, and
 * choose the lexically earliest one.
 */
void draw_input_from_net(ivl_nexus_t nex)
{
      const char*nex_private = (const char*)ivl_nexus_get_private(nex);
      if (nex_private == 0)
	    nex_private = draw_net_input(nex);
      assert(nex_private);
      fprintf(vvp_out, "%s", nex_private);
}

/*
 * This function draws a reg/int/variable in the scope. This is a very
 * simple device to draw as there are no inputs to connect so no need
 * to scan the nexus.
 */
static void draw_reg_in_scope(ivl_signal_t sig)
{
      int msb = ivl_signal_msb(sig);
      int lsb = ivl_signal_lsb(sig);

      const char*signed_flag = ivl_signal_integer(sig) ? "/i" :
			       ivl_signal_signed(sig)? "/s" : "";

      fprintf(vvp_out, "V_%s .var%s \"%s\", %d, %d;\n",
	      vvp_signal_label(sig), signed_flag,
	      vvp_mangle_name(ivl_signal_basename(sig)), msb, lsb);
}

/*
 * This function draws a net. This is a bit more complicated as we
 * have to find an appropriate functor to connect to the input.
 */
static void draw_net_in_scope(ivl_signal_t sig)
{
      int msb = ivl_signal_msb(sig);
      int lsb = ivl_signal_lsb(sig);
      typedef const char*const_charp;
      const char* arg;

      const char*signed_flag = ivl_signal_signed(sig)? "/s" : "";

	/* Skip the local signal. */
      if (ivl_signal_local(sig))
	    return;

	/* Connect the pin of the signal to something. */
      {
	    ivl_nexus_t nex = ivl_signal_nex(sig);
	    arg = draw_net_input(nex);
      }

      fprintf(vvp_out, "V_%s .net%s \"%s\", %d, %d, %s;\n",
	      vvp_signal_label(sig), signed_flag,
	      vvp_mangle_name(ivl_signal_basename(sig)), msb, lsb, arg);

}

static void draw_delay(ivl_net_logic_t lptr)
{
      unsigned d0 = ivl_logic_delay(lptr, 0);
      unsigned d1 = ivl_logic_delay(lptr, 1);
      unsigned d2 = ivl_logic_delay(lptr, 2);

      if (d0 == 0 && d1 == 0 && d2 == 0)
	    return;

      if (d0 == d1 && d1 == d2)
	    fprintf(vvp_out, " (%d)", d0);
      else
	    fprintf(vvp_out, " (%d,%d,%d)", d0, d1, d2);
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
		"UDP_%s .udp/sequ \"%s\", %d, %d",
		vvp_mangle_id(ivl_udp_name(udp)),
		vvp_mangle_name(ivl_udp_name(udp)),
		ivl_udp_nin(udp),
		init );
  else
	fprintf(vvp_out,
		"UDP_%s .udp/comb \"%s\", %d",
		vvp_mangle_id(ivl_udp_name(udp)),
		vvp_mangle_name(ivl_udp_name(udp)),
		ivl_udp_nin(udp));

  for (i=0; i<ivl_udp_rows(udp); i++)
    fprintf(vvp_out, "\n ,\"%s\"", ivl_udp_row(udp, i) );

  fprintf(vvp_out, ";\n");
}

static void draw_udp_in_scope(ivl_net_logic_t lptr)
{
  unsigned pdx;

  ivl_udp_t udp = ivl_logic_udp(lptr);

  static ivl_udp_t *udps = 0x0;
  static int nudps = 0;
  int i;

  for (i=0; i<nudps; i++)
    if (udps[i] == udp)
      break;

  if (i >= nudps)
    {
      udps = realloc(udps, (nudps+1)*sizeof(ivl_udp_t));
      assert(udps);
      udps[nudps++] = udp;
      draw_udp_def(udp);
    }

  fprintf(vvp_out, "L_%p .udp", lptr);
  fprintf(vvp_out, " UDP_%s",
	  vvp_mangle_id(ivl_udp_name(udp)));
  draw_delay(lptr);

  for (pdx = 1 ;  pdx < ivl_logic_pins(lptr) ;  pdx += 1) {
	ivl_nexus_t nex = ivl_logic_pin(lptr, pdx);

	  /* Unlike other logic gates, primitives may have unconnected
	     inputs. The proper behavior is to attach a HiZ to the
	     port. */
	if (nex == 0) {
	      assert(ivl_logic_width(lptr) == 1);
	      fprintf(vvp_out, ", C4<z>");

	} else {
	      fprintf(vvp_out, ", ");
	      draw_input_from_net(nex);
	}
  }

  fprintf(vvp_out, ";\n");
}

static void draw_logic_in_scope(ivl_net_logic_t lptr)
{
      unsigned pdx;
      const char*ltype = "?";
      const char*lcasc = 0x0;
      char identity_val = '0';

      unsigned vector_width = width_of_nexus(ivl_logic_pin(lptr, 0));

      ivl_drive_t str0, str1;

      int level;
      int ninp = ivl_logic_pins(lptr) - 1;
      typedef const char*const_charp;
      const_charp*input_strings = calloc(ninp, sizeof(const_charp));

      for (pdx = 0 ;  pdx < ninp ;  pdx += 1) {
	    ivl_nexus_t nex = ivl_logic_pin(lptr, pdx+1);
	    if (nex == 0) {
		    /* Only UDPs can have unconnected inputs. */
		  assert(ivl_logic_type(lptr) == IVL_LO_UDP);
		  input_strings[pdx] = 0;
	    } else {
		  input_strings[pdx] = draw_net_input(nex);
	    }
      }

      switch (ivl_logic_type(lptr)) {

          case IVL_LO_UDP:
	    free(input_strings);
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

	  case IVL_LO_PULLDOWN:
	  case IVL_LO_PULLUP:
	      /* Skip pullup and pulldown objects. Things that have
		 pull objects as inputs will instead generate the
		 appropriate C<?> symbol. */
	    free(input_strings);
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

	  case IVL_LO_PMOS:
	    ltype = "PMOS";
	    break;

	  case IVL_LO_NMOS:
	    ltype = "NMOS";
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
	    fprintf(stderr, "vvp.tgt: error: Unhandled logic type: %u\n",
		    ivl_logic_type(lptr));
	    ltype = "?";
	    break;
      }

      { ivl_nexus_t nex = ivl_logic_pin(lptr, 0);
        ivl_nexus_ptr_t nptr = 0;
        unsigned idx;
	for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	      nptr = ivl_nexus_ptr(nex,idx);
	      if (ivl_nexus_ptr_log(nptr) != lptr)
		    continue;
	      if (ivl_nexus_ptr_pin(nptr) != 0)
		    continue;
	      break;
	}
        str0 = ivl_nexus_ptr_drive0(nptr);
	str1 = ivl_nexus_ptr_drive1(nptr);
      }

      if (!lcasc)
	lcasc = ltype;

	/* Get all the input label that I will use for parameters to
	   the functor that I create later. */
      ninp = ivl_logic_pins(lptr) - 1;
      input_strings = calloc(ninp, sizeof(char*));
      for (pdx = 0 ;  pdx < ninp ;  pdx += 1)
	    input_strings[pdx] = draw_net_input(ivl_logic_pin(lptr, pdx+1));

      level = 0;
      ninp = ivl_logic_pins(lptr) - 1;
      while (ninp) {
	    int inst;
	    for (inst = 0; inst < ninp; inst += 4) {
		  if (ninp > 4)
			fprintf(vvp_out, "L_%p/%d/%d .functor %s %u",
				lptr, level, inst, lcasc, vector_width);
		  else {
			fprintf(vvp_out, "L_%p .functor %s %u",
				lptr, ltype, vector_width);

			draw_delay(lptr);

			if (str0 != IVL_DR_STRONG || str1 != IVL_DR_STRONG)
			      fprintf(vvp_out, " [%u %u]", str0, str1);

		  }
		  for (pdx = inst; pdx < ninp && pdx < inst+4 ; pdx += 1) {
			if (level) {
			      fprintf(vvp_out, ", L_%p/%d/%d",
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
}

static void draw_event_in_scope(ivl_event_t obj)
{
      unsigned nany = ivl_event_nany(obj);
      unsigned nneg = ivl_event_nneg(obj);
      unsigned npos = ivl_event_npos(obj);

      unsigned cnt = 0;

	/* Figure out how many probe functors are needed. */
      if (nany > 0)
	    cnt += (nany+3) / 4;

      if (nneg > 0)
	    cnt += (nneg+3) / 4;

      if (npos > 0)
	    cnt += (npos+3) / 4;

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

	    for (idx = 0 ;  idx < nany ;  idx += 4, ecnt += 1) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%p/%u .event edge", obj, ecnt);

		  top = idx + 4;
		  if (nany < top)
			top = nany;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, sub);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    for (idx = 0 ;  idx < nneg ;  idx += 4, ecnt += 1) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%p/%u .event negedge", obj, ecnt);

		  top = idx + 4;
		  if (nneg < top)
			top = nneg;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, sub);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    for (idx = 0 ;  idx < npos ;  idx += 4, ecnt += 1) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%p/%u .event posedge", obj, ecnt);

		  top = idx + 4;
		  if (npos < top)
			top = npos;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, sub);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    assert(ecnt == cnt);

	    fprintf(vvp_out, "E_%p .event/or", obj);
	    fprintf(vvp_out, " E_%p/0",  obj);

	    for (idx = 1 ;  idx < cnt ;  idx += 1)
		  fprintf(vvp_out, ", E_%p/%u", obj, idx);

	    fprintf(vvp_out, ";\n");

      } else {
	    unsigned num_input_strings = nany + nneg + npos;
	    unsigned idx;
	    const char* input_strings[4];
	    const char*edge = 0;

	    if (nany > 0) {
		  assert((nneg + npos) == 0);
		  assert(nany <= 4);

		  edge = "edge";

		  for (idx = 0 ;  idx < nany ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, idx);
			input_strings[idx] = draw_net_input(nex);
		  }

	    } else if (nneg > 0) {
		  assert((nany + npos) == 0);
		  edge = "negedge";

		  for (idx = 0 ;  idx < nneg ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, idx);
			input_strings[idx] = draw_net_input(nex);
		  }

	    } else {
		  assert((nany + nneg) == 0);
		  edge = "posedge";

		  for (idx = 0 ;  idx < npos ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, idx);
			input_strings[idx] = draw_net_input(nex);
		  }
	    }

	    fprintf(vvp_out, "E_%p .event %s", obj, edge);
	    for (idx = 0 ;  idx < num_input_strings ;  idx += 1)
		  fprintf(vvp_out, ", %s", input_strings[idx]);

	    fprintf(vvp_out, ";\n");
      }
}

static void draw_lpm_ram(ivl_lpm_t net)
{
      ivl_memory_t mem = ivl_lpm_memory(net);
      ivl_nexus_t clk = ivl_lpm_clk(net);
      ivl_nexus_t pin;

      if (clk) {
	    fprintf(vvp_out, "CLK_%p .event posedge, ", net);
	    draw_input_from_net(clk);
	    fprintf(vvp_out, ";\n");
      }

      fprintf(vvp_out, "L_%p .mem/port M_%s, ", net, vvp_memory_label(mem));

      pin = ivl_lpm_select(net);
      draw_input_from_net(pin);

      if (clk) {
	    fprintf(vvp_out, ", CLK_%p, ",  net);
	    pin = ivl_lpm_enable(net);
	    if (pin)
		  draw_input_from_net(pin);
	    else
		  fprintf(vvp_out, "C4<1>");

	    pin = ivl_lpm_data(net, 0);
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(pin);
      }

      fprintf(vvp_out, ";\n");
}

static void draw_lpm_arith_a_b_inputs(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned idx;

      assert(width > 0);
      ivl_nexus_t nex = ivl_lpm_data(net, 0);
      ivl_signal_t sig = 0;

      ivl_nexus_ptr_t np;
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    np = ivl_nexus_ptr(nex,idx);
	    sig = ivl_nexus_ptr_sig(np);
	    if (sig != 0)
		  break;
      }

      assert(sig != 0);

      fprintf(vvp_out, ", V_%s", vvp_signal_label(sig));

      sig = 0;
      nex = ivl_lpm_data(net, 1);
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    np = ivl_nexus_ptr(nex,idx);
	    sig = ivl_nexus_ptr_sig(np);
	    if (sig != 0)
		  break;
      }

      assert(sig != 0);

      fprintf(vvp_out, ", V_%s", vvp_signal_label(sig));
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

static void draw_lpm_add(ivl_lpm_t net)
{
      const char*src_table[2];
      unsigned width;
      const char*type = "";

      width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_ADD:
	    type = "sum";
	    break;
	  case IVL_LPM_SUB:
	    type = "sub";
	    break;
	  case IVL_LPM_MULT:
	    type = "mult";
	    break;
	  case IVL_LPM_DIVIDE:
	    if (ivl_lpm_signed(net))
		  type = "div.s";
	    else
		  type = "div";
	    break;
	  case IVL_LPM_MOD:
	    type = "mod";
	    break;
	  default:
	    assert(0);
      }

      draw_lpm_data_inputs(net, 0, 2, src_table);
      fprintf(vvp_out, "L_%p .arith/%s %u, %s, %s;\n",
	      net, type, width, src_table[0], src_table[1]);
}

static void draw_lpm_cmp(ivl_lpm_t net)
{
      const char*src_table[2];
      unsigned width;
      const char*type = "";
      const char*signed_string = ivl_lpm_signed(net)? ".s" : "";

      width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_CMP_EEQ:
	    type = "eeq";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_EQ:
	    type = "eq";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_GE:
	    type = "ge";
	    break;
	  case IVL_LPM_CMP_GT:
	    type = "gt";
	    break;
	  case IVL_LPM_CMP_NE:
	    type = "ne";
	    signed_string = "";
	    break;
	  case IVL_LPM_CMP_NEE:
	    type = "nee";
	    signed_string = "";
	    break;
	  default:
	    assert(0);
      }

      draw_lpm_data_inputs(net, 0, 2, src_table);
      fprintf(vvp_out, "L_%p .cmp/%s%s %u, %s, %s;\n",
	      net, type, signed_string, width,
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
      unsigned icnt = ivl_lpm_selects(net);

      if (icnt <= 4) {
	      /* This is the easies case. There are 4 or fewer input
		 vectors, so the entire IVL_LPM_CONCAT can be
		 implemented with a single .concat node. */
	    draw_lpm_data_inputs(net, 0, icnt, src_table);
	    fprintf(vvp_out, "L_%p .concat ", net);
	    lpm_concat_inputs(net, 0, icnt, src_table);

      } else {
	      /* If there are more then 4 inputs, things get more
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
		  fprintf(vvp_out, "LS_%p_0_%u .concat ", net, idx);
		  wid = lpm_concat_inputs(net, idx, trans, src_table);

		  tree[idx/4].base = idx;
		  tree[idx/4].wid  = wid;
	    }

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

			fprintf(vvp_out, "LS_%p_%u_%u .concat [",
				net, depth, idx);

			for (tdx = 0 ;  tdx < trans ;  tdx += 1) {
			      fprintf(vvp_out, " %u", tree[idx].wid);
			      wid += tree[idx].wid;
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
	    fprintf(vvp_out, "L_%p .concat [", net);
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
 *  primitive FD (q, clk, ce, d);
 *    output q;
 *    reg q;
 *    input clk, ce, d;
 *    table
 *    // clk ce  d r s   q   q+
 *        r   1  0 0 0 : ? : 0;
 *        r   1  1 0 0 : ? : 1;
 *        f   1  ? 0 0 : ? : -;
 *        ?   1  ? 0 0 : ? : -;
 *        *   0  ? 0 0 : ? : -;
 *        ?   ?  ? 1 ? : ? : 0;
 *        ?   ?  ? 0 1 : ? : 1;
 *    endtable
 *  endprimitive
 */
static void draw_lpm_ff(ivl_lpm_t net)
{
      ivl_expr_t aset_expr = 0;
      const char*aset_bits = 0;

      ivl_nexus_t nex;
      unsigned width;

      width = ivl_lpm_width(net);

      aset_expr = ivl_lpm_aset_value(net);
      if (aset_expr) {
	    assert(ivl_expr_width(aset_expr) == width);
	    aset_bits = ivl_expr_bits(aset_expr);
      }


      fprintf(vvp_out, "L_%p .dff ", net);

      nex = ivl_lpm_data(net,0);
      assert(nex);
      draw_input_from_net(nex);

      nex = ivl_lpm_clk(net);
      assert(nex);
      fprintf(vvp_out, ", ");
      draw_input_from_net(nex);

      nex = ivl_lpm_enable(net);
      if (nex) {
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(nex);
      } else {
	    fprintf(vvp_out, ", C4<1>");
      }

	/* Stub asynchronous input for now. */
      fprintf(vvp_out, ", C4<z>");

      fprintf(vvp_out, ";\n");
}

static void draw_lpm_shiftl(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);


      if (ivl_lpm_type(net) == IVL_LPM_SHIFTR)
	    fprintf(vvp_out, "L_%p .shift/r %u", net, width);
      else
	    fprintf(vvp_out, "L_%p .shift/l %u", net, width);

      fprintf(vvp_out, ", ");
      draw_input_from_net(ivl_lpm_data(net, 0));

      fprintf(vvp_out, ", ");
      draw_input_from_net(ivl_lpm_data(net, 1));

      fprintf(vvp_out, ";\n");
}

static void draw_lpm_ufunc(ivl_lpm_t net)
{
      unsigned idx;
      ivl_scope_t def = ivl_lpm_define(net);

      fprintf(vvp_out, "L_%p .ufunc TD_%s, %u", net,
	      ivl_scope_name(def),
	      ivl_lpm_width(net));

	/* Print all the net signals that connect to the input of the
	   function. */
      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 1) {
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(ivl_lpm_data(net, idx));
      }


      assert((ivl_lpm_size(net)+1) == ivl_scope_ports(def));

	/* Now print all the variables in the function scope that
	   receive the input values given in the previous list. */
      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 1) {
	    ivl_signal_t psig = ivl_scope_port(def, idx+1);

	    if (idx == 0)
		  fprintf(vvp_out, " (");
	    else
		  fprintf(vvp_out, ", ");

	    fprintf(vvp_out, "V_%s", vvp_signal_label(psig));
      }

      fprintf(vvp_out, ")");

	/* Finally, print the reference to the signal from which the
	   result is collected. */
      { ivl_signal_t psig = ivl_scope_port(def, 0);
        assert(ivl_lpm_width(net) == ivl_signal_width(psig));

	fprintf(vvp_out, " V_%s", vvp_signal_label(psig));
      }

      fprintf(vvp_out, ";\n");
}

/*
 * Handle a PART SELECT device. This has a single input and output,
 * plus an optional extra input that is a non-constant base.
 */
static void draw_lpm_part(ivl_lpm_t net)
{
      unsigned width, base;
      ivl_nexus_t sel;

      width = ivl_lpm_width(net);
      base = ivl_lpm_base(net);
      sel = ivl_lpm_data(net,1);

      if (sel == 0) {
	    fprintf(vvp_out, "L_%p .part ", net);
	    draw_input_from_net(ivl_lpm_data(net, 0));
	    fprintf(vvp_out, ", %u, %u;\n", base, width);
      } else {
	    fprintf(vvp_out, "L_%p .part/v ", net);
	    draw_input_from_net(ivl_lpm_data(net,0));
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(sel);
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
      unsigned signal_width = width_of_nexus(ivl_lpm_q(net,0));

      fprintf(vvp_out, "L_%p .part/pv ", net);
      draw_input_from_net(ivl_lpm_data(net, 0));

      fprintf(vvp_out, ", %u, %u, %u;\n", base, width, signal_width);
}

/*
 * Draw unary reduction devices.
 */
static void draw_lpm_re(ivl_lpm_t net, const char*type)
{
      fprintf(vvp_out, "L_%p .reduce/%s ", net, type);
      draw_input_from_net(ivl_lpm_data(net,0));
      fprintf(vvp_out, ";\n");
}

static void draw_lpm_repeat(ivl_lpm_t net)
{
      fprintf(vvp_out, "L_%p .repeat %u, %u, ", net,
	      ivl_lpm_width(net), ivl_lpm_size(net));
      draw_input_from_net(ivl_lpm_data(net,0));
      fprintf(vvp_out, ";\n");
}

static void draw_lpm_sign_ext(ivl_lpm_t net)
{
      fprintf(vvp_out, "L_%p .extend/s %u, ", net,
	      ivl_lpm_width(net));
      draw_input_from_net(ivl_lpm_data(net,0));
      fprintf(vvp_out, ";\n");
}

static void draw_lpm_in_scope(ivl_lpm_t net)
{
      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_RAM:
	    draw_lpm_ram(net);
	    return;

	  case IVL_LPM_ADD:
	  case IVL_LPM_SUB:
	  case IVL_LPM_MULT:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	    draw_lpm_add(net);
	    return;

	  case IVL_LPM_PART_VP:
	    draw_lpm_part(net);
	    return;

	  case IVL_LPM_PART_PV:
	    draw_lpm_part_pv(net);
	    return;

	  case IVL_LPM_CONCAT:
	    draw_lpm_concat(net);
	    return;

	  case IVL_LPM_FF:
	    draw_lpm_ff(net);
	    return;

	  case IVL_LPM_CMP_EEQ:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_CMP_NEE:
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

	  case IVL_LPM_UFUNC:
	    draw_lpm_ufunc(net);
	    return;

	  default:
	    fprintf(stderr, "XXXX LPM not supported: %s.%s\n",
		    ivl_scope_name(ivl_lpm_scope(net)), ivl_lpm_basename(net));
      }
}


static void draw_mem_in_scope(ivl_memory_t net)
{
      int root = ivl_memory_root(net);
      int last = root + ivl_memory_size(net) - 1;
      int msb = ivl_memory_width(net) - 1;
      int lsb = 0;
      fprintf(vvp_out, "M_%s .mem \"%s\", %u,%u, %u,%u;\n",
	      vvp_memory_label(net),
	      vvp_mangle_name(ivl_memory_basename(net)),
	      msb, lsb, root, last);
}


int draw_scope(ivl_scope_t net, ivl_scope_t parent)
{
      unsigned idx;
      const char *type;
      switch (ivl_scope_type(net)) {
      case IVL_SCT_MODULE:   type = "module";   break;
      case IVL_SCT_FUNCTION: type = "function"; break;
      case IVL_SCT_TASK:     type = "task";     break;
      case IVL_SCT_BEGIN:    type = "begin";    break;
      case IVL_SCT_FORK:     type = "fork";     break;
      default:               type = "?";        assert(0);
      }

      fprintf(vvp_out, "S_%p .scope %s, \"%s\" \"%s\"",
	      net, type, vvp_mangle_name(ivl_scope_basename(net)),
	      ivl_scope_tname(net));

      if (parent) {
	    fprintf(vvp_out, ", S_%p;\n", parent);
      } else {

	    fprintf(vvp_out, ";\n");
      }

      fprintf(vvp_out, " .timescale %d;\n", ivl_scope_time_units(net));

      for (idx = 0 ;  idx < ivl_scope_params(net) ;  idx += 1) {
	    ivl_parameter_t par = ivl_scope_param(net, idx);
	    ivl_expr_t pex = ivl_parameter_expr(par);
	    switch (ivl_expr_type(pex)) {
		case IVL_EX_STRING:
		  fprintf(vvp_out, "P_%p .param \"%s\", string, \"%s\";\n",
			  par, ivl_parameter_basename(par),
			  ivl_expr_string(pex));
		  break;
		default:
		  break;
	    }
      }

	/* Scan the scope for logic devices. For each device, draw out
	   a functor that connects pin 0 to the output, and the
	   remaining pins to inputs. */

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1) {
	    ivl_net_logic_t lptr = ivl_scope_log(net, idx);
	    draw_logic_in_scope(lptr);
      }


	/* Scan the scope for word variables. */
      for (idx = 0 ;  idx < ivl_scope_vars(net) ;  idx += 1) {
	    ivl_variable_t var = ivl_scope_var(net, idx);
	    const char*type = "real";

	    fprintf(vvp_out, "W_%s .word %s, \"%s\";\n",
		    vvp_word_label(var), type,
		    ivl_variable_name(var));
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

      for (idx = 0 ;  idx < ivl_scope_mems(net) ;  idx += 1) {
	    ivl_memory_t mem = ivl_scope_mem(net, idx);
	    draw_mem_in_scope(mem);
      }

      for (idx = 0 ;  idx < ivl_scope_lpms(net) ;  idx += 1) {
	    ivl_lpm_t lpm = ivl_scope_lpm(net, idx);
	    draw_lpm_in_scope(lpm);
      }

      if (ivl_scope_type(net) == IVL_SCT_TASK)
	    draw_task_definition(net);

      if (ivl_scope_type(net) == IVL_SCT_FUNCTION)
	    draw_func_definition(net);

      ivl_scope_children(net, (ivl_scope_f*) draw_scope, net);
      return 0;
}

/*
 * $Log: vvp_scope.c,v $
 * Revision 1.129  2005/06/17 03:46:52  steve
 *  Make functors know their own width.
 *
 * Revision 1.128  2005/05/24 01:44:28  steve
 *  Do sign extension of structuran nets.
 *
 * Revision 1.127  2005/05/08 23:44:08  steve
 *  Add support for variable part select.
 *
 * Revision 1.126  2005/04/24 23:44:02  steve
 *  Update DFF support to new data flow.
 *
 * Revision 1.125  2005/04/06 05:29:09  steve
 *  Rework NetRamDq and IVL_LPM_RAM nodes.
 *
 * Revision 1.124  2005/04/04 05:29:53  steve
 *  Generate the right coes for unconnected UDP port.
 *
 * Revision 1.123  2005/03/19 06:23:49  steve
 *  Handle LPM shifts.
 *
 * Revision 1.122  2005/03/18 02:56:04  steve
 *  Add support for LPM_UFUNC user defined functions.
 *
 * Revision 1.121  2005/03/09 05:52:04  steve
 *  Handle case inequality in netlists.
 *
 * Revision 1.120  2005/03/09 04:53:40  steve
 *  Generate code for new form of memory ports.
 *
 * Revision 1.119  2005/02/13 01:15:07  steve
 *  Replace supply nets with wires connected to pullup/down supply devices.
 *
 * Revision 1.118  2005/02/12 22:54:29  steve
 *  Implement a-b muxes as vector devices
 *
 * Revision 1.117  2005/02/12 06:25:15  steve
 *  Draw C4 and C8 constants to account for strength.
 *
 * Revision 1.116  2005/02/10 04:55:45  steve
 *  Get the C4 width right for undriven nexa.
 *
 * Revision 1.115  2005/02/08 00:12:36  steve
 *  Add the NetRepeat node, and code generator support.
 *
 * Revision 1.114  2005/02/04 05:13:57  steve
 *  Support .concat with arbitrary input counts.
 *
 * Revision 1.113  2005/02/03 04:56:21  steve
 *  laborate reduction gates into LPM_RED_ nodes.
 *
 * Revision 1.112  2005/01/22 16:22:13  steve
 *  LPM_CMP_NE/EQ are vectored devices.
 *
 * Revision 1.111  2005/01/22 01:06:55  steve
 *  Change case compare from logic to an LPM node.
 *
 * Revision 1.110  2005/01/16 04:20:32  steve
 *  Implement LPM_COMPARE nodes as two-input vector functors.
 *
 * Revision 1.109  2005/01/12 05:31:50  steve
 *  More robust input code generation for LPM_ADD.
 *
 * Revision 1.108  2005/01/12 03:16:35  steve
 *  More complete drawing of concat inputs.
 *
 * Revision 1.107  2005/01/10 01:42:59  steve
 *  Handle concatenations with up to 16 inputs.
 *
 * Revision 1.106  2005/01/09 20:16:01  steve
 *  Use PartSelect/PV and VP to handle part selects through ports.
 *
 * Revision 1.105  2004/12/29 23:52:09  steve
 *  Generate code for the .concat functors, from NetConcat objects.
 *  Generate C<> constants of correct widths for functor arguments.
 *
 * Revision 1.104  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.103  2004/10/04 01:10:57  steve
 *  Clean up spurious trailing white space.
 */

