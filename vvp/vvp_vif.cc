/*
 * Copyright (c) 2026 Stephen Williams (steve@icarus.com)
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

# include  "vvp_vif.h"
# include  "event.h"
# include  "compile.h"
# include  "codes.h"
# include  <cassert>

using namespace std;

vvp_vif::vvp_vif(size_t nmembers)
: nmembers_(nmembers),
  signals_(nmembers, static_cast<vvp_net_t*>(0)),
  pos_events_(nmembers, static_cast<vvp_net_t*>(0)),
  neg_events_(nmembers, static_cast<vvp_net_t*>(0)),
  any_events_(nmembers, static_cast<vvp_net_t*>(0))
{
}

vvp_vif::~vvp_vif()
{
}

void vvp_vif::set_member(size_t idx, vvp_net_t*sig)
{
      assert(idx < nmembers_);
      signals_[idx] = sig;
}

vvp_net_t* vvp_vif::signal(size_t idx) const
{
      assert(idx < nmembers_);
      return signals_[idx];
}

vvp_net_t* vvp_vif::edge_event(size_t idx, edge_t edge)
{
      assert(idx < nmembers_);

      vvp_net_t*sig = signals_[idx];
      assert(sig);

      vvp_net_t*&cache = (edge==EDGE_POSEDGE) ? pos_events_[idx]
                        : (edge==EDGE_NEGEDGE)? neg_events_[idx]
                        :                       any_events_[idx];

      if (cache != 0)
	    return cache;

      vvp_net_t*ep = new vvp_net_t;
      if (edge == EDGE_ANYEDGE)
	    ep->fun = new vvp_fun_anyedge_sa;
      else
	    ep->fun = new vvp_fun_edge_sa(edge==EDGE_POSEDGE
	                                   ? vvp_edge_posedge
	                                   : vvp_edge_negedge);

	// Wire the member signal's output into the new edge
	// detector's only input, so it sees every change of the
	// signal from now on.
      sig->link(vvp_net_ptr_t(ep, 0));

      cache = ep;
      return ep;
}

/*
 * %new/vif <n>, <sig0>, <sig1>, ...
 *
 * This is compiled much like a .event statement: the operand list is
 * a variable-length list of symbols, so it cannot go through the
 * generic, fixed-arity %-opcode table (compile_code()/opcode_table).
 * Instead it gets its own lexer/grammar rule (see the K_new_vif token
 * in lexor.lex/parse.y), the same way %vpi_call does.
 *
 * The <n> member signals are resolved (immediately, or postponed if
 * they are forward references) to their vvp_net_t, and that list is
 * attached to the emitted %new/vif instruction. At run time,
 * of_NEW_VIF() uses that list to build a fresh vvp_vif object with
 * one member per net.
 */
void compile_new_vif(char*label, uint64_t n, unsigned argc, struct symb_s*argv)
{
      if (label)
	    compile_codelabel(label);

      assert(n == argc);

      vvp_net_t**list = new vvp_net_t*[argc];
      for (unsigned idx = 0 ; idx < argc ; idx += 1) {
	    list[idx] = 0;
	    functor_ref_lookup(&list[idx], argv[idx].text);
      }

      vvp_code_t code = codespace_allocate();
      code->opcode = of_NEW_VIF;
      code->net_list = list;
      code->bit_idx[0] = argc;

      free(argv);
}
