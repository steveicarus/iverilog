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
#ident "$Id: vvp_scope.c,v 1.160 2007/04/12 04:40:37 steve Exp $"
#endif

# include  "vvp_priv.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <inttypes.h>
# include  <assert.h>

struct vvp_nexus_data {
	/* draw_net_input uses this */
      const char*net_input;
      unsigned drivers_count;
      int flags;
	/* draw_net_in_scope uses these to identify the controlling word. */
      ivl_signal_t net;
      unsigned net_word;
};
#define VVP_NEXUS_DATA_STR 0x0001


static struct vvp_nexus_data*new_nexus_data()
{
      struct vvp_nexus_data*data = calloc(1, sizeof(struct vvp_nexus_data));
      return data;
}

/*
 *  Escape non-symbol characters in ids, and quotes in strings.
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

ivl_signal_t signal_of_nexus(ivl_nexus_t nex, unsigned*word)
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

/*
 * Given a nexus, look for a signal that has module delay
 * paths. Return that signal. (There should be no more than 1.) If we
 * don't find any, then return nil.
 */
static ivl_signal_t find_modpath(ivl_nexus_t nex)
{
      unsigned idx;
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex,idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0)
		  continue;
	    if (ivl_signal_npath(sig) == 0)
		  continue;

	    return sig;
      }

      return 0;
}

static char* draw_C4_to_string(ivl_net_const_t cptr)
{
      const char*bits = ivl_const_bits(cptr);
      unsigned idx;

      size_t result_len = 5 + ivl_const_width(cptr);
      char*result = malloc(result_len);
      char*dp = result;
      strcpy(dp, "C4<");
      dp += strlen(dp);

      for (idx = 0 ;  idx < ivl_const_width(cptr) ;  idx += 1) {
	    char bitchar = bits[ivl_const_width(cptr)-idx-1];
	    *dp++ = bitchar;
	    assert((dp - result) < result_len);
      }

      strcpy(dp, ">");
      return result;
}

static char* draw_C8_to_string(ivl_net_const_t cptr,
			       ivl_drive_t dr0, ivl_drive_t dr1)
{
      size_t nresult = 5 + 3*ivl_const_width(cptr);
      char*result = malloc(nresult);
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
      return result;
}

/*
 * This function takes a nexus and looks for an input functor. It then
 * draws to the output a string that represents that functor. What we
 * are trying to do here is find the input to the net that is attached
 * to this nexus.
 */

static char* draw_net_input_drive(ivl_nexus_t nex, ivl_nexus_ptr_t nptr)
{
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

		  return strdup(draw_net_input(ivl_logic_pin(lptr, 1)));
	    } while(0);

	/* If this is a pulldown device, then there is a single pin
	   that drives a constant value to the entire width of the
	   vector. The driver normally drives a pull0 value, so a C8<>
	   constant is appropriate, but if the drive is really strong,
	   then we can draw a C4<> constant instead. */
      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLDOWN)) {
	    if (ivl_nexus_ptr_drive0(nptr) == IVL_DR_STRONG) {
		  size_t result_len = ivl_logic_width(lptr) + 5;
		  char*result = malloc(result_len);
		  char*dp = result;
		  strcpy(dp, "C4<");
		  dp += strlen(dp);
		  str_repeat(dp, "0", ivl_logic_width(lptr));
		  dp += ivl_logic_width(lptr);
		  *dp++ = '>';
		  *dp = 0;
		  assert((dp-result) <= result_len);
		  return result;
	    } else {
		  char val[4];
		  size_t result_len = 3*ivl_logic_width(lptr) + 5;
		  char*result = malloc(result_len);
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
		  assert((dp-result) <= result_len);
		  return result;
	    }
      }

      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLUP)) {
	    if (ivl_nexus_ptr_drive1(nptr) == IVL_DR_STRONG) {
		  size_t result_len = 5 + ivl_logic_width(lptr);
		  char*result = malloc(result_len);
		  char*dp = result;
		  strcpy(dp, "C4<");
		  dp += strlen(dp);
		  str_repeat(dp, "1", ivl_logic_width(lptr));
		  dp += ivl_logic_width(lptr);
		  *dp++ = '>';
		  *dp = 0;
		  assert((dp-result) <= result_len);
		  return result;
	    } else {
		  char val[4];
		  size_t result_len = 5 + 3*ivl_logic_width(lptr);
		  char*result = malloc(result_len);
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
		  assert((dp-result) <= result_len);
		  return result;
	    }
      }

      if (lptr && (nptr_pin == 0)) {
	    char tmp[128];
	    snprintf(tmp, sizeof tmp, "L_%p", lptr);
	    return strdup(tmp);
      }

      sptr = ivl_nexus_ptr_sig(nptr);
      if (sptr && (ivl_signal_type(sptr) == IVL_SIT_REG)) {
	    char tmp[128];
	      /* Input is a .var. This device may be a non-zero pin
	         because it may be an array of reg vectors. */
	    snprintf(tmp, sizeof tmp, "v%p_%u", sptr, nptr_pin);
	    return strdup(tmp);
      }

      cptr = ivl_nexus_ptr_con(nptr);
      if (cptr) {
	      /* Constants should have exactly 1 pin, with a literal value. */
	    assert(nptr_pin == 0);
	    char *result = 0;

	    switch (ivl_const_type(cptr)) {
		case IVL_VT_LOGIC:
		case IVL_VT_BOOL:
		  if ((ivl_nexus_ptr_drive0(nptr) == IVL_DR_STRONG)
		      && (ivl_nexus_ptr_drive1(nptr) == IVL_DR_STRONG)) {

			result = draw_C4_to_string(cptr);

		  } else {
			result = draw_C8_to_string(cptr,
						   ivl_nexus_ptr_drive0(nptr),
						   ivl_nexus_ptr_drive1(nptr));
		  }
		  break;

		case IVL_VT_REAL:
		    { char tmp[256];
		      snprintf(tmp, sizeof(tmp),
			       "Cr<%lg>", ivl_const_real(cptr));
		      result = strdup(tmp);
		    }
		  break;

		default:
		  assert(0);
		  break;
	    }

	    return result;
      }

      lpm = ivl_nexus_ptr_lpm(nptr);
      if (lpm) switch (ivl_lpm_type(lpm)) {

	  case IVL_LPM_FF:
	  case IVL_LPM_ADD:
	  case IVL_LPM_ARRAY:
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
	  case IVL_LPM_SFUNC:
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
		  char tmp[128];
		  snprintf(tmp, sizeof tmp, "L_%p", lpm);
		  return strdup(tmp);
	    }
	    break;

	  case IVL_LPM_PART_BI:
	    if (ivl_lpm_q(lpm, 0) == nex) {
		  char tmp[128];
		  snprintf(tmp, sizeof tmp, "L_%p/P", lpm);
		  return strdup(tmp);
	    } else if (ivl_lpm_data(lpm,0) == nex) {
		  char tmp[128];
		  snprintf(tmp, sizeof tmp, "L_%p/V", lpm);
		  return strdup(tmp);
	    }
	    break;
      }

      fprintf(stderr, "internal error: no input to nexus %s\n",
	      ivl_nexus_name(nex));
      assert(0);
      return strdup("C<z>");
}

static int nexus_drive_is_strength_aware(ivl_nexus_ptr_t nptr)
{
      if (ivl_nexus_ptr_drive0(nptr) != IVL_DR_STRONG)
	    return 1;
      if (ivl_nexus_ptr_drive1(nptr) != IVL_DR_STRONG)
	    return 1;

      ivl_net_logic_t log = ivl_nexus_ptr_log(nptr);
      if (log != 0) {
	      /* These logic gates are able to generate unusual
	         strength values and so their outputs are considered
	         strength aware. */
	    if (ivl_logic_type(log) == IVL_LO_BUFIF0)
		  return 1;
	    if (ivl_logic_type(log) == IVL_LO_BUFIF1)
		  return 1;
	    if (ivl_logic_type(log) == IVL_LO_PMOS)
		  return 1;
	    if (ivl_logic_type(log) == IVL_LO_NMOS)
		  return 1;
	    if (ivl_logic_type(log) == IVL_LO_CMOS)
		  return 1;
      }

      return 0;
}

/*
 * This function draws the input to a net into a string. What that
 * means is that it returns a static string that can be used to
 * represent a resolved driver to a nexus. If there are multiple
 * drivers to the nexus, then it writes out the resolver declarations
 * needed to perform strength resolution.
 *
 * The string that this returns is malloced, and that means that the
 * caller must free the string or store it permanently. This function
 * does *not* check for a previously calculated string. Use the
 * draw_net_input for the general case.
 */
  /* Omit LPMPART_BI device pin-data(0) drivers. */
# define OMIT_PART_BI_DATA 0x0001

static char* draw_net_input_x(ivl_nexus_t nex,
			      ivl_nexus_ptr_t omit_ptr, int omit_flags,
			      struct vvp_nexus_data*nex_data)
{
      ivl_signal_type_t res;
      char result[512];
      unsigned idx;
      int level;
      unsigned ndrivers = 0;
      static ivl_nexus_ptr_t *drivers = 0x0;
      static unsigned adrivers = 0;

      const char*resolv_type;

      char*nex_private = 0;

	/* Accumulate nex_data flags. */
      int nex_flags = 0;

      res = signal_type_of_nexus(nex);
      switch (res) {
	  case IVL_SIT_TRI:
	    resolv_type = "tri";
	    break;
	  case IVL_SIT_TRI0:
	    resolv_type = "tri0";
	    nex_flags |= VVP_NEXUS_DATA_STR;
	    break;
	  case IVL_SIT_TRI1:
	    resolv_type = "tri1";
	    nex_flags |= VVP_NEXUS_DATA_STR;
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
	    ivl_lpm_t lpm_tmp;
	    ivl_nexus_ptr_t nptr = ivl_nexus_ptr(nex, idx);

	      /* If we are supposed to skip LPM_PART_BI data pins,
		 check that this driver is that. */
	    if ((omit_flags&OMIT_PART_BI_DATA)
		&& (lpm_tmp = ivl_nexus_ptr_lpm(nptr))
		&& (nex == ivl_lpm_data(lpm_tmp,0)))
		  continue;

	    if (nptr == omit_ptr)
		  continue;

	      /* Skip input only pins. */
	    if ((ivl_nexus_ptr_drive0(nptr) == IVL_DR_HiZ)
		&& (ivl_nexus_ptr_drive1(nptr) == IVL_DR_HiZ))
		  continue;

	      /* Mark the strength-aware flag if the driver can
		 generate values other than the standard "6"
		 strength. */
	    if (nexus_drive_is_strength_aware(nptr))
		  nex_flags |= VVP_NEXUS_DATA_STR;

	      /* Save this driver. */
	    if (ndrivers >= adrivers) {
		  adrivers += 4;
		  drivers = realloc(drivers, adrivers*sizeof(ivl_nexus_ptr_t));
		  assert(drivers);
	    }
	    drivers[ndrivers] = nptr;
	    ndrivers += 1;
      }

	/* If the caller is collecting nexus information, then save
	   the nexus driver count in the nex_data. */
      if (nex_data) {
	    nex_data->drivers_count = ndrivers;
	    nex_data->flags |= nex_flags;
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
	    return nex_private;
      }


	/* If the nexus has exactly one driver, then simply draw
	   it. Note that this will *not* work if the nexus is not a
	   TRI type nexus. */
      if (ndrivers == 1 && res == IVL_SIT_TRI) {
	    ivl_signal_t path_sig = find_modpath(nex);
	    if (path_sig) {
		  char*nex_str = draw_net_input_drive(nex, drivers[0]);
		  char modpath_label[64];
		  snprintf(modpath_label, sizeof modpath_label,
			   "V_%p/m", path_sig);
		  nex_private = strdup(modpath_label);
		  draw_modpath(path_sig, nex_str);

	    } else {
		  nex_private = draw_net_input_drive(nex, drivers[0]);
	    }
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
			      char*drive = draw_net_input_drive(nex, drivers[idx]);
			      fprintf(vvp_out, ", %s", drive);
			      free(drive);
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
      return nex_private;
}

/*
 * Get a cached description of the nexus input, or create one if this
 * nexus has not been cached yet. This is a wrapper for the common
 * case call to draw_net_input_x.
 */
const char*draw_net_input(ivl_nexus_t nex)
{
      struct vvp_nexus_data*nex_data = (struct vvp_nexus_data*)
	    ivl_nexus_get_private(nex);

	/* If this nexus already has a label, then its input is
	   already figured out. Just return the existing label. */
      if (nex_data && nex_data->net_input)
	    return nex_data->net_input;

      if (nex_data == 0) {
	    nex_data = new_nexus_data();
	    ivl_nexus_set_private(nex, nex_data);
      }

      assert(nex_data->net_input == 0);
      nex_data->net_input = draw_net_input_x(nex, 0, 0, nex_data);

      return nex_data->net_input;
}

const char*draw_input_from_net(ivl_nexus_t nex)
{
      static char result[32];
      unsigned word;

      ivl_signal_t sig = signal_of_nexus(nex, &word);
      if (sig == 0)
	    return draw_net_input(nex);

      snprintf(result, sizeof result, "v%p_%u", sig, word);
      return result;
}


/*
 * This function draws a reg/int/variable in the scope. This is a very
 * simple device to draw as there are no inputs to connect so no need
 * to scan the nexus. We do have to account for the possibility that
 * the device is arrayed, though, by making a node for each array element.
 */
static void draw_reg_in_scope(ivl_signal_t sig)
{
      int msb = ivl_signal_msb(sig);
      int lsb = ivl_signal_lsb(sig);

      const char*datatype_flag = ivl_signal_integer(sig) ? "/i" :
			       ivl_signal_signed(sig)? "/s" : "";
      const char*local_flag = ivl_signal_local(sig)? "*" : "";

      switch (ivl_signal_data_type(sig)) {
	  case IVL_VT_REAL:
	    datatype_flag = "/real";
	    break;
	  default:
	    break;
      }

	/* If the reg objects are collected into an array, then first
	   write out the .array record to declare the array indices. */
      if (ivl_signal_dimensions(sig) > 0) {
	    unsigned word_count = ivl_signal_array_count(sig);
	    int last = ivl_signal_array_base(sig)+word_count-1;
	    int first = ivl_signal_array_base(sig);
	    fprintf(vvp_out, "v%p .array%s \"%s\", %d %d, %d %d;\n",
		    sig, datatype_flag,
		    vvp_mangle_name(ivl_signal_basename(sig)),
		    last, first, msb, lsb);

      } else {

	    fprintf(vvp_out, "v%p_0 .var%s %s\"%s\", %d %d;%s\n",
		    sig, datatype_flag, local_flag,
		    vvp_mangle_name(ivl_signal_basename(sig)), msb, lsb,
		    ivl_signal_local(sig)? " Local signal" : "");
      }
}


/*
 * This function draws a net. This is a bit more complicated as we
 * have to find an appropriate functor to connect to the input.
 */
static void draw_net_in_scope(ivl_signal_t sig)
{
      int msb = ivl_signal_msb(sig);
      int lsb = ivl_signal_lsb(sig);

      const char*datatype_flag = ivl_signal_signed(sig)? "/s" : "";
      const char*local_flag = ivl_signal_local(sig)? "*" : "";
      unsigned iword;

      switch (ivl_signal_data_type(sig)) {
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
			int last = ivl_signal_array_base(sig) + word_count-1;
			int first = ivl_signal_array_base(sig);
			fprintf(vvp_out, "v%p .array \"%s\", %d %d;\n",
				sig, vvp_mangle_name(ivl_signal_basename(sig)),
				last, first);
		  }
		  if (dimensions > 0) {
			/* If this is a word of an array, then use an
			   array reference in place of the net name. */
			fprintf(vvp_out, "v%p_%u .net%s%s v%p %u, %d %d, %s;"
				" %u drivers%s\n",
				sig, iword, vec8, datatype_flag, sig,
				iword, msb, lsb, driver,
				nex_data->drivers_count,
				strength_aware_flag?", strength-aware":"");
		  } else {
			/* If this is an isolated word, it uses its
			   own name. */
			assert(word_count == 1);
			fprintf(vvp_out, "v%p_%u .net%s%s %s\"%s\", %d %d, %s;"
				" %u drivers%s\n",
				sig, iword, vec8, datatype_flag, local_flag,
				vvp_mangle_name(ivl_signal_basename(sig)),
				msb, lsb, driver,
				nex_data->drivers_count,
				strength_aware_flag?", strength-aware":"");
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
		  assert(word_count == ivl_signal_array_count(nex_data->net));

		  if (iword == 0) {
			fprintf(vvp_out, "v%p .array \"%s\", v%p; Alias to %s\n",
				sig, vvp_mangle_name(ivl_signal_basename(sig)),
				nex_data->net, ivl_signal_basename(nex_data->net));
		  }
		    /* An alias for the individual words? */
#if 0
		  fprintf(vvp_out, "v%p_%u .alias%s v%p, %d %d, v%p_%u;\n",
			  sig, iword, datatype_flag, sig,
			  msb, lsb, nex_data->net, nex_data->net_word);
#endif
	    } else {
		    /* Finally, we may have an alias that is a word
		       connected to another word. Again, this is a
		       case of port collapsing. */

		    /* For the alias, create a different kind of node
		       that refers to the alias source data instead of
		       holding our own data. */
		  fprintf(vvp_out, "v%p_%u .alias%s \"%s\", %d %d, v%p_%u;\n",
			  sig, iword, datatype_flag,
			  vvp_mangle_name(ivl_signal_basename(sig)),
			  msb, lsb, nex_data->net, nex_data->net_word);
	    }
      }
}

static void draw_delay(ivl_net_logic_t lptr)
{
      ivl_expr_t d0 = ivl_logic_delay(lptr, 0);
      ivl_expr_t d1 = ivl_logic_delay(lptr, 1);
      ivl_expr_t d2 = ivl_logic_delay(lptr, 2);

      if (d0 == 0 && d1 == 0 && d2 == 0)
	    return;

	/* FIXME: Assume that the expression is a constant */
      assert(number_is_immediate(d0, 64));
      assert(number_is_immediate(d1, 64));
      assert(number_is_immediate(d2, 64));

      if (d0 == d1 && d1 == d2)
	    fprintf(vvp_out, " (%lu)", get_number_immediate(d0));
      else
	    fprintf(vvp_out, " (%lu,%lu,%lu)",
		    get_number_immediate(d0),
		    get_number_immediate(d1),
		    get_number_immediate(d2));
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
	      fprintf(vvp_out, ", %s", draw_net_input(nex));
	}
  }

  fprintf(vvp_out, ";\n");
}

static void draw_logic_in_scope(ivl_net_logic_t lptr)
{
      unsigned pdx;
      const char*ltype = "?";
      const char*lcasc = 0;
      char identity_val = '0';

      int need_delay_flag = ivl_logic_delay(lptr,0)? 1 : 0;

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
			fprintf(vvp_out, "L_%p%s .functor %s %u",
				lptr, need_delay_flag? "/d" : "",
				ltype, vector_width);

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

	/* If there are delays, then draw the delay functor to carry
	   that delay. This is the final output. */
      if (need_delay_flag) {
	    ivl_expr_t rise_exp  = ivl_logic_delay(lptr,0);
	    ivl_expr_t fall_exp  = ivl_logic_delay(lptr,1);
	    ivl_expr_t decay_exp = ivl_logic_delay(lptr,2);

	    if (number_is_immediate(rise_exp,64)
		&& number_is_immediate(fall_exp,64)
		&& number_is_immediate(decay_exp,64)) {

		  fprintf(vvp_out, "L_%p .delay (%lu,%lu,%lu) L_%p/d;\n",
			  lptr, get_number_immediate(rise_exp),
			  get_number_immediate(rise_exp),
			  get_number_immediate(rise_exp), lptr);
	    } else {
		  ivl_signal_t sig;
		  assert(ivl_expr_type(rise_exp) == IVL_EX_SIGNAL);
		  assert(ivl_expr_type(fall_exp) == IVL_EX_SIGNAL);
		  assert(ivl_expr_type(decay_exp) == IVL_EX_SIGNAL);

		  fprintf(vvp_out, "L_%p .delay  L_%p/d", lptr, lptr);

		  sig = ivl_expr_signal(rise_exp);
		  assert(ivl_signal_array_count(sig) == 1);
		  fprintf(vvp_out, ", v%p_0", sig);

		  sig = ivl_expr_signal(fall_exp);
		  assert(ivl_signal_array_count(sig) == 1);
		  fprintf(vvp_out, ", v%p_0", sig);

		  sig = ivl_expr_signal(decay_exp);
		  assert(ivl_signal_array_count(sig) == 1);
		  fprintf(vvp_out, ", v%p_0;\n", sig);
	    }
      }
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
			fprintf(vvp_out, ", %s", draw_input_from_net(nex));
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
			fprintf(vvp_out, ", %s", draw_input_from_net(nex));
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
			fprintf(vvp_out, ", %s", draw_input_from_net(nex));
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
	    ivl_nexus_t input_nexa[4];
	    const char*edge = 0;

	    assert(num_input_strings <= 4);

	    if (nany > 0) {
		  assert((nneg + npos) == 0);
		  assert(nany <= 4);

		  edge = "edge";

		  for (idx = 0 ;  idx < nany ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, idx);
			input_nexa[idx] = nex;
		  }

	    } else if (nneg > 0) {
		  assert((nany + npos) == 0);
		  edge = "negedge";

		  for (idx = 0 ;  idx < nneg ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, idx);
			input_nexa[idx] = nex;
		  }

	    } else {
		  assert((nany + nneg) == 0);
		  edge = "posedge";

		  for (idx = 0 ;  idx < npos ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, idx);
			input_nexa[idx] = nex;
		  }
	    }

	    fprintf(vvp_out, "E_%p .event %s", obj, edge);
	    for (idx = 0 ;  idx < num_input_strings ;  idx += 1)
		  fprintf(vvp_out, ", %s", draw_input_from_net(input_nexa[idx]));

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

static void draw_lpm_add(ivl_lpm_t net)
{
      const char*src_table[2];
      unsigned width;
      const char*type = "";
      ivl_variable_type_t dta = data_type_of_nexus(ivl_lpm_data(net,0));
      ivl_variable_type_t dtb = data_type_of_nexus(ivl_lpm_data(net,1));
      ivl_variable_type_t dto = IVL_VT_LOGIC;

      if (dta == IVL_VT_REAL && dtb == IVL_VT_REAL)
	    dto = IVL_VT_REAL;

      width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_ADD:
	    type = "sum";
	    break;
	  case IVL_LPM_SUB:
	    if (dto == IVL_VT_REAL)
		  type = "sub.r";
	    else
		  type = "sub";
	    break;
	  case IVL_LPM_MULT:
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
	    type = "mod";
	    break;
	  default:
	    assert(0);
      }

      draw_lpm_data_inputs(net, 0, 2, src_table);
      fprintf(vvp_out, "L_%p .arith/%s %u, %s, %s;\n",
	      net, type, width, src_table[0], src_table[1]);
}

/*
* The read port to an array is generated as a single record that takes
* the address as an input.
*/
static void draw_lpm_array(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      ivl_signal_t mem = ivl_lpm_array(net);

      fprintf(vvp_out, "L_%p .array/port v%p, ", net, mem);

      nex = ivl_lpm_select(net);
      fprintf(vvp_out, "%s", draw_net_input(nex));

      fprintf(vvp_out, ";\n");
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
	      /* This is the easiest case. There are 4 or fewer input
		 vectors, so the entire IVL_LPM_CONCAT can be
		 implemented with a single .concat node. */
	    draw_lpm_data_inputs(net, 0, icnt, src_table);
	    fprintf(vvp_out, "L_%p .concat ", net);
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
		  fprintf(vvp_out, "LS_%p_0_%u .concat ", net, idx);
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

			fprintf(vvp_out, "LS_%p_%u_%u .concat [",
				net, depth, idx);

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
      fprintf(vvp_out, "%s", draw_net_input(nex));

      nex = ivl_lpm_clk(net);
      assert(nex);
      fprintf(vvp_out, ", %s", draw_net_input(nex));

      nex = ivl_lpm_enable(net);
      if (nex) {
	    fprintf(vvp_out, ", %s", draw_net_input(nex));
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
      const char* signed_flag = ivl_lpm_signed(net)? "s" : "";

      if (ivl_lpm_type(net) == IVL_LPM_SHIFTR)
	    fprintf(vvp_out, "L_%p .shift/r%s %u", net, signed_flag, width);
      else
	    fprintf(vvp_out, "L_%p .shift/l %u", net, width);

      fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net, 0)));

      fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net, 1)));

      fprintf(vvp_out, ";\n");
}

static void draw_type_string_of_nex(ivl_nexus_t nex)
{
      switch (data_type_of_nexus(nex)) {
	  case IVL_VT_REAL:
	    fprintf(vvp_out, "r");
	    break;
	  case IVL_VT_LOGIC:
          case IVL_VT_BOOL:
	    fprintf(vvp_out, "v%d", width_of_nexus(nex));
	    break;
	  default:
	    assert(0);
	    break;
      }
}

static void draw_lpm_sfunc(ivl_lpm_t net)
{
      unsigned idx;
      fprintf(vvp_out, "L_%p .sfunc \"%s\"", net, ivl_lpm_string(net));

	/* Print the function type descriptor string. */
      fprintf(vvp_out, ", \"");

      draw_type_string_of_nex(ivl_lpm_q(net,0));

      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 1)
	    draw_type_string_of_nex(ivl_lpm_data(net,idx));

      fprintf(vvp_out, "\"");

      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 1) {
	    fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,idx)));
      }

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
	    fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net, idx)));
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

	    assert(ivl_signal_array_count(psig) == 1);
	    fprintf(vvp_out, "v%p_0", psig);
      }

      fprintf(vvp_out, ")");

	/* Finally, print the reference to the signal from which the
	   result is collected. */
      { ivl_signal_t psig = ivl_scope_port(def, 0);
        assert(ivl_lpm_width(net) == ivl_signal_width(psig));
	assert(ivl_signal_array_count(psig) == 1);

	fprintf(vvp_out, " v%p_0", psig);
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
	    fprintf(vvp_out, "L_%p .part %s",
		    net, draw_net_input(ivl_lpm_data(net, 0)));
	    fprintf(vvp_out, ", %u, %u;\n", base, width);
      } else {
	    fprintf(vvp_out, "L_%p .part/v %s",
		    net, draw_net_input(ivl_lpm_data(net,0)));
	    fprintf(vvp_out, ", %s", draw_net_input(sel));
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

      fprintf(vvp_out, "L_%p .part/pv %s",
	      net, draw_net_input(ivl_lpm_data(net, 0)));

      fprintf(vvp_out, ", %u, %u, %u;\n", base, width, signal_width);
}

/*
 * Handle the drawing of a bi-directional part select. The two ports
 * are simultaneously input and output. A simple minded connect of the
 * input to the output causes a functor cycle which will lock into an
 * X value, so something special is needed.
 *
 * NOTE: The inputs of the tran device at this point need to be from
 * all the drivers of the nexus *except* the tran itself. This
 * function will draw three labels that can be linked:
 *
 * The ivl_lpm_q of a part(bi) may be a smaller vector then the
 * ivl_lpm_data, the tran acts like a forward part select in that
 * way.
 *
 * The device creates these nodes:
 *
 * - L_%p/i
 * This is the Q port of the tran resolved and padded to the maximum
 * width of the tran. The tran itself is not included in the
 * resolution of this port.
 *
 * - L_%p/V
 * This is the Q and D parts resolved together, still without the tran
 * driving anything.
 *
 * - L_%p/P
 * This is the /V node part-selected back to the dimensions of the Q
 * side.
 */
static void draw_lpm_part_bi(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned base  = ivl_lpm_base(net);
      unsigned signal_width = width_of_nexus(ivl_lpm_data(net,0));

      unsigned idx;
      ivl_nexus_t nex;
      ivl_nexus_ptr_t ptr = 0;

      char*p_str;
      char*v_str;

	/* It seems implausible that the two inputs of a tran will be
	   connected together. So assert that this is so to simplify
	   the code to look for the nexus_ptr_t objects. */
      assert(ivl_lpm_q(net,0) != ivl_lpm_data(net,0));

      nex = ivl_lpm_q(net,0);
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ptr = ivl_nexus_ptr(nex, idx);
	    if (ivl_nexus_ptr_lpm(ptr) == net)
		  break;
      }
      assert(ptr != 0);
      p_str = draw_net_input_x(nex, ptr, 0, 0);

      nex = ivl_lpm_data(net,0);
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ptr = ivl_nexus_ptr(nex, idx);
	    if (ivl_nexus_ptr_lpm(ptr) == net)
		  break;
      }
      v_str = draw_net_input_x(nex, ptr, OMIT_PART_BI_DATA, 0);

	/* Pad the part-sized input out to a common width...
	   The /i label is the Q side of the tran, resolved except for
	   the tran itself and padded (with z) to the larger width. */
      fprintf(vvp_out, "L_%p/i .part/pv %s, %u, %u, %u;\n",
	      net, p_str, base, width, signal_width);

	/* Resolve together the two halves of the tran...
	   The /V label is the ports of the tran (now the same width)
	   resolved together. Neither input to this resolver includes
	   the tran itself. */
      fprintf(vvp_out, "L_%p/V .resolv tri, L_%p/i, %s;\n",
	      net, net, v_str);

	/* The full-width side is created by the tran device, all we
	   have left to to is take a part select of that for the
	   smaller output, and this becomes the part select output of
	   the BI device. */
      fprintf(vvp_out, "L_%p/P .part L_%p/V, %u, %u;\n", net,
	      net, base, width);

      free(p_str);
      free(v_str);
}

/*
 * Draw unary reduction devices.
 */
static void draw_lpm_re(ivl_lpm_t net, const char*type)
{
      fprintf(vvp_out, "L_%p .reduce/%s %s;\n",
	      net, type, draw_net_input(ivl_lpm_data(net,0)));
}

static void draw_lpm_repeat(ivl_lpm_t net)
{
      fprintf(vvp_out, "L_%p .repeat %u, %u, %s;\n", net,
	      ivl_lpm_width(net), ivl_lpm_size(net),
	      draw_net_input(ivl_lpm_data(net,0)));
}

static void draw_lpm_sign_ext(ivl_lpm_t net)
{
      fprintf(vvp_out, "L_%p .extend/s %u, %s;\n",
	      net, ivl_lpm_width(net),
	      draw_net_input(ivl_lpm_data(net,0)));
}

static void draw_lpm_in_scope(ivl_lpm_t net)
{
      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_ADD:
	  case IVL_LPM_SUB:
	  case IVL_LPM_MULT:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	    draw_lpm_add(net);
	    return;

	  case IVL_LPM_ARRAY:
	    draw_lpm_array(net);
	    return;

	  case IVL_LPM_PART_BI:
	    draw_lpm_part_bi(net);
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

	  case IVL_LPM_SFUNC:
	    draw_lpm_sfunc(net);
	    return;

	  case IVL_LPM_UFUNC:
	    draw_lpm_ufunc(net);
	    return;

	  default:
	    fprintf(stderr, "XXXX LPM not supported: %s.%s\n",
		    ivl_scope_name(ivl_lpm_scope(net)), ivl_lpm_basename(net));
      }
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
      case IVL_SCT_GENERATE: type = "generate"; break;
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

      fprintf(vvp_out, " .timescale %d %d;\n", ivl_scope_time_units(net),
                                               ivl_scope_time_precision(net));

      for (idx = 0 ;  idx < ivl_scope_params(net) ;  idx += 1) {
	    ivl_parameter_t par = ivl_scope_param(net, idx);
	    ivl_expr_t pex = ivl_parameter_expr(par);
	    switch (ivl_expr_type(pex)) {
		case IVL_EX_STRING:
		  fprintf(vvp_out, "P_%p .param/str \"%s\", \"%s\";\n",
			  par, ivl_parameter_basename(par),
			  ivl_expr_string(pex));
		  break;
		case IVL_EX_NUMBER:
		  fprintf(vvp_out, "P_%p .param/l \"%s\", %sC4<",
			  par, ivl_parameter_basename(par),
			  ivl_expr_signed(pex)? "+":"");
		  { const char*bits = ivl_expr_bits(pex);
		    unsigned nbits = ivl_expr_width(pex);
		    unsigned bb;
		    for (bb = 0 ;  bb < nbits;  bb += 1)
			  fprintf(vvp_out, "%c", bits[nbits-bb-1]);
		  }
		  fprintf(vvp_out, ">;\n");
		  break;
		default:
		  fprintf(vvp_out, "; parameter type %d unsupported\n",
			  ivl_expr_type(pex));
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

      if (ivl_scope_type(net) == IVL_SCT_TASK)
	    draw_task_definition(net);

      if (ivl_scope_type(net) == IVL_SCT_FUNCTION)
	    draw_func_definition(net);

      ivl_scope_children(net, (ivl_scope_f*) draw_scope, net);
      return 0;
}

