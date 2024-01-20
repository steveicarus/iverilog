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
# include  <limits.h>
# include  <assert.h>
# include  "ivl_alloc.h"

static ivl_signal_type_t signal_type_of_nexus(ivl_nexus_t nex)
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
	    if (stype == IVL_SIT_UWIRE) return IVL_SIT_UWIRE;
	    out = stype;
      }

      return out;
}

static ivl_variable_type_t signal_data_type_of_nexus(ivl_nexus_t nex)
{
      unsigned idx;
      ivl_variable_type_t out = IVL_VT_NO_TYPE;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_variable_type_t vtype;
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0) continue;

	    vtype = ivl_signal_data_type(sig);
	    if (out == IVL_VT_NO_TYPE && vtype == IVL_VT_BOOL) {
		  out = vtype;
		  continue;
	    }
	    if (out != IVL_VT_LOGIC && vtype == IVL_VT_LOGIC) {
		  out = vtype;
		  continue;
	    }
	    if (vtype == IVL_VT_REAL) {
		  out = vtype;
		  break;
	    }
      }

      return out;
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
	    assert(dp >= result);
	    assert((unsigned)(dp - result) < result_len);
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
	    assert(dp >= result);
	    assert((unsigned)(dp - result) < nresult);
      }

      strcpy(dp, ">");
      return result;
}

static struct vvp_nexus_data*new_nexus_data(void)
{
      struct vvp_nexus_data*data = calloc(1, sizeof(struct vvp_nexus_data));
      return data;
}

static int nexus_drive_is_strength_aware(ivl_nexus_ptr_t nptr)
{
      ivl_net_logic_t logic;

      if (ivl_nexus_ptr_drive0(nptr) != IVL_DR_STRONG)
	    return 1;
      if (ivl_nexus_ptr_drive1(nptr) != IVL_DR_STRONG)
	    return 1;

      logic = ivl_nexus_ptr_log(nptr);
      if (logic != 0) {
	      /* These logic gates are able to generate unusual
	         strength values and so their outputs are considered
	         strength aware. */
	    if (ivl_logic_type(logic) == IVL_LO_BUFIF0)
		  return 1;
	    if (ivl_logic_type(logic) == IVL_LO_BUFIF1)
		  return 1;
	    if (ivl_logic_type(logic) == IVL_LO_PMOS)
		  return 1;
	    if (ivl_logic_type(logic) == IVL_LO_NMOS)
		  return 1;
	    if (ivl_logic_type(logic) == IVL_LO_CMOS)
		  return 1;
      }

      return 0;
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
 * This function draws a BUFT to drive a net pullup or pulldown value.
 * If the drive strength is strong we can draw a C4<> constant as the
 * pull value, otherwise we need to draw a C8<> constant.
 */
static char* draw_net_pull(ivl_net_logic_t lptr, ivl_drive_t drive, const char*level)
{
      char*result;
      char tmp[32];
      if (drive == IVL_DR_STRONG) {
            size_t result_len = 5 + ivl_logic_width(lptr);
            result = malloc(result_len);
            char*dp = result;
            strcpy(dp, "C4<");
            dp += strlen(dp);
            str_repeat(dp, level, ivl_logic_width(lptr));
            dp += ivl_logic_width(lptr);
            *dp++ = '>';
            *dp = 0;
            assert(dp >= result);
            assert((unsigned)(dp - result) <= result_len);
      } else {
            char val[4];
            size_t result_len = 5 + 3*ivl_logic_width(lptr);
            result = malloc(result_len);
            char*dp = result;

            val[0] = "01234567"[drive];
            val[1] = val[0];
            val[2] = level[0];
            val[3] = 0;

            strcpy(dp, "C8<");
            dp += strlen(dp);
            str_repeat(dp, val, ivl_logic_width(lptr));
            dp += 3*ivl_logic_width(lptr);
            *dp++ = '>';
            *dp = 0;
            assert(dp >= result);
            assert((unsigned)(dp - result) <= result_len);
      }

        /* Make the constant an argument to a BUFZ, which is
           what we use to drive the PULLed value. */
      fprintf(vvp_out, "L_%p .functor BUFT 1, %s, C4<0>, C4<0>, C4<0>;\n",
              lptr, result);
      snprintf(tmp, sizeof tmp, "L_%p", lptr);
      result = realloc(result, strlen(tmp)+1);
      strcpy(result, tmp);
      return result;
}

/*
 * This function takes a nexus and looks for an input functor. It then
 * draws to the output a string that represents that functor. What we
 * are trying to do here is find the input to the net that is attached
 * to this nexus.
 */

static char* draw_net_input_drive(const ivl_nexus_t nex, ivl_nexus_ptr_t nptr)
{
      unsigned nptr_pin = ivl_nexus_ptr_pin(nptr);
      ivl_net_const_t cptr;
      ivl_net_logic_t lptr;
      ivl_signal_t sptr;
      ivl_lpm_t lpm;

      lptr = ivl_nexus_ptr_log(nptr);
      if (lptr
	  && ((ivl_logic_type(lptr)==IVL_LO_BUFZ)||(ivl_logic_type(lptr)==IVL_LO_BUFT))
	  && (nptr_pin == 0))
	    do {
		  if (! can_elide_bufz(lptr, nptr))
			break;

		  return strdup(draw_net_input(ivl_logic_pin(lptr, 1)));
	    } while(0);

      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLDOWN)) {
	    return draw_net_pull(lptr, ivl_nexus_ptr_drive0(nptr), "0");
      }

      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLUP)) {
	    return draw_net_pull(lptr, ivl_nexus_ptr_drive1(nptr), "1");
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

	    if (ivl_signal_dimensions(sptr) > 0) {
		  fprintf(vvp_out, "v%p_%u .array/port v%p, %u;\n",
			  sptr, nptr_pin, sptr, nptr_pin);
	    }

	    return strdup(tmp);
      }

      cptr = ivl_nexus_ptr_con(nptr);
      if (cptr) {
	    char tmp[64];
	    char *result = 0;
	    ivl_expr_t d_rise, d_fall, d_decay;
            unsigned dly_width = 0;
	    char *dly;

	      /* Constants should have exactly 1 pin, with a literal value. */
	    assert(nptr_pin == 0);

	    switch (ivl_const_type(cptr)) {
		case IVL_VT_LOGIC:
		case IVL_VT_BOOL:
		case IVL_VT_STRING:
		  if ((ivl_nexus_ptr_drive0(nptr) == IVL_DR_STRONG)
		      && (ivl_nexus_ptr_drive1(nptr) == IVL_DR_STRONG)) {

			result = draw_C4_to_string(cptr);

		  } else {
			result = draw_C8_to_string(cptr,
						   ivl_nexus_ptr_drive0(nptr),
						   ivl_nexus_ptr_drive1(nptr));
		  }
                  dly_width = ivl_const_width(cptr);
		  break;

		case IVL_VT_REAL:
		  result = draw_Cr_to_string(ivl_const_real(cptr));
                  dly_width = 0;
		  break;

		default:
		  assert(0);
		  break;
	    }

	    d_rise = ivl_const_delay(cptr, 0);
	    d_fall = ivl_const_delay(cptr, 1);
	    d_decay = ivl_const_delay(cptr, 2);

	    dly = "";
	    if (d_rise != 0) {
		  draw_delay(cptr, dly_width, 0, d_rise, d_fall, d_decay);
		  dly = "/d";
	    }
	    fprintf(vvp_out, "L_%p%s .functor BUFT 1, %s, C4<0>, C4<0>, C4<0>;\n",
		    cptr, dly, result);
	    free(result);

	    snprintf(tmp, sizeof tmp, "L_%p", cptr);
	    return strdup(tmp);
      }

      lpm = ivl_nexus_ptr_lpm(nptr);
      if (lpm) switch (ivl_lpm_type(lpm)) {

	  case IVL_LPM_FF:
	  case IVL_LPM_LATCH:
	  case IVL_LPM_ABS:
	  case IVL_LPM_ADD:
	  case IVL_LPM_ARRAY:
	  case IVL_LPM_CAST_INT2:
	  case IVL_LPM_CAST_INT:
	  case IVL_LPM_CAST_REAL:
	  case IVL_LPM_CONCAT:
	  case IVL_LPM_CONCATZ:
	  case IVL_LPM_CMP_EEQ:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_WEQ:
	  case IVL_LPM_CMP_WNE:
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
	  case IVL_LPM_SFUNC:
	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	  case IVL_LPM_SIGN_EXT:
	  case IVL_LPM_SUB:
	  case IVL_LPM_MULT:
	  case IVL_LPM_MUX:
	  case IVL_LPM_POW:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_UFUNC:
	  case IVL_LPM_PART_VP:
	  case IVL_LPM_PART_PV: /* NOTE: This is only a partial driver. */
	  case IVL_LPM_REPEAT:
	  case IVL_LPM_SUBSTITUTE:
	    if (ivl_lpm_q(lpm) == nex) {
		  char tmp[128];
		  snprintf(tmp, sizeof tmp, "L_%p", lpm);
		  return strdup(tmp);
	    }
	    break;

      }

      fprintf(stderr, "vvp.tgt error: no input to nexus.\n");
      assert(0);
      return strdup("C<z>");
}

static char* draw_island_port(ivl_island_t island, int island_input_flag,
			      ivl_nexus_t nex, struct vvp_nexus_data*nex_data,
			      const char*src)
{
      char result[64];
      if (ivl_island_flag_test(island,0) == 0) {
	    fprintf(vvp_out, "I%p .island tran;\n", island);
	    ivl_island_flag_set(island,0,1);
      }

      snprintf(result, sizeof result, "p%p", nex);
      assert(nex_data->island == 0);
      nex_data->island = island;
      assert(nex_data->island_input == 0);
      nex_data->island_input = strdup(result);

      if (island_input_flag) {
	    fprintf(vvp_out, "p%p .import I%p, %s;\n", nex, island, src);
	    return strdup(src);
      } else {
	    fprintf(vvp_out, "p%p .port I%p, %s;\n", nex, island, src);
	    return strdup(nex_data->island_input);
      }
}

/*
 * This routine is called to display an error message when a uwire or
 * wire real has multiple drivers.
 */
typedef enum mdriver_type_e {
      MDRV_UWIRE = 0,
      MDRV_REAL  = 1
} mdriver_type_t;

static void display_multi_driver_error(ivl_nexus_t nex, unsigned ndrivers,
                                       mdriver_type_t type)
{
      unsigned idx;
      unsigned scope_len = UINT_MAX;
      ivl_signal_t sig = 0;
	/* Find the signal. */
      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t tsig = ivl_nexus_ptr_sig(ptr);
	    if (tsig != 0) {
		  ivl_scope_t scope;
		  unsigned len;
		  if (ivl_signal_local(tsig)) continue;

		    /* If this is not a local signal then find the signal
		     * that has the shortest scope (is the furthest up
		     * the hierarchy). */
		  scope = ivl_signal_scope(tsig);
		  assert(scope);
		  len = strlen(ivl_scope_name(scope));
		  if (len < scope_len) {
			scope_len = len;
			sig = tsig;
		  }
	    }
      }
      assert(sig);

      fprintf(stderr, "%s:%u: vvp.tgt error: ",
                      ivl_signal_file(sig), ivl_signal_lineno(sig));
      switch (type) {
	  case MDRV_UWIRE:
	    if (ivl_signal_type(sig) != IVL_SIT_UWIRE) {
		  fprintf(stderr, "(implicit) ");
	    }
	    fprintf(stderr, "uwire");
	    break;

	  case MDRV_REAL:
	    assert(ivl_signal_type(sig) == IVL_SIT_TRI);
	    if (ivl_signal_data_type(sig) != IVL_VT_REAL) {
		  fprintf(stderr, "(implicit) ");
	    }
	    fprintf(stderr, "wire real");
	    break;

	  default:
	    assert(0);;
      }
      fprintf(stderr, " \"%s\" must have a single driver, found (%u).\n",
                      ivl_signal_basename(sig), ndrivers);
      vvp_errors += 1;
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

static ivl_nexus_ptr_t *drivers = 0x0;
static unsigned adrivers = 0;

void EOC_cleanup_drivers(void)
{
      free(drivers);
      drivers = NULL;
      adrivers = 0;
}

static void draw_net_input_x(ivl_nexus_t nex,
			     struct vvp_nexus_data*nex_data)
{
      ivl_island_t island = 0;
      int island_input_flag = -1;
      ivl_signal_type_t res;
      char result[512];
      unsigned idx;
      char**driver_labels;
      unsigned ndrivers = 0;

      const char*resolv_type;

      char*nex_private = 0;

	/* Accumulate nex_data flags. */
      int nex_flags = 0;

      res = signal_type_of_nexus(nex);
      switch (res) {
	  case IVL_SIT_TRI:
	  case IVL_SIT_UWIRE:
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
	    fprintf(stderr, "vvp.tgt: Unsupported signal type: %d\n", res);
	    assert(0);
	    resolv_type = "tri";
	    break;
      }


      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_switch_t sw = 0;
	    ivl_nexus_ptr_t nptr = ivl_nexus_ptr(nex, idx);

	      /* If this object is part of an island, then we'll be
	         making a port. If this nexus is an output from any
	         switches in the island, then set island_input_flag to
	         false. Save the island cookie. */
	    if ( (sw = ivl_nexus_ptr_switch(nptr)) ) {
		  assert(island == 0 || island == ivl_switch_island(sw));
		  island = ivl_switch_island(sw);
		  if (nex == ivl_switch_a(sw)) {
			nex_flags |= VVP_NEXUS_DATA_STR;
			island_input_flag = 0;
		  } else if (nex == ivl_switch_b(sw)) {
			nex_flags |= VVP_NEXUS_DATA_STR;
			island_input_flag = 0;
		  } else if (island_input_flag == -1) {
			assert(nex == ivl_switch_enable(sw));
			island_input_flag = 1;
		  }
	    }

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
	    }
	    drivers[ndrivers] = nptr;
	    ndrivers += 1;
      }

      if (island_input_flag < 0)
	    island_input_flag = 0;

	/* Save the nexus driver count in the nex_data. */
      assert(nex_data);
      nex_data->drivers_count = ndrivers;
      nex_data->flags |= nex_flags;

	/* If the nexus has no drivers, then send a constant HiZ or
	   0.0 into the net. Use a lower case 'c' prefix for the
	   constant to inform vvp that this is an undriven value. */
      if (ndrivers == 0) {
	    unsigned wid = width_of_nexus(nex);
	    int pull = (res == IVL_SIT_TRI0) || (res == IVL_SIT_TRI1);
	      /* For real nets put 0.0. */
	    if (signal_data_type_of_nexus(nex) == IVL_VT_REAL) {
		  nex_private = draw_Cr_to_string(0.0);
		  nex_private[0] = 'c';
	    } else {
		  unsigned jdx;
		  char*tmp = malloc((pull ? 3 : 1) * wid + 5);
		  nex_private = tmp;
		  strcpy(tmp, pull ? "c8<" : "c4<");
		  tmp += strlen(tmp);
		  switch (res) {
		      case IVL_SIT_TRI:
		      case IVL_SIT_TRIAND:
		      case IVL_SIT_TRIOR:
		      case IVL_SIT_UWIRE:
			for (jdx = 0 ;  jdx < wid ;  jdx += 1)
			      *tmp++ = 'z';
			break;
		      case IVL_SIT_TRI0:
			for (jdx = 0 ;  jdx < wid ;  jdx += 1) {
			      *tmp++ = '5';
			      *tmp++ = '5';
			      *tmp++ = '0';
			}
			break;
		      case IVL_SIT_TRI1:
			for (jdx = 0 ;  jdx < wid ;  jdx += 1) {
			      *tmp++ = '5';
			      *tmp++ = '5';
			      *tmp++ = '1';
			}
			break;
		      default:
			assert(0);
		  }
		  *tmp++ = '>';
		  *tmp = 0;
	    }

	      /* Create an "open" driver to hold the HiZ or 0.0. We need
	         to do this so that .nets have something to hang onto. */
	    char buf[64];
	    snprintf(buf, sizeof buf, "o%p", nex);
	    if (pull) {
		  fprintf(vvp_out, "%s .functor BUFT %u, %s, C4<0>, C4<0>, C4<0>; pull drive\n",
			  buf, wid, nex_private);
	    } else {
		  fprintf(vvp_out, "%s .functor BUFZ %u, %s; HiZ drive\n",
			  buf, wid, nex_private);
	    }
	    nex_private = realloc(nex_private, strlen(buf)+1);
	    strcpy(nex_private, buf);

	    if (island) {
		  char*tmp2 = draw_island_port(island, island_input_flag, nex, nex_data, nex_private);
		  free(nex_private);
		  nex_private = tmp2;
	    }
	    assert(nex_data->net_input == 0);
	    nex_data->net_input = nex_private;
	    return;
      }

	/* A uwire is a tri with only one driver. */
      if (res == IVL_SIT_UWIRE) {
	    if (ndrivers > 1) {
		  display_multi_driver_error(nex, ndrivers, MDRV_UWIRE);
	    }
	    res = IVL_SIT_TRI;
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
			   "V_%p_0/m", path_sig);
		  nex_private = strdup(modpath_label);
		  draw_modpath(path_sig, nex_str, 0);

	    } else {
		  nex_private = draw_net_input_drive(nex, drivers[0]);
	    }
	    if (island) {
		  char*tmp = draw_island_port(island, island_input_flag, nex, nex_data, nex_private);
		  free(nex_private);
		  nex_private = tmp;
	    }
	    assert(nex_data->net_input == 0);
	    nex_data->net_input = nex_private;
	    return;
      }

	/* We currently only support one driver on real nets. */
      if (ndrivers > 1 && signal_data_type_of_nexus(nex) == IVL_VT_REAL) {
	    display_multi_driver_error(nex, ndrivers, MDRV_REAL);
      }

      driver_labels = malloc(ndrivers * sizeof(char*));
      ivl_signal_t path_sig = find_modpath(nex);
      if (path_sig) {
	    for (idx = 0; idx < ndrivers; idx += 1) {
		  char*nex_str = draw_net_input_drive(nex, drivers[idx]);
		  char modpath_label[64];
		  snprintf(modpath_label, sizeof modpath_label,
			   "V_%p_%u/m", path_sig, idx);
		  driver_labels[idx] = strdup(modpath_label);
		  draw_modpath(path_sig, nex_str, idx);
	    }
      } else {
	    for (idx = 0; idx < ndrivers; idx += 1) {
		  driver_labels[idx] = draw_net_input_drive(nex, drivers[idx]);
	    }
      }
      fprintf(vvp_out, "RS_%p .resolv %s", nex, resolv_type);
      for (idx = 0; idx < ndrivers; idx += 1) {
            fprintf(vvp_out, ", %s", driver_labels[idx]);
	    free(driver_labels[idx]);
      }
      fprintf(vvp_out, ";\n");
      free(driver_labels);

      snprintf(result, sizeof result, "RS_%p", nex);

      if (island)
	    nex_private = draw_island_port(island, island_input_flag, nex, nex_data, result);
      else
	    nex_private = strdup(result);

      assert(nex_data->net_input == 0);
      nex_data->net_input = nex_private;
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
      draw_net_input_x(nex, nex_data);

      return nex_data->net_input;
}

const char*draw_island_net_input(const ivl_island_t island, ivl_nexus_t nex)
{
      struct vvp_nexus_data*nex_data = (struct vvp_nexus_data*)
	    ivl_nexus_get_private(nex);

	/* If this nexus already has a label, then its input is
	   already figured out. Just return the existing label. */
      if (nex_data && nex_data->island_input) {
	    assert(nex_data->island == island);
	    return nex_data->island_input;
      }

      if (nex_data == 0) {
	    nex_data = new_nexus_data();
	    ivl_nexus_set_private(nex, nex_data);
      }

      assert(nex_data->net_input == 0);
      draw_net_input_x(nex, nex_data);

      assert(nex_data->island == island);
      assert(nex_data->island_input);

      return nex_data->island_input;
}
