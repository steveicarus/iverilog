/*
 * Copyright (c) 2008-2022 Stephen Williams (steve@icarus.com)
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

# include  "vvp_island.h"
# include  "compile.h"
# include  "symbols.h"
# include  "schedule.h"
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <iostream>
# include  <list>
# include  <cassert>
# include  <cstdlib>
# include  <cstring>
# include "ivl_alloc.h"

using namespace std;

#ifdef CHECK_WITH_VALGRIND
static bool at_EOS = false;
#endif

void island_send_value(vvp_net_t*net, const vvp_vector8_t&val)
{
      vvp_island_port*fun = dynamic_cast<vvp_island_port*>(net->fun);
      if (fun->outvalue .eeq(val))
	    return;

      fun->outvalue = val;
      net->send_vec8(fun->outvalue);
}

/*
 * Implementations...
 */
vvp_island::vvp_island()
{
      flagged_ = false;
      branches_ = 0;
      ports_ = 0;
      anodes_ = 0;
      bnodes_ = 0;
}

vvp_island::~vvp_island()
{
	// We can only delete islands at the end of simulation.
#ifdef CHECK_WITH_VALGRIND
      if (!at_EOS) assert(0);
#endif

      while (branches_) {
	    vvp_island_branch *next_br = branches_->next_branch;
	    delete branches_;
	    branches_ = next_br;
      }
}

void vvp_island::flag_island()
{
      if (flagged_ == true)
	    return;

      schedule_generic(this, 0, false, false);
      flagged_ = true;
}

/*
* This method handles the callback from the scheduler. It does basic
* housecleaning and calls the run_island() method implemented by the
* derived class.
*/
void vvp_island::run_run()
{
      flagged_ = false;
      run_island();
}


void vvp_island::add_port(const char*key, vvp_net_t*net)
{
      if (ports_ == 0)
	    ports_ = new symbol_map_s<vvp_net_t>;

	// each port should have a unique label
      assert(ports_->sym_get_value(key) == 0);

      ports_->sym_set_value(key, net);
}

void vvp_island::add_branch(vvp_island_branch*branch, const char*pa, const char*pb)
{
      vvp_island_branch*cur;
      assert(ports_);
      branch->a = ports_->sym_get_value(pa);
      branch->b = ports_->sym_get_value(pb);
      assert(branch->a && branch->b);

      vvp_branch_ptr_t ptra (branch, 0);
      vvp_branch_ptr_t ptrb (branch, 1);
      if (anodes_ == 0)
	    anodes_ = new symbol_map_s<vvp_island_branch>;
      if (bnodes_ == 0)
	    bnodes_ = new symbol_map_s<vvp_island_branch>;

      if ((cur = anodes_->sym_get_value(pa))) {
	    branch->link[0] = cur->link[0];
	    cur->link[0] = ptra;
      } else if ((cur = bnodes_->sym_get_value(pa))) {
	    branch->link[0] = cur->link[1];
	    cur->link[1] = ptra;
      } else {
	    branch->link[0] = ptra;
	    anodes_->sym_set_value(pa, branch);
      }

      if ((cur = anodes_->sym_get_value(pb))) {
	    branch->link[1] = cur->link[0];
	    cur->link[0] = ptrb;
      } else if ((cur = bnodes_->sym_get_value(pb))) {
	    branch->link[1] = cur->link[1];
	    cur->link[1] = ptrb;
      } else {
	    branch->link[1] = ptrb;
	    bnodes_->sym_set_value(pb, branch);
      }

      branch->next_branch = branches_;
      branches_ = branch;
}

vvp_net_t* vvp_island::find_port(const char*key)
{
      if (ports_ == 0)
	    return 0;
      else
	    return ports_->sym_get_value(key);
}

void vvp_island::compile_cleanup()
{
      delete ports_;
      ports_ = 0;

      delete anodes_;
      anodes_ = 0;

      delete bnodes_;
      bnodes_ = 0;
}

vvp_island_port::vvp_island_port(vvp_island*ip)
: island_(ip)
{
}

vvp_island_port::~vvp_island_port()
{
}

void vvp_island_port::recv_vec4(vvp_net_ptr_t, const vvp_vector4_t&bit,
                                vvp_context_t)
{
      vvp_vector8_t tmp (bit, 6, 6);
      if (invalue .eeq(tmp))
	    return;

      invalue = tmp;
      island_->flag_island();
}

void vvp_island_port::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				   unsigned base, unsigned vwid, vvp_context_t)
{
      vvp_vector8_t tmp(bit, 6, 6);
      recv_vec8_pv(port, tmp, base, vwid);
}


void vvp_island_port::recv_vec8(vvp_net_ptr_t, const vvp_vector8_t&bit)
{
      if (invalue .eeq(bit))
	    return;

      invalue = bit;
      island_->flag_island();
}

void vvp_island_port::recv_vec8_pv(vvp_net_ptr_t, const vvp_vector8_t&bit,
                                   unsigned base, unsigned vwid)
{
      if (invalue.size() == 0) {
	    invalue = part_expand(bit, vwid, base);
      } else {
	    unsigned wid = bit.size();
	    assert(invalue.size() == vwid);
	    for (unsigned idx = 0; idx < wid ; idx += 1) {
		  if ((base+idx) >= vwid)
			break;
		  invalue.set_bit(base+idx, bit.value(idx));
	    }
      }

      island_->flag_island();
}

void vvp_island_port::force_flag(bool run_now)
{
      if (run_now)
	    island_->run_island();
      else
	    island_->flag_island();
}

vvp_island_branch::~vvp_island_branch()
{
}

void island_collect_node(list<vvp_branch_ptr_t>&conn, vvp_branch_ptr_t cur)
{
      conn .push_back(cur);
      for (vvp_branch_ptr_t idx = next(cur) ; idx != cur ; idx = next(idx))
	    conn.push_back(idx);
}

/* **** COMPILE/LINK SUPPORT **** */

/*
* We need to keep an island symbol table to make island labels to
* islands, and we need a list of the islands that we can run through
* during cleanup. After linking is done, the compile_island_cleanup() is
* called to erase the symbol table, we still need the list to cleanup the
* island memory at EOS.
*/
static symbol_map_s<vvp_island>* island_table = 0;
static vvp_island** island_list = 0;
static unsigned island_count = 0;

#ifdef CHECK_WITH_VALGRIND
void island_delete()
{
      at_EOS = true;
      for (unsigned idx = 0; idx < island_count; idx += 1) {
	    delete island_list[idx];
      }
      free(island_list);
      island_list = 0;
      island_count = 0;
}
#endif

void compile_island_base(char*label, vvp_island*use_island)
{
      if (island_table == 0)
	    island_table = new symbol_map_s<vvp_island>;

      island_table->sym_set_value(label, use_island);
      island_count += 1;
      island_list = (vvp_island **)realloc(island_list,
                                           island_count*sizeof(vvp_island **));
      island_list[island_count-1] = use_island;
      free(label);
}

vvp_island* compile_find_island(const char*island)
{
      assert(island_table);
      vvp_island*use_island = island_table->sym_get_value(island);
      assert(use_island);
      return use_island;
}

/*
 * This handles the compile of a .port record. A .port is a 2-way port
 * between the island and the outside. For example,
 *
 *   <label> .port <island> <src> ;
 *
 * The <src> is a label in the domain outside the island, and the
 * <label> is in the domain inside the island. Since this port is
 * bi-directional, the <label> is also available in the domain outside
 * the island. The outside should use the <label> to access the nexus
 * that this port represents, because the island will resolve internal
 * drivers with the external driver and make the output available on
 * <label>.
 */
void compile_island_port(char*label, char*island, char*src)
{
      assert(island_table);
      vvp_island*use_island = island_table->sym_get_value(island);
      assert(use_island);
      free(island);

      vvp_net_t*net = new vvp_net_t;
      vvp_island_port*fun = new vvp_island_port(use_island);
      net->fun = fun;

	// Get the source from outside the island
      input_connect(net, 0, src);

	// Define the functor outside the island.
      define_functor_symbol(label, net);

	// Also define it inside the island.
      use_island->add_port(label, net);

      free(label);
}

void compile_island_export(char*label, char*island)
{
      fprintf(stderr, "XXXX %s .export %s;\n", label, island);
      free(label);
      free(island);
}

/*
 * This handles the compile of a .import record. A .import is an input
 * port into the island from the outside domain. For example,
 *
 *   <label> .import <island> <src> ;
 *
 * The <src> is a label in the domain outside the island, and the
 * <label> is in the domain inside the island. Branches within the
 * island use the <label>.
 */
void compile_island_import(char*label, char*island, char*src)
{
      assert(island_table);
      vvp_island*use_island = island_table->sym_get_value(island);
      assert(use_island);
      free(island);

      vvp_net_t*net = new vvp_net_t;
      vvp_island_port*fun = new vvp_island_port(use_island);
      net->fun = fun;

	// Get the source from outside the island
      input_connect(net, 0, src);

	// Define the functor only inside the island.
      use_island->add_port(label, net);

      free(label);
}

void compile_island_cleanup(void)
{
	// Call the per-island cleanup to get rid of local symbol tables.
      for (unsigned idx = 0; idx < island_count; idx += 1) {
	    island_list[idx]->compile_cleanup();
      }

	// If we are not doing valgrind checking then free the list.
#ifndef CHECK_WITH_VALGRIND
      free(island_list);
      island_list = 0;
      island_count = 0;
#endif

	// Remove the island symbol table itself.
      delete island_table;
      island_table = 0;
}
