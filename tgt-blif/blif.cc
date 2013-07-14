/*
 * Copyright (c) 2013 Stephen Williams (steve@icarus.com)
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

# include  "version_base.h"
# include  "version_tag.h"
# include  "config.h"
# include  "ivl_target.h"
# include  "nex_data.h"
# include  <vector>
# include  <cstdio>
# include  <cstring>
# include  <cassert>

using namespace std;

/*
 * This is a BLIF target module.
 */

static const char*version_string =
"Icarus Verilog BLIF Code Generator " VERSION " (" VERSION_TAG ")\n\n"
"Copyright (c) 2013 Stephen Williams (steve@icarus.com)\n\n"
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

static int emit_blif(const char*blif_path, ivl_scope_t model);

int target_design(ivl_design_t des)
{
      int rc = 0;
      const char*blif_path = ivl_design_flag(des, "-o");

	// Locate the root scope for the design. Note that the BLIF
	// format implies that there is a single root of the model.
      ivl_scope_t*roots;
      unsigned nroots;
      ivl_design_roots(des, &roots, &nroots);
      if (nroots != 1) {
	    fprintf(stderr, "BLIF: The BLIF code generator requires that there be only one root scope.\n");
	    return 1;
      }

      assert(roots[0]);

      if (ivl_scope_type(roots[0]) != IVL_SCT_MODULE) {
	    fprintf(stderr, "BLIF: The root scope %s must be a module.\n", ivl_scope_basename(roots[0]));
	    return 1;
      }

	// Emit to the destination file.
      assert(blif_path);
      rc += emit_blif(blif_path, roots[0]);

      return rc;
}


const char* target_query(const char*key)
{
      if (strcmp(key,"version") == 0)
	    return version_string;

      return 0;
}

/*
 * Print all the bits of a signal. This is for the .input or .output
 * lines of a .model. All the bits need to be exploded, so print each
 * bit of a vector as its own name.
 */
static void print_signal_bits(FILE*fd, ivl_signal_t sig)
{
      ivl_nexus_t nex = ivl_signal_nex(sig, 0);
      blif_nex_data_t* ned = blif_nex_data_t::get_nex_data(nex);
      ned->set_name(ivl_signal_basename(sig));

      if (ivl_signal_packed_dimensions(sig) == 0) {
	    fprintf(fd, " %s", ivl_signal_basename(sig));
	    return;
      }

      assert(ivl_signal_packed_dimensions(sig) == 1);

      int msb = ivl_signal_packed_msb(sig,0);
      int lsb = ivl_signal_packed_lsb(sig,0);
      if (msb < lsb) {
	    int tmp = msb;
	    msb = lsb;
	    lsb = tmp;
      }

      for (int idx = msb ; idx >= lsb ; idx -= 1) {
	    fprintf(fd, " %s[%d]", ivl_signal_basename(sig), idx);
      }
}

static void print_logic_gate(FILE*fd, ivl_net_logic_t net)
{
#if 0
      fprintf(fd, "# LOGIC: name=%s, type=%d, pins=%u, width=%u\n",
	      ivl_logic_basename(net), ivl_logic_type(net),
	      ivl_logic_pins(net), ivl_logic_width(net));
#endif

      fprintf(fd, ".names");
      ivl_nexus_t nex;
      blif_nex_data_t*ned;
      for (unsigned idx = 1 ; idx < ivl_logic_pins(net) ; idx += 1) {
	    nex = ivl_logic_pin(net,idx);
	    ned = blif_nex_data_t::get_nex_data(nex);
 	    fprintf(fd, " %s", ned->get_name());
      }
      nex = ivl_logic_pin(net,0);
      ned = blif_nex_data_t::get_nex_data(nex);
      fprintf(fd, " %s", ned->get_name());
      fprintf(fd, "\n");

      switch (ivl_logic_type(net)) {
	  case IVL_LO_AND:
	    for (unsigned idx = 1 ; idx < ivl_logic_pins(net) ; idx += 1)
		  fprintf(fd, "1");
	    fprintf(fd, " 1\n");
	    break;
	  case IVL_LO_OR:
	    assert(ivl_logic_pins(net)==3);
	    fprintf(fd, "1- 1\n");
	    fprintf(fd, "-1 1\n");
	    break;
	  case IVL_LO_XOR:
	    assert(ivl_logic_pins(net)==3);
	    fprintf(fd, "10 1\n");
	    fprintf(fd, "01 1\n");
	    break;
	  default:
	    fprintf(fd, "# ERROR: Logic type not handled\n");
	    break;
      }
}

static int emit_blif(const char*blif_path, ivl_scope_t model)
{
      int rc = 0;

      FILE*fd = fopen(blif_path, "wt");
      if (fd == 0) {
	    perror(blif_path);
	    return 1;
      }

      fprintf(fd, ".model %s\n", ivl_scope_basename(model));

	// The root scope 
      vector<ivl_signal_t> ports_in;
      vector<ivl_signal_t> ports_out;
      for (unsigned idx = 0 ; idx < ivl_scope_sigs(model) ; idx += 1) {
	    ivl_signal_t prt = ivl_scope_sig(model, idx);
	    ivl_signal_port_t dir = ivl_signal_port(prt);

	    switch (dir) {
		case IVL_SIP_NONE:
		  break;
		case IVL_SIP_INPUT:
		  ports_in.push_back(prt);
		  break;
		case IVL_SIP_OUTPUT:
		  ports_out.push_back(prt);
		  break;
		case IVL_SIP_INOUT:
		  fprintf(stderr, "BLIF: error: "
			  "Model port %s is bi-directional.\n",
			  ivl_signal_basename(prt));
		  rc += 1;
		  ports_in.push_back(prt);
		  ports_out.push_back(prt);
		  break;
	    }
      }

      if (ports_in.size() > 0) {
	    fprintf(fd, ".inputs");
	    for (size_t idx = 0 ; idx < ports_in.size() ; idx += 1) {
		  ivl_signal_t prt = ports_in[idx];
		  print_signal_bits(fd, prt);
	    }
	    fprintf(fd, "\n");
      }
      if (ports_out.size() > 0) {
	    fprintf(fd, ".outputs");
	    for (size_t idx = 0 ; idx < ports_out.size() ; idx += 1) {
		  ivl_signal_t prt = ports_out[idx];
		  print_signal_bits(fd, prt);
	    }
	    fprintf(fd, "\n");
      }

      for (unsigned idx = 0 ; idx < ivl_scope_logs(model) ; idx += 1) {
	    ivl_net_logic_t net = ivl_scope_log(model, idx);
	    assert(net);
	    print_logic_gate(fd, net);
      }

      fprintf(fd, ".end\n");
      fclose(fd);

      return rc;
}
