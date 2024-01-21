/*
 * Copyright (c) 2000-2024 Stephen Williams (steve@icarus.com)
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

/*
 * This is a sample target module. All this does is write to the
 * output file some information about each object handle when each of
 * the various object functions is called. This can be used to
 * understand the behavior of the core as it uses a target module.
 */

# include "version_base.h"
# include "version_tag.h"
# include "config.h"
# include "priv.h"
# include <stdlib.h>
# include <inttypes.h>
# include <string.h>
# include <assert.h>
# include "ivl_alloc.h"

static const char*version_string =
"Icarus Verilog STUB Code Generator " VERSION " (" VERSION_TAG ")\n\n"
"Copyright (c) 2000-2024 Stephen Williams (steve@icarus.com)\n\n"
"  This program is free software; you can redistribute it and/or modify\n"
"  it under the terms of the GNU General Public License as published by\n"
"  the Free Software Foundation; either version 2 of the License, or\n"
"  (at your option) any later version.\n"
"\n"
"  This program is distributed in the hope that it will be useful,\n"
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"  GNU General Public License for more details.\n"
"\n"
"  You should have received a copy of the GNU General Public License along\n"
"  with this program; if not, write to the Free Software Foundation, Inc.,\n"
"  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n"
;

FILE*out;
int stub_errors = 0;

static struct udp_define_cell {
      ivl_udp_t udp;
      unsigned ref;
      struct udp_define_cell*next;
}*udp_define_list = 0;

static void reference_udp_definition(ivl_udp_t udp)
{
      struct udp_define_cell*cur;

      if (udp_define_list == 0) {
	    udp_define_list = calloc(1, sizeof(struct udp_define_cell));
	    udp_define_list->udp = udp;
	    udp_define_list->ref = 1;
	    return;
      }

      cur = udp_define_list;
      while (cur->udp != udp) {
	    if (cur->next == 0) {
		  cur->next = calloc(1, sizeof(struct udp_define_cell));
		  cur->next->udp = udp;
		  cur->next->ref = 1;
		  return;
	    }

	    cur = cur->next;
      }

      cur->ref += 1;
}

/*
 * This function finds the vector width of a signal. It relies on the
 * assumption that all the signal inputs to the nexus have the same
 * width. The ivl_target API should assert that condition.
 */
unsigned width_of_nexus(ivl_nexus_t nex)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);

	    if (sig != 0) {
		  return ivl_signal_width(sig);
	    }
      }

	/* ERROR: A nexus should have at least one signal to carry
	   properties like width. */
      return 0;
}

ivl_discipline_t discipline_of_nexus(ivl_nexus_t nex)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex); idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);

	    if (sig != 0) {
		  return ivl_signal_discipline(sig);
	    }
      }

	/* ERROR: A nexus should have at least one signal to carry
	   properties like the data type. */
      return 0;
}

ivl_variable_type_t type_of_nexus(ivl_nexus_t net)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(net); idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(net, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);

	    if (sig != 0) {
		  return ivl_signal_data_type(sig);
	    }
      }

	/* ERROR: A nexus should have at least one signal to carry
	   properties like the data type. */
      return IVL_VT_NO_TYPE;
}

const char*data_type_string(ivl_variable_type_t vtype)
{
      const char*vt = "??";

      switch (vtype) {
	  case IVL_VT_NO_TYPE:
	    vt = "NO_TYPE";
	    break;
	  case IVL_VT_VOID:
	    vt = "void";
	    break;
	  case IVL_VT_BOOL:
	    vt = "bool";
	    break;
	  case IVL_VT_REAL:
	    vt = "real";
	    break;
	  case IVL_VT_LOGIC:
	    vt = "logic";
	    break;
	  case IVL_VT_STRING:
	    vt = "string";
	    break;
	  case IVL_VT_DARRAY:
	    vt = "darray";
	    break;
	  case IVL_VT_CLASS:
	    vt = "class";
	    break;
	  case IVL_VT_QUEUE:
	    vt = "queue";
	    break;
      }

      return vt;
}

/*
 * The compiler will check the types of drivers to signals and will
 * only connect outputs to signals that are compatible. This function
 * shows the type compatibility that the compiler enforces at the
 * ivl_target.h level.
 */
static int check_signal_drive_type(ivl_variable_type_t sig_type,
				    ivl_variable_type_t driver_type)
{
      if (sig_type == IVL_VT_LOGIC && driver_type == IVL_VT_BOOL)
	    return !0;
      if (sig_type == IVL_VT_LOGIC && driver_type == IVL_VT_LOGIC)
	    return !0;
      if (sig_type == IVL_VT_BOOL && driver_type == IVL_VT_BOOL)
	    return !0;
      if (sig_type == driver_type)
	    return !0;
      return 0;
}

/*
 * The compare-like LPM nodes have input widths that match the
 * ivl_lpm_width() value, and an output width of 1. This function
 * checks that that is so, and indicates errors otherwise.
 */
static void check_cmp_widths(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

	/* Check that the input widths are as expected. The inputs
	   must be the width of the ivl_lpm_width() for this device,
	   even though the output for this device is 1 bit. */
      if (width != width_of_nexus(ivl_lpm_data(net,0))) {
	    fprintf(out, "    ERROR: Width of A is %u, not %u\n",
		    width_of_nexus(ivl_lpm_data(net,0)), width);
	    stub_errors += 1;
      }

      if (width != width_of_nexus(ivl_lpm_data(net,1))) {
	    fprintf(out, "    ERROR: Width of B is %u, not %u\n",
		    width_of_nexus(ivl_lpm_data(net,1)), width);
	    stub_errors += 1;
      }

      if (width_of_nexus(ivl_lpm_q(net)) != 1) {
	    fprintf(out, "    ERROR: Width of Q is %u, not 1\n",
		    width_of_nexus(ivl_lpm_q(net)));
	    stub_errors += 1;
      }
}

static void show_lpm_arithmetic_pins(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      nex = ivl_lpm_q(net);
      fprintf(out, "    Q: %p\n", nex);

      nex = ivl_lpm_data(net, 0);
      fprintf(out, "    DataA: %p\n", nex);

      nex = ivl_lpm_data(net, 1);
      fprintf(out, "    DataB: %p\n", nex);
}

static void show_lpm_abs(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      ivl_nexus_t nex;

      fprintf(out, "  LPM_ABS %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      nex = ivl_lpm_q(net);
      fprintf(out, "    Q: %p\n", nex);

      nex = ivl_lpm_data(net, 0);
      fprintf(out, "    D: %p\n", nex);
      if (nex == 0) {
	    fprintf(out, "    ERROR: missing input\n");
	    stub_errors += 1;
	    return;
      }

      if (width_of_nexus(nex) != width) {
	    fprintf(out, "    ERROR: D width (%u) is wrong\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }
}

static void show_lpm_add(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_ADD %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      show_lpm_arithmetic_pins(net);
}

static void show_lpm_array(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      unsigned width = ivl_lpm_width(net);
      ivl_signal_t array = ivl_lpm_array(net);

      fprintf(out, "  LPM_ARRAY: <width=%u, signal=%s>\n",
	      width, ivl_signal_basename(array));
      nex = ivl_lpm_q(net);
      assert(nex);
      fprintf(out, "    Q: %p\n", nex);
      nex = ivl_lpm_select(net);
      assert(nex);
      fprintf(out, "    Address: %p (address width=%u)\n",
	      nex, ivl_lpm_selects(net));

      if (width_of_nexus(ivl_lpm_q(net)) != width) {
	    fprintf(out, "    ERROR: Data Q width doesn't match "
		    "nexus width=%u\n", width_of_nexus(ivl_lpm_q(net)));
	    stub_errors += 1;
      }

      if (ivl_signal_width(array) != width) {
	    fprintf(out, "    ERROR: Data  width doesn't match "
		    "word width=%u\n", ivl_signal_width(array));
	    stub_errors += 1;
      }
}

static void show_lpm_cast_int(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      ivl_nexus_t q, a;

      fprintf(out, "  LPM_CAST_INT %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      q = ivl_lpm_q(net);
      a = ivl_lpm_data(net,0);
      fprintf(out, "    O: %p\n", ivl_lpm_q(net));
      fprintf(out, "    A: %p\n", ivl_lpm_data(net,0));

      if (type_of_nexus(q) == IVL_VT_REAL) {
	    fprintf(out, "    ERROR: Data type of Q is %s, expecting !real\n",
		    data_type_string(type_of_nexus(q)));
	    stub_errors += 1;
      }

      if (type_of_nexus(a) != IVL_VT_REAL) {
	    fprintf(out, "    ERROR: Data type of A is %s, expecting real\n",
		    data_type_string(type_of_nexus(a)));
	    stub_errors += 1;
      }
}

static void show_lpm_cast_real(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      ivl_nexus_t q, a;

      fprintf(out, "  LPM_CAST_REAL %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      q = ivl_lpm_q(net);
      a = ivl_lpm_data(net,0);
      fprintf(out, "    O: %p\n", ivl_lpm_q(net));
      fprintf(out, "    A: %p\n", ivl_lpm_data(net,0));

      if (type_of_nexus(q) != IVL_VT_REAL) {
	    fprintf(out, "    ERROR: Data type of Q is %s, expecting real\n",
		    data_type_string(type_of_nexus(q)));
	    stub_errors += 1;
      }

      if (type_of_nexus(a) == IVL_VT_REAL) {
	    fprintf(out, "    ERROR: Data type of A is %s, expecting !real\n",
		    data_type_string(type_of_nexus(a)));
	    stub_errors += 1;
      }
}

static void show_lpm_divide(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_DIVIDE %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      show_lpm_arithmetic_pins(net);
}

/* IVL_LPM_CMP_EEQ/EQX/EQZ/NEE
 * This LPM node supports two-input compare. The output width is
 * actually always 1, the lpm_width is the expected width of the inputs.
 */
static void show_lpm_cmp_eeq(ivl_lpm_t net)
{
      const char*str = 0;
      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_CMP_EEQ:
	    str = "EEQ";
	    break;
	  case IVL_LPM_CMP_EQX:
	    str = "EQX";
	    break;
	  case IVL_LPM_CMP_EQZ:
	    str = "EQZ";
	    break;
	  case IVL_LPM_CMP_NEE:
	    str = "NEE";
	    break;
	  case IVL_LPM_CMP_WEQ:
	    str = "WEQ";
	    break;
	  case IVL_LPM_CMP_WNE:
	    str = "WNE";
	    break;
	  default:
	    assert(0);
	    break;
      }

      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_CMP_%s %s: <width=%u>\n", str,
	      ivl_lpm_basename(net), width);

      fprintf(out, "    O: %p\n", ivl_lpm_q(net));
      fprintf(out, "    A: %p\n", ivl_lpm_data(net,0));
      fprintf(out, "    B: %p\n", ivl_lpm_data(net,1));
      check_cmp_widths(net);
}

/* IVL_LPM_CMP_GE
 * This LPM node supports two-input compare.
 */
static void show_lpm_cmp_ge(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_CMP_GE %s: <width=%u %s>\n",
	      ivl_lpm_basename(net), width,
	      ivl_lpm_signed(net)? "signed" : "unsigned");

      fprintf(out, "    O: %p\n", ivl_lpm_q(net));
      fprintf(out, "    A: %p\n", ivl_lpm_data(net,0));
      fprintf(out, "    B: %p\n", ivl_lpm_data(net,1));
      check_cmp_widths(net);
}

/* IVL_LPM_CMP_GT
 * This LPM node supports two-input compare.
 */
static void show_lpm_cmp_gt(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_CMP_GT %s: <width=%u %s>\n",
	      ivl_lpm_basename(net), width,
	      ivl_lpm_signed(net)? "signed" : "unsigned");

      fprintf(out, "    O: %p\n", ivl_lpm_q(net));
      fprintf(out, "    A: %p\n", ivl_lpm_data(net,0));
      fprintf(out, "    B: %p\n", ivl_lpm_data(net,1));
      check_cmp_widths(net);
}

/* IVL_LPM_CMP_NE
 * This LPM node supports two-input compare. The output width is
 * actually always 1, the lpm_width is the expected width of the inputs.
 */
static void show_lpm_cmp_ne(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_CMP_NE %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      fprintf(out, "    O: %p\n", ivl_lpm_q(net));
      fprintf(out, "    A: %p\n", ivl_lpm_data(net,0));
      fprintf(out, "    B: %p\n", ivl_lpm_data(net,1));
      check_cmp_widths(net);
}

/* IVL_LPM_CONCAT, IVL_LPM_CONCATZ
 * The concat device takes N inputs (N=ivl_lpm_size) and generates
 * a single output. The total output is known from the ivl_lpm_width
 * function. The widths of all the inputs are inferred from the widths
 * of the signals connected to the nexus of the inputs. The compiler
 * makes sure the input widths add up to the output width.
 */
static void show_lpm_concat(ivl_lpm_t net)
{
      unsigned idx;

      unsigned width_sum = 0;
      unsigned width = ivl_lpm_width(net);
      const char*z = ivl_lpm_type(net)==IVL_LPM_CONCATZ? "Z" : "";

      fprintf(out, "  LPM_CONCAT%s %s: <width=%u, inputs=%u>\n",
	      z, ivl_lpm_basename(net), width, ivl_lpm_size(net));
      fprintf(out, "    O: %p\n", ivl_lpm_q(net));

      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_lpm_data(net, idx);
	    unsigned signal_width = width_of_nexus(nex);

	    fprintf(out, "    I%u: %p (width=%u)\n", idx, nex, signal_width);
	    width_sum += signal_width;
      }

      if (width_sum != width) {
	    fprintf(out, "    ERROR! Got %u bits input, expecting %u!\n",
		    width_sum, width);
      }
}

static void show_lpm_ff(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      char*edge = ivl_lpm_negedge(net) ? "negedge" : "posedge";
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_FF %s: <polarity=%s> <width=%u>\n",
	      ivl_lpm_basename(net), edge, width);

      nex = ivl_lpm_clk(net);
      fprintf(out, "    clk: %p\n", nex);
      if (width_of_nexus(nex) != 1) {
	    fprintf(out, "    clk: ERROR: Nexus width is %u\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }

      if (ivl_lpm_enable(net)) {
	    nex = ivl_lpm_enable(net);
	    fprintf(out, "    CE: %p\n", nex);
	    if (width_of_nexus(nex) != 1) {
		  fprintf(out, "    CE: ERROR: Nexus width is %u\n",
			  width_of_nexus(nex));
		  stub_errors += 1;
	    }
      }

      if (ivl_lpm_async_clr(net)) {
	    nex = ivl_lpm_async_clr(net);
	    fprintf(out, "   Aclr: %p\n", nex);
	    if (width_of_nexus(nex) != 1) {
		  fprintf(out, "  Aclr: ERROR: Nexus width is %u\n",
			  width_of_nexus(nex));
		  stub_errors += 1;
	    }
      }

      if (ivl_lpm_async_set(net)) {
	    nex = ivl_lpm_async_set(net);
	    fprintf(out, "   Aset: %p\n", nex);
	    if (width_of_nexus(nex) != 1) {
		  fprintf(out, "  Aset: ERROR: Nexus width is %u\n",
			  width_of_nexus(nex));
		  stub_errors += 1;
	    }
      }

      nex = ivl_lpm_data(net,0);
      fprintf(out, "      D: %p\n", nex);
      if (width_of_nexus(nex) != width) {
	    fprintf(out, "    D: ERROR: Nexus width is %u\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }

      nex = ivl_lpm_q(net);
      fprintf(out, "      Q: %p\n", nex);
      if (width_of_nexus(nex) != width) {
	    fprintf(out, "    Q: ERROR: Nexus width is %u\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }

}

static void show_lpm_latch(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_LATCH %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      nex = ivl_lpm_enable(net);
      fprintf(out, "      E: %p\n", nex);
      if (width_of_nexus(nex) != 1) {
	    fprintf(out, "    E: ERROR: Nexus width is %u\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }

      nex = ivl_lpm_data(net,0);
      fprintf(out, "      D: %p\n", nex);
      if (width_of_nexus(nex) != width) {
	    fprintf(out, "    D: ERROR: Nexus width is %u\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }

      nex = ivl_lpm_q(net);
      fprintf(out, "      Q: %p\n", nex);
      if (width_of_nexus(nex) != width) {
	    fprintf(out, "    Q: ERROR: Nexus width is %u\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }
}

static void show_lpm_mod(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_MOD %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      show_lpm_arithmetic_pins(net);
}

/*
 * The LPM_MULT node has a Q output and two data inputs. The width of
 * the Q output must be the width of the node itself.
 */
static void show_lpm_mult(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_MULT %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      fprintf(out, "    O: %p\n", ivl_lpm_q(net));
      fprintf(out, "    A: %p <width=%u>\n",
	      ivl_lpm_data(net,0), width_of_nexus(ivl_lpm_data(net,0)));
      fprintf(out, "    B: %p <width=%u>\n",
	      ivl_lpm_data(net,1), width_of_nexus(ivl_lpm_data(net,1)));

      if (width != width_of_nexus(ivl_lpm_q(net))) {
	    fprintf(out, "    ERROR: Width of Q is %u, not %u\n",
		    width_of_nexus(ivl_lpm_q(net)), width);
	    stub_errors += 1;
      }
}

/*
 * Show an IVL_LPM_MUX.
 *
 * The compiler is supposed to make sure that the Q output and data
 * inputs all have the width of the device. The ivl_lpm_select input
 * has its own width.
 */
static void show_lpm_mux(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      unsigned idx;
      unsigned width = ivl_lpm_width(net);
      unsigned size  = ivl_lpm_size(net);
      ivl_drive_t drive0 = ivl_lpm_drive0(net);
      ivl_drive_t drive1 = ivl_lpm_drive1(net);

      fprintf(out, "  LPM_MUX %s: <width=%u, size=%u>\n",
	      ivl_lpm_basename(net), width, size);

      nex = ivl_lpm_q(net);
      fprintf(out, "    Q: %p <drive0/1 = %d/%d>\n", nex, drive0, drive1);
      if (width != width_of_nexus(nex)) {
	    fprintf(out, "    Q: ERROR: Nexus width is %u\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }

	/* The select input is a vector with the width from the
	   ivl_lpm_selects function. */
      nex = ivl_lpm_select(net);
      fprintf(out, "    S: %p <width=%u>\n",
	      nex, ivl_lpm_selects(net));
      if (ivl_lpm_selects(net) != width_of_nexus(nex)) {
	    fprintf(out, "    S: ERROR: Nexus width is %u\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }

	/* The ivl_lpm_size() method give the number of inputs that
	   can be selected from. */
      for (idx = 0 ;  idx < size ;  idx += 1) {
	    nex = ivl_lpm_data(net,idx);
	    fprintf(out, "    D%u: %p\n", idx, nex);
	    if (width != width_of_nexus(nex)) {
		  fprintf(out, "    D%u: ERROR, Nexus width is %u\n",
			  idx, width_of_nexus(nex));
		  stub_errors += 1;
	    }
      }
}

static void show_lpm_part(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned base  = ivl_lpm_base(net);
      ivl_nexus_t sel = ivl_lpm_data(net,1);
      const char*part_type_string = "";

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_PART_VP:
	    part_type_string = "VP";
	    break;
	  case IVL_LPM_PART_PV:
	    part_type_string = "PV";
	    break;
	  default:
	    break;
      }

      fprintf(out, "  LPM_PART_%s %s: <width=%u, base=%u, signed=%d>\n",
	      part_type_string, ivl_lpm_basename(net),
	      width, base, ivl_lpm_signed(net));
      fprintf(out, "    O: %p\n", ivl_lpm_q(net));
      fprintf(out, "    I: %p\n", ivl_lpm_data(net,0));

      if (sel != 0) {
	    fprintf(out, "    S: %p\n", sel);
	    if (base != 0) {
		  fprintf(out, "   ERROR: Part select has base AND selector\n");
		  stub_errors += 1;
	    }
      }

	/* The compiler must assure that the base plus the part select
	   width fits within the input to the part select. */
      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_PART_VP:
	    if (width_of_nexus(ivl_lpm_data(net,0)) < (width+base)) {
		  fprintf(out, "    ERROR: Part select is out of range."
			  " Data nexus width=%u, width+base=%u\n",
			  width_of_nexus(ivl_lpm_data(net,0)), width+base);
		  stub_errors += 1;
	    }

	    if (width_of_nexus(ivl_lpm_q(net)) != width) {
		  fprintf(out, "    ERROR: Part select input mismatch."
			  " Nexus width=%u, expect width=%u\n",
			  width_of_nexus(ivl_lpm_q(net)), width);
		  stub_errors += 1;
	    }
	    break;

	  case IVL_LPM_PART_PV:
	    if (width_of_nexus(ivl_lpm_q(net)) < (width+base)) {
		  fprintf(out, "    ERROR: Part select is out of range."
			  " Target nexus width=%u, width+base=%u\n",
			  width_of_nexus(ivl_lpm_q(net)), width+base);
		  stub_errors += 1;
	    }

	    if (width_of_nexus(ivl_lpm_data(net,0)) != width) {
		  fprintf(out, "    ERROR: Part select input mismatch."
			  " Nexus width=%u, expect width=%u\n",
			  width_of_nexus(ivl_lpm_data(net,0)), width);
		  stub_errors += 1;
	    }
	    break;

	  default:
	    assert(0);
      }
}

/*
 * The reduction operators have similar characteristics and are
 * displayed here.
 */
static void show_lpm_re(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      const char*type = "?";
      unsigned width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_RE_AND:
	    type = "AND";
	    break;
	  case IVL_LPM_RE_NAND:
	    type = "NAND";
	    break;
	  case IVL_LPM_RE_OR:
	    type = "OR";
	    break;
	  case IVL_LPM_RE_NOR:
	    type = "NOR";
	    break;
	  case IVL_LPM_RE_XOR:
	    type = "XOR";
	    break;
	  case IVL_LPM_RE_XNOR:
	    type = "XNOR";
	  default:
	    break;
      }

      fprintf(out, "  LPM_RE_%s: %s <width=%u>\n",
	      type, ivl_lpm_name(net),width);

      nex = ivl_lpm_q(net);
      fprintf(out, "    Q: %p\n", nex);

      nex = ivl_lpm_data(net, 0);
      fprintf(out, "    D: %p\n", nex);

      nex = ivl_lpm_q(net);

      if (1 != width_of_nexus(nex)) {
	    fprintf(out, "    ERROR: Width of Q is %u, expecting 1\n",
		    width_of_nexus(nex));
	    stub_errors += 1;
      }

      nex = ivl_lpm_data(net, 0);
      if (width != width_of_nexus(nex)) {
	    fprintf(out, "    ERROR: Width of input is %u, expecting %u\n",
		    width_of_nexus(nex), width);
	    stub_errors += 1;
      }
}

static void show_lpm_repeat(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned count = ivl_lpm_size(net);
      ivl_nexus_t nex_q = ivl_lpm_q(net);
      ivl_nexus_t nex_a = ivl_lpm_data(net,0);

      fprintf(out, "  LPM_REPEAT %s: <width=%u, count=%u>\n",
	      ivl_lpm_basename(net), width, count);

      fprintf(out, "    Q: %p\n", nex_q);
      fprintf(out, "    D: %p\n", nex_a);

      if (width != width_of_nexus(nex_q)) {
	    fprintf(out, "    ERROR: Width of Q is %u, expecting %u\n",
		    width_of_nexus(nex_q), width);
	    stub_errors += 1;
      }

      if (count == 0 || count > width || (width%count != 0)) {
	    fprintf(out, "    ERROR: Repeat count not reasonable\n");
	    stub_errors += 1;

      } else if (width/count != width_of_nexus(nex_a)) {
	    fprintf(out, "    ERROR: Width of D is %u, expecting %u\n",
		    width_of_nexus(nex_a), width/count);
	    stub_errors += 1;
      }
}

static void show_lpm_shift(ivl_lpm_t net, const char*shift_dir)
{
      ivl_nexus_t nex;
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_SHIFT%s %s: <width=%u, %ssigned>\n", shift_dir,
	      ivl_lpm_basename(net), width,
	      ivl_lpm_signed(net)? "" : "un");

      nex = ivl_lpm_q(net);
      fprintf(out, "    Q: %p\n", nex);

      if (width != width_of_nexus(nex)) {
	    fprintf(out, "    ERROR: Q output nexus width=%u "
		    "does not match part width\n", width_of_nexus(nex));
	    stub_errors += 1;
      }

      nex = ivl_lpm_data(net, 0);
      fprintf(out, "    D: %p\n", nex);

      if (width != width_of_nexus(nex)) {
	    fprintf(out, "    ERROR: Q output nexus width=%u "
		    "does not match part width\n", width_of_nexus(nex));
	    stub_errors += 1;
      }

      nex = ivl_lpm_data(net, 1);
      fprintf(out, "    S: %p <width=%u>\n", nex, width_of_nexus(nex));
}

static void show_lpm_sign_ext(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      ivl_nexus_t nex_q = ivl_lpm_q(net);
      ivl_nexus_t nex_a = ivl_lpm_data(net,0);

      fprintf(out, "  LPM_SIGN_EXT %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      fprintf(out, "    Q: %p\n", nex_q);
      fprintf(out, "    D: %p <width=%u>\n", nex_a, width_of_nexus(nex_a));

      if (width != width_of_nexus(nex_q)) {
	    fprintf(out, "    ERROR: Width of Q is %u, expecting %u\n",
		    width_of_nexus(nex_q), width);
	    stub_errors += 1;
      }
}

static void show_lpm_sub(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_SUB %s: <width=%u>\n",
	      ivl_lpm_basename(net), width);

      show_lpm_arithmetic_pins(net);
}

static void show_lpm_substitute(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      ivl_nexus_t nex_q = ivl_lpm_q(net);
      ivl_nexus_t nex_a = ivl_lpm_data(net,0);
      ivl_nexus_t nex_s = ivl_lpm_data(net,1);

      unsigned sbase  = ivl_lpm_base(net);
      unsigned swidth = width_of_nexus(nex_s);

      fprintf(out, "  LPM_SUBSTITUTE %s: <width=%u, sbase=%u, swidth=%u>\n",
	      ivl_lpm_basename(net), width, sbase, swidth);
      fprintf(out, "    Q: %p\n", nex_q);
      if (width != width_of_nexus(nex_q)) {
	    fprintf(out, "    ERROR: Width of Q is %u, expecting %u\n",
		    width_of_nexus(nex_q), width);
	    stub_errors += 1;
      }
      fprintf(out, "    A: %p\n", nex_a);
      if (width != width_of_nexus(nex_a)) {
	    fprintf(out, "    ERROR: Width of A is %u, expecting %u\n",
		    width_of_nexus(nex_a), width);
	    stub_errors += 1;
      }
      fprintf(out, "    S: %p\n", nex_s);
      if (sbase + swidth > width) {
	    fprintf(out, "    ERROR: S part is out of bounds\n");
	    stub_errors += 1;
      }
}

static void show_lpm_sfunc(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned ports = ivl_lpm_size(net);
      ivl_variable_type_t data_type = type_of_nexus(ivl_lpm_q(net));
      ivl_nexus_t nex;
      unsigned idx;

      fprintf(out, "  LPM_SFUNC %s: <call=%s, width=%u, type=%s, ports=%u>\n",
	      ivl_lpm_basename(net), ivl_lpm_string(net),
	      width, data_type_string(data_type), ports);

      nex = ivl_lpm_q(net);
      if (width != width_of_nexus(nex)) {
	    fprintf(out, "    ERROR: Q output nexus width=%u "
		    " does not match part width\n", width_of_nexus(nex));
	    stub_errors += 1;
      }

      fprintf(out, "    Q: %p\n", nex);
      for (idx = 0 ;  idx < ports ;  idx += 1) {
	    nex = ivl_lpm_data(net, idx);
	    fprintf(out, "    D%u: %p <width=%u, type=%s>\n", idx,
		    nex, width_of_nexus(nex),
		    data_type_string(type_of_nexus(nex)));
      }
}

static void show_lpm_delays(ivl_lpm_t net)
{
      ivl_expr_t rise = ivl_lpm_delay(net, 0);
      ivl_expr_t fall = ivl_lpm_delay(net, 1);
      ivl_expr_t decay= ivl_lpm_delay(net, 2);

      if (rise==0 && fall==0 && decay==0)
	    return;

      fprintf(out, "    #DELAYS\n");
      if (rise)
	    show_expression(rise, 8);
      else
	    fprintf(out, "        ERROR: missing rise delay\n");
      if (fall)
	    show_expression(fall, 8);
      else
	    fprintf(out, "        ERROR: missing fall delay\n");
      if (decay)
	    show_expression(decay, 8);
      else
	    fprintf(out, "        ERROR: missing decay delay\n");
      fprintf(out, "    #END DELAYS\n");
}

static void show_lpm_ufunc(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned ports = ivl_lpm_size(net);
      ivl_scope_t def = ivl_lpm_define(net);
      ivl_nexus_t nex;
      unsigned idx;

      fprintf(out, "  LPM_UFUNC %s: <call=%s, width=%u, ports=%u>\n",
	      ivl_lpm_basename(net), ivl_scope_name(def), width, ports);

      show_lpm_delays(net);

      nex = ivl_lpm_q(net);
      if (width != width_of_nexus(nex)) {
	    fprintf(out, "    ERROR: Q output nexus width=%u "
		    " does not match part width\n", width_of_nexus(nex));
	    stub_errors += 1;
      }

      fprintf(out, "    Q: %p\n", nex);

      for (idx = 0 ;  idx < ports ;  idx += 1) {
	    nex = ivl_lpm_data(net, idx);
	    fprintf(out, "    D%u: %p <width=%u>\n", idx,
		    nex, width_of_nexus(nex));
      }
}

static void show_lpm(ivl_lpm_t net)
{

      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_ABS:
	    show_lpm_abs(net);
	    break;

	  case IVL_LPM_ADD:
	    show_lpm_add(net);
	    break;

	  case IVL_LPM_ARRAY:
	    show_lpm_array(net);
	    break;

	  case IVL_LPM_CAST_INT:
	    show_lpm_cast_int(net);
	    break;

	  case IVL_LPM_CAST_REAL:
	    show_lpm_cast_real(net);
	    break;

	  case IVL_LPM_DIVIDE:
	    show_lpm_divide(net);
	    break;

	  case IVL_LPM_CMP_EEQ:
	  case IVL_LPM_CMP_EQX:
	  case IVL_LPM_CMP_EQZ:
	  case IVL_LPM_CMP_NEE:
	  case IVL_LPM_CMP_WEQ:
	  case IVL_LPM_CMP_WNE:
	    show_lpm_cmp_eeq(net);
	    break;

	  case IVL_LPM_FF:
	    show_lpm_ff(net);
	    break;

	  case IVL_LPM_LATCH:
	    show_lpm_latch(net);
	    break;

	  case IVL_LPM_CMP_GE:
	    show_lpm_cmp_ge(net);
	    break;

	  case IVL_LPM_CMP_GT:
	    show_lpm_cmp_gt(net);
	    break;

	  case IVL_LPM_CMP_NE:
	    show_lpm_cmp_ne(net);
	    break;

	  case IVL_LPM_CONCAT:
	  case IVL_LPM_CONCATZ:
	    show_lpm_concat(net);
	    break;

	  case IVL_LPM_RE_AND:
	  case IVL_LPM_RE_NAND:
	  case IVL_LPM_RE_NOR:
	  case IVL_LPM_RE_OR:
	  case IVL_LPM_RE_XOR:
	  case IVL_LPM_RE_XNOR:
	    show_lpm_re(net);
	    break;

	  case IVL_LPM_SHIFTL:
	    show_lpm_shift(net, "L");
	    break;

	  case IVL_LPM_SIGN_EXT:
	    show_lpm_sign_ext(net);
	    break;

	  case IVL_LPM_SHIFTR:
	    show_lpm_shift(net, "R");
	    break;

	  case IVL_LPM_SUB:
	    show_lpm_sub(net);
	    break;

	  case IVL_LPM_SUBSTITUTE:
	    show_lpm_substitute(net);
	    break;

	  case IVL_LPM_MOD:
	    show_lpm_mod(net);
	    break;

	  case IVL_LPM_MULT:
	    show_lpm_mult(net);
	    break;

	  case IVL_LPM_MUX:
	    show_lpm_mux(net);
	    break;

	  case IVL_LPM_PART_VP:
	  case IVL_LPM_PART_PV:
	    show_lpm_part(net);
	    break;

	  case IVL_LPM_REPEAT:
	    show_lpm_repeat(net);
	    break;

	  case IVL_LPM_SFUNC:
	    show_lpm_sfunc(net);
	    break;

	  case IVL_LPM_UFUNC:
	    show_lpm_ufunc(net);
	    break;

	  default:
	    fprintf(out, "  LPM(%d) %s: <width=%u, signed=%d>\n",
		    ivl_lpm_type(net),
		    ivl_lpm_basename(net),
		    ivl_lpm_width(net),
		    ivl_lpm_signed(net));
      }
}


static int show_process(ivl_process_t net, void*x)
{
      unsigned idx;

      (void)x; /* Parameter is not used. */

      switch (ivl_process_type(net)) {
	  case IVL_PR_INITIAL:
	    if (ivl_process_analog(net))
		  fprintf(out, "analog initial\n");
	    else
		  fprintf(out, "initial\n");
	    break;
	  case IVL_PR_ALWAYS:
	    if (ivl_process_analog(net))
		  fprintf(out, "analog\n");
	    else
		  fprintf(out, "always\n");
	    break;
	  case IVL_PR_ALWAYS_COMB:
	    if (ivl_process_analog(net))
		  assert(0);
	    else
		  fprintf(out, "always_comb\n");
	    break;
	  case IVL_PR_ALWAYS_FF:
	    if (ivl_process_analog(net))
		  assert(0);
	    else
		  fprintf(out, "always_ff\n");
	    break;
	  case IVL_PR_ALWAYS_LATCH:
	    if (ivl_process_analog(net))
		  assert(0);
	    else
		  fprintf(out, "always_latch\n");
	    break;
	  case IVL_PR_FINAL:
	    if (ivl_process_analog(net))
		  fprintf(out, "analog final\n");
	    else
		  fprintf(out, "final\n");
	    break;
      }

      for (idx = 0 ;  idx < ivl_process_attr_cnt(net) ;  idx += 1) {
	    ivl_attribute_t attr = ivl_process_attr_val(net, idx);
	    switch (attr->type) {
		case IVL_ATT_VOID:
		  fprintf(out, "    (* %s *)\n", attr->key);
		  break;
		case IVL_ATT_STR:
		  fprintf(out, "    (* %s = \"%s\" *)\n", attr->key,
			  attr->val.str);
		  break;
		case IVL_ATT_NUM:
		  fprintf(out, "    (* %s = %ld *)\n", attr->key,
			  attr->val.num);
		  break;
	    }
      }

      show_statement(ivl_process_stmt(net), 4);

      return 0;
}

static void show_parameter(ivl_parameter_t net)
{
      const char*name = ivl_parameter_basename(net);
      fprintf(out, "   parameter %s;\n", name);
      show_expression(ivl_parameter_expr(net), 7);
}

static void show_event(ivl_event_t net)
{
      unsigned idx;
      fprintf(out, "  event %s (%u pos, %u neg, %u any);\n",
	      ivl_event_basename(net), ivl_event_npos(net),
	      ivl_event_nneg(net), ivl_event_nany(net));

      for (idx = 0 ;  idx < ivl_event_nany(net) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_event_any(net, idx);
	    fprintf(out, "      ANYEDGE: %p\n", nex);
      }

      for (idx = 0 ;  idx < ivl_event_nneg(net) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_event_neg(net, idx);
	    fprintf(out, "      NEGEDGE: %p\n", nex);
      }

      for (idx = 0 ;  idx < ivl_event_npos(net) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_event_pos(net, idx);
	    fprintf(out, "      POSEDGE: %p\n", nex);
      }
}

static const char* str_tab[8] = {
      "HiZ", "small", "medium", "weak",
      "large", "pull", "strong", "supply"};

/*
 * This function is used by the show_signal to dump a constant value
 * that is connected to the signal. While we are here, check that the
 * value is consistent with the signal itself.
 */
static void signal_nexus_const(ivl_signal_t sig,
			       ivl_nexus_ptr_t ptr,
			       ivl_net_const_t con)
{
      const char*dr0 = str_tab[ivl_nexus_ptr_drive0(ptr)];
      const char*dr1 = str_tab[ivl_nexus_ptr_drive1(ptr)];

      const char*bits;
      unsigned idx, width = ivl_const_width(con);

      fprintf(out, "      const-");

      switch (ivl_const_type(con)) {
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    bits = ivl_const_bits(con);
	    assert(bits);
	    for (idx = 0 ;  idx < width ;  idx += 1) {
		  fprintf(out, "%c", bits[width-idx-1]);
	    }
	    break;

	  case IVL_VT_REAL:
	    fprintf(out, "%f", ivl_const_real(con));
	    break;

	  default:
	    fprintf(out, "????");
	    break;
      }

      fprintf(out, " (%s0, %s1, width=%u)\n", dr0, dr1, width);

      if (ivl_signal_width(sig) != width) {
	    fprintf(out, "ERROR: Width of signal does not match "
		    "width of connected constant vector.\n");
	    stub_errors += 1;
      }

      int drive_type_ok = check_signal_drive_type(ivl_signal_data_type(sig),
						  ivl_const_type(con));

      if (! drive_type_ok) {
	    fprintf(out, "ERROR: Signal data type does not match"
		    " literal type.\n");
	    stub_errors += 1;
      }
}

static void show_nexus_details(ivl_signal_t net, ivl_nexus_t nex)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_net_const_t con;
	    ivl_net_logic_t logic;
	    ivl_lpm_t lpm;
	    ivl_signal_t sig;
	    ivl_switch_t swt;
	    ivl_branch_t bra;
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);

	    const char*dr0 = str_tab[ivl_nexus_ptr_drive0(ptr)];
	    const char*dr1 = str_tab[ivl_nexus_ptr_drive1(ptr)];

	    if ((sig = ivl_nexus_ptr_sig(ptr))) {
		  fprintf(out, "      SIG %s word=%u (%s0, %s1)",
			  ivl_signal_name(sig), ivl_nexus_ptr_pin(ptr), dr0, dr1);

		  if (ivl_signal_width(sig) != ivl_signal_width(net)) {
			fprintf(out, " (ERROR: Width=%u)",
				ivl_signal_width(sig));
			stub_errors += 1;
		  }

		  if (ivl_signal_data_type(sig) != ivl_signal_data_type(net)) {
			fprintf(out, " (ERROR: data type mismatch : %s vs. %s)",
				data_type_string(ivl_signal_data_type(sig)),
				data_type_string(ivl_signal_data_type(net)));
			stub_errors += 1;
		  }

		  fprintf(out, "\n");

	    } else if ((logic = ivl_nexus_ptr_log(ptr))) {
		  fprintf(out, "      LOG %s.%s[%u] (%s0, %s1)\n",
			  ivl_scope_name(ivl_logic_scope(logic)),
			  ivl_logic_basename(logic),
			  ivl_nexus_ptr_pin(ptr), dr0, dr1);

	    } else if ((lpm = ivl_nexus_ptr_lpm(ptr))) {
		  fprintf(out, "      LPM %s.%s (%s0, %s1)\n",
			  ivl_scope_name(ivl_lpm_scope(lpm)),
			  ivl_lpm_basename(lpm), dr0, dr1);

	    } else if ((swt = ivl_nexus_ptr_switch(ptr))) {
		  fprintf(out, "      SWITCH %s.%s\n",
			  ivl_scope_name(ivl_switch_scope(swt)),
			  ivl_switch_basename(swt));

	    } else if ((con = ivl_nexus_ptr_con(ptr))) {
		  signal_nexus_const(net, ptr, con);

	    } else if ((bra = ivl_nexus_ptr_branch(ptr))) {
		  fprintf(out, "      BRANCH %p terminal %u\n",
			  bra, ivl_nexus_ptr_pin(ptr));

	    } else {
		  fprintf(out, "      ?[%u] (%s0, %s1)\n",
			  ivl_nexus_ptr_pin(ptr), dr0, dr1);
	    }
      }
}

static void show_signal(ivl_signal_t net)
{
      unsigned idx;

      const char*type = "?";
      const char*port = "";
      const char*sign = ivl_signal_signed(net)? "signed" : "unsigned";

      switch (ivl_signal_type(net)) {
	  case IVL_SIT_REG:
	    type = "reg";
	    if (ivl_signal_integer(net))
		  type = "integer";
	    break;
	  case IVL_SIT_TRI:
	    type = "tri";
	    break;
	  case IVL_SIT_TRI0:
	    type = "tri0";
	    break;
	  case IVL_SIT_TRI1:
	    type = "tri1";
	    break;
	  case IVL_SIT_UWIRE:
	    type = "uwire";
	    break;
	  default:
	    break;
      }

      switch (ivl_signal_port(net)) {

	  case IVL_SIP_INPUT:
	    port = "input ";
	    break;

	  case IVL_SIP_OUTPUT:
	    port = "output ";
	    break;

	  case IVL_SIP_INOUT:
	    port = "inout ";
	    break;

	  case IVL_SIP_NONE:
	    break;
      }

      const char*discipline_txt = "NONE";
      if (ivl_signal_discipline(net)) {
	    ivl_discipline_t dis = ivl_signal_discipline(net);
	    discipline_txt = ivl_discipline_name(dis);
      }

      for (idx = 0 ;  idx < ivl_signal_array_count(net) ; idx += 1) {

	    ivl_nexus_t nex = ivl_signal_nex(net, idx);

	    fprintf(out, "  %s %s %s", type, sign, port);
	    show_type_of_signal(net);
	    fprintf(out, " %s[word=%u, adr=%u]  <width=%u%s> <discipline=%s> ",
		    ivl_signal_basename(net),
		    idx, ivl_signal_array_base(net)+idx,
		    ivl_signal_width(net),
		    ivl_signal_local(net)? ", local":"",
		    discipline_txt);
	    if (nex == NULL) {
		  fprintf(out, "nexus=<virtual>\n");
		  continue;
	    } else {
		  fprintf(out, "nexus=%p\n", nex);
	    }

	    show_nexus_details(net, nex);
      }

      for (idx = 0 ;  idx < ivl_signal_npath(net) ;  idx += 1) {
	    ivl_delaypath_t path = ivl_signal_path(net,idx);
	    ivl_nexus_t nex = ivl_path_source(path);
	    ivl_nexus_t con = ivl_path_condit(path);
	    int posedge = ivl_path_source_posedge(path);
	    int negedge = ivl_path_source_negedge(path);

	    fprintf(out, "      path %p", nex);
	    if (posedge) fprintf(out, " posedge");
	    if (negedge) fprintf(out, " negedge");
	    if (con) fprintf(out, " (if %p)", con);
	    else if (ivl_path_is_condit(path)) fprintf(out, " (ifnone)");
	    fprintf(out, " %" PRIu64 ",%" PRIu64 ",%" PRIu64
		         " %" PRIu64 ",%" PRIu64 ",%" PRIu64
		         " %" PRIu64 ",%" PRIu64 ",%" PRIu64
		         " %" PRIu64 ",%" PRIu64 ",%" PRIu64,
		    ivl_path_delay(path, IVL_PE_01),
		    ivl_path_delay(path, IVL_PE_10),
		    ivl_path_delay(path, IVL_PE_0z),
		    ivl_path_delay(path, IVL_PE_z1),
		    ivl_path_delay(path, IVL_PE_1z),
		    ivl_path_delay(path, IVL_PE_z0),
		    ivl_path_delay(path, IVL_PE_0x),
		    ivl_path_delay(path, IVL_PE_x1),
		    ivl_path_delay(path, IVL_PE_1x),
		    ivl_path_delay(path, IVL_PE_x0),
		    ivl_path_delay(path, IVL_PE_xz),
		    ivl_path_delay(path, IVL_PE_zx));
	    fprintf(out, " scope=%s\n", ivl_scope_name(ivl_path_scope(path)));
      }

      for (idx = 0 ;  idx < ivl_signal_attr_cnt(net) ;  idx += 1) {
	    ivl_attribute_t atr = ivl_signal_attr_val(net, idx);

	    switch (atr->type) {
		case IVL_ATT_STR:
		  fprintf(out, "    %s = %s\n", atr->key, atr->val.str);
		  break;
		case IVL_ATT_NUM:
		  fprintf(out, "    %s = %ld\n", atr->key, atr->val.num);
		  break;
		case IVL_ATT_VOID:
		  fprintf(out, "    %s\n", atr->key);
		  break;
	    }
      }

      switch (ivl_signal_data_type(net)) {
	  case IVL_VT_NO_TYPE:
	  case IVL_VT_VOID:
	    fprintf(out, "  ERROR: Invalid type for signal.\n");
	    stub_errors += 1;
	    break;
	  default:
	    break;
      }

      if (ivl_signal_integer(net) && ivl_signal_type(net)!=IVL_SIT_REG) {
	    fprintf(out, "  ERROR: integers must be IVL_SIT_REG\n");
	    stub_errors += 1;
      }
}

void test_expr_is_delay(ivl_expr_t expr)
{
      switch (ivl_expr_type(expr)) {
	  case IVL_EX_ULONG:
	    return;
	  case IVL_EX_NUMBER:
	    return;
	  case IVL_EX_SIGNAL:
	    return;
	  default:
	    break;
      }

      fprintf(out, "      ERROR: Expression is not a suitable delay\n");
      stub_errors += 1;
}

/*
 * All logic gates have inputs and outputs that match exactly in
 * width. For example, and AND gate with 4 bit inputs generates a 4
 * bit output, and all the inputs are 4 bits.
 */
static void show_logic(ivl_net_logic_t net)
{
      unsigned npins, idx;
      const char*name = ivl_logic_basename(net);
      ivl_drive_t drive0 = ivl_logic_drive0(net);
      ivl_drive_t drive1 = ivl_logic_drive1(net);

      switch (ivl_logic_type(net)) {
	  case IVL_LO_AND:
	    fprintf(out, "  and %s", name);
	    break;
	  case IVL_LO_BUF:
	    fprintf(out, "  buf %s", name);
	    break;
	  case IVL_LO_BUFIF0:
	    fprintf(out, "  bufif0 %s", name);
	    break;
	  case IVL_LO_BUFIF1:
	    fprintf(out, "  bufif1 %s", name);
	    break;
	  case IVL_LO_BUFT:
	    fprintf(out, "  buft %s", name);
	    break;
	  case IVL_LO_BUFZ:
	    fprintf(out, "  bufz %s", name);
	    break;
	  case IVL_LO_CMOS:
	    fprintf(out, "  cmos %s", name);
	    break;
	  case IVL_LO_NAND:
	    fprintf(out, "  nand %s", name);
	    break;
	  case IVL_LO_NMOS:
	    fprintf(out, "  nmos %s", name);
	    break;
	  case IVL_LO_NOR:
	    fprintf(out, "  nor %s", name);
	    break;
	  case IVL_LO_NOT:
	    fprintf(out, "  not %s", name);
	    break;
	  case IVL_LO_NOTIF0:
	    fprintf(out, "  notif0 %s", name);
	    break;
	  case IVL_LO_NOTIF1:
	    fprintf(out, "  notif1 %s", name);
	    break;
	  case IVL_LO_OR:
	    fprintf(out, "  or %s", name);
	    break;
	  case IVL_LO_PMOS:
	    fprintf(out, "  pmos %s", name);
	    break;
	  case IVL_LO_PULLDOWN:
	    fprintf(out, "  pulldown %s", name);
	    break;
	  case IVL_LO_PULLUP:
	    fprintf(out, "  pullup %s", name);
	    break;
	  case IVL_LO_RCMOS:
	    fprintf(out, "  rcmos %s", name);
	    break;
	  case IVL_LO_RNMOS:
	    fprintf(out, "  rnmos %s", name);
	    break;
	  case IVL_LO_RPMOS:
	    fprintf(out, "  rpmos %s", name);
	    break;
	  case IVL_LO_XNOR:
	    fprintf(out, "  xnor %s", name);
	    break;
	  case IVL_LO_XOR:
	    fprintf(out, "  xor %s", name);
	    break;

	  case IVL_LO_UDP:
	    fprintf(out, "  primitive<%s> %s",
		    ivl_udp_name(ivl_logic_udp(net)), name);
	    break;

	  default:
	    fprintf(out, "  unsupported gate<type=%d> %s", ivl_logic_type(net), name);
	    break;
      }

      fprintf(out, " <width=%u>\n", ivl_logic_width(net));

      fprintf(out, "    <Delays...>\n");
      if (ivl_logic_delay(net,0)) {
	    test_expr_is_delay(ivl_logic_delay(net,0));
	    show_expression(ivl_logic_delay(net,0), 6);
      }
      if (ivl_logic_delay(net,1)) {
	    test_expr_is_delay(ivl_logic_delay(net,1));
	    show_expression(ivl_logic_delay(net,1), 6);
      }
      if (ivl_logic_delay(net,2)) {
	    test_expr_is_delay(ivl_logic_delay(net,2));
	    show_expression(ivl_logic_delay(net,2), 6);
      }

      npins = ivl_logic_pins(net);

	/* Show the pins of the gate. Pin-0 is always the output, and
	   the remaining pins are the inputs. Inputs may be
	   unconnected, but if connected the nexus width must exactly
	   match the gate width. */
      for (idx = 0 ;  idx < npins ;  idx += 1) {
	    ivl_nexus_t nex = ivl_logic_pin(net, idx);

	    fprintf(out, "    %u: %p", idx, nex);
	    if (idx == 0)
		  fprintf(out, " <drive0/1 = %d/%d>", drive0, drive1);
	    fprintf(out, "\n");

	    if (nex == 0) {
		  if (idx == 0) {
			fprintf(out, "    0: ERROR: Pin 0 must not "
				"be unconnected\n");
			stub_errors += 1;
		  }
		  continue;
	    }

	    if (ivl_logic_width(net) != width_of_nexus(nex)) {
		  fprintf(out, "    %u: ERROR: Nexus width is %u\n",
			  idx, width_of_nexus(nex));
		  stub_errors += 1;
	    }
      }

	/* If this is an instance of a UDP, then check that the
	   instantiation is consistent with the definition. */
      if (ivl_logic_type(net) == IVL_LO_UDP) {
	    ivl_udp_t udp = ivl_logic_udp(net);
	    if (npins != 1+ivl_udp_nin(udp)) {
		  fprintf(out, "    ERROR: UDP %s expects %u inputs\n",
			  ivl_udp_name(udp), ivl_udp_nin(udp));
		  stub_errors += 1;
	    }

	      /* Add a reference to this udp definition. */
	    reference_udp_definition(udp);
      }

      npins = ivl_logic_attr_cnt(net);
      for (idx = 0 ;  idx < npins ;  idx += 1) {
	    ivl_attribute_t cur = ivl_logic_attr_val(net,idx);
	    switch (cur->type) {
		case IVL_ATT_VOID:
		  fprintf(out, "    %s\n", cur->key);
		  break;
		case IVL_ATT_NUM:
		  fprintf(out, "    %s = %ld\n", cur->key, cur->val.num);
		  break;
		case IVL_ATT_STR:
		  fprintf(out, "    %s = %s\n", cur->key, cur->val.str);
		  break;
	    }
      }
}

static int show_scope(ivl_scope_t net, void*x)
{
      unsigned idx;
      const char *is_auto;

      (void)x; /* Parameter is not used. */

      fprintf(out, "scope: %s (%u parameters, %u signals, %u logic)",
	      ivl_scope_name(net), ivl_scope_params(net),
	      ivl_scope_sigs(net), ivl_scope_logs(net));

      is_auto = ivl_scope_is_auto(net) ? "automatic " : "";
      switch (ivl_scope_type(net)) {
	  case IVL_SCT_MODULE:
	    fprintf(out, " module %s%s", ivl_scope_tname(net),
                    ivl_scope_is_cell(net) ? " (cell)" : "");
	    break;
	  case IVL_SCT_FUNCTION:
	    fprintf(out, " function %s%s", is_auto, ivl_scope_tname(net));
	    break;
	  case IVL_SCT_BEGIN:
	    fprintf(out, " begin : %s", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_FORK:
	    fprintf(out, " fork : %s", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_TASK:
	    fprintf(out, " task %s%s", is_auto, ivl_scope_tname(net));
	    break;
	  case IVL_SCT_CLASS:
	    fprintf(out, " class %s", ivl_scope_tname(net));
	    break;
	  default:
	    fprintf(out, " type(%d) %s", ivl_scope_type(net),
		    ivl_scope_tname(net));
	    break;
      }

      fprintf(out, " time units = 1e%d\n", ivl_scope_time_units(net));
      fprintf(out, " time precision = 1e%d\n", ivl_scope_time_precision(net));

      for (idx = 0 ;  idx < ivl_scope_attr_cnt(net) ;  idx += 1) {
	    ivl_attribute_t attr = ivl_scope_attr_val(net, idx);
	    switch (attr->type) {
		case IVL_ATT_VOID:
		  fprintf(out, "  (* %s *)\n", attr->key);
		  break;
		case IVL_ATT_STR:
		  fprintf(out, "  (* %s = \"%s\" *)\n", attr->key,
			  attr->val.str);
		  break;
		case IVL_ATT_NUM:
		  fprintf(out, "  (* %s = %ld *)\n", attr->key,
			  attr->val.num);
		  break;
	    }
      }

      for (idx = 0 ;  idx < ivl_scope_classes(net) ; idx += 1)
	    show_class(ivl_scope_class(net, idx));

      for (idx = 0 ;  idx < ivl_scope_params(net) ;  idx += 1)
	    show_parameter(ivl_scope_param(net, idx));

      for (idx = 0 ; idx < ivl_scope_enumerates(net) ; idx += 1)
	    show_enumerate(ivl_scope_enumerate(net, idx));

      for (idx = 0 ;  idx < ivl_scope_sigs(net) ;  idx += 1)
	    show_signal(ivl_scope_sig(net, idx));

      for (idx = 0 ;  idx < ivl_scope_events(net) ;  idx += 1)
	    show_event(ivl_scope_event(net, idx));

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1)
	    show_logic(ivl_scope_log(net, idx));

      for (idx = 0 ;  idx < ivl_scope_lpms(net) ;  idx += 1)
	    show_lpm(ivl_scope_lpm(net, idx));

      for (idx = 0 ; idx < ivl_scope_switches(net) ; idx += 1)
	    show_switch(ivl_scope_switch(net, idx));

      switch (ivl_scope_type(net)) {
	  case IVL_SCT_FUNCTION:
	  case IVL_SCT_TASK:
	    fprintf(out, "  scope function/task definition\n");
	    if (ivl_scope_def(net) == 0) {
		  fprintf(out, "  ERROR: scope missing required task definition\n");
		  stub_errors += 1;
	    } else {
		  show_statement(ivl_scope_def(net), 6);
	    }
	    break;

	  default:
	    if (ivl_scope_def(net)) {
		  fprintf(out, "  ERROR: scope has an attached task definition:\n");
		  show_statement(ivl_scope_def(net), 6);
		  stub_errors += 1;
	    }
	    break;
      }

      fprintf(out, "end scope %s\n", ivl_scope_name(net));
      return ivl_scope_children(net, show_scope, 0);
}

static void show_primitive(ivl_udp_t net, unsigned ref_count)
{
      unsigned rdx;

      fprintf(out, "primitive %s (referenced %u times)\n",
	      ivl_udp_name(net), ref_count);

      if (ivl_udp_sequ(net))
	    fprintf(out, "    reg out = %c;\n", ivl_udp_init(net));
      else
	    fprintf(out, "    wire out;\n");
      fprintf(out, "    table\n");
      for (rdx = 0 ;  rdx < ivl_udp_rows(net) ;  rdx += 1) {

	      /* Each row has the format:
		 Combinational: iii...io
		 Sequential:  oiii...io
		 In the sequential case, the o value in the front is
		 the current output value. */
	    unsigned idx;
	    const char*row = ivl_udp_row(net,rdx);

	    fprintf(out, "    ");

	    if (ivl_udp_sequ(net))
		  fprintf(out, " cur=%c :", *row++);

	    for (idx = 0 ;  idx < ivl_udp_nin(net) ;  idx += 1)
		  fprintf(out, " %c", *row++);

	    fprintf(out, " : out=%c\n", *row++);
      }
      fprintf(out, "    endtable\n");

      fprintf(out, "endprimitive\n");
}

int target_design(ivl_design_t des)
{
      ivl_scope_t*root_scopes;
      unsigned nroot = 0;
      unsigned idx;

      const char*path = ivl_design_flag(des, "-o");
      if (path == 0) {
	    return -1;
      }

      out = fopen(path, "w");
      if (out == 0) {
	    perror(path);
	    return -2;
      }

      for (idx = 0 ; idx < ivl_design_disciplines(des) ; idx += 1) {
	    ivl_discipline_t dis = ivl_design_discipline(des,idx);
	    fprintf(out, "discipline %s\n", ivl_discipline_name(dis));
      }

      ivl_design_roots(des, &root_scopes, &nroot);
      for (idx = 0 ;  idx < nroot ;  idx += 1) {

	    ivl_scope_t cur = root_scopes[idx];
	    switch (ivl_scope_type(cur)) {
		case IVL_SCT_TASK:
		  fprintf(out, "task = %s\n", ivl_scope_name(cur));
		  break;
		case IVL_SCT_FUNCTION:
		  fprintf(out, "function = %s\n", ivl_scope_name(cur));
		  break;
		case IVL_SCT_CLASS:
		  fprintf(out, "class = %s\n", ivl_scope_name(cur));
		  break;
		case IVL_SCT_PACKAGE:
		  fprintf(out, "package = %s\n", ivl_scope_name(cur));
		  break;
		case IVL_SCT_MODULE:
		  fprintf(out, "root module = %s\n", ivl_scope_name(cur));
		  break;
		default:
		  fprintf(out, "ERROR scope %s unknown type\n", ivl_scope_name(cur));
		  stub_errors += 1;
		  break;
	    }

	    show_scope(root_scopes[idx], 0);
      }

      while (udp_define_list) {
	    struct udp_define_cell*cur = udp_define_list;
	    udp_define_list = cur->next;
	    show_primitive(cur->udp, cur->ref);
	    free(cur);
      }

      fprintf(out, "# There are %u constants detected\n", ivl_design_consts(des));
      for (idx = 0 ; idx < ivl_design_consts(des) ; idx += 1) {
	    ivl_net_const_t con = ivl_design_const(des, idx);
	    show_constant(con);
      }

      ivl_design_process(des, show_process, 0);
      fclose(out);

      return stub_errors;
}

const char* target_query(const char*key)
{
      if (strcmp(key,"version") == 0)
	    return version_string;

      return 0;
}
